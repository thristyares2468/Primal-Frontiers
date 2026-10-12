// Real owning-client RPC race/stale-ID diagnostic in a disposable M6 world.
// The frozen replicated clock coordinates phases; consuming one test water
// acknowledges each client's ordered controller RPCs without adding test RPCs.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "Inventory/PFItemPickup.h"
#include "Survival/PFInteraction.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "World/PFWorldClock.h"
#include "PFRequestCodes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace PFInventoryRace
{
    const FName Food=TEXT("Item_Food"),Water=TEXT("Item_Water");
    APFItemPickup* FindFood(UWorld* W)
    {
        for(TActorIterator<APFItemPickup> It(W);It;++It)
        {if(It->GetContents().ItemId==Food){return *It;}}
        return nullptr;
    }
}

class FPFInventoryRaceExercise : public IAutomationLatentCommand
{
public:
    FPFInventoryRaceExercise(FAutomationTestBase* InTest,int32 InExpected)
        :Test(InTest),Expected(InExpected),Started(FPlatformTime::Seconds()){}

    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Stage==99){return Now-Changed>(Client?4:14);}
        if(Now-Started>100)
        {Test->AddError(FString::Printf(TEXT("[PrimalAgentTools] Inventory race timeout stage=%d"),Stage));return true;}
        UWorld* W=nullptr;
        for(const auto& C:GEngine->GetWorldContexts())
        {if(C.World() && C.World()->IsGameWorld()){W=C.World();break;}}
        if(!W){return false;}
        if(UWorld::RemovePIEPrefix(W->GetMapName())!=TEXT("L_M6Creatures") || W->GetNetMode()==NM_Standalone)
        {Test->AddError(TEXT("[PrimalAgentTools] Race requires dedicated L_M6Creatures and real clients"));return true;}
        Client=W->GetNetMode()==NM_Client;
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It)
        {
            auto* P=Cast<APFSurvivalPlayerController>(It->Get());
            if(P && P->GetPawn() && P->GetInventory() && P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>())
            {Players.Add(P);}
        }
        if(Players.Num()!=(Client?1:Expected)){return false;}
        APFWorldClock* Clock=nullptr;
        for(TActorIterator<APFWorldClock> It(W);It;++It){Clock=*It;}
        if(Client){return UpdateClient(W,Players[0],Clock,Now);}
        return UpdateServer(W,Players,Clock,Now);
    }

