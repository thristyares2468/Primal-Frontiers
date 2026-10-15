#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFResourceNode.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFSaveFileStore.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFInteraction.h"
#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "World/PFWorldClock.h"
#include "PFRequestCodes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
class FCopperMigrationLive : public IAutomationLatentCommand
{
public:
    explicit FCopperMigrationLive(FAutomationTestBase* T):Test(T)
    {FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot);Restart=FParse::Param(FCommandLine::Get(),TEXT("PFLoadSave"));}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>110){Test->AddError(FString::Printf(TEXT("Copper migration timeout phase=%d"),Stage));return true;}
        UWorld* W=nullptr;for(const auto& X:GEngine->GetWorldContexts()){if(X.World() && X.World()->IsGameWorld()){W=X.World();break;}}
        if(!W){return false;}Client=W->GetNetMode()==NM_Client;
        if(Stage==99){return Now-Changed>(Client?3:12);}
        if(UWorld::RemovePIEPrefix(W->GetMapName())!=TEXT("L_PrimalFrontier_OpenWorld") || W->GetNetMode()==NM_Standalone){Test->AddError(TEXT("Requires actual OpenWorld dedicated migration fixture"));return true;}
        APFWorldClock* Clock=nullptr;for(TActorIterator<APFWorldClock> It(W);It;++It){Clock=*It;}
        if(!Clock){return false;}
        if(!Client && !Frozen)
        {
            Clock->SetActorTickEnabled(false);for(TActorIterator<APFCreatureSpawner> It(W);It;++It){It->bAutoSpawn=false;}
            for(TActorIterator<APFCreature> It(W);It;++It){It->Destroy();}
            CurrentResources=0;for(TActorIterator<APFResourceNode> It(W);It;++It){It->SetActorTickEnabled(false);++CurrentResources;}Frozen=true;
        }
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It){auto* PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC && Cast<APFSurvivorCharacter>(PC->GetPawn()) && PC->GetInventory() && PC->GetInventory()->Catalog){Players.Add(PC);}}
        if(Players.Num()!=(Client?1:Expected)){return false;}
        APFResourceNode* Copper=nullptr;for(TActorIterator<APFResourceNode> It(W);It;++It){if(It->ActorHasTag(TEXT("PF.M12.CopperTest")) || It->ResourceId==TEXT("Node_Copper")){Copper=*It;}}
        return Client?Owner(W,Players[0],Clock,Copper,Now):Server(W,Players,Clock,Copper,Now);
    }
