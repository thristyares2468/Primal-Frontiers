# M3 — 999k protocol and transaction boundary

October12,2026. This prerequisite raises the supported maximum to999,000, including catalog validation, inventory insertion/craft output, owned-stack RPCs, chest transfers and button amounts. Double multiplication protects weight accumulation. Per-item defaults and hard carrying limits are unchanged in this checkpoint; playable encumbrance/traversal remains the next task.

Editor builds succeeded23.63s then4.80s, Game29.04s; no compiler warnings. Logs Saved/Logs/PFLargeStackEditorBuild.log, PFLargeStackFinalEditorBuild.log, PFLargeStackGameBuild.log.

Native initial5/5: Automation_LargeStacks_20261012_051304650_c59ad009. Final3/3: Automation_LargeStacksFinal_20261012_052001877_9dab49d5, PF.Inventory.LargeStacks, PF.Inventory.WorldTransfers, PF.Building.PlacementAndStorage. Exact999,000 archive encode/decode,333,000 repeated runtime restore, split/transfer conservation, capacity/invalid/client-role refusals tested. Native raw/test severity0, exact selections/exits0/no timeout/fatal/ensure. Peak3.018working/2.867privateGiB.

Existing rendered PF.UI.BuildingMenusLive passed1/1 at720p/HUD1.5: M11BuildingMenus720_20261012_052138555_1a49cbbb;38.43s,3.126working/5.337privateGiB. Seven PNGs inspected under Saved/AutomationReports/ControlsUI/<run>/. Chest/filter/capacity refusal and picker/wall/window/stairs regression passed. Raw26warnings/14experimental engine Python errors match normalized prior baseline; no new gameplay error/fatal/ensure.372unrelated saves/default settings unchanged. This is not a network999k or human/performance certificate.

Native command: `Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Inventory.LargeStacks+PF.Inventory.WorldTransfers+PF.Building.PlacementAndStorage' -Label LargeStacksFinal -TimeoutSeconds 180`. Rendered command: `Scripts/RunControlsAutomation.ps1 -TestCase BuildingMenus -Resolution 720 -HUDScale 1.5 -TimeoutSeconds 180`. Reports index.json/run-summary.json live under Saved/AutomationReports/<run>/; logs under Saved/Logs/.

Trello https://trello.com/c/5JX9gnCr is this bounded prerequisite. Parent https://trello.com/c/DMKjODqr remains open. No assets/schema/private saves changed. Installed engine5.8.3 differs from requested5.8.2; host31.93GiB, minimum16GB certification remains unverified.
