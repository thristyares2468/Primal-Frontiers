#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFSaveFileStore.h"
#include "Crafting/PFCraftingComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "World/PFWorldClock.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "HAL/FileManager.h"
#include "Tests/AutomationCommon.h"
namespace
{
bool V1World(FTestWorldWrapper& F,APFSurvivalPlayerController*& PC,APFWorldClock*& Clock)
{
    if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFV1Original/")+FGuid::NewGuid().ToString(EGuidFormats::Digits)+TEXT("/L_Automation")),nullptr,REN_DontCreateRedirectors|REN_NonTransactional);
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Floor=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Floor);Floor->SetRootComponent(Box);Box->SetBoxExtent(FVector(3000,3000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Floor->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APlayerStart>(FVector(0,0,120),FRotator::ZeroRotator);Clock=W->SpawnActor<APFWorldClock>();Clock->SetActorTickEnabled(false);if(!F.BeginPlayInTestWorld()){return false;}
    PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);auto* P=W->GetSubsystem<UPFWorldPersistence>();P->Login(PC,TEXT(""));return P->RestorePlayer(PC) && PC->GetPawn() && PC->GetInventory() && PC->GetCrafting();
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFV1OriginalRefusal,"PF.Persistence.V1OriginalRefusal",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFV1OriginalRefusal::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    AddExpectedError(TEXT("Startup restoration preparation refused:"),EAutomationExpectedErrorFlags::Contains,3);
    for(bool Startup:{false,true}){for(int32 Fault=0;Fault<3;++Fault)
    {
        FTestWorldWrapper F;APFSurvivalPlayerController* PC=nullptr;APFWorldClock* Clock=nullptr;if(!V1World(F,PC,Clock)){AddError(TEXT("V1 native fixture failed"));return false;}
        auto* W=F.GetTestWorld();auto* P=W->GetSubsystem<UPFWorldPersistence>();auto* I=PC->GetInventory();auto* C=PC->GetCrafting();auto* Needs=PC->GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
        const FString Slot=TEXT("AutomationV1Original")+FGuid::NewGuid().ToString(EGuidFormats::Digits),Live=Slot+TEXT("Live"),Original=FPFSaveFileStore::OriginalPath(Slot,TEXT("WorldProgressionV1"));FString Error;
        ON_SCOPE_EXIT{P->ActiveSlot.Reset();for(const auto& S:{Slot,Live}){for(bool B:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(S,B));}}if(IFileManager::Get().DirectoryExists(*Original)){IFileManager::Get().DeleteDirectory(*Original,false,false);}else{IFileManager::Get().Delete(*Original);}IFileManager::Get().DeleteDirectory(*FPaths::GetPath(Original),false,false);};
        if(!TestTrue(TEXT("Seed real bag and current live save"),I->Grant(TEXT("Item_Wood"),4) && I->Grant(TEXT("Item_Fibre"),6) && P->Save(Live,Error))){return false;}
        Needs->SetHealth(33);Clock->SetHour(21);FPFWorldSaveData Old;TArray<uint8> Payload;
        if(!P->Capture(Old,Error)){AddError(Error);return false;}Old.Version=1;for(auto& Player:Old.Players){Player.Progression.Reset();}
        if(!TestTrue(TEXT("Actual compatible V1 source"),P->Encode(Old,Payload,Error) && FPFSaveFileStore::Write(Slot,Payload,Error))){AddError(Error);return false;}
        const TArray<uint8> Corrupt={1,2,3};
        if(Fault==0){IFileManager::Get().MakeDirectory(*FPaths::GetPath(Original),true);if(!FFileHelper::SaveArrayToFile(Corrupt,*Original)){return false;}}
        else if(Fault==1){if(!IFileManager::Get().MakeDirectory(*Original,true)){return false;}}
        else if(!FFileHelper::SaveArrayToFile(Corrupt,*FPFSaveFileStore::Path(Slot,true))){return false;}
        TArray<uint8> SourceBefore,LiveBefore;FFileHelper::LoadFileToArray(SourceBefore,*FPFSaveFileStore::Path(Slot,false));FFileHelper::LoadFileToArray(LiveBefore,*FPFSaveFileStore::Path(Live,false));
        Needs->SetHealth(99);Clock->SetHour(9);I->Grant(TEXT("Item_Stone"),1);if(!TestTrue(TEXT("Actual job active before refusal"),C->Start(TEXT("Recipe_Cord"),PC->GetPawn()))){return false;}
        const double Finish=C->FinishAt;const FGuid Owner=PC->GetPlayerState<APFInventoryPlayerState>()->PersistentPlayerId;
        TSet<AActor*> Actors;for(TActorIterator<AActor> It(W);It;++It){Actors.Add(*It);}
        const auto Preserved=[&]()
        {
            TestTrue(TEXT("Refusal preserves unsaved vitals clock inventory identity and pending job"),Needs->GetVitals().Health==99 && Clock->Hour==9 && I->Count(TEXT("Item_Wood"))==4 && I->Count(TEXT("Item_Fibre"))==6 && I->Count(TEXT("Item_Stone"))==1 && PC->GetPlayerState<APFInventoryPlayerState>()->PersistentPlayerId==Owner && C->ActiveRecipe==TEXT("Recipe_Cord") && C->FinishAt==Finish);
            TSet<AActor*> AfterActors;for(TActorIterator<AActor> It(W);It;++It){AfterActors.Add(*It);}TestTrue(TEXT("Refusal leaves actor set unchanged"),Actors.Difference(AfterActors).IsEmpty() && AfterActors.Difference(Actors).IsEmpty());
            TArray<uint8> After;TestTrue(TEXT("V1 source bytes remain exact"),FFileHelper::LoadFileToArray(After,*FPFSaveFileStore::Path(Slot,false)) && After==SourceBefore);TestTrue(TEXT("Previous live save bytes remain exact"),FFileHelper::LoadFileToArray(After,*FPFSaveFileStore::Path(Live,false)) && After==LiveBefore);
            if(Fault==0){TestTrue(TEXT("Corrupt original evidence retained"),FFileHelper::LoadFileToArray(After,*Original) && After==Corrupt);}if(Fault==1){TestTrue(TEXT("Occupied original directory retained"),IFileManager::Get().DirectoryExists(*Original));}if(Fault==2){TestTrue(TEXT("Corrupt sibling evidence retained"),FFileHelper::LoadFileToArray(After,*FPFSaveFileStore::Path(Slot,true)) && After==Corrupt);}
        };
        if(Startup)
        {
            {const FString Before(FCommandLine::Get());ON_SCOPE_EXIT{FCommandLine::Set(*Before);};FCommandLine::Set(*(Before+TEXT(" -PFSaveSlot=")+Slot+TEXT(" -PFLoadSave")));P->ConfigureStartup();P->ApplyStartup();}
            TestFalse(TEXT("Failed V1 startup blocks login"),P->CheckLogin(TEXT(""),Error));FPFWorldSaveData Out;Out.Map=TEXT("Sentinel");TestFalse(TEXT("Failed V1 startup blocks capture"),P->Capture(Out,Error));TestEqual(TEXT("Failed capture preserves output"),Out.Map,FString(TEXT("Sentinel")));
            TestFalse(TEXT("Failed V1 startup cannot save defaults over source"),P->Save(Slot,Error));TestFalse(TEXT("Failed V1 startup cannot save defaults over previous slot"),P->Save(Live,Error));Preserved();
        }
        else{for(int32 Repeat=0;Repeat<2;++Repeat){TestFalse(TEXT("Manual V1 Load refuses unavailable original"),P->Load(Slot,Error));TestTrue(TEXT("Explicit original/source refusal reason"),Error.Contains(Fault==0?TEXT("Corrupt original"):Fault==1?TEXT("directory"):TEXT("Corrupt source")));TestEqual(TEXT("Manual refusal retains active slot"),P->ActiveSlot,Live);Preserved();}}
        F.ForwardErrorMessages(this);
    }}
    AddInfo(TEXT("[PrimalPersistence] V1OriginalRefusal: six actual manual/startup original/corrupt-sibling/directory failures preserve live bag/vitals/owner/craft/actors/source and evidence; startup blocks login/capture/save. Unique synthetic native slots only."));return true;
}
#endif
