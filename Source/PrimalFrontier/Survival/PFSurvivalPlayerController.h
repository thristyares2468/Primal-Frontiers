// PFSurvivalPlayerController.h
//
// The survivor's PlayerController: the client-side hub that turns key/gamepad
// presses into *requests* and sends them to the server through RPCs. The server
// re-validates every request (life state, reach, ownership, rate limits) before
// changing inventory, crafting, building or creature state. Clients never send
// item IDs to grant, damage amounts, targets or positions.
//
// It also creates the native placeholder HUD widgets (survival, inventory,
// crafting, building, pause menu) for the local player and keeps the overlays
// mutually exclusive so contextual buttons are unambiguous.
//
// Split across files: keyboard bindings and RPCs here, gamepad bindings in
// PFGamepadInput.cpp. Building RPCs live on UPFBuildingComponent.
//
// History: grew one milestone at a time (M1 HUD, M2 interact, M3 inventory,
//          M4 crafting, M5 building, M6 attack, M7 pause/gamepad/prompts).
// Docs:    Docs/INVENTORY_M3.md, Docs/GATHERING_CRAFTING_M4.md, Docs/BUILDING_M5.md,
//          Docs/CREATURES_M6.md, Docs/PLAYTEST.md (controls)

#pragma once
#include "PrimalFrontierPlayerController.h"
#include "PFSurvivalPlayerController.generated.h"
class UPFSurvivalHUD;
class UPFInventoryHUD;
class UPFInventoryComponent;
class UPFCraftingComponent;
class UPFCraftingHUD;
class UPFBuildingComponent;
class UPFBuildingHUD;
class UPFPauseMenu;

UCLASS()
class PRIMALFRONTIER_API APFSurvivalPlayerController : public APrimalFrontierPlayerController
{
    GENERATED_BODY()

public:
    APFSurvivalPlayerController();

    /** Building mode state, placement preview and building RPCs (lives on the controller). */
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UPFBuildingComponent> Building;

    /** HUD class to create; a Blueprint subclass can replace the native placeholder. */
    UPROPERTY(EditDefaultsOnly, Category="Survival|UI") TSubclassOf<UPFSurvivalHUD> SurvivalHUDClass;
    UPROPERTY(Transient, BlueprintReadOnly, Category="Survival|UI") TObjectPtr<UPFSurvivalHUD> SurvivalHUD;

    /** E / Pad X: pick up, gather or eat a ration in reach; otherwise open an owned door/storage. */
    UFUNCTION(BlueprintCallable, Category="Survival") void Interact();

    // ---- Pause menu (local UI only; world pauses only in Standalone) ----
    void SetPauseMenuOpen(bool bOpen);
    bool IsPauseMenuOpen() const {return bPauseMenuOpen;}

    // ---- Text for the HUD ----
    /** Context prompt for whatever the crosshair is on (e.g. "E / Pad X - Pick up Wood x5"). */
    FString InteractionPrompt() const;
    /** Last server feedback message, shown for 4 seconds after it arrives. */
    FString RecentInteractionMessage() const;

    // ---- Accessors for components that live on the PlayerState ----
    UPFInventoryComponent* GetInventory() const;
    UPFCraftingComponent* GetCrafting() const;

    // ---- Crafting ----
    bool IsCraftingOpen() const {return bCraftingOpen;}
    /** Ask the server to start RecipeId or cancel the active job. */
    UFUNCTION(Server,Reliable) void ServerCraftAction(FName RecipeId,bool bCancel);

    // ---- Combat ----
    /** Ask the server to attack whatever creature the survivor's own view trace hits. */
    UFUNCTION(Server,Reliable) void ServerAttackCreature();

    // ---- Inventory ----
    bool IsInventoryOpen() const {return bInventoryOpen;}
    int32 GetSelectedInventoryIndex() const {return SelectedInventoryIndex;}
    const FString& GetInventoryMessage() const {return InventoryMessage;}
    // Only operations on the owning player's existing stack IDs. No item-grant RPC.
    /** Action codes (PFInventoryAction in PFRequestCodes.h): 0 = split Quantity off, 1 = drop Quantity, 2 = eat one (Quantity must be 1). */
    UFUNCTION(Server,Reliable) void ServerInventoryAction(FGuid StackId,uint8 Action,int32 Quantity);

protected:
    /** Local player only: create and add the placeholder HUD widgets. */
    virtual void BeginPlay() override;
    /** Keyboard bindings (gamepad bindings are added by BindGamepadControls). */
    virtual void SetupInputComponent() override;
    /** Server half of Interact(): re-traces from the pawn's own eyes and acts on the target. */
    UFUNCTION(Server, Reliable) void ServerInteract();

private:
    // ---- Gamepad routing (PFGamepadInput.cpp) ----
    // Face/D-pad buttons are contextual: they act on whichever overlay is open.
    void BindGamepadControls();
    void GamepadPrimary();
    void GamepadBack();
    void GamepadUp();
    void GamepadDown();
    void GamepadLeft();
    void GamepadRight();

    // ---- Pause state ----
    UPROPERTY() TObjectPtr<UPFPauseMenu> PauseMenu;
    bool bPauseMenuOpen=false;
    bool bPausedWorld=false;      // true only if we actually paused a Standalone world
    double MessageUntil=0;        // real time until which InventoryMessage is shown
    void TogglePauseMenu();

    // ---- Building input (forwards to UPFBuildingComponent RPCs) ----
    UPROPERTY() TObjectPtr<UPFBuildingHUD> BuildingHUD;
    void ToggleBuilding();
    void NextBuilding();
    void RotateBuilding();
    void PlaceBuilding();         // also performs the melee attack when no overlay is open
    void DemolishBuilding();
    void DamageBuilding();
    void StoreItem();
    void TakeStoredItem();

    // ---- Server-side rate limits (world time of the next accepted request) ----
    double NextInteractionTime = 0;
    double NextInventoryTime = 0;
    double NextCraftTime = 0;
    double NextAttackTime = 0;

    // ---- Crafting input ----
    bool bCraftingOpen=false;
    UPROPERTY() TObjectPtr<UPFCraftingHUD> CraftingHUD;
    void ToggleCrafting();
    void CraftTool();
    void CookFood();
    void DryFood();
    void CancelCraft();

    // ---- Inventory input ----
    bool bInventoryOpen=false;
    int32 SelectedInventoryIndex=0;
    FString InventoryMessage;     // last feedback from the server (any system, not just inventory)
    UPROPERTY() TObjectPtr<UPFInventoryHUD> InventoryHUD;
    void ToggleInventory();
    void InventoryNext();
    void InventoryPrevious();
    void InventorySplit();
    void InventoryDrop();
    void InventoryConsume();
    /** Send ServerInventoryAction for the selected stack (split sends half, others send 1). */
    void SendInventoryAction(uint8 Action);

    /** Server -> owning client feedback text ("Picked up", "Refused: ..."). */
    UFUNCTION(Client,Reliable) void ClientInventoryFeedback(const FString& Message);
};
