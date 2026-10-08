# Primal Frontier – Free Asset List

Historical candidate shortlist; prices, rights and UE compatibility below were not comprehensively verified and must not be treated as intake approval. A plain file extension alone does not establish skeleton, material, collision or rendering compatibility. Follow ASSET_PIPELINE_M9.md and confirm the exact source/license/acquisition evidence before selecting an asset. Nothing in this list authorizes a download/import or source-pack redistribution. The original recommendations below are retained as historical research.

Compiled 2026-10-06 and reformatted for readability.

---

## How to read this list

**License: can I use it?**

| Tag | Meaning |
|---|---|
| 🟢 **CC0** | Public domain. Use freely, no credit needed. |
| 🟡 **CC-BY** | Free, but you **must credit the author** in the game's credits (see the end of this file). |
| 🔵 **Epic** | Free from Epic. Commercial use allowed, **Unreal projects only**. |
| 🔵 **Fab Free** | Free on Fab, commercial use allowed. |
| 🟠 **Unity Free** | Free, but it comes as a Unity package. You need the free Unity editor to unpack it. |
| ⚪ **Check** | Search results said free, but nobody confirmed it on the page. Check the price before claiming. |

**Will it work in Unreal Engine 5.8.3?**

| Mark | Meaning |
|---|---|
| ✅ **Yes** | A plain file format (FBX, glTF, OBJ, PNG, WAV, SVG). Imports into any Unreal 5 version, including 5.8.3. |
| ✅ **Built in** | Already part of Unreal 5.8.3. Nothing to download. |
| ☑️ **Probably** | A UE5 Fab pack. These usually upgrade fine. On the Fab page, check that "Supported Engine Versions" includes 5.8 (or open it in 5.8 and let Unreal convert it). |
| ⚠️ **Older** | Made for UE4 or early UE5. It can usually be opened in 5.8 by converting a copy, but materials or Blueprints may need fixing. Test it before relying on it. |
| 🔧 **Convert** | Not an Unreal file. Export to FBX first (e.g. Unity package → FBX). |

> **Honesty note:** fab.com couldn't be opened from the machine that wrote this list. So the Fab entries' supported engine versions weren't read off the pages. They're marked from what kind of pack they are. The plain-file entries (Sketchfab, Poly Haven, Quaternius, Kenney, audio, icons) work in 5.8.3 regardless, because Unreal imports those formats directly.

**Before you add anything**
- Put imports under `Content/PrimalFrontier/` (project rule in `AGENTS.md`).
- **Sketchfab:** only download models with a **Download** button and a **CC0 or CC-BY** license. Skip anything marked NonCommercial or NoDerivs, and anything ripped from other games (Jurassic Park, ARK, Turok…).
- **Quaternius and Kenney** are low-poly cartoon style. Use them as placeholders only.
- **Some packs include their own gameplay Blueprints.** Use only their meshes, animations, materials and sounds; the game's C++ handles gameplay.
- **Fab giveaways** (free for a limited time) aren't listed, except where noted.

---

## ⭐ Start here: the 10 best picks

