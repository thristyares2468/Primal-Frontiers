# M9 — controlled asset pipeline planning

This is a documentation/read-only audit increment authorized while M7/M8 manual acceptance waits. It does not approve imported packs, start a final art pass or certify a new playable milestone. Existing greybox mechanics remain the baseline. No new asset is needed to finish the current manual survival/save tests.

## Intake and provenance

Before using a third-party asset, record its exact source URL, title, creator, acquisition date, license/version and attribution requirements, original engine/file format, required plugins, source dimensions/scale/skeleton and intended role. Keep private receipts/account information outside Git; retain a private proof reference. A generic list saying free, a familiar folder name or an import by Claude is not evidence of rights or compatibility. ASSET_SOURCES.md is a historical candidate shortlist, not intake approval.

The initial inventory reads saved package statistics and Asset Registry metadata only. It never calls GetAsset, loads models/textures or writes assets. Registry tags can be absent/stale. Disk bytes do not measure resident CPU/GPU memory; class presence does not prove an animation retargets or a mesh has valid collision/LODs. No render/cook/license validation is implied.

Existing packs outside Content/PrimalFrontier are grandfathered for audit; do not move them to satisfy a folder policy. Future approved project-owned content goes under Content/PrimalFrontier/{Characters,Creatures,Environment,Structures,Items,UI,Audio,Materials}, with isolated source-pack composition and a manifest. Keep original intake and runtime wrapper references distinct. Never copy another game's expressive creatures, characters, maps, UI or sounds. Existing Epic template assets remain separate.

## Naming and authority

Use the established prefixes: L_, BP_, BPI_, UI_, ABP_, SM_, SKM_, SK_, M_, MI_, MF_, T_, DA_, DT_, IA_, IMC_, S_, SC_, NS_. Follow PRIMAL_AGENT_TOOLS.md / plugin naming policy for class coverage. Project-owned identifiers must be unique, descriptive and stable. Do not bulk rename legacy imports or update binary paths outside Unreal.

Authoritative item/creature/structure IDs and Gameplay Tags stay stable across presentation replacement. Gameplay/networking remains C++; Blueprint wrappers provide meshes, animations, effects, UI and tuning. Do not replace validated inventory, crafting, building, creature or save logic with pack gameplay Blueprints. Preserve separate local first-person arms/items and the remote full-body mesh. No third-person gameplay is introduced.

## Initial performance budgets (proposed, not benchmarks)

Default to existing simple materials and a small representative sample, not a whole pack demo map. Aim for 1K ordinary textures; 2K only for a measured nearby visual need. Any 4K+ texture needs an explicit measured justification before runtime integration. No blanket texture resizing/reimport is authorized by this plan.

Prefer opaque simple master materials plus instances; target one or two slots per small prop. Review transparent/foliage overdraw, displacement, heavy layered shaders, Niagara and material dependency fan-out separately. Do not increase graphics quality, add heavy effects or compile a whole pack's shaders for intake.

Use inexpensive simple collision for pickups/tools/props and predictable structure collision matching current build-grid bounds. Navigation obstacles and terrain must be tested with actual player/creature capsules. Never use visual-only meshes as gameplay authority. Validate LOD screen transitions, triangle reduction and draw calls in one representative rendered scene before broad deployment; registry counts are only screening metadata. Nanite availability is not a memory/performance pass.

Maintain the 16 GB runtime target as a budget even though this host now reports about 32 GB. Benchmark one rendered client with t.MaxFPS 0, r.VSync 0; record frame smoothing, actual resolution/render scale/adapter, CPU/GPU frame times, FPS distribution, working/private memory, VRAM and traversal stutter. Use NullRHI for correctness/metadata/network tests. Compare against the same greybox scene/settings before approving an art replacement. No native-1440p/minimum-spec pass is inferred from older small-window or stationary captures.

## Reversible replacement and validation

