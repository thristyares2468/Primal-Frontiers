// PFCreatureLiveTests.cpp
//
// Live network test: PF.Creatures.Live   (M6, 0c2d935)
// Runs on L_M6Creatures in a real server plus 1-2 clients:
//
//   Server stage 0: pause spawners, clear creatures, place players, disable needs
//     drain; verify a real navmesh path detours around the obstacle; spawn one
//     forager and one prowler at fixed fixture positions.
//   Client stage 0: direct damage, direct spawning and PF.SpawnCreature are all
//     refused on the client.
//   Then: both creatures must physically move (flee/chase). Once the prowler has
//     damaged the survivor, the client attacks it through ServerAttackCreature
//     until it dies, then picks up its food loot through the interaction RPC.
//   Server: once the prowler is dead and someone recovered 3 food, check movement,
//     AI damage and PF.TestCreatureAI integrity; export a report.
//
// Launch with: -PFRunCreatureLiveTests -PFExpectedPlayers=1|2.
// History: the first client run compared against a late replicated position and
// failed; it now compares against the known fixture spawn points.

#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Inventory/PFItemPickup.h"
#include "Persistence/PFWorldPersistence.h"
#include "World/PFWorldClock.h"
#include "PFCommands.h"
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/DamageEvents.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

class FPFEcologyPreviewExercise : public IAutomationLatentCommand
{
public:
    FPFEcologyPreviewExercise(FAutomationTestBase* InTest,FString InDirectory):Test(InTest),Directory(MoveTemp(InDirectory)),Started(FPlatformTime::Seconds()){}
    virtual bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>90){Test->AddError(TEXT("[PrimalAgentTools] Rendered ecology preview timeout"));return true;}
        UWorld* W=nullptr;
        for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld()){W=C.World();break;}}
        if(!W){return false;}
        auto* PC=Cast<APFSurvivalPlayerController>(W->GetFirstPlayerController());
        if(!PC || !PC->GetPawn() || !PC->GetInventory()){return false;}
        if(Now<Until){return false;}
        APFCreature* Forager=nullptr; APFCreature* Prowler=nullptr;
        for(TActorIterator<APFCreature> It(W);It;++It)
        {if(It->DefinitionId==TEXT("Creature_Forager")){Forager=*It;}else if(It->DefinitionId==TEXT("Creature_Prowler")){Prowler=*It;}}
        if(Stage==0)
        {
            const auto Result=PF::AgentTools::ExecuteCommand(TEXT("PF.PreviewEcology"),{},W);
            if(!Test->TestFalse(TEXT("Actual standalone preview command succeeded"),Result.HasErrors())){return true;}
            auto* Saves=W->GetSubsystem<UPFWorldPersistence>(); const FString OriginalSlot=Saves->ActiveSlot;
            Saves->ActiveSlot=TEXT("PersonalPreviewProbe");
            Test->TestTrue(TEXT("Preview refuses personal saved worlds"),PF::AgentTools::ExecuteCommand(TEXT("PF.PreviewEcology"),{},W,false).HasErrors());
            Saves->ActiveSlot=OriginalSlot;
            auto* V=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
            V->HungerDrainPerSecond=0; V->ThirstDrainPerSecond=0;
            InitialWood=PC->GetInventory()->Count(TEXT("Item_Wood"));
            Stage=1; Until=Now+2; return false;
        }
        if(Stage==1)
        {
            if(!Forager){return false;}
            Test->TestNull(TEXT("Daytime ridge has no prowler"),Prowler);
            Test->TestEqual(TEXT("One daytime shore resident"),APFCreatureSpawner::Count(W),1);
            if(!Test->TestFalse(TEXT("Night through actual developer time command"),PF::AgentTools::ExecuteCommand(TEXT("PF.SetTimeOfDay"),{TEXT("21")},W).HasErrors())){return true;}
            Stage=2; return false;
        }
        if(Stage==2)
        {
            if(!Forager || !Prowler){return false;}
            Test->TestEqual(TEXT("Both policy residents in rendered world"),APFCreatureSpawner::Count(W),2);
            OriginalResident=Prowler; Prowler->SetActorTickEnabled(false); Prowler->GetCharacterMovement()->DisableMovement();
            const FVector At=Prowler->GetActorLocation(); PC->GetPawn()->SetActorLocation(At+FVector(-400,0,80));
            FVector Eye;FRotator View;PC->GetPawn()->GetActorEyesViewPoint(Eye,View);PC->SetControlRotation((At-Eye).Rotation());
            Stage=3; Until=Now+1; return false;
        }
        if(Stage==3)
        {Shot=Directory/TEXT("night_resident.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);Stage=4;Until=Now+1;return false;}
        if(Stage==4)
        {
            Test->TestTrue(TEXT("Night screenshot written"),IFileManager::Get().FileSize(*Shot)>0);
            PF::AgentTools::ExecuteCommand(TEXT("PF.SetTimeOfDay"),{TEXT("9")},W);
            Stage=5;Until=Now+1;return false;
        }
        if(Stage==5)
        {
            Test->TestEqual(TEXT("Dawn preserves prowler rather than replacing it"),Prowler,OriginalResident.Get());
            Test->TestEqual(TEXT("Dawn retains both residents"),APFCreatureSpawner::Count(W),2);
            Shot=Directory/TEXT("day_retained_resident.png");FScreenshotRequest::RequestScreenshot(Shot,true,false);
            Stage=6;Until=Now+1;return false;
        }
        if(Stage==6)
        {
            Test->TestTrue(TEXT("Dawn screenshot written"),IFileManager::Get().FileSize(*Shot)>0);
            Test->TestFalse(TEXT("Preview reset is repeatable"),PF::AgentTools::ExecuteCommand(TEXT("PF.PreviewEcology"),{},W).HasErrors());
            Stage=7;Until=Now+2;return false;
        }
        Test->TestEqual(TEXT("Repeat preview returns to one daytime resident"),APFCreatureSpawner::Count(W),1);
        Test->TestNull(TEXT("Repeat preview returns ridge to dormant"),Prowler);
        Test->TestEqual(TEXT("Preview does not grant player wood"),PC->GetInventory()->Count(TEXT("Item_Wood")),InitialWood);
        Test->AddInfo(TEXT("[PrimalAgentTools] Rendered ecology preview, personal-save refusal, day/night/dawn residents, repeat reset and two screenshots verified"));
        return true;
    }
private:
    FAutomationTestBase* Test; FString Directory,Shot;
    double Started,Until=0;int32 Stage=0,InitialWood=0;
    TWeakObjectPtr<APFCreature> OriginalResident;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFEcologyPreviewTest,"PF.Creatures.EcologyPreviewLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFEcologyPreviewTest::RunTest(const FString&)
{
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunEcologyPreviewTest")) || Label.IsEmpty() || Label.Len()>48)
    {AddError(TEXT("Requires isolated rendered L_M7SurvivalArena -PFRunEcologyPreviewTest and bounded PFControlsEvidence label"));return false;}
    for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){AddError(TEXT("Unsafe preview evidence label"));return false;}}
    const FString Directory=FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label;
    IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FPFEcologyPreviewExercise(this,Directory));return true;
}

