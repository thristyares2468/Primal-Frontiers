# Primal Agent Tools

The plugin now separates non-Shipping runtime developer commands from editor-only asset and fixture tools. See [the command interface](../Plugins/PrimalAgentTools/DEVELOPER_COMMANDS.md), [plugin documentation](../Plugins/PrimalAgentTools/README.md), and [verification evidence](../Plugins/PrimalAgentTools/VERIFICATION.md).

Runtime integration uses Unreal authority/collision APIs and the project survival component. Health/Stamina and M2 hunger/thirst/exposure have real adapters. M3 adds validated inventory grant/removal, M5 adds building integrity/reset, and M6 adds bounded creature spawning/reset/integrity. M7 world time and M8 PF.SaveWorld/PF.LoadWorld/PF.TestPersistence now use real server APIs; 26 focused regressions and one-/two-client NullRHI create/restart pass. Their full manual milestone gates remain open. See the command interface and PERSISTENCE_M8.md for exact scope. No MCP server is added.

## Milestone 4 integration

The runtime backend implements `PF.TestGathering` (resource integrity), `PF.TestCrafting` (recipe/queue integrity), `PF.Craft RecipeId` and `PF.CancelCraft`. Functional coverage uses `PF.Crafting.Transactions`, `PF.Crafting.Gathering` and opt-in `PF.Crafting.Live` with `-PFRunCraftingLiveTests -PFExpectedPlayers=1|2`. See `GATHERING_CRAFTING_M4.md` and `MILESTONES.md` for scope and verification. All gameplay commands remain server-only and non-Shipping.
