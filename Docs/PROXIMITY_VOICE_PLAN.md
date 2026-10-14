# Proximity voice chat — future M15 multiplayer objective

Requested and confirmed by the user October 13, 2026: nearby multiplayer players should hear each other based on distance. Playable audio is **planned, not implemented or tested**. October15 metadata-only routing policy foundation passes two native tests and both targets; see PROXIMITY_VOICE_POLICY_M15.md. Actual server snapshot authority is the next separate gate. No microphone, transport, service, device UI or listening pass exists. It is proximity voice, not a global text chat request.

## Bounded future scope

October15 supported transport audit now in VOICE_TRANSPORT_AUDIT_M15.md: current direct-IP/Null development sessions, no configured online voice provider. Recommend bounded LAN capture/relay first; next implementation needs user scope choice versus intended online platform/product. Installed EOS positional update is unimplemented, so no automatic proximity/provider selection assumed. No plugin/module/service/capture added and no Personal listening test exists yet.

October15 backend checkpoint: server snapshot bridge now technically passes native3/3, actual one2/2 and impairedtwo3/3 metadata gates; PROXIMITY_VOICE_AUTHORITY_M15.md records scope. No provider/plugin/module or audio transmission selected/added. The initial paragraph's upcoming snapshot note is historical; actual transport audit is next before capture/UI or physical listening.

- Optional push-to-talk, with an accessible input binding, microphone/output-device selection and clear speaking/receiving indicators. Default to no recording or transmission until the player deliberately enables voice.
- Distance-based audibility/attenuation using authoritative player positions and the actual connected session. Account for dedicated servers; never trust a client-supplied position or arbitrary recipient identity for routing.
- Local per-player mute, voice volume, block controls and clear device/network failure feedback. Preserve text/readable gameplay cues so voice is not necessary to play.
- Keep transport bounded in bandwidth, connected participants and processing cost; no voice history in world saves, logs or test reports.
- Choose supported Unreal/online-service voice APIs only after auditing the actual multiplayer/session backend, platform/account requirements and packaging. No new plugin, service, paid dependency or invented backend is selected by this planning note. Refresh official documentation at implementation time.

## Verification before completion

Level: current existing baseline `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`; use `/Game/PrimalFrontier/Maps/L_M7SurvivalArena` for a smaller isolated fixture when sufficient. The future voice-specific fixture/runner is **TBD**, not available yet.

Start here for AI: inspect the actual connection/session/backend and current M15 authority evidence, then propose one supported transport and its dependencies before adding them. Build and test opt-in/off, identity/range authorization and join/leave/reconnect without audio capture. Follow with one server/client, then two clients using low-cost maps and NullRHI where rendering is unnecessary.

A later physical listening gate requires two participants/devices: push-to-talk near each other, move beyond range and back, change volume/mute, disconnect/reconnect and disable voice. Expected: intelligible speech only when enabled/in range, no accidental transmission, muted players remain inaudible, safe device failure and normal gameplay. That is not a current Personal assignment; no microphone or voice feature is present yet. Document exact controls, device/session prerequisites, near/far thresholds, evidence paths and privacy limits on its Trello task before asking anyone to test.

Voice does not replace authoritative gameplay validation and does not complete full M15, Internet hosting, packaged-server readiness or security/moderation acceptance. Do not label headless/network assertions a microphone listening pass.
