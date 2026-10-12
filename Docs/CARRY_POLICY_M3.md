# M3 — Stackable defaults and soft player carrying

October12,2026, https://trello.com/c/PRleHRMh. Baseline resource/food stacks now allow999,000; tools/weapons/protection stay single items. Per-unit kg, GUIDs,8slots, batch-specific expiration and atomic mutations remain. Player30kg is a soft carrying threshold; container/chest60kg remains hard. Movement penalties are the next slice, not passed here.

Trusted runtime/save-validation policy distinguishes bags/pickups from hard containers. Legal overweight records load through existing V1 inventory/V1–V2 world codecs. Unknown IDs, duplicate GUIDs, oversized quantities/slots, invalid deadlines, nonfinite/unrepresentable totals remain refused. No save format or binary asset change. Existing hard-refusal fixtures explicitly opt into hard policy; legacy quantity rejection uses999001 rather than21.

Final Editor5.46s (PFCarryFileEditorBuild.log); production Editor5.51s and Game44.36s (PFCarryRegressionEditorBuild.log, PFCarryPolicyGameBuild.log), clean. Retained initial test-only C2665 void-Login build failure39.83s and missing-clock capture failure Automation_CarryPolicy_20261012_053000117_991d00bc; corrected test fixture only. Retry1/1 then final real-file1/1 passed.

Final native reports in Saved/AutomationReports:
- Automation_CarryFile_20261012_053615622_102df59c: PF.Inventory.CarryPolicy1/1; actual loaded catalogs999k, max stone drop/pickup/capture, strict container refusal and real save/load twice without duplication. Disposable fixture slot cleaned using Unreal API; no personal save edits.
- Automation_CarryTransactions_20261012_053242913_a66b68c8:6/6 Inventory.LargeStacks/Transactions/WorldTransfers, Persistence.PlayerRoundTrip/PlayerValidation/CorruptPlayerData.
- Automation_CarryWorld_20261012_053324680_c3a9a719:6/6 Crafting.Transactions/Gathering, Building.PlacementAndStorage, Persistence.WorldRuntime/WorldRecords/OfflineFoodAging.
- Automation_CarryProgression_20261012_053407116_3e0899f9:5/5 Crafting.ToolProgression/WeaponProgression/Protection, Progression.Component/GatherEvents.

18distinct final native tests passed, exact selections/exits0/test and raw severity0/no timeout/fatal/ensure. Peak3.019working/2.875privateGiB. No level required: transient worlds/NullRHI. No rendered/network/movement/human/FPS certificate from this slice. Runtime stock assets read only. Actual host31.93GiB, engine5.8.3/requested5.8.2 discrepancy and installed packaged-Server restriction retained. Next: predicted encumbrance/sprint/glide and server-validated low-ledge climb, real one-client/rendered then two NullRHI clients.
