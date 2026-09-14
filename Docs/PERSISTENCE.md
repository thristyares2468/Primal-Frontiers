\# Persistence



\## Save Requirements



The game must save:



\- Player location

\- Player attributes

\- Inventory

\- Equipped items

\- Technology progression

\- Creature ownership and state

\- Built structures

\- Storage contents

\- World resource state

\- Time of day and world settings

\- Frontier progression



\## Rules



\- The server owns saved multiplayer state.

\- Save data includes a version number.

\- Save format changes need migration plans.

\- Save failures must not overwrite a valid previous save.

\- Development saves should be easy to reset.

\- Important save operations should create backups.



\## Initial Implementation



\- Local save files for development

\- Manual save command

\- Periodic autosave

\- Load on server startup

\- Player state restoration on reconnect



\## Future Work



\- Automatic backup rotation

\- Admin restore tools

\- World migration tools

\- Cloud or database persistence if needed

\- Cross-server character transfer only if explicitly designed

