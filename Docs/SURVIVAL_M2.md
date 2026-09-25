# Milestone 2: hunger, thirst and exposure

`UPFPlayerSurvivalComponent` replicates food reserve (`Hunger`), water reserve (`Thirst`) and normalized `Exposure` alongside Health/Stamina. Food/water range from 0 to 100; a high value is healthy. Their Gameplay Tags are `Attribute.Survival.Hunger`, `.Thirst` and `.Exposure`.

The server drains 0.2 food and 0.3 water per simulation second. Empty food causes 2 damage/second; empty water causes 3. Damage uses the exact portion of the interval spent empty, so crossing zero in a long frame does not charge damage for the entire frame. Need simulation stops at death, and a replacement pawn starts with full reserves. Stamina recovery requires nonempty reserves and zero exposure. Optional fed health recovery defaults to zero to preserve M1 behavior; when enabled, both reserves must exceed 25 and exposure must be zero. Designer rates are finite, nonnegative and capped at 1000; attribute mutation rejects invalid inputs and client authority.

`APFSurvivalHazard` is a primitive box overlap region. The server samples overlapping regions; the strongest intensity wins, including when one region is destroyed. Exposure causes up to 5 damage/second. Leaving all regions restores neutral exposure, unless a developer override is active. This is a danger/exposure interface, not a temperature, weather or climate simulation.

`APFRecoveryPickup` is a single-use placeholder ration that restores 35 food/water. E on the first-person controller sends a rate-limited interaction request. The server performs its own 250 cm view trace and checks possession, life state, distance, line of sight, authority and prior consumption. It accepts no client-supplied attribute values or recovery quantities. Consumption destroys the replicated actor. This is not an inventory/item-registry implementation; those belong to M3.

Rations must be found in the world; players have no innate food supply. Each ration expires after 300 simulation seconds by default, configurable on its actor. The server assigns an expiration deadline at spawn and replicates it; the world label shows remaining freshness using synchronized server time. Consumption checks the deadline even before cleanup runs. Expired food is removed by the server and cannot restore attributes. The timer pauses/slows with game simulation, does not reset per client, and has no persistence yet. Future inventory/cooking must preserve this deadline instead of refreshing food on transfer. Making/cooking food belongs to later milestones, not M2.

The five-minute deadline is fast test tuning. [Food and preservation research](FOOD_AND_PRESERVATION.md) documents the Palworld/ARK references, original proposed lifetimes, cooking/drying and preservation countermeasures, and the later inventory/save freshness contract. Those later systems are not implemented by M2.

The placeholder HUD displays Health/Stamina, food/water bars and an exposure percentage/warning. `PresentNeeds` exposes the values to Blueprint presentation alongside the existing `PresentVitals` event. All visual assets are engine primitives and existing template presentation.

## Development commands

Run on the standalone/server console with one possessed survivor:

- `PF.SetHunger 20`, `PF.SetThirst 30`: clamp reserves to 0..100.
- `PF.SetExposure 1`: enable test exposure; `PF.SetExposure 0` restores volume-only exposure.
- `PF.RecoverNeeds`: restore 35 food/water through the same component API.
- `PF.ExportTestReport M2Manual`: export command history. Client mutation attempts intentionally record failures with `NotAuthority`.

These commands remain non-Shipping and are listed by `PF.Help`. The gameplay E interaction remains a validated gameplay action in all builds.

## Verification procedure

`Scripts/SetupSurvivalMilestone2.py` creates `/Game/PrimalFrontier/Maps/L_M2Survival` from the existing small M1 map using Unreal Editor APIs and refuses to overwrite an existing destination. It adds one hazard and three rations; no M1/template asset is rewritten.

Run `PF.Survival.Needs` for defaults, drain, threshold timing, recovery, death and invalid/authority guards. `PF.Survival.Environment` checks actual overlaps, region exit/removal, ration range/obstruction/authority and duplicate refusal. Keep `PF.Survival.Component`, `.Lifecycle` and the three runtime tooling regression tests in the gate.

`PF.Survival.NeedsLive` requires an isolated M2 game/server session with `-PFRunNeedsLiveTests -PFExpectedPlayers=1` (or 2). It observes every player's initial reserves, the stable authoritative 40/50/0.25 sample, 75/85 recovery, empty reserves with Health 90, death and respawn reset. Each client checks both local and remote vitals and rejects direct and console mutation probes. Tests time out as failures. Temporary tuning occurs only in the disposable live test session. Use unique process log/report paths, `-TestExit="Automation Test Queue Empty"`, and NullRHI for nonvisual clients/server.

Actual pass/fail, manual observations, limits and evidence are recorded in `MILESTONES.md`; this procedure alone is not an acceptance result.
