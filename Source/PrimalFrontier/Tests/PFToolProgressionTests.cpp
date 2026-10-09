#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFResourceNode.h"
#include "Inventory/PFItemCatalog.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Persistence/PFPlayerSaveAdapter.h"
#include "Persistence/PFPlayerSaveFormat.h"
#include "PFAssetPaths.h"
#include "Creatures/PFCreature.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Progression/PFProgressionComponent.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFToolProgressionTest,"PF.Crafting.ToolProgression",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFToolProgressionTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}
    auto* W=F.GetTestWorld();W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    W->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* I=PC->GetInventory();auto* C=PC->GetCrafting();
    if(!P || !I || !C){return false;}P->GetCharacterMovement()->DisableMovement();
    // Real loaded assets, not just constructor defaults. Designer overrides must be integrated.
    if(!TestNotNull(TEXT("Loaded cord definition"),I->Definition(TEXT("Item_Cord"))) ||
       !TestNotNull(TEXT("Loaded bound tool definition"),I->Definition(TEXT("Item_BoundTool"))) ||
       !TestNotNull(TEXT("Loaded cord recipe"),C->Catalog->Recipe(TEXT("Recipe_Cord"),I->Catalog)) ||
       !TestNotNull(TEXT("Loaded upgrade recipe"),C->Catalog->Recipe(TEXT("Recipe_BoundTool"),I->Catalog))){return false;}
    TestEqual(TEXT("Loaded baseline tool performance"),I->Definition(TEXT("Item_Tool"))->GatheringHits,2);
    // Copy real definitions to shorten durations in this isolated native fixture only.
    C->Catalog=DuplicateObject<UPFCraftingCatalog>(C->Catalog,C);for(auto& R:C->Catalog->Recipes){R.Duration=0.5f;}
    auto Advance=[&](){for(int32 N=0;N<8;++N){F.TickTestWorld(0.1f);}};
    TestEqual(TEXT("Bare hands gather"),I->GatheringHits(),1);TestEqual(TEXT("Bare hands combat"),I->MeleeDamage(),20.f);
    TestFalse(TEXT("Missing fibre refused"),C->Start(TEXT("Recipe_Cord"),P));
    TestTrue(TEXT("Fibre supplied only by authority fixture"),I->Grant(TEXT("Item_Fibre"),8));
    TestTrue(TEXT("Cord starts"),C->Start(TEXT("Recipe_Cord"),P));TestFalse(TEXT("Duplicate starts refused"),C->Start(TEXT("Recipe_Cord"),P));
    TestTrue(TEXT("Cancel cord"),C->Cancel());Advance();TestEqual(TEXT("Cancel preserves fibre"),I->Count(TEXT("Item_Fibre")),8);
    TestTrue(TEXT("Cord restart"),C->Start(TEXT("Recipe_Cord"),P));Advance();
    TestEqual(TEXT("Single cord output"),I->Count(TEXT("Item_Cord")),1);TestEqual(TEXT("Exact fibre cost"),I->Count(TEXT("Item_Fibre")),4);
    Advance();TestEqual(TEXT("No repeated output"),I->Count(TEXT("Item_Cord")),1);
    TestTrue(TEXT("Second cord for capacity probe"),C->Start(TEXT("Recipe_Cord"),P));I->WeightLimit=0.1f;Advance();I->WeightLimit=30;
    TestEqual(TEXT("Capacity refusal preserves fibre"),I->Count(TEXT("Item_Fibre")),4);TestEqual(TEXT("Capacity refusal no output"),I->Count(TEXT("Item_Cord")),1);
    I->Grant(TEXT("Item_Tool"),1);I->Grant(TEXT("Item_Wood"),2);
    TestEqual(TEXT("Baseline tier gathering"),I->GatheringHits(),2);TestEqual(TEXT("Baseline tier melee"),I->MeleeDamage(),35.f);
    auto* G=PC->GetPlayerState<APFInventoryPlayerState>()->Progression.Get();FPFProgressionRecord Seed=G->GetRecord();Seed.Experience=100;FString AccessError;
    TestFalse(TEXT("Tool upgrade requires learned knowledge"),C->Start(TEXT("Recipe_BoundTool"),P));
    TestTrue(TEXT("Explicit trusted XP fixture for tier mechanics"),G->Restore(Seed,AccessError));PC->ServerLearnKnowledge(TEXT("Tech_FieldTools"));
    TestTrue(TEXT("Upgrade starts"),C->Start(TEXT("Recipe_BoundTool"),P));TestTrue(TEXT("Upgrade cancellation"),C->Cancel());
    TestEqual(TEXT("Cancellation retains tool"),I->Count(TEXT("Item_Tool")),1);
    TestTrue(TEXT("Upgrade restarts"),C->Start(TEXT("Recipe_BoundTool"),P));Advance();
    TestEqual(TEXT("Upgrade consumes baseline tool"),I->Count(TEXT("Item_Tool")),0);TestEqual(TEXT("Upgrade consumes cord"),I->Count(TEXT("Item_Cord")),0);
    TestEqual(TEXT("Upgrade consumes wood"),I->Count(TEXT("Item_Wood")),0);TestEqual(TEXT("One upgraded tool"),I->Count(TEXT("Item_BoundTool")),1);
    TestEqual(TEXT("Bound gathering tier"),I->GatheringHits(),3);TestEqual(TEXT("Bound melee tier"),I->MeleeDamage(),45.f);
    TestTrue(TEXT("Bound tool retains replicated held-tool presentation"),P->HasGatheringTool());
    // Actual traced gathering consumes all three hits once; total yield remains six.
    PC->SetControlRotation(FRotator::ZeroRotator);FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);
    auto* Node=W->SpawnActor<APFResourceNode>(Eye+Look.Vector()*150,FRotator::ZeroRotator);
    TestTrue(TEXT("Tier tool gathers traced node"),Node->Gather(P));TestEqual(TEXT("Finite six wood yield"),I->Count(TEXT("Item_Wood")),6);
    TestEqual(TEXT("All hits depleted"),Node->HitsRemaining,0);TestFalse(TEXT("Spam/depleted refusal"),Node->Gather(P));
    // Existing version1 save data stores IDs; restore preserves stats derived from catalog.
    FPFPlayerSaveData Saved;Saved.PlayerId=FGuid::NewGuid();Saved.Inventory=FPFPlayerSaveAdapter::CaptureInventory(*I);
    FPFPlayerSaveLimits Limits;TArray<uint8> Bytes;FString Error;TestTrue(TEXT("Encode new item IDs"),FPFPlayerSaveFormat::Encode(Saved,*I->Catalog,Limits,Bytes,Error));
    FPFPlayerSaveData Decoded;TestTrue(TEXT("Decode new item IDs"),FPFPlayerSaveFormat::Decode(Bytes,*I->Catalog,Limits,Decoded,Error));
    I->RemoveItem(TEXT("Item_BoundTool"),1);TestEqual(TEXT("Removed tool no lingering effect"),I->GatheringHits(),1);
    TestTrue(TEXT("Restore saved bag"),I->RestorePersistence(Decoded.Inventory,0,Error));TestEqual(TEXT("Restored tool tier"),I->GatheringHits(),3);
    Node->Destroy();
    auto* Creature=W->SpawnActor<APFCreature>(Eye+Look.Vector()*150,FRotator::ZeroRotator);
    Creature->GetCharacterMovement()->DisableMovement();Creature->SetActorTickEnabled(false);
    const float HealthBefore=Creature->Health;
    auto* Needs=P->FindComponentByClass<UPFPlayerSurvivalComponent>();
    const float StaminaBefore=Needs->GetVitals().Stamina;
    PC->ServerAttackCreature();
    TestEqual(TEXT("Actual traced server attack uses bound tier"),HealthBefore-Creature->Health,45.f);
    TestEqual(TEXT("Actual attack spends five stamina"),StaminaBefore-Needs->GetVitals().Stamina,5.f);
    PC->ServerAttackCreature();TestEqual(TEXT("Immediate duplicate swing refused"),Creature->Health,HealthBefore-45.f);
    PC->PlayerState->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Client direct upgrade rejected"),C->Start(TEXT("Recipe_BoundTool"),P));
    TestFalse(TEXT("Client direct grant rejected"),I->Grant(TEXT("Item_BoundTool"),1));PC->PlayerState->SetRole(ROLE_Authority);
    // Bad authored performance cannot confer tool power.
    auto* Invalid=NewObject<UPFItemCatalog>();auto& Tool=*Invalid->Items.FindByPredicate([](const auto& D){return D.Id==TEXT("Item_BoundTool");});
    Tool.MeleeDamage=std::numeric_limits<float>::quiet_NaN();TestNull(TEXT("NaN damage invalidates definition"),Invalid->Find(Tool.Id));
    Tool.MeleeDamage=45;Tool.GatheringHits=9;TestNull(TEXT("Out of range gathering invalid"),Invalid->Find(Tool.Id));
    auto& Wood=*Invalid->Items.FindByPredicate([](const auto& D){return D.Id==TEXT("Item_Wood");});Wood.GatheringHits=3;
    TestNull(TEXT("Non-tool cannot declare tool benefit"),Invalid->Find(Wood.Id));
    AddInfo(TEXT("[PrimalCrafting] Real catalog, cord/upgrade conservation, authority, finite yield, tier removal/restore and invalid data checked."));
    F.ForwardErrorMessages(this);return true;
}
#endif
