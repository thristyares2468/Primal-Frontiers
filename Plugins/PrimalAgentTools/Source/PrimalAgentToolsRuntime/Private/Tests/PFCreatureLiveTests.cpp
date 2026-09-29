#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Inventory/PFItemPickup.h"
#include "PFCommands.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/DamageEvents.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/PlayerState.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

class FPFCreatureLiveExercise : public IAutomationLatentCommand
{
public:
    explicit FPFCreatureLiveExercise(FAutomationTestBase* InTest):Test(InTest),Started(FPlatformTime::Seconds())
    {FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);}
    virtual bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Stage==99){return Now-Changed>(bClient?4:14);}
        if(Now-Started>120){Test->AddError(FString::Printf(TEXT("[PrimalCreatures] Live timeout stage=%d movedPassive=%d movedHostile=%d"),Stage,bPassiveMoved,bHostileMoved));return true;}
        UWorld* W=nullptr;for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld()){W=C.World();break;}}if(!W){return false;}bClient=W->GetNetMode()==NM_Client;
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It){auto* PC=Cast<APFSurvivalPlayerController>(It->Get());if(PC && PC->GetPawn() && PC->GetInventory()){Players.Add(PC);}}
        if(Players.Num()!=(bClient?1:Expected)){return false;}
        auto* PC=Players[0];auto* V=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();if(!V){return false;}
        auto Advance=[&](int32 NewStage){Stage=NewStage;Changed=Now;Test->AddInfo(FString::Printf(TEXT("[PrimalCreatures] %s stage=%d"),bClient?TEXT("Client"):TEXT("Server"),Stage));};
        if(!bClient && Stage==0)
        {
            for(TActorIterator<APFCreatureSpawner> It(W);It;++It){It->bAutoSpawn=false;}
            for(TActorIterator<APFCreature> It(W);It;++It){It->Destroy();}
            int32 Index=0;for(auto* Player:Players){Player->GetPawn()->SetActorLocation(FVector(-1200+Index++*2400,4200,100));auto* Needs=Player->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();Needs->SetHealth(100);Needs->HungerDrainPerSecond=0;Needs->ThirstDrainPerSecond=0;}
            auto* Path=UNavigationSystemV1::FindPathToLocationSynchronously(W,FVector(-500,4400,10),FVector(500,4400,10));
            if(!Test->TestTrue(TEXT("Real nav path detours around obstacle"),Path && Path->IsValid() && !Path->IsPartial() && Path->PathPoints.Num()>2)){Advance(99);return false;}
            auto* P=APFCreatureSpawner::Spawn(W,TEXT("Creature_Forager"),FVector(-1000,4500,50));auto* H=APFCreatureSpawner::Spawn(W,TEXT("Creature_Prowler"),FVector(-800,4200,50));
            if(!Test->TestNotNull(TEXT("Navigation-valid passive spawn"),P) || !Test->TestNotNull(TEXT("Navigation-valid hostile spawn"),H)){Advance(99);return false;}
            PassiveStart=P->GetActorLocation();HostileStart=H->GetActorLocation();Advance(1);return false;
        }
        APFCreature* Passive=nullptr;APFCreature* Hostile=nullptr;
        for(TActorIterator<APFCreature> It(W);It;++It){if(It->DefinitionId==TEXT("Creature_Forager")){Passive=*It;}if(It->DefinitionId==TEXT("Creature_Prowler")){Hostile=*It;}}
        if(!Passive || !Hostile){return false;}
        if(bClient && Stage==0)
        {
            if(Hostile->Health<=0 || !Passive->State.IsValid() || Hostile->GetActorLocation().X>-500){return false;}
            // The server may have moved before the client's test runner starts.
            // Compare against the known fixture, not a late replicated snapshot.
            PassiveStart=FVector(-1000,4500,52);HostileStart=FVector(-800,4200,52);
            Test->TestEqual(TEXT("Direct client damage rejected"),Hostile->TakeDamage(99,FDamageEvent(),PC,PC->GetPawn()),0.f);
            Test->TestNull(TEXT("Direct client spawning rejected"),APFCreatureSpawner::Spawn(W,TEXT("Creature_Forager"),FVector(-500,4000,0)));
            Test->TestTrue(TEXT("Client developer spawn denied"),PF::AgentTools::ExecuteCommand(TEXT("PF.SpawnCreature"),{TEXT("Creature_Forager")},W,false).Issues.ContainsByPredicate([](const auto& E){return E.Code==TEXT("NotAuthority");}));Advance(1);
        }
        bPassiveMoved|=FVector::Dist2D(PassiveStart,Passive->GetActorLocation())>50;
        bHostileMoved|=FVector::Dist2D(HostileStart,Hostile->GetActorLocation())>50;
        if(bClient)
        {
            if(!Hostile->IsDead() && V->GetVitals().Health<100 && Now-LastRequest>0.65)
            {
                FVector Eye;FRotator Look;PC->GetPawn()->GetActorEyesViewPoint(Eye,Look);PC->SetControlRotation((Hostile->GetActorLocation()-Eye).Rotation());
                if(Now-Changed>0.8){PC->ServerAttackCreature();LastRequest=Now;}
            }
            if(Hostile->IsDead())
            {
                if(V->GetVitals().Health<100 && PC->GetInventory()->Count(TEXT("Item_Food"))==0)
                {
                    for(TActorIterator<APFItemPickup> It(W);It;++It){if(It->GetContents().ItemId==TEXT("Item_Food") && FVector::Dist(It->GetActorLocation(),PC->GetPawn()->GetActorLocation())<250){FVector Eye;FRotator Look;PC->GetPawn()->GetActorEyesViewPoint(Eye,Look);PC->SetControlRotation((It->GetActorLocation()-Eye).Rotation());if(Now-LastRequest>0.8){PC->Interact();LastRequest=Now;}}}
                    return false;
                }
                Test->TestTrue(TEXT("Replicated passive movement"),bPassiveMoved);Test->TestTrue(TEXT("Replicated hostile movement"),bHostileMoved);
                Test->TestEqual(TEXT("Replicated death tag"),Hostile->State.ToString(),FString(TEXT("Creature.State.Dead")));
                if(V->GetVitals().Health<100){Test->TestEqual(TEXT("Loot recovered through interaction RPC"),PC->GetInventory()->Count(TEXT("Item_Food")),3);}
                PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"),{TEXT("M6LiveClient")},W);Advance(99);
            }
        }
        else
        {
            bool LootRecovered=false;bool SurvivorDamaged=false;
            for(auto* Player:Players){LootRecovered|=Player->GetInventory()->Count(TEXT("Item_Food"))==3;SurvivorDamaged|=Player->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->GetVitals().Health<100;}
            if(Hostile->IsDead() && LootRecovered)
            {
                Test->TestTrue(TEXT("Passive physically fled"),bPassiveMoved);Test->TestTrue(TEXT("Hostile physically chased"),bHostileMoved);Test->TestTrue(TEXT("AI dealt server damage"),SurvivorDamaged);
                Test->TestFalse(TEXT("Runtime creature integrity"),PF::AgentTools::ExecuteCommand(TEXT("PF.TestCreatureAI"),{},W).HasErrors());
                PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"),{TEXT("M6LiveServer")},W);Advance(99);
            }
        }
        return false;
    }
private:
    FAutomationTestBase* Test;double Started,Changed=0,LastRequest=0;int32 Stage=0,Expected=1;bool bClient=false,bPassiveMoved=false,bHostileMoved=false;FVector PassiveStart,HostileStart;
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCreatureLiveTest,"PF.Creatures.Live",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFCreatureLiveTest::RunTest(const FString&)
{if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunCreatureLiveTests"))){AddError(TEXT("Requires isolated L_M6Creatures -PFRunCreatureLiveTests -PFExpectedPlayers=1|2"));return false;}ADD_LATENT_AUTOMATION_COMMAND(FPFCreatureLiveExercise(this));return true;}
#endif
