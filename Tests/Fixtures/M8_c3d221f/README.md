# Frozen historical M8 writer fixture

Do not regenerate this baseline during normal tests. `fixture.json` contains the exact2022-byte worldV1 payload and2054-byte fileV1 envelope emitted by production writers at commit `c3d221fdd3a68daacb58b72d0e8a021fff19d9c3`. Both are Base64 text; the current writer did not re-encode them as old data. `manifest.json` records SHA256/source/archive/harness provenance, byte sizes, original generation/UTC and regression hashes.

All repeated GUIDs are deliberate fake test identities, including the `22222222` reconnect value. This is not a user world or credential and must never be used as a real hosted world/account. It has one player, foundation/chest, depleted wood node, creature/spawner, pickup, separate bag/storage food batches and no progression or chest-filter fields. Current tests require neutral defaults, exact permanent quantities/identities, no free progression and natural offline expiry without renewing old food.

Generation rebuilt historical source with installed UE5.8.3 CL58210709 and an added synthetic test harness. It is **not** an archived original executable or proof of a5.8.2 build. Only source/config/project/plugin were exported; launch config in the temporary copy selected `/Engine/Maps/Entry` and an empty Content directory. No historical assets/private saves downloaded or checked out. Production writer sources were hashed before/after generation and stayed identical. Temporary project was removed after validating its exact owned path. Only small fixture/manifest files are checked in; binaries/source ZIP/build reports remain untracked generated evidence.

Reproduce a NEW candidate (never overwrites this frozen baseline) from project root, with Unreal closed:

```powershell
.\Scripts\GenerateLegacyM8Fixture.ps1
```

This performs a source-only historical Editor build and exact `PF.Persistence.LegacyWriterExport` NullRHI test. It requires the local historical Git commit and installed engine, creates no service/dependency, and accepts only clean raw/test logs. Output is a unique `Saved/AutomationReports/LegacyM8Writer_<UTC>_<guid>` directory. Timestamps differ between candidates; do not replace this baseline simply because a newly generated hash differs. Any future baseline update requires reviewed writer provenance and expected reader behavior. Normal tests only read this folder.

Current verification: `PF.Persistence.FrozenM8Records` and `PF.Persistence.FrozenM8Restore`; no authored level/user action required. Exact build/run/retained-warning evidence is in `Docs/FROZEN_WRITER_M18.md`. Hashes detect regression, not authentication. Multi-engine/archive-executable/content-version/OS-crash/network compatibility remains separate.