| # | Asset | What it gives you | License | UE 5.8.3 |
|---|---|---|---|---|
| 1 | [Game Animation Sample](https://www.fab.com/listings/880e319a-a59e-4ed2-b268-b32dac7fa016) | 500+ movement animations for the full-body player | 🔵 Epic | ☑️ Probably |
| 2 | [Animal Variety Pack](https://www.fab.com/listings/2dd7964c-a601-4264-a53d-465dcae1644c) | Deer, wolf, fox, crow, fully animated | 🔵 Fab Free | ☑️ Probably |
| 3 | [Open World Demo Collection](https://www.fab.com/listings/3262ab8f-f64a-4124-8efd-82cb19df6249) | Photoscanned trees, rocks, cliffs, grass | 🔵 Epic | ⚠️ Older |
| 4 | [Megaplants – English Oak](https://www.fab.com/listings/83642c38-7661-4df1-8629-0422e1898d26) (and others below) | Realistic Nanite trees | 🔵 Fab Free | ☑️ Probably |
| 5 | [Niagara Examples Pack](https://www.fab.com/listings/0e188eca-4e54-4fb2-a9ed-d8b8a565e600) | Fire, smoke, impacts, footsteps | 🔵 Epic | ☑️ Probably |
| 6 | [PBR Velociraptor (Animated)](https://sketchfab.com/3d-models/pbr-velociraptor-animated-8f1744af7b0847a2aabe3df90be802f0) | Realistic game-ready raptor | 🟡 CC-BY | ✅ Yes |
| 7 | [Animated T-Rex](https://sketchfab.com/3d-models/animated-tyrannosaurus-rex-dinosaur-running-loop-38007d947ae74dea83988cb0b08ee053) | Run, roar, bite, idle, tail attack | 🟡 CC-BY | ✅ Yes |
| 8 | [Poly Haven](https://polyhaven.com) | Photoreal rocks, logs, crates, textures, skies | 🟢 CC0 | ✅ Yes |
| 9 | [Sonniss GDC Audio Bundles](https://gdc.sonniss.com/) | 200+ GB of sound effects, no credit needed | Royalty-free | ✅ Yes |
| 10 | [game-icons.net](https://game-icons.net) | 4000+ inventory and skill icons | 🟡 CC-BY | ✅ Yes |

---

## 1. Characters and animation

| Asset | What it's for | License | UE 5.8.3 |
|---|---|---|---|
| [Game Animation Sample](https://www.fab.com/listings/880e319a-a59e-4ed2-b268-b32dac7fa016) | Movement for the full-body character other players see | 🔵 Epic | ☑️ Probably |
| [Free Animation Pack](https://www.fab.com/listings/8de31c5d-93bc-4bd4-9606-ca789ce91b99) | 24 motion-capture animations | ⚪ Check | ☑️ Probably |
| [Mixamo](https://www.mixamo.com) ([license FAQ](https://helpx.adobe.com/creative-cloud/faq/mixamo-faq.html)) | Thousands of humanoid animations; retarget to the UE mannequin | Free with Adobe ID | ✅ Yes (FBX) |
| [MetaHuman](https://dev.epicgames.com/documentation/metahuman/metahumans-on-fab) | Realistic survivor faces and bodies | Free under $1M/yr revenue | ✅ Built in |
| [Survival Character FREE](https://www.fab.com/listings/11d20d01-b764-4936-8163-cb20d05c369e) | Survivor character | ⚪ Check | ☑️ Probably |

**First-person arms**
| Asset | What it's for | License | UE 5.8.3 |
|---|---|---|---|
| Template arms already in `Content/FirstPerson` | The current first-person arms | Already in the project | ✅ Built in |
| [Unreal Engine FPS Arms](https://sketchfab.com/3d-models/unreal-engine-fps-arms-for-first-person-shooters-3978de7e44404707a732d2745db8a5c7) | Mannequin arms, same skeleton | Check page | ✅ Yes |
| [First Person Arms (DJMaesen)](https://sketchfab.com/3d-models/first-person-arms-e3c42c05b22944e5839deb8e003f0987) | Rigged low-poly arms | 🟡 CC-BY | ✅ Yes |
| [Control Rig Samples + Mannequins](https://www.unrealengine.com/en-US/blog/free-control-rig-and-mannequin-asset-packs-now-available-for-real-time-animators) | Tools for making your own arm animations | 🔵 Epic | ⚠️ Older |

---

## 2. Creatures

**Wildlife and fantasy**
| Asset | What it's for | License | UE 5.8.3 |
|---|---|---|---|
| [Animal Variety Pack](https://www.fab.com/listings/2dd7964c-a601-4264-a53d-465dcae1644c) | Deer stag/doe, wolf, fox, crow | 🔵 Fab Free | ☑️ Probably |
| [Quadruped Fantasy Creatures](https://www.fab.com/listings/52d686b6-1180-4f26-901f-ce3c69a14767) | Griffon, dragon, centaur, barghest; recolor them to keep the roster original | 🔵 Fab Free | ☑️ Probably |

**Dinosaurs – realistic**
| Asset | What it's for | License | UE 5.8.3 |
|---|---|---|---|
| [PBR Velociraptor](https://sketchfab.com/3d-models/pbr-velociraptor-animated-8f1744af7b0847a2aabe3df90be802f0) | Raptor, 5 skins | 🟡 CC-BY | ✅ Yes |
| [PBR Pachycephalosaurus](https://sketchfab.com/3d-models/pbr-pachycephalasaurus-animated-6eea5cee4afa4730bf75c6329a43e56d) | Dome-headed herbivore, 5 skins | 🟡 CC-BY | ✅ Yes |
| [Ferocious Industries collection](https://sketchfab.com/ferociousindustries.matthias/collections/animated-creatures-3500404251274934ad95b25bbf3cdd69) | Includes a stegosaurus. Some models are paid; only take ones with a free Download button | 🟡 CC-BY | ✅ Yes |
| [PBR Animated Dinosaurs](https://assetstore.unity.com/packages/3d/characters/animals/pbr-animated-dinosaurs-256019) | The same creator's bundle | 🟠 Unity Free | 🔧 Convert |
| [Animated T-Rex (LasquetiSpice)](https://sketchfab.com/3d-models/animated-tyrannosaurus-rex-dinosaur-running-loop-38007d947ae74dea83988cb0b08ee053) | Run, roar, bite, tail attack | 🟡 CC-BY | ✅ Yes |
| [T-Rex walk/run (Mat Rex)](https://sketchfab.com/3d-models/t-rex-with-walking-and-running-animation-41a498f69e7b43a4b4d72bede1019b5a) | A second T-Rex option | 🟡 CC-BY | ✅ Yes |
| [Animated Argentinosaurus](https://sketchfab.com/3d-models/animated-argentinosaurus-free-e5f130a58bb04eb18520bc25f9f01576) | Giant long-neck dinosaur | 🟡 CC-BY | ✅ Yes |

**Dinosaurs and animals – low-poly placeholders**
| Asset | What it's for | License | UE 5.8.3 |
|---|---|---|---|
| [Quaternius Animated Dinosaurs](https://quaternius.com/packs/animateddinosaurs.html) | T-Rex, raptor, triceratops, stegosaurus and more | 🟢 CC0 | ✅ Yes |
| [Quaternius Ultimate Animals](https://quaternius.com/packs/ultimateanimatedanimals.html) | 12 animated animals | 🟢 CC0 | ✅ Yes |

---

## 3. Starting forest, terrain and foraging

| Asset | What it's for | License | UE 5.8.3 |
|---|---|---|---|
| [Open World Demo Collection](https://www.fab.com/listings/3262ab8f-f64a-4124-8efd-82cb19df6249) | 88 trees, bushes, stumps, rocks, cliffs | 🔵 Epic | ⚠️ Older |
| [A Boy and His Kite](https://www.fab.com/listings/4d3e971c-7651-4965-b932-6f49efc5fc1a) | The full demo landscape (very old UE4 project) | 🔵 Epic | ⚠️ Older; may not open, so use the collection above instead |
| [Spruce Forest (Project Nature)](https://www.fab.com/listings/f8044501-17a2-498f-b198-5f1bc71ee87a) | 15 spruce trees | 🔵 Fab Free | ☑️ Probably |
| [Electric Dreams](https://www.fab.com/listings/d79688f5-29be-4fb2-a650-2d4a813f5306) | Dense forest, cliffs, procedural (PCG) setup | 🔵 Epic | ☑️ Probably |
| [Project Titan](https://www.fab.com/listings/c05aac82-4c1a-4e42-96b3-be668dc40fca) | Huge open world with 10,000+ meshes to borrow | 🔵 Epic | ☑️ Probably |
| [Modular Rural House & Pine Forest](https://www.fab.com/listings/a081748c-6a49-4ba4-9008-9b10fadf8f73) | Pine forest and a house | ⚪ Check | ☑️ Probably |
| Megascans free selection ([FAQ](https://support.fab.com/s/article/Fab-Transition-FAQs?language=en_US)) | About 1500 free rocks, ground surfaces, logs, plants. On Fab, search Megascans with "Price: Free" | 🔵 Fab Free | ☑️ Probably |
| [Free Stone Material Pack](https://www.fab.com/listings/c19914ba-fc5f-43b5-a52a-b7dd8d9466ac) | 6 stone materials, 4K | 🔵 Fab Free | ☑️ Probably |

**Megaplants (realistic Nanite trees)**: all 🔵 Fab Free, ☑️ Probably. They are built for the Procedural Vegetation Editor that ships with recent Unreal 5 versions. ([announcement](https://quixel.com/news/discover-the-latest-quixel-megascans-and-free-megaplants))
[English Oak](https://www.fab.com/listings/83642c38-7661-4df1-8629-0422e1898d26) ·
[Baltic Pine](https://www.fab.com/listings/a2b04e81-5075-479f-a9d2-4940022f330a) ·
[Pine Saplings](https://www.fab.com/listings/ae732b47-14db-422e-b11d-c93537cb5165) ·
[European Beech](https://www.fab.com/listings/cefe5722-9c31-4aa2-9ee2-e5426610d5e6) ·
[European Aspen](https://www.fab.com/listings/ffa90e1a-e420-43d6-ade3-daa4bc189a0a) ·
[Huckleberry Oak](https://www.fab.com/listings/ccd9583a-c459-458c-af11-b48138c2bdcc) ·
[Aleppo Pine](https://www.fab.com/listings/a441387b-c3e0-4982-82c4-5f661ccca6dd)

**Poly Haven models**: all 🟢 CC0, ✅ Yes.
[Rocks](https://polyhaven.com/models/rocks) ·
[Trees](https://polyhaven.com/models/nature/trees) ·
[Tree Stump 01](https://polyhaven.com/a/tree_stump_01) ·
[Tree Stump 02](https://polyhaven.com/a/tree_stump_02) ·
[Dead Tree Trunk](https://polyhaven.com/a/dead_tree_trunk_02) ·
[Collections](https://polyhaven.com/collections)

**Textures and skies**: all 🟢 CC0, ✅ Yes.
[Poly Haven](https://polyhaven.com) ([license](https://polyhaven.com/license)) · [ambientCG](https://ambientcg.com)

---

## 4. Ancient ruins (distant landmarks)

| Asset | What it's for | License | UE 5.8.3 |
|---|---|---|---|
| [Sun Temple](https://www.fab.com/listings/b5516e01-8511-4ff4-b658-a6efd6bc7c6f) | Ornate stone temple | 🔵 Epic | ⚠️ Older |
| [Valley of the Ancient](https://www.fab.com/listings/0c19880e-21bd-42ba-8287-1caccc3951b1) | Canyon rock and a "dark world". Use the environment only; its characters belong to Epic | 🔵 Epic | ⚠️ Older (early UE5) |
| [Fable X – Ancient Ruins](https://forums.unrealengine.com/t/fable-x-ancient-ruins-free-asset-pack-26-pieces-custom-rematerials/2431144) | 26 modular ruin pieces; swap the red stone for grey | ⚪ Check | ☑️ Probably |
| [Ancient Temple Ruins](https://www.fab.com/listings/460311fc-fd70-4660-ac6f-1d5aac245f5c) | Only free if you claimed it before 13 Jan 2026 | ⚪ Check | ☑️ Probably |

---

## 5. Frontier biomes

| Biome | Asset | License | UE 5.8.3 |
|---|---|---|---|
| Volcanic | [Free Lava Material](https://www.fab.com/listings/dac5f176-9ee6-4f6f-84ab-fae2bdce0abc) | 🔵 Fab Free | ☑️ Probably |
| Volcanic | Basalt, ash and lava textures from Poly Haven and ambientCG | 🟢 CC0 | ✅ Yes |
| Frozen | [Fresh Windswept Snow](https://www.fab.com/listings/afd08895-296c-47bf-ad51-565672b4a515) | 🔵 Fab Free | ☑️ Probably |
| Frozen | [ICE VOL1: A Frozen Place](https://www.fab.com/listings/21f20110-da6f-48a8-a0d1-572f313c9bf0) | ⚪ Check | ☑️ Probably |
| Overgrown | Electric Dreams and Project Titan (section 3) | 🔵 Epic | ☑️ Probably |
| Corrupted | Valley of the Ancient dark world (section 4) | 🔵 Epic | ⚠️ Older |
| Crystal | No free option; see Gaps | – | – |

---

## 6. Building and camp props

| Asset | What it's for | License | UE 5.8.3 |
|---|---|---|---|
| [Modular Rural Cabins](https://www.fab.com/listings/508fe84a-4976-4cfe-9a40-c2b9533da601) | 156 wooden building pieces and props. May have been a time-limited promo; check the page | ⚪ Check | ☑️ Probably |
| [UNIBLOCKS FREE](https://www.fab.com/listings/e36c2bc1-49d4-4918-a0d7-f09a90ec7a57) | Block-by-block building kit for greyboxing | ⚪ Check | ☑️ Probably |
| Poly Haven props: [Crate 01](https://polyhaven.com/a/wooden_crate_01), [Crate 02](https://polyhaven.com/a/wooden_crate_02), [Treasure Chest](https://polyhaven.com/a/treasure_chest), [Barrels](https://polyhaven.com/a/wooden_barrels_01), [Bucket](https://polyhaven.com/a/wooden_bucket_01) | Storage boxes and camp clutter | 🟢 CC0 | ✅ Yes |
| [Kenney Survival Kit](https://kenney.nl/assets/survival-kit) | Campfire, workbench, tent, tools (placeholder) | 🟢 CC0 | ✅ Yes |

---

## 7. Tools and weapons

| Asset | What it's for | License | UE 5.8.3 |
|---|---|---|---|
| [Primitive Stone Pickaxe](https://sketchfab.com/3d-models/simple-primitive-stone-pickaxe-free-d66c840a98b149c18168b3e18b5cdcce) | Stone pickaxe | 🟡 CC-BY | ✅ Yes |
| [Stone Spear (Welham14)](https://sketchfab.com/3d-models/stone-spear-f53e2c0fe24846e886e7c8779e0efea5) | Spear | Check page | ✅ Yes |
| [Spear – Stone Age (Faust_D_Readfull)](https://sketchfab.com/3d-models/spear-stone-ageprimal-ee07f678f5b349f88fad2964d594d26a) | Spear | Check page | ✅ Yes |
| [Free Fantasy Weapon Sample Pack](https://www.fab.com/listings/d5be0dc9-1a41-4be2-a63a-5ed436f3445d) | Metal swords and axes for later tiers | ⚪ Check | ☑️ Probably |
| [Kenney Survival Kit](https://kenney.nl/assets/survival-kit) | Placeholder tools | 🟢 CC0 | ✅ Yes |

---

## 8. Visual effects

| Asset | What it's for | License | UE 5.8.3 |
|---|---|---|---|
| [Niagara Examples Pack](https://www.fab.com/listings/0e188eca-4e54-4fb2-a9ed-d8b8a565e600) | 50+ effects: fire, smoke, sparks, hits | 🔵 Epic | ☑️ Probably |
| [Realistic Fire & Explosion Starter](https://www.fab.com/listings/2adf4e4e-5f1d-4543-9e8c-192fe3c9a8ed) | Campfires and torches | 🔵 Fab Free | ☑️ Probably |

## 9. Sky, weather and time of day

Use Unreal's built-in Sky Atmosphere, Volumetric Clouds, Height Fog, Sky Light and Directional Light (Window → Env. Light Mixer). That's ✅ built into 5.8.3 with nothing to download. Rain and snow can come from the Niagara Examples Pack.

---

## 10. Audio

All audio is plain WAV/OGG/MP3, so ✅ every item works in 5.8.3.

| Asset | What it's for | License |
|---|---|---|
| [Sonniss GDC Bundles](https://gdc.sonniss.com/) ([older years](https://sonniss.com/gameaudiogdc/)) | Creatures, nature, foley, impacts | Royalty-free, no credit |
| [Freesound](https://freesound.org) | Individual sounds; filter by "Creative Commons 0" | 🟢 CC0 / 🟡 CC-BY per sound |
| [Monster Sounds pack](https://freesound.org/people/cylon8472/packs/15382/) | Big creature roars | Check each sound |
| [Sea Creature Roar](https://freesound.org/people/Bikkit99/sounds/837799/) | Roar | 🟢 CC0 |
| [UI SFX Free Pack](https://www.fab.com/listings/a6ca37d8-2df5-42ac-905f-377e387b74ef) | Menu sounds | ⚪ Check |
| [Kenney Interface Sounds](https://kenney.nl/assets/interface-sounds) · [Kenney UI Audio](https://kenney.nl/assets/ui-audio) | Menu sounds | 🟢 CC0 |
| [Free Music Pack (Alexander Ehlers)](https://opengameart.org/content/free-music-pack) | 7 music tracks | 🟢 CC0 |
| [Calm / Relaxing Music](https://opengameart.org/content/cc0-calm-relaxing-music) | Background music | 🟢 CC0 |
| [Background Ambience](https://opengameart.org/content/cc0-background-ambience) | Ambient loops | 🟢 CC0 |
| [Natural Forest Fantasy Music](https://opengameart.org/content/natural-forest-fantasy-music) | Forest music | Check page |

---

## 11. UI and icons

| Asset | What it's for | License | UE 5.8.3 |
|---|---|---|---|
| [game-icons.net](https://game-icons.net) ([license](https://game-icons.net/about.html)) | 4000+ icons. Export them as PNG for Unreal | 🟡 CC-BY | ✅ Yes |
| [60 Free Icons](https://www.fab.com/listings/fc9f4a87-3168-4c9b-a71c-89b4bf31692e) | Fantasy inventory icons | ⚪ Check | ✅ Yes |
| [itch.io survival icon packs](https://itch.io/game-assets/free/tag-icons/tag-survival) | More icon packs | Varies per pack | ✅ Yes |

---

## 🛠️ Gaps: no good free asset (candidates for Higgsfield)

For Unreal, ask for **FBX** (meshes and animations) or **PNG** (2D art). Model at real-world scale in centimetres.

| # | What's needed | Format |
|---|---|---|
| 1 | Primitive building pieces (stick/thatch/hide): foundation, wall, doorway, door, roof | Static mesh, fits a 400 cm grid |
| 2 | Stone and metal building pieces, same set | Static mesh, 400 cm grid |
| 3 | Realistic stone axe, club, bow and arrows, torch, knife | Static mesh |
| 4 | First-person tool animations: chop, mine, thrust, bow, torch, eat/drink | Animations on the UE5 mannequin skeleton |
| 5 | Campfire with spit, crafting bench, drying rack, kiln, storage bin, bed | Static mesh |
| 6 | Hide, fur and woven prehistoric clothing | Skeletal mesh for the UE5 mannequin or MetaHuman |
| 7 | Realistic triceratops, ankylosaurus, parasaurolophus, brachiosaurus, with eat/sleep/death animations | Rigged and animated skeletal mesh |
| 8 | Original Frontier creatures | Rigged and animated skeletal mesh |
| 9 | Crystal clusters and glowing crystal rocks | Static mesh + glowing material |
| 10 | Ice formations, frozen waterfalls, snowy rocks and dead trees | Static mesh |
| 11 | Lava rock, obsidian, vents, scorched trees | Static mesh |
| 12 | Portals, research stations, ancient machines | Static mesh + glowing material |
| 13 | Painted item icons in one consistent style | 512 px PNG, transparent background |
| 14 | Tech-tree and HUD art: frames, backgrounds, status icons | PNG |
| 15 | Voice sets per creature: idle, alert, attack, hurt, death | WAV |
| 16 | Original music: main theme, exploration, combat, Frontier ambience | WAV |

---

## 📝 Credits (required for CC-BY assets)

For every 🟡 CC-BY asset you ship, write down its title, author, link and license. For example:

> "PBR Velociraptor (Animated)" by Ferocious Industries, Sketchfab, CC BY 4.0.

This covers the Sketchfab models, game-icons.net icons and any CC-BY Freesound clips. CC0, Epic and Fab Free assets need no credit.
