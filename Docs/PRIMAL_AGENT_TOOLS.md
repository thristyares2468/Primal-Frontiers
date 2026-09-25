# Primal Agent Tools

The plugin now separates non-Shipping runtime developer commands from editor-only asset and fixture tools. See [the command interface](../Plugins/PrimalAgentTools/DEVELOPER_COMMANDS.md), [plugin documentation](../Plugins/PrimalAgentTools/README.md), and [verification evidence](../Plugins/PrimalAgentTools/VERIFICATION.md).

Runtime integration uses Unreal authority/collision APIs and the project survival component. Health/Stamina and M2 hunger/thirst/exposure have real adapters; inventory, creatures, world time and persistence remain explicit blockers. See `SURVIVAL_M2.md` for command and live-test contracts. No MCP server is added.
