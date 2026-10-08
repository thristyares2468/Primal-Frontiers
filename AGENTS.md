# Primal Frontier development instructions

## Environment

* Engine: Unreal Engine 5.8.2
* Engine root: C:\\Program Files\\Epic Games\\UE\_5.8
* Platform: Windows 11
* Editor target: PrimalFrontierEditor
* Game target: PrimalFrontier
* Server target: PrimalFrontierServer
* Default build configuration: Development Editor Win64
* Project file: PrimalFrontier.uproject

## Project objective

Build a server-authoritative open-world survival game inspired by the
structure of ARK-style survival games, without copying protected names,
characters, maps, assets, sounds, story, or other expressive content.

## Architecture

* Implement authoritative gameplay, networking, persistence, inventory,
combat, crafting, construction, and creature logic in C++.
* Use Blueprints for presentation, asset composition, effects, UI, and tuning.
* Use data assets or data tables for item, recipe, creature, and structure data.
* Use Gameplay Tags rather than arbitrary state strings.
* Multiplayer gameplay must be server-authoritative.
* Important systems must support dedicated servers.
* Avoid hard references when soft references or asset manager references are appropriate.
* Keep classes focused and avoid oversized god objects.
* Expose tuning values to designers without putting core authority in Blueprints.

## Content

* Store project-owned assets under Content/PrimalFrontier.
* Use the project's established asset naming prefixes.
* Use placeholder assets until gameplay is validated.
* Never modify .uasset or .umap files as raw binary data.
* Change binary assets through Unreal Editor, Python, commandlets, or supported import tools.
* Do not rename or move assets without checking redirectors and references.

## Generated files

* Never edit files under Binaries, Intermediate, Saved, or DerivedDataCache.
* Do not commit generated build directories.
* Do not commit machine-specific editor settings.

## Verification

* Compile PrimalFrontierEditor after C++ changes.
* Run the narrowest relevant automated test.
* Check Saved/Logs/PrimalFrontier.log after launches and tests.
* Treat compiler warnings and ensure failures as problems to investigate.
* Do not claim an editor-facing feature works without testing it in Unreal.
* For multiplayer features, test a server and at least two clients when practical.
* Record evidence for important visual or gameplay changes.

## Change management

* Make small, reviewable commits.
* Preserve unrelated user changes.
* Explain new dependencies and plugins before adding them.
* Prefer reversible migrations.
* Update architecture and decision documents when changing a major system.
* When the user says continue, first inspect Trello for the next eligible task and reconcile it with Docs/CURRENT_STATE.md and Docs/MILESTONES.md. At the start of every project task, correct stale status and keep the bounded active task in Doing. Immediately after completing a task, update its Trello evidence/status before starting another task. Do not mark manual/unverified gates Done; preserve unrelated cards, deadlines and assignments. Follow Docs/TRELLO_SYNC.md.
* Every project Trello card title starts with (Personal) or (AI). Use (Personal) when the next required action must be performed by the user; explain the concrete action and why it needs them in the description. Use (AI) for independently actionable agent work and future agent implementation plans. Reevaluate the prefix when a dependency resolves; do not label every future milestone Personal just because it may eventually need playtesting. These labels indicate the next actor, not historical authorship.
* After every meaningful change and build/test result, update Docs/CURRENT_STATE.md with the current game state, completed work, verification gaps and next development step. Do not present implemented but untested work as passed.


## Camera and player presentation



* First person is the primary camera and gameplay experience.
* Design interaction, weapons, building placement, UI, and creature encounters for first person.
* Maintain a full-body replicated character mesh for remote players.
* Use separate first-person arms, camera, and item meshes for the locally controlled player.
* Keep camera switching architecture extensible so optional third person can be added later.
* Do not implement third-person gameplay or UI unless explicitly requested.

