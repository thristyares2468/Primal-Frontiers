#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Inventory/PFHotbar.h"
#include "Inventory/PFInventoryComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFHotbarTest,"PF.Input.Hotbar",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFHotbarTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper Fixture;if(!Fixture.CreateTestWorld(EWorldType::Game)){return false;}
    auto* W=Fixture.GetTestWorld();W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));W->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);
    if(!Fixture.BeginPlayInTestWorld()){return false;}auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* I=PC->GetInventory();if(!P || !I){return false;}P->GetCharacterMovement()->DisableMovement();PC->SetAsLocalPlayerController();PC->InitInputSystem();
    auto Press=[&](FKey K){for(const auto& B:PC->InputComponent->KeyBindings){if(B.Chord.Key==K && B.KeyEvent==IE_Pressed){B.KeyDelegate.Execute(K);return;}}AddError(TEXT("Missing actual hotbar binding ")+K.ToString());};
    Press(EKeys::Gamepad_DPad_Left);TestEqual(TEXT("First left cycle starts at last slot"),PC->GetHotbar().Selected(),7);Press(EKeys::Gamepad_DPad_Right);TestEqual(TEXT("Cycle wraps to first"),PC->GetHotbar().Selected(),0);
    TestTrue(TEXT("Fixture wood"),I->Grant(TEXT("Item_Wood"),5));TestTrue(TEXT("Fixture food"),I->Grant(TEXT("Item_Food"),3));const FGuid Wood=I->GetStacks()[0].StackId,Food=I->GetStacks()[1].StackId;
    TestFalse(TEXT("Pin refused outside inventory"),PC->PinInventoryToHotbar(0));Press(EKeys::Tab);TestTrue(TEXT("Select actual food"),PC->SelectInventoryStack(Food));PC->HandleInventoryKey(EKeys::Eight);
    TestEqual(TEXT("Slot8 exact identity"),PC->GetHotbar().Identity(7),Food);TestFalse(TEXT("Invalid index preserves slots"),PC->PinInventoryToHotbar(8));
    TestTrue(TEXT("Select wood"),PC->SelectInventoryStack(Wood));PC->HandleInventoryKey(EKeys::One);TestEqual(TEXT("Slot1 actual wood"),PC->GetHotbar().Identity(0),Wood);
    TestTrue(TEXT("Reassign same batch without duplication"),PC->PinInventoryToHotbar(1));TestFalse(TEXT("Old shortcut cleared"),PC->GetHotbar().Identity(0).IsValid());TestEqual(TEXT("Quantity conserved on assignment"),I->Count(TEXT("Item_Wood")),5);
    TestTrue(TEXT("Sort actual bag"),I->SortStacks());TestEqual(TEXT("Sorting preserves food binding"),PC->GetHotbar().Resolve(7,I,UPFInventoryComponent::ServerTime(W))->StackId,Food);
    FPFHotbar Model;TestFalse(TEXT("Foreign GUID refuses assignment"),Model.Assign(0,FGuid::NewGuid(),I,0));TestFalse(TEXT("Negative index refused"),Model.Assign(-1,Food,I,0));
    Press(EKeys::Tab);Press(EKeys::Eight);TestEqual(TEXT("Gameplay8 selects without creating craft"),PC->GetHotbar().Selected(),7);TestEqual(TEXT("Selection preserves food"),I->Count(TEXT("Item_Food")),3);
    P->FindComponentByClass<UPFPlayerSurvivalComponent>()->SetHunger(40);Press(EKeys::Q);TestEqual(TEXT("Existing server path consumes exactly one"),I->Count(TEXT("Item_Food")),2);Press(EKeys::Q);TestEqual(TEXT("Rate limit retained"),I->Count(TEXT("Item_Food")),2);
    Fixture.TickTestWorld(.2f);Press(EKeys::Two);Press(EKeys::Q);TestEqual(TEXT("Resource cannot be consumed"),I->Count(TEXT("Item_Wood")),5);
    Press(EKeys::Tab);TestFalse(TEXT("Gameplay selection refuses modal"),PC->SelectHotbar(7));TestTrue(TEXT("Reselect actual food"),PC->SelectInventoryStack(Food));PC->HandleInventoryKey(EKeys::Eight);PC->UseHotbar();TestEqual(TEXT("Use refuses bag modal"),I->Count(TEXT("Item_Food")),2);
    PC->HandleInventoryKey(EKeys::Zero);TestFalse(TEXT("Clear unbinds without deleting food"),PC->GetHotbar().Identity(7).IsValid());TestEqual(TEXT("Clear conserved"),I->Count(TEXT("Item_Food")),2);
    PC->HandleInventoryKey(EKeys::Eight);Press(EKeys::Tab);TestTrue(TEXT("Remove pinned batch"),I->Remove(Food,2));TestTrue(TEXT("New same-type batch"),I->Grant(TEXT("Item_Food"),1));Press(EKeys::Eight);Press(EKeys::Q);TestEqual(TEXT("Removed batch never falls back to fresh same item"),I->Count(TEXT("Item_Food")),1);
    const auto NewFood=I->GetStacks().FindByPredicate([](const auto& S){return S.ItemId==TEXT("Item_Food");})->StackId;Model.Assign(0,NewFood,I,0);TestNull(TEXT("Expired deadline refuses even before prune"),Model.Resolve(0,I,86401));
    Press(EKeys::Tab);PC->SelectInventoryStack(NewFood);PC->HandleInventoryKey(EKeys::Eight);Press(EKeys::Tab);P->FindComponentByClass<UPFPlayerSurvivalComponent>()->ApplyDamage(10000);PC->UseHotbar();TestEqual(TEXT("Dead player cannot use shortcut"),I->Count(TEXT("Item_Food")),1);TestFalse(TEXT("Dead player cannot select"),PC->SelectHotbar(0));
    AddInfo(TEXT("[PrimalUI] Hotbar ownership, bounds, exact identity, sorting, modal/death/expiry/rate refusal and server consumption conservation checked."));Fixture.ForwardErrorMessages(this);return true;
}
#endif
