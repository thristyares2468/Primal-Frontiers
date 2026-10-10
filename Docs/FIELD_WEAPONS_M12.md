# M12 — Field weapon knowledge

Bounded technical checkpoint, October 10. Full M11/M12 and existing Personal usability, combat/pacing, controller and rendered-save gates remain open. Installed engine is 5.8.3 versus requested 5.8.2; host reports 31.93 GiB RAM, not a minimum-16GB certificate.

## Behavior and compatibility

The loaded C++ progression catalog now includes original `Tech_FieldWeapons`: level 3 (250 XP), three points, prerequisite `Tech_FieldTools`. It unlocks only the existing `Recipe_BoundClub`. Field Tools remains level 2/two points. Both purchases cost five points total; at level three six earned points leave one. Baseline tool, cord, wooden club, food, guard and building stay available. No item/recipe IDs, assets, schema, damage, timer, durability/equipment or dependencies changed.

Purchase and craft reservation/completion continue through existing owned authoritative validation. Client IDs cannot assert prices, XP or another player's record. Record and feedback remain owner-only. An old V1 inventory containing a stone-bound club retains its sixty damage at zero progression; recreating it needs the new knowledge. No confiscation, retrospective reward, private save migration or grant.

The unchanged club conversion consumes one wooden club, one cord, two stone and two wood after eight seconds. Cancel/duplicate calls conserve inputs and grant no XP; first completion gives 20 XP once. Club adds no gathering bonus; the carried bound tool still gathers three hits. Creature protection remains 25%, nonstacking.

## Exact completed evidence

Paths below are project-relative; reports live under `Saved/AutomationReports`, raw logs under `Saved/Logs`. Generated files are intentionally untracked.

| Gate | Exact run / result | Seconds | Peak working / private GiB |
| --- | --- | --- | --- |
| Initial Editor | `PFM12FieldWeaponsEditorBuild.log`, passed, no compiler warnings | 18.80 | build host only |
| Native earned route | `Automation_M12EarnedWeapon_20261010_050620023_a715a354`, `PF.Progression.EarnedWeapon`, 1/1 | 34.16 | 3.133 / 3.044 |
| Native affected regressions | `Automation_M12FieldWeaponRegression_20261010_050730009_161f6bcb`, 6/6 | 16.64 | 2.977 / 2.830 |
| Live fixture include retry Editor | `PFM12FieldWeaponsLiveEditorRetry.log`, passed, no compiler warnings | 7.55 | build host only |
| Rendered 720p / HUD 1.5 | `M12EarnedWeapon720_20261010_051712800_53e9529c`, `PF.Progression.EarnedWeaponLive`, 1/1 | 136.17 | 3.168 / 5.342 |
| Scroll fixture Editor | `PFM12FieldWeaponsScrollEditorBuild.log`, passed, no compiler warnings | 7.41 | build host only |
| Corrected rendered 1440p / HUD 1.5 | `M12EarnedWeapon1440_20261010_052410389_5b0ee6c7`, `PF.Progression.EarnedWeaponLive`, 1/1 | 135.79 | 3.244 / 5.651 |
| One NullRHI client create/restart | `M12KnowledgeWeaponLive1_20261010_052214736_eb4bdcee`, four `PF.Progression.KnowledgeLive` role reports, 4/4 | per-role summary | 1.719–1.806 / 1.563–1.743 |
| Two NullRHI clients, 75ms/1%loss, create/restart | `M12KnowledgeWeaponLive2_20261010_052654170_cbe20c63`, six role reports, 6/6 | per-role summary | 1.719–1.809 / 1.563–1.747 |
| Game Development Win64 | `PFM12FieldWeaponsGameBuild.log`, passed, no compiler warnings | 24.84 | build host only |
| Shared base Knowledge create/restart regression | `M12KnowledgeLive1_20261010_053045040_7b0b94ff`, four role reports, 4/4 | per-role summary | 1.719–1.803 / 1.564–1.736 |

Six native selectors: `PF.Crafting.WeaponProgression`, `PF.Progression.EarnedUpgrade`, `PF.Progression.KnowledgeRequests`, `PF.Progression.RecipeAccess`, `PF.Progression.Records`, `PF.UI.ProgressionDetails`. Exact selectors/exits/test severity were checked; no timeout. Shared base single-purchase network regression passed4/4, preserving the existing100XP/FieldTools path; its four raw logs also match the network baseline/no fatalensure. All technical gates for this bounded slice passed; parent/manual milestone acceptance stays open.

### Earned route versus seeded boundary proof

