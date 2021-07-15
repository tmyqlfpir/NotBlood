# NotBlood
A gameplay modification of Blood, based upon the EDuke32 driven Blood reverse-engineer port NBlood

New features include:
* Difficulty based invulnerability timer for bullet/spirit/tesla damage
* More forgiving hitbox size when testing wall collision for player spawned projectiles ([before](https://files.catbox.moe/3peiru.mp4)/[after](https://files.catbox.moe/zso8g4.mp4))
* Optional quad damage replacement for guns akimbo powerup
* Throwable lifeleech alt fire and cheat (big bertha)
* Increased enemy damage for lifeleech's sentry mode
* Randomize mode for enemies and pickups (multiplayer supported)
* Autosaving on collecting keys and start of level
* Mirror mode

### Installing
1. Extract NotBlood to a new directory
2. Copy the following files from Blood 1.21 to NotBlood folder:

   BLOOD.INI  
   BLOOD.RFF  
   BLOOD000.DEM, ..., BLOOD003.DEM (optional)  
   CP01.MAP, ..., CP09.MAP (optional, Cryptic Passage)  
   CPART07.AR_ (optional, Cryptic Passage)  
   CPART15.AR_ (optional, Cryptic Passage)  
   CPBB01.MAP, ..., CPBB04.MAP (optional, Cryptic Passage)  
   CPSL.MAP (optional, Cryptic Passage)  
   CRYPTIC.INI (optional, Cryptic Passage)  
   CRYPTIC.SMK (optional, Cryptic Passage)  
   CRYPTIC.WAV (optional, Cryptic Passage)  
   GUI.RFF  
   SOUNDS.RFF  
   SURFACE.DAT  
   TILES000.ART, ..., TILES017.ART  
   VOXEL.DAT  

3. Optionally, if you want to use CD audio tracks instead of MIDI, provide FLAC/OGG recordings in following format: bloodXX.flac/ogg, where XX is track number. Make sure to enable Redbook audio option in sound menu.
4. Optionally, if you want cutscenes and you have the original CD, copy the `movie` folder into NotBlood's folder (the folder itself too).
If you have the GOG version of the game, do the following:
   * make a copy of `game.ins` (or `game.inst`) named `game.cue`
   * mount the `.cue` as a virtual CD (for example with `WinCDEmu`)
   * copy the `movie` folder from the mounted CD into NotBlood's folder
5. Launch NotBlood (on Linux, to play Cryptic Passage, launch with the `-ini CRYPTIC.INI` parameter)

## Building from source
See: https://wiki.eduke32.com/wiki/Main_Page

## Acknowledgments
  See AUTHORS.md
