#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Creatures/PFCreature.h"
#include "Creatures/PFCreatureSpawner.h"
#include "World/PFWorldClock.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFSaveFileStore.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

class FPFAuthoredEcologyExercise : public IAutomationLatentCommand
{
public:
    explicit FPFAuthoredEcologyExercise(FAutomationTestBase* In):Test(In),Started(FPlatformTime::Seconds())
    {FParse::Value(FCommandLine::Get(),TEXT("PFExpectedPlayers="),Expected);FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot);Restart=FParse::Param(FCommandLine::Get(),TEXT("PFLoadSave"));}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();
        if(Done){return Now-Done>(Client?4:20);}
        if(Now-Started>120){Test->AddError(FString::Printf(TEXT("[PrimalAgentTools] Authored ecology timeout stage=%d"),Stage));return true;}
        UWorld* W=nullptr;for(const auto& C:GEngine->GetWorldContexts()){if(C.World() && C.World()->IsGameWorld()){W=C.World();break;}}
        if(!W){return false;}Client=W->GetNetMode()==NM_Client;
        TArray<APFSurvivalPlayerController*> Players;
        for(auto It=W->GetPlayerControllerIterator();It;++It){auto* P=Cast<APFSurvivalPlayerController>(It->Get());if(P && P->GetPawn() && P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()){Players.Add(P);}}
        if(Players.Num()!=(Client?1:Expected)){return false;}
        APFWorldClock* Clock=nullptr;for(TActorIterator<APFWorldClock> It(W);It;++It){if(Clock){Test->AddError(TEXT("Multiple clocks"));return true;}Clock=*It;}
        if(!Clock){return false;}auto* Persistence=W->GetSubsystem<UPFWorldPersistence>();
        auto* V=Players[0]->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
        if(Client)
        {
            if(V->GetVitals().Health!=96 || APFCreatureSpawner::Count(W)!=2 || Clock->Hour>=18){return false;}
            FString Error;Test->TestFalse(TEXT("Client cannot save authored world"),Persistence->Save(Slot,Error));
            Test->TestNull(TEXT("Client cannot spawn authored creature"),APFCreatureSpawner::Spawn(W,TEXT("Creature_Forager"),FVector::ZeroVector));
            Test->TestTrue(TEXT("Reconnect outcome matches phase"),Players[0]->GetPlayerSetupStatus()==(Restart?EPFPlayerSetupStatus::Restored:EPFPlayerSetupStatus::NewSurvivor));
            int32 Forager=0,Prowler=0;for(TActorIterator<APFCreature> It(W);It;++It){Forager+=!It->bHostile;Prowler+=It->bHostile;Test->TestTrue(TEXT("Replicated resident is living"),It->Health>0);Test->TestTrue(TEXT("Replicated ID is an authored group member"),It->bHostile?(It->DefinitionId==TEXT("Creature_Prowler") || It->DefinitionId==TEXT("Creature_DuskPouncer")):(It->DefinitionId==TEXT("Creature_Forager") || It->DefinitionId==TEXT("Creature_Scrabbler")));}
            Test->TestTrue(TEXT("Exactly one replicated resident of each type"),Forager==1 && Prowler==1);
            Test->AddInfo(TEXT("[PrimalAgentTools] Authored ecology client replicated dawn residents, reconnect and authority refusal verified"));Done=Now;return false;
        }
        TArray<APFCreatureSpawner*> Points;for(TActorIterator<APFCreatureSpawner> It(W);It;++It){Points.Add(*It);}
        if(Points.Num()!=2){Test->AddError(TEXT("Authored map requires exactly two points"));return true;}
        Points.Sort([](const auto& A,const auto& B){return A.GetName()<B.GetName();});
        if(Stage==0)
        {
            for(int32 I=0;I<2;++I)
            {
                auto* P=Points[I];Test->TestEqual(TEXT("Stable authored point name"),P->GetName(),FString::Printf(TEXT("PFCreatureSpawner_%d"),I));
                Test->TestTrue(TEXT("Authored policy configured without fixture assignment"),P->bUseSpawnPolicy && P->SpawnPolicies && P->SpawnPolicies->GetPathName()==TEXT("/Game/PrimalFrontier/Creatures/DA_OpenWorldSpawnPolicies.DA_OpenWorldSpawnPolicies"));
                Test->TestEqual(TEXT("Authored biome"),P->Biome.ToString(),FString(I==0?TEXT("Ecology.Biome.Shore"):TEXT("Ecology.Biome.Ridge")));
            }
            for(auto* P:Players){auto* Stats=P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();Stats->HungerDrainPerSecond=0;Stats->ThirstDrainPerSecond=0;}
            Clock->DayLengthSeconds=86400;
            if(Restart){Stage=3;Changed=Now;return false;}
            if(!IsValid(Points[0]->Resident)){return false;}
            Test->TestNull(TEXT("Authored daytime ridge dormant"),Points[1]->Resident.Get());Test->TestEqual(TEXT("Fresh daytime resident count"),APFCreatureSpawner::Count(W),1);
            Test->TestTrue(TEXT("Server moves clock to night"),Clock->SetHour(21));Stage=1;Changed=Now;return false;
        }
        if(Stage==1)
        {
            if(!IsValid(Points[1]->Resident)){return false;}
            Test->TestEqual(TEXT("Night residents bounded to two"),APFCreatureSpawner::Count(W),2);
            Test->TestTrue(TEXT("Dawn set on server"),Clock->SetHour(9));Stage=2;Changed=Now;return false;
        }
        if(Stage==2 && Now-Changed<3){return false;}
        if(Stage==3 && Now-Changed<3){return false;}
        FString Error;FPFWorldSaveData Saved;FPFSavedFile File;
        if(Restart)
        {
            if(!Test->TestTrue(TEXT("Read prior process actual save"),FPFSaveFileStore::Read(Slot,File,Error)) || !Test->TestTrue(TEXT("Decode actual prior save"),Persistence->Decode(File.Payload,Saved,Error))){return true;}
            Test->TestEqual(TEXT("Prior save has two spawner records"),Saved.Spawners.Num(),2);
            for(auto* P:Points)
            {
                const auto* R=Saved.Spawners.FindByPredicate([&](const auto& X){return X.Name==P->GetName();});
                Test->TestTrue(TEXT("New server restores original resident GUID and enabled state"),R && IsValid(P->Resident) && P->Resident->PersistentId==R->Resident && P->bAutoSpawn==R->Enabled);
                Test->TestTrue(TEXT("Valid saved/runtime respawn delay"),R && R->Respawn>=0 && R->Respawn<=30 && P->PersistenceRespawnRemaining()<=30);
            }
        }
        Test->TestEqual(TEXT("Dawn/new startup has no duplicate residents"),APFCreatureSpawner::Count(W),2);
        for(auto* P:Players){P->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>()->SetHealth(96);}
        if(!Test->TestTrue(TEXT("Server save succeeds without live reset"),Persistence->Save(Slot,Error))){Test->AddError(Error);return true;}
        Test->TestEqual(TEXT("Save leaves both residents"),APFCreatureSpawner::Count(W),2);
        Test->AddInfo(TEXT("[PrimalAgentTools] Authored ecology server day/night, stable residents and real save/restart verified"));Done=Now;return false;
    }
private:
    FAutomationTestBase* Test;double Started,Changed=0,Done=0;int32 Expected=1,Stage=0;FString Slot;bool Restart=false,Client=false;
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFAuthoredEcologyLiveTest,"PF.Creatures.AuthoredLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)
bool FPFAuthoredEcologyLiveTest::RunTest(const FString&)
{
    FString Slot;
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunAuthoredEcologyTests")) || !FParse::Value(FCommandLine::Get(),TEXT("PFSaveSlot="),Slot) || !Slot.StartsWith(TEXT("AutomationM13Authored")) || !FPFSaveFileStore::ValidSlot(Slot))
    {AddError(TEXT("Requires disposable authored ecology runner"));return false;}
    ADD_LATENT_AUTOMATION_COMMAND(FPFAuthoredEcologyExercise(this));return true;
}
#endif
