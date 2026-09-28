#include "Crafting/PFCraftingHUD.h"
#include "Crafting/PFCraftingComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
void UPFCraftingHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Canvas;
    Panel=WidgetTree->ConstructWidget<UBorder>();auto* Placement=Canvas->AddChildToCanvas(Panel);
    Placement->SetPosition(FVector2D(20,20));Placement->SetSize(FVector2D(670,690));
    Panel->SetBrushColor(FLinearColor(0.02f,0.02f,0.02f,0.95f));Panel->SetPadding(FMargin(12));
    Text=WidgetTree->ConstructWidget<UTextBlock>();auto Font=Text->GetFont();Font.Size=22;Text->SetFont(Font);Text->SetAutoWrapText(true);Panel->SetContent(Text);
    SetVisibility(ESlateVisibility::HitTestInvisible);
}
void UPFCraftingHUD::NativeTick(const FGeometry& Geometry,float Delta)
{
    Super::NativeTick(Geometry,Delta);const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());if(!PC){return;}
    Panel->SetVisibility(PC->IsCraftingOpen()?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);if(!PC->IsCraftingOpen()){return;}
    const auto* C=PC->GetCrafting();const auto* I=PC->GetInventory();if(!C || !C->Catalog || !I){Text->SetText(FText::FromString(TEXT("Crafting unavailable")));return;}
    FString Lines=TEXT("CRAFTING  C close | R cancel\nKeep ingredients in your bag until complete.\n\n");
    const FName Ids[]={TEXT("Recipe_Tool"),TEXT("Recipe_Cook"),TEXT("Recipe_Dry")};
    for(int32 N=0;N<3;++N)
    {
        const auto* D=C->Catalog->Recipe(Ids[N],I->Catalog);if(!D){continue;}
        Lines+=FString::Printf(TEXT("%d: %s (%.0fs)\n"),N+1,*D->DisplayName.ToString(),D->Duration);
        for(const auto& Ingredient:D->Ingredients)
        {const auto* Item=I->Definition(Ingredient.ItemId);Lines+=FString::Printf(TEXT("  %s %d/%d\n"),*Item->DisplayName.ToString(),I->Count(Ingredient.ItemId),Ingredient.Quantity);}
    }
    if(!C->ActiveRecipe.IsNone()){Lines+=FString::Printf(TEXT("\nWorking: %.1fs remaining\n"),FMath::Max(0.0,C->FinishAt-UPFInventoryComponent::ServerTime(GetWorld())));}
    Lines+=TEXT("\n")+C->Feedback;Text->SetText(FText::FromString(Lines));
}
