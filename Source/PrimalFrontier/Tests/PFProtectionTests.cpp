#if WITH_DEV_AUTOMATION_TESTS
#include "Crafting/PFCraftingComponent.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Creatures/PFCreature.h"
#include "Persistence/PFPlayerSaveAdapter.h"
#include "Persistence/PFPlayerSaveFormat.h"
#include "UI/PFInventoryDetails.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include <limits>
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFProtectionTest,"PF.Crafting.Protection",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFProtectionTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}
    auto* W=F.GetTestWorld();W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    W->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();auto* Mode=W->GetAuthGameMode<APFSurvivalGameMode>();Mode->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* I=PC->GetInventory();auto* C=PC->GetCrafting();if(!P || !I || !C){return false;}
    P->GetCharacterMovement()->DisableMovement();TestEqual(TEXT("No free initial protection"),I->CreatureHitReduction(),0.f);
    if(!TestNotNull(TEXT("Real loaded guard"),I->Definition(TEXT("Item_WovenGuard"))) || !TestNotNull(TEXT("Real loaded guard recipe"),C->Catalog->Recipe(TEXT("Recipe_WovenGuard"),I->Catalog))){return false;}
    TestEqual(TEXT("Authored guard duration"),C->Catalog->Recipe(TEXT("Recipe_WovenGuard"),I->Catalog)->Duration,6.f);
    C->Catalog=DuplicateObject<UPFCraftingCatalog>(C->Catalog,C);for(auto& R:C->Catalog->Recipes){R.Duration=.5f;}
    auto Advance=[&](){for(int N=0;N<8;++N){F.TickTestWorld(.1f);}};
    TestFalse(TEXT("Missing inputs refused"),C->Start(TEXT("Recipe_WovenGuard"),P));I->Grant(TEXT("Item_Fibre"),8);I->Grant(TEXT("Item_Cord"),1);I->Grant(TEXT("Item_Wood"),2);
    TestTrue(TEXT("Guard starts"),C->Start(TEXT("Recipe_WovenGuard"),P));TestFalse(TEXT("Duplicate refused"),C->Start(TEXT("Recipe_WovenGuard"),P));TestTrue(TEXT("Cancel"),C->Cancel());Advance();
    TestEqual(TEXT("Cancel preserves fibre"),I->Count(TEXT("Item_Fibre")),8);TestEqual(TEXT("Cancel no protection"),I->CreatureHitReduction(),0.f);
    TestTrue(TEXT("Capacity probe"),C->Start(TEXT("Recipe_WovenGuard"),P));I->WeightLimit=.1f;Advance();I->WeightLimit=30;
    TestEqual(TEXT("Capacity refusal preserves inputs"),I->Count(TEXT("Item_Cord")),1);TestEqual(TEXT("Capacity refusal no output"),I->Count(TEXT("Item_WovenGuard")),0);
    TestTrue(TEXT("Guard restarts"),C->Start(TEXT("Recipe_WovenGuard"),P));Advance();TestEqual(TEXT("One guard"),I->Count(TEXT("Item_WovenGuard")),1);
    for(const TCHAR* Id:{TEXT("Item_Fibre"),TEXT("Item_Cord"),TEXT("Item_Wood")}){TestEqual(TEXT("Exact input cost"),I->Count(Id),0);}
    TestEqual(TEXT("Carried guard reduction"),I->CreatureHitReduction(),.25f);TestEqual(TEXT("Guard no gather power"),I->GatheringHits(),1);TestEqual(TEXT("Guard no melee power"),I->MeleeDamage(),20.f);
    I->Grant(TEXT("Item_WovenGuard"),1);TestEqual(TEXT("Two guards do not stack mitigation"),I->CreatureHitReduction(),.25f);I->RemoveItem(TEXT("Item_WovenGuard"),1);
    TestTrue(TEXT("Inventory guard detail explicit"),PFInventoryDetails::Describe(&I->GetStacks()[0],I->Definition(TEXT("Item_WovenGuard")),0).Body.ToString().Contains(TEXT("25% creature hit reduction")));
    auto* Creature=W->SpawnActor<APFCreature>(FVector(1000,0,100),FRotator::ZeroRotator);Creature->SetActorTickEnabled(false);
    auto* Needs=P->Survival.Get();Needs->SetHealth(100);TestEqual(TEXT("Real creature-caused twenty loses fifteen"),P->TakeDamage(20,FDamageEvent(),nullptr,Creature),15.f);
    TestEqual(TEXT("Reduced health actual"),Needs->GetVitals().Health,85.f);TestEqual(TEXT("Generic damage is not mitigated"),P->TakeDamage(20,FDamageEvent(),nullptr,nullptr),20.f);
    Needs->SetHealth(100);Needs->SetHunger(0);Needs->SetThirst(0);Needs->SetExposure(1);Needs->AdvanceNeeds(1);
    TestEqual(TEXT("Starvation dehydration exposure unchanged"),Needs->GetVitals().Health,90.f);Needs->SetHunger(100);Needs->SetThirst(100);Needs->SetExposure(0);
    FPFPlayerSaveData Saved;Saved.PlayerId=FGuid::NewGuid();Saved.Inventory=FPFPlayerSaveAdapter::CaptureInventory(*I);
    FPFPlayerSaveLimits Limits;TArray<uint8> Bytes;FString Error;FPFPlayerSaveData Decoded;
    TestTrue(TEXT("V1 guard encode"),FPFPlayerSaveFormat::Encode(Saved,*I->Catalog,Limits,Bytes,Error));TestTrue(TEXT("V1 guard decode"),FPFPlayerSaveFormat::Decode(Bytes,*I->Catalog,Limits,Decoded,Error));
    I->RemoveItem(TEXT("Item_WovenGuard"),1);TestEqual(TEXT("Removal immediately removes protection"),I->CreatureHitReduction(),0.f);
    TestEqual(TEXT("Creature hit after removal full amount"),P->TakeDamage(20,FDamageEvent(),nullptr,Creature),20.f);
    TestTrue(TEXT("Restore guard"),I->RestorePersistence(Decoded.Inventory,0,Error));TestEqual(TEXT("Restored mitigation derives once"),I->CreatureHitReduction(),.25f);
    P->SetRole(ROLE_AutonomousProxy);const float Before=Needs->GetVitals().Health;TestEqual(TEXT("Client cannot apply creature damage"),P->TakeDamage(20,FDamageEvent(),nullptr,Creature),0.f);TestEqual(TEXT("Client health unchanged"),Needs->GetVitals().Health,Before);P->SetRole(ROLE_Authority);
    PC->PlayerState->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Client direct grant refused"),I->Grant(TEXT("Item_WovenGuard"),1));TestFalse(TEXT("Client craft refused"),C->Start(TEXT("Recipe_WovenGuard"),P));PC->PlayerState->SetRole(ROLE_Authority);
    Mode->RespawnDelay=.1f;P->TakeDamage(1000,FDamageEvent(),nullptr,Creature);TestTrue(TEXT("Guard cannot prevent death"),Needs->IsDead());Advance();
    P=Cast<APFSurvivorCharacter>(PC->GetPawn());if(!TestNotNull(TEXT("Valid respawn"),P)){return false;}P->GetCharacterMovement()->DisableMovement();
    TestEqual(TEXT("Fresh respawn health"),P->Survival->GetVitals().Health,100.f);TestEqual(TEXT("PlayerState gear persists across respawn"),I->CreatureHitReduction(),.25f);
    TestEqual(TEXT("Respawn derives protection once"),P->TakeDamage(20,FDamageEvent(),nullptr,Creature),15.f);
    auto* Bad=NewObject<UPFItemCatalog>();auto& Guard=*Bad->Items.FindByPredicate([](const auto& D){return D.Id==TEXT("Item_WovenGuard");});
    Guard.CreatureHitReduction=std::numeric_limits<float>::quiet_NaN();TestNull(TEXT("NaN protection refused"),Bad->Find(Guard.Id));Guard.CreatureHitReduction=.51f;TestNull(TEXT("Over-cap protection refused"),Bad->Find(Guard.Id));
    Guard.CreatureHitReduction=.25f;Guard.StackLimit=2;TestNull(TEXT("Stackable protection refused"),Bad->Find(Guard.Id));
    auto& Wood=*Bad->Items.FindByPredicate([](const auto& D){return D.Id==TEXT("Item_Wood");});Wood.CreatureHitReduction=.25f;TestNull(TEXT("Non-protection cannot confer reduction"),Bad->Find(Wood.Id));
    AddInfo(TEXT("[PrimalAgentTools] Real protection craft/damage/hazard distinction, nonstacking/removal/respawn/V1 restore and authority checked."));F.ForwardErrorMessages(this);return true;
}
#endif
