// PFSurvivalPlayerController.cpp
//
// Client input -> server request routing for the survivor. Pattern used throughout:
//   local key handler (checks UI state)  ->  Server*_Implementation (rate limit,
//   life state, server-derived target)  ->  gameplay component API  ->
//   ClientInventoryFeedback(message).
// Gamepad bindings are in PFGamepadInput.cpp and reuse the same handlers.

#include "Survival/PFSurvivalPlayerController.h"
#include "PFRequestCodes.h"
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
#include "Settings/PFGameUserSettings.h"
#include "Persistence/PFLocalPlayer.h"
#include "Persistence/PFWorldPersistence.h"
#include "EnhancedPlayerInput.h"
#include "EnhancedActionKeyMapping.h"
#include "InputAction.h"

APFSurvivalPlayerController::APFSurvivalPlayerController()
{
    SurvivalHUDClass = UPFSurvivalHUD::StaticClass();
    Building=CreateDefaultSubobject<UPFBuildingComponent>(TEXT("Building"));
}

void APFSurvivalPlayerController::ClientRememberReconnectCredential_Implementation(FGuid Credential)
{
    if (!IsLocalController() || !GetLocalPlayer()) { return; }
    if (!UPFLocalPlayer::RememberCredential(GetWorld(), Credential))
    {
        UE_LOG(LogPFSurvival, Warning, TEXT("[PrimalPersistence] Local reconnect profile could not be saved; credential omitted from log"));
    }
    else { UE_LOG(LogPFSurvival, Display, TEXT("[PrimalPersistence] Local reconnect profile saved; credential omitted from log")); }
}

void APFSurvivalPlayerController::Destroyed()
{
    // Engine PlayerController::Destroyed removes the pawn before GameMode::Logout.
    // Capture a departing player's real state while the pawn still exists.
    if (HasAuthority() && GetWorld() && !GetWorld()->bIsTearingDown)
    {
        if (auto* Persistence = GetWorld()->GetSubsystem<UPFWorldPersistence>()) { Persistence->Logout(this); }
    }
    Super::Destroyed();
}

