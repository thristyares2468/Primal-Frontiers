// Isolated, non-rendered M7 streaming probe. Deliberate test teleports do not
// certify manual traversal. No map/asset/config is saved by this test.
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR && !UE_BUILD_SHIPPING
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "World/PFWorldClock.h"
#include "Crafting/PFResourceNode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"

class FPFWorldStreamingExercise : public IAutomationLatentCommand
{
public:
    explicit FPFWorldStreamingExercise(FAutomationTestBase* InTest)
        : Test(InTest), Started(FPlatformTime::Seconds()) {}
    ~FPFWorldStreamingExercise() override { RestorePlayer(); }

    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>60)
        {
            Test->AddError(FString::Printf(TEXT("[PrimalWorld] Streaming timeout stage=%d; real unload/reload was not verified."),Stage));
            return true;
        }
        UWorld* World=nullptr;
        for(const auto& Context:GEngine->GetWorldContexts())
        {
            if(Context.World() && Context.World()->IsGameWorld()){World=Context.World();break;}
        }
        if(!World){return false;}
        if(World->GetNetMode()!=NM_Standalone || !World->GetMapName().EndsWith(TEXT("L_PrimalFrontier_OpenWorld")))
        {
            Test->AddError(TEXT("[PrimalWorld] Requires an isolated Standalone L_PrimalFrontier_OpenWorld session."));
            return true;
        }
        auto* Streaming=World->GetSubsystem<UWorldPartitionSubsystem>();
        if(!World->GetWorldPartition() || !Streaming)
        {
            Test->AddError(TEXT("[PrimalWorld] World Partition streaming subsystem is unavailable."));
            return true;
        }
        TSet<FString> Landmarks;
        for(TActorIterator<AStaticMeshActor> It(World);It;++It)
        {
            if(It->GetActorLabel().StartsWith(TEXT("OW_Landmark"))){Landmarks.Add(It->GetActorLabel());}
        }
        if(Stage==0)
        {
            auto* PC=Cast<APFSurvivalPlayerController>(World->GetFirstPlayerController());
            auto* Character=PC?Cast<ACharacter>(PC->GetPawn()):nullptr;
            auto* Vitals=Character?Character->FindComponentByClass<UPFPlayerSurvivalComponent>():nullptr;
            APFResourceNode* Wood=nullptr;
            for(TActorIterator<APFResourceNode> It(World);It;++It)
            {
                if(It->ResourceId==TEXT("Node_Wood") && It->GetActorLocation().Y<0){Wood=*It;}
            }
            for(TActorIterator<APFWorldClock> It(World);It;++It){Clock=*It;}
            if(!Character || !Vitals || !Wood || !Clock.IsValid() || Landmarks.Num()!=4 || !Streaming->IsAllStreamingCompleted()){return false;}
            Pawn=Character;
            OriginalPosition=Character->GetActorLocation();
            OriginalRotation=PC->GetControlRotation();
            OriginalMovement=Character->GetCharacterMovement()->MovementMode;
            OriginalCustomMovement=Character->GetCharacterMovement()->CustomMovementMode;
            OriginalHungerRate=Vitals->HungerDrainPerSecond;
            OriginalThirstRate=Vitals->ThirstDrainPerSecond;
            OriginalDayLength=Clock->DayLengthSeconds;
            OriginalHour=Clock->Hour;
            bRestore=true;
            Character->GetCharacterMovement()->DisableMovement();
            Vitals->HungerDrainPerSecond=0;
            Vitals->ThirstDrainPerSecond=0;
            Clock->DayLengthSeconds=86400;
            if(!Test->TestTrue(TEXT("Authoritative night setup"),Clock->SetHour(22))){return true;}
            Character->SetActorLocation(Wood->GetActorLocation()+FVector(0,-180,40));
            FVector Eye;FRotator Look;
            Character->GetActorEyesViewPoint(Eye,Look);
            PC->SetControlRotation((Wood->GetActorLocation()-Eye).Rotation());
            const int32 Before=Wood->HitsRemaining;
            if(!Test->TestTrue(TEXT("Harvest through real server validation"),Wood->Gather(Character))){return true;}
            Test->TestTrue(TEXT("Harvest spent finite node state"),Wood->HitsRemaining<Before);
            for(TActorIterator<APFResourceNode> It(World);It;++It){Nodes.Add(*It);Hits.Add(It->HitsRemaining);}
            if(!Test->TestEqual(TEXT("Fifteen core resources loaded"),Nodes.Num(),15)){return true;}
            OriginalLandmarks=Landmarks;
            // Outside the tiny map and beyond all landmark cell loading ranges.
            // Movement is disabled so this probe cannot fall/respawn accidentally.
            Character->SetActorLocation(FVector(-100000,-100000,100));
            Stage=1;Changed=Now;
            Test->AddInfo(TEXT("[PrimalWorld] Streaming probe moved outside landmark loading range; waiting for actual unload."));
            return false;
        }
        if(Now-Changed<1 || !Streaming->IsAllStreamingCompleted()){return false;}
        if(Stage==1)
        {
            if(!Landmarks.IsEmpty()){return false;}
            VerifyCore(World);
            if(!Pawn.IsValid()){Test->AddError(TEXT("[PrimalWorld] Probe pawn vanished."));return true;}
            Pawn->SetActorLocation(OriginalPosition);
            Stage=2;Changed=Now;
            Test->AddInfo(TEXT("[PrimalWorld] All four landmarks unloaded; returning to the camp."));
            return false;
        }
        for(const FString& Label:OriginalLandmarks){if(!Landmarks.Contains(Label)){return false;}}
        VerifyCore(World);
        Test->TestEqual(TEXT("No duplicate landmarks after reload"),Landmarks.Num(),4);
        Test->AddInfo(TEXT("[PrimalWorld] Actual landmark unload/reload preserved core actor identity, harvested hits, night clock and camp ground collision. Manual route/overnight acceptance remains separate."));
        return true;
    }

