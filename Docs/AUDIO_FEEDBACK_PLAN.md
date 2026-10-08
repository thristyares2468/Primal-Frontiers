# Bounded audio and feedback — M9/M17 plan

October 8, 2026. **Planning only: no audio emitters or assets added, no audibility pass.** This completes the independent Sound & Audio planning task while M7/M8 manual gates wait. M17 implementation remains gated. [AUDIO_COVERAGE.md](AUDIO_COVERAGE.md) records the actual present controls and missing authored coverage; [ASSET_PIPELINE_M9.md](ASSET_PIPELINE_M9.md) governs future intake.

Existing local Master/Music/Effects/UI preferences and unfocused mute are implemented; only a template weapon SoundWave is present in the read-only /Game inventory. Do not use that weapon sound for food, UI or creature feedback just to populate a checklist. Existing engine/template/Blueprint sound is not ruled out, and its category routing has not been validated by playback. No new sound is required to complete this planning task.

## Minimal coverage, in priority order

| Proposed event | Authoritative trigger / local origin | Audience and class | Visual fallback |
| --- | --- | --- | --- |
| UI accept/refuse | Local menu navigation; server result for gameplay requests | Owner only, SC_UI | Current text result/reason; never a success cue before acceptance |
| Pickup / gathering | Successful inventory transfer or resource transaction | Nearby world listeners, SC_Effects | Item/count/result and depleted state |
| Craft complete / fail / cancel | Final conversion outcome, not request accepted | Owner only, SC_UI | Queue state and explicit reason |
| Build / demolish / door | Successful server placement/removal/door transition | Nearby world listeners, SC_Effects | Piece/door state and refusal reason |
| Player damage / death | Server damage/life transition | Local response for owner; nearby impact for observers, SC_Effects | Health change, damage indicator and death state |
| Creature attack / hit / death | Accepted server combat/state transition | Nearby world listeners, SC_Effects | Windup, hit and corpse/loot state |
| Footstep / landing | Grounded movement/landing presentation | Local and nearby remote listeners, SC_Effects | Movement remains legible without sound |
| Needs warning | Owner enters a low-needs band | Owner only, SC_UI | Hunger/thirst/stamina indicator and text |
| Zone ambience | Local zone/listener state | Listener-local SC_Effects loop | Landmark/zone lighting; no gameplay authority |

Start later with one UI result pair, one pickup/gather impact and one craft result. Reuse a verified suitable cue across related actions where readable, rather than inventing a large library. Add combat/footsteps only after this small route works. At most one crossfaded ambience pair; music is optional and deferred until a suitable source is approved. Do not add dialogue, creature vocal packs, reverb simulation or continuous crafting loops to prove feedback.

## Event routing and duplicate prevention

Core C++ keeps gameplay authority; a replaceable local presenter resolves a data-defined event ID/Gameplay Tag to soft sound/attenuation/concurrency references. Blueprint composes/tunes the presentation. A missing cue is a silent presentation gap with a one-time development warning, never a failed inventory/crafting transaction. No gameplay module dependency on PrimalAgentTools and no new plugin is proposed.

Existing ClientInventoryFeedback and building ClientResult carry human-readable text, while crafting/vitals/creature state replicate. Do not parse translated strings or every inventory/vitals delta to synthesize success events. Later add a bounded typed result/event at the validated outcome, with a per-source sequence and short lifetime. Do not emit for fixture construction, save restoration, first replication snapshot, join-in-progress or every clock/vitals tick. Repeated restoration must not replay pickup/build/death effects.

Owner UI is local; accepted transaction feedback comes from the server. World one-shots are cosmetic, relevancy/range-limited and may be dropped without losing gameplay state. Do not use reliable per-frame/footstep multicasts. A listen-server owner follows one presenter path so authority and local callbacks cannot double-play. For the initial slice, do not predict the same world success sound; local button/navigation sounds remain distinct. If prediction is introduced later, reconcile using bounded event IDs rather than playing both request and acknowledgement.

Stop actor-attached loops when actors end play/unload, die or are replaced by load. Do not replicate sounds or persistent audio components in save records. Ambient selection is listener-local; server time may inform the phase, but no server audio device/asset load/tick is needed. Dedicated-server guards occur before loading presentation references. One-shots derived from remote replicated motion are local presentation with movement/ground/cooldown checks; they cannot authorize damage, gathering or sneak detection.

## Proposed spam, range and memory budgets

These are initial configuration targets to measure, not current assets/settings or performance results.

- Use a shared global Sound Concurrency group of 16 active gameplay sounds, with smaller groups: UI 2, impacts 6, footsteps 4 and ambience 2. Critical death/damage has higher priority than ambience; reject/stop lower-priority voices at the cap. Counts cover active components as well as audible voices; no silent loop may occupy a slot forever.
- UI retrigger interval 0.1 s; refusal warning 1 s; damage response 0.25 s; creature idle cue at least 4 s. Need warnings trigger on entering a band, with 10 s cooldown and hysteresis before rearming; restoring low vitals initially shows text without a burst of warnings. Do not apply audio cooldowns to gameplay action validity.
- Grounded footsteps no faster than every 0.25 s per actor, none while stationary/airborne; remote footsteps stop outside a proposed 12 m audible distance. Small interaction impacts start at 10 m and creature/combat cues at 20 m, with attenuation and spatialization configured and tested. Range limits must fit within actual network relevancy; attenuation alone does not cap network traffic.
- Avoid per-frame occlusion queries or sophisticated reverb in the first slice. Use short mono spatial effects; reserve stereo for listener-local UI/ambience where useful. Proposed first slice: at most 12 short source cues and 8 MiB measured additional resident audio memory. A decoded mono 48 kHz/16-bit second is approximately 96,000 bytes; import/compression/runtime buffers add costs, so file size is not resident memory.
- Soft-reference/load only the small active cue set at session/zone setup, rather than synchronously discovering/loading a whole pack on every action. Stop/release unused loops on travel and menu/session teardown. No change to project-wide engine voice limits or rendering quality is authorized by this plan.

Epic's [Sound Concurrency reference](https://dev.epicgames.com/documentation/unreal-engine/sound-concurrency-reference-guide) documents group limits, per-owner limits, retrigger time, resolution rules and active-component counting; multiple concurrency rules can constrain one source. Its [Sound Attenuation reference](https://dev.epicgames.com/documentation/unreal-engine/sound-attenuation-in-unreal-engine) documents distance falloff and spatialization. Checked October 8 against the current 5.8 documentation. The numerical budgets and networking/presentation policy above are original proposals, not Epic recommendations or a measured optimization result.

## Assets, settings and accessibility

Use a verified existing project-owned or permitted engine cue first. If there is no suitable source when implementation is eligible, request only the small missing set: UI result pair, neutral item/gather impact and craft completion cue. Ask for exact provenance/rights and source/version mapping under ASSET_PIPELINE_M9.md; do not assume imported packs include audio or that an advertised free pack permits standalone source redistribution. No audio download, generation, import or purchase is performed in this task.

Future project-owned sound wrappers route to SC_UI/SC_Effects; music routes to SC_Music. Every cue must respect Master/category zero, saved preferences and unfocused mute. Settings are listener-local and cannot modify another player's volume. Do not silently replace imported SoundClasses or normalize an entire pack. Validate one approved cue at a time through supported Unreal tools; avoid looping loud test signals or changing Windows audio settings.

Keep interaction/refusal/needs/death information visible without audio. Critical gameplay cannot rely on stereo direction or a cue alone. Later controls should offer a visible sound-event indicator and a separate repetitive-warning preference if needed; do not claim these settings already exist. Menus in multiplayer do not pause the world, so urgent visual feedback remains visible. Physical audibility and comfort require a human listener; NullRHI cannot supply this evidence.

## Future implementation and acceptance

1. After the implementation/source gates resolve, add the three small event routes and presentation data only; compile Editor and run narrow result/deduplication tests. No world rebuild, extra art or imported demo level.
2. Native tests: emit only after accepted outcomes; failed/cancelled craft has no success event; owner/UI isolation; missing cue is harmless; rate limits/sequence dedupe remain bounded; initial replication/load/JIP are silent; no sound-reference load on dedicated server; loop cleanup. Keep PF.Settings.Preferences regression but do not treat it as playback proof.
3. One rendered player: hear one cue per pickup/gather/craft, clear refuse reasons, individual category/mute control and no double cue after save/load/death/restart. Verify sound-class routes with an actual appropriate cue, not just a slider. Inspect logs for asset/audio errors and record resolution, uncapped frame times, working/private memory and audio-resident increase.
4. One server/client first, then two NullRHI clients for event counts/authority/privacy/cleanup; this checks routing, not sound quality. Use at most one rendered listener per low-memory run to listen to local/remote distance attenuation. A human audibility check still remains separate from event-count automation.
5. Saturate interaction/creature events in a bounded small test area, confirm concurrency priorities and no repeated low-needs spam. Check muted/unfocused, menu transitions, unload/reconnect and save restoration. Record a genuine sound gap or hardware/RHI block, never a fabricated audio pass.

Planning validation: reviewed actual crafting completion, interaction/building results, survival snapshots/delegates, creature states and preference class routes; checked this plan against M9 intake and current missing audio coverage. No assets/source/settings changed or new Unreal session launched. No new build, gameplay automation, audibility test, FPS or memory measurement is claimed; M8 results and the M9 inventory remain the latest respective evidence.
