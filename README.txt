# NotBlood
A gameplay modification of Blood, based upon the EDuke32 driven Blood reverse-engineer port NBlood

New features include:
* Default midi uses a SC55 soundfont by trevor0402 (https://github.com/trevor0402/SC55Soundfont)
* Difficulty based invulnerability timer for player damage
* Smaller hitboxes for player projectiles
* Raymarching collision testing for player projectiles
* Replaced sector based damage logic for player explosions
* Vanilla mode with optional DOS mouse handling
* Quad damage replacement for guns akimbo powerup
* Randomize mode for enemies and pickups (multiplayer supported)
* Increase drawn sprites on screen limit from 2560 to 4096
* Autosaving on collecting keys and start of level
* Bullet projectiles for hitscan enemies
* Fixed bullet casings clipping into walls
* New enhancements menu under game options
* New last weapon key binding (default Q)
* Lower gravity of bullet casings and gibs underwater
* Improved spawning randomization logic for bloodbath mode
* Allow particle sprites to traverse through room over room sectors
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
* Customizable palette adjustment
* Weapon selection bar
* Mirror mode

### New cheats (press t in-game to type codes)
* BIG BERTHA - Toggle randomized lifeleech projectiles (works for alt fire mode)
* QSKFA - Toggle blood alpha alt fire missile for pitchfork
* ZOOMIES - Toggle fast enemies
* SONIC - Toggle fast player movement
* NO U - Activates reflect shots power-up

### Enhancements Options
* Replace guns akimbo with quad damage
    - Replaces the guns akimbo powerup with Quake's quad damage (lasts 22 seconds)
* Player damage invulnerability
    - Apply a short invulnerability state for the player for bullet hitscans/spirit/tesla damage
    - Invulnerability duration changes depending on enemy difficulty/current player health (lower health = longer invulnerability state)
* Explosions behavior
    - Replace the default vanilla explosion calculation with an new improved sector scanning system
    - Fixes rare cases where enemies appear to ignore explosive damage due to sector/span order
    - Includes two methods to choose from: NotBlood (integer based calculation) and Raze (double-precision floating-point calculation)
* Projectiles behavior
    - For player projectiles, it'll use the improved eduke32's clipmove() logic as well as raymatching per tick
    - This feature also adjusts certain hitboxes so it's easier to throw/target around corners
* Enemy behavior
    - Fixes various original 1.21 bugs with enemies such as:
    - Tiny Caleb using the wrong burning sprite
    - Enemies sometimes burning indefinitely
    - Ignited cultists switching weapons when extinguished in water
    - Cerberus spinning uselessly on lava
    - Improved beast stomp attack sector scanning
    - Check if enemy is alive before setting target for AI
    - Fixes tesla cultists bugged prone attack sequence
* Random Cultist TNT
    - This will make cultists use a variety of random thrown sprites such as:
    - Napalm balls, proxy bundles, armed spray cans or pod projectiles
* Weapon behavior
    - Select between original weapon behavior, NBlood's V1.X behavior or NotBlood's tweaked weapon set which include:
    - Adjust pitch offset for spray/missile firing
    - Adds a charge up stab for pitchfork's alt fire
    - Makes lifeleech throwable and increases damage while in sentry mode
    - Do double melee damage if attacking enemies from 45 degrees behind
    - Allows tesla projectiles to be reflected back with reflective shots powerup
    - Adds a special alt fire missile attack for pitchfork's charge up while quad damage is active
* Sector behavior
    - Fixes room over room sector handling with hitscan calculations
    - This lets hitscan weapons to be fired when floating above water
* Hitscan projectiles
    - Makes enemies that use hitscan bullets spawn physical sprite based bullets with travel time
    - Projectile speed is adjusted depending on difficulty and if bullet is underwater
* Randomizer mode
    - Set the enemy/pickups randomizer mode
* Randomizer seed
    - Set the enemy/pickups randomizer's seed
    - An empty string will regenerate anew for every level start

### New console variables for NotBlood
* notarget
* r_mirrormode
* r_rotatespriteinterp
* r_rotatespriteinterpquantize
* r_shadowvoxels
* cl_chatsnd
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
* hud_showweaponselectscale

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
* PESTCTRL - No hands/rats/spiders
* IH8PETS! - No hands/rats/spiders/bats/hell hounds

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
   - make a copy of `game.ins` (or `game.inst`) named `game.cue`
   - mount the `.cue` as a virtual CD (for example with `WinCDEmu`)
   - copy the `movie` folder from the mounted CD into NotBlood's folder
5. Launch NotBlood (on Linux, to play Cryptic Passage, launch with the `-ini CRYPTIC.INI` parameter)