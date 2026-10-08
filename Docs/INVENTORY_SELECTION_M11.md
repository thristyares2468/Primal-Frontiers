# Stable inventory selection — M11

The controller now remembers a stack GUID, not its numeric row. Removing an earlier stack moves the highlight with the same item. Split retains the selected source ID. If that stack expires, is consumed/dropped/transferred away, or disappears after replication/load, no other item inherits selection. The HUD says no stack is selected; Up/Down or D-pad deliberately selects a current stack. Closing/reopening the bag does not replace a lost choice. New local selection feedback shows the actual name/quantity, clearing the old refusal. First-use bag/deposit chooses the first available stack as before.

Consume/split/drop/store send the chosen existing GUID through the unchanged server-authoritative validation APIs. Invalid local selection sends no mutation RPC. This adds no items, save format, replicated state, assets or dependencies. The initial regression reproduced accidental dropping of replacement wood after food expiry.

## Exact verification

| Check | Outcome |
| --- | --- |
| PF.Input.InventorySelection before fix | FAILED7 assertions; engine0/strict verdict1. Automation_M11SelectionBefore_20261008_092218459_50014db0 retained |
| Same test after stable-ID fix | PASSED1/1. Automation_M11SelectionAfter_20261008_092442365_6d29383a;16.08 s; working/private2.929/2.784 GiB |
| Related regression | PASSED4/4: PF.Input.Gamepad, PF.Input.InventorySelection, PF.Inventory.Transactions, PF.Inventory.WorldTransfers. Automation_M11SelectionRegression_20261008_092832381_e40e63da;16.25 s;2.895/2.782 GiB |
| Final focused native | PASSED1/1. Automation_M11SelectionFinal_20261008_093208176_8117ade0;16.08 s;2.927/2.768 GiB |
| Final rendered720p | PF.UI.InventorySelectionLive PASSED1/1. M11Inventory720_20261008_093254278_5af0d388;30.16 s;3.045/4.051 GiB |
| Final rendered1440p | PF.UI.InventorySelectionLive PASSED1/1. M11Inventory1440_20261008_093403536_9b3050d2;30.12 s;3.208/4.428 GiB |

All successful reports: engine/strict verdict0, zero test warnings/errors. Native raw logs zero warning/error/ensure/fatal. Rendered raw logs retain24 known widget/HLOD warnings and14 engine Python toolset error lines, no crash/ensure/fatal. Final expired-selection and reselected-stack PNGs at both resolutions inspected. Reports: Saved/AutomationReports/<run>/index.json; screenshots: Saved/AutomationReports/ControlsUI/<run>/. Matching logs under Saved/Logs (native prefix PFAutomation_).

Builds: oracle Editor19.46 s; stable-ID Editor21.16 s; final feedback Editor6.84 s; final Development Game15.16 s — PASSED, no compiler warnings. A new live test was omitted by cached UBT discovery (“up to date”); supported -NoUBTMakefiles gathered/compiled it6.15 s without changing generated files manually. Earlier rendered passes before feedback cleanup retained: M11Inventory720_20261008_092909188_0e598404 and M11Inventory1440_20261008_093014720_f54c111c.

```powershell
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunNativeAutomation.ps1 -TestFilter PF.Input.InventorySelection -Label Selection
powershell.exe -NoProfile -ExecutionPolicy RemoteSigned -File Scripts/RunControlsAutomation.ps1 -Resolution 720 -TestCase Inventory
```

Rendered smoke uses real world expiry, real inventory and bound input delegates; it does not certify physical button delivery, human usability, sustained FPS or full separate-client acceptance. Uncapped/VSync0 confirmed. No new network run is claimed; authority/ownership/quantity checks remain unchanged. Personal tests remain nonblocking/unverified. Full M11 is still in progress; next independent slice is clearer damage/urgent-state and possession-transition HUD feedback using existing replicated vitals.