Native test starts empty at zero XP with normal eight slots/30kg. Twenty-five actual gathers across five categories earn 125 XP; eight distinct timed crafts earn 160, ending at 285 XP/level three/one remaining point. Normal unused drops free capacity, preserve batch expiry and earn nothing. No injected player XP/items, expanded capacity, clock writes or shortened jobs. Actual bound-club damage is sixty.

Rendered standalone route first completes the existing earned guard/default creature/ordinary death/default respawn conservation exercise at 205 XP. Four remaining stone credits and five water credits earn 45 more. It buys Field Weapons through actual Slate controls at 250 XP, makes normal repeated cord without bonus XP, cancels then completes the unchanged club for 270 XP/eight credited recipes/one point. Two accepted traced swings kill a default-100HP creature; each costs five stamina, immediate repeats refuse. Target placement and AI tick isolation are controlled test conditions, not navigation/human combat feel. Fourteen 720p PNGs were inspected, including locked/available/learned knowledge, completion, damage, death/respawn and inventory loot. Screens: `Saved/AutomationReports/ControlsUI/<run>/`.

Network fixtures separately seed trusted 250 XP and inputs only for request boundaries. They do not prove earned multiplayer pacing. One owner buys parent then child; duplicate/direct client mutation refuses, bound-club craft/cancel conserves all six ingredient/identity stacks and no craft reward. Actual save, clean new-server launch and identity reconnect retain the two-node graph/one point. Both practical one- and two-client runs passed; the second owner retains six points, no knowledge, locked optional recipe and exact inputs. Foreign XP/knowledge/craft ledger/feedback remain private. Client2 did not crash with NullRHI.

## Retained failures and narrow fixes

- `PFM12FieldWeaponsLiveEditorBuild.log`: failed in 13.43s, C2027 incomplete `UPFItemPicture` plus dependent C2661. Added the concrete UI header; retry passed. Production untouched by this fixture fix.
- `M12EarnedWeapon1440_20261010_051946220_4afac396`: 135.25s, one failed “Earned feedback inside inner scroll” assertion, no timeout, engine exit zero/strict verdict one; 3.322/5.140 GiB. Actual screenshot clips the reward's final wrapped line because fixture requested end scrolling before completion reflow settled. Added a later supported `ScrollWidgetIntoView` request on the painted reward, then another paint wait; failed-only1440p replay passed and its complete reward text was inspected. Keep failed report/screenshot. All learning/conversion/combat assertions completed; never call the original failed run passed.

## Logs, limits and replay

Initial native logs contain no warning/error/assert/fatal/ensure. Final720p and corrected1440p normalized severity match the established rendered baseline (26 warning lines plus 14 experimental Python definition errors); all ten advanced network logs match the established network baseline (24 warnings plus the same 14 errors), with zero new normalized findings or fatal/ensure. These raw startup findings are not zero-error logs. Known categories include EditorDataStorage, uncooked HLOD, and rendered scalability priority. Native automation events themselves have zero warnings/errors. Ordinary PrimalFrontier.log is older; the unique launch logs above are actual current evidence.

720p/corrected1440p runner guards confirmed 290/296 unrelated save files and default preference hashes unchanged. All28 final PNGs inspected. Runs request uncapped FPS/VSync zero, but controlled rendered scenes are not sustained walking FPS or stutter measurements. No crash observed in completed final runs. Installed distribution still cannot build packaged Server targets; this slice uses supported uncooked dedicated `UnrealEditor -server`.

With no competing Unreal process, start from project root:

```powershell
./Scripts/RunNativeAutomation.ps1 -TestFilter PF.Progression.EarnedWeapon -Label M12EarnedWeapon
./Scripts/RunControlsAutomation.ps1 -TestCase EarnedWeapon -Resolution 720 -HUDScale 1.5 -TimeoutSeconds 240
./Scripts/RunControlsAutomation.ps1 -TestCase EarnedWeapon -Resolution 1440 -HUDScale 1.5 -TimeoutSeconds 240
./Scripts/RunPersistenceAutomation.ps1 -Players 1 -Knowledge -WeaponKnowledge
./Scripts/RunPersistenceAutomation.ps1 -Players 2 -Knowledge -WeaponKnowledge -SimulateLagLoss
```

Native uses a disposable Unreal Game world (no level). Rendered/network level is `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`; fixtures isolate their own profiles/slot/report names. Pass requires exact clean Success records, no timeout, exit zero, artifact inspection and normalized raw-log review. Stop on failure and fix only this scope. Ordinary player guidance is step3b of `TRELLO_TEST_GUIDES.md` in the same existing G2 session; no extra human task is assigned.
