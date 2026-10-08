// PFInventoryHUD.cpp — see PFInventoryHUD.h.

#include "Inventory/PFInventoryHUD.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "UI/PFReadOnlyOverlay.h"
#include "Settings/PFGameUserSettings.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"

void UPFInventoryHUD::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    UBorder* P;UTextBlock* H;UTextBlock* B;UTextBlock* R;
    PFReadOnlyOverlay::Build(WidgetTree,TEXT("PF_InventoryPanel"),P,H,B,R);
    Panel=P;Heading=H;Text=B;Result=R;
    Panel->SetVisibility(ESlateVisibility::Collapsed);
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UPFInventoryHUD::NativeTick(const FGeometry& Geometry,float Delta)
{
    Super::NativeTick(Geometry,Delta);
    const auto* PC=Cast<APFSurvivalPlayerController>(GetOwningPlayer());
    const bool bOpen=PC && PC->IsInventoryOpen() && !PC->IsPauseMenuOpen();
    Panel->SetVisibility(bOpen?ESlateVisibility::HitTestInvisible:ESlateVisibility::Collapsed);
    Refresh+=Delta;
    if(!bOpen){bWasOpen=false;return;}
    if(bWasOpen && Refresh<0.1f){return;}
    bWasOpen=true;Refresh=0;
    const auto* Settings=UPFGameUserSettings::Get();
    PFReadOnlyOverlay::SetScale(Heading,Text,Result,Settings?Settings->Preferences.HUDScale:1);
    // The inventory replicates with the PlayerState; it can be briefly missing after joining.
    const auto* I=PC->GetInventory();
    if(!I){Heading->SetText(FText::FromString(TEXT("INVENTORY")));Text->SetText(FText::FromString(TEXT("Waiting for inventory...")));Result->SetText(FText::GetEmpty());return;}

    // Header with capacity and controls.
    FString Header=FString::Printf(TEXT("INVENTORY %d/%d slots | %.1f/%.1f kg"),I->GetStacks().Num(),I->SlotLimit,I->GetWeight(),I->WeightLimit);
    if(!Settings || Settings->Preferences.bControlHints){Header+=TEXT("\nTab / View close | Up/Down select\nX / D-left split | G / D-right drop\nQ / pad X eat/drink");}
    Heading->SetText(FText::FromString(Header));

    // One row per stack: ">" marks the selection; perishables show seconds remaining.
    const int32 Selected=PC->GetSelectedInventoryIndex();
    const auto& Stacks=I->GetStacks();
    // Window follows the resolved GUID selection; it never picks a replacement stack.
    constexpr int32 VisibleRows=4;
    const int32 Start=Selected==INDEX_NONE?0:FMath::Clamp(Selected-VisibleRows+1,0,FMath::Max(0,Stacks.Num()-VisibleRows));
    const int32 End=FMath::Min(Start+VisibleRows,Stacks.Num());
    FString Lines=Stacks.IsEmpty()?TEXT("Empty - find world pickups (E)."):
        FString::Printf(TEXT("Rows %d-%d of %d\n"),Start+1,End,Stacks.Num());
    for(int32 Index=Start;Index<End;++Index)
    {
        const auto& S=Stacks[Index];
        const auto* D=I->Definition(S.ItemId);
        const FString Name=D ? D->DisplayName.ToString() : S.ItemId.ToString();
        const FString Fresh=S.ExpiresAt>0 ? FString::Printf(TEXT(" [%ds fresh]"),FMath::Max(0,FMath::CeilToInt(S.ExpiresAt-UPFInventoryComponent::ServerTime(GetWorld())))) : TEXT("");
        Lines+=FString::Printf(TEXT("%s %d. %s x%d%s\n"),Index==Selected ? TEXT(">") : TEXT(" "),Index+1,*Name,S.Quantity,*Fresh);
    }
    Text->SetText(FText::FromString(Lines.TrimEnd()));
    FString Feedback=PC->GetInventoryMessage();
    if(Selected==INDEX_NONE && !Stacks.IsEmpty()){Feedback=TEXT("No selection - Up/Down or D-pad.\nExpired/removed stacks never reselect.");}
    else if(!Feedback.IsEmpty()){Feedback=TEXT("Last: ")+Feedback;}
    Result->SetText(FText::FromString(Feedback));
}
