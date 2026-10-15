#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFResourceNode.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Crafting/PFCraftingComponent.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFSaveFileStore.h"
#include "Building/PFBuildPiece.h"
#include "Building/PFBuildingCatalog.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFInteraction.h"
#include "World/PFWorldClock.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "HAL/FileManager.h"
#include "Tests/AutomationCommon.h"

namespace
{
struct FCopperWorld
{
    FTestWorldWrapper Fixture;UWorld* W=nullptr;APFSurvivalPlayerController* PC=nullptr;APFSurvivorCharacter* P=nullptr;
    APFInventoryPlayerState* PS=nullptr;UPFInventoryComponent* I=nullptr;UPFCraftingComponent* C=nullptr;UPFProgressionComponent* G=nullptr;
    UPFWorldPersistence* Save=nullptr;APFWorldClock* Clock=nullptr;TArray<APFResourceNode*> Nodes;APFResourceNode* Copper=nullptr;
    bool Start()
    {
        if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}W=Fixture.GetTestWorld();
        W->GetOutermost()->Rename(*(TEXT("/Temp/PFCopper/")+FGuid::NewGuid().ToString(EGuidFormats::Digits)+TEXT("/L_PrimalFrontier_OpenWorld")),nullptr,REN_DontCreateRedirectors|REN_NonTransactional);
        W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
        auto* Floor=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Floor);Floor->SetRootComponent(Box);Box->SetBoxExtent(FVector(6000,6000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Floor->SetActorLocation(FVector(0,0,-25));
        W->SpawnActor<APlayerStart>(FVector(-250,-1100,100),FRotator::ZeroRotator);Clock=W->SpawnActor<APFWorldClock>();Clock->SetActorTickEnabled(false);
        const TCHAR* Defs[]={TEXT("Node_Wood"),TEXT("Node_Stone"),TEXT("Node_Food"),TEXT("Node_Wood"),TEXT("Node_Wood"),TEXT("Node_Stone"),TEXT("Node_Food"),TEXT("Node_Food"),TEXT("Node_Stone"),TEXT("Node_Water"),TEXT("Node_Water"),TEXT("Node_Fibre"),TEXT("Node_Fibre"),TEXT("Node_Stone"),TEXT("Node_Food")};
        const float X[]={-400,0,400,-800,-400,400,800,-700,700,-1200,1450,-1300,-1900,1850,-1950};
        const float Y[]={-600,-500,-600,1900,2300,2000,1800,4200,4600,-400,600,900,1400,1500,4450};
        for(int32 Index=0;Index<15;++Index){FActorSpawnParameters Params;Params.Name=FName(*FString::Printf(TEXT("PFResourceNode_%d"),Index));auto* Node=W->SpawnActor<APFResourceNode>(FVector(X[Index],Y[Index],60),FRotator::ZeroRotator,Params);Node->ResourceId=Defs[Index];Nodes.Add(Node);}
        FActorSpawnParameters Params;Params.Name=TEXT("CopperFixture");Copper=W->SpawnActor<APFResourceNode>(FVector(0,-850,60),FRotator::ZeroRotator,Params);Copper->ResourceId=TEXT("Node_Copper");Copper->Tags.Add(TEXT("PF.M12.CopperTest"));
        if(!Fixture.BeginPlayInTestWorld()){return false;}PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
        P=Cast<APFSurvivorCharacter>(PC->GetPawn());PS=PC->GetPlayerState<APFInventoryPlayerState>();if(!P || !PS){return false;}
        I=PS->Inventory;C=PS->Crafting;G=PS->Progression;Save=W->GetSubsystem<UPFWorldPersistence>();Save->Login(PC,TEXT(""));Save->ActiveSlot.Reset();
        P->GetCharacterMovement()->DisableMovement();P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;
        return I && C && G && Copper->IsConfigurationValid();
    }
    void Tick(float Seconds){for(int32 N=0;N<FMath::CeilToInt(Seconds*10);++N){Fixture.TickTestWorld(.1f);}}
    bool Aim(APFResourceNode* Node)
    {P->SetActorLocation(Node->GetActorLocation()+FVector(-190,0,40));FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);PC->SetControlRotation((Node->GetActorLocation()-Eye).Rotation());return PFInteraction::FindTarget(P)==Node;}
    bool Craft(FName Recipe)
    {if(!C->Start(Recipe,P)){return false;}Tick(12);return C->ActiveRecipe.IsNone() && C->Feedback.StartsWith(TEXT("Completed"));}
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCopperEarnedTest,"PF.Harvest.CopperEarned",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFCopperEarnedTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FCopperWorld F;if(!TestTrue(TEXT("Native saved-definition fixture"),F.Start())){return false;}FString Error;
    TestTrue(TEXT("Real aim at Copper"),F.Aim(F.Copper));TestFalse(TEXT("Bare access denied"),F.Copper->Gather(F.P,&Error));TestEqual(TEXT("Readable Stone threshold"),Error,FString(TEXT("Requires Stone pick")));
    TestEqual(TEXT("Denied node retains all hits"),F.Copper->HitsRemaining,6);TestEqual(TEXT("Denied Copper no XP"),F.G->GetExperience(),0);
    TestTrue(TEXT("Wood real target"),F.Aim(F.Nodes[0]));for(int32 N=0;N<3;++N){TestTrue(TEXT("Actual bare wood gather"),F.Nodes[0]->Gather(F.P));F.Tick(.6f);}
    TestTrue(TEXT("Earn Wood tool using gathered materials"),F.Craft(TEXT("Recipe_WoodTool")));TestEqual(TEXT("One real Wood tool"),F.I->Count(TEXT("Item_WoodTool")),1);
    TestTrue(TEXT("Wood tool Copper target"),F.Aim(F.Copper));TestFalse(TEXT("Wood material cannot shortcut Stone gate"),F.Copper->Gather(F.P));TestEqual(TEXT("No failed access depletion"),F.Copper->HitsRemaining,6);
    TestTrue(TEXT("Loose Stone actual target"),F.Aim(F.Nodes[1]));TestTrue(TEXT("Actual hand Stone"),F.Nodes[1]->Gather(F.P));F.Tick(.6f);
    TestTrue(TEXT("Earn existing Stone tool with real resources"),F.Craft(TEXT("Recipe_Tool")));TestEqual(TEXT("Two first crafts are not enough for free level2"),F.G->GetLevel(),1);
    TestTrue(TEXT("Stone Copper target"),F.Aim(F.Copper));TestTrue(TEXT("Owner prompt confirms compatible tool"),F.Copper->HarvestPrompt(F.P).Contains(TEXT("compatible")));
    for(int32 N=0;N<3;++N){TestTrue(TEXT("Stone gathers finite Copper"),F.Copper->Gather(F.P));F.Tick(.6f);}
    TestTrue(TEXT("Finite twelve ore, no duplicated depleted yield"),F.I->Count(TEXT("Item_CopperOre"))==12 && F.Copper->HitsRemaining==0 && !F.Copper->Gather(F.P));
    F.Tick(21);TestTrue(TEXT("Regrown wood target"),F.Aim(F.Nodes[0]));for(int32 N=0;N<2;++N){TestTrue(TEXT("Owned Stone gathers remaining wood"),F.Nodes[0]->Gather(F.P));F.Tick(.6f);}
    const int32 Ore=F.I->Count(TEXT("Item_CopperOre")),Wood=F.I->Count(TEXT("Item_Wood")),XP=F.G->GetExperience();
    TestTrue(TEXT("Start refinement then cancel"),F.C->Start(TEXT("Recipe_CopperRefine"),F.P));TestTrue(TEXT("Cancel no output"),F.C->Cancel());F.Tick(1);
    TestTrue(TEXT("Cancel conserves ingredients and rewards"),F.I->Count(TEXT("Item_CopperOre"))==Ore && F.I->Count(TEXT("Item_Wood"))==Wood && F.I->Count(TEXT("Item_CopperIngot"))==0 && F.G->GetExperience()==XP);
    for(int32 N=0;N<3;++N){TestTrue(TEXT("Refine actual Copper and fuel"),F.Craft(TEXT("Recipe_CopperRefine")));}
    TestTrue(TEXT("Finite refinement conservation"),F.I->Count(TEXT("Item_CopperIngot"))==3 && F.I->Count(TEXT("Item_CopperOre"))==6);
    TestTrue(TEXT("Fibre target"),F.Aim(F.Nodes[11]));TestTrue(TEXT("Actual fibre"),F.Nodes[11]->Gather(F.P));F.Tick(.6f);TestTrue(TEXT("Earn cord"),F.Craft(TEXT("Recipe_Cord")));
    TestFalse(TEXT("Copper pick requires actual learned FieldTools"),F.C->Start(TEXT("Recipe_CopperPick"),F.P));TestEqual(TEXT("Refusal keeps ingots"),F.I->Count(TEXT("Item_CopperIngot")),3);
    TestEqual(TEXT("Real gathering/refining/cord route earns level2"),F.G->GetLevel(),2);
    const int32 StatPoints=F.G->GetAvailableStatPoints();TestTrue(TEXT("Earned separate technology purchase"),F.G->RequestKnowledge(TEXT("Tech_FieldTools"),F.P,Error));TestEqual(TEXT("Tech does not spend stat points"),F.G->GetAvailableStatPoints(),StatPoints);
    TestTrue(TEXT("Earn Copper pick"),F.Craft(TEXT("Recipe_CopperPick")));TestTrue(TEXT("One output, exact inputs"),F.I->Count(TEXT("Item_CopperPick"))==1 && F.I->Count(TEXT("Item_CopperIngot"))==0 && F.I->Count(TEXT("Item_Cord"))==0);
    TestTrue(TEXT("Copper is Pick only, four hits"),F.I->Definition(TEXT("Item_CopperPick"))->HarvestFamilies.Num()==1 && F.I->GatheringHits()==4);
    TestTrue(TEXT("Copper regrown target"),F.Aim(F.Copper));TestTrue(TEXT("Copper first action four hits"),F.Copper->Gather(F.P));TestEqual(TEXT("Compatible speed changes, total yield bounded"),F.Copper->HitsRemaining,2);
    F.Fixture.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalAgentTools] CopperEarned: real traced wood/stone/ore/fibre, normal timed crafts, no grants, Wood refusal/Stone access/Copper speed, finite yields, cancelled refinement, actual knowledge and separate stat budget verified. Native pacing only."));return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCopperMigrationTest,"PF.Persistence.CopperLayout",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFCopperMigrationTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FCopperWorld F;if(!F.Start()){return false;}FString Error;const FString Slot=TEXT("AutomationCopper")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ON_SCOPE_EXIT{F.Save->ActiveSlot.Reset();for(bool B:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot,B));}IFileManager::Get().Delete(*FPFSaveFileStore::OriginalPath(Slot,TEXT("M12CopperLayout1")));};
    FPFProgressionRecord Progress;Progress.Experience=100;Progress.Stats.Stamina=1;TestTrue(TEXT("Explicit upgraded-owner boundary seed"),F.G->Restore(Progress,Error));
    F.P->Survival->StaminaRecoveryPerSecond=0;F.P->Survival->ChangeStamina(12);F.P->Survival->SetHealth(33);F.P->Survival->SetHunger(47);F.P->Survival->SetThirst(27);
    TestTrue(TEXT("Finite source food fixture"),F.I->Grant(TEXT("Item_Food"),2));TestTrue(TEXT("Source depletion"),F.Nodes[0]->RestorePersistence(0,12));F.Clock->SetHour(21);
    auto* Buildings=NewObject<UPFBuildingCatalog>();auto Piece=[&](FName Id,FVector At,APFBuildPiece* Parent){const FTransform T(At);auto* A=F.W->SpawnActorDeferred<APFBuildPiece>(APFBuildPiece::StaticClass(),T);A->Initialize(*Buildings->Find(Id),F.PC->PlayerState,Parent);A->FinishSpawning(T);return A;};
    auto* Base=Piece(TEXT("Build_Foundation"),FVector(2500,-1100,10),nullptr);auto* Chest=Piece(TEXT("Build_Storage"),FVector(2500,-1100,60),Base);
    Chest->bRestrictStorageItems=true;Chest->AllowedStorageItems={FName(TEXT("Item_Wood"))};TestTrue(TEXT("Source storage"),Chest->Storage->Grant(TEXT("Item_Wood"),7));
    const FGuid Owner=Chest->PersistentOwnerId,ChestId=Chest->PersistentId,FoodBatch=F.I->GetStacks()[0].StackId;
    FPFWorldSaveData Old;if(!TestTrue(TEXT("Capture actual current world"),F.Save->Capture(Old,Error))){AddError(Error);return false;}
    Old.Resources.RemoveAll([](const auto& R){return R.Definition==TEXT("Node_Copper");});TArray<uint8> Bytes;
    TestTrue(TEXT("Synthetic previous15-layout whole data valid"),F.Save->Validate(Old,Error));F.Save->Encode(Old,Bytes,Error);TestTrue(TEXT("Write unique previous-layout source"),FPFSaveFileStore::Write(Slot,Bytes,Error));
    F.Copper->HitsRemaining=1;F.P->Survival->SetHealth(99);
    FPFSavedFile BeforeInspection,AfterInspection;FPFSaveInspection Inspection;
    TestTrue(TEXT("Read prior layout before inspection"),FPFSaveFileStore::Read(Slot,BeforeInspection,Error));
    for(int32 Repeat=0;Repeat<2;++Repeat)
    {
        TestTrue(TEXT("Read-only inspection accepts reviewed prior layout"),F.Save->InspectSave(Slot,Inspection,Error));
        TestTrue(TEXT("Inspection explicitly requires Copper migration with prepared count"),Inspection.bRequiresCopperMigration && Inspection.Resources==16 && !Inspection.bRecoveredBackup);
        TestTrue(TEXT("Inspection leaves live old/current state and actors intact"),F.Copper->HitsRemaining==1 && F.P->Survival->GetVitals().Health==99 && IsValid(Chest) && Chest->PersistentId==ChestId && Chest->Storage->Count(TEXT("Item_Wood"))==7 && F.Save->ActiveSlot.IsEmpty());
        TestTrue(TEXT("Inspection does not create migration archive"),IFileManager::Get().FileSize(*FPFSaveFileStore::OriginalPath(Slot,TEXT("M12CopperLayout1")))<0);
        TestTrue(TEXT("Inspection preserves source payload generation and UTC"),FPFSaveFileStore::Read(Slot,AfterInspection,Error) && AfterInspection.Payload==BeforeInspection.Payload && AfterInspection.Generation==BeforeInspection.Generation && AfterInspection.SavedUtc==BeforeInspection.SavedUtc);
    }
    if(!TestTrue(TEXT("Real server Load applies approved15->16 edge"),F.Save->Load(Slot,Error))){AddError(Error);return false;}
    TestTrue(TEXT("Old depleted state and new default retained"),F.Nodes[0]->HitsRemaining==0 && F.Copper->HitsRemaining==6 && F.Clock->Hour==21);
    TestTrue(TEXT("Upgraded vitals and food batch preserved"),F.P->Survival->GetVitals().Health==33 && F.P->Survival->GetVitals().Stamina==112 && F.P->Survival->GetVitals().MaxStamina==120 && F.I->Count(TEXT("Item_Food"))==2 && F.I->GetStacks()[0].StackId==FoodBatch);
    APFBuildPiece* LoadedChest=nullptr;for(TActorIterator<APFBuildPiece> It(F.W);It;++It){if(It->PersistentId==ChestId){LoadedChest=*It;}}
    TestTrue(TEXT("Storage/filter/support/ownership conserved"),LoadedChest && LoadedChest->PersistentOwnerId==Owner && LoadedChest->Support && LoadedChest->bRestrictStorageItems && LoadedChest->AllowedStorageItems==TArray<FName>{FName(TEXT("Item_Wood"))} && LoadedChest->Storage->Count(TEXT("Item_Wood"))==7);
    FPFSavedFile Original,Source;TestTrue(TEXT("Verified original exact bytes"),FPFSaveFileStore::ReadOriginal(Slot,TEXT("M12CopperLayout1"),Original,Error) && Original.Payload==Bytes);
    TestTrue(TEXT("Load preserves original source and time anchor"),FPFSaveFileStore::Read(Slot,Source,Error) && Source.Payload==Bytes && Source.SavedUtc==Original.SavedUtc);
    TestTrue(TEXT("Ordinary save captures expanded aged runtime"),F.Save->Save(Slot,Error));F.Copper->HitsRemaining=2;
    TestTrue(TEXT("Inspection distinguishes current layout without applying it"),F.Save->InspectSave(Slot,Inspection,Error) && !Inspection.bRequiresCopperMigration && F.Copper->HitsRemaining==2);
    TestTrue(TEXT("Current-layout repeated load"),F.Save->Load(Slot,Error));TestEqual(TEXT("Current node state restored without duplicate/reset edge"),F.Copper->HitsRemaining,6);
    FPFWorldSaveData Now;TestTrue(TEXT("Exactly16 resources after reload"),F.Save->Capture(Now,Error) && Now.Resources.Num()==16 && Now.Players.Num()==1);
    TestTrue(TEXT("Original survives new current save generation"),FPFSaveFileStore::ReadOriginal(Slot,TEXT("M12CopperLayout1"),Original,Error) && Original.Payload==Bytes);
    const FString RefusedSlot=Slot+TEXT("Bad");ON_SCOPE_EXIT{for(bool B:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(RefusedSlot,B));}};
    auto Unreviewed=Old;Unreviewed.Resources[0].Name=TEXT("UnreviewedResource");TArray<uint8> InvalidLayout;
    TestTrue(TEXT("Structurally valid unknown manifest boundary source"),F.Save->Encode(Unreviewed,InvalidLayout,Error) && FPFSaveFileStore::Write(RefusedSlot,InvalidLayout,Error));
    F.Copper->HitsRemaining=2;const float HealthBefore=F.P->Survival->GetVitals().Health;
    TestFalse(TEXT("Actual Load refuses unreviewed old resource identity"),F.Save->Load(RefusedSlot,Error));
    TestTrue(TEXT("Refused edge preserves runtime and creates no invented original"),F.Copper->HitsRemaining==2 && F.P->Survival->GetVitals().Health==HealthBefore && IFileManager::Get().FileSize(*FPFSaveFileStore::OriginalPath(RefusedSlot,TEXT("M12CopperLayout1")))<0);
    TestTrue(TEXT("Refused source payload remains unchanged"),FPFSaveFileStore::Read(RefusedSlot,Source,Error) && Source.Payload==InvalidLayout);
    F.Fixture.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalAgentTools] CopperLayout: actual server15->16 Load, whole owner/vitals/stats/food/storage/filter/support/depletion/clock conservation, verified original and unchanged source UTC, normal save/reload/no duplicates. Synthetic old-layout source, not archived writer/private save."));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCopperBackupFailureTest,"PF.Persistence.CopperBackupFailure",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFCopperBackupFailureTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    AddExpectedError(TEXT("Startup Copper layout refused:"),EAutomationExpectedErrorFlags::Contains,3);
    for(bool Startup:{false,true})
    {
        for(int32 Fault=0;Fault<3;++Fault)
        {
            FCopperWorld F;if(!TestTrue(TEXT("Actual Copper world for refusal"),F.Start())){return false;}
            FString Error;const FString Slot=TEXT("AutomationCopperFail")+FGuid::NewGuid().ToString(EGuidFormats::Digits),LiveSlot=Slot+TEXT("Live");
            const FString Original=FPFSaveFileStore::OriginalPath(Slot,TEXT("M12CopperLayout1"));
            ON_SCOPE_EXIT
            {
                F.Save->ActiveSlot.Reset();
                for(const auto& OwnedSlot:{Slot,LiveSlot}){for(bool Second:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(OwnedSlot,Second));}}
                if(IFileManager::Get().DirectoryExists(*Original)){IFileManager::Get().DeleteDirectory(*Original,false,false);}
                else{IFileManager::Get().Delete(*Original);}
                IFileManager::Get().DeleteDirectory(*FPaths::GetPath(Original),false,false);
            };
            TestTrue(TEXT("Seed actual bag"),F.I->Grant(TEXT("Item_Wood"),4) && F.I->Grant(TEXT("Item_Fibre"),6));
            F.P->Survival->SetHealth(33);F.Clock->SetHour(21);
            if(!TestTrue(TEXT("Save current live slot"),F.Save->Save(LiveSlot,Error))){AddError(Error);return false;}
            FPFWorldSaveData Old;TArray<uint8> Payload;
            if(!TestTrue(TEXT("Capture real prior-layout candidate"),F.Save->Capture(Old,Error))){return false;}
            Old.Resources.RemoveAll([](const auto& R){return R.Definition==TEXT("Node_Copper");});
            if(!TestTrue(TEXT("Install valid unique old-layout file"),F.Save->Encode(Old,Payload,Error) && FPFSaveFileStore::Write(Slot,Payload,Error))){AddError(Error);return false;}
            const TArray<uint8> Corrupt={1,2,3};
            if(Fault==0)
            {
                TestTrue(TEXT("Create test original folder"),IFileManager::Get().MakeDirectory(*FPaths::GetPath(Original),true));
                TestTrue(TEXT("Install corrupt original evidence"),FFileHelper::SaveArrayToFile(Corrupt,*Original));
            }
            else if(Fault==1){TestTrue(TEXT("Original destination occupied by directory"),IFileManager::Get().MakeDirectory(*Original,true));}
            else{TestTrue(TEXT("Install corrupt sibling evidence"),FFileHelper::SaveArrayToFile(Corrupt,*FPFSaveFileStore::Path(Slot,true)));}
            TArray<uint8> SourceBefore,LiveBefore;
            TestTrue(TEXT("Snapshot source envelope"),FFileHelper::LoadFileToArray(SourceBefore,*FPFSaveFileStore::Path(Slot,false)));
            TestTrue(TEXT("Snapshot current live save envelope"),FFileHelper::LoadFileToArray(LiveBefore,*FPFSaveFileStore::Path(LiveSlot,false)));
            TestTrue(TEXT("Unsaved inventory change"),F.I->Grant(TEXT("Item_Stone"),1));F.P->Survival->SetHealth(99);F.Clock->SetHour(9);F.Copper->HitsRemaining=1;
            if(!TestTrue(TEXT("Actual active job before failure"),F.C->Start(TEXT("Recipe_Cord"),F.P))){return false;}
            const FName Recipe=F.C->ActiveRecipe;const auto Finish=F.C->FinishAt;
            TSet<AActor*> Actors;for(TActorIterator<AActor> It(F.W);It;++It){Actors.Add(*It);}
            const auto Preserved=[&]()
            {
                TestTrue(TEXT("Failure preserves unsaved vitals time resource and items"),F.P->Survival->GetVitals().Health==99 && F.Clock->Hour==9 && F.Copper->HitsRemaining==1 && F.I->Count(TEXT("Item_Stone"))==1 && F.I->Count(TEXT("Item_Wood"))==4 && F.I->Count(TEXT("Item_Fibre"))==6);
                TestTrue(TEXT("Failure does not cancel or restart active craft"),F.C->ActiveRecipe==Recipe && F.C->FinishAt==Finish);
                TSet<AActor*> After;for(TActorIterator<AActor> It(F.W);It;++It){After.Add(*It);}TestEqual(TEXT("No actor count change on failure"),After.Num(),Actors.Num());for(auto* Actor:Actors){TestTrue(TEXT("Same actual actors after failure"),After.Contains(Actor));}
                TArray<uint8> Bytes;TestTrue(TEXT("Source envelope remains byte exact"),FFileHelper::LoadFileToArray(Bytes,*FPFSaveFileStore::Path(Slot,false)) && Bytes==SourceBefore);
                TestTrue(TEXT("Other live save envelope remains byte exact"),FFileHelper::LoadFileToArray(Bytes,*FPFSaveFileStore::Path(LiveSlot,false)) && Bytes==LiveBefore);
                if(Fault==0){TestTrue(TEXT("Corrupt original not deleted or repaired"),FFileHelper::LoadFileToArray(Bytes,*Original) && Bytes==Corrupt);}
                else if(Fault==1){TestTrue(TEXT("Occupied original directory retained"),IFileManager::Get().DirectoryExists(*Original));}
                else{TestTrue(TEXT("Corrupt sibling not deleted or repaired"),FFileHelper::LoadFileToArray(Bytes,*FPFSaveFileStore::Path(Slot,true)) && Bytes==Corrupt);TestFalse(TEXT("Refused source creates no original"),IFileManager::Get().FileExists(*Original));}
            };
            FPFSaveInspection Info;TestTrue(TEXT("Inspection does not claim archive publication readiness"),F.Save->InspectSave(Slot,Info,Error) && Info.bRequiresCopperMigration && Info.bRecoveredBackup==(Fault==2));Preserved();
            if(Startup)
            {
                const FString Previous(FCommandLine::Get());
                {ON_SCOPE_EXIT{FCommandLine::Set(*Previous);};FCommandLine::Set(*(Previous+TEXT(" -PFSaveSlot=")+Slot+TEXT(" -PFLoadSave")));F.Save->ConfigureStartup();F.Save->ApplyStartup();}
                TestFalse(TEXT("Failed migration startup blocks login"),F.Save->CheckLogin(TEXT(""),Error));
                FPFWorldSaveData Output;Output.Map=TEXT("Sentinel");TestFalse(TEXT("Failed migration startup blocks capture"),F.Save->Capture(Output,Error));TestEqual(TEXT("Failed capture preserves output"),Output.Map,FString(TEXT("Sentinel")));
                TestFalse(TEXT("Failed startup cannot overwrite target with defaults"),F.Save->Save(Slot,Error));
                TestFalse(TEXT("Failed startup cannot overwrite previous live slot"),F.Save->Save(LiveSlot,Error));Preserved();
            }
            else
            {
                for(int32 Repeat=0;Repeat<2;++Repeat)
                {
                    TestFalse(TEXT("Actual migration Load refuses backup failure"),F.Save->Load(Slot,Error));
                    const TCHAR* Reason=Fault==0?TEXT("Corrupt original"):Fault==1?TEXT("directory"):TEXT("Corrupt source");TestTrue(TEXT("Refusal identifies actual backup problem"),Error.Contains(Reason));
                    TestEqual(TEXT("Rejected manual migration retains active live slot"),F.Save->ActiveSlot,LiveSlot);Preserved();
                }
            }
            F.Fixture.ForwardErrorMessages(this);
        }
    }
    AddInfo(TEXT("[PrimalAgentTools] CopperBackupFailure: corrupt original, occupied original directory and corrupt sibling exercise repeated actual Load and startup refusal; source/evidence, unsaved world/craft and manual active slot preserved, failed startup blocks login/capture/save. Synthetic files only; not crash injection or archived writer proof."));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFEryndorRuntimeTest,"PF.Eryndor.Runtime",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFEryndorRuntimeTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FCopperWorld F;if(!F.Start()){return false;}FString Error;
    // Deliberate capability boundary setup, using the real eight-slot bag and ordinary craft timers.
    F.I->Catalog=NewObject<UPFItemCatalog>(F.I);F.C->Catalog=NewObject<UPFCraftingCatalog>(F.C);
    FPFProgressionRecord R;R.Experience=700;
    if(!TestTrue(TEXT("Level5 boundary record"),F.G->Restore(R,Error))){return false;}
    for(FName Id:{FName(TEXT("Tech_FieldTools")),FName(TEXT("Tech_Metalwork")),FName(TEXT("Tech_Gemwork")),FName(TEXT("Tech_Titanium"))})
    {TestTrue(TEXT("Real server knowledge purchase"),F.G->RequestKnowledge(Id,F.P,Error));F.Tick(.6f);}
    auto Grant=[&](FName Id,int32 Count){return TestTrue(*Id.ToString(),F.I->Grant(Id,Count));};
    Grant(TEXT("Item_CopperPick"),1);Grant(TEXT("Item_IronOre"),6);Grant(TEXT("Item_Coal"),3);Grant(TEXT("Item_Flint"),3);Grant(TEXT("Item_Cord"),1);
    TestFalse(TEXT("Unowned Diamond upgrade cannot start"),F.C->Start(TEXT("Recipe_DiamondPick"),F.P));
    TestTrue(TEXT("Start real iron then cancel"),F.C->Start(TEXT("Recipe_IronRefine"),F.P));TestTrue(TEXT("Cancel"),F.C->Cancel());
    TestEqual(TEXT("Cancel consumes no ore"),F.I->Count(TEXT("Item_IronOre")),6);
    for(int32 N=0;N<3;++N){TestTrue(TEXT("Real iron refinement"),F.Craft(TEXT("Recipe_IronRefine")));}
    TestTrue(TEXT("Upgrade consumes old pick once"),F.Craft(TEXT("Recipe_IronPick")));TestEqual(TEXT("Old pick removed"),F.I->Count(TEXT("Item_CopperPick")),0);
    auto* Node=F.W->SpawnActorDeferred<APFResourceNode>(APFResourceNode::StaticClass(),FTransform(FVector(3500,-1100,60)));
    Node->ResourceId=TEXT("Node_Diamond");Node->FinishSpawning(FTransform(FVector(3500,-1100,60)));
    TestTrue(TEXT("Real first-person aim"),F.Aim(Node));TestTrue(TEXT("Iron actually gathers diamond"),Node->Gather(F.P));F.Tick(.6f);
    TestEqual(TEXT("Finite compatible five hits yields ten"),F.I->Count(TEXT("Item_Diamond")),10);
    Grant(TEXT("Item_Cord"),1);TestTrue(TEXT("Diamond upgrade starts"),F.C->Start(TEXT("Recipe_DiamondPick"),F.P));F.Tick(15);
    TestEqual(TEXT("Diamond pick output"),F.I->Count(TEXT("Item_DiamondPick")),1);TestEqual(TEXT("Iron consumed exactly once"),F.I->Count(TEXT("Item_IronPick")),0);
    Grant(TEXT("Item_TitaniumOre"),6);Grant(TEXT("Item_Coal"),6);Grant(TEXT("Item_Obsidian"),3);Grant(TEXT("Item_Cord"),1);
    for(int32 N=0;N<3;++N){TestTrue(TEXT("Titanium refine starts"),F.C->Start(TEXT("Recipe_TitaniumRefine"),F.P));F.Tick(13);TestTrue(TEXT("Titanium complete"),F.C->Feedback.StartsWith(TEXT("Completed")));}
    TestTrue(TEXT("Titanium upgrade starts"),F.C->Start(TEXT("Recipe_TitaniumPick"),F.P));F.Tick(17);
    TestTrue(TEXT("Final single upgrade, no duplication"),F.I->Count(TEXT("Item_TitaniumPick"))==1 && F.I->Count(TEXT("Item_DiamondPick"))==0 && F.I->Count(TEXT("Item_TitaniumIngot"))==0);
    int32 Hits=0;TestTrue(TEXT("Titanium resolves actual Diamond requirement"),F.I->ResolveHarvest(F.C->Catalog->Resource(TEXT("Node_Titanium"),F.I->Catalog)->HarvestRequirement,Hits,Error));TestEqual(TEXT("Seven hits"),Hits,7);
    TestFalse(TEXT("No duplicate final craft"),F.C->Start(TEXT("Recipe_TitaniumPick"),F.P));
    // The transient interaction probe is not an authored layout identity.
    Node->Destroy();
    const FString Slot=TEXT("AutomationEryndorRuntime")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ON_SCOPE_EXIT{F.Save->ActiveSlot.Reset();for(bool B:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot,B));}};
    F.P->Survival->SetHealth(44);F.P->Survival->SetHunger(55);F.P->Survival->SetThirst(66);
    auto* Buildings=NewObject<UPFBuildingCatalog>();
    auto Piece=[&](FName Id,FVector At,APFBuildPiece* Parent){const FTransform T(At);auto* A=F.W->SpawnActorDeferred<APFBuildPiece>(APFBuildPiece::StaticClass(),T);A->Initialize(*Buildings->Find(Id),F.PC->PlayerState,Parent);A->FinishSpawning(T);return A;};
    auto* Base=Piece(TEXT("Build_Foundation"),FVector(2500,-1100,10),nullptr);auto* Chest=Piece(TEXT("Build_Storage"),FVector(2500,-1100,60),Base);
    Chest->bRestrictStorageItems=true;Chest->AllowedStorageItems={TEXT("Item_Quartz"),TEXT("Item_TitaniumIngot")};
    TestTrue(TEXT("New mineral storage"),Chest->Storage->Grant(TEXT("Item_Quartz"),7));const FGuid ChestId=Chest->PersistentId,Owner=Chest->PersistentOwnerId;
    TestTrue(TEXT("Actual save with new tool/knowledge/minerals/filter"),F.Save->Save(Slot,Error));
    for(int32 N=0;N<2;++N)
    {
        TestTrue(TEXT("Unsaved tool removal"),F.I->RemoveItem(TEXT("Item_TitaniumPick"),1));F.P->Survival->SetHealth(99);
        TestTrue(TEXT("Actual world restore"),F.Save->Load(Slot,Error));
        TestTrue(TEXT("New tool/knowledge/vitals restore once"),F.I->Count(TEXT("Item_TitaniumPick"))==1 && F.G->GetRecord().Knowledge.Contains(TEXT("Tech_Titanium")) && F.P->Survival->GetVitals().Health==44 && F.P->Survival->GetVitals().Hunger==55 && F.P->Survival->GetVitals().Thirst==66);
        APFBuildPiece* Restored=nullptr;int32 Matches=0;for(TActorIterator<APFBuildPiece> It(F.W);It;++It){if(It->PersistentId==ChestId){Restored=*It;++Matches;}}
        TestTrue(TEXT("New content storage/ownership/filter restored without duplication"),Matches==1 && Restored && Restored->PersistentOwnerId==Owner && Restored->Storage->Count(TEXT("Item_Quartz"))==7 && Restored->AllowedStorageItems==TArray<FName>{TEXT("Item_Quartz"),TEXT("Item_TitaniumIngot")} && Restored->bRestrictStorageItems);
    }
    F.Fixture.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalAgentTools] Eryndor.Runtime: actual eight-slot transactions, owned knowledge, timed refine/cancel/upgrade, traced Diamond gather and consumed tool chain. Explicit level/material boundary seeds, not ordinary earned pacing."));return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFEryndorLayoutTest,"PF.Eryndor.Layout",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFEryndorLayoutTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FCopperWorld F;if(!F.Start()){return false;}FString Error;FPFWorldSaveData Old;
    F.P->Survival->SetHealth(33);TestTrue(TEXT("Existing finite food"),F.I->Grant(TEXT("Item_Food"),2));F.Nodes[0]->RestorePersistence(0,12);
    if(!TestTrue(TEXT("Capture actual sixteen-node source"),F.Save->Capture(Old,Error))){return false;}
    const TCHAR* Names[]={TEXT("Coal"),TEXT("Flint"),TEXT("Iron"),TEXT("Diamond"),TEXT("Gold"),TEXT("Silver"),TEXT("Quartz"),TEXT("Obsidian"),TEXT("Titanium")};
    TArray<APFResourceNode*> Added;
    for(int32 N=0;N<9;++N)
    {
        FActorSpawnParameters P;P.Name=FName(*(FString(TEXT("PF_Eryndor_"))+Names[N]));P.bDeferConstruction=true;
        auto* A=F.W->SpawnActor<APFResourceNode>(FVector(4000,N*400,60),FRotator::ZeroRotator,P);
        A->ResourceId=FName(*(FString(TEXT("Node_"))+Names[N]));A->Tags.Add(TEXT("PF.M12.EryndorTest"));A->FinishSpawning(FTransform(FVector(4000,N*400,60)));Added.Add(A);
    }
    for(int32 SourceCount:{16,15})
    {
        auto Source=Old;if(SourceCount==15){Source.Resources.RemoveAll([](const auto& D){return D.Definition==TEXT("Node_Copper");});}
        const FString Slot=TEXT("AutomationEryndor")+FGuid::NewGuid().ToString(EGuidFormats::Digits);TArray<uint8> Bytes;
        ON_SCOPE_EXIT{F.Save->ActiveSlot.Reset();for(bool B:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot,B));}IFileManager::Get().Delete(*FPFSaveFileStore::OriginalPath(Slot,TEXT("M12EryndorLayout1")));};
        TestTrue(TEXT("Encode/write prior layout"),F.Save->Encode(Source,Bytes,Error) && FPFSaveFileStore::Write(Slot,Bytes,Error));
        FPFSaveInspection Info;Added[0]->HitsRemaining=1;F.P->Survival->SetHealth(99);
        TestTrue(TEXT("Read-only preflight"),F.Save->InspectSave(Slot,Info,Error));TestTrue(TEXT("Correct two migration indicators"),Info.bRequiresEryndorMigration && Info.bRequiresCopperMigration==(SourceCount==15) && Info.Resources==25);
        TestTrue(TEXT("Preflight leaves live state"),Added[0]->HitsRemaining==1 && F.P->Survival->GetVitals().Health==99);
        for(int32 N=0;N<2;++N)
        {
            if(!TestTrue(TEXT("Actual reviewed Load"),F.Save->Load(Slot,Error))){AddError(Error);return false;}
            TestTrue(TEXT("Legacy state restored, new default once"),Added[0]->HitsRemaining==8 && F.Nodes[0]->HitsRemaining==0 && F.P->Survival->GetVitals().Health==33 && F.I->Count(TEXT("Item_Food"))==2);
            FPFSavedFile Original,Current;TestTrue(TEXT("Original/source payload and time anchor conserved"),FPFSaveFileStore::ReadOriginal(Slot,TEXT("M12EryndorLayout1"),Original,Error) && FPFSaveFileStore::Read(Slot,Current,Error) && Original.Payload==Bytes && Current.Payload==Bytes && Original.SavedUtc==Current.SavedUtc && Original.Generation==Current.Generation);
        }
        const FString Previous(FCommandLine::Get());
        {ON_SCOPE_EXIT{FCommandLine::Set(*Previous);};FCommandLine::Set(*(Previous+TEXT(" -PFSaveSlot=")+Slot+TEXT(" -PFLoadSave")));F.Save->ConfigureStartup();F.Save->ApplyStartup();}
        TestTrue(TEXT("Actual startup applies the same reviewed edge"),Added[0]->HitsRemaining==8 && F.P->Survival->GetVitals().Health==33);
        const FString FailSlot=Slot+TEXT("Fail"),BadOriginal=FPFSaveFileStore::OriginalPath(FailSlot,TEXT("M12EryndorLayout1"));
        ON_SCOPE_EXIT{for(bool B:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(FailSlot,B));}IFileManager::Get().Delete(*BadOriginal);};
        TestTrue(TEXT("Write distinct refusal source"),FPFSaveFileStore::Write(FailSlot,Bytes,Error));
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(BadOriginal),true);const TArray<uint8> Corrupt={1,2,3};FFileHelper::SaveArrayToFile(Corrupt,*BadOriginal);
        F.P->Survival->SetHealth(99);Added[0]->HitsRemaining=1;
        TestFalse(TEXT("Eryndor original corruption refuses before apply"),F.Save->Load(FailSlot,Error));
        TestTrue(TEXT("Refusal retains actual live state and slot"),F.P->Survival->GetVitals().Health==99 && Added[0]->HitsRemaining==1 && F.Save->ActiveSlot==Slot);
        TArray<uint8> OriginalAfter;TestTrue(TEXT("Refusal does not replace corrupt evidence"),FFileHelper::LoadFileToArray(OriginalAfter,*BadOriginal) && OriginalAfter==Corrupt);
        TestTrue(TEXT("Valid source remains loadable"),F.Save->Load(Slot,Error));
        TestTrue(TEXT("Ordinary new-layout Save/Load"),F.Save->Save(Slot,Error) && F.Save->Load(Slot,Error));
        TestTrue(TEXT("Current layout needs no migration"),F.Save->InspectSave(Slot,Info,Error) && !Info.bRequiresEryndorMigration && !Info.bRequiresCopperMigration);
        FPFWorldSaveData Now;TestTrue(TEXT("Exactly25 nodes no duplicated identities"),F.Save->Capture(Now,Error) && Now.Resources.Num()==25);
        Added.Last()->ResourceId=TEXT("Node_Wood");TestFalse(TEXT("Unknown authored definition refuses"),F.Save->Load(Slot,Error));Added.Last()->ResourceId=TEXT("Node_Titanium");
    }
    F.Fixture.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalAgentTools] Eryndor.Layout: exact15/16->25 manifests, read-only preflight, original envelope/UTC, repeated Load/current Save and unknown-definition refusal verified."));return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFEryndorHarvestMatrixTest,"PF.Eryndor.HarvestMatrix",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFEryndorHarvestMatrixTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FCopperWorld F;if(!F.Start()){return false;}FString Error;
    F.I->Catalog=NewObject<UPFItemCatalog>(F.I);F.C->Catalog=NewObject<UPFCraftingCatalog>(F.C);
    const TCHAR* Names[]={TEXT("Coal"),TEXT("Flint"),TEXT("Iron"),TEXT("Diamond"),TEXT("Gold"),TEXT("Silver"),TEXT("Quartz"),TEXT("Obsidian"),TEXT("Titanium")};
    TestTrue(TEXT("Real carried top mechanical tool"),F.I->Grant(TEXT("Item_TitaniumPick"),1));
    for(int32 N=0;N<9;++N)
    {
        const FTransform At(FVector(3500,N*400,60));auto* Node=F.W->SpawnActorDeferred<APFResourceNode>(APFResourceNode::StaticClass(),At);
        Node->ResourceId=FName(*(FString(TEXT("Node_"))+Names[N]));Node->FinishSpawning(At);
        const auto* D=F.C->Catalog->Resource(Node->ResourceId,F.I->Catalog);
        if(!TestNotNull(TEXT("Actual saved definition"),D)){return false;}
        TestTrue(TEXT("Actual first-person trace for each mineral"),F.Aim(Node));
        TestTrue(TEXT("Compatible owned gather for each mineral"),Node->Gather(F.P,&Error));F.Tick(.6f);
        TestTrue(TEXT("Seven hits exactly fourteen yield; one hit left"),Node->HitsRemaining==1 && F.I->Count(D->YieldItem)==14);
        TestTrue(TEXT("Recover bag capacity without deleting other item types"),F.I->RemoveItem(D->YieldItem,14));
        TestTrue(TEXT("Second finite action"),Node->Gather(F.P,&Error));F.Tick(.6f);
        TestTrue(TEXT("Remaining one hit yields two"),Node->HitsRemaining==0 && F.I->Count(D->YieldItem)==2);
        const int32 XP=F.G->GetExperience();TestFalse(TEXT("Depleted node cannot duplicate"),Node->Gather(F.P,&Error));
        TestEqual(TEXT("Failed request yields no XP"),F.G->GetExperience(),XP);
        TestTrue(TEXT("Remove actual two yields"),F.I->RemoveItem(D->YieldItem,2));
        TestTrue(TEXT("Remove actual tool"),F.I->RemoveItem(TEXT("Item_TitaniumPick"),1));
        TestTrue(TEXT("Reset finite fixture before denial"),Node->RestorePersistence(D->Hits,0));
        TestFalse(TEXT("No cached capability after removal"),Node->Gather(F.P,&Error));TestEqual(TEXT("Denied mineral unchanged"),Node->HitsRemaining,D->Hits);
        TestTrue(TEXT("Reacquire actual capability"),F.I->Grant(TEXT("Item_TitaniumPick"),1));F.Tick(.6f);
    }
    TestEqual(TEXT("All nine ores share one bounded five-action XP window"),F.G->GetExperience(),25);
    F.Fixture.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalAgentTools] Eryndor.HarvestMatrix: all9 actual traced mineral nodes; exact finite yield/depletion, tool removal refusal and shared25XP cap; explicit carried-tool seed, no earned-tier claim."));return true;
}
#endif
