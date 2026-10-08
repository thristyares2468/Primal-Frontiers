# M11 action overlay readability

The native crafting and building panels now live on the right side of the first-person view. They leave the centred aim marker and bottom-left survival vitals clear, respect the local HUD scale, wrap long text, and refresh at 10 Hz while open. Crafting ingredient rows are compacted into readable per-recipe lines. Building storage shows four stacks and an explicit remainder count rather than overflowing an unbounded list.

Crafting keeps the server-owned job state and latest refusal/acceptance/cancellation result in a separate footer. Building labels its preview as advisory and shows the latest authoritative result separately. The interaction prompt is reflowed to the free view area while either action overlay is open; gameplay authority, RPC validation, save schema, input meanings and assets are unchanged.

## Verification

- Editor rebuild: `M11ActionOverlayStorageFitBuild_20261008.log`, passed 5.76 s, no compiler warnings.
- Game build: `M11ActionOverlayGameBuild_20261008.log`, passed 26.45 s, no compiler warnings.
- Native `PF.Crafting.Transactions`, `PF.Building.PlacementAndStorage`, `PF.Input.Gamepad`: 3/3 passed in `Automation_M11ActionOverlay_20261008_102112850_bd26f85a` (16.09 s; 2.925 / 2.774 GiB; raw/test warning and error counts zero).
- Rendered `PF.UI.ActionOverlaysLive`: 1/1 at maximum HUD scale 1.5 in both `M11Overlays720_20261008_102413446_f016a620` (27.04 s; 3.076 / 4.102 GiB) and `M11Overlays1440_20261008_102624552_bf04e6e7` (27.01 s; 3.209 / 4.425 GiB). Engine and strict verdicts are zero; no VSM overflow or test warning/error occurred. Screenshots were inspected for craft refusal/start/cancel and build refusal/storage states.

The isolated rendered logs retain the known engine startup categories (24 widget/HLOD warnings and 14 StateTree/Python lines), with no ensure, fatal or crash. `t.MaxFPS 0` and `r.VSync 0` were requested and confirmed; these short UI scenarios do not establish sustained traversal FPS.

Manual M7/M8 traversal, overnight survival, full rendered persistence and physical-controller checks remain unverified.