void APFSurvivalPlayerController::BeginPlay()
{
    Super::BeginPlay();
    // Widgets exist only for the local player (never on the server for remote clients).
    if (IsLocalController() && GetLocalPlayer() && SurvivalHUDClass)
    {
        if(auto* Settings=UPFGameUserSettings::Get()){Settings->ApplyNonResolutionSettings();Settings->ApplyToWorld(GetWorld());}
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
    RegisteredControlHints.Reset();
    BindGamepadControls();
    // Legacy key bindings for the greybox survival actions. Movement/look/jump come
    // from the template Enhanced Input context (IMC_Default) via the base class.
    // P is the reliable pause key in PIE, where Esc may end the Play session.
    BindControl(EKeys::P,&APFSurvivalPlayerController::TogglePauseMenu,TEXT("General"),TEXT("Pause / resume (preferred in Editor Play)"),true);
    BindControl(EKeys::Escape,&APFSurvivalPlayerController::TogglePauseMenu,TEXT("General"),TEXT("Pause / resume (Editor may stop PIE)"),true);
    BindControl(EKeys::E,&APFSurvivalPlayerController::Interact,TEXT("World"),TEXT("Pick up / gather / use door or storage"));
    // Inventory (M3)
    BindControl(EKeys::Tab,&APFSurvivalPlayerController::ToggleInventory,TEXT("General"),TEXT("Open / close inventory"));
    BindControl(EKeys::Down,&APFSurvivalPlayerController::InventoryNext,TEXT("Inventory"),TEXT("Select next stack"));
    BindControl(EKeys::Up,&APFSurvivalPlayerController::InventoryPrevious,TEXT("Inventory"),TEXT("Select previous stack"));
    BindControl(EKeys::X,&APFSurvivalPlayerController::InventorySplit,TEXT("Inventory"),TEXT("Split selected stack"));
    BindControl(EKeys::G,&APFSurvivalPlayerController::InventoryDrop,TEXT("Inventory"),TEXT("Drop one selected item"));
    BindControl(EKeys::Q,&APFSurvivalPlayerController::InventoryConsume,TEXT("Inventory"),TEXT("Consume one selected edible item"));
    // Crafting (M4)
    BindControl(EKeys::C,&APFSurvivalPlayerController::ToggleCrafting,TEXT("General"),TEXT("Open / close crafting"));
    BindControl(EKeys::One,&APFSurvivalPlayerController::CraftTool,TEXT("Crafting"),TEXT("Craft primitive tool"));
    BindControl(EKeys::Two,&APFSurvivalPlayerController::CookFood,TEXT("Crafting"),TEXT("Cook food"));
    BindControl(EKeys::Three,&APFSurvivalPlayerController::DryFood,TEXT("Crafting"),TEXT("Dry food"));
    BindControl(EKeys::R,&APFSurvivalPlayerController::CancelCraft,TEXT("Crafting"),TEXT("Cancel craft; ingredients are not consumed"));
    // Building (M5) and attack (M6, left click outside overlays)
    BindControl(EKeys::B,&APFSurvivalPlayerController::ToggleBuilding,TEXT("General"),TEXT("Open / close building"));
    BindControl(EKeys::N,&APFSurvivalPlayerController::NextBuilding,TEXT("Building"),TEXT("Select next piece"));
    BindControl(EKeys::T,&APFSurvivalPlayerController::RotateBuilding,TEXT("Building"),TEXT("Rotate preview"));
    BindControl(EKeys::LeftMouseButton,&APFSurvivalPlayerController::PlaceBuilding,TEXT("Building / world"),TEXT("Place preview; attack when overlays are closed"));
    BindControl(EKeys::H,&APFSurvivalPlayerController::DemolishBuilding,TEXT("Building"),TEXT("Demolish owned targeted piece"));
    BindControl(EKeys::J,&APFSurvivalPlayerController::DamageBuilding,TEXT("Building"),TEXT("Developer damage to targeted piece"));
    BindControl(EKeys::U,&APFSurvivalPlayerController::StoreItem,TEXT("Building"),TEXT("Store one item in targeted owned storage"));
    BindControl(EKeys::O,&APFSurvivalPlayerController::TakeStoredItem,TEXT("Building"),TEXT("Take one item from targeted owned storage"));
}

void APFSurvivalPlayerController::BindControl(FKey Key,void (APFSurvivalPlayerController::*Handler)(),const TCHAR* Context,const TCHAR* Action,bool bWhenPaused)
{
    InputComponent->BindKey(Key,IE_Pressed,this,Handler).bExecuteWhenPaused=bWhenPaused;
    RegisteredControlHints.Add({Key,Context,Action});
}

TArray<FPFControlHint> APFSurvivalPlayerController::GetControlHints(bool bGamepad) const
{
    TArray<FPFControlHint> Result;
    // Query current player mappings, including context changes, rather than duplicating template keys.
    if(const auto* Enhanced=Cast<UEnhancedPlayerInput>(PlayerInput))
    {
        for(const auto& Mapping:Enhanced->GetEnhancedActionMappingsView())
        {
            if(!Mapping.Action || Mapping.Key.IsGamepadKey()!=bGamepad){continue;}
            FString Action=Mapping.Action->ActionDescription.ToString();
            if(Action.IsEmpty()){Action=Mapping.Action->GetName();Action.RemoveFromStart(TEXT("IA_"));}
            if(!Result.ContainsByPredicate([&](const FPFControlHint& H){return H.Key==Mapping.Key && H.Action==Action;}))
            {Result.Add({Mapping.Key,TEXT("Movement / view"),Action});}
        }
    }
    for(const auto& Hint:RegisteredControlHints){if(Hint.Key.IsGamepadKey()==bGamepad){Result.Add(Hint);}}
    return Result;
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
        if(PauseMenu){PauseMenu->CloseChildMenus();}
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
    else if(Building->TracedPiece()){Building->ServerTargetAction(PFBuildAction::Interact);}  // open door / storage
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
void APFSurvivalPlayerController::InventorySplit(){SendInventoryAction(PFInventoryAction::Split);}
void APFSurvivalPlayerController::InventoryDrop(){SendInventoryAction(PFInventoryAction::Drop);}
void APFSurvivalPlayerController::InventoryConsume(){SendInventoryAction(PFInventoryAction::Consume);}

void APFSurvivalPlayerController::SendInventoryAction(uint8 Action)
{
    const auto* I=GetInventory();
    if(bPauseMenuOpen || !bInventoryOpen || !I || I->GetStacks().IsEmpty()){return;}
    SelectedInventoryIndex=FMath::Clamp(SelectedInventoryIndex,0,I->GetStacks().Num()-1);
    const auto S=I->GetStacks()[SelectedInventoryIndex];
    // Only the stable stack GUID travels; the server looks it up in *its own* copy.
    ServerInventoryAction(S.StackId,Action,Action==PFInventoryAction::Split ? S.Quantity/2 : 1);
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
        if(Action==PFInventoryAction::Split){Accepted=I->Split(StackId,Quantity);}
        else if(Action==PFInventoryAction::Drop){Accepted=I->Drop(StackId,Quantity,GetPawn())!=nullptr;}
        else if(Action==PFInventoryAction::Consume && Quantity==1){Accepted=I->Consume(StackId,GetPawn());}
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
void APFSurvivalPlayerController::DemolishBuilding(){if(Building->bBuildMode){Building->ServerTargetAction(PFBuildAction::Demolish);}}
void APFSurvivalPlayerController::DamageBuilding(){if(Building->bBuildMode){Building->ServerTargetAction(PFBuildAction::Damage);}}

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
