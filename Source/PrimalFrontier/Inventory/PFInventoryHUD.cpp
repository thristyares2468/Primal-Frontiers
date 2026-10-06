// PFInventoryHUD.cpp — see PFInventoryHUD.h.

#include "Inventory/PFInventoryHUD.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

void UPFInventoryHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    // 640x540 dark panel anchored to the top-right corner with one wrapped text block.
    auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>(); WidgetTree->RootWidget=Canvas;
    Panel=WidgetTree->ConstructWidget<UBorder>(); auto* Placement=Canvas->AddChildToCanvas(Panel);
    Placement->SetAnchors(FAnchors(1,0)); Placement->SetAlignment(FVector2D(1,0));
    Placement->SetPosition(FVector2D(-20,20)); Placement->SetSize(FVector2D(640,540));
    Panel->SetBrushColor(FLinearColor(0.02f,0.02f,0.02f,0.9f)); Panel->SetPadding(FMargin(12));
    Text=WidgetTree->ConstructWidget<UTextBlock>();
    FSlateFontInfo Font=Text->GetFont();
    Font.Size=22;
    Text->SetFont(Font);
    Text->SetAutoWrapText(true);
    Panel->SetContent(Text);
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPFInventoryHUD::NativeTick(const FGeometry& Geometry,float Delta)
{
    Super::NativeTick(Geometry,Delta);
    const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());
    if(!PC){return;}
    Panel->SetVisibility(PC->IsInventoryOpen() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    if(!PC->IsInventoryOpen()){return;}
    // The inventory replicates with the PlayerState; it can be briefly missing after joining.
    const auto* I=PC->GetInventory();
    if(!I){Text->SetText(FText::FromString(TEXT("Waiting for inventory...")));return;}

    // Header with capacity and controls.
    FString Lines=FString::Printf(TEXT("INVENTORY   %d/%d slots   %.1f/%.1f kg\nTab / View close | Up/Down select\nKeys: X split | G drop | Q eat\nPad: D-left split | D-right drop | X eat\n\n"),I->GetStacks().Num(),I->SlotLimit,I->GetWeight(),I->WeightLimit);

    // One row per stack: ">" marks the selection; perishables show seconds remaining.
    const int32 Selected=FMath::Clamp(PC->GetSelectedInventoryIndex(),0,I->GetStacks().Num()-1);
    int32 Index=0;
    for(const auto& S:I->GetStacks())
    {
        const auto* D=I->Definition(S.ItemId);
        const FString Name=D ? D->DisplayName.ToString() : S.ItemId.ToString();
        const FString Fresh=S.ExpiresAt>0 ? FString::Printf(TEXT(" [%ds fresh]"),FMath::Max(0,FMath::CeilToInt(S.ExpiresAt-UPFInventoryComponent::ServerTime(GetWorld())))) : TEXT("");
        Lines+=FString::Printf(TEXT("%s %s x%d%s\n"),Index++==Selected ? TEXT(">") : TEXT(" "),*Name,S.Quantity,*Fresh);
    }
    if(I->GetStacks().IsEmpty()){Lines+=TEXT("Empty - find world pickups and press E\n");}
    Lines+=TEXT("\n")+PC->GetInventoryMessage();
    Text->SetText(FText::FromString(Lines));
}
