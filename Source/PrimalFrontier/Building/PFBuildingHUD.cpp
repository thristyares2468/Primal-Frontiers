#include "Building/PFBuildingHUD.h"
#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildPiece.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Inventory/PFInventoryComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
void UPFBuildingHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();auto* Canvas=WidgetTree->ConstructWidget<UCanvasPanel>();WidgetTree->RootWidget=Canvas;
    Panel=WidgetTree->ConstructWidget<UBorder>();auto* Placement=Canvas->AddChildToCanvas(Panel);Placement->SetPosition(FVector2D(20,20));Placement->SetSize(FVector2D(640,650));
    Panel->SetBrushColor(FLinearColor(0.02f,0.02f,0.02f,0.9f));Panel->SetPadding(FMargin(12));
    Text=WidgetTree->ConstructWidget<UTextBlock>();auto Font=Text->GetFont();Font.Size=22;Text->SetFont(Font);Text->SetAutoWrapText(true);Panel->SetContent(Text);SetVisibility(ESlateVisibility::HitTestInvisible);
}
void UPFBuildingHUD::NativeTick(const FGeometry& Geometry,float Delta)
{
    Super::NativeTick(Geometry,Delta);auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());auto* B=PC?PC->Building.Get():nullptr;if(!B){return;}
    Panel->SetVisibility(B->bBuildMode?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);if(!B->bBuildMode){return;}
    FString Lines=TEXT("BUILD  B / RB close | N / D-up next\nT / D-down rotate | Click / RT place\nH / LB demolish | J damage (debug)\nE / X door / open storage\nU / D-left store | O / D-right take first\n\n");
    const auto* D=B->Catalog?B->Catalog->Find(B->SelectedId()):nullptr;
    if(D){Lines+=FString::Printf(TEXT("%s | %d wood | %d degrees\n%s\n"),*D->Name.ToString(),D->WoodCost,B->Rotation*90,*B->PreviewMessage);}
    if(auto* Target=B->TracedPiece()){Lines+=FString::Printf(TEXT("Target: %s | Health %.0f\n"),*Target->DefinitionId.ToString(),Target->Health);}
    if(IsValid(B->OpenStorage))
    {
        Lines+=TEXT("\nSTORAGE (no preservation)\n");const auto* I=B->OpenStorage->Storage.Get();
        for(const auto& S:I->GetStacks()){Lines+=FString::Printf(TEXT("%s x%d"),*S.ItemId.ToString(),S.Quantity);if(S.ExpiresAt>0){Lines+=FString::Printf(TEXT(" [%.0fs]"),FMath::Max(0.0,S.ExpiresAt-UPFInventoryComponent::ServerTime(GetWorld())));}Lines+=TEXT("\n");}
        if(I->GetStacks().IsEmpty()){Lines+=TEXT("Empty\n");}
    }
    Lines+=TEXT("\n")+B->Feedback;Text->SetText(FText::FromString(Lines));
}
