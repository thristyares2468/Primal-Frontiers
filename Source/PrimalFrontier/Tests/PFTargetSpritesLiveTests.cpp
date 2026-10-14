// Real HUD sprite selection against transient actors; never edits the authored map.
#if WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivalHUD.h"
#include "Survival/PFSurvivorCharacter.h"
#include "UI/PFItemPicture.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemPickup.h"
#include "Inventory/PFItemCatalog.h"
#include "Crafting/PFResourceNode.h"
#include "Crafting/PFCraftingCatalog.h"
#include "Creatures/PFCreature.h"
#include "PFAssetPaths.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "HAL/PlatformTime.h"
#include "HAL/FileManager.h"
#include "UnrealClient.h"

namespace
{
class FTargetSpritesExercise final : public IAutomationLatentCommand
{
public:
    FTargetSpritesExercise(FAutomationTestBase* InTest,FString InDirectory):Test(InTest),Directory(MoveTemp(InDirectory)){}
    bool Update() override
    {
        const double Now=FPlatformTime::Seconds();if(Now-Started>70){Test->AddError(TEXT("Target sprite timeout"));return true;}if(Now<Until){return false;}
        if(!PC.IsValid())
        {
            for(const auto& Context:GEngine->GetWorldContexts()){if(auto* World=Context.World();World && World->IsGameWorld()){auto* Candidate=Cast<APFSurvivalPlayerController>(World->GetFirstPlayerController());if(Candidate && Candidate->GetPawn() && Candidate->SurvivalHUD && Candidate->GetInventory()){PC=Candidate;break;}}}
            if(!PC.IsValid()){return false;}
            if(!PC->HasAuthority() || !PC->GetInventory()->GetStacks().IsEmpty()){Test->AddError(TEXT("Refusing non-fresh standalone sprite fixture"));return true;}
            auto* Pawn=Cast<APFSurvivorCharacter>(PC->GetPawn());if(!Pawn){return true;}Pawn->GetCharacterMovement()->DisableMovement();Pawn->SetActorLocation(FVector(-9000,-9000,5000));PC->SetControlRotation(FRotator::ZeroRotator);
            Picture=Cast<UPFItemPicture>(PC->SurvivalHUD->WidgetTree->FindWidget(TEXT("PF_TargetPicture")));if(!Test->TestNotNull(TEXT("Actual aimed-target picture exists"),Picture.Get())){return true;}
            for(const auto& Item:PC->GetInventory()->Catalog->Items){ItemIds.Add(Item.Id);}
            Recipes=LoadObject<UPFCraftingCatalog>(nullptr,PFAssetPaths::CraftingCatalog);Creatures=LoadObject<UPFCreatureCatalog>(nullptr,PFAssetPaths::CreatureCatalog);
            if(!Recipes || !Creatures){Test->AddError(TEXT("Actual catalogs missing"));return true;}
            Until=Now+.4;return false;
        }
        if(bAwaiting)
        {
            Test->TestEqual(TEXT("HUD cue identifies the actual looked-at definition"),Picture->GetItemId(),Expected);
            Test->TestTrue(TEXT("Aimed sprite is visible"),Picture->IsVisible());const auto* Image=Cast<UImage>(Picture->WidgetTree->RootWidget);
            Test->TestTrue(TEXT("Actual aimed sprite texture is loaded"),Image && Image->GetBrush().GetResourceObject()!=nullptr);
            if(Index==0 || Index==ItemIds.Num() || Index>=ItemIds.Num()+Recipes->Resources.Num())
            {FScreenshotRequest::RequestScreenshot(Directory/(Expected.ToString()+TEXT(".png")),true,false);}
            Target->Destroy();Target.Reset();bAwaiting=false;++Index;Until=Now+.3;return false;
        }
        const int32 Total=ItemIds.Num()+Recipes->Resources.Num()+Creatures->Creatures.Num();
        if(Index>=Total)
        {
            Test->TestFalse(TEXT("Destroyed target clears the cue"),Picture->IsVisible());
            Test->TestEqual(TEXT("Read-only cue never grants items"),PC->GetInventory()->GetStacks().Num(),0);
            for(const auto& D:Creatures->Creatures){Test->TestTrue(TEXT("Creature badge screenshot exists"),IFileManager::Get().FileSize(*(Directory/(D.Id.ToString()+TEXT(".png"))))>0);}
            Test->AddInfo(FString::Printf(TEXT("[PrimalAgentTools] Target sprites PASS: %d item pickups, %d resource yields, %d creature classifications; transient actors only."),ItemIds.Num(),Recipes->Resources.Num(),Creatures->Creatures.Num()));return true;
        }
        auto* W=PC->GetWorld();FVector Eye;FRotator View;PC->GetPawn()->GetActorEyesViewPoint(Eye,View);const FTransform At(View,Eye+View.Vector()*180);
        if(Index<ItemIds.Num())
        {
            Expected=ItemIds[Index];const auto* Definition=PC->GetInventory()->Definition(Expected);
            auto* Pickup=W->SpawnActorDeferred<APFItemPickup>(APFItemPickup::StaticClass(),At,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
            Pickup->Initialize(Expected,1,Definition->ShelfLifeSeconds>0?UPFInventoryComponent::ServerTime(W)+Definition->ShelfLifeSeconds:0);Pickup->FinishSpawning(At);Target=Pickup;
        }
        else if(Index<ItemIds.Num()+Recipes->Resources.Num())
        {
            const auto& Definition=Recipes->Resources[Index-ItemIds.Num()];Expected=Definition.YieldItem;
            auto* Node=W->SpawnActorDeferred<APFResourceNode>(APFResourceNode::StaticClass(),At,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);Node->ResourceId=Definition.Id;Node->FinishSpawning(At);Target=Node;
        }
        else
        {
            Expected=Creatures->Creatures[Index-ItemIds.Num()-Recipes->Resources.Num()].Id;
            auto* Creature=W->SpawnActorDeferred<APFCreature>(APFCreature::StaticClass(),At,nullptr,nullptr,ESpawnActorCollisionHandlingMethod::AlwaysSpawn);Creature->CreatureId=Expected;Creature->FinishSpawning(At);Creature->SetActorTickEnabled(false);Creature->GetCharacterMovement()->DisableMovement();Target=Creature;
        }
        bAwaiting=true;Until=Now+.35;return false;
    }
private:
    FAutomationTestBase* Test;FString Directory;TWeakObjectPtr<APFSurvivalPlayerController> PC;TWeakObjectPtr<UPFItemPicture> Picture;TWeakObjectPtr<AActor> Target;
    UPFCraftingCatalog* Recipes=nullptr;UPFCreatureCatalog* Creatures=nullptr;TArray<FName> ItemIds;FName Expected;int32 Index=0;bool bAwaiting=false;double Until=0,Started=FPlatformTime::Seconds();
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPFTargetSpritesTest,"PF.UI.TargetSpritesLive",EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)
bool FPFTargetSpritesTest::RunTest(const FString&)
{
    if(!FParse::Param(FCommandLine::Get(),TEXT("PFRunControlsUITest")) || FParse::Param(FCommandLine::Get(),TEXT("nullrhi"))){AddError(TEXT("Requires isolated rendered UI launcher"));return false;}
    FString Label;FParse::Value(FCommandLine::Get(),TEXT("PFControlsEvidence="),Label);if(Label.IsEmpty() || Label.Len()>48){return false;}
    for(TCHAR C:Label){if(!FChar::IsAlnum(C) && C!=TEXT('_')){return false;}}
    const FString Directory=FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()/TEXT("AutomationReports/ControlsUI")/Label);if(IFileManager::Get().DirectoryExists(*Directory)){AddError(TEXT("Refusing reused sprite evidence"));return false;}IFileManager::Get().MakeDirectory(*Directory,true);
    ADD_LATENT_AUTOMATION_COMMAND(FTargetSpritesExercise(this,Directory));return true;
}
#endif
