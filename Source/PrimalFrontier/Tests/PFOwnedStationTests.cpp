#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildPiece.h"
#include "Crafting/PFCraftingStation.h"
#include "Crafting/PFCraftingComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFSaveFileStore.h"
#include "World/PFWorldClock.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "HAL/FileManager.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFOwnedStationTest,"PF.Crafting.OwnedStations",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFOwnedStationTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}auto* W=Fixture.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFOwnedStations/")+FGuid::NewGuid().ToString(EGuidFormats::Digits)+TEXT("/L_Automation")),nullptr,REN_DontCreateRedirectors|REN_NonTransactional);
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Ground=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Ground);Ground->SetRootComponent(Box);Box->RegisterComponent();
    Box->SetBoxExtent(FVector(3000,3000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Ground->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APlayerStart>(FVector(-450,0,100),FRotator::ZeroRotator);if(!Fixture.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* Pawn=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* Bag=PC->GetInventory();auto* Craft=PC->GetCrafting();auto* B=PC->Building.Get();
    if(!Pawn || !Bag || !Craft || !B){return false;}
    Pawn->GetCharacterMovement()->DisableMovement();Pawn->Survival->HungerDrainPerSecond=0;Pawn->Survival->ThirstDrainPerSecond=0;
    B->Catalog=NewObject<UPFBuildingCatalog>(B);Bag->Catalog=NewObject<UPFItemCatalog>(Bag);Craft->Catalog=NewObject<UPFCraftingCatalog>(Craft);
    auto Tick=[&](int32 Count){for(int32 N=0;N<Count;++N){Fixture.TickTestWorld(.1f);}};
    auto Aim=[&](FVector At){FVector Eye;FRotator View;Pawn->GetActorEyesViewPoint(Eye,View);PC->SetControlRotation((At-Eye).Rotation());};
    auto FindPiece=[&](EPFBuildKind Kind){for(TActorIterator<APFBuildPiece> It(W);It;++It){if(It->Kind==Kind){return *It;}}return static_cast<APFBuildPiece*>(nullptr);};
    auto FindStation=[&](APFBuildPiece* Provider){for(TActorIterator<APFCraftingStation> It(W);It;++It){if(It->ProviderPiece==Provider){return *It;}}return static_cast<APFCraftingStation*>(nullptr);};
    TestTrue(TEXT("Placement and craft inputs"),Bag->Grant(TEXT("Item_Wood"),40) && Bag->Grant(TEXT("Item_Stone"),8));
    Aim(FVector(0,0,0));TestTrue(TEXT("Real foundation placement"),B->Place(TEXT("Build_Foundation"),0)!=nullptr);
    auto* Base=FindPiece(EPFBuildKind::Foundation);if(!TestNotNull(TEXT("Foundation"),Base)){return false;}
    Aim(Base->GetActorLocation()+FVector(-100,0,10));TestTrue(TEXT("Real owned bench placement"),B->Place(TEXT("Build_Workbench"),0)!=nullptr);
    auto* Bench=FindPiece(EPFBuildKind::Workbench);auto* Station=FindStation(Bench);
    if(!TestNotNull(TEXT("Owned station helper"),Station) || !TestNotNull(TEXT("Workbench structure"),Bench)){return false;}
    TestEqual(TEXT("Foundation and bench exact wood cost"),Bag->Count(TEXT("Item_Wood")),30);
    TestTrue(TEXT("Helper is owned, hidden and has no duplicate collision"),Station->bOwnedProvider && Station->IsHidden() && !Station->GetActorEnableCollision());
    Pawn->SetActorLocation(Bench->GetActorLocation()+FVector(-150,0,90));
    const auto* Tool=Craft->Catalog->Recipe(TEXT("Recipe_Tool"),Bag->Catalog);
    if(!TestNotNull(TEXT("Tool recipe"),Tool)){return false;}
    TestTrue(TEXT("Owner gets private bench bonus"),APFCraftingStation::FindBest(*Tool,Pawn,true)==Station);
    auto* Other=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(Other);
    if(!TestNotNull(TEXT("Second player pawn"),Other->GetPawn().Get())){return false;}
    Other->GetPawn()->SetActorLocation(Pawn->GetActorLocation());
    TestFalse(TEXT("Different owner cannot use bench"),Station->CanUse(*Tool,Other->GetPawn(),true));Other->GetPawn()->SetActorLocation(FVector(-1400,0,100));
    Bench->SetRole(ROLE_SimulatedProxy);TestFalse(TEXT("Non-authoritative provider refused"),Station->CanUse(*Tool,Pawn,true));Bench->SetRole(ROLE_Authority);
    TestTrue(TEXT("Start accepted owner craft"),Craft->Start(Tool->Id,Pawn));TestEqual(TEXT("Owned bonus snapshot"),Craft->ActiveStationMultiplier,1.25f);
    Tick(45);TestEqual(TEXT("Exactly one completed tool"),Bag->Count(TEXT("Item_Tool")),1);TestEqual(TEXT("Craft ingredients consumed once"),Bag->Count(TEXT("Item_Wood")),27);
    TestTrue(TEXT("Start second craft before demolition"),Craft->Start(Tool->Id,Pawn));
    const int32 BeforeCancel=Bag->Count(TEXT("Item_Wood"));Bench->Destroy();Tick(2);
    TestTrue(TEXT("Demolition removes helper and cancels without item loss"),(!IsValid(Station) || Station->IsActorBeingDestroyed()) && Craft->ActiveRecipe.IsNone() && Bag->Count(TEXT("Item_Wood"))==BeforeCancel);
    Pawn->SetActorLocation(FVector(-450,0,100));Aim(Base->GetActorLocation()+FVector(100,0,10));
    TestTrue(TEXT("Real refinery placement"),B->Place(TEXT("Build_Refinery"),0)!=nullptr);
    auto* Refinery=FindPiece(EPFBuildKind::Refinery);auto* Refiner=FindStation(Refinery);
    if(!TestNotNull(TEXT("Owned refinery helper"),Refiner)){return false;}
    Pawn->SetActorLocation(Refinery->GetActorLocation()+FVector(-150,0,90));
    const auto* Metal=Craft->Catalog->Recipe(TEXT("Recipe_CopperRefine"),Bag->Catalog);
    if(!TestNotNull(TEXT("Copper recipe"),Metal)){return false;}
    TestTrue(TEXT("Refinery gives owner metal bonus"),APFCraftingStation::FindBest(*Metal,Pawn,true)==Refiner && Refiner->SpeedMultiplier==1.5f);
    TestFalse(TEXT("Refinery does not accelerate tools"),Refiner->Supports(*Tool));
    TestEqual(TEXT("Refinery charges twelve wood once"),Bag->Count(TEXT("Item_Wood")),BeforeCancel-12);
    TestFalse(TEXT("Supported foundation cannot be removed"),Base->CanRemove());
    AddInfo(TEXT("[PrimalCrafting] OwnedStations: actual placement/cost/support, private ownership, authority, bonus/atomic completion and demolition cleanup/cancel verified. Stateless; no fuel/power claim."));
    Fixture.ForwardErrorMessages(this);return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFOwnedStationRestoreTest,"PF.Crafting.OwnedStationsRestore",
    EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFOwnedStationRestoreTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}auto* W=Fixture.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFOwnedStations/")+FGuid::NewGuid().ToString(EGuidFormats::Digits)+TEXT("/L_Automation")),nullptr,REN_DontCreateRedirectors|REN_NonTransactional);
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Ground=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Ground);Ground->SetRootComponent(Box);Box->RegisterComponent();
    Box->SetBoxExtent(FVector(3000,3000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Ground->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APFWorldClock>();
    W->SpawnActor<APlayerStart>(FVector(-500,0,100),FRotator::ZeroRotator);if(!Fixture.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* Persistence=W->GetSubsystem<UPFWorldPersistence>();FString Error;Persistence->Login(PC,TEXT(""));
    auto* Catalog=NewObject<UPFBuildingCatalog>();
    auto Spawn=[&](FName Id,FVector Position,APFBuildPiece* Parent)
    {
        const FTransform T(Position);auto* Piece=W->SpawnActorDeferred<APFBuildPiece>(APFBuildPiece::StaticClass(),T);
        Piece->Initialize(*Catalog->Find(Id),PC->PlayerState,Parent);Piece->FinishSpawning(T);return Piece;
    };
    auto* Base=Spawn(TEXT("Build_Foundation"),FVector(0,0,10),nullptr);
    auto* Bench=Spawn(TEXT("Build_Workbench"),FVector(-100,-100,55),Base);
    auto* Refinery=Spawn(TEXT("Build_Refinery"),FVector(100,100,55),Base);
    const FGuid BenchId=Bench->PersistentId,RefineryId=Refinery->PersistentId,Owner=Bench->PersistentOwnerId;
    const FString Slot=TEXT("AutomationOwnedRestore")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ON_SCOPE_EXIT{for(bool Second:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot,Second));IFileManager::Get().Delete(*(FPFSaveFileStore::Path(Slot,Second)+TEXT(".pending")));}};
    auto* Pawn=CastChecked<APFSurvivorCharacter>(PC->GetPawn());Pawn->GetCharacterMovement()->DisableMovement();
    Pawn->Survival->HungerDrainPerSecond=0;Pawn->Survival->ThirstDrainPerSecond=0;
    Pawn->SetActorLocation(Refinery->GetActorLocation()+FVector(-150,0,90));
    TestTrue(TEXT("Batch persistence inputs"),PC->GetInventory()->Grant(TEXT("Item_CopperOre"),8) && PC->GetInventory()->Grant(TEXT("Item_Wood"),2));
    if(!TestTrue(TEXT("Batch at owned refinery before real disk save"),PC->GetCrafting()->Start(TEXT("Recipe_CopperBatch"),Pawn))){return false;}
    for(int32 N=0;N<90;++N){Fixture.TickTestWorld(.1f);}
    TestTrue(TEXT("Owned batch completed before save"),PC->GetCrafting()->ActiveRecipe.IsNone() && PC->GetInventory()->Count(TEXT("Item_CopperIngot"))==2);
    FPFWorldSaveData Snapshot;
    if(!TestTrue(TEXT("Capture owned stations"),Persistence->Capture(Snapshot,Error)) ||
        !TestTrue(TEXT("Actual disk save accepts added definitions"),Persistence->Save(Slot,Error))){AddError(Error);return false;}
    TestEqual(TEXT("Only three structures saved; derived station helpers not duplicated"),Snapshot.Structures.Num(),3);
    for(int32 Attempt=0;Attempt<2;++Attempt)
    {
        if(!TestTrue(TEXT("Load owned station structures"),Persistence->Load(Slot,Error))){AddError(Error);return false;}
        int32 Structures=0,Helpers=0;APFBuildPiece* LoadedBench=nullptr;APFBuildPiece* LoadedRefinery=nullptr;
        for(TActorIterator<APFBuildPiece> It(W);It;++It)
        {
            ++Structures;
            if(It->PersistentId==BenchId){LoadedBench=*It;TestTrue(TEXT("Workbench ID/support/owner restored"),It->Kind==EPFBuildKind::Workbench && IsValid(It->Support) && It->PersistentOwnerId==Owner && It->IsOwnedBy(PC->PlayerState));}
            if(It->PersistentId==RefineryId){LoadedRefinery=*It;TestTrue(TEXT("Refinery ID/support/owner restored"),It->Kind==EPFBuildKind::Refinery && IsValid(It->Support) && It->PersistentOwnerId==Owner && It->IsOwnedBy(PC->PlayerState));}
        }
        for(TActorIterator<APFCraftingStation> It(W);It;++It)
        {
            if(!It->bOwnedProvider || It->IsActorBeingDestroyed()){continue;}
            ++Helpers;TestTrue(TEXT("Restored helper has live owned provider"),IsValid(It->ProviderPiece) && !It->ProviderPiece->IsActorBeingDestroyed() && It->ProviderPiece->IsOwnedBy(PC->PlayerState));
        }
        TestEqual(TEXT("Repeated load keeps exactly three structures"),Structures,3);TestEqual(TEXT("Exactly one derived helper per station"),Helpers,2);
        if(!TestNotNull(TEXT("Restored workbench"),LoadedBench)){return false;}
        PC->GetPawn()->SetActorLocation(LoadedBench->GetActorLocation()+FVector(-150,0,90));
        auto* Craft=PC->GetCrafting();const auto* Tool=Craft->Catalog->Recipe(TEXT("Recipe_Tool"),PC->GetInventory()->Catalog);
        if(!TestNotNull(TEXT("Restored tool definition"),Tool)){return false;}
        auto* Best=APFCraftingStation::FindBest(*Tool,PC->GetPawn(),true);
        TestTrue(TEXT("Saved owner can use restored bench"),Best && Best->ProviderPiece==LoadedBench);
        if(!TestNotNull(TEXT("Restored refinery for required batch"),LoadedRefinery)){return false;}
        auto* Bag=PC->GetInventory();
        TestTrue(TEXT("Repeated restore conserves batch ore/fuel/output"),Bag->Count(TEXT("Item_CopperIngot"))==2 && Bag->Count(TEXT("Item_CopperOre"))==4 && Bag->Count(TEXT("Item_Wood"))==1);
        auto* Progression=PC->GetPlayerState<APFInventoryPlayerState>()->Progression.Get();
        TestTrue(TEXT("New batch reward ledger survives actual disk restore"),Progression->GetRecord().CreditedCrafts.Contains(TEXT("Recipe_CopperBatch")));
        PC->GetPawn()->SetActorLocation(LoadedRefinery->GetActorLocation()+FVector(-150,0,90));
        TestTrue(TEXT("Restored required refinery can start remaining batch"),Craft->Start(TEXT("Recipe_CopperBatch"),PC->GetPawn()));TestTrue(TEXT("Cancel restored batch keeps fuel"),Craft->Cancel() && Bag->Count(TEXT("Item_Wood"))==1);
    }
    AddInfo(TEXT("[PrimalCrafting] OwnedStationsRestore: actual save/repeated load retains structure IDs/owners/support and rebuilds exactly two stateless providers without duplication."));
    Fixture.ForwardErrorMessages(this);return true;
}
#endif
