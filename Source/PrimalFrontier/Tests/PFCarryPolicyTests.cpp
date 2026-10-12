#if WITH_DEV_AUTOMATION_TESTS
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFItemPickup.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFWorldSaveFormat.h"
#include "Persistence/PFSaveFileStore.h"
#include "Components/BoxComponent.h"
#include "HAL/FileManager.h"
#include "Misc/ScopeExit.h"
#include "Building/PFBuildingCatalog.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Creatures/PFCreatureCatalog.h"
#include "World/PFWorldClock.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFCarryPolicyTest,"PF.Inventory.CarryPolicy",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFCarryPolicyTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFCarry/")+FGuid::NewGuid().ToString(EGuidFormats::Digits)+TEXT("/L_Automation")),nullptr,REN_DontCreateRedirectors|REN_NonTransactional);
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    W->SpawnActor<APFWorldClock>();
    auto* Floor=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Floor);Floor->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(2000,2000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Floor->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* I=PC->GetInventory();if(!P || !I || !I->Catalog){return false;}
    P->GetCharacterMovement()->DisableMovement();
    TestFalse(TEXT("Actual player bag defaults to soft carry policy"),I->bEnforceWeightLimit);
    for(const TCHAR* Id:{TEXT("Item_Wood"),TEXT("Item_Stone"),TEXT("Item_Food"),TEXT("Item_Water"),TEXT("Item_Cord")})
    {const auto* D=I->Definition(Id);if(!TestNotNull(TEXT("Real loaded stackable definition"),D)){return false;}TestEqual(TEXT("Actual stackable default 999k"),D->StackLimit,999000);}
    TestEqual(TEXT("Gear remains nonstackable"),I->Definition(TEXT("Item_BoundTool"))->StackLimit,1);
    TestTrue(TEXT("Real overweight max stack accepted"),I->Grant(TEXT("Item_Stone"),999000));
    TestEqual(TEXT("Exact real weight kg"),I->GetWeight(),999000.f);TestEqual(TEXT("Bounded one slot"),I->GetStacks().Num(),1);
    I->SlotLimit=1;TestFalse(TEXT("Soft weight does not bypass slots"),I->Grant(TEXT("Item_Wood"),1));I->SlotLimit=8;
    auto* Drop=I->Drop(I->GetStacks()[0].StackId,999000,P);if(!TestNotNull(TEXT("Actual max stack drop"),Drop)){return false;}
    TestEqual(TEXT("Drop conservation"),I->Count(TEXT("Item_Stone"))+Drop->GetContents().Quantity,999000);
    auto* Persistence=W->GetSubsystem<UPFWorldPersistence>();FString Error;FPFWorldSaveData Data;
    Persistence->Login(PC,TEXT(""));
    if(!TestTrue(TEXT("Capture large dropped stack"),Persistence->Capture(Data,Error))){AddError(Error);return false;}
    TestTrue(TEXT("Max stack pickup accepted overweight"),Drop->TryPickup(P));
    if(!TestTrue(TEXT("Capture real overweight player"),Persistence->Capture(Data,Error))){AddError(Error);return false;}
    TArray<uint8> Bytes;FPFWorldSaveData Decoded;
    TestTrue(TEXT("World metadata codec"),FPFWorldSaveFormat::Encode(Data,Bytes,Error));
    TestTrue(TEXT("Semantic decode keeps legal overweight player"),FPFWorldSaveFormat::DecodeValidated(Bytes,*I->Catalog,*NewObject<UPFBuildingCatalog>(),*NewObject<UPFCraftingCatalog>(),*NewObject<UPFCreatureCatalog>(),Decoded,Error));
    FPFPlayerSaveData Bag;TestTrue(TEXT("Trusted soft unpack"),FPFWorldSaveFormat::UnpackPlayer(Data.Players[0].Data,*I->Catalog,30,Bag,Error,false));
    TestFalse(TEXT("Same overweight bytes refused for hard chest policy"),FPFWorldSaveFormat::UnpackPlayer(Data.Players[0].Data,*I->Catalog,60,Bag,Error));
    TestTrue(TEXT("Repeated restoration keeps max count"),I->RestorePersistence(Bag.Inventory,0,Error));
    TestEqual(TEXT("No duplication during restore"),I->Count(TEXT("Item_Stone")),999000);
    const FString Slot=TEXT("AutomationCarry_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ON_SCOPE_EXIT {for(bool Backup:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot,Backup));}};
    if(!TestTrue(TEXT("Real file saves overweight player"),Persistence->Save(Slot,Error))){AddError(Error);return false;}
    TestTrue(TEXT("Change live state after save"),I->RemoveItem(TEXT("Item_Stone"),10));
    for(int32 N=0;N<2;++N)
    {if(!TestTrue(TEXT("Real file loads overweight player repeatedly"),Persistence->Load(Slot,Error))){AddError(Error);return false;}TestEqual(TEXT("Repeated load exact999k conservation"),I->Count(TEXT("Item_Stone")),999000);}
    I->bEnforceWeightLimit=true;TestFalse(TEXT("Hard container still refuses excess"),I->Grant(TEXT("Item_Wood"),1));
    F.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalInventory] Actual loaded999k defaults, soft bag/hard container, max drop/pickup/world archive and slot conservation checked."));return true;
}
#endif
