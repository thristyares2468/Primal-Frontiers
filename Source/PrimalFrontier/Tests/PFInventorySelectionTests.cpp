#if WITH_DEV_AUTOMATION_TESTS
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Inventory/PFInventoryComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFInventorySelectionTest,"PF.Input.InventorySelection",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFInventorySelectionTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}
    auto* W=F.GetTestWorld();W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    W->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* I=PC->GetInventory();if(!P || !I){return false;}
    P->GetCharacterMovement()->DisableMovement();PC->SetAsLocalPlayerController();PC->InitInputSystem();
    auto Press=[&](FKey Key){for(const auto& B:PC->InputComponent->KeyBindings){if(B.Chord.Key==Key && B.KeyEvent==IE_Pressed){B.KeyDelegate.Execute(Key);return;}}AddError(TEXT("Missing test binding ")+Key.ToString());};
    TestTrue(TEXT("Short-lived food fixture"),I->AddExisting(TEXT("Item_Food"),1,UPFInventoryComponent::ServerTime(W)+0.2));
    TestTrue(TEXT("Permanent second stack"),I->Grant(TEXT("Item_Wood"),5));
    Press(EKeys::Tab);TestEqual(TEXT("First opening selects first stack"),PC->GetSelectedInventoryIndex(),0);
    F.TickTestWorld(0.3f);I->PruneExpired();
    TestEqual(TEXT("Expired selection becomes none, not replacement wood"),PC->GetSelectedInventoryIndex(),INDEX_NONE);
    Press(EKeys::G);TestEqual(TEXT("Drop cannot silently act on replacement stack"),I->Count(TEXT("Item_Wood")),5);
    TestTrue(TEXT("Missing selection gives explicit reason"),PC->GetInventoryMessage().Contains(TEXT("Select")));
    Press(EKeys::Tab);Press(EKeys::Tab);
    TestEqual(TEXT("Reopening does not silently reselect after expiry"),PC->GetSelectedInventoryIndex(),INDEX_NONE);
    Press(EKeys::Gamepad_DPad_Down);TestEqual(TEXT("Explicit pad selection chooses remaining stack"),PC->GetSelectedInventoryIndex(),0);
    F.TickTestWorld(0.2f);Press(EKeys::X);TestEqual(TEXT("Split preserves selected source identity"),PC->GetSelectedInventoryIndex(),0);
    if(!TestEqual(TEXT("Split fixture makes two stacks"),I->GetStacks().Num(),2)){return false;}
    const auto First=I->GetStacks()[0],Second=I->GetStacks()[1];
    Press(EKeys::Down);TestEqual(TEXT("Next selects second GUID"),PC->GetSelectedInventoryIndex(),1);
    TestTrue(TEXT("Remove unrelated earlier row"),I->Remove(First.StackId,First.Quantity));
    TestEqual(TEXT("Selection follows same GUID to its new row"),PC->GetSelectedInventoryIndex(),0);
    TestTrue(TEXT("Remove selected row"),I->Remove(Second.StackId,Second.Quantity));
    TestTrue(TEXT("Replacement item arrives"),I->Grant(TEXT("Item_Stone"),3));
    TestEqual(TEXT("New item cannot inherit old selection"),PC->GetSelectedInventoryIndex(),INDEX_NONE);
    F.TickTestWorld(0.2f);Press(EKeys::Gamepad_DPad_Right);
    TestEqual(TEXT("Pad drop preserves unselected replacement item"),I->Count(TEXT("Item_Stone")),3);
    Press(EKeys::Up);TestEqual(TEXT("Explicit keyboard selection recovers"),PC->GetSelectedInventoryIndex(),0);
    Press(EKeys::G);TestEqual(TEXT("Explicit selection allows actual drop"),I->Count(TEXT("Item_Stone")),2);
    AddInfo(TEXT("[PrimalInput] Stable selection, expiry/refusal, row shifts, split, replacement and explicit reselection checked using bound inputs."));
    F.ForwardErrorMessages(this);return true;
}
#endif
