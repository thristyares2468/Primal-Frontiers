// PFInteraction.h
//
// One shared "what is the survivor looking at?" query, used by BOTH the local HUD
// prompt and the server's authoritative interaction. Keeping a single function
// guarantees the prompt never promises something the server will refuse.
//
// History: M7 (b5a3165) — extracted after playtests showed small dropped items
//          were hard to target; added a 12 cm aim tolerance with an occlusion check.
// Docs:    Docs/DECISIONS.md (2026-10-04 Interaction alignment)

#pragma once
#include "CoreMinimal.h"
class APawn;
class AActor;
namespace PFInteraction
{
    // Same server-derived view query for prompts and authoritative interaction.
    /** Returns the interactable actor (item pickup, resource node or ration) under the
     *  pawn's eye ray within Reach cm, or nullptr. Reach is limited to (0, 500];
     *  gameplay uses the 250 cm default, the HUD uses 500 for a "move closer" hint. */
    PRIMALFRONTIER_API AActor* FindTarget(const APawn* Pawn, float Reach=250.f);
}
