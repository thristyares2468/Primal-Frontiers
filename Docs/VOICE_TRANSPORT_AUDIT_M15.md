# M15 voice transport readiness — October15

Audit task https://trello.com/c/i7Lh8WDt ProgrammingDone/complete readback. Next user scope choice https://trello.com/c/91ZwTwxp (Personal 2), MainToDo/incomplete confirmed. Documentation only: no plugin/module/provider/account/netdriver/config/audio capture change or new build. Policy3117b35 and actual authority bridgee65bf35 are pushed; metadata tests are not an audio implementation.

## Actual project

- PFSessionGameInstance manages solo world intent/OpenLevel; multiplayer join is direct IP/ClientTravel through existing menu/controller. No production online lobby/account/session provider selected in project Config/Build.cs/uproject.
- Uncooked dedicated diagnostics actually mount the engine's OnlineSubsystemNull and use IpNetDriver; development PersistentPlayerId/reconnect GUID is not EOS ProductUserId, Steam account auth or an Internet identity.
- Build.cs has no active Voice/VoiceChat/OnlineSubsystem dependency (only historical comments). Engine-installed VoiceChat interface/EOSVoiceChat/OnlineSubsystemNull packages exist; presence on disk/default Null fallback is not project integration.
- Native metadata grants no client transport/API/device rights and does not create channels. Full M15 still needs device settings/PTT UI, rate/queue bounds, join/leave/token/identity handling and physical listening.

## Supported choices and limitations

| Scope | Available engine path | What it still requires |
| --- | --- | --- |
| Bounded Windows direct-IP/LAN development prototype | Built-in OnlineSubsystemNull legacy voice/Voice/OnlineSubsystemUtils plus reviewed native connection/channel integration | New explicit dependencies, opt-in-only local capture lifecycle, server ingress bound to the actual originating connection, route policy before forwarding, bounded unreliable voice work, decoder/output gain and real two-device listening. No Internet account security/production claim. |
| Online service direction | VoiceChat interface with a supported provider such as EOSVoiceChat, explicit product/user identity and trusted-server channels | User-owned product/platform decision, Developer Portal setup, product/deployment/client policy configuration and trusted token service; server secrets stay server-side/local secure setup, never chat/Git. Provider integration is not a one-line proximity toggle. |

The generic [Voice Chat Interface](https://dev.epicgames.com/documentation/unreal-engine/voice-chat-interface-in-unreal-engine) supports user lifecycle, channels, devices and mute/block. It is an abstraction requiring a provider. [Epic's EOS integration guide](https://dev.epicgames.com/documentation/en-us/unreal-engine/voice-chat-with-epic-online-services) requires product configuration and trusted-server room credentials for that method. No application/product or token authority exists in this repo.

Important installed-source finding: FEOSVoiceChatUser::Set3DPosition is explicitly unimplemented in UE_5.8's EOSVoiceChatUser.cpp:652; new sessions use NonPositional at1110. Do not infer proximity from the generic Positional enum or advertise automatic3D EOS audio. EOS room/local receive-volume controls are not by themselves an authoritative distance/privacy boundary. [EOS voice guidance](https://dev.epicgames.com/docs/epic-online-services/multiplayer/voice-and-rtc-interface/voice-interface/using-voice) distinguishes local receive controls from trusted-server access/tokens/admin operations. Any online choice needs a measured room/routing design, not global room plus a client-only volume slider.

The existing installed legacy path likewise needs integration: VoiceChannel.cpp:29 deserializes incoming audio and calls NetDriver::ReplicateVoicePacket; NetDriver.cpp:1684 skips originating connection and checks recipient mute readiness. NetConnection.cpp::ShouldReplicateVoicePacketFrom is not a virtual project hook and checks sender validity/mute handshake, not the new distance policy. Received transport's identity cannot simply be accepted as a trusted public gameplay ID. No claim is made that a forged packet actually succeeds; the missing project identity/range integration is the finding. Do not edit Engine source or add an unreviewed NetDriver merely to expose a toggle.

## Recommended next bounded scope / required decision

Recommend a direct-IP/LAN voice prototype first to match the current project and avoid forcing online account setup now. It remains a dedicated-server-compatible development slice, not production authenticated Internet voice. Before implementation, the user should choose that scope or their intended online platform/provider. This is a product/architecture preference, not a new microphone listening test or a request for secrets. User input can be one line: `LAN voice first` or `Online voice — <intended platform/provider>`. If they already have a platform/product configured, report its existence only; private credentials/configuration use a separate secure setup after the choice.

After the choice: explain exact dependencies, implement one bounded opt-in lifecycle and ingress/routing bridge, compile/narrow tests, actual1then2headless roles for metadata/packet authority, then expose reachable menu controls. Only after playable capture exists provide a precise Personal two-device near/far/PTT/mute/disable guide. Stop before physical acceptance if no second participant/device is available; never claim NullRHI metadata heard speech.

No level required for this audit; no commands for the user to run. Existing baseline /Game/PrimalFrontier/Maps/L_PrimalFrontier_OpenWorld, future playable voice fixture/controls **TBD**. Other unreported COPPER_LOOP/STATS_UPGRADE/physicalcontroller/audio/1440p/renderedmultiplayer gates remain separate. More playable mineral tiers remain behind ordinary COPPER_LOOP acceptance; backend evidence does not bypass it.

## Local evidence and verification

Reviewed project .uproject/Build.cs/Config, PFSessionGameInstance and controller travel, current successful M15VoiceMetadata2 logs, installed VoiceChat.h, EOSVoiceChatUser.cpp, VoiceChannel.cpp/NetDriver.cpp/NetConnection.cpp and Null voice implementation header. Official documentation refreshed October15; service interfaces may change, recheck at integration. No assets/maps/settings/saves/code changed by this audit; docs consistency/diff checks suffice. Both targets and final prior3native+5actual role records remain passed, failed fixture histories retained in PROXIMITY_VOICE_AUTHORITY_M15.md. Separate earlier D3D12 GPU crash is unresolved; this audit does not diagnose driver/Lumen root cause.
