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
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFWeaponProgressionTest,"PF.Crafting.WeaponProgression",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFWeaponProgressionTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}
    auto* W=F.GetTestWorld();W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    W->SpawnActor<APlayerStart>(FVector(0,0,150),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* I=PC->GetInventory();auto* C=PC->GetCrafting();
    if(!P || !I || !C){return false;}P->GetCharacterMovement()->DisableMovement();
    TestTrue(TEXT("Fresh player receives no free weapons"),I->GetStacks().IsEmpty());
    if(!TestNotNull(TEXT("Real loaded wooden club"),I->Definition(TEXT("Item_Club"))) ||
       !TestNotNull(TEXT("Real loaded bound club"),I->Definition(TEXT("Item_BoundClub"))) ||
       !TestNotNull(TEXT("Real loaded wooden club recipe"),C->Catalog->Recipe(TEXT("Recipe_Club"),I->Catalog)) ||
       !TestNotNull(TEXT("Real loaded bound club recipe"),C->Catalog->Recipe(TEXT("Recipe_BoundClub"),I->Catalog))){return false;}
    TestEqual(TEXT("Authored club duration"),C->Catalog->Recipe(TEXT("Recipe_Club"),I->Catalog)->Duration,6.f);
    TestEqual(TEXT("Authored upgrade duration"),C->Catalog->Recipe(TEXT("Recipe_BoundClub"),I->Catalog)->Duration,8.f);
    C->Catalog=DuplicateObject<UPFCraftingCatalog>(C->Catalog,C);for(auto& R:C->Catalog->Recipes){R.Duration=0.5f;}
    auto Advance=[&](){for(int N=0;N<8;++N){F.TickTestWorld(0.1f);}};
    TestFalse(TEXT("Missing ingredients refused"),C->Start(TEXT("Recipe_Club"),P));
    I->Grant(TEXT("Item_Wood"),5);I->Grant(TEXT("Item_Cord"),2);I->Grant(TEXT("Item_Stone"),2);
    TestTrue(TEXT("Club starts"),C->Start(TEXT("Recipe_Club"),P));TestFalse(TEXT("Duplicate refused"),C->Start(TEXT("Recipe_Club"),P));
    TestTrue(TEXT("Cancel accepted"),C->Cancel());Advance();TestEqual(TEXT("Cancel no wood charged"),I->Count(TEXT("Item_Wood")),5);
    TestTrue(TEXT("Capacity probe starts"),C->Start(TEXT("Recipe_Club"),P));I->WeightLimit=.1f;Advance();I->WeightLimit=30;
    TestEqual(TEXT("Failed capacity creates no club"),I->Count(TEXT("Item_Club")),0);TestEqual(TEXT("Failed capacity preserves inputs"),I->Count(TEXT("Item_Cord")),2);
    TestTrue(TEXT("Club restarts"),C->Start(TEXT("Recipe_Club"),P));Advance();
    TestEqual(TEXT("One wooden club"),I->Count(TEXT("Item_Club")),1);TestEqual(TEXT("Exact wood cost"),I->Count(TEXT("Item_Wood")),2);
    TestEqual(TEXT("Exact cord cost"),I->Count(TEXT("Item_Cord")),1);TestEqual(TEXT("Wood club damage"),I->MeleeDamage(),40.f);
    TestEqual(TEXT("Club no gathering bonus"),I->GatheringHits(),1);TestEqual(TEXT("Public held identity"),P->GetHeldMeleeItem(),FName(TEXT("Item_Club")));
    Advance();TestEqual(TEXT("No duplicated completion"),I->Count(TEXT("Item_Club")),1);
    PC->SetControlRotation(FRotator::ZeroRotator);FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);
    FTransform At(FRotator::ZeroRotator,Eye+Look.Vector()*150);
    auto* Creature=W->SpawnActorDeferred<APFCreature>(APFCreature::StaticClass(),At,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    Creature->CreatureId=TEXT("Creature_Prowler");Creature->FinishSpawning(At);Creature->GetCharacterMovement()->DisableMovement();Creature->SetActorTickEnabled(false);
    auto* Needs=P->FindComponentByClass<UPFPlayerSurvivalComponent>();const float Stamina=Needs->GetVitals().Stamina;
    PC->ServerAttackCreature();TestEqual(TEXT("Real server traced club damage"),Creature->Health,60.f);
    TestEqual(TEXT("Swing spends five stamina"),Needs->GetVitals().Stamina,Stamina-5);
    PC->ServerAttackCreature();TestEqual(TEXT("Duplicate cooldown prevents damage"),Creature->Health,60.f);
    TestTrue(TEXT("Upgrade starts"),C->Start(TEXT("Recipe_BoundClub"),P));TestTrue(TEXT("Upgrade cancel"),C->Cancel());
    TestEqual(TEXT("Canceled upgrade retains club"),I->Count(TEXT("Item_Club")),1);
    TestTrue(TEXT("Upgrade restarts"),C->Start(TEXT("Recipe_BoundClub"),P));Advance();
    TestEqual(TEXT("Upgrade consumes old club"),I->Count(TEXT("Item_Club")),0);TestEqual(TEXT("Upgrade consumes all wood"),I->Count(TEXT("Item_Wood")),0);
    TestEqual(TEXT("Upgrade consumes all stone"),I->Count(TEXT("Item_Stone")),0);TestEqual(TEXT("Upgrade consumes all cord"),I->Count(TEXT("Item_Cord")),0);
    TestEqual(TEXT("Exactly one bound club"),I->Count(TEXT("Item_BoundClub")),1);TestEqual(TEXT("Bound club damage"),I->MeleeDamage(),60.f);
    TestEqual(TEXT("Bound club no gathering bonus"),I->GatheringHits(),1);TestEqual(TEXT("Upgraded public identity"),P->GetHeldMeleeItem(),FName(TEXT("Item_BoundClub")));
    const float BeforeSecond=Needs->GetVitals().Stamina;PC->ServerAttackCreature();TestEqual(TEXT("Actual bound swing kills remaining sixty health"),Creature->Health,0.f);
    TestEqual(TEXT("Bound swing spends five stamina"),Needs->GetVitals().Stamina,BeforeSecond-5);
    I->Grant(TEXT("Item_BoundTool"),1);TestEqual(TEXT("Independent best gather tool"),I->GatheringHits(),3);TestEqual(TEXT("Best weapon remains strongest"),I->MeleeDamage(),60.f);
    FPFPlayerSaveData Saved;Saved.PlayerId=FGuid::NewGuid();Saved.Inventory=FPFPlayerSaveAdapter::CaptureInventory(*I);
    FPFPlayerSaveLimits Limits;TArray<uint8> Bytes;FString Error;FPFPlayerSaveData Decoded;
    TestTrue(TEXT("Version1 accepts weapon IDs"),FPFPlayerSaveFormat::Encode(Saved,*I->Catalog,Limits,Bytes,Error));
    TestTrue(TEXT("Version1 decodes weapon IDs"),FPFPlayerSaveFormat::Decode(Bytes,*I->Catalog,Limits,Decoded,Error));
    I->RemoveItem(TEXT("Item_BoundClub"),1);Advance();TestEqual(TEXT("Removal falls back to carried tool"),I->MeleeDamage(),45.f);
    TestTrue(TEXT("Restore weapon bag"),I->RestorePersistence(Decoded.Inventory,0,Error));Advance();TestEqual(TEXT("Restored weapon identity"),P->GetHeldMeleeItem(),FName(TEXT("Item_BoundClub")));
    I->RemoveItem(TEXT("Item_BoundClub"),1);I->RemoveItem(TEXT("Item_BoundTool"),1);Advance();TestEqual(TEXT("Removal returns bare damage"),I->MeleeDamage(),20.f);
    TestEqual(TEXT("No stale public equipped identity"),P->GetHeldMeleeItem(),NAME_None);
    PC->PlayerState->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Client direct craft denied"),C->Start(TEXT("Recipe_Club"),P));
    TestFalse(TEXT("Client direct weapon creation denied"),I->Grant(TEXT("Item_BoundClub"),1));PC->PlayerState->SetRole(ROLE_Authority);
    auto* Bad=NewObject<UPFItemCatalog>();auto& Club=*Bad->Items.FindByPredicate([](const auto& D){return D.Id==TEXT("Item_Club");});
    Club.GatheringHits=1;TestNull(TEXT("Weapon cannot declare gathering benefit"),Bad->Find(Club.Id));Club.GatheringHits=0;
    Club.MeleeDamage=std::numeric_limits<float>::quiet_NaN();TestNull(TEXT("NaN refused"),Bad->Find(Club.Id));Club.MeleeDamage=40;
    Club.StackLimit=2;TestNull(TEXT("Stackable weapon definition refused"),Bad->Find(Club.Id));
    auto& Wood=*Bad->Items.FindByPredicate([](const auto& D){return D.Id==TEXT("Item_Wood");});Wood.MeleeDamage=40;TestNull(TEXT("Resource cannot declare melee power"),Bad->Find(Wood.Id));
    AddInfo(TEXT("[PrimalAgentTools] Weapon catalog, exact costs/cancel/capacity, traced damage/death/stamina/cooldown, authority, carried fallback and V1 restore verified."));
    F.ForwardErrorMessages(this);return true;
}
#endif