private:
    void Finish(double Now){Stage=99;Changed=Now;}
    bool Check(bool Value,const TCHAR* Name)
    {return Test->TestTrue(Name,Value);}
    void Phase(APFWorldClock* Clock,const TArray<APFSurvivalPlayerController*>& Players,int32 NewStage,double Now)
    {
        for(auto* PC:Players){PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->SetThirst(10);}
        Clock->SetHour(float(NewStage));Clock->ForceNetUpdate();Stage=NewStage;Changed=Now;
        Test->AddInfo(FString::Printf(TEXT("[PrimalAgentTools] Inventory race server phase=%d"),Stage));
    }
    bool UpdateServer(UWorld* W,const TArray<APFSurvivalPlayerController*>& Players,APFWorldClock* Clock,double Now)
    {
        if(Stage==0)
        {
            for(TActorIterator<APFCreatureSpawner> It(W);It;++It){It->bAutoSpawn=false;}
            for(TActorIterator<APFCreature> It(W);It;++It){It->Destroy();}
            for(TActorIterator<APFItemPickup> It(W);It;++It){It->Destroy();}
            if(!Clock){Clock=W->SpawnActor<APFWorldClock>();}
            if(!Check(IsValid(Clock),TEXT("Fixture replicated clock exists"))){Finish(Now);return false;}
            Clock->SetActorTickEnabled(false);
            int32 Index=0;
            for(auto* PC:Players)
            {
                if(!Check(PC->GetInventory()->GetStacks().IsEmpty(),TEXT("Fresh diagnostic bag, no personal inventory")))
                {Finish(Now);return false;}
                PC->GetPawn()->SetActorLocation(FVector(-1200,4140+120*Index++,100));
                CastChecked<ACharacter>(PC->GetPawn())->GetCharacterMovement()->DisableMovement();
                auto* V=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
                V->HungerDrainPerSecond=0;V->ThirstDrainPerSecond=0;V->SetHunger(50);
                if(!Check(PC->GetInventory()->Grant(PFInventoryRace::Water,5),TEXT("Ready barrier plus four private acknowledgements per participant")))
                {Finish(Now);return false;}
            }
            FVector Eye;FRotator Look;Players[0]->GetPawn()->GetActorEyesViewPoint(Eye,Look);
            const FVector At(-1020,4200,Eye.Z);
            const FTransform Transform(FRotator::ZeroRotator,At);
            auto* Pickup=W->SpawnActorDeferred<APFItemPickup>(APFItemPickup::StaticClass(),Transform,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
            Deadline=UPFInventoryComponent::ServerTime(W)+180;
            Pickup->Initialize(PFInventoryRace::Food,11,Deadline);Pickup->FinishSpawning(Transform);
            for(auto* PC:Players)
            {
                PC->GetPawn()->GetActorEyesViewPoint(Eye,Look);PC->SetControlRotation((At-Eye).Rotation());
                if(!Check(PFInteraction::FindTarget(PC->GetPawn())==Pickup,TEXT("Each authoritative eye trace reaches contested pickup")))
                {Finish(Now);return false;}
            }
            Phase(Clock,Players,9,Now);return false;
        }
        const int32 ExpectedWater=13-Stage; // phase10 ->3, 11 ->2, 12 ->1, 13 ->0
        for(auto* PC:Players){if(PC->GetInventory()->Count(PFInventoryRace::Water)!=ExpectedWater){return false;}}
        if(Now-Changed<2){return false;}
        if(Stage==9){Phase(Clock,Players,10,Now);return false;}
        int32 Total=0,FoodSlots=0,Winners=0,WorldFood=0,WorldBatches=0;
        TSet<FGuid> IDs;
        for(auto* PC:Players)
        {
            auto* I=PC->GetInventory();
            if(I->Count(PFInventoryRace::Food)>0){++Winners;Winner=PC;}
            if(!Check(PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->GetVitals().Hunger==50,TEXT("Invalid/stale consume never recovers hunger")))
            {Finish(Now);return false;}
            for(const auto& S:I->GetStacks())
            {
                if(S.ItemId!=PFInventoryRace::Food){continue;}
                Total+=S.Quantity;++FoodSlots;
                if(!Check(S.ExpiresAt==Deadline && !IDs.Contains(S.StackId),TEXT("Bag batch identity and original deadline conserved")))
                {Finish(Now);return false;}
                IDs.Add(S.StackId);
            }
        }
        for(TActorIterator<APFItemPickup> It(W);It;++It)
        {
            const auto& S=It->GetContents();if(S.ItemId!=PFInventoryRace::Food){continue;}
            WorldFood+=S.Quantity;Total+=S.Quantity;++WorldBatches;
            if(!Check(S.ExpiresAt==Deadline && !IDs.Contains(S.StackId),TEXT("World drop unique batch and original deadline conserved")))
            {Finish(Now);return false;}
            IDs.Add(S.StackId);
        }
        if(!Check(Total==11 && Winners==1,TEXT("Exactly one pickup winner, all eleven food conserved")))
        {Finish(Now);return false;}
        if(Stage==10)
        {
            if(!Check(FoodSlots==1 && WorldBatches==0 && Winner->GetInventory()->Count(PFInventoryRace::Food)==11,TEXT("Competing E requests take contested pickup exactly once")))
            {Finish(Now);return false;}
            Phase(Clock,Players,11,Now);return false;
        }
        if(Stage==11)
        {
            if(!Check(FoodSlots==2 && WorldFood==0,TEXT("Same-frame split burst accepts one operation only")))
            {Finish(Now);return false;}
            const auto& Bag=Winner->GetInventory()->GetStacks();
            if(!Check(Bag.ContainsByPredicate([](const auto& S){return S.ItemId==PFInventoryRace::Food && S.Quantity==7;}) && Bag.ContainsByPredicate([](const auto& S){return S.ItemId==PFInventoryRace::Food && S.Quantity==4;}),TEXT("Split burst leaves seven plus four")))
            {Finish(Now);return false;}
            Phase(Clock,Players,12,Now);return false;
        }
        if(!Check(FoodSlots==1 && WorldBatches==1 && WorldFood==7 && Winner->GetInventory()->Count(PFInventoryRace::Food)==4,TEXT("Repeated drop/stale requests preserve four bag plus seven world")))
        {Finish(Now);return false;}
        if(Stage==12){Phase(Clock,Players,13,Now);return false;}
        Clock->SetHour(15);Clock->ForceNetUpdate();
        Test->AddInfo(TEXT("[PrimalAgentTools] Inventory race server verified all client acknowledgements, single winner, split cooldown, stale IDs, unique batches and eleven-food deadline conservation"));
        Finish(Now);return false;
    }

    bool UpdateClient(UWorld* W,APFSurvivalPlayerController* PC,APFWorldClock* Clock,double Now)
    {
        if(!Clock || PC->GetPlayerSetupStatus()==EPFPlayerSetupStatus::Unconfirmed){return false;}
        const int32 PhaseNumber=FMath::RoundToInt(Clock->Hour);
        if(PhaseNumber==15)
        {
            auto* Drop=PFInventoryRace::FindFood(W);
            if(!Drop || Drop->GetContents().Quantity!=7 || PC->GetInventory()->Count(PFInventoryRace::Water)!=0){return false;}
            Check(PC->GetInventory()->Count(PFInventoryRace::Food)==(Won?4:0),TEXT("Owner receives exact final private inventory"));
            Check(Drop->GetContents().ExpiresAt==Deadline,TEXT("Replicated public drop keeps original food deadline"));
            for(TActorIterator<APlayerState> It(W);It;++It)
            {
                if(*It==PC->PlayerState){continue;}
                auto* Bag=It->FindComponentByClass<UPFInventoryComponent>();
                Check(Bag && Bag->GetStacks().IsEmpty(),TEXT("Peer inventory contents remain owner-private"));
            }
            Test->AddInfo(TEXT("[PrimalAgentTools] Inventory race client verified owned RPC phases, direct mutation refusal, final private inventory and public drop expiry"));
            Finish(Now);return false;
        }
        if(PhaseNumber<9 || PhaseNumber>13){return false;}
        if(Stage!=PhaseNumber)
        {
            if(PhaseNumber==9)
            {
                auto* Pickup=PFInventoryRace::FindFood(W);
                if(!Pickup || Pickup->GetContents().Quantity!=11 || PC->GetInventory()->Count(PFInventoryRace::Water)!=5){return false;}
                Deadline=Pickup->GetContents().ExpiresAt;
                for(const auto& S:PC->GetInventory()->GetStacks()){if(S.ItemId==PFInventoryRace::Water){WaterId=S.StackId;}}
                FVector Eye;FRotator Look;PC->GetPawn()->GetActorEyesViewPoint(Eye,Look);PC->SetControlRotation((Pickup->GetActorLocation()-Eye).Rotation());
                if(!Check(PFInteraction::FindTarget(PC->GetPawn())==Pickup,TEXT("Client crosshair sees contested pickup")))
                {Finish(Now);return false;}
                Check(!Pickup->TryPickup(PC->GetPawn()),TEXT("Direct client pickup mutation refused"));
                Check(!PC->GetInventory()->AddExisting(PFInventoryRace::Food,11,Deadline),TEXT("Direct client bag mutation refused"));
            }
            Stage=PhaseNumber;Changed=Now;Action=0;
        }
        if(Now-Changed<1){return false;}
        if(Action==0)
        {
            if(Stage==10){PC->Interact();PC->Interact();}
            else if(Stage==11)
            {
                Won=PC->GetInventory()->Count(PFInventoryRace::Food)==11;
                if(Won)
                {
                    for(const auto& S:PC->GetInventory()->GetStacks()){if(S.ItemId==PFInventoryRace::Food){OriginalId=S.StackId;}}
                    PC->ServerInventoryAction(OriginalId,PFInventoryAction::Split,4);
                    PC->ServerInventoryAction(OriginalId,PFInventoryAction::Split,4);
                }
            }
            else if(Stage==12 && Won)
            {
                PC->ServerInventoryAction(OriginalId,PFInventoryAction::Drop,7);
                PC->ServerInventoryAction(OriginalId,PFInventoryAction::Drop,7);
            }
            else if(Stage==13)
            {
                Check(!PC->GetInventory()->GetStacks().ContainsByPredicate([&](const auto& S){return S.StackId==OriginalId;}),TEXT("Removed stack is absent before stale requests"));
                if(!Won){OriginalId=FGuid::NewGuid();}
                PC->ServerInventoryAction(OriginalId,PFInventoryAction::Split,1);
            }
            Action=1;Changed=Now;return false;
        }
        if(Stage==13 && Action<3)
        {
            if(Now-Changed<0.5){return false;}
            PC->ServerInventoryAction(OriginalId,Action==1?PFInventoryAction::Drop:PFInventoryAction::Consume,1);
            ++Action;Changed=Now;return false;
        }
        if(Action==4 || Now-Changed<0.5){return false;}
        PC->ServerInventoryAction(WaterId,PFInventoryAction::Consume,1);Action=4;
        Test->AddInfo(FString::Printf(TEXT("[PrimalAgentTools] Inventory race client acknowledged phase=%d winner=%d"),Stage,Won));
        return false;
    }
    FAutomationTestBase* Test;
    int32 Expected,Stage=0,Action=0;
    double Started,Changed=0,Deadline=0;
    bool Client=false,Won=false;
    FGuid WaterId,OriginalId;
    TWeakObjectPtr<APFSurvivalPlayerController> Winner;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFInventoryRaceLiveTest,"PF.Inventory.RaceLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFInventoryRaceLiveTest::RunTest(const FString&)
{
    int32 Players=0;FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Players);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunInventoryRaceLiveTests")) || (Players!=1 && Players!=2))
    {AddError(TEXT("[PrimalAgentTools] Requires isolated inventory race runner, real dedicated L_M6Creatures and PFExpectedPlayers=1|2"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FPFInventoryRaceExercise(this,Players));return true;
}
#endif
