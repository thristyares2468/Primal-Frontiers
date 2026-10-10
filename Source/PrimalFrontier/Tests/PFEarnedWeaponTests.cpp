#if WITH_DEV_AUTOMATION_TESTS
#include "Progression/PFProgressionComponent.h"
#include "Progression/PFProgressionCatalog.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemPickup.h"
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFResourceNode.h"
#include "Survival/PFSurvivalGameMode.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFInteraction.h"
#include "Persistence/PFWorldPersistence.h"
#include "Components/BoxComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Tests/AutomationCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFEarnedWeaponTest,"PF.Progression.EarnedWeapon",EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPFEarnedWeaponTest::RunTest(const FString&)
{
    AddExpectedMessage(TEXT("GetSocketInfoByName.*No SkeletalMesh for Component"),EAutomationExpectedMessageFlags::Contains,0);
    FTestWorldWrapper F;if(!F.CreateTestWorld(EWorldType::Game)){return false;}auto* W=F.GetTestWorld();
    W->SetGameMode(FURL(nullptr,TEXT("/Engine/Maps/Entry?game=/Script/PrimalFrontier.PFSurvivalGameMode"),TRAVEL_Absolute));
    auto* Floor=W->SpawnActor<AActor>();auto* Box=NewObject<UBoxComponent>(Floor);Floor->SetRootComponent(Box);Box->SetBoxExtent(FVector(4000,4000,25));Box->SetCollisionProfileName(TEXT("BlockAll"));Box->RegisterComponent();Floor->SetActorLocation(FVector(0,0,-25));
    W->SpawnActor<APlayerStart>(FVector(-500,0,120),FRotator::ZeroRotator);if(!F.BeginPlayInTestWorld()){return false;}
    auto* PC=W->SpawnActor<APFSurvivalPlayerController>();W->GetAuthGameMode<APFSurvivalGameMode>()->RestartPlayer(PC);
    auto* P=Cast<APFSurvivorCharacter>(PC->GetPawn());auto* PS=PC->GetPlayerState<APFInventoryPlayerState>();if(!P || !PS || !PS->Progression){return false;}
    auto* G=PS->Progression.Get();auto* I=PS->Inventory.Get();auto* C=PS->Crafting.Get();FString Error;
    P->GetCharacterMovement()->DisableMovement();P->Survival->HungerDrainPerSecond=0;P->Survival->ThirstDrainPerSecond=0;PC->SetControlRotation(FRotator::ZeroRotator);
    W->GetSubsystem<UPFWorldPersistence>()->ActiveSlot.Reset();int32 Dropped=0;
    auto Advance=[&](int32 Ticks=6){for(int32 N=0;N<Ticks;++N){F.TickTestWorld(0.1f);}};
    auto Drop=[&](FName Id,int32 Amount)
    {
        const auto* S=I->GetStacks().FindByPredicate([&](const auto& X){return X.ItemId==Id;});if(!S){return false;}
        const auto Before=G->GetRecord();const double Expiry=S->ExpiresAt;
        auto* Pickup=I->Drop(S->StackId,Amount,P);if(!TestNotNull(TEXT("Normal unused resource drop frees capacity"),Pickup)){return false;}
        TestTrue(TEXT("Dropped finite batch keeps quantity and freshness without XP"),Pickup->GetContents().ItemId==Id && Pickup->GetContents().Quantity==Amount && Pickup->GetContents().ExpiresAt==Expiry && G->GetExperience()==Before.Experience && G->GetRecord().GatherWindows==Before.GatherWindows);
        // Keep actual dropped pickups out of subsequent controlled interaction traces.
        Pickup->SetActorLocation(FVector(-500,600+(++Dropped)*100,100));return true;
    };
    auto Gather=[&](FName Id)
    {
        Advance();FVector Eye;FRotator Look;P->GetActorEyesViewPoint(Eye,Look);const FTransform At(FRotator::ZeroRotator,Eye+Look.Vector()*150);
        auto* Node=W->SpawnActorDeferred<APFResourceNode>(APFResourceNode::StaticClass(),At,P,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);if(!Node){return false;}
        Node->ResourceId=Id;Node->FinishSpawning(At);ON_SCOPE_EXIT{Node->Destroy();};
        return TestTrue(TEXT("Real owned gather trace"),PFInteraction::FindTarget(P)==Node) && TestTrue(TEXT("Actual finite gathering"),Node->Gather(P));
    };
    auto Craft=[&](FName Id,int32 XP)
    {
        const auto* D=C->Catalog->Recipe(Id,I->Catalog);if(!D){return false;}const int32 Before=I->Count(D->Output);
        if(!TestTrue(TEXT("Actual normal timed job starts"),C->Start(Id,P))){return false;}
        Advance(FMath::CeilToInt(D->Duration*10)+6);
        return TestTrue(TEXT("Actual completion converts once with exact earned XP"),C->ActiveRecipe.IsNone() && C->Feedback==TEXT("Completed") && I->Count(D->Output)==Before+1 && G->GetExperience()==XP);
    };
    TestTrue(TEXT("Fresh zero XP empty normal bag"),G->GetExperience()==0 && I->GetStacks().IsEmpty() && I->SlotLimit==8 && I->WeightLimit==30);
    const auto* D=GetDefault<UPFProgressionCatalog>()->Find(TEXT("Tech_FieldWeapons"));
    if(!TestTrue(TEXT("Original level-three graph and unchanged existing recipe"),D && D->MinimumLevel==3 && D->PointCost==3 && D->Prerequisites==TArray<FName>{TEXT("Tech_FieldTools")} && D->Recipes==TArray<FName>{TEXT("Recipe_BoundClub")})){return false;}
    FPFProgressionRecord Boundary;Boundary.Experience=250;
    TestFalse(TEXT("Pure complete purchase rejects missing parent at sufficient level"),FPFProgressionTransactions::Purchase(Boundary,D->Id,*GetDefault<UPFProgressionCatalog>(),*C->Catalog,*I->Catalog,Error));
    TestTrue(TEXT("Missing parent has no partial spend"),Boundary.Knowledge.IsEmpty() && Boundary.Experience==250);
    for(FName Node:{FName(TEXT("Node_Wood")),FName(TEXT("Node_Stone")),FName(TEXT("Node_Fibre")),FName(TEXT("Node_Water"))}){for(int32 N=0;N<5;++N){if(!Gather(Node)){return false;}}}
    for(int32 N=0;N<5;++N)
    {
        if(!Gather(TEXT("Node_Food"))){return false;}
        // Retain the first three edible pieces; later normal drops avoid manufacturing extra bag slots.
        const int32 Excess=I->Count(TEXT("Item_Food"))-3;
        if(Excess>0)
        {
            int32 Left=Excess;
            while(Left>0){const auto* S=I->GetStacks().FindByPredicate([](const auto& X){return X.ItemId==TEXT("Item_Food");});if(!S){return false;}const int32 Amount=FMath::Min(Left,S->Quantity);if(!Drop(TEXT("Item_Food"),Amount)){return false;}Left-=Amount;}
        }
    }
    TestTrue(TEXT("Twenty-five real gathers earn125XP, five exhausted category budgets"),G->GetExperience()==125 && G->GetLevel()==2 && G->GetAvailablePoints()==3 && G->GetRecord().GatherWindows.Num()==5);
    if(!Drop(TEXT("Item_Water"),10) || !G->RequestKnowledge(TEXT("Tech_FieldTools"),P,Error)){return false;}
    Advance();TestFalse(TEXT("Second knowledge requires actual level three"),G->RequestKnowledge(TEXT("Tech_FieldWeapons"),P,Error));
    TestTrue(TEXT("Early refusal spends no points"),G->GetAvailablePoints()==1 && G->GetRecord().Knowledge==TArray<FName>{TEXT("Tech_FieldTools")});
    if(!Craft(TEXT("Recipe_Tool"),145) || !Craft(TEXT("Recipe_Cook"),165) || !Craft(TEXT("Recipe_Dry"),185) || !Craft(TEXT("Recipe_Cord"),205) || !Craft(TEXT("Recipe_Club"),225)){return false;}
    if(!Drop(TEXT("Item_CookedFood"),1) || !Drop(TEXT("Item_DriedFood"),1)){return false;}
    for(int32 N=0;N<4;++N){if(!Gather(TEXT("Node_Fibre"))){return false;}} // Normal carried tool yields four; XP budget remains exhausted.
    if(!Gather(TEXT("Node_Wood")) || !Craft(TEXT("Recipe_Cord"),225) || !Craft(TEXT("Recipe_WovenGuard"),245) || !Craft(TEXT("Recipe_Cord"),245) || !Craft(TEXT("Recipe_BoundTool"),265)){return false;}
    TestTrue(TEXT("Earned seventh unique craft crosses level three"),G->GetLevel()==3 && G->GetAvailablePoints()==4 && G->GetRecord().CreditedCrafts.Num()==7);
    if(!TestTrue(TEXT("Actual owned second purchase uses earned points"),G->RequestKnowledge(TEXT("Tech_FieldWeapons"),P,Error))){AddError(Error);return false;}
    TestTrue(TEXT("Exact parent/child graph costs five of six earned points"),G->GetAvailablePoints()==1 && G->GetExperience()==265 && G->GetRecord().Knowledge==TArray<FName>{TEXT("Tech_FieldTools"),TEXT("Tech_FieldWeapons")});
    Advance();TestFalse(TEXT("Duplicate second purchase refuses without spend"),G->RequestKnowledge(TEXT("Tech_FieldWeapons"),P,Error));
    PS->SetRole(ROLE_AutonomousProxy);TestFalse(TEXT("Client direct graph mutation refuses"),G->RequestKnowledge(TEXT("Tech_FieldWeapons"),P,Error));PS->SetRole(ROLE_Authority);
    if(!Gather(TEXT("Node_Wood")) || !Craft(TEXT("Recipe_Cord"),265)){return false;}
    // The earned bound tool supplies six finite wood; stone stock remains eight.
    TestTrue(TEXT("Normal bound-club job starts with earned access"),C->Start(TEXT("Recipe_BoundClub"),P));TestFalse(TEXT("Duplicate job cannot duplicate output"),C->Start(TEXT("Recipe_BoundClub"),P));
    TestTrue(TEXT("Cancel keeps earned inputs"),C->Cancel());TestTrue(TEXT("Cancelled job does not award XP or weapon"),G->GetExperience()==265 && I->Count(TEXT("Item_Club"))==1 && I->Count(TEXT("Item_Cord"))==1 && I->Count(TEXT("Item_BoundClub"))==0);
    if(!Craft(TEXT("Recipe_BoundClub"),285)){return false;}
    TestTrue(TEXT("Earned stronger weapon keeps independent tool/guard and exact spent points"),I->Count(TEXT("Item_BoundClub"))==1 && I->Count(TEXT("Item_Club"))==0 && I->Count(TEXT("Item_Cord"))==0 && I->Count(TEXT("Item_Wood"))==5 && I->Count(TEXT("Item_Stone"))==6 && I->Count(TEXT("Item_Fibre"))==2 && I->GatheringHits()==3 && I->MeleeDamage()==60 && I->CreatureHitReduction()==0.25f && G->GetAvailablePoints()==1 && G->GetRecord().CreditedCrafts.Num()==8);
    F.ForwardErrorMessages(this);AddInfo(TEXT("[PrimalAgentTools] EarnedWeapon: real default-limit gathering/normal unused drops, eight first crafts285XP, level3/FieldTools→FieldWeapons/five spent points/one remaining, default8s sixty-damage club conversion/cancel/client/duplicate refusal. No item/XP grants, progression seeds, shortened jobs or private files; pure250XP missing-parent boundary is not injected into the player."));return true;
}
#endif