// Isolated real-nav ecology integration. No map edits or personal save/profile use.
class FPFEcologyLiveExercise : public IAutomationLatentCommand
{
public:
    explicit FPFEcologyLiveExercise(FAutomationTestBase* InTest):Test(InTest),Started(FPlatformTime::Seconds())
    {FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected); FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot);}
    virtual bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Now-Started>100){Test->AddError(FString::Printf(TEXT("[PrimalAgentTools] Ecology live timeout stage=%d"),Stage));return true;}
        UWorld* W=nullptr;
        for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld()){W=C.World();break;}}
        if(!W){return false;}
        if(UWorld::RemovePIEPrefix(W->GetMapName())!=TEXT("L_M7SurvivalArena"))
        {Test->AddError(TEXT("[PrimalAgentTools] Ecology fixture requires L_M7SurvivalArena"));return true;}
        const bool Client=W->GetNetMode()==NM_Client;
        if(Stage==99){return Now-Changed>(Client?2:12);}
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It)
        {auto* P=Cast<APFSurvivalPlayerController>(It->Get()); if(P && P->GetPawn() && P->GetInventory()){Players.Add(P);}}
        if(Players.Num()!=(Client?1:Expected)){return false;}
        auto Finish=[&](){Stage=99; Changed=Now;};
        FString Error;
        if(Client)
        {
            APFWorldClock* ClientClock=nullptr;
            for(TActorIterator<APFWorldClock> It(W);It;++It){ClientClock=*It;}
            APFCreature* Creature=nullptr; int32 Count=0;
            for(TActorIterator<APFCreature> It(W);It;++It){Creature=*It;++Count;}
            // 09:15 is the server's post-restore marker, distinct from legacy map startup09:00.
            if(!ClientClock || !Creature || ClientClock->Hour!=9.25f || Creature->DefinitionId!=TEXT("Creature_Prowler")){return false;}
            if(Stage==0)
            {
                // A locally created actor has an Authority role; NM_Client must still refuse.
                auto* Probe=W->SpawnActor<APFCreatureSpawner>(); Probe->bUseSpawnPolicy=true;
                Probe->SpawnPolicies=NewObject<UPFCreatureSpawnCatalog>(Probe);
                Probe->Biome=Probe->SpawnPolicies->Policies[2].Biome;
                Probe->Catalog=NewObject<UPFCreatureCatalog>(Probe);
                FName Id(TEXT("Unchanged")); float Delay=123;
                Test->TestFalse(TEXT("Real client with local Authority role cannot select ecology"),Probe->ResolveEcologyRequest(0,Id,Delay,Error));
                Test->TestTrue(TEXT("Explicit real client authority reason"),Error.Contains(TEXT("authoritative")));
                Test->TestEqual(TEXT("Client refusal preserves ID"),Id,FName(TEXT("Unchanged")));
                Test->TestEqual(TEXT("Client refusal preserves delay"),Delay,123.f);
                Test->TestFalse(TEXT("Real client cannot spawn ecology"),Probe->TrySpawnEcology(Error));
                Test->TestTrue(TEXT("Real client cannot reset ecology preview"),PF::AgentTools::ExecuteCommand(TEXT("PF.PreviewEcology"),{},W,false).HasErrors());
                Probe->Destroy();
                Stage=1; Changed=Now; return false;
            }
            if(Now-Changed<3){return false;}
            Test->TestEqual(TEXT("One restored resident replicated without duplicates"),Count,1);
            Test->TestEqual(TEXT("Existing night resident survives replicated day phase"),ClientClock->Phase.ToString(),FString(TEXT("World.Time.Day")));
            Test->TestEqual(TEXT("Resident definition and health replicated"),Creature->Health,100.f);
            Test->AddInfo(TEXT("[PrimalAgentTools] Ecology client authority, restored resident and day phase verified"));
            Finish(); return false;
        }
        if(Stage==0)
        {
            for(TActorIterator<APFCreatureSpawner> It(W);It;++It)
            {It->bAutoSpawn=false; if(It->CreatureId==TEXT("Creature_Prowler")){Point=*It;}}
            for(TActorIterator<APFCreature> It(W);It;++It){It->Destroy();}
            int32 PlayerIndex=0;
            for(auto* P:Players)
            {
                P->GetPawn()->SetActorLocation(FVector(-1200+PlayerIndex++*400,-1100,100));
                auto* Needs=P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
                Needs->HungerDrainPerSecond=0; Needs->ThirstDrainPerSecond=0;
            }
            int32 Clocks=0;
            for(TActorIterator<APFWorldClock> It(W);It;++It){Clock=*It;++Clocks;}
            if(Clocks!=1 || !Clock.IsValid() || !Point.IsValid())
            {Test->AddError(TEXT("Ecology fixture needs exactly one existing clock and a prowler spawn point"));Finish();return false;}
            Clock->SetActorTickEnabled(false); Clock->SetHour(9);
            Point->bUseSpawnPolicy=true; Point->SpawnPolicies=NewObject<UPFCreatureSpawnCatalog>(Point.Get());
            Point->Biome=Point->SpawnPolicies->Policies[2].Biome;
            Point->RestorePersistence(nullptr,0,true);
            Test->TestFalse(TEXT("Real daytime ridge refuses instead of falling back"),Point->TrySpawnEcology(Error));
            Test->TestTrue(TEXT("Dormant refusal explained"),Error.Contains(TEXT("eligible")));
            Test->TestEqual(TEXT("Dormant point did not create any creature"),APFCreatureSpawner::Count(W),0);
            Stage=1; Changed=Now; return false;
        }
        if(Stage==1)
        {
            if(Now-Changed<3){return false;}
            Clock->SetHour(21); Point->RestorePersistence(nullptr,0,true);
            if(!Test->TestTrue(TEXT("Night policy spawns through real navmesh and clearance"),Point->TrySpawnEcology(Error)))
            {Test->AddError(Error);Finish();return false;}
            auto* Resident=Point->Resident.Get(); const FGuid Identity=Resident->PersistentId;
            Resident->SetActorTickEnabled(false); Resident->GetCharacterMovement()->DisableMovement();
            Test->TestEqual(TEXT("Server night definition"),Resident->DefinitionId,FName(TEXT("Creature_Prowler")));
            Test->TestFalse(TEXT("Repeated request cannot replace resident"),Point->TrySpawnEcology(Error));
            Clock->SetHour(9);
            Test->TestEqual(TEXT("Day change retains existing resident"),Point->Resident.Get(),Resident);
            auto* Saves=W->GetSubsystem<UPFWorldPersistence>();
            if(!Test->TestTrue(TEXT("Actual ecology world save"),Saves->Save(Slot,Error)))
            {Test->AddError(Error);Finish();return false;}
            for(int32 Repeat=0;Repeat<2;++Repeat)
            {
                if(!Test->TestTrue(TEXT("Actual ecology resident load"),Saves->Load(Slot,Error)))
                {Test->AddError(Error);Finish();return false;}
                Resident=Point->Resident.Get();
                if(!Test->TestNotNull(TEXT("Restored resident assigned to original point"),Resident)){Finish();return false;}
                Resident->SetActorTickEnabled(false); Resident->GetCharacterMovement()->DisableMovement();
                Test->TestEqual(TEXT("Stable resident save identity"),Resident->PersistentId,Identity);
                Test->TestEqual(TEXT("Load never duplicates resident"),APFCreatureSpawner::Count(W),1);
                Test->TestTrue(TEXT("Original opt-in configuration stays map-owned"),Point->bUseSpawnPolicy && Point->SpawnPolicies && Point->Biome.ToString()==TEXT("Ecology.Biome.Ridge"));
                Test->TestTrue(TEXT("Restored cooldown bounded"),Point->PersistenceRespawnRemaining()>0 && Point->PersistenceRespawnRemaining()<=30);
            }
            Clock->SetHour(9.25f);
            Test->AddInfo(TEXT("[PrimalAgentTools] Ecology server real navigation, dormant/night/day resident and twice-loaded stable save identity verified"));
            Finish(); return false;
        }
        return false;
    }
