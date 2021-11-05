# NotBlood
A gameplay modification of Blood, based upon the EDuke32 driven Blood reverse-engineer port NBlood

New features include:
* Default midi uses a SC55 soundfont by trevor0402 (https://github.com/trevor0402/SC55Soundfont)
* Difficulty based invulnerability timer for bullet/spirit/tesla damage
* Smaller hitboxes for player projectiles
* Raymarching collision testing for player projectiles
* Replaced sector based damage logic for player explosions
* Quad damage replacement for guns akimbo powerup
* Throwable lifeleech alt fire and cheat 'big bertha'
* Increased enemy damage for lifeleech's sentry mode
* Randomize mode for enemies and pickups (multiplayer supported)
* Increase drawn sprites on screen limit from 2560 to 4096
* Autosaving on collecting keys and start of level
* Reflection sphere will reflect tesla fire
* Charge up alt fire for pitchfork
* Bullet projectiles for hitscan enemies
* New enhancements menu under game options
* New last weapon key binding (default Q)
* Fixed various original glitches with burning enemies
* Fixed underwater cultists changing weapons when ignited
* Improved spawning randomization logic for bloodbath mode
* Switch to last active weapon if TNT/spray can is active when entering water
* Increased blood splatter duration and improved floor collision detection
* Basic room over room support for positional audio
* Fixed missiles colliding with water sector edges
* Fixed underwater issue with hitscan weapons
* Respawning enemies option for singleplayer
* Custom difficult options for singleplayer
* Set item box selection to activated item
* Ability to record 1.21 compatible demos
* Interpolated level texture panning
* Drag and drop folder mod support
* Weapon selection bar
* Mirror mode

### New cheats (press t in-game to type codes)
* BIG BERTHA - Toggle randomized lifeleech projectiles (works for alt fire mode)
* QSKFA - Toggle blood alpha alt fire missile for pitchfork
* ZOOMIES - Toggle fast enemies
* SONIC - Toggle fast player movement
* NO U - Activates reflect shots power-up

### Enhancements Options
* Replace guns akimbo with quad damage - Self explanatory, replaces the guns akimbo powerup with Quake's quad damage (lasts 25 seconds)
* Hitscan damage invulnerability - Apply a short invulnerability state for the player for bullet/spirit/tesla damage
* Explosions behavior - Replace the default vanilla explosion calculation with an new improved sector scanning system (fixes cases where enemies did not take explosive damage)
* Projectiles behavior - For player projectiles, it'll use the improved eduke32's clipmove() logic as well as raymatching per tick. This feature also adjusts certain hitboxes so it's easier to target around corners
* Enemy behavior - Fixes bugs with the vanilla enemy behavior such as tiny Caleb using the wrong burning sprite, enemies sometimes burning indefinitely, cultists switching weapons if extinguished in water, Cerberus spinning uselessly on lava, etc
* Weapon behavior - Adds a charge up stab for pitchfork's alt fire, makes lifeleech throwable and increass the damage in sentry mode, and adds a special alt fire missile attack for pitchfork's charge up if quad damage is active
* Sector behavior - Fixes room over room sector handling with hitscan calculations (this allows hitscan weapons to be fired when floating above water)
* Hitscan projectiles - Makes enemies that would use hitscan bullets spawn physical sprite based bullets with travel time
* Randomizer mode - Set the enemy/pickups randomizer mode
* Randomizer seed - Set the enemy/pickups randomizer's seed (an empty string will regenerate anew for every level start)

### New console variables for NotBlood
* notarget
* r_mirrormode
* r_rotatespriteinterp
* r_rotatespriteinterpquantize
* r_shadowvoxels
* cl_idletalk
* cl_interpolatepanning
* cl_interpolateweapon
* cl_weaponhbob
* cl_slowroomflicker
* cl_shadowsfake3d
* cl_smoketrail3d
* cl_particlesduration
* cl_packitemswitch
* hud_powerupdurationticks
* hud_showweaponselect
* hud_showweaponselecttimestart
* hud_showweaponselecttimehold
* hud_showweaponselecttimeend
* hud_showweaponselectposition

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
* OKBOOMER - TNT cultists only
* SNEAKYFU - Proned shotgun/tommy gun cultists only

### Additional randomizer seed cheats
* GHSTBSTR - No phantoms
* SAFEWORD - No hands
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