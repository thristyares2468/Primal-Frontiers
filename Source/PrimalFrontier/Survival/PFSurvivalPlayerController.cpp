#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivalHUD.h"
#include "Survival/PFRecoveryPickup.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemPickup.h"
#include "Inventory/PFInventoryHUD.h"
#include "GameFramework/PlayerState.h"

APFSurvivalPlayerController::APFSurvivalPlayerController() { SurvivalHUDClass = UPFSurvivalHUD::StaticClass(); }
void APFSurvivalPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (IsLocalController() && GetLocalPlayer() && SurvivalHUDClass)
    {
        SurvivalHUD = CreateWidget<UPFSurvivalHUD>(this, SurvivalHUDClass);
        if (SurvivalHUD) { SurvivalHUD->AddToPlayerScreen(); }
        InventoryHUD=CreateWidget<UPFInventoryHUD>(this,UPFInventoryHUD::StaticClass());
        if(InventoryHUD){InventoryHUD->AddToPlayerScreen();}
    }
}
void APFSurvivalPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindKey(EKeys::E,IE_Pressed,this,&APFSurvivalPlayerController::Interact);
    InputComponent->BindKey(EKeys::Tab,IE_Pressed,this,&APFSurvivalPlayerController::ToggleInventory);
    InputComponent->BindKey(EKeys::Down,IE_Pressed,this,&APFSurvivalPlayerController::InventoryNext);
    InputComponent->BindKey(EKeys::Up,IE_Pressed,this,&APFSurvivalPlayerController::InventoryPrevious);
    InputComponent->BindKey(EKeys::X,IE_Pressed,this,&APFSurvivalPlayerController::InventorySplit);
    InputComponent->BindKey(EKeys::G,IE_Pressed,this,&APFSurvivalPlayerController::InventoryDrop);
    InputComponent->BindKey(EKeys::Q,IE_Pressed,this,&APFSurvivalPlayerController::InventoryConsume);
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
    {
        if(auto* Item=Cast<APFItemPickup>(Hit.GetActor()))
        {ClientInventoryFeedback(Item->TryPickup(GetPawn()) ? TEXT("Picked up") : TEXT("Pickup refused: full, expired or invalid"));}
        else if (auto* Pickup = Cast<APFRecoveryPickup>(Hit.GetActor())) { Pickup->TryConsume(GetPawn()); }
    }
}
UPFInventoryComponent* APFSurvivalPlayerController::GetInventory() const
{return PlayerState ? PlayerState->FindComponentByClass<UPFInventoryComponent>() : nullptr;}
void APFSurvivalPlayerController::ToggleInventory(){bInventoryOpen=!bInventoryOpen;}
void APFSurvivalPlayerController::InventoryNext()
{if(bInventoryOpen){if(const auto* I=GetInventory()){SelectedInventoryIndex=FMath::Min(SelectedInventoryIndex+1,I->GetStacks().Num()-1);}}}
void APFSurvivalPlayerController::InventoryPrevious(){if(bInventoryOpen){SelectedInventoryIndex=FMath::Max(0,SelectedInventoryIndex-1);}}
void APFSurvivalPlayerController::InventorySplit(){SendInventoryAction(0);}
void APFSurvivalPlayerController::InventoryDrop(){SendInventoryAction(1);}
void APFSurvivalPlayerController::InventoryConsume(){SendInventoryAction(2);}
void APFSurvivalPlayerController::SendInventoryAction(uint8 Action)
{
    const auto* I=GetInventory(); if(!bInventoryOpen || !I || I->GetStacks().IsEmpty()){return;}
    SelectedInventoryIndex=FMath::Clamp(SelectedInventoryIndex,0,I->GetStacks().Num()-1);
    const auto S=I->GetStacks()[SelectedInventoryIndex];
    ServerInventoryAction(S.StackId,Action,Action==0 ? S.Quantity/2 : 1);
}
void APFSurvivalPlayerController::ServerInventoryAction_Implementation(FGuid StackId,uint8 Action,int32 Quantity)
{
    const double Now=GetWorld()->GetTimeSeconds(); if(Now<NextInventoryTime){return;} NextInventoryTime=Now+0.15;
    auto* I=GetInventory(); const auto* S=GetPawn() ? GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>() : nullptr;
    bool Accepted=false;
    if(HasAuthority() && I && S && !S->IsDead() && Quantity>0 && Quantity<=1000)
    {
        if(Action==0){Accepted=I->Split(StackId,Quantity);}
        else if(Action==1){Accepted=I->Drop(StackId,Quantity,GetPawn())!=nullptr;}
        else if(Action==2 && Quantity==1){Accepted=I->Consume(StackId,GetPawn());}
    }
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalInventory] Request action=%d quantity=%d accepted=%d owner=%s"),Action,Quantity,Accepted,*GetName());
    ClientInventoryFeedback(Accepted ? TEXT("Done") : TEXT("Refused: invalid stack, quantity, space or life state"));
}
void APFSurvivalPlayerController::ClientInventoryFeedback_Implementation(const FString& Message){InventoryMessage=Message;}
