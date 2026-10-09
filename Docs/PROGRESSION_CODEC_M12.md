# M12 — separate progression codec and validated legacy defaults

October9,2026. Pure bounded native encoding/decoding; no existing player/world writer, file rewrite, actor mutation, component/RPC/UI/reward or recipe restriction. Trello https://trello.com/c/p08EeRT7. Full M12 and Personal M8/M11 remain open. This is a prerequisite for future compatible world/player integration, not a completed world migration.

## Wire and legacy contract

FPFProgressionSaveFormat carries a separate PFXP/version1/length/CRC16byte envelope with at most16KiB. Payload is explicit XP/int32, knowledge-count/length-bounded ASCII IDs, craft-count/IDs; no reflected FString/TArray allocation from untrusted lengths, UObject paths or persisted level/point cache. Array caps32knowledge/128craft, IDs1–64bytes. Decode resolves only already-known catalog names, without interning unknown strings. Duplicate/unknown IDs, invalid accounting, future version, wrong signature/checksum/length, oversized/truncated/trailing bytes refuse atomically. CRC detects corruption, not authentication; only trusted server files may become authoritative state.

Encode validates the complete record/catalog before replacing output bytes. Decode builds a complete candidate, checks all bounds/known IDs and record prerequisites/accounting before replacing output. DecodeLegacyPlayer accepts a validated V1 player record through the existing player codec, then gives zero XP/empty knowledge/empty craft ledger. Existing inventory is not proof of crafting; no retrospective reward. Vitals/position/ID/stack IDs/item IDs/freshness remain exact; no file I/O or original bytes changed. Failed legacy reads preserve both output player and progression. Repeated legacy reads do not generate points.

Existing main world and player versions remain V1. No save reader/writer calls this new codec yet. A separate world envelope integration must validate every owner and retain legacy V1 fallback before runtime hooks; do not add XP rewards before their state can safely survive actual restart.

## Verification

Editor initial14.49s/final4.98s passed without compiler warnings; logs:`Saved/Logs/PFM12ProgressionCodecEditorBuild.log`, `PFM12ProgressionCodecFinalEditorBuild.log`. Native5/5 PASSED `Automation_M12ProgressionCodec_20261009_105939642_27cbf1c8`,16.23s,working/private2.913/2.798GiB. Exact tests PF.Progression.Codec,Records,PF.Persistence.PlayerRoundTrip,CorruptPlayerData,WorldRecords. All raw/test warnings/errors0,engine/runner0,no timeout/assertion/ensure/fatal. Records/default catalogs and corrupt-byte fixtures only, no level required. No failure in this codec gate.

Tests verify exact roundtripXP/knowledge/craft identities/derived points and no repeated craft award, version/signature/CRC/truncation/trailing-byte refusal, CRC-valid invalid XP/counts/lengths/unknown/nonASCII/duplicate data,16KiB cap, invalid encode preserving prior bytes. Real V1 player codec preserves position/vitals/stable ID/owned upgraded tool/two food items/12.5seconds freshness; corrupt legacy checksum preserves both outputs; repeated defaults remain zeroXP. This is not a real live save/reconnect or byte-to-world application test.

Report:`Saved/AutomationReports/<run>/index.json` and `run-summary.json`; log:`Saved/Logs/PF<run>.log`. No screenshot/FPS/VRAM measurement applicable to pure records. NullRHI process memory is not16GB certification; actual host31.93GiB and installed engine5.8.3 instead of requested5.8.2. Packaged Server limitation remains.

Development Game build PASSED21.83s without compiler warnings:`Saved/Logs/PFM12ProgressionCodecGameBuild.log`. Build logs and successful native raw log reviewed. Bounded Trello evidence/Done readback follows; no failed build/test in this slice.

Replay with other Unreal processes closed:

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Progression.Codec+PF.Progression.Records+PF.Persistence.PlayerRoundTrip+PF.Persistence.CorruptPlayerData+PF.Persistence.WorldRecords' -Label M12ProgressionCodec
```

Changed files: Progression/PFProgressionSaveFormat.h/.cpp, Tests/PFProgressionCodecTests.cpp and evidence docs. Existing player/world codecs, maps/assets/config/baseline recipes untouched. Next: separately verified owner-bound world archive compatibility, then focused authoritative PlayerState integration, successful-event earning, private replication, knowledge/purchase UI and one-/two-client restart tests. No adaptations/M13 advance or full M12 acceptance inferred.
