// PFSurvivalPlayerController.cpp
//
// Client input -> server request routing for the survivor. Pattern used throughout:
//   local key handler (checks UI state)  ->  Server*_Implementation (rate limit,
//   life state, server-derived target)  ->  gameplay component API  ->
//   ClientInventoryFeedback(message).
// Gamepad bindings are in PFGamepadInput.cpp and reuse the same handlers.

#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivalHUD.h"
#include "Survival/PFRecoveryPickup.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "InputCoreTypes.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFItemCatalog.h"
#include "Inventory/PFItemPickup.h"
#include "Inventory/PFInventoryHUD.h"
#include "GameFramework/PlayerState.h"
#include "Crafting/PFCraftingComponent.h"
#include "Crafting/PFCraftingHUD.h"
#include "Crafting/PFResourceNode.h"
#include "Building/PFBuildingComponent.h"
#include "Building/PFBuildingHUD.h"
#include "Building/PFBuildPiece.h"
#include "Creatures/PFCreature.h"
#include "Engine/DamageEvents.h"
#include "Survival/PFPauseMenu.h"
#include "Survival/PFInteraction.h"
#include "GameFramework/PlayerInput.h"

APFSurvivalPlayerController::APFSurvivalPlayerController()
{
    SurvivalHUDClass = UPFSurvivalHUD::StaticClass();
    Building=CreateDefaultSubobject<UPFBuildingComponent>(TEXT("Building"));
}

void APFSurvivalPlayerController::BeginPlay()
{
    Super::BeginPlay();
    // Widgets exist only for the local player (never on the server for remote clients).
    if (IsLocalController() && GetLocalPlayer() && SurvivalHUDClass)
    {
        SurvivalHUD = CreateWidget<UPFSurvivalHUD>(this, SurvivalHUDClass);
        if (SurvivalHUD) { SurvivalHUD->AddToPlayerScreen(); }
        InventoryHUD=CreateWidget<UPFInventoryHUD>(this,UPFInventoryHUD::StaticClass());
        if(InventoryHUD){InventoryHUD->AddToPlayerScreen();}
        CraftingHUD=CreateWidget<UPFCraftingHUD>(this,UPFCraftingHUD::StaticClass());
        if(CraftingHUD){CraftingHUD->AddToPlayerScreen();}
        BuildingHUD=CreateWidget<UPFBuildingHUD>(this,UPFBuildingHUD::StaticClass());
        if(BuildingHUD){BuildingHUD->AddToPlayerScreen();}
    }
}

void APFSurvivalPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    BindGamepadControls();
    // Legacy key bindings for the greybox survival actions. Movement/look/jump come
    // from the template Enhanced Input context (IMC_Default) via the base class.
    // P is the reliable pause key in PIE, where Esc may end the Play session.
    InputComponent->BindKey(EKeys::P,IE_Pressed,this,&APFSurvivalPlayerController::TogglePauseMenu).bExecuteWhenPaused=true;
    InputComponent->BindKey(EKeys::Escape,IE_Pressed,this,&APFSurvivalPlayerController::TogglePauseMenu).bExecuteWhenPaused=true;
    InputComponent->BindKey(EKeys::E,IE_Pressed,this,&APFSurvivalPlayerController::Interact);
    // Inventory (M3)
    InputComponent->BindKey(EKeys::Tab,IE_Pressed,this,&APFSurvivalPlayerController::ToggleInventory);
    InputComponent->BindKey(EKeys::Down,IE_Pressed,this,&APFSurvivalPlayerController::InventoryNext);
    InputComponent->BindKey(EKeys::Up,IE_Pressed,this,&APFSurvivalPlayerController::InventoryPrevious);
    InputComponent->BindKey(EKeys::X,IE_Pressed,this,&APFSurvivalPlayerController::InventorySplit);
    InputComponent->BindKey(EKeys::G,IE_Pressed,this,&APFSurvivalPlayerController::InventoryDrop);
    InputComponent->BindKey(EKeys::Q,IE_Pressed,this,&APFSurvivalPlayerController::InventoryConsume);
    // Crafting (M4)
    InputComponent->BindKey(EKeys::C,IE_Pressed,this,&APFSurvivalPlayerController::ToggleCrafting);
    InputComponent->BindKey(EKeys::One,IE_Pressed,this,&APFSurvivalPlayerController::CraftTool);
    InputComponent->BindKey(EKeys::Two,IE_Pressed,this,&APFSurvivalPlayerController::CookFood);
    InputComponent->BindKey(EKeys::Three,IE_Pressed,this,&APFSurvivalPlayerController::DryFood);
    InputComponent->BindKey(EKeys::R,IE_Pressed,this,&APFSurvivalPlayerController::CancelCraft);
    // Building (M5) and attack (M6, left click outside overlays)
    InputComponent->BindKey(EKeys::B,IE_Pressed,this,&APFSurvivalPlayerController::ToggleBuilding);
    InputComponent->BindKey(EKeys::N,IE_Pressed,this,&APFSurvivalPlayerController::NextBuilding);
    InputComponent->BindKey(EKeys::T,IE_Pressed,this,&APFSurvivalPlayerController::RotateBuilding);
    InputComponent->BindKey(EKeys::LeftMouseButton,IE_Pressed,this,&APFSurvivalPlayerController::PlaceBuilding);
    InputComponent->BindKey(EKeys::H,IE_Pressed,this,&APFSurvivalPlayerController::DemolishBuilding);
    InputComponent->BindKey(EKeys::J,IE_Pressed,this,&APFSurvivalPlayerController::DamageBuilding);
    InputComponent->BindKey(EKeys::U,IE_Pressed,this,&APFSurvivalPlayerController::StoreItem);
    InputComponent->BindKey(EKeys::O,IE_Pressed,this,&APFSurvivalPlayerController::TakeStoredItem);
}

// ---------------------------------------------------------------------------
// Pause menu
// ---------------------------------------------------------------------------

void APFSurvivalPlayerController::TogglePauseMenu(){SetPauseMenuOpen(!bPauseMenuOpen);}

void APFSurvivalPlayerController::SetPauseMenuOpen(bool bOpen)
{
    if(!IsLocalController() || bOpen==bPauseMenuOpen){return;}
    if(bOpen && !PauseMenu)
    {
        PauseMenu=CreateWidget<UPFPauseMenu>(this,UPFPauseMenu::StaticClass());
        if(!PauseMenu){return;}
    }
    bPauseMenuOpen=bOpen;
    SetIgnoreMoveInput(bOpen);
    SetIgnoreLookInput(bOpen);
    bShowMouseCursor=bOpen;
    // Drop held keys so nothing stays "pressed" across the menu transition.
    if(PlayerInput){PlayerInput->FlushPressedKeys();}
    if(bOpen)
    {
        // Close other overlays; only freeze the world when we are the only player.
        bInventoryOpen=false;
        bCraftingOpen=false;
        Building->bBuildMode=false;
        bPausedWorld=GetNetMode()==NM_Standalone && SetPause(true);
        PauseMenu->AddToPlayerScreen(100);
        PauseMenu->Refresh();
        FInputModeUIOnly Mode;
        Mode.SetWidgetToFocus(PauseMenu->TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(Mode);
        PauseMenu->SetKeyboardFocus();
    }
    else
    {
        if(bPausedWorld)
        {
            SetPause(false);
            bPausedWorld=false;
        }
        if(PauseMenu){PauseMenu->RemoveFromParent();}
        FInputModeGameOnly Mode;
        Mode.SetConsumeCaptureMouseDown(false);
        SetInputMode(Mode);
    }
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalUI] Menu open=%d worldPaused=%d netMode=%d"),bPauseMenuOpen,bPausedWorld,int32(GetNetMode()));
}

// ---------------------------------------------------------------------------
// Interaction (E / Pad X)
// ---------------------------------------------------------------------------

void APFSurvivalPlayerController::Interact()
{
    if(!IsLocalController() || bPauseMenuOpen){return;}
    // E prioritizes a nearby pickup/resource even while build preview is open.
    if(PFInteraction::FindTarget(GetPawn())){ServerInteract();}
    else if(Building->TracedPiece()){Building->ServerTargetAction(1);}  // 1 = open door / storage
    else{ServerInteract();}  // server replies with a "nothing in reach" hint
}

void APFSurvivalPlayerController::ServerInteract_Implementation()
{
    if (!HasAuthority() || !GetPawn()) { return; }
    const double Now = GetWorld()->GetTimeSeconds();
    if (Now < NextInteractionTime) { return; }
    NextInteractionTime = Now + 0.25;
    const auto* S = GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
    if (!S || S->IsDead()) { return; }
    // The server repeats the view query itself; the client never names the target.
    if (AActor* Target=PFInteraction::FindTarget(GetPawn()))
    {
        if(auto* Item=Cast<APFItemPickup>(Target))
        {
            ClientInventoryFeedback(Item->TryPickup(GetPawn()) ? TEXT("Picked up") : TEXT("Pickup refused: full, expired or invalid"));
        }
        else if (auto* Pickup = Cast<APFRecoveryPickup>(Target)) { Pickup->TryConsume(GetPawn()); }
        else if(auto* Node=Cast<APFResourceNode>(Target))
        {
            ClientInventoryFeedback(Node->Gather(GetPawn())?TEXT("Gathered"):TEXT("Gather refused: depleted, cooldown or full inventory"));
        }
    }
    else{ClientInventoryFeedback(TEXT("No item in reach - aim at a labelled pickup or resource within 2.5 m."));}
}

FString APFSurvivalPlayerController::InteractionPrompt() const
{
    if(bPauseMenuOpen || !GetPawn()){return FString();}
    if(AActor* Target=PFInteraction::FindTarget(GetPawn()))
    {
        if(auto* Item=Cast<APFItemPickup>(Target))
        {
            const auto* I=GetInventory();
            const auto* D=I?I->Definition(Item->GetContents().ItemId):nullptr;
            return FString::Printf(TEXT("E / Pad X - Pick up %s x%d"),D?*D->DisplayName.ToString():*Item->GetContents().ItemId.ToString(),Item->GetContents().Quantity);
        }
        if(auto* Node=Cast<APFResourceNode>(Target)){return Node->HitsRemaining>0?TEXT("E / Pad X - Gather resource"):TEXT("Resource depleted - wait for regrowth");}
        return TEXT("E / Pad X - Recover");
    }
    // Something interactable is visible but beyond the 2.5 m reach.
    if(PFInteraction::FindTarget(GetPawn(),500)){return TEXT("Move closer to interact (2.5 m)");}
    if(Building->TracedPiece()){return TEXT("E - Open owned door / storage");}
    return FString();
}

FString APFSurvivalPlayerController::RecentInteractionMessage() const{return GetWorld()->GetRealTimeSeconds()<MessageUntil?InventoryMessage:FString();}

UPFInventoryComponent* APFSurvivalPlayerController::GetInventory() const
{
    return PlayerState ? PlayerState->FindComponentByClass<UPFInventoryComponent>() : nullptr;
}

// ---------------------------------------------------------------------------
// Inventory (Tab overlay)
// ---------------------------------------------------------------------------

void APFSurvivalPlayerController::ToggleInventory()
{
    if(!bPauseMenuOpen)
    {
        bInventoryOpen=!bInventoryOpen;
        if(bInventoryOpen)
        {
            bCraftingOpen=false;
            Building->bBuildMode=false;
        }
    }
}

void APFSurvivalPlayerController::InventoryNext()
{
    if(bInventoryOpen){if(const auto* I=GetInventory()){SelectedInventoryIndex=FMath::Min(SelectedInventoryIndex+1,I->GetStacks().Num()-1);}}
}

