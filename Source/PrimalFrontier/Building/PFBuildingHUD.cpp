// PFBuildingHUD.cpp — see PFBuildingHUD.h.

#include "Building/PFBuildingHUD.h"
#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildPiece.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "UI/PFReadOnlyOverlay.h"
#include "Settings/PFGameUserSettings.h"

void UPFBuildingHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    UBorder* P;UTextBlock* H;UTextBlock* Body;UTextBlock* R;
    PFReadOnlyOverlay::Build(WidgetTree,TEXT("PF_BuildingPanel"),P,H,Body,R);
    Panel=P;Heading=H;Text=Body;Result=R;
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPFBuildingHUD::NativeTick(const FGeometry& Geometry,float Delta)
{
    Super::NativeTick(Geometry,Delta);
    auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());
    auto* B=PC?PC->Building.Get():nullptr;
    const bool bOpen=B && B->bBuildMode && !PC->IsPauseMenuOpen();
    Panel->SetVisibility(bOpen?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);
    Refresh+=Delta;
    if(!bOpen){bWasOpen=false;return;}
    if(bWasOpen && Refresh<0.1f){return;}
    bWasOpen=true;Refresh=0;
    const auto* Settings=UPFGameUserSettings::Get();
    PFReadOnlyOverlay::SetScale(Heading,Text,Result,Settings?Settings->Preferences.HUDScale:1);
    Heading->SetText(FText::FromString(!Settings || Settings->Preferences.bControlHints?
        TEXT("BUILD  B / RB close | N / D-up next\nT / D-down rotate | Click / RT place\nH / LB demolish | J debug damage\nE / X door/storage | U / D-left store\nO / D-right take first stored item"):TEXT("BUILD")));

    FString Lines;
    // Selected piece, its cost, rotation and the preview's validity message.
    const auto* D=B->Catalog?B->Catalog->Find(B->SelectedId()):nullptr;
    if(D){Lines+=FString::Printf(TEXT("%s | %d wood | %d degrees\nPreview (advisory): %s\n"),*D->Name.ToString(),D->WoodCost,B->Rotation*90,*B->PreviewMessage);}
    else{Lines+=TEXT("Building definition unavailable\n");}
    if(auto* Target=B->TracedPiece()){Lines+=FString::Printf(TEXT("Target: %s | Health %.0f\n"),*Target->DefinitionId.ToString(),Target->Health);}
    // Open storage contents (owner-only replicated); food keeps ageing inside.
    if(IsValid(B->OpenStorage) && B->OpenStorage->Storage)
    {
        Lines+=TEXT("\nSTORAGE (no preservation)\n");
        const auto* I=B->OpenStorage->Storage.Get();
        int32 Shown=0;
        for(const auto& S:I->GetStacks())
        {
            if(Shown++>=4){break;}
            const auto* Item=I->Definition(S.ItemId);
            Lines+=FString::Printf(TEXT("%s x%d"),Item?*Item->DisplayName.ToString():*S.ItemId.ToString(),S.Quantity);
            if(S.ExpiresAt>0){Lines+=FString::Printf(TEXT(" [%.0fs]"),FMath::Max(0.0,S.ExpiresAt-UPFInventoryComponent::ServerTime(GetWorld())));}
            Lines+=TEXT("\n");
        }
        if(I->GetStacks().Num()>4){Lines+=FString::Printf(TEXT("+%d more stacks; take first\n"),I->GetStacks().Num()-4);}
        if(I->GetStacks().IsEmpty()){Lines+=TEXT("Empty\n");}
    }
    Text->SetText(FText::FromString(Lines));
    Result->SetText(FText::FromString(TEXT("Last server result: ")+(B->Feedback.IsEmpty()?FString(TEXT("No result yet")):B->Feedback)));
}
