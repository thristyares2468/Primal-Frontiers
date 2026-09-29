#include "PFCommands.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildPiece.h"
#include "Inventory/PFInventoryComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "GameFramework/PlayerState.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
class FPFBuildingLiveExercise final : public IAutomationLatentCommand
{
public:
    explicit FPFBuildingLiveExercise(FAutomationTestBase* In):Test(In),Started(FPlatformTime::Seconds()){FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Stage==99){return Now-Changed>(bClient?3:12);}if(Now-Started>180){Test->AddError(FString::Printf(TEXT("[PrimalBuilding] Live timeout stage=%d"),Stage));return true;}
        UWorld* W=nullptr;for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld()){W=C.World();break;}}if(!W){return false;}bClient=W->GetNetMode()==NM_Client;
        TArray<APFSurvivalPlayerController*> Players;for(auto It=W->GetPlayerControllerIterator();It;++It){auto* PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC && PC->GetPawn() && PC->GetInventory() && PC->Building->Catalog){Players.Add(PC);}}if(Players.Num()!=(bClient?1:Expected)){return false;}
        auto* PC=Players[0];auto* B=PC->Building.Get();auto* I=PC->GetInventory();
        APFBuildPiece* Base=nullptr;APFBuildPiece* Wall=nullptr;APFBuildPiece* Chest=nullptr;APFBuildPiece* Roof=nullptr;int32 OtherCount=0;
        for(TActorIterator<APFBuildPiece> It(W);It;++It){if(It->Builder==PC->PlayerState){if(It->Kind==EPFBuildKind::Foundation){Base=*It;}else if(It->Kind==EPFBuildKind::Wall){Wall=*It;}else if(It->Kind==EPFBuildKind::Storage){Chest=*It;}else if(It->Kind==EPFBuildKind::Ceiling){Roof=*It;}}else{++OtherCount;}}
        auto Advance=[&](int32 Next){Stage=Next;Changed=Now;Test->AddInfo(FString::Printf(TEXT("[PrimalBuilding] %s stage=%d wood=%d food=%d"),bClient?TEXT("Client"):TEXT("Server"),Stage,I->Count(TEXT("Item_Wood")),I->Count(TEXT("Item_Food"))));};
        auto Aim=[&](FVector At){FVector Eye;FRotator R;PC->GetPawn()->GetActorEyesViewPoint(Eye,R);PC->SetControlRotation((At-Eye).Rotation());};
        if(!bClient)
        {
            if(Stage==0){int32 N=0;for(auto* P:Players){auto* V=P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();V->HungerDrainPerSecond=0;V->ThirstDrainPerSecond=0;P->GetPawn()->SetActorLocation(FVector(-450,1600+800*N++,100));P->GetInventory()->Grant(TEXT("Item_Wood"),12);P->GetInventory()->Grant(TEXT("Item_Food"),2);}Advance(1);}
            else if(Stage==1)
            {
                int32 Ready=0;for(TActorIterator<APFBuildPiece> It(W);It;++It){if(It->Kind==EPFBuildKind::Storage && It->Health==75 && It->Storage->Count(TEXT("Item_Food"))==1){++Ready;}}
                if(Ready!=Expected){return false;}for(auto* P:Players){Test->TestEqual(TEXT("Server building costs"),P->GetInventory()->Count(TEXT("Item_Wood")),4);Test->TestEqual(TEXT("Server deposit conservation"),P->GetInventory()->Count(TEXT("Item_Food")),1);}
                Test->TestFalse(TEXT("Server structure validation"),PF::AgentTools::ExecuteCommand(TEXT("PF.TestBuildingPlacement"),{},W).HasErrors());PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"),{TEXT("M5LiveServer")},W);Advance(99);
            }
        }
        else
        {
            if(Stage==0){if(I->Count(TEXT("Item_Wood"))!=12 || FMath::Abs(PC->GetPawn()->GetActorLocation().Y)<1000){return false;}Test->TestNull(TEXT("Direct client place rejected"),B->Place(TEXT("Build_Foundation"),0));B->ServerPlace(TEXT("Build_Unknown"),0);Aim(FVector(0,FMath::GridSnap(PC->GetPawn()->GetActorLocation().Y,400.0),0));Advance(1);}
            else if(Stage==1 && Now-Changed>0.7){Test->TestEqual(TEXT("Unknown piece costs nothing"),I->Count(TEXT("Item_Wood")),12);B->ServerPlace(TEXT("Build_Foundation"),0);Advance(2);}
            else if(Stage==2 && Base){Test->TestEqual(TEXT("Replicated owner"),Base->Builder.Get(),PC->PlayerState.Get());Aim(Base->GetActorLocation());Advance(3);}
            else if(Stage==3 && Now-Changed>0.7){B->ServerPlace(TEXT("Build_Wall"),0);Advance(4);}
            else if(Stage==4 && Wall){Test->TestEqual(TEXT("Replicated support"),Wall->Support.Get(),Base);Aim(Wall->GetActorLocation());Advance(5);}
            else if(Stage==5 && Now-Changed>0.7){B->ServerPlace(TEXT("Build_Ceiling"),0);Advance(6);}
            else if(Stage==6 && Roof){Aim(Base->GetActorLocation());Advance(7);}
            else if(Stage==7 && Now-Changed>0.7){B->ServerPlace(TEXT("Build_Storage"),0);Advance(8);}
            else if(Stage==8 && Chest){Aim(Chest->GetActorLocation());Advance(9);}
            else if(Stage==9 && Now-Changed>0.7){B->ServerTargetAction(1);Advance(10);}
            else if(Stage==10 && B->OpenStorage==Chest && Now-Changed>0.7){const auto* Food=I->GetStacks().FindByPredicate([](const auto& S){return S.ItemId==TEXT("Item_Food");});if(!Food){return false;}Deadline=Food->ExpiresAt;B->ServerTransfer(true,Food->StackId,1);Advance(11);}
            else if(Stage==11 && Chest && Chest->Storage->Count(TEXT("Item_Food"))==1 && Now-Changed>0.7){Test->TestEqual(TEXT("Replicated freshness conserved"),Chest->Storage->GetStacks()[0].ExpiresAt,Deadline);B->ServerTargetAction(0);Advance(12);}
            else if(Stage==12 && Now-Changed>0.7){Test->TestNotNull(TEXT("Occupied storage demolition rejected"),Chest);if(!Chest){return true;}B->ServerTargetAction(2);Advance(13);}
            else if(Stage==13 && Chest && Chest->Health==75 && OtherCount==4*(Expected-1))
            {
                Test->TestEqual(TEXT("Replicated bag conservation"),I->Count(TEXT("Item_Food")),1);Test->TestEqual(TEXT("Replicated costs"),I->Count(TEXT("Item_Wood")),4);
                for(TActorIterator<APFBuildPiece> It(W);It;++It){if(It->Builder!=PC->PlayerState){Test->TestFalse(TEXT("Client cannot demolish other structure"),B->Demolish(*It));if(It->Kind==EPFBuildKind::Storage){Test->TestTrue(TEXT("Other storage remains private"),It->Storage->GetStacks().IsEmpty());}}}
                const auto Denied=PF::AgentTools::ExecuteCommand(TEXT("PF.ResetBuildings"),{},W,false);Test->TestTrue(TEXT("Client reset denied"),Denied.Issues.ContainsByPredicate([](const auto& E){return E.Code==TEXT("NotAuthority");}));PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"),{TEXT("M5LiveClient")},W);Advance(99);
            }
        }
        return false;
    }
private:
    FAutomationTestBase* Test;double Started,Changed=0,Deadline=0;int32 Stage=0,Expected=1;bool bClient=false;
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFBuildingLiveTest,"PF.Building.Live",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFBuildingLiveTest::RunTest(const FString&)
{if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunBuildingLiveTests"))){AddError(TEXT("Requires isolated M5 session -PFRunBuildingLiveTests -PFExpectedPlayers=1|2"));return false;}ADD_LATENT_AUTOMATION_COMMAND(FPFBuildingLiveExercise(this));return true;}
#endif
