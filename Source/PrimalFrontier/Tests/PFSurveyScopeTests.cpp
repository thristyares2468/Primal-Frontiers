#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFCraftingStation.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Settings/PFGameUserSettings.h"
#include "Persistence/PFWorldPersistence.h"
#include "Persistence/PFSaveFileStore.h"
#include "World/PFWorldClock.h"
#include "Components/BoxComponent.h"
#include "Components/InputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "HAL/FileManager.h"
#include "Tests/AutomationCommon.h"
#include <limits>
namespace
{
bool ScopeWorld(FTestWorldWrapper& F,APFSurvivalPlayerController*& PC)
{
    if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    W->GetOutermost()->Rename(*(TEXT("/Temp/PFScope/")+FGuid::NewGuid().ToString(EGuidFormats::Digits)+TEXT("/L_Automation")),nullptr,REN_DontCreateRedirectors|REN_NonTransactional);
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Floor=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Floor);Floor->SetRootComponent(Box);Box->RegisterComponent();Box->SetBoxExtent(FVector(3000,3000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Floor->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APFWorldClock>();W->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());if(!P || !PC->GetInventory() || !PC->GetCrafting()){return false;}
    P->GetCharacterMovement()->DisableMovement();P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;
    return true;
}
bool ScopeInputs(APFSurvivalPlayerController* PC)
{
    auto* I=PC->GetInventory();return I->Grant(TEXT("Item_QuartzLens"),1) && I->Grant(TEXT("Item_CopperIngot"),2) && I->Grant(TEXT("Item_Cord"),1) && I->Grant(TEXT("Item_Wood"),3);
}
bool ScopeKnowledge(APFSurvivalPlayerController* PC)
{
    FPFProgressionRecord R;R.Experience=250;R.Knowledge={TEXT("Tech_FieldTools"),TEXT("Tech_Metalwork"),TEXT("Tech_Gemwork")};FString Error;
    return PC->GetPlayerState<APFInventoryPlayerState>()->Progression->Restore(R,Error);
}
APFCraftingStation* ScopeBench(APFSurvivalPlayerController* PC)
{return PC->GetWorld()->SpawnActor<APFCraftingStation>(PC->GetPawn()->GetActorLocation()+FVector(120,0,0),FRotator::ZeroRotator);}
void ScopeTick(FTestWorldWrapper& F,int32 Count){for(int32 N=0;N<Count;++N){F.TickTestWorld(.1f);}}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSurveyLifecycleTest,"PF.Optics.Lifecycle",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFSurveyLifecycleTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;APFSurvivalPlayerController* PC=nullptr;if(!ScopeWorld(F,PC)){return false;}
    auto* I=PC->GetInventory();auto* C=PC->GetCrafting();auto* P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());const FName Recipe=TEXT("Recipe_SurveyScope"),Item=TEXT("Item_SurveyScope");
    if(!TestNotNull(TEXT("Actual loaded optical item"),I->Definition(Item)) || !TestNotNull(TEXT("Actual loaded optical recipe"),C->Catalog->Recipe(Recipe,I->Catalog))){return false;}
    TestTrue(TEXT("Scope cannot boost harvesting or attack"),I->Definition(Item)->GatheringHits==0 && I->Definition(Item)->MeleeDamage==0 && I->Definition(Item)->OpticalMagnification==2);
    auto* Bad=NewObject<UPFItemCatalog>();auto* D=Bad->Items.FindByPredicate([&](const auto& X){return X.Id==Item;});
    D->OpticalMagnification=std::numeric_limits<float>::quiet_NaN();TestNull(TEXT("NaN magnification refused"),Bad->Find(Item));D->OpticalMagnification=20;TestNull(TEXT("Excessive magnification refused"),Bad->Find(Item));D->OpticalMagnification=2;D->GatheringHits=1;TestNull(TEXT("Optic cannot smuggle gathering benefit"),Bad->Find(Item));
    TestTrue(TEXT("Real ingredients fixture"),ScopeInputs(PC));TestFalse(TEXT("Knowledge required"),C->Start(Recipe,P));TestTrue(TEXT("Learned boundary fixture"),ScopeKnowledge(PC));TestFalse(TEXT("Workbench required"),C->Start(Recipe,P));
    auto* Bench=ScopeBench(PC);if(!TestNotNull(TEXT("Workbench"),Bench)){return false;}
    PC->PlayerState->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Client direct craft refused"),C->Start(Recipe,P));PC->PlayerState->SetRole(ROLE_Authority);
    TestTrue(TEXT("Start actual assembly"),C->Start(Recipe,P));TestTrue(TEXT("Cancel preserves lens"),C->Cancel() && I->Count(TEXT("Item_QuartzLens"))==1);
    TestTrue(TEXT("Start capacity boundary"),C->Start(Recipe,P));I->SlotLimit=1;ScopeTick(F,110);I->SlotLimit=8;
    TestTrue(TEXT("Capacity failure preserves all inputs"),I->Count(Item)==0 && I->Count(TEXT("Item_QuartzLens"))==1 && I->Count(TEXT("Item_Wood"))==3);
    TestTrue(TEXT("Actual assembly completes"),C->Start(Recipe,P));ScopeTick(F,110);
    TestTrue(TEXT("Exact scope costs and output"),I->Count(Item)==1 && I->Count(TEXT("Item_Wood"))==1 && I->Count(TEXT("Item_QuartzLens"))==0 && I->Count(TEXT("Item_CopperIngot"))==0 && I->Count(TEXT("Item_Cord"))==0);
    PC->SetAsLocalPlayerController();PC->InitInputSystem();
    auto Press=[&](FKey Key){for(const auto& B:PC->InputComponent->KeyBindings){if(B.Chord.Key==Key && B.KeyEvent==IE_Pressed){B.KeyDelegate.Execute(Key);return;}}AddError(TEXT("Missing real scope input binding"));};
    auto Pin=[&](){const auto* S=I->GetStacks().FindByPredicate([&](const auto& X){return X.ItemId==Item;});if(!S){return false;}PC->SetInventoryOpen(true);const bool Ok=PC->SelectInventoryStack(S->StackId) && PC->PinInventoryToHotbar(0);PC->SetInventoryOpen(false);return Ok && PC->SelectHotbar(0);};
    const float Base=UPFGameUserSettings::Get()->Preferences.FieldOfView;
    TestTrue(TEXT("Pin exact actual scope batch"),Pin());Press(EKeys::Q);P->Tick(.1f);
    TestEqual(TEXT("Actual hotbar key engages2x"),PC->GetSurveyMagnification(),2.f);
    const float Expected=FMath::RadiansToDegrees(2*FMath::Atan(FMath::Tan(FMath::DegreesToRadians(Base*.5f))/2));
    TestTrue(TEXT("Real first-person camera applies optical FOV"),FMath::IsNearlyEqual(P->GetFirstPersonCameraComponent()->FieldOfView,Expected,.01f));
    for(float FOV:{70.f,90.f,110.f}){TestTrue(TEXT("All supported base FOVs derive exact2x"),FMath::IsNearlyEqual(PC->ResolveSurveyFieldOfView(FOV),FMath::RadiansToDegrees(2*FMath::Atan(FMath::Tan(FMath::DegreesToRadians(FOV*.5f))/2)),.01f));}
    TestEqual(TEXT("User FOV preference never changed"),UPFGameUserSettings::Get()->Preferences.FieldOfView,Base);
    Press(EKeys::Gamepad_DPad_Down);P->Tick(.1f);TestTrue(TEXT("Gamepad lowers and restores base"),PC->GetSurveyMagnification()==0 && P->GetFirstPersonCameraComponent()->FieldOfView==Base);
    Press(EKeys::Q);PC->SetInventoryOpen(true);TestTrue(TEXT("Inventory immediately clears and resets optics"),PC->GetSurveyMagnification()==0 && P->GetFirstPersonCameraComponent()->FieldOfView==Base);PC->SetInventoryOpen(false);TestEqual(TEXT("Closing menu never resumes stale scope"),PC->GetSurveyMagnification(),0.f);
    Press(EKeys::Q);TestTrue(TEXT("Switching hotbar resets"),PC->SelectHotbar(1) && PC->GetSurveyMagnification()==0);PC->SelectHotbar(0);Press(EKeys::Q);
    const FGuid Old=PC->GetHotbar().Identity(0);TestTrue(TEXT("Remove bound scope"),I->Remove(Old,1));TestTrue(TEXT("Same type replacement"),I->Grant(Item,1));P->Tick(.1f);PC->UseHotbar();
    TestTrue(TEXT("Replacement never substitutes lost GUID"),PC->GetSurveyMagnification()==0 && P->GetFirstPersonCameraComponent()->FieldOfView==Base);TestTrue(TEXT("Repin replacement"),Pin());Press(EKeys::Q);
    auto* OldPawn=P;PC->UnPossess();F.GetTestWorld()->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);P=CastChecked<APFSurvivorCharacter>(PC->GetPawn());OldPawn->Destroy();
    TestTrue(TEXT("Different pawn cannot inherit local scope state"),P!=OldPawn && PC->ResolveSurveyFieldOfView(Base)==Base && PC->GetSurveyMagnification()==0);
    Press(EKeys::Q);P->Survival->ApplyDamage(10000);TestEqual(TEXT("Death refuses optical state"),PC->ResolveSurveyFieldOfView(Base),Base);PC->UseHotbar();TestEqual(TEXT("Dead use refused"),PC->GetSurveyMagnification(),0.f);
    TestTrue(TEXT("Scope gave no harvesting, melee or item duplication"),I->GatheringHits()==1 && I->MeleeDamage()==20 && I->Count(Item)==1);
    AddInfo(TEXT("[PrimalOptics] Lifecycle: actual catalog/craft/cancel/capacity, exact hotbar GUID, camera FOV, controller route, settings/modal/pawn/death reset and unchanged combat/gathering verified."));F.ForwardErrorMessages(this);return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFSurveyPersistenceTest,"PF.Optics.Persistence",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FPFSurveyPersistenceTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;APFSurvivalPlayerController* PC=nullptr;if(!ScopeWorld(F,PC)){return false;}
    auto* Save=F.GetTestWorld()->GetSubsystem<UPFWorldPersistence>();FString Error;Save->Login(PC,TEXT(""));
    TestTrue(TEXT("Scope ingredients and knowledge"),ScopeInputs(PC) && ScopeKnowledge(PC));ScopeBench(PC);
    if(!TestTrue(TEXT("Assemble before disk save"),PC->GetCrafting()->Start(TEXT("Recipe_SurveyScope"),PC->GetPawn()))){return false;}ScopeTick(F,110);
    auto* I=PC->GetInventory();const auto* Scope=I->GetStacks().FindByPredicate([](const auto& S){return S.ItemId==TEXT("Item_SurveyScope");});if(!TestNotNull(TEXT("Actual completed scope"),Scope)){return false;}const FGuid Id=Scope->StackId;
    const FString Slot=TEXT("AutomationScope")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    ON_SCOPE_EXIT{for(bool Second:{false,true}){IFileManager::Get().Delete(*FPFSaveFileStore::Path(Slot,Second));IFileManager::Get().Delete(*(FPFSaveFileStore::Path(Slot,Second)+TEXT(".pending")));}};
    if(!TestTrue(TEXT("Real world disk save"),Save->Save(Slot,Error))){AddError(Error);return false;}
    for(int32 N=0;N<2;++N)
    {
        TestTrue(TEXT("Unsaved scope removed"),I->Remove(Id,1));if(!TestTrue(TEXT("Actual repeated load"),Save->Load(Slot,Error))){AddError(Error);return false;}
        const auto* Restored=I->GetStacks().FindByPredicate([&](const auto& S){return S.StackId==Id;});TestTrue(TEXT("Same identity,exact output/leftovers"),Restored && Restored->ItemId==TEXT("Item_SurveyScope") && Restored->Quantity==1 && I->Count(TEXT("Item_Wood"))==1);
        const auto& R=PC->GetPlayerState<APFInventoryPlayerState>()->Progression->GetRecord();TestTrue(TEXT("Knowledge and once-only craft credit retained"),R.Knowledge.Contains(TEXT("Tech_Gemwork")) && R.CreditedCrafts.Contains(TEXT("Recipe_SurveyScope")) && R.Experience==270);
    }
    TestEqual(TEXT("Zoom is never restored as gameplay authority"),PC->GetSurveyMagnification(),0.f);
    AddInfo(TEXT("[PrimalOptics] Persistence: real disk save/repeated load preserves scope identity, cost, knowledge and first-craft ledger without duplication or saved zoom."));F.ForwardErrorMessages(this);return true;
}
#endif
