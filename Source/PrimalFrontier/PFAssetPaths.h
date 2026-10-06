// PFAssetPaths.h
//
// Object paths of the project-owned catalog data assets. Components fall back to
// loading these when no catalog was assigned in the editor. Keep this the only place
// the paths are written: if an asset is moved or renamed (check redirectors first),
// update it here. Long term, prefer soft references or the Asset Manager (AGENTS.md).
// The assets are created by Scripts/Setup*Milestone*.py.
#pragma once

#include "CoreMinimal.h"

namespace PFAssetPaths
{
    /** UPFItemCatalog (M3, Scripts/SetupInventoryMilestone3.py). */
    inline constexpr const TCHAR* ItemCatalog=TEXT("/Game/PrimalFrontier/Items/DA_ItemCatalog.DA_ItemCatalog");
    /** UPFCraftingCatalog: recipes and resource nodes (M4, Scripts/SetupGatheringMilestone4.py). */
    inline constexpr const TCHAR* CraftingCatalog=TEXT("/Game/PrimalFrontier/Crafting/DA_CraftingCatalog.DA_CraftingCatalog");
    /** UPFBuildingCatalog (M5, Scripts/SetupBuildingMilestone5.py). */
    inline constexpr const TCHAR* BuildingCatalog=TEXT("/Game/PrimalFrontier/Building/DA_BuildingCatalog.DA_BuildingCatalog");
    /** UPFCreatureCatalog (M6, Scripts/SetupCreaturesMilestone6.py). */
    inline constexpr const TCHAR* CreatureCatalog=TEXT("/Game/PrimalFrontier/Creatures/DA_CreatureCatalog.DA_CreatureCatalog");
}
