#include "Survival/PFSurvivalHUD.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Pawn.h"

void UPFSurvivalHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    // A Blueprint child can replace this placeholder layout and use PresentVitals.
    if (WidgetTree->RootWidget) { return; }
    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;
    UTextBlock* AimMarker=WidgetTree->ConstructWidget<UTextBlock>();
    AimMarker->SetText(FText::FromString(TEXT("+")));AimMarker->SetShadowColorAndOpacity(FLinearColor::Black);AimMarker->SetShadowOffset(FVector2D(1,1));
    UCanvasPanelSlot* AimPlacement=Canvas->AddChildToCanvas(AimMarker);
    AimPlacement->SetAnchors(FAnchors(0.5f,0.5f));AimPlacement->SetAlignment(FVector2D(0.5f,0.5f));AimPlacement->SetAutoSize(true);
    AimPlacement->SetPosition(FVector2D::ZeroVector);
    InteractionLabel=WidgetTree->ConstructWidget<UTextBlock>();InteractionLabel->SetAutoWrapText(true);InteractionLabel->SetJustification(ETextJustify::Center);InteractionLabel->SetShadowColorAndOpacity(FLinearColor::Black);InteractionLabel->SetShadowOffset(FVector2D(1,1));
    auto InteractionFont=InteractionLabel->GetFont();InteractionFont.Size=26;InteractionLabel->SetFont(InteractionFont);
    auto* InteractionPlacement=Canvas->AddChildToCanvas(InteractionLabel);InteractionPlacement->SetAnchors(FAnchors(0.5f,0.62f));InteractionPlacement->SetAlignment(FVector2D(0.5f,0));InteractionPlacement->SetSize(FVector2D(800,140));
    UVerticalBox* Panel = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(Panel);
    PanelSlot->SetAnchors(FAnchors(0.f, 1.f));
    PanelSlot->SetAlignment(FVector2D(0.f, 1.f));
    PanelSlot->SetPosition(FVector2D(32.f, -32.f));
    PanelSlot->SetSize(FVector2D(440.f, 215.f));
    HealthLabel = WidgetTree->ConstructWidget<UTextBlock>(); Panel->AddChild(HealthLabel);
    HealthBar = WidgetTree->ConstructWidget<UProgressBar>(); Panel->AddChild(HealthBar);
    HealthBar->SetFillColorAndOpacity(FLinearColor(0.8f, 0.15f, 0.1f));
    StaminaLabel = WidgetTree->ConstructWidget<UTextBlock>(); Panel->AddChild(StaminaLabel);
    StaminaBar = WidgetTree->ConstructWidget<UProgressBar>(); Panel->AddChild(StaminaBar);
    StaminaBar->SetFillColorAndOpacity(FLinearColor(0.2f, 0.8f, 0.35f));
    NeedsLabel = WidgetTree->ConstructWidget<UTextBlock>(); Panel->AddChild(NeedsLabel);
    HungerBar = WidgetTree->ConstructWidget<UProgressBar>(); Panel->AddChild(HungerBar);
    HungerBar->SetFillColorAndOpacity(FLinearColor(0.8f,0.65f,0.25f));
    ThirstBar = WidgetTree->ConstructWidget<UProgressBar>(); Panel->AddChild(ThirstBar);
    ThirstBar->SetFillColorAndOpacity(FLinearColor(0.2f,0.55f,0.9f));
    StateLabel = WidgetTree->ConstructWidget<UTextBlock>(); Panel->AddChild(StateLabel);
    SetVisibility(ESlateVisibility::HitTestInvisible);
}
void UPFSurvivalHUD::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
    Super::NativeTick(Geometry, DeltaTime);
    InteractionRefresh+=DeltaTime;
    if(InteractionLabel && InteractionRefresh>=0.1f)
    {
        InteractionRefresh=0;if(const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer()))
        {InteractionLabel->SetText(FText::FromString(PC->IsPauseMenuOpen()?FString():PC->InteractionPrompt()+TEXT("\n")+PC->RecentInteractionMessage()));}
    }
    const APawn* Pawn = GetOwningPlayerPawn();
    const UPFPlayerSurvivalComponent* Survival = Pawn ? Pawn->FindComponentByClass<UPFPlayerSurvivalComponent>() : nullptr;
    if (!Survival) { if (StateLabel) { StateLabel->SetText(FText::FromString(TEXT("Waiting for survivor..."))); } return; }
    const FPFPlayerVitals V = Survival->GetVitals();
    if (HealthLabel)
    {
        HealthLabel->SetText(FText::FromString(FString::Printf(TEXT("HEALTH   %.0f / %.0f"), V.Health, V.MaxHealth)));
        StaminaLabel->SetText(FText::FromString(FString::Printf(TEXT("STAMINA   %.0f / %.0f"), V.Stamina, V.MaxStamina)));
        HealthBar->SetPercent(V.Health / FMath::Max(1.f, V.MaxHealth));
        StaminaBar->SetPercent(V.Stamina / FMath::Max(1.f, V.MaxStamina));
        NeedsLabel->SetText(FText::FromString(FString::Printf(TEXT("FOOD %.0f | WATER %.0f | EXPOSURE %.0f%%"),V.Hunger,V.Thirst,V.Exposure*100)));
        HungerBar->SetPercent(V.Hunger/100.f); ThirstBar->SetPercent(V.Thirst/100.f);
        StateLabel->SetText(FText::FromString(Survival->IsDead() ? TEXT("You died - respawning...") :
            (V.Hunger <= 0 || V.Thirst <= 0 ? TEXT("STARVING / DEHYDRATED - find a ration!") :
            (V.Exposure > 0 ? TEXT("DANGER - leave exposure zone!") : TEXT("WASD | E Interact | Tab Bag | C Craft | B Build | P Menu")))));
    }
    PresentVitals(V.Health, V.MaxHealth, V.Stamina, V.MaxStamina, Survival->IsDead());
    PresentNeeds(V.Hunger,V.Thirst,V.Exposure);
}
