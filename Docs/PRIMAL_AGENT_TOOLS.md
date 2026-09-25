# Primal Agent Tools

The plugin now separates non-Shipping runtime developer commands from editor-only asset and fixture tools. See [the command interface](../Plugins/PrimalAgentTools/DEVELOPER_COMMANDS.md), [plugin documentation](../Plugins/PrimalAgentTools/README.md), and [verification evidence](../Plugins/PrimalAgentTools/VERIFICATION.md).

Runtime integration uses Unreal authority/collision APIs and the project survival component. Health/Stamina and M2 hunger/thirst/exposure have real adapters. M3 adds validated inventory grant/removal; creatures, world time and persistence remain explicit blockers. See `SURVIVAL_M2.md` and `INVENTORY_M3.md` for command and live-test contracts. No MCP server is added.
