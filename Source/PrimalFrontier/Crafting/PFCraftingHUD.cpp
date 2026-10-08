// PFCraftingHUD.cpp — see PFCraftingHUD.h.

#include "Crafting/PFCraftingHUD.h"
#include "Crafting/PFCraftingComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "UI/PFReadOnlyOverlay.h"
#include "Settings/PFGameUserSettings.h"

void UPFCraftingHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    UBorder* P;UTextBlock* H;UTextBlock* B;UTextBlock* R;
    PFReadOnlyOverlay::Build(WidgetTree,TEXT("PF_CraftingPanel"),P,H,B,R);
    Panel=P;Heading=H;Text=B;Result=R;
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPFCraftingHUD::NativeTick(const FGeometry& Geometry,float Delta)
{
    Super::NativeTick(Geometry,Delta);
    const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());
    const bool bOpen=PC && PC->IsCraftingOpen() && !PC->IsPauseMenuOpen();
    Panel->SetVisibility(bOpen?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);
    Refresh+=Delta;
    if(!bOpen){bWasOpen=false;return;}
    if(bWasOpen && Refresh<0.1f){return;}
    bWasOpen=true;Refresh=0;
    const auto* Settings=UPFGameUserSettings::Get();
    PFReadOnlyOverlay::SetScale(Heading,Text,Result,Settings?Settings->Preferences.HUDScale:1);
    Heading->SetText(FText::FromString(!Settings || Settings->Preferences.bControlHints?
        TEXT("CRAFTING  C / Y close | R / D-left cancel\n1 / X tool | 2 / D-up cook | 3 / D-down dry"):TEXT("CRAFTING")));
    const auto* C=PC->GetCrafting();
    const auto* I=PC->GetInventory();
    if(!C || !C->Catalog || !I){Text->SetText(FText::FromString(TEXT("Crafting unavailable")));Result->SetText(FText::GetEmpty());return;}

    FString Lines;
    // Fixed list matching hotkeys 1/2/3 in the controller.
    const FName Ids[]={TEXT("Recipe_Tool"),TEXT("Recipe_Cook"),TEXT("Recipe_Dry")};
    for(int32 N=0;N<3;++N)
    {
        const auto* D=C->Catalog->Recipe(Ids[N],I->Catalog);
        if(!D){continue;}
        Lines+=FString::Printf(TEXT("%d: %s (%.0fs)\n"),N+1,*D->DisplayName.ToString(),D->Duration);
        // "have/need" per ingredient (Recipe() already guaranteed each item exists).
        TArray<FString> Ingredients;
        for(const auto& Ingredient:D->Ingredients)
        {
            const auto* Item=I->Definition(Ingredient.ItemId);
            Ingredients.Add(FString::Printf(TEXT("%s %d/%d"),*Item->DisplayName.ToString(),I->Count(Ingredient.ItemId),Ingredient.Quantity));
        }
        Lines+=FString::Join(Ingredients,TEXT(" | "))+TEXT("\n\n");
    }
    Text->SetText(FText::FromString(Lines));
    FString Status=C->ActiveRecipe.IsNone()?TEXT("Job: idle"):FString::Printf(TEXT("Job: %.1fs remaining"),FMath::Max(0.0,C->FinishAt-UPFInventoryComponent::ServerTime(GetWorld())));
    Status+=TEXT("\nLast craft status: ")+(C->Feedback.IsEmpty()?FString(TEXT("No server result yet")):C->Feedback);
    const FString Request=PC->GetInventoryMessage();
    if(Request.StartsWith(TEXT("Craft "))){Status+=TEXT("\nLast request: ")+Request;}
    Result->SetText(FText::FromString(Status));
}
