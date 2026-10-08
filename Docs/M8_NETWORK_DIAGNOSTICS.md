# M8 network diagnostics — October 8, 2026

Existing PF.Persistence.Live scenario, current compiled Editor code including the test-only DeadPlayerRespawn addition. No new C++ behavior, project/engine settings, assets, map or authentication service changed. This diagnostic extends the no-emulation baseline in M8_POST_FIX_VERIFICATION.md; it is not a rendered/manual milestone gate or a new M15 hardening system.

## One-client modest lag/loss

Uncooked UnrealEditor-Cmd server plus one separate client, NullRHI/no sound/unattended/NoLiveCoding/NoSaveConfig. Same open-world map, PF.Persistence.Live automation command and queue-empty test exit as the baseline. Dedicated localhost port 17985; unique AutomationM8Lag slot and private per-endpoint/profile identity kept across a new server process restart. The only network changes are session command-line **-PktLag=75 -PktLoss=1** on both processes, parsed by installed UNetDriver/FPacketSimulationSettings. The log in every process confirms **PktLag set to 75** and **PktLoss set to 1**.

This configures 75 ms outgoing packet delay and 1% outgoing simulated loss on each side. It is not a measured RTT/loss rate, a deterministic packet-drop trace, or certification of arbitrary jitter/loss. The same development fixture remains mostly server-driven actions plus real replication/client authority refusals/disconnect/reconnect; it does not cover client request races or flood limits.

All four reports passed PF.Persistence.Live, 1/1 each, zero errors/test warnings, engine/report exits 0. No timeout, ensure, fatal or error outside the known installed-engine Python startup traces. Reports are Saved/AutomationReports/<run>/index.json, logs Saved/Logs/PF<run>.log:

| Run | Working GiB | Private GiB | Phase elapsed s |
| --- | --- | --- | --- |
| M8Lag1_20261008_061744_29d940CreateServer | 1.719 | 1.619 | 51.75 |
| M8Lag1_20261008_061744_29d940CreateClient1 | 1.805 | 1.780 | 52.21 |
| M8Lag1_20261008_061744_29d940RestartServer | 1.711 | 1.609 | 38.51 |
| M8Lag1_20261008_061744_29d940RestartClient1 | 1.792 | 1.730 | 38.91 |

Elapsed time is the phase plus sequential report inspection, not a per-process frame-time measurement. Peak working/private values are sampled; do not add independent peaks and call that a simultaneous system peak.

Test exercised real gathering, timed tool crafting, foundation/storage, item conservation, server save/load, client save/load/grant refusal, replicated tool/health/world time/public identity, departure capture after manual save and separate-process reconnect restoration. Known raw logs on each process: 24 warning lines (21 editor-widget factories, three uncooked HLOD imports/settings) and 14 installed StateTreeToolset/ToolsetRegistry Python traceback lines. No warnings outside those categories and no errors outside Python. Raw logs are not clean; test events are clean.

No repeated build was needed for diagnostic-only session arguments. Earlier Editor test build passed 6.20 s; runtime Editor/Game collision builds and source checkpoint are documented in PERSISTENCE_M8.md. Installed engine is 5.8.3; packaged Server remains blocked by its distribution. User saves/profiles and all failed evidence are preserved; the bounded helper lives outside the repo. NullRHI has no FPS/stutter/rendered-physics/manual traversal proof, and this 31.93 GiB host does not certify a 16 GB minimum.

One-client bounded Trello diagnostic is complete. A separate two-client task may follow; neither closes M7 route/overnight, M8 rendered persistence, physical-controller or asset-provenance gates.
