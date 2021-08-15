# NotBlood
A gameplay modification of Blood, based upon the EDuke32 driven Blood reverse-engineer port NBlood

New features include:
* Default midi uses the excellent SC55 soundfont by trevor0402 (https://github.com/trevor0402/SC55Soundfont)
* Difficulty based invulnerability timer for bullet/spirit/tesla damage
* Smaller hitboxes for player projectiles
* Raytrace collision logic for player projectiles
* Proximity radius damage logic for player explosions
* Quad damage replacement for guns akimbo powerup
* Throwable lifeleech alt fire and cheat 'big bertha'
* Increased enemy damage for lifeleech's sentry mode
* Randomize mode for enemies and pickups (multiplayer supported)
* Autosaving on collecting keys and start of level
* Reflection sphere will reflect tesla fire
* Charge up alt fire for pitchfork
* Bullet projectiles for hitscan enemies
* New enhancements menu under game options
* New last weapon key binding (default Q)
* Fixed various original glitches with burning enemies
* Fixed underwater cultists changing weapons when ignited
* Fixed buggy weapon switching behavior
* Weapon selection bar
* Mirror mode

### New cheats (press t in-game to type codes)
* BIG BERTHA - Toggle randomized lifeleech projectiles (works for alt fire mode)
* QSKFA - Toggle blood alpha alt fire missile for pitchfork
* ZOOMIES - Toggle fast enemies
* NO U - Activates reflect shots power-up

### Randomizer seed cheats
* AAAAAAAA - Phantoms only
* BUTCHERS - Butchers only
* SOULSEEK - Hands only
* EPISODE6 - Cultists only
* GARGOYLE - Gargoyles only
* FLAMEDOG - Hell hounds only
* CAPYBARA - Rats only
* HURTSCAN - Shotgun/tommy gun cultists only
* HUGEFISH - Gill beasts only
* SHOCKING - Tesla cultists only
* CRUONITA - Boss types only
* BILLYRAY - Shotgun cultists only
* WEED420! - Cultists only but they're green
* BRAAAINS - Zombies only
* SNEAKYFU - Proned shotgun/tommy gun cultists only

### Additional randomizer seed cheats (can softlock levels if enemies have keys)
* GHSTBSTR - No phantoms
* NOHANDS! - No hands
* SAFEWATR - No hands/gill beasts
* PESTCTRL - No rats/hands/spiders
* IH8PETS! - No rats/hands/spiders/bats/hell hounds

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