// PFSurvivalHUD.cpp
//
// See PFSurvivalHUD.h. Layout: aim "+" at screen centre, prompt text just below
// centre, and a vitals panel anchored to the bottom-left corner.

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
#include "Settings/PFGameUserSettings.h"

void UPFSurvivalHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    // A Blueprint child can replace this placeholder layout and use PresentVitals.
    if (WidgetTree->RootWidget) { return; }
    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;

    // Centred aim marker (M4): makes the server-derived interaction ray aimable.
    AimMarker=WidgetTree->ConstructWidget<UTextBlock>();
    AimMarker->SetText(FText::FromString(TEXT("+")));
    AimMarker->SetShadowColorAndOpacity(FLinearColor::Black);
    AimMarker->SetShadowOffset(FVector2D(1,1));
    UCanvasPanelSlot* AimPlacement=Canvas->AddChildToCanvas(AimMarker);
    AimPlacement->SetAnchors(FAnchors(0.5f,0.5f));
    AimPlacement->SetAlignment(FVector2D(0.5f,0.5f));
    AimPlacement->SetAutoSize(true);
    AimPlacement->SetPosition(FVector2D::ZeroVector);

    // Interaction prompt + latest server feedback, below the crosshair (M7).
    InteractionLabel=WidgetTree->ConstructWidget<UTextBlock>();
    InteractionLabel->SetAutoWrapText(true);
    InteractionLabel->SetJustification(ETextJustify::Center);
    InteractionLabel->SetShadowColorAndOpacity(FLinearColor::Black);
    InteractionLabel->SetShadowOffset(FVector2D(1,1));
    auto InteractionFont=InteractionLabel->GetFont();
    InteractionFont.Size=26;
    InteractionLabel->SetFont(InteractionFont);
    auto* InteractionPlacement=Canvas->AddChildToCanvas(InteractionLabel);
    InteractionPlacement->SetAnchors(FAnchors(0.5f,0.62f));
    InteractionPlacement->SetAlignment(FVector2D(0.5f,0));
    InteractionPlacement->SetSize(FVector2D(800,140));

    // Vitals panel, bottom-left.
    UVerticalBox* Panel = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(Panel);
    PanelSlot->SetAnchors(FAnchors(0.f, 1.f));
    PanelSlot->SetAlignment(FVector2D(0.f, 1.f));
    PanelSlot->SetPosition(FVector2D(32.f, -32.f));
    PanelSlot->SetSize(FVector2D(440.f, 215.f));
    PanelSlot->SetAutoSize(true);
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

    // Purely visual: never steal mouse clicks from gameplay.
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPFSurvivalHUD::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
    Super::NativeTick(Geometry, DeltaTime);

    // Prompt text involves traces, so rebuild it at 10 Hz rather than every frame.
    InteractionRefresh+=DeltaTime;
    if(InteractionRefresh<0.1f){return;}
    InteractionRefresh=0;
    if(const auto* Settings=UPFGameUserSettings::Get())
    {
        const auto& P=Settings->Preferences;
        for(auto* Label:{HealthLabel.Get(),StaminaLabel.Get(),NeedsLabel.Get(),StateLabel.Get(),InteractionLabel.Get()})
        {
            if(Label){auto Font=Label->GetFont();const int32 Size=FMath::RoundToInt(22*P.HUDScale);if(Font.Size!=Size){Font.Size=Size;Label->SetFont(Font);}}
        }
        if(AimMarker){AimMarker->SetVisibility(P.bCrosshair?ESlateVisibility::HitTestInvisible:ESlateVisibility::Hidden);auto Font=AimMarker->GetFont();const int32 Size=FMath::RoundToInt(24*P.CrosshairScale);if(Font.Size!=Size){Font.Size=Size;AimMarker->SetFont(Font);}}
    }
    if(InteractionLabel)
    {
        InteractionRefresh=0;
        if(const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer()))
        {
            InteractionLabel->SetText(FText::FromString(PC->IsPauseMenuOpen()?FString():PC->InteractionPrompt()+TEXT("\n")+PC->RecentInteractionMessage()));
        }
    }

    // The pawn changes on respawn, so look the survival component up every frame.
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
        HungerBar->SetPercent(V.Hunger/100.f);
        ThirstBar->SetPercent(V.Thirst/100.f);
        // Most urgent condition first; otherwise a short controls reminder.
        StateLabel->SetText(FText::FromString(Survival->IsDead() ? TEXT("You died - respawning...") :
            (V.Hunger <= 0 || V.Thirst <= 0 ? TEXT("STARVING / DEHYDRATED - gather food or water!") :
            (V.Exposure > 0 ? TEXT("DANGER - leave exposure zone!") : (UPFGameUserSettings::Get() && !UPFGameUserSettings::Get()->Preferences.bControlHints?TEXT(""):TEXT("WASD | E Interact | Tab Bag | C Craft | B Build | P Menu"))))));
    }
    // Blueprint presentation hooks (no-ops unless a BP child implements them).
    PresentVitals(V.Health, V.MaxHealth, V.Stamina, V.MaxStamina, Survival->IsDead());
    PresentNeeds(V.Hunger,V.Thirst,V.Exposure);
}
