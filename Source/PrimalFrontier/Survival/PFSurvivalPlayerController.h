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

UCLASS()
class PRIMALFRONTIER_API APFSurvivalPlayerController : public APrimalFrontierPlayerController
{
    GENERATED_BODY()
public:
    APFSurvivalPlayerController();
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UPFBuildingComponent> Building;
    UPROPERTY(EditDefaultsOnly, Category="Survival|UI") TSubclassOf<UPFSurvivalHUD> SurvivalHUDClass;
    UPROPERTY(Transient, BlueprintReadOnly, Category="Survival|UI") TObjectPtr<UPFSurvivalHUD> SurvivalHUD;
    UFUNCTION(BlueprintCallable, Category="Survival") void Interact();
    UPFInventoryComponent* GetInventory() const;
    UPFCraftingComponent* GetCrafting() const;
    bool IsCraftingOpen() const {return bCraftingOpen;}
    UFUNCTION(Server,Reliable) void ServerCraftAction(FName RecipeId,bool bCancel);
    bool IsInventoryOpen() const {return bInventoryOpen;}
    int32 GetSelectedInventoryIndex() const {return SelectedInventoryIndex;}
    const FString& GetInventoryMessage() const {return InventoryMessage;}
    // Only operations on the owning player's existing stack IDs. No item-grant RPC.
    UFUNCTION(Server,Reliable) void ServerInventoryAction(FGuid StackId,uint8 Action,int32 Quantity);
protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    UFUNCTION(Server, Reliable) void ServerInteract();
private:
    UPROPERTY() TObjectPtr<UPFBuildingHUD> BuildingHUD;
    void ToggleBuilding();
    void NextBuilding();
    void RotateBuilding();
    void PlaceBuilding();
    void DemolishBuilding();
    void DamageBuilding();
    void StoreItem();
    void TakeStoredItem();
    double NextInteractionTime = 0;
    double NextInventoryTime = 0;
    double NextCraftTime = 0;
    bool bCraftingOpen=false;
    UPROPERTY() TObjectPtr<UPFCraftingHUD> CraftingHUD;
    void ToggleCrafting();
    void CraftTool();
    void CookFood();
    void DryFood();
    void CancelCraft();
    bool bInventoryOpen=false;
    int32 SelectedInventoryIndex=0;
    FString InventoryMessage;
    UPROPERTY() TObjectPtr<UPFInventoryHUD> InventoryHUD;
    void ToggleInventory();
    void InventoryNext();
    void InventoryPrevious();
    void InventorySplit();
    void InventoryDrop();
    void InventoryConsume();
    void SendInventoryAction(uint8 Action);
    UFUNCTION(Client,Reliable) void ClientInventoryFeedback(const FString& Message);
};
