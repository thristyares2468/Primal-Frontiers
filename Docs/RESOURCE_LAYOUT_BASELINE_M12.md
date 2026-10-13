# M12 — saved resource-layout baseline

Read-only audit October14,2026, published source4aa5e37. Level `/Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld`,85 saved actors/descriptors,15 resources,2 starts and2 spawners. Actor GUIDs/transforms and current serialized catalog fields are in the project-local audit JSON; no private world/profile records were read.

| Persistent actor name | Resource definition |
| --- | --- |
| PFResourceNode_0 | Node_Wood |
| PFResourceNode_1 | Node_Stone |
| PFResourceNode_2 | Node_Food |
| PFResourceNode_3 | Node_Wood |
| PFResourceNode_4 | Node_Wood |
| PFResourceNode_5 | Node_Stone |
| PFResourceNode_6 | Node_Food |
| PFResourceNode_7 | Node_Food |
| PFResourceNode_8 | Node_Stone |
| PFResourceNode_9 | Node_Water |
| PFResourceNode_10 | Node_Water |
| PFResourceNode_11 | Node_Fibre |
| PFResourceNode_12 | Node_Fibre |
| PFResourceNode_13 | Node_Stone |
| PFResourceNode_14 | Node_Food |

Starts are PlayerStart_1 at(-250,-1100,100) and PlayerStart_2 at(250,-1100,100), both non-spatial. Resource actors are non-spatial; preserve all names/GUIDs/configuration and saved depleted/respawn states in any later approved mapping. Serialized assets contain13 items,8 recipes,5 resource definitions; legacy harvest family/grade requirements are neutral. Native constructors alone cannot be treated as proof that a later saved asset contains new entries; supported guarded catalog authoring/readback is required.

Proposed first ore candidate `(0,-850,60)` is between the starts and current Stone node; NOT CREATED and runtime capsule/placement/reach clearance is unverified. A later implementation must add its actual stable name/GUID/definition, mark the fixture, validate normal paths/spawn and provide explicit old-to-new resource-manifest mapping. Do not bypass strict count validation, rewrite old node states or ask a user to test an absent ore fixture.

Evidence: `Saved/AutomationReports/M12LayoutAudit_20261013_213325591_d5070ee7/layout.json`, matching `Saved/Logs/PFM12LayoutAudit_20261013_213325591_d5070ee7.log`, commandlet exit0,0 script warnings/errors, reported1993MB peak physical/1897MB peak virtual. Raw shutdown crowd-manager/Recast warning and existing imported MetaHuman custom-version display remain separate; no render/FPS/minimum-spec claim. First audit213030282_df2219fc also succeeded; final script stores semantic tags instead of nondeterministic struct memory-address repr. `git diff --name-only -- Content Config` empty after both runs. No C++ change/new build required for this script/document audit.
