# PrimalAgentTools decision — 2026-09-12

Primal Frontier's local validation and development fixture tooling lives in an isolated C++ Editor module at `Plugins/PrimalAgentTools`. It exposes seven `PF.*` console commands, with no MCP server or network service. See the plugin README for commands, safety boundaries, naming rules, and verification instructions.

Core gameplay and dedicated-server authority remain in runtime C++. Editor tooling uses Unreal's Data Validation subsystem, Asset Registry, Undo transactions, native actor creation, viewport readback, and JSON export. DataValidation is the only added plugin dependency; no Python or Editor Scripting Utilities dependency is needed.

Scenario commands operate only on tagged, native Engine actors in the currently open approved `L_Automation` map. They do not clear maps, switch maps, save assets, or fix redirectors automatically. Existing `/Game/Maps/L_Automation` remains supported without an unsolicited content migration. New project content belongs under `/Game/PrimalFrontier`.

Shipping exclusion is enforced at the project reference, plugin module descriptor, module build rules, and command compile guards. There is no cookable plugin content. Test fixture actors are editor-only and have no references to plugin-defined actor classes.

Structured reports and screenshots are generated under `Saved/AutomationReports`; incomplete coverage is reported explicitly. Package-level reference validation cannot prove dynamically generated or object-level references. This plugin is a development aid and is not a replacement for dedicated-server, two-client, persistence, or gameplay verification.
