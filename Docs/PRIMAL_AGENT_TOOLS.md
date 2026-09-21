# Primal Agent Tools

The plugin now separates non-Shipping runtime developer commands from editor-only asset and fixture tools. See [the command interface](../Plugins/PrimalAgentTools/DEVELOPER_COMMANDS.md), [plugin documentation](../Plugins/PrimalAgentTools/README.md), and [verification evidence](../Plugins/PrimalAgentTools/VERIFICATION.md).

Runtime integration uses the existing character/controller inheritance and Unreal authority/collision APIs. Survival inventory, attributes, creatures, world time and persistence are not implemented in this checkout. Commands for them remain explicit blockers. No MCP server is added.