private:
    FAutomationTestBase* Test;
    double Started,Changed=0;
    int32 Stage=0,Expected=1;
    FString Slot;
    TWeakObjectPtr<APFCreatureSpawner> Point;
    TWeakObjectPtr<APFWorldClock> Clock;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFEcologyLiveTest,"PF.Creatures.EcologyLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFEcologyLiveTest::RunTest(const FString&)
{
    FString Slot; int32 Players=0;
    FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot); FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Players);
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunEcologyLiveTests")) || !Slot.StartsWith(TEXT("AutomationM13Ecology")) || (Players!=1 && Players!=2))
    {AddError(TEXT("Requires isolated L_M7SurvivalArena, -PFRunEcologyLiveTests, unique AutomationM13Ecology slot and PFExpectedPlayers=1|2"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FPFEcologyLiveExercise(this)); return true;
}

class FPFCreatureLiveExercise : public IAutomationLatentCommand
{
public:
    explicit FPFCreatureLiveExercise(FAutomationTestBase* InTest,bool InVariety=false):Test(InTest),Started(FPlatformTime::Seconds()),Variety(InVariety)
    {
        FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);
        if(Variety){PassiveId=TEXT("Creature_Scrabbler");HostileId=TEXT("Creature_DuskPouncer");FoodCount=4;}
    }

    virtual bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Stage==99){return Now-Changed>(bClient?4:14);}
        if(Now-Started>120){Test->AddError(FString::Printf(TEXT("[PrimalCreatures] Live timeout stage=%d movedPassive=%d movedHostile=%d"),Stage,bPassiveMoved,bHostileMoved));return true;}

        // World and players.
        UWorld* W=nullptr;
        for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld()){W=C.World();break;}}
        if(!W){return false;}
        bClient=W->GetNetMode()==NM_Client;
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It)
        {
            auto* PC=Cast<APFSurvivalPlayerController>(It->Get());
            if(PC && PC->GetPawn() && PC->GetInventory()){Players.Add(PC);}
        }
        if(Players.Num()!=(bClient?1:Expected)){return false;}
        auto* PC=Players[0];
        auto* V=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
        if(!V){return false;}
        auto Advance=[&](int32 NewStage)
        {
            Stage=NewStage;
            Changed=Now;
            Test->AddInfo(FString::Printf(TEXT("[PrimalCreatures] %s stage=%d"),bClient?TEXT("Client"):TEXT("Server"),Stage));
        };

        // Server setup (once).
        if(!bClient && Stage==0)
        {
            for(TActorIterator<APFCreatureSpawner> It(W);It;++It){It->bAutoSpawn=false;}
            for(TActorIterator<APFCreature> It(W);It;++It){It->Destroy();}
            int32 Index=0;
            for(auto* Player:Players)
            {
                Player->GetPawn()->SetActorLocation(FVector(-1200+Index++*2400,4200,100));
                auto* Needs=Player->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
                Needs->SetHealth(100);
                Needs->HungerDrainPerSecond=0;
                Needs->ThirstDrainPerSecond=0;
            }
            auto* Path=UNavigationSystemV1::FindPathToLocationSynchronously(W,FVector(-500,4400,10),FVector(500,4400,10));
            if(!Test->TestTrue(TEXT("Real nav path detours around obstacle"),Path && Path->IsValid() && !Path->IsPartial() && Path->PathPoints.Num()>2)){Advance(99);return false;}
            auto* P=APFCreatureSpawner::Spawn(W,PassiveId,FVector(-1000,4500,50));
            auto* H=APFCreatureSpawner::Spawn(W,HostileId,FVector(-800,4200,50));
            if(!Test->TestNotNull(TEXT("Navigation-valid passive spawn"),P) || !Test->TestNotNull(TEXT("Navigation-valid hostile spawn"),H)){Advance(99);return false;}
            PassiveStart=P->GetActorLocation();
            HostileStart=H->GetActorLocation();
            Advance(1);
            return false;
        }

        // Find the two (replicated) creatures.
        APFCreature* Passive=nullptr;
        APFCreature* Hostile=nullptr;
        for(TActorIterator<APFCreature> It(W);It;++It)
        {
            if(It->DefinitionId==PassiveId){Passive=*It;}
            if(It->DefinitionId==HostileId){Hostile=*It;}
        }
        if(!Passive || !Hostile){return false;}

        // Client authority probes (once).
        if(bClient && Stage==0)
        {
            if(Hostile->Health<=0 || !Passive->State.IsValid() || Hostile->GetActorLocation().X>-500){return false;}
            // The server may have moved before the client's test runner starts.
            // Compare against the known fixture, not a late replicated snapshot.
            PassiveStart=FVector(-1000,4500,52);
            HostileStart=FVector(-800,4200,52);
            Test->TestEqual(TEXT("Direct client damage rejected"),Hostile->TakeDamage(99,FDamageEvent(),PC,PC->GetPawn()),0.f);
            Test->TestNull(TEXT("Direct client spawning rejected"),APFCreatureSpawner::Spawn(W,PassiveId,FVector(-500,4000,0)));
            Test->TestTrue(TEXT("Client developer spawn denied"),PF::AgentTools::ExecuteCommand(TEXT("PF.SpawnCreature"),{PassiveId.ToString()},W,false).Issues.ContainsByPredicate([](const auto& E){return E.Code==TEXT("NotAuthority");}));
            Advance(1);
        }

        // Movement is sticky: once moved >50 cm from the start it counts.
        bPassiveMoved|=FVector::Dist2D(PassiveStart,Passive->GetActorLocation())>50;
        bHostileMoved|=FVector::Dist2D(HostileStart,Hostile->GetActorLocation())>50;

        if(bClient)
        {
            // After taking damage, fight back through the owned attack RPC (0.65 s between requests).
            if(!Hostile->IsDead() && V->GetVitals().Health<100 && Now-LastRequest>0.65)
            {
                FVector Eye;
                FRotator Look;
                PC->GetPawn()->GetActorEyesViewPoint(Eye,Look);
                PC->SetControlRotation((Hostile->GetActorLocation()-Eye).Rotation());
                if(Now-Changed>0.8){PC->ServerAttackCreature();LastRequest=Now;}
            }
            if(Hostile->IsDead())
            {
                // This client fought: collect the prowler's food drop through Interact.
                if(V->GetVitals().Health<100 && (PC->GetInventory()->Count(TEXT("Item_Food"))==0 || (Variety && PC->GetInventory()->Count(TEXT("Item_Fibre"))==0)))
                {
                    for(TActorIterator<APFItemPickup> It(W);It;++It)
                    {
                        if(It->GetContents().ItemId==(PC->GetInventory()->Count(TEXT("Item_Food"))==0?FName(TEXT("Item_Food")):FName(TEXT("Item_Fibre"))) && FVector::Dist(It->GetActorLocation(),PC->GetPawn()->GetActorLocation())<250)
                        {
                            FVector Eye;
                            FRotator Look;
                            PC->GetPawn()->GetActorEyesViewPoint(Eye,Look);
                            PC->SetControlRotation((It->GetActorLocation()-Eye).Rotation());
                            if(Now-LastRequest>0.8){PC->Interact();LastRequest=Now;}
                        }
                    }
                    return false;
                }
                Test->TestTrue(TEXT("Replicated passive movement"),bPassiveMoved);
                Test->TestTrue(TEXT("Replicated hostile movement"),bHostileMoved);
                Test->TestEqual(TEXT("Replicated death tag"),Hostile->State.ToString(),FString(TEXT("Creature.State.Dead")));
                if(V->GetVitals().Health<100){Test->TestEqual(TEXT("Loot recovered through interaction RPC"),PC->GetInventory()->Count(TEXT("Item_Food")),FoodCount);if(Variety && V->GetVitals().Health<100){Test->TestEqual(TEXT("Supplemental loot recovered through owned interaction"),PC->GetInventory()->Count(TEXT("Item_Fibre")),2);}}
                PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"),{TEXT("M6LiveClient")},W);
                Advance(99);
            }
        }
        else
        {
            // Server: finish when the prowler is dead and some player holds its 3 food.
            bool LootRecovered=false;
            bool SurvivorDamaged=false;
            for(auto* Player:Players)
            {
                LootRecovered|=Player->GetInventory()->Count(TEXT("Item_Food"))==FoodCount && (!Variety || Player->GetInventory()->Count(TEXT("Item_Fibre"))==2);
                SurvivorDamaged|=Player->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->GetVitals().Health<100;
            }
            if(Hostile->IsDead() && LootRecovered)
            {
                Test->TestTrue(TEXT("Passive physically fled"),bPassiveMoved);
                Test->TestTrue(TEXT("Hostile physically chased"),bHostileMoved);
                Test->TestTrue(TEXT("AI dealt server damage"),SurvivorDamaged);
                Test->TestFalse(TEXT("Runtime creature integrity"),PF::AgentTools::ExecuteCommand(TEXT("PF.TestCreatureAI"),{},W).HasErrors());
                PF::AgentTools::ExecuteCommand(TEXT("PF.ExportTestReport"),{TEXT("M6LiveServer")},W);
                Advance(99);
            }
        }
        return false;
    }

private:
    FAutomationTestBase* Test;
    double Started,Changed=0,LastRequest=0;
    int32 Stage=0,Expected=1,FoodCount=3;
    bool Variety=false;
    FName PassiveId=TEXT("Creature_Forager"),HostileId=TEXT("Creature_Prowler");
    bool bClient=false,bPassiveMoved=false,bHostileMoved=false;
    FVector PassiveStart,HostileStart;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCreatureLiveTest,"PF.Creatures.Live",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFCreatureLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunCreatureLiveTests"))){AddError(TEXT("Requires isolated L_M6Creatures -PFRunCreatureLiveTests -PFExpectedPlayers=1|2"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FPFCreatureLiveExercise(this));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCreatureVarietyLiveTest,"PF.Creatures.VarietyLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFCreatureVarietyLiveTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunCreatureVarietyLiveTests"))){AddError(TEXT("Requires isolated creature variety runner"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FPFCreatureLiveExercise(this,true));return true;
}
#endif
