// PFRequestCodes.h
//
// Names for the small uint8 action codes that clients send in server RPCs. The RPC
// signatures stay uint8 (and the automation tests send the raw values), so these
// names only make call sites readable. Never renumber an existing code; unknown codes
// are ignored by the server.
#pragma once

#include "CoreMinimal.h"

/** Codes for APFSurvivalPlayerController::ServerInventoryAction (M3). */
namespace PFInventoryAction
{
    inline constexpr uint8 Split=0;    // move Quantity into a new stack
    inline constexpr uint8 Drop=1;     // drop Quantity as a world pickup in front of the pawn
    inline constexpr uint8 Consume=2;  // eat one item (Quantity must be 1)
}

/** Codes for UPFBuildingComponent::ServerTargetAction on the traced piece (M5). */
namespace PFBuildAction
{
    inline constexpr uint8 Demolish=0;  // owner removes a piece with no dependents and empty storage
    inline constexpr uint8 Interact=1;  // open storage or toggle a door
    inline constexpr uint8 Damage=2;    // owner hammer hit (25 damage)
}
