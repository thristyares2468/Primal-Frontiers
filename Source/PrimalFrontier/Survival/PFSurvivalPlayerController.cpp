#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivalHUD.h"
#include "Survival/PFRecoveryPickup.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"

APFSurvivalPlayerController::APFSurvivalPlayerController() { SurvivalHUDClass = UPFSurvivalHUD::StaticClass(); }
void APFSurvivalPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (IsLocalController() && GetLocalPlayer() && SurvivalHUDClass)
    {
        SurvivalHUD = CreateWidget<UPFSurvivalHUD>(this, SurvivalHUDClass);
        if (SurvivalHUD) { SurvivalHUD->AddToPlayerScreen(); }
    }
}
void APFSurvivalPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindKey(EKeys::E,IE_Pressed,this,&APFSurvivalPlayerController::Interact);
}
void APFSurvivalPlayerController::Interact() { if (IsLocalController()) { ServerInteract(); } }
void APFSurvivalPlayerController::ServerInteract_Implementation()
{
    if (!HasAuthority() || !GetPawn()) { return; }
    const double Now = GetWorld()->GetTimeSeconds();
    if (Now < NextInteractionTime) { return; }
    NextInteractionTime = Now + 0.25;
    const auto* S = GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
    if (!S || S->IsDead()) { return; }
    FVector Eye; FRotator Look; GetPawn()->GetActorEyesViewPoint(Eye,Look);
    FHitResult Hit; FCollisionQueryParams Params(SCENE_QUERY_STAT(PFInteraction),false,GetPawn());
    if (GetWorld()->LineTraceSingleByChannel(Hit,Eye,Eye + Look.Vector()*250.f,ECC_Visibility,Params))
    { if (auto* Pickup = Cast<APFRecoveryPickup>(Hit.GetActor())) { Pickup->TryConsume(GetPawn()); } }
}
