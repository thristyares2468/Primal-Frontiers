# M8 network diagnostics — October 8, 2026

Existing PF.Persistence.Live scenario. The initial lag/loss runs below used the compiled Editor code including the test-only DeadPlayerRespawn addition; the final section extends only the live test to exercise an actual invalid client RPC. No gameplay behavior, project/engine settings, assets, map or authentication service changed. These diagnostics extend the no-emulation baseline in M8_POST_FIX_VERIFICATION.md; they are not rendered/manual milestone gates or a new M15 hardening system.

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

One-client bounded Trello diagnostic is complete, committed/pushed 8b62612. The separate two-client scope below also passed; neither closes M7 route/overnight, M8 rendered persistence, physical-controller or asset-provenance gates.

## Two-client modest lag/loss

After the one-client prerequisite, repeated the same profile/map/test/exit/port with a fresh isolated save and two independent private profiles. Every one of the six engine logs confirms PktLag set to 75 and PktLoss set to 1. Each PF.Persistence.Live report passed 1/1, engine/report exits 0, zero test errors/warnings/ensure/fatal/timeouts. Client 2 passed create and restart without crashing.

| Run | Working GiB | Private GiB | Phase elapsed s |
| --- | --- | --- | --- |
| M8Lag2_20261008_062152_3aa7a6CreateServer | 1.723 | 1.627 | 51.17 |
| M8Lag2_20261008_062152_3aa7a6CreateClient1 | 1.814 | 1.771 | 51.62 |
| M8Lag2_20261008_062152_3aa7a6CreateClient2 | 1.801 | 1.768 | 52.05 |
| M8Lag2_20261008_062152_3aa7a6RestartServer | 1.716 | 1.614 | 39.88 |
| M8Lag2_20261008_062152_3aa7a6RestartClient1 | 1.791 | 1.709 | 40.30 |
| M8Lag2_20261008_062152_3aa7a6RestartClient2 | 1.794 | 1.722 | 40.73 |

All six raw logs retain the same 24 editor-widget/HLOD warnings and 14 installed-engine Toolsets Python error lines, with no other warning/error categories. All owned processes finished and no user Editor session was closed. Bounded two-client Trello task complete; no additional source/assets/settings or build change. Profile parameters are session-only and do not establish measured RTT/loss, a soak, reliable-queue limits, client-request races or rendered stability. Broader tests are described in NETWORK_PROFILING_READINESS.md.

## Actual invalid client inventory RPC

Extended only the opt-in `PFPersistenceLiveTests.cpp` fixture: each owning client calls the generated `ServerInventoryAction` RPC once with an existing tool stack, Split action and quantity -1. This does not invoke `_Implementation` locally. An empty initial feedback value prevents a stale refusal from falsely acknowledging the probe. The client requires the server's reliable refusal within 10 seconds, waits another second for property delivery, then checks exact batch IDs, item IDs, quantities and freshness deadlines, slot count, tool, health and structure conservation. The existing post-save Fibre marker must also arrive before the snapshot. No production authority/cooldown code changed.

Editor build passed 22.36 s and Development Game build passed 28.26 s, zero compiler warnings: Saved/Logs/PFM8InvalidRPCEditorBuild.log and PFM8InvalidRPCGameBuild.log. Test remains non-Shipping and opt-in.

One-client Create/Restart passed first, then two-client Create/Restart. All ten process reports passed **PF.Persistence.Live 1/1**, engine/report exits 0, zero test warnings/errors/timeout/ensure/fatal. Every server log records exactly one `Request action=0 quantity=-1 accepted=0` per connected client; every client report includes the fresh refusal acknowledgement and passes conservation checks. Client 2 did not crash.

Fresh isolated AutomationM8RPC saves/profiles, localhost port 17986, same open-world map/automation command/queue-empty exit, NullRHI and session-only PktLag=75/PktLoss=1. Each engine log confirms both network settings. Reports: Saved/AutomationReports/<run>/index.json; logs: Saved/Logs/PF<run>.log.

| Run | Working GiB | Private GiB | Phase elapsed s |
| --- | --- | --- | --- |
| M8RPC1_20261008_063219_5cade8CreateServer | 1.712 | 1.604 | 51.32 |
| M8RPC1_20261008_063219_5cade8CreateClient1 | 1.812 | 1.774 | 51.78 |
| M8RPC1_20261008_063219_5cade8RestartServer | 1.715 | 1.618 | 39.78 |
| M8RPC1_20261008_063219_5cade8RestartClient1 | 1.795 | 1.716 | 40.18 |
| M8RPC2_20261008_063412_47b0f8CreateServer | 1.715 | 1.613 | 51.08 |
| M8RPC2_20261008_063412_47b0f8CreateClient1 | 1.812 | 1.764 | 51.5 |
| M8RPC2_20261008_063412_47b0f8CreateClient2 | 1.812 | 1.783 | 51.93 |
| M8RPC2_20261008_063412_47b0f8RestartServer | 1.714 | 1.617 | 41.12 |
| M8RPC2_20261008_063412_47b0f8RestartClient1 | 1.795 | 1.766 | 41.61 |
| M8RPC2_20261008_063412_47b0f8RestartClient2 | 1.792 | 1.779 | 42.11 |

All ten raw logs retain only the known 24 widget/HLOD warning lines and 14 installed Toolsets Python error lines each. No other warning/error categories. Reports were inspected independently of engine exit codes; all owned processes finished. Peak individual sampled working/private memory was 1.812/1.783 GiB. Elapsed values include phase/report inspection, not frame timing.

This proves one malformed quantity is rejected over a real owning-client connection before and after restart. It does not establish flood/race resistance, arbitrary invalid packets, inventory privacy after reconnect, Internet hosting, rendered FPS/stutter or the 16 GB minimum. M7/M8 manual and packaged-server gates remain open.
