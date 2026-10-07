# Local settings and performance

Open **P → Settings** while playing. Game, Graphics, Audio and Accessibility tabs are available with mouse, keyboard and controller. Arrow keys/D-pad select and change rows; controller LB/RB change tabs; Enter/A applies, B returns, Y selects defaults. Changes are a draft until Apply. Cancel discards the draft. Display changes require confirmation within 15 real-time seconds, including while the solo world is paused; timeout restores the prior display.

Game controls cover FOV, look sensitivity, vertical inversion and control hints. Graphics include window mode, supported display resolutions, render scale, VSync, uncapped/limited FPS, motion blur, depth of field, quality presets and individual scalability groups. PIE owns its own window resolution/mode, so those controls are unavailable there. Test display changes in a standalone game window.

Motion blur and depth of field default off. The default quality mix is Medium with High textures and 75% render scale, VSync off and unlimited FPS. This is a starting point, not a hardware certification. Lowering render scale reduces sharpness; raise it to 100% if performance permits. Preferences are stored by Unreal's GameUserSettings system, separately from future world saves.

Audio provides master, music, effects and UI volume plus unfocused muting. The project sound classes live under `/Game/PrimalFrontier/Audio`: route authored music to SC_Music, effects to SC_Effects and UI sounds to SC_UI. The default class is SC_Effects. Existing sounds with explicit engine/third-party classes need deliberate routing before the category slider can control them. The greybox has no authored music/dialogue; no media was downloaded or imported for this work.

Accessibility includes HUD text size, crosshair visibility/size and Unreal's colour-vision correction. Numeric survival values remain visible alongside coloured bars. These controls do not alter server gameplay rules.

Use the same map, camera, output resolution, render scale and quality when comparing performance. The old 960x540 sample cannot establish 1440p performance. PIE includes editor overhead and may throttle when unfocused. A paused menu sample is not a traversal benchmark. Keep FPS uncapped during measurement and record frame times, memory and hitches as well as average FPS.

October 8: settings persistence is verified at Editor startup and PIE (75% render scale, blur off). Prefer **Selected Viewport**, then **F11** for immersive view: a 1,200-frame stationary sample at the measured 2560x1392 viewport averaged 107.81 effective FPS, with no frame over 33.33 ms. New Editor Window still runs slowly: native trace identifies about 64 ms per D3D12 presentation, two presentations per engine frame. This is a verified launch-mode workaround, not a resolved driver/overlay cause or a full traversal benchmark. See MILESTONES.md for CSV/trace/log paths and the intermittent DXGI presentation launch failure.
