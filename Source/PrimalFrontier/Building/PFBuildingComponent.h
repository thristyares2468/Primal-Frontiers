// PFBuildingComponent.h
//
// Building logic for one player, living on APFSurvivalPlayerController.
//
// Client side: tracks build mode, selected piece and rotation, and every 0.1 s
// draws a green/red wireframe preview of where the piece would go (advisory only).
//
// Server side: ServerPlace sends only a piece ID and a quarter turn. The server
// recomputes the placement from the player's own view (Candidate), re-checking
// reach, ownership, support, collision (including players), the 128-piece limit
// and the wood cost, then spawns the piece and charges the wood.
//
// Snapping rules (400 cm grid):
//  - Foundation: grid-snapped on flat ground (centre + four corners checked).
//  - Wall/Door:  on an owned platform's edge, facing the chosen quarter turn.
//  - Floor/Ceiling: 320 cm above the platform that holds the wall/door you aim at.
//  - Storage:    on an owned platform, snapped to a 100 cm sub-grid.
//
// History: M5 (5702d4b). Docs: Docs/BUILDING_M5.md

#pragma once
#include "Components/ActorComponent.h"
#include "Building/PFBuildingCatalog.h"
#include "PFBuildingComponent.generated.h"
class APFBuildPiece;
class APFSurvivalPlayerController;
UCLASS(ClassGroup=(PrimalFrontier),meta=(BlueprintSpawnableComponent))
class PRIMALFRONTIER_API UPFBuildingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPFBuildingComponent();

    /** Piece definitions; loaded from DA_BuildingCatalog at BeginPlay if unset. */
    UPROPERTY(EditAnywhere,BlueprintReadOnly) TObjectPtr<UPFBuildingCatalog> Catalog;
    /** Storage box currently opened with E (owner-only replicated); closes when out of reach. */
    UPROPERTY(Replicated,BlueprintReadOnly) TObjectPtr<APFBuildPiece> OpenStorage;

    // ---- Local (client) UI state; not replicated ----
    bool bBuildMode=false;
    int32 Selection=0;       // index into Catalog->Pieces
    int32 Rotation=0;        // quarter turns, 0..3
    FString PreviewMessage;  // why the current preview is valid/invalid
    FString Feedback;        // last server result

    /** Catalog ID of the selected piece (None if none). */
    FName SelectedId() const;

    /** Compute where piece Id would go from the player's current view. Fills Out (transform),
     *  Parent (supporting piece) and Reason (human-readable result). Used for both the
     *  client preview and the server's authoritative check. */
    bool Candidate(FName Id,int32 QuarterTurns,FTransform& Out,APFBuildPiece*& Parent,FString& Reason) const;

    // ---- Server-only actions (called by the RPCs below) ----
    /** Validate, spawn and charge wood. Returns the new piece or nullptr (reason in Feedback). */
    APFBuildPiece* Place(FName Id,int32 QuarterTurns);
    /** Remove an owned, aimed-at piece that supports nothing and holds nothing. No refund. */
    bool Demolish(APFBuildPiece* Piece);
    /** Toggle an owned door, or open/close an owned storage box. */
    bool Interact(APFBuildPiece* Piece);
    /** Move Quantity of a stack between the bag and OpenStorage (bDeposit = bag -> storage). */
    bool Transfer(bool bDeposit,FGuid StackId,int32 Quantity);

    /** The build piece under the player's view within 7 m (any owner), or nullptr. */
    APFBuildPiece* TracedPiece() const;
    APFSurvivalPlayerController* Controller() const;

    // ---- RPCs (owning client -> server), rate limited to one per 0.25 s ----
    UFUNCTION(Server,Reliable) void ServerPlace(FName Id,int32 QuarterTurns);
    /** Action on the aimed-at piece: 0 = demolish, 1 = interact (door/storage), 2 = 25 owner damage. */
    UFUNCTION(Server,Reliable) void ServerTargetAction(uint8 Action);
    UFUNCTION(Server,Reliable) void ServerTransfer(bool bDeposit,FGuid StackId,int32 Quantity);

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
    virtual void BeginPlay() override;
    /** Server: auto-close storage out of reach. Local player: draw the placement preview. */
    virtual void TickComponent(float Delta,ELevelTick Type,FActorComponentTickFunction* Tick) override;

private:
    double NextRequest=0;
    /** Controller has a living possessed survivor and a PlayerState. */
    bool Living() const;
    /** Piece is valid, owned by this player and within 7 m. */
    bool InReach(APFBuildPiece* Piece) const;
    /** Server-side request throttle; false if called too soon or without authority. */
    bool RateLimit();
    /** Server -> owning client result text. */
    UFUNCTION(Client,Reliable) void ClientResult(const FString& Message);
};
