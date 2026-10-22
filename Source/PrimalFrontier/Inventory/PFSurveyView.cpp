#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Settings/PFGameUserSettings.h"
#include "Camera/CameraComponent.h"

float APFSurvivalPlayerController::GetSurveyMagnification() const
{
    if(!SurveyBatch.IsValid() || SurveyPawn.Get()!=GetPawn() || !CanUseHotbar()){return 0;}
    const auto* I=GetInventory();const auto* S=Hotbar.Resolve(Hotbar.Selected(),I,UPFInventoryComponent::ServerTime(GetWorld()));
    const auto* D=S && S->StackId==SurveyBatch?I->Definition(S->ItemId):nullptr;
    return D?D->OpticalMagnification:0;
}
float APFSurvivalPlayerController::ResolveSurveyFieldOfView(float Base)
{
    const float SafeBase=FMath::IsFinite(Base) && Base>=70 && Base<=110?Base:90;
    const float Magnification=GetSurveyMagnification();
    if(Magnification<=0){SurveyBatch.Invalidate();SurveyPawn.Reset();return SafeBase;}
    // Exact optical magnification for any supported user FOV; never persist a zoomed preference.
    return FMath::RadiansToDegrees(2*FMath::Atan(FMath::Tan(FMath::DegreesToRadians(SafeBase*.5f))/Magnification));
}
void APFSurvivalPlayerController::CancelSurveyView()
{
    SurveyBatch.Invalidate();SurveyPawn.Reset();
    if(!IsLocalController()){return;}
    if(auto* P=Cast<APFSurvivorCharacter>(GetPawn()))
    {
        const auto* Settings=UPFGameUserSettings::Get();const float Base=Settings?Settings->Preferences.FieldOfView:90;
        P->GetFirstPersonCameraComponent()->SetFieldOfView(ResolveSurveyFieldOfView(Base));
    }
}
void APFSurvivalPlayerController::ToggleSurveyView()
{
    if(!CanUseHotbar()){return;}
    if(GetSurveyMagnification()>0){CancelSurveyView();ClientInventoryFeedback_Implementation(TEXT("Survey scope lowered"));return;}
    const auto* I=GetInventory();const auto* S=Hotbar.Resolve(Hotbar.Selected(),I,UPFInventoryComponent::ServerTime(GetWorld()));
    const auto* D=S?I->Definition(S->ItemId):nullptr;
    if(!D || D->OpticalMagnification<=0){return;}
    SurveyBatch=S->StackId;SurveyPawn=GetPawn();
    ClientInventoryFeedback_Implementation(FString::Printf(TEXT("Survey scope %.1fx | Q / D-down to lower. Optical view only; interaction reach unchanged."),D->OpticalMagnification));
}