void APFSurvivalPlayerController::InventoryPrevious(){if(bInventoryOpen){SelectedInventoryIndex=FMath::Max(0,SelectedInventoryIndex-1);}}
void APFSurvivalPlayerController::InventorySplit(){SendInventoryAction(0);}
void APFSurvivalPlayerController::InventoryDrop(){SendInventoryAction(1);}
void APFSurvivalPlayerController::InventoryConsume(){SendInventoryAction(2);}

void APFSurvivalPlayerController::SendInventoryAction(uint8 Action)
{
    const auto* I=GetInventory();
    if(bPauseMenuOpen || !bInventoryOpen || !I || I->GetStacks().IsEmpty()){return;}
    SelectedInventoryIndex=FMath::Clamp(SelectedInventoryIndex,0,I->GetStacks().Num()-1);
    const auto S=I->GetStacks()[SelectedInventoryIndex];
    // Only the stable stack GUID travels; the server looks it up in *its own* copy.
    ServerInventoryAction(S.StackId,Action,Action==0 ? S.Quantity/2 : 1);
}

void APFSurvivalPlayerController::ServerInventoryAction_Implementation(FGuid StackId,uint8 Action,int32 Quantity)
{
    const double Now=GetWorld()->GetTimeSeconds();
    if(Now<NextInventoryTime){return;}
    NextInventoryTime=Now+0.15;
    auto* I=GetInventory();
    const auto* S=GetPawn() ? GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>() : nullptr;
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

void APFSurvivalPlayerController::ClientInventoryFeedback_Implementation(const FString& Message)
{
    InventoryMessage=Message;
    MessageUntil=GetWorld()->GetRealTimeSeconds()+4;
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalInteraction] %s"),*Message);
}

// ---------------------------------------------------------------------------
// Crafting (C overlay)
// ---------------------------------------------------------------------------

UPFCraftingComponent* APFSurvivalPlayerController::GetCrafting() const
{
    return PlayerState?PlayerState->FindComponentByClass<UPFCraftingComponent>():nullptr;
}

void APFSurvivalPlayerController::ToggleCrafting()
{
    if(bPauseMenuOpen){return;}
    bCraftingOpen=!bCraftingOpen;
    if(bCraftingOpen)
    {
        Building->bBuildMode=false;
        bInventoryOpen=false;
    }
}

// Recipe hotkeys 1/2/3 map to the three greybox recipes in DA_CraftingCatalog.
void APFSurvivalPlayerController::CraftTool(){if(bCraftingOpen){ServerCraftAction(TEXT("Recipe_Tool"),false);}}
void APFSurvivalPlayerController::CookFood(){if(bCraftingOpen){ServerCraftAction(TEXT("Recipe_Cook"),false);}}
void APFSurvivalPlayerController::DryFood(){if(bCraftingOpen){ServerCraftAction(TEXT("Recipe_Dry"),false);}}
void APFSurvivalPlayerController::CancelCraft(){if(bCraftingOpen){ServerCraftAction(NAME_None,true);}}

void APFSurvivalPlayerController::ServerCraftAction_Implementation(FName Id,bool bCancel)
{
    if(!HasAuthority()){return;}
    const double Now=GetWorld()->GetTimeSeconds();
    if(Now<NextCraftTime){return;}
    NextCraftTime=Now+0.25;
    // The recipe ID is only a lookup key; the server's catalog decides inputs, time and output.
    auto* C=GetCrafting();
    const bool Accepted=C && (bCancel?C->Cancel():C->Start(Id,GetPawn()));
    UE_LOG(LogPFSurvival,Display,TEXT("[PrimalCrafting] Request recipe=%s cancel=%d accepted=%d owner=%s"),*Id.ToString(),bCancel,Accepted,*GetName());
    ClientInventoryFeedback(Accepted?TEXT("Craft request accepted"):TEXT("Craft refused: busy, invalid recipe, ingredients or life state"));
}