private:
    bool Check(bool V,const TCHAR* Name){return Test->TestTrue(Name,V);}
    void Phase(APFWorldClock* Clock,const TArray<APFSurvivalPlayerController*>& Players,int32 N,double Now)
    {for(auto* PC:Players){CastChecked<APFSurvivorCharacter>(PC->GetPawn())->Survival->SetThirst(10);}Clock->SetHour(float(N));Stage=N;Changed=Now;}
    void Done(double Now){Stage=99;Changed=Now;}
    bool Server(UWorld* W,const TArray<APFSurvivalPlayerController*>& Players,APFWorldClock* Clock,APFResourceNode* Copper,double Now)
    {
        auto* Save=W->GetSubsystem<UPFWorldPersistence>();FString Error;
        if(Stage==0)
        {
            if(!Restart)
            {
                if(!Check(Copper && Copper->ResourceId==TEXT("Node_Copper") && Copper->HitsRemaining==6,TEXT("Actual authored Copper fixture exists"))){return true;}
                Copper->Destroy(); // Old executable layout simulated in disposable runtime only.
                // Later reviewed home-world fixtures must also be absent from the
                // synthetic historical executable layout, never from the saved editor map.
                for(TActorIterator<APFResourceNode> It(W);It;++It){if(It->ActorHasTag(TEXT("PF.M12.EryndorTest"))){It->Destroy();}}
                int32 Role=0;
                for(auto* PC:Players)
                {
                    auto* I=PC->GetInventory();auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* G=PC->GetPlayerState<APFInventoryPlayerState>()->Progression.Get();
                    if(!Check(I->GetStacks().IsEmpty(),TEXT("Fresh isolated owner"))){return true;}
                    FPFProgressionRecord Seed;Seed.Experience=100;Seed.Knowledge={FName(TEXT("Tech_FieldTools"))};if(Role==0){Seed.Stats.Stamina=1;}else{Seed.Stats.Weight=1;}
                    if(!Check(G->Restore(Seed,Error),TEXT("Explicit legacy upgraded-owner boundary seed"))){return true;}
                    P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;P->Survival->StaminaRecoveryPerSecond=0;P->Survival->SetHealth(33.f+Role);P->Survival->SetHunger(47);P->Survival->ChangeStamina(Role==0?12.f:-5.f);
                    P->SetActorLocation(FVector(Role==0?-250:250,-1100,100));P->GetCharacterMovement()->DisableMovement();++Role;
                    if(!Check(I->Grant(TEXT("Item_Tool"),1) && I->Grant(TEXT("Item_Food"),2) && I->Grant(TEXT("Item_Water"),3),TEXT("Old catalog items only, never Copper/XP client grants"))){return true;}
                }
            }
            else
            {
                FPFSavedFile Original;FPFWorldSaveData Old;
                if(!Check(FPFSaveFileStore::ReadOriginal(Slot,CurrentResources==25?TEXT("M12EryndorLayout1"):TEXT("M12CopperLayout1"),Original,Error) && Save->Decode(Original.Payload,Old,Error) && Old.Resources.Num()==15,TEXT("Actual startup preserved verified old15 original"))){Test->AddError(Error);return true;}
                if(!Check(CurrentResources==16 || CurrentResources==25,TEXT("Only reviewed current layouts accepted"))){return true;}
                for(TActorIterator<APFResourceNode> It(W);It;++It){if(It->ActorHasTag(TEXT("PF.M12.EryndorTest"))){Check(It->HitsRemaining==8,TEXT("New home-world mineral initialized once after startup"));}}
                if(!Check(Copper && Copper->HitsRemaining==6,TEXT("New default appears once after actual server restart"))){return true;}
                int32 Lane=0;
                for(auto* PC:Players)
                {
                    auto* I=PC->GetInventory();auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());auto* G=PC->GetPlayerState<APFInventoryPlayerState>()->Progression.Get();
                    const int32 Role=FMath::RoundToInt(P->Survival->GetVitals().Health)-33;
                    if(!Check(Role>=0 && Role<Expected && I->Count(TEXT("Item_Tool"))==1 && I->Count(TEXT("Item_Food"))==2 && I->Count(TEXT("Item_Water"))==1 && I->Count(TEXT("Item_CopperOre"))==0 && G->GetExperience()==100 && G->GetAvailablePoints()==1 && G->GetAvailableStatPoints()==0,TEXT("Restart restores independent old owner state without Copper grants"))){return true;}
                    if(!Check(P->Survival->GetVitals().MaxStamina==(Role==0?120:100) && G->GetRecord().Stats.Stamina==(Role==0?1:0) && G->GetRecord().Stats.Weight==(Role==0?0:1),TEXT("Upgraded stamina/weight restored from private record"))){return true;}
                    P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;P->GetCharacterMovement()->DisableMovement();P->SetActorLocation(FVector(-190,-850+(Expected==2?(Lane?100:-100):0),100));++Lane;
                    FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);const auto Facing=(Copper->GetActorLocation()-Eye).Rotation();PC->SetControlRotation(Facing);PC->ClientSetRotation(Facing,false);
                }
            }
            Phase(Clock,Players,Restart?10:8,Now);return false;
        }
        const int32 Water=Restart?0:Stage==8?2:1;
        for(auto* PC:Players){if(PC->GetInventory()->Count(TEXT("Item_Water"))!=Water){return false;}}
        if(Now-Changed<2){return false;}
        if(!Restart && Stage==8)
        {
            for(auto* PC:Players){CastChecked<APFSurvivorCharacter>(PC->GetPawn())->Survival->SetThirst(27);}
            for(TActorIterator<APFResourceNode> It(W);It;++It){if(It->GetName()==TEXT("PFResourceNode_0")){It->RestorePersistence(0,12);}}
            FPFWorldSaveData Old;TArray<uint8> Bytes;
            if(!Check(Save->Capture(Old,Error) && Old.Resources.Num()==15 && Save->Encode(Old,Bytes,Error) && FPFSaveFileStore::Write(Slot,Bytes,Error),TEXT("Actual old-layout source saved with canonical owners"))){Test->AddError(Error);return true;}
            Phase(Clock,Players,9,Now);return false;
        }
        if(!Restart)
        {
            for(auto* PC:Players){CastChecked<APFSurvivorCharacter>(PC->GetPawn())->Survival->SetThirst(27);}
            Test->AddInfo(TEXT("[PrimalAgentTools] Copper create server actual old15 layout and independent owners saved"));Clock->SetHour(14);Done(Now);return false;
        }
        int32 Total=0;for(auto* PC:Players){const int32 Ore=PC->GetInventory()->Count(TEXT("Item_CopperOre"));Total+=Ore;Check(Ore==4 && PC->GetPlayerState<APFInventoryPlayerState>()->Progression->GetExperience()==100,TEXT("Each actual owned gather is exact finite ore without XP grant"));}
        if(!Check(Copper && Copper->HitsRemaining==6-2*Expected && Total==4*Expected,TEXT("Shared authored node cannot duplicate finite yield"))){return true;}
        if(!Check(Save->Save(Slot,Error),TEXT("Normal post-migration save captures current aged runtime"))){Test->AddError(Error);return true;}
        FPFSavedFile Current,Original;FPFWorldSaveData Data,Old;
        if(!Check(FPFSaveFileStore::Read(Slot,Current,Error) && Save->Decode(Current.Payload,Data,Error) && Data.Resources.Num()==CurrentResources && Data.Players.Num()==Expected && FPFSaveFileStore::ReadOriginal(Slot,CurrentResources==25?TEXT("M12EryndorLayout1"):TEXT("M12CopperLayout1"),Original,Error) && Save->Decode(Original.Payload,Old,Error) && Old.Resources.Num()==15,TEXT("Reviewed current layout saved and immutable old15 retained after rotation"))){return true;}
        Test->AddInfo(FString::Printf(TEXT("[PrimalAgentTools] Copper restart server actual migration, preserved original, independent owners, finite shared gather and current%d save verified"),CurrentResources));Clock->SetHour(14);Done(Now);return false;
    }
    bool Owner(UWorld* W,APFSurvivalPlayerController* PC,APFWorldClock* Clock,APFResourceNode* Copper,double Now)
    {
        const int32 PhaseId=FMath::RoundToInt(Clock->Hour);if(PhaseId==14 && Stage>0){Done(Now);return false;}
        if(PhaseId!=(Restart?10:8) && (Restart || PhaseId!=9)){return false;}if(Stage==PhaseId){return false;}
        if(Observed!=PhaseId){Observed=PhaseId;ObservedAt=Now;return false;}if(Now-ObservedAt<.5){return false;}
        auto* I=PC->GetInventory();auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());
        const int32 Required=Restart?1:PhaseId==8?3:2;if(I->Count(TEXT("Item_Water"))!=Required || P->Survival->GetVitals().Thirst!=10 || !PS || !PS->Progression){return false;}
        if(!Restart)
        {
            if(I->Count(TEXT("Item_Tool"))!=1 || I->Count(TEXT("Item_Food"))!=2){return false;}
            Check(!I->Grant(TEXT("Item_CopperOre"),1),TEXT("Client cannot manufacture new material"));
            if(PhaseId==9){Test->AddInfo(TEXT("[PrimalAgentTools] Copper create client old-layout seed and private acknowledgement verified"));}
        }
        else
        {
            if(!Copper || I->Count(TEXT("Item_Tool"))!=1 || PS->Progression->GetExperience()!=100){return false;}
            FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);PC->SetControlRotation((Copper->GetActorLocation()-Eye).Rotation());
            if(PFInteraction::FindTarget(P)!=Copper){return false;}
            Check(!Copper->Gather(P),TEXT("Client cannot directly mutate actual authored vein"));
            for(TActorIterator<APFInventoryPlayerState> It(W);It;++It){if(*It!=PS){Check(It->Inventory->GetStacks().IsEmpty() && It->Progression->GetExperience()==0,TEXT("Foreign original inventory/progression remains private"));}}
            UFunction* Interact=PC->FindFunctionChecked(TEXT("ServerInteract"));PC->ProcessEvent(Interact,nullptr);PC->ProcessEvent(Interact,nullptr);
            Test->AddInfo(TEXT("[PrimalAgentTools] Copper restart client actual saved owner, private state and owned authored gather verified"));
        }
        const auto* WaterStack=I->GetStacks().FindByPredicate([](const auto& S){return S.ItemId==TEXT("Item_Water");});if(!WaterStack){return false;}
        PC->ServerInventoryAction(WaterStack->StackId,PFInventoryAction::Consume,1);Stage=PhaseId;return false;
    }
    FAutomationTestBase* Test;FString Slot;int32 Expected=1,Stage=0,Observed=0,CurrentResources=16;bool Restart=false,Client=false,Frozen=false;
    double Started=FPlatformTime::Seconds(),Changed=0,ObservedAt=0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCopperMigrationLiveTest,"PF.Persistence.CopperLive",EAutomationTestFlags::ClientContext|EAutomationTestFlags::ServerContext|EAutomationTestFlags::EngineFilter)
bool FPFCopperMigrationLiveTest::RunTest(const FString&)
{
    FString Slot;int32 Players=0;
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunCopperMigrationTests")) || !FParse::Param(FCommandLine::Get(),TEXT("nullrhi")) ||
       !FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Players) || (Players!=1 && Players!=2) ||
       !FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot) || !Slot.StartsWith(TEXT("AutomationM12CopperLive")) || !FPFSaveFileStore::ValidSlot(Slot))
    {AddError(TEXT("Requires isolated NullRHI Copper runner,1|2players and disposable AutomationM12CopperLive slot"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FCopperMigrationLive(this));return true;
}
#endif
