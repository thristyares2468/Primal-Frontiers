#include "Persistence/PFPlayerSaveAdapter.h"
#include "Inventory/PFInventoryComponent.h"
#include "Inventory/PFInventoryPlayerState.h"
#include "Survival/PFSurvivalPlayerController.h"
#include "Survival/PFSurvivorCharacter.h"
#include "Survival/PFPlayerSurvivalComponent.h"
#include "Crafting/PFCraftingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"

namespace
{
    bool Ready(APFSurvivalPlayerController* PC, FString& Error)
    {
        if (!IsInGameThread() || !IsValid(PC) || !PC->HasAuthority() || !PC->GetWorld()->IsGameWorld() ||
            !PC->GetPlayerState<APFInventoryPlayerState>() || !Cast<APFSurvivorCharacter>(PC->GetPawn()) ||
            !PC->GetInventory() || !PC->GetInventory()->Catalog)
        {
            Error = TEXT("Requires authoritative survivor, PlayerState and catalog"); return false;
        }
        return true;
    }
    FPFPlayerSaveLimits Limits(APFSurvivalPlayerController* PC)
    {
        FPFPlayerSaveLimits Result;
        Result.Slots = PC->GetInventory()->SlotLimit; Result.Weight = PC->GetInventory()->WeightLimit;
        const auto V = CastChecked<APFSurvivorCharacter>(PC->GetPawn())->Survival->GetVitals();
        Result.MaxHealth = V.MaxHealth; Result.MaxStamina = V.MaxStamina; return Result;
    }
}

TArray<FPFSavedItemStack> FPFPlayerSaveAdapter::CaptureInventory(const UPFInventoryComponent& Inventory)
{
    TArray<FPFSavedItemStack> Result;
    const double Now = UPFInventoryComponent::ServerTime(Inventory.GetWorld());
    for (const auto& Stack : Inventory.GetStacks())
    {
        if (Stack.ExpiresAt > 0 && Stack.ExpiresAt <= Now) { continue; }
        Result.Add({Stack.StackId, Stack.ItemId, Stack.Quantity, Stack.ExpiresAt > 0 ? Stack.ExpiresAt - Now : 0});
    }
    return Result;
}

bool FPFPlayerSaveAdapter::Capture(APFSurvivalPlayerController* PC, FPFPlayerSaveData& Out, FString& Error)
{
    if (!Ready(PC, Error)) { return false; }
    FPFPlayerSaveData Data;
    Data.PlayerId = PC->GetPlayerState<APFInventoryPlayerState>()->PersistentPlayerId;
    Data.Location = PC->GetPawn()->GetActorLocation(); Data.Rotation = PC->GetControlRotation().GetNormalized();
    const auto V = CastChecked<APFSurvivorCharacter>(PC->GetPawn())->Survival->GetVitals();
    Data.Health = V.Health; Data.Stamina = V.Stamina; Data.Hunger = V.Hunger; Data.Thirst = V.Thirst;
    Data.Inventory = CaptureInventory(*PC->GetInventory());
    if (!FPFPlayerSaveFormat::Validate(Data, *PC->GetInventory()->Catalog, Limits(PC), Error)) { return false; }
    Out = MoveTemp(Data); return true;
}

bool FPFPlayerSaveAdapter::CanRestore(APFSurvivalPlayerController* PC, const FPFPlayerSaveData& Data,
    double Age, FString& Error)
{
    if (!Ready(PC, Error)) { return false; }
    auto* Pawn = CastChecked<APFSurvivorCharacter>(PC->GetPawn());
    if (Pawn->Survival->IsDead() || Data.PlayerId != PC->GetPlayerState<APFInventoryPlayerState>()->PersistentPlayerId)
    {
        Error = TEXT("Player identity mismatch or current pawn awaits respawn"); return false;
    }
    if (!FPFPlayerSaveFormat::Validate(Data, *PC->GetInventory()->Catalog, Limits(PC), Error)) { return false; }
    TArray<FPFItemStack> Proposed;
    if (!PC->GetInventory()->PreparePersistence(Data.Inventory, Age, Proposed, Error)) { return false; }
    FCollisionQueryParams Params(SCENE_QUERY_STAT(PFSaveSpawn), false, Pawn);
    const auto* Capsule = Pawn->GetCapsuleComponent();
    FHitResult Ground;
    if (Data.Location.Z < PC->GetWorld()->GetWorldSettings()->KillZ ||
        PC->GetWorld()->OverlapBlockingTestByChannel(Data.Location, FQuat::Identity, ECC_Pawn,
            FCollisionShape::MakeCapsule(Capsule->GetScaledCapsuleRadius(), Capsule->GetScaledCapsuleHalfHeight() - 1), Params) ||
        !PC->GetWorld()->LineTraceSingleByChannel(Ground, Data.Location, Data.Location - FVector(0, 0, 10000), ECC_Visibility, Params) ||
        Ground.ImpactNormal.Z < 0.6)
    {
        Error = TEXT("Saved player location has no safe capsule clearance/walkable ground"); return false;
    }
    return true;
}

bool FPFPlayerSaveAdapter::Restore(APFSurvivalPlayerController* PC, const FPFPlayerSaveData& Data,
    double Age, FString& Error)
{
    if (!CanRestore(PC, Data, Age, Error)) { return false; }
    auto* Pawn = CastChecked<APFSurvivorCharacter>(PC->GetPawn());
    const FVector PreviousLocation = Pawn->GetActorLocation(); const FRotator PreviousRotation = Pawn->GetActorRotation();
    if (!Pawn->TeleportTo(Data.Location, FRotator(0, Data.Rotation.Yaw, 0), false, false))
    {
        Error = TEXT("Collision-aware player teleport failed"); return false;
    }
    if (!PC->GetInventory()->RestorePersistence(Data.Inventory, Age, Error))
    {
        Pawn->TeleportTo(PreviousLocation, PreviousRotation, false, true); return false;
    }
    if (PC->GetCrafting()) { PC->GetCrafting()->Cancel(); }
    PC->SetControlRotation(Data.Rotation); Pawn->GetCharacterMovement()->StopMovementImmediately();
    Pawn->Survival->SetHunger(Data.Hunger); Pawn->Survival->SetThirst(Data.Thirst);
    Pawn->Survival->ChangeStamina(Data.Stamina - Pawn->Survival->GetVitals().Stamina);
    Pawn->Survival->SetHealth(Data.Health); Pawn->ForceNetUpdate();
    UE_LOG(LogPFSurvival, Display, TEXT("[PrimalPersistence] Restored player inventory/vitals/location on server"));
    return true;
}
