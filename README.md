[![Build](https://github.com/carnivoroussociety/NotBlood/actions/workflows/build.yml/badge.svg)](https://github.com/carnivoroussociety/NotBlood/actions/workflows/build.yml) [![Build](https://raw.githubusercontent.com/carnivoroussociety/NotBlood/master/.github/workflows/download.svg)](https://github.com/carnivoroussociety/NotBlood/releases)

# NotBlood
A gameplay modification of Blood, based upon the EDuke32 driven Blood reverse-engineer port NBlood

New features include:
* Default midi uses a [SC55 soundfont by trevor0402](https://github.com/trevor0402/SC55Soundfont)
* Difficulty based invulnerability timer for player damage ([before](https://files.catbox.moe/ucs7gp.mp4)/[after](https://files.catbox.moe/8hyaqm.mp4))
* Smaller hitboxes for player projectiles ([before](https://files.catbox.moe/3peiru.mp4)/[after](https://files.catbox.moe/zso8g4.mp4))
* Raymarching collision testing for player projectiles ([before](https://files.catbox.moe/qxtv05.mp4)/[after](https://files.catbox.moe/vo03ck.mp4))
* Replaced sector based damage logic for player explosions ([before](https://files.catbox.moe/h6xcrg.mp4)/[after](https://files.catbox.moe/35e08d.mp4))
* Vanilla mode with optional DOS mouse handling
* Quad damage replacement for guns akimbo powerup
* Randomize mode for enemies and pickups (multiplayer supported)
* Increase drawn sprites on screen limit from 2560 to 4096
* Autosaving on collecting keys and start of level
* Bullet projectiles for hitscan enemies
* Fixed bullet casings clipping into walls
* New enhancements menu under game options
* New last weapon key binding (default Q) ([demo](https://files.catbox.moe/28cirg.mp4))
* Lower gravity of bullet casings and gibs underwater
* Fix blood/bullet casings not being dragged with sectors
* Improved spawning randomization logic for bloodbath mode
* Cloak powerup hides player weapon icon for bloodbath mode
* Allow particle sprites to traverse through room over room sectors
* Switch to last active weapon if TNT/spray can is active when entering water
* Increased blood splatter duration and improved floor/slope collision detection
* Basic room over room support for positional audio ([before](https://files.catbox.moe/qca0k4.mp4)/[after](https://files.catbox.moe/wq1so4.mp4))
* Fixed missiles colliding with water sector edges ([before](https://files.catbox.moe/38t9t8.mp4)/[after](https://files.catbox.moe/smvi92.mp4))
* Fixed underwater issue with hitscan weapons ([before](https://files.catbox.moe/k9dxjj.mp4)/[after](https://files.catbox.moe/gfahdq.mp4))
* Respawning enemies option for singleplayer
* Custom difficult options for singleplayer
* Set item box selection to activated item
* Ability to record 1.21 compatible demos
* Interpolated level texture panning
* Drag and drop folder mod support ([demo](https://files.catbox.moe/lb7nxb.mp4))
* Customizable palette adjustment
* Weapon selection bar ([demo](https://files.catbox.moe/0zh37q.mp4))
* Mirror mode

### Installing
1. Extract NotBlood to a new directory
2. Copy the following files from Blood 1.21 to NotBlood folder:

   ```
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
   VOXEL.DAT
   ```

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