// ---------------------------------------------------------------------------
// Building (B overlay) and melee attack
// ---------------------------------------------------------------------------

void APFSurvivalPlayerController::ToggleBuilding()
{
    if(bPauseMenuOpen){return;}
    Building->bBuildMode=!Building->bBuildMode;
    if(Building->bBuildMode)
    {
        bCraftingOpen=false;
        bInventoryOpen=false;
    }
}

void APFSurvivalPlayerController::NextBuilding()
{
    if(Building->bBuildMode && Building->Catalog && Building->Catalog->Pieces.Num()>0){Building->Selection=(Building->Selection+1)%Building->Catalog->Pieces.Num();}
}

void APFSurvivalPlayerController::RotateBuilding(){if(Building->bBuildMode){Building->Rotation=(Building->Rotation+1)%4;}}

// Left click / RT: place in build mode; otherwise (no overlay open) attack.
void APFSurvivalPlayerController::PlaceBuilding()
{
    if(Building->bBuildMode){Building->ServerPlace(Building->SelectedId(),Building->Rotation);}
    else if(!bInventoryOpen && !bCraftingOpen){ServerAttackCreature();}
}

void APFSurvivalPlayerController::ServerAttackCreature_Implementation()
{
    if(!HasAuthority() || !GetPawn()){return;}
    const double Now=GetWorld()->GetTimeSeconds();
    if(Now<NextAttackTime){return;}
    NextAttackTime=Now+0.5;
    auto* V=GetPawn()->FindComponentByClass<UPFPlayerSurvivalComponent>();
    if(!V || V->IsDead()){return;}
    // Server-derived 2.5 m view trace: the client cannot choose the target or the reach.
    FVector Eye;
    FRotator Look;
    GetPawn()->GetActorEyesViewPoint(Eye,Look);
    FHitResult Hit;
    if(!GetWorld()->LineTraceSingleByChannel(Hit,Eye,Eye+Look.Vector()*250,ECC_Visibility,FCollisionQueryParams(SCENE_QUERY_STAT(PFCreatureAttack),false,GetPawn()))){return;}
    // Each swing costs 5 stamina; the gathering tool raises damage from 20 to 35.
    auto* Creature=Cast<APFCreature>(Hit.GetActor());
    if(!Creature || Creature->IsDead() || !V->SpendStamina(5)){return;}
    const auto* I=GetInventory();
    const float Amount=I && I->Count(TEXT("Item_Tool"))>0?35.f:20.f;
    const bool HitCreature=Creature->TakeDamage(Amount,FDamageEvent(),this,GetPawn())>0;
    ClientInventoryFeedback(HitCreature?TEXT("Creature hit"):TEXT("Attack refused"));
}

// Building target actions: 0 = demolish, 1 = open door/storage, 2 = 25-damage owner hammer.
void APFSurvivalPlayerController::DemolishBuilding(){if(Building->bBuildMode){Building->ServerTargetAction(0);}}
void APFSurvivalPlayerController::DamageBuilding(){if(Building->bBuildMode){Building->ServerTargetAction(2);}}

// Deposit one of the selected bag stack into the open storage box.
void APFSurvivalPlayerController::StoreItem()
{
    auto* I=GetInventory();
    if(Building->bBuildMode && I && I->GetStacks().IsValidIndex(SelectedInventoryIndex)){Building->ServerTransfer(true,I->GetStacks()[SelectedInventoryIndex].StackId,1);}
}

// Withdraw one item from the first stack in the open storage box.
void APFSurvivalPlayerController::TakeStoredItem()
{
    auto* P=Building->OpenStorage.Get();
    if(Building->bBuildMode && IsValid(P) && !P->Storage->GetStacks().IsEmpty()){Building->ServerTransfer(false,P->Storage->GetStacks()[0].StackId,1);}
}
