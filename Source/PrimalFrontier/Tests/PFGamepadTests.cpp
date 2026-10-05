#if WITH_DEV_AUTOMATION_TESTS
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Building/PFBuildingComponent.h"
#include "Inventory/PFItemPickup.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFGamepadTest,"PF.Input.Gamepad",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFGamepadTest::RunTest(const FString&)
{
    const auto* Context=LoadObject<UInputMappingContext>(nullptr,TEXT("/Game/Input/IMC_Default.IMC_Default"));
    if(!TestNotNull(TEXT("Template input context loads"),Context)){return false;}
    TSet<FName> PadActions;
    for(const auto& Mapping:Context->GetMappings())
    {if(Mapping.Key.IsGamepadKey() && Mapping.Action){PadActions.Add(Mapping.Action->GetFName());AddInfo(Mapping.Key.ToString()+TEXT(" -> ")+Mapping.Action->GetName());}}
    for(FName Action:{FName(TEXT("IA_Move")),FName(TEXT("IA_Look")),FName(TEXT("IA_Jump"))}){TestTrue(TEXT("Template gamepad action ")+Action.ToString(),PadActions.Contains(Action));}

    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));W->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());if(!P || !PC->GetInventory()){return false;}
    P->GetCharacterMovement()->DisableMovement();PC->SetControlRotation(FRotator::ZeroRotator);PC->SetAsLocalPlayerController();PC->InitInputSystem();
    auto Press=[&](FKey Key){for(const auto& Binding:PC->InputComponent->KeyBindings){if(Binding.Chord.Key==Key && Binding.KeyEvent==IE_Pressed){Binding.KeyDelegate.Execute(Key);return;}}AddError(TEXT("Missing gamepad binding ")+Key.ToString());};
    Press(EKeys::Gamepad_RightShoulder);TestTrue(TEXT("RB opens building"),PC->Building->bBuildMode);
    FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);W->SpawnActor<APFItemPickup>(Eye+FVector(150,0,0),FRotator::ZeroRotator);
    Press(EKeys::Gamepad_FaceButton_Left);TestEqual(TEXT("Pad X picks up while building"),PC->GetInventory()->Count(TEXT("Item_Wood")),5);
    Press(EKeys::Gamepad_Special_Left);TestTrue(TEXT("View opens bag"),PC->IsInventoryOpen());TestFalse(TEXT("Bag closes build overlay"),PC->Building->bBuildMode);
    Press(EKeys::Gamepad_DPad_Left);TestEqual(TEXT("D-left splits without duplicating"),PC->GetInventory()->Count(TEXT("Item_Wood")),5);TestEqual(TEXT("Split makes two stacks"),PC->GetInventory()->GetStacks().Num(),2);
    F.TickTestWorld(0.2f);Press(EKeys::Gamepad_DPad_Down);TestEqual(TEXT("Pad selects next stack"),PC->GetSelectedInventoryIndex(),1);
    Press(EKeys::Gamepad_DPad_Right);TestEqual(TEXT("D-right drops exactly one"),PC->GetInventory()->Count(TEXT("Item_Wood")),4);
    Press(EKeys::Gamepad_FaceButton_Top);TestTrue(TEXT("Y opens crafting"),PC->IsCraftingOpen());TestFalse(TEXT("Craft closes bag"),PC->IsInventoryOpen());
    Press(EKeys::Gamepad_FaceButton_Right);TestFalse(TEXT("B closes crafting"),PC->IsCraftingOpen());
    AddInfo(TEXT("[PrimalInput] Existing analog/jump mappings and bound gamepad pickup, split, drop and overlay routes verified. Physical controller and OS input delivery remain separate manual checks."));
    F.ForwardErrorMessages(this);return true;
}
#endif
