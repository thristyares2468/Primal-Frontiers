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
#include "Components/SizeBox.h"
#include "GameFramework/Pawn.h"
#include "Settings/PFGameUserSettings.h"
#include "Building/PFBuildingComponent.h"

void UPFSurvivalHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    // A Blueprint child can replace this placeholder layout and use PresentVitals.
    if (WidgetTree->RootWidget) { return; }
    UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Canvas;

    // Centred aim marker (M4): makes the server-derived interaction ray aimable.
    AimMarker=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PF_AimMarker"));
    AimMarker->SetText(FText::FromString(TEXT("+")));
    AimMarker->SetShadowColorAndOpacity(FLinearColor::Black);
    AimMarker->SetShadowOffset(FVector2D(1,1));
    UCanvasPanelSlot* AimPlacement=Canvas->AddChildToCanvas(AimMarker);
    AimPlacement->SetAnchors(FAnchors(0.5f,0.5f));
    AimPlacement->SetAlignment(FVector2D(0.5f,0.5f));
    AimPlacement->SetAutoSize(true);
    AimPlacement->SetPosition(FVector2D::ZeroVector);

    // Interaction prompt + latest server feedback, below the crosshair (M7).
    InteractionLabel=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PF_InteractionLabel"));
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
    USizeBox* Bounds = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("PF_VitalsBounds"));
    Bounds->SetWidthOverride(480); Bounds->AddChild(Panel);
    UCanvasPanelSlot* PanelSlot = Canvas->AddChildToCanvas(Bounds);
    PanelSlot->SetAnchors(FAnchors(0.f, 1.f));
    PanelSlot->SetAlignment(FVector2D(0.f, 1.f));
    PanelSlot->SetPosition(FVector2D(32.f, -32.f));
    PanelSlot->SetSize(FVector2D(440.f, 215.f));
    PanelSlot->SetAutoSize(true);
    HealthLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PF_HealthLabel")); Panel->AddChild(HealthLabel);
    HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("PF_HealthBar")); Panel->AddChild(HealthBar);
    HealthBar->SetFillColorAndOpacity(FLinearColor(0.8f, 0.15f, 0.1f));
    StaminaLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PF_StaminaLabel")); Panel->AddChild(StaminaLabel);
    StaminaBar = WidgetTree->ConstructWidget<UProgressBar>(); Panel->AddChild(StaminaBar);
    StaminaBar->SetFillColorAndOpacity(FLinearColor(0.2f, 0.8f, 0.35f));
    NeedsLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PF_NeedsLabel")); Panel->AddChild(NeedsLabel);
    NeedsLabel->SetAutoWrapText(true);
    HungerBar = WidgetTree->ConstructWidget<UProgressBar>(); Panel->AddChild(HungerBar);
    HungerBar->SetFillColorAndOpacity(FLinearColor(0.8f,0.65f,0.25f));
    ThirstBar = WidgetTree->ConstructWidget<UProgressBar>(); Panel->AddChild(ThirstBar);
    ThirstBar->SetFillColorAndOpacity(FLinearColor(0.2f,0.55f,0.9f));
    StateLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PF_StateLabel")); Panel->AddChild(StateLabel);

    StateLabel->SetAutoWrapText(true);
    StateLabel->SetShadowColorAndOpacity(FLinearColor::Black);
    StateLabel->SetShadowOffset(FVector2D(1,1));
    DamageLabel=WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PF_DamageLabel"));
    DamageLabel->SetColorAndOpacity(FLinearColor(1.f,0.65f,0.4f));
    DamageLabel->SetShadowColorAndOpacity(FLinearColor::Black);
    DamageLabel->SetShadowOffset(FVector2D(1,1));
    Panel->AddChild(DamageLabel);

    // Purely visual: never steal mouse clicks from gameplay.
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPFSurvivalHUD::NativeTick(const FGeometry& Geometry, float DeltaTime)
{
    Super::NativeTick(Geometry, DeltaTime);
    InteractionRefresh+=DeltaTime;
    if(InteractionRefresh<0.1f){return;}
    InteractionRefresh=0;
    const auto* Settings=UPFGameUserSettings::Get();
    for(auto* Label:{HealthLabel.Get(),StaminaLabel.Get(),NeedsLabel.Get(),StateLabel.Get(),DamageLabel.Get(),InteractionLabel.Get()})
    {
        if(Label){auto Font=Label->GetFont();const int32 Size=FMath::RoundToInt(22*(Settings?Settings->Preferences.HUDScale:1.f));if(Font.Size!=Size){Font.Size=Size;Label->SetFont(Font);}}
    }
    const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());
    const APawn* Pawn=GetOwningPlayerPawn();
    const auto* Survival=Pawn?Pawn->FindComponentByClass<UPFPlayerSurvivalComponent>():nullptr;
    const bool bAvailable=Survival!=nullptr;
    const bool bDead=bAvailable && Survival->IsDead();
    const bool bCanAim=bAvailable && !bDead && PC && !PC->IsPauseMenuOpen();
    if(AimMarker)
    {
        AimMarker->SetVisibility(bCanAim && (!Settings || Settings->Preferences.bCrosshair)?ESlateVisibility::HitTestInvisible:ESlateVisibility::Hidden);
        auto Font=AimMarker->GetFont();const int32 Size=FMath::RoundToInt(24*(Settings?Settings->Preferences.CrosshairScale:1.f));
        if(Font.Size!=Size){Font.Size=Size;AimMarker->SetFont(Font);}
    }
    if(InteractionLabel)
    {
        const bool bActionOverlay=PC && (PC->IsCraftingOpen() || (PC->Building && PC->Building->bBuildMode));
        auto* Placement=Cast<UCanvasPanelSlot>(InteractionLabel->Slot);
        if(Placement)
        {
            Placement->SetAnchors(bActionOverlay?FAnchors(0.285f,0.24f):FAnchors(0.5f,0.62f));
            Placement->SetSize(bActionOverlay?Geometry.GetLocalSize()*FVector2D(0.49f,0.26f):FVector2D(800,140));
        }
        // Craft request feedback already has its own footer. Do not leave it under
        // another panel after changing mode; pickup/gather feedback remains visible.
        FString Message=bCanAim?PC->RecentInteractionMessage():FString();
        if(bActionOverlay && Message.StartsWith(TEXT("Craft "))){Message.Empty();}
        InteractionLabel->SetText(bCanAim?FText::FromString(PC->InteractionPrompt()+TEXT("\n")+Message):FText::GetEmpty());
    }

    if(!bAvailable)
    {
        ObservedPawn.Reset();PreviousHealth=0;DamageUntil=0;DamageText=FText::GetEmpty();
        if(HealthLabel){HealthLabel->SetText(FText::FromString(TEXT("HEALTH --")));}
        if(StaminaLabel){StaminaLabel->SetText(FText::FromString(TEXT("STAMINA --")));}
        if(NeedsLabel){NeedsLabel->SetText(FText::FromString(TEXT("FOOD -- | WATER -- | EXPOSURE --")));}
        for(auto* Bar:{HealthBar.Get(),StaminaBar.Get(),HungerBar.Get(),ThirstBar.Get()}){if(Bar){Bar->SetPercent(0);}}
        const FText Waiting=FText::FromString(TEXT("Waiting for survivor..."));
        if(StateLabel){StateLabel->SetText(Waiting);StateLabel->SetVisibility(ESlateVisibility::HitTestInvisible);}
        if(DamageLabel){DamageLabel->SetText(DamageText);DamageLabel->SetVisibility(ESlateVisibility::Collapsed);}
        PresentVitals(0,0,0,0,false);PresentNeeds(0,0,0);PresentStatus(false,Waiting,DamageText);return;
    }

    const FPFPlayerVitals V=Survival->GetVitals();
    const double Now=GetWorld()->GetRealTimeSeconds();
    // A new pawn's first snapshot is a baseline, never damage from the old pawn.
    if(ObservedPawn.Get()!=Pawn){ObservedPawn=const_cast<APawn*>(Pawn);DamageUntil=0;DamageText=FText::GetEmpty();}
    else if(!bDead && V.Health<PreviousHealth-0.01f)
    {DamageText=FText::FromString(FString::Printf(TEXT("DAMAGE -%.2f health"),PreviousHealth-V.Health));DamageUntil=Now+1.5;}
    PreviousHealth=V.Health;
    if(bDead || Now>=DamageUntil){DamageText=FText::GetEmpty();}
    if(DamageLabel){DamageLabel->SetText(DamageText);DamageLabel->SetVisibility(DamageText.IsEmpty()?ESlateVisibility::Collapsed:ESlateVisibility::HitTestInvisible);}
    if(HealthLabel){HealthLabel->SetText(FText::FromString(FString::Printf(TEXT("HEALTH   %.0f / %.0f"),V.Health,V.MaxHealth)));}
    if(StaminaLabel){StaminaLabel->SetText(FText::FromString(FString::Printf(TEXT("STAMINA   %.0f / %.0f"),V.Stamina,V.MaxStamina)));}
    if(NeedsLabel){NeedsLabel->SetText(FText::FromString(FString::Printf(TEXT("FOOD %.0f | WATER %.0f | EXPOSURE %.0f%%"),V.Hunger,V.Thirst,V.Exposure*100)));}
    if(HealthBar){HealthBar->SetPercent(V.Health/FMath::Max(1.f,V.MaxHealth));}
    if(StaminaBar){StaminaBar->SetPercent(V.Stamina/FMath::Max(1.f,V.MaxStamina));}
    if(HungerBar){HungerBar->SetPercent(V.Hunger/100.f);}if(ThirstBar){ThirstBar->SetPercent(V.Thirst/100.f);}
    TArray<FString> Warnings;
    if(bDead){Warnings.Add(TEXT("You died - waiting for server respawn..."));}
    else
    {
        if(V.Health<=V.MaxHealth*0.25f){Warnings.Add(TEXT("CRITICAL HEALTH"));}
        if(V.Hunger<=0){Warnings.Add(TEXT("STARVING - eat fresh food"));}
        else if(V.Hunger<=25){Warnings.Add(TEXT("LOW FOOD - find food"));}
        if(V.Thirst<=0){Warnings.Add(TEXT("DEHYDRATED - drink safe water"));}
        else if(V.Thirst<=25){Warnings.Add(TEXT("LOW WATER - find water"));}
        if(V.Exposure>0){Warnings.Add(TEXT("EXPOSURE - leave hazard"));}
        if(Warnings.IsEmpty() && (!Settings || Settings->Preferences.bControlHints))
        {Warnings.Add(TEXT("P / Menu: Controls & help"));}
    }
    const FText Status=FText::FromString(FString::Join(Warnings,TEXT("\n")));
    if(StateLabel){StateLabel->SetText(Status);StateLabel->SetVisibility(Status.IsEmpty()?ESlateVisibility::Collapsed:ESlateVisibility::HitTestInvisible);}
    PresentVitals(V.Health,V.MaxHealth,V.Stamina,V.MaxStamina,bDead);
    PresentNeeds(V.Hunger,V.Thirst,V.Exposure);PresentStatus(true,Status,DamageText);
}
