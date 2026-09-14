

\---



\## `NETWORKING.md`



```md

\# Networking



\## Target



Primal Frontier supports:



\- Single-player

\- Cooperative multiplayer

\- Dedicated-server hosting

\- Future optional PvP



\## Authority Model



The game uses server-authoritative multiplayer.



The server validates:



\- Player movement where needed

\- Damage

\- Resource gathering

\- Inventory changes

\- Crafting

\- Building placement

\- Creature ownership

\- Creature commands

\- Technology progression

\- Saving and loading



\## Replicated State



Initial replicated systems:



\- Player movement

\- Health and survival attributes

\- Inventory summaries

\- Equipped items

\- Built structures

\- Creature movement and state

\- Combat events

\- World resource state



\## Multiplayer Tests



\- Two players connect to one server.

\- Players can see each other correctly.

\- Damage is consistent for both clients.

\- Gathering is validated by the server.

\- Inventory changes replicate.

\- Crafted items replicate.

\- Structures replicate.

\- Creature behaviour is visible to both players.

\- Disconnecting and reconnecting preserves player state.



\## Future PvP



PvP is not a primary early-development focus.



If added later:



\- Player damage requires dedicated balancing.

\- Building ownership and permissions need clearer rules.

\- Anti-cheat requirements increase.

\- Server performance and moderation become more important.

