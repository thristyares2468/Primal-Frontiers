# M18 verified original copy into an unused slot

October22 follow-up: [ORIGINAL_COPY_COMMAND_M18.md](ORIGINAL_COPY_COMMAND_M18.md) records a separate non-Shipping operator-console wrapper. "No command yet" below describes this earlier API-only slice; current wrapper still never performs a live load, semantic migration or active-slot change.

October22 2026, bounded task https://trello.com/c/2425XbNK. Technical PASS for a
trusted backend primitive; fullM18, generic migration and manual recovery remain
incomplete. It is not a new UI/console command or automatic live-world rollback.

## Behavior

FPFSaveFileStore::CopyOriginalToNewSlot(SourceSlot, Edge, NewSlot, Error) reads
only the preserved original under Saved/Persistence/Originals. It validates a
bounded checksummed envelope before copying exact bytes to the new slot's A
generation. Payload, generation and SavedUtc remain unchanged: copying does not
renew food time or invent a newer save generation. Ordinary A/B writing can
subsequently advance the copied slot. The original stays unchanged, even when
both active source generations are corrupt. No payload/world/catalog migration,
owner rewriting, startup/load, remote request or UI side effect is performed.

Require game thread, valid bounded identifiers and a distinct slot (case aliases
also refuse). Reject any destination A/B generation, .pending file/directory or
existing Originals evidence. Missing/truncated/corrupt/unknown-version/oversized
source refuses before publication. No-replace archive creation avoids replacing
the destination A file; write/verification failure retains partial evidence and
must be handled explicitly. Nothing deletes a user's destination to retry.

This is a file-store primitive, not proof that the payload can be restored in
the current map/catalog. Caller must validate semantics and coordinate any
other processes. No cross-process whole-slot lock, OS-crash durability, actual
disk-full/ACL failure injection or automatic complete-world rollback is claimed.
There is no player/client RPC or ordinary console route to this API yet.

## Actual checks

Installed engine is5.8.3 CL58210709 despite requested5.8.2. No engine association
or setting changed. Editor/Game compile passes with no compiler severity:

- C:/UnrealSource/BuildLogs/M18OriginalCopy_Editor_20261022.log:67.40s,exit0.
- M18OriginalCopy_Game_20261022.log:58.01s,exit0.
- EditorFinal_20261022.log:13.43s,exit0, after adding corrupt-active-A/B case.
- GameFinal_20261022.log:24.81s,exit0, same final source.

First focused run Automation_M18OriginalCopy_20261021_232043926_641b9523:
2/2 clean,49.60s,2.998working/2.932privateGiB peak,exit0.

Final Automation_M18OriginalCopyFinal_20261021_232334499_2cfcf3fd:
6/6 clean,43.26s,2.961working/2.871privateGiB,exit0, no raw/test warnings, errors,
fatal or ensure. Exact paths under C:/UnrealProjects/PrimalFrontier:

- Saved/AutomationReports/<run>/index.json and run-summary.json.
- Saved/Logs/PF<run>.log (checked after launch).
- Ordinary Saved/Logs/PrimalFrontier.log is not this isolated run's evidence.

Passing selectors:
1. PF.Persistence.OriginalCopy.Exact — exact old time/generation/bytes, repeat
   refusal, ordinary A/B continuation, salvage despite corrupt active A/B,
   unchanged source evidence, genuine c3d221f writer envelope copy.
2. PF.Persistence.OriginalCopy.Refusal — invalid/overlong/path/case-alias/thread,
   missing/truncated/checksum/future-version/oversize originals, every A/B/pending
   file/directory and archived destination occupant; evidence retained.
3. PF.Persistence.OriginalBackup.
4. PF.Persistence.FileGenerations.
5. PF.Persistence.FrozenM8Records.
6. PF.Persistence.FrozenM8Restore.

636 pre-existing persistence files have identical before/after SHA256 aggregate
and count. No private file contents/paths enter the report. Tests generate only
unique AutomationCopy... slots and remove their explicit fixture files without
recursive directory deletion. No .uasset/.umap/Config or map identity changed.

## Reproduce (AI, no authored level or rendering)

Start here: close Unreal/game windows and open PowerShell in
C:/UnrealProjects/PrimalFrontier. Engine target Development Editor Win64 must
have been built after the source change. Runner uses one NullRHI Editor process.

1. Run `./Scripts/RunNativeAutomation.ps1 -TestFilter 'PF.Persistence.OriginalCopy+PF.Persistence.OriginalBackup+PF.Persistence.FileGenerations+PF.Persistence.FrozenM8Records+PF.Persistence.FrozenM8Restore' -Label M18OriginalCopyFinal -TimeoutSeconds 180`.
2. Expect all six named selectors Success/exit0, exact selection checked,
   raw/test warning/error/fatal/ensure0 and no timeout. Inspect the new unique
   report/log paths printed by the runner; a missing selector is a failure.
3. The tests make actual bounded files from synthetic/committed frozen data;
   they never choose a personal slot or apply the API to an authored level.

No multiplayer behavior or replicated world state changed. This native file
gate requires no new rendered two-client run and makes no rendered/manual,
uncappedFPS/stutter or minimum16GiB certification. Full-world recovery needs a
separate explicit semantic/operator workflow and authority/restart tests.