1. Resolve the actual asset provenance/usage rights and engine/plugin requirements. Record approval for a single candidate and the intended gameplay role.
2. Inspect that candidate through supported Unreal APIs/editor in an isolated test scene; check centimetre scale, pivot, bounds, materials/textures, collision, LODs, skeleton/animations and soft-reference dependencies. Do not modify the source pack or the open-world map yet.
3. Add a small project-owned presentation wrapper/tuning reference, preserving gameplay IDs, interaction bounds, replicated bodies, support dimensions and navigation. Keep a reversible reference to the current placeholder. Use soft references where suitable; avoid loading a pack at startup.
4. Build if C++ changed; run the relevant existing regression and PF.ValidateAssets/PF.CheckNaming/PF.CheckReferences on the bounded scope. Existing project starter-reference failures remain separate; do not hide them behind an art pass.
5. Playtest first-person reach/tool visibility, gather/craft, placement/storage, creature combat, death/respawn and save/reconnect where affected. Capture a before/after single-client performance sample and a screenshot. Run low-memory multiplayer validation if replication/presentation changed.
6. Approve broader use only after those checks pass. Document exact source/limits in CURRENT_STATE, MILESTONES, DECISIONS and Trello; commit only that reviewed replacement. No bulk conversion, shader build, imported demo-map expansion or reset of user work.

## Redirectors and Git/LFS

Never fix redirectors automatically during inventory. First list dependencies/referencers and identify affected maps/external actors/data assets. A separately authorized move/rename must use Unreal Editor/AssetTools, save only affected assets, run scoped reference validation and verify loads/cooks before any obsolete redirector is removed. Keep unrelated starter content findings unchanged.

Git LFS 3.7.1 is installed; current .gitattributes tracks *.uasset, *.umap and *.psd. Source/config/docs stay ordinary Git; Binaries, Intermediate, Saved, DerivedDataCache and machine settings stay ignored. Inspect scoped status and staged diff, use normal pushes and verify remote HEAD; no force push/history rewrite. Raw source art/audio needs an explicit size/storage/tracking review before it is added—this plan changes no patterns and imports nothing. Do not assume a public repository can redistribute source packs; verify the actual license/provenance separately.

## Audit evidence and remaining work

Scripts/AuditExistingAssets.py completed in an isolated UE 5.8.3 Python commandlet, engine exit 0. Report: Saved/AutomationReports/M9AssetInventory_20261008T030703Z_8a96b8c959134bbdb6908726df977477/inventory.json and summary.md; log Saved/Logs/PFM9AssetInventory.log. No warning/error/ensure/fatal lines were found. InventoryCompleted is a metadata verdict, not asset approval. Git showed no asset/map changes.

Measured inventory: **7,268 registry assets, 7,270 package files, 44,799,493,793 bytes (41.723 GiB), 13 redirectors, zero registry packages without matching package files**. Script loaded/saved zero asset objects. Sampled process working/private memory **1.820/1.720 GiB**, NullRHI; no runtime scene/FPS measurement. Package/asset counts differ because packages can contain multiple assets; do not interpret the difference as missing content.

Largest existing folders: OldWest 18.661 GiB; Megaplant_Library 8.005; Brushify 4.658; Adventures_Pack 3.075; Singapore_Canal 2.084; Modular_Rural_Cabin 1.545. Registry has 1,829 Texture2D, 1,660 StaticMesh, 771 SkeletalMesh and 225 AnimSequence assets. Texture Dimensions tags list **1,014 at 4096** and **94 at 8192** maximum dimensions. These are screening candidates for a future memory review, not proof of current runtime residency or the cause of low FPS. Do not resize/reimport them in this audit.

Redirectors are listed in the report, including legacy FirstPerson/Lvl_FirstPerson, Bike materials and imported Megaplant/Polyphoria entries. None were fixed/deleted. PrimalFrontier has 27 registry assets including eight Worlds, four gameplay catalogs, three SoundClasses and one SoundMix; user imports remain in their original folders.

Primary-source research (October 8): [Modular Rural Cabins](https://www.fab.com/listings/508fe84a-4976-4cfe-9a40-c2b9533da601) by Maarten Hof is currently listed free, in Unreal format, and its description claims an April 2026 Nanite/Lumen update. The public view did not expose exact license terms or supported engine versions and showed an AI-use field of No. This is a possible source match for the 587 local package files/160 static meshes, not verified acquisition or compatibility. Record the actual pack version/license and clarify any applicable AI-use terms before candidate processing/integration. The [Fab license summary](https://www.fab.com/eula) describes incorporated-project use and private collaborator sharing separately from standalone source redistribution; do not treat free pricing as blanket redistribution approval.

M7/M8 manual acceptance is still required before gated first-art/gameplay integration. Existing pack source/proof mapping must be supplied or verified before any specific imported asset is approved. First-person tool animation suitability, original creature visuals, collision/LOD budgets and actual authored audio coverage remain unknown until candidate-level inspection. Do not request/download replacement assets solely because the inventory lacks a confirmed candidate.