private:
    void VerifyCore(UWorld* World)
    {
        TSet<APFResourceNode*> LiveNodes;
        for(TActorIterator<APFResourceNode> It(World);It;++It){LiveNodes.Add(*It);}
        Test->TestEqual(TEXT("Resource count survives streaming"),LiveNodes.Num(),15);
        for(int32 I=0;I<Nodes.Num();++I)
        {
            if(Test->TestTrue(TEXT("Same core resource instance remains active in this world"),Nodes[I].IsValid() && LiveNodes.Contains(Nodes[I].Get())))
            {
                Test->TestEqual(TEXT("Harvested state does not refill on streaming"),Nodes[I]->HitsRemaining,Hits[I]);
            }
        }
        int32 ClockCount=0;
        APFWorldClock* LiveClock=nullptr;
        for(TActorIterator<APFWorldClock> It(World);It;++It){LiveClock=*It;++ClockCount;}
        Test->TestEqual(TEXT("Unique clock survives streaming"),ClockCount,1);
        if(Test->TestTrue(TEXT("Same clock instance remains active in this world"),Clock.IsValid() && LiveClock==Clock.Get()))
        {
            Test->TestTrue(TEXT("Night clock did not reset to initial daytime"),Clock->Hour>=22 && Clock->Hour<23);
        }
        FHitResult Hit;
        Test->TestTrue(TEXT("Camp retains ground collision while landmarks stream"),World->LineTraceSingleByChannel(Hit,FVector(0,-1500,300),FVector(0,-1500,-300),ECC_Visibility));
    }
    void RestorePlayer()
    {
        if(!bRestore){return;}
        bRestore=false;
        if(Pawn.IsValid())
        {
            Pawn->SetActorLocation(OriginalPosition);
            Pawn->GetCharacterMovement()->SetMovementMode(OriginalMovement,OriginalCustomMovement);
            if(auto* PC=Pawn->GetController()){PC->SetControlRotation(OriginalRotation);}
            if(auto* Vitals=Pawn->FindComponentByClass<UPFPlayerSurvivalComponent>())
            {
                Vitals->HungerDrainPerSecond=OriginalHungerRate;
                Vitals->ThirstDrainPerSecond=OriginalThirstRate;
            }
        }
        if(Clock.IsValid()){Clock->DayLengthSeconds=OriginalDayLength;Clock->SetHour(OriginalHour);}
    }
    FAutomationTestBase* Test;
    double Started,Changed=0;
    int32 Stage=0;
    bool bRestore=false;
    TWeakObjectPtr<ACharacter> Pawn;
    TWeakObjectPtr<APFWorldClock> Clock;
    TArray<TWeakObjectPtr<APFResourceNode>> Nodes;
    TArray<int32> Hits;
    TSet<FString> OriginalLandmarks;
    FVector OriginalPosition;
    FRotator OriginalRotation;
    EMovementMode OriginalMovement=MOVE_Walking;
    uint8 OriginalCustomMovement=0;
    float OriginalHungerRate=0,OriginalThirstRate=0,OriginalDayLength=900,OriginalHour=9;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWorldStreamingTest,"PF.World.Streaming",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFWorldStreamingTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunWorldStreamingTests")))
    {
        AddError(TEXT("Requires isolated Standalone L_PrimalFrontier_OpenWorld -PFRunWorldStreamingTests; NullRHI is supported."));
        return false;
    }
    ADD_LATENT_AUTOMATION_COMMAND(FPFWorldStreamingExercise(this));
    return true;
}
#endif
