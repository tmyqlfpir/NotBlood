//-------------------------------------------------------------------------
/*
Copyright (C) 2010-2019 EDuke32 developers and contributors
Copyright (C) 2019 Nuke.YKT

This file is part of NBlood.

NBlood is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License version 2
as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
//-------------------------------------------------------------------------
#include <stdio.h>
#include "build.h"
#include "compat.h"
#include "mmulti.h"
#include "common_game.h"
#include "config.h"
#include "ai.h"
#include "asound.h"
#include "blood.h"
#include "demo.h"
#include "globals.h"
#include "db.h"
#include "messages.h"
#include "menu.h"
#include "network.h"
#include "loadsave.h"
#include "resource.h"
#include "screen.h"
#include "sectorfx.h"
#include "seq.h"
#include "sfx.h"
#include "sound.h"
#include "view.h"
#ifdef NOONE_EXTENSIONS
#include "nnexts.h"
#endif

GAMEOPTIONS gSaveGameOptions[kMaxLoadSaveSlot];
int gSaveGameProfileSkill[kMaxLoadSaveSlot];
char *gSaveGamePic[kMaxLoadSaveSlot];
unsigned int gSavedOffset = 0;

LoadSave LoadSave::head(123);
FILE *LoadSave::hSFile = NULL;
int LoadSave::hLFile = -1;

void LoadSave::Save(void)
{
    ThrowError("Pure virtual function called");
}

void LoadSave::Load(void)
{
    ThrowError("Pure virtual function called");
}

void LoadSave::Read(void *pData, int nSize)
{
    dassert(hLFile != -1);
    if (kread(hLFile, pData, nSize) != nSize)
        ThrowError("Error reading save file.");
}

void LoadSave::Write(void *pData, int nSize)
{
    dassert(hSFile != NULL);
    if (fwrite(pData, 1, nSize, hSFile) != (size_t)nSize)
        ThrowError("File error #%d writing save file.", errno);
}

void LoadSave::LoadGame(char *pzFile)
{
    const char bDemoWasPlayed = gDemo.bPlaying;
    const char bGameWasStarted = gGameStarted;
    if (gDemo.bPlaying || gDemo.bRecording)
        gDemo.Close();

    gViewPos = VIEWPOS_0;
    gViewIndex = myconnectindex;
    sndKillAllSounds();
    sfxKillAllSounds();
    ambKillAll();
    seqKillAll();
    if (!gGameStarted)
    {
        memset(xsprite, 0, sizeof(XSPRITE)*kMaxXSprites);
        memset(sprite, 0, sizeof(spritetype)*kMaxSprites);
        automapping = 1;
    }
    hLFile = kopen4load(pzFile, 0);
    if (hLFile == -1)
        ThrowError("Error loading save file.");
    LoadSave *rover = head.next;
    while (rover != &head)
    {
        rover->Load();
        rover = rover->next;
    }
    kclose(hLFile);
    hLFile = -1;
    if (!gGameStarted)
        scrLoadPLUs();
    InitSectorFX();
    viewInitializePrediction();
    PreloadCache();
    if (!VanillaMode()) // set reverb sound effect state
        sfxSetReverb(packItemActive(gMe, kPackDivingSuit) || powerupCheck(gMe, kPwUpReflectShots));
    ambInit();
#ifdef YAX_ENABLE
    yax_update(numyaxbunches > 0 ? 2 : 1);
#endif
    calc_sector_reachability();
    memset(myMinLag, 0, sizeof(myMinLag));
    otherMinLag = 0;
    myMaxLag = 0;
    gNetFifoClock = 0;
    gNetFifoTail = 0;
    memset(gNetFifoHead, 0, sizeof(gNetFifoHead));
    gPredictTail = 0;
    gNetFifoMasterTail = 0;
    memset(gFifoInput, 0, sizeof(gFifoInput));
    memset(gChecksum, 0, sizeof(gChecksum));
    memset(gCheckFifo, 0, sizeof(gCheckFifo));
    memset(gCheckHead, 0, sizeof(gCheckHead));
    gSendCheckTail = 0;
    gCheckTail = 0;
    gBufferJitter = 0;
    bOutOfSync = 0;
    for (int i = 0; i < gNetPlayers; i++)
        playerSetRace(&gPlayer[i], gPlayer[i].lifeMode);
    if (VanillaMode())
        viewSetMessage("");
    else
        gGameMessageMgr.Clear();
    viewSetErrorMessage("");
    viewResizeView(gViewSize);
    if (!gGameStarted)
    {
        netWaitForEveryone(0);
        memset(gPlayerReady, 0, sizeof(gPlayerReady));
    }
    gFrameTicks = 0;
    gFrame = 0;
    gCacheMiss = 0;
    gFrameRate = 0;
    totalclock = 0;
    gPaused = 0;
    gGameStarted = 1;
    

#ifdef USE_STRUCT_TRACKERS
    Bmemset(sectorchanged, 0, sizeof(sectorchanged));
    Bmemset(spritechanged, 0, sizeof(spritechanged));
    Bmemset(wallchanged, 0, sizeof(wallchanged));
#endif

#ifdef USE_OPENGL
    Polymost_prepare_loadboard();
#endif

#ifdef POLYMER
    if (videoGetRenderMode() == REND_POLYMER)
        polymer_loadboard();

    // this light pointer nulling needs to be outside the videoGetRenderMode check
    // because we might be loading the savegame using another renderer but
    // change to Polymer later
    for (int i=0; i<kMaxSprites; i++)
    {
        gPolymerLight[i].lightptr = NULL;
        gPolymerLight[i].lightId = -1;
    }
#endif

    if (gGameOptions.nEpisode >= gEpisodeCount || gGameOptions.nLevel >= gEpisodeInfo[gGameOptions.nEpisode].nLevels
        || Bstrcasecmp(gEpisodeInfo[gGameOptions.nEpisode].levelsInfo[gGameOptions.nLevel].Filename, gGameOptions.zLevelName) != 0)
    {
        if (!gSysRes.Lookup(gGameOptions.zLevelName, "MAP"))
        {
            gGameOptions.nEpisode = 0;
            gGameOptions.nLevel = 0;
        }
        else
        {
            levelAddUserMap(gGameOptions.zLevelName);
        }
    }

    if (MusicRestartsOnLoadToggle
        || bDemoWasPlayed
        || !bGameWasStarted
        || (gMusicPrevLoadedEpisode != gGameOptions.nEpisode || gMusicPrevLoadedLevel != gGameOptions.nLevel))
    {
        levelTryPlayMusicOrNothing(gGameOptions.nEpisode, gGameOptions.nLevel);
    }
    gMusicPrevLoadedEpisode = gGameOptions.nEpisode;
    gMusicPrevLoadedLevel = gGameOptions.nLevel;

    netBroadcastPlayerInfo(myconnectindex);
    //sndPlaySong(gGameOptions.zLevelSong, 1);

    if ((gGameOptions.nGameType == kGameTypeSinglePlayer) && (numplayers == 1)) // if single-player, update the game options/player profile by loading the current set settings
    {
        if (!VanillaMode())
        {
            gGameOptions.nMonsterSettings = ClipRange(gMonsterSettings, 0, 2);
            if (gMonsterSettings <= 1)
                gGameOptions.nMonsterRespawnTime = 3600; // default (30 secs)
            else if (gMonsterSettings == 2)
                gGameOptions.nMonsterRespawnTime = 15 * kTicRate; // 15 secs
            else
                gGameOptions.nMonsterRespawnTime = (gMonsterSettings - 2) * 30 * kTicRate;
        }
        gGameOptions.bQuadDamagePowerup = gQuadDamagePowerup;
        gGameOptions.bDamageInvul = gDamageInvul;
        gGameOptions.nExplosionBehavior = gExplosionBehavior;
        gGameOptions.nProjectileBehavior = gProjectileBehavior;
        gGameOptions.bEnemyBehavior = gEnemyBehavior;
        gGameOptions.bEnemyRandomTNT = gEnemyRandomTNT;
        gGameOptions.nWeaponsVer = gWeaponsVer;
        gGameOptions.bSectorBehavior = gSectorBehavior;
        gGameOptions.bHitscanProjectiles = gHitscanProjectiles;
        gGameOptions.nRandomizerMode = gRandomizerMode;
        Bmemcpy(gGameOptions.szRandomizerSeed, gzRandomizerSeed, sizeof(gGameOptions.szRandomizerSeed));
    }
}

void LoadSave::SaveGame(char *pzFile)
{
    hSFile = fopen(pzFile, "wb");
    if (hSFile == NULL)
        ThrowError("File error #%d creating save file.", errno);
    LoadSave *rover = head.next;
    while (rover != &head)
    {
        rover->Save();
        rover = rover->next;
    }
    fclose(hSFile);
    hSFile = NULL;
}

class MyLoadSave : public LoadSave
{
public:
    virtual void Load(void);
    virtual void Save(void);
};

void MyLoadSave::Load(void)
{
    psky_t *pSky = tileSetupSky(0);
    int id;
    Read(&id, sizeof(id));
    if (id != 0x5653424e/*'VSBN'*/)
        ThrowError("Old saved game found");
    short version;
    Read(&version, sizeof(version));
    if (version != BYTEVERSION)
        ThrowError("Incompatible version of saved game found!");
    short nSkill;
    Read(&nSkill, sizeof(nSkill));
    Read(&gGameOptions, sizeof(gGameOptions));
    Read(&numsectors, sizeof(numsectors));
    Read(&numwalls, sizeof(numwalls));
    Read(&numsectors, sizeof(numsectors));
    int nNumSprites;
    Read(&nNumSprites, sizeof(nNumSprites));
    memset(sector, 0, sizeof(sector[0])*kMaxSectors);
    memset(wall, 0, sizeof(wall[0])*kMaxWalls);
    memset(sprite, 0, sizeof(sprite[0])*kMaxSprites);
    memset(spriteext, 0, sizeof(spriteext[0])*kMaxSprites);
    Read(sector, sizeof(sector[0])*numsectors);
    Read(wall, sizeof(wall[0])*numwalls);
    Read(sprite, sizeof(sprite[0])*kMaxSprites);
    Read(spriteext, sizeof(spriteext[0])*kMaxSprites);
    Read(qsector_filler, sizeof(qsector_filler[0])*numsectors);
    Read(qsprite_filler, sizeof(qsprite_filler[0])*kMaxSprites);
    Read(&randomseed, sizeof(randomseed));
    Read(&parallaxtype, sizeof(parallaxtype));
    Read(&showinvisibility, sizeof(showinvisibility));
    Read(&pSky->horizfrac, sizeof(pSky->horizfrac));
    Read(&pSky->yoffs, sizeof(pSky->yoffs));
    Read(&pSky->yscale, sizeof(pSky->yscale));
    Read(&gVisibility, sizeof(gVisibility));
    Read(&g_visibility, sizeof(g_visibility));
    Read(&parallaxvisibility, sizeof(parallaxvisibility));
    Read(pSky->tileofs, sizeof(pSky->tileofs));
    Read(&pSky->lognumtiles, sizeof(pSky->lognumtiles));
    Read(headspritesect, sizeof(headspritesect));
    Read(headspritestat, sizeof(headspritestat));
    Read(prevspritesect, sizeof(prevspritesect));
    Read(prevspritestat, sizeof(prevspritestat));
    Read(nextspritesect, sizeof(nextspritesect));
    Read(nextspritestat, sizeof(nextspritestat));
    Read(show2dsector, sizeof(show2dsector));
    Read(show2dwall, sizeof(show2dwall));
    Read(show2dsprite, sizeof(show2dsprite));
    Read(&automapping, sizeof(automapping));
    Read(gotpic, sizeof(gotpic));
    Read(gotsector, sizeof(gotsector));
    Read(&gFrameClock, sizeof(gFrameClock));
    Read(&gFrameTicks, sizeof(gFrameTicks));
    Read(&gFrame, sizeof(gFrame));
    ClockTicks nGameClock;
    Read(&totalclock, sizeof(totalclock));
    totalclock = nGameClock;
    Read(&gLevelTime, sizeof(gLevelTime));
    Read(&gPaused, sizeof(gPaused));
    Read(&gbAdultContent, sizeof(gbAdultContent));
    Read(baseWall, sizeof(baseWall[0])*numwalls);
    Read(baseSprite, sizeof(baseSprite[0])*nNumSprites);
    Read(baseFloor, sizeof(baseFloor[0])*numsectors);
    Read(baseCeil, sizeof(baseCeil[0])*numsectors);
    Read(velFloor, sizeof(velFloor[0])*numsectors);
    Read(velCeil, sizeof(velCeil[0])*numsectors);
    Read(&gHitInfo, sizeof(gHitInfo));
    Read(&byte_1A76C6, sizeof(byte_1A76C6));
    Read(&byte_1A76C8, sizeof(byte_1A76C8));
    Read(&byte_1A76C7, sizeof(byte_1A76C7));
    Read(&byte_19AE44, sizeof(byte_19AE44));
    Read(gStatCount, sizeof(gStatCount));
    Read(nextXSprite, sizeof(nextXSprite));
    Read(nextXWall, sizeof(nextXWall));
    Read(nextXSector, sizeof(nextXSector));
    memset(xsprite, 0, sizeof(xsprite[0])*kMaxXSprites);
    for (int nSprite = 0; nSprite < kMaxSprites; nSprite++)
    {
        int nXSprite = sprite[nSprite].extra;
        if (nXSprite > 0)
            Read(&xsprite[nXSprite], sizeof(XSPRITE));
    }
    memset(xwall, 0, sizeof(xwall));
    for (int nWall = 0; nWall < numwalls; nWall++)
    {
        int nXWall = wall[nWall].extra;
        if (nXWall > 0)
            Read(&xwall[nXWall], sizeof(XWALL));
    }
    memset(xsector, 0, sizeof(xsector));
    for (int nSector = 0; nSector < numsectors; nSector++)
    {
        int nXSector = sector[nSector].extra;
        if (nXSector > 0)
            Read(&xsector[nXSector], sizeof(XSECTOR));
    }
    Read(xvel, nNumSprites*sizeof(xvel[0]));
    Read(yvel, nNumSprites*sizeof(yvel[0]));
    Read(zvel, nNumSprites*sizeof(zvel[0]));
    Read(&gMapRev, sizeof(gMapRev));
    Read(&gSongId, sizeof(gSkyCount));
    Read(&gFogMode, sizeof(gFogMode));
#ifdef NOONE_EXTENSIONS
    Read(&gModernMap, sizeof(gModernMap));
#endif
#ifdef YAX_ENABLE
    Read(&numyaxbunches, sizeof(numyaxbunches));
#endif
    psky_t skyInfo;
    Read(&skyInfo, sizeof(skyInfo));

    *tileSetupSky(0) = skyInfo;
    gCheatMgr.ResetCheats();

}

void MyLoadSave::Save(void)
{
    psky_t *pSky = tileSetupSky(0);
    int nNumSprites = 0;
    int id = 0x5653424e/*'VSBN'*/;
    Write(&id, sizeof(id));
    short version = BYTEVERSION;
    Write(&version, sizeof(version));
    short nSkill = (short)gProfile[myconnectindex].skill;
    Write(&nSkill, sizeof(nSkill));
    for (int nSprite = 0; nSprite < kMaxSprites; nSprite++)
    {
        if (sprite[nSprite].statnum < kMaxStatus && nSprite > nNumSprites)
            nNumSprites = nSprite;
    }
    nNumSprites++;
    Write(&gGameOptions, sizeof(gGameOptions));
    Write(&numsectors, sizeof(numsectors));
    Write(&numwalls, sizeof(numwalls));
    Write(&numsectors, sizeof(numsectors));
    Write(&nNumSprites, sizeof(nNumSprites));
    Write(sector, sizeof(sector[0])*numsectors);
    Write(wall, sizeof(wall[0])*numwalls);
    Write(sprite, sizeof(sprite[0])*kMaxSprites);
    Write(spriteext, sizeof(spriteext[0])*kMaxSprites);
    Write(qsector_filler, sizeof(qsector_filler[0])*numsectors);
    Write(qsprite_filler, sizeof(qsprite_filler[0])*kMaxSprites);
    Write(&randomseed, sizeof(randomseed));
    Write(&parallaxtype, sizeof(parallaxtype));
    Write(&showinvisibility, sizeof(showinvisibility));
    Write(&pSky->horizfrac, sizeof(pSky->horizfrac));
    Write(&pSky->yoffs, sizeof(pSky->yoffs));
    Write(&pSky->yscale, sizeof(pSky->yscale));
    Write(&gVisibility, sizeof(gVisibility));
    Write(&g_visibility, sizeof(g_visibility));
    Write(&parallaxvisibility, sizeof(parallaxvisibility));
    Write(pSky->tileofs, sizeof(pSky->tileofs));
    Write(&pSky->lognumtiles, sizeof(pSky->lognumtiles));
    Write(headspritesect, sizeof(headspritesect));
    Write(headspritestat, sizeof(headspritestat));
    Write(prevspritesect, sizeof(prevspritesect));
    Write(prevspritestat, sizeof(prevspritestat));
    Write(nextspritesect, sizeof(nextspritesect));
    Write(nextspritestat, sizeof(nextspritestat));
    Write(show2dsector, sizeof(show2dsector));
    Write(show2dwall, sizeof(show2dwall));
    Write(show2dsprite, sizeof(show2dsprite));
    Write(&automapping, sizeof(automapping));
    Write(gotpic, sizeof(gotpic));
    Write(gotsector, sizeof(gotsector));
    Write(&gFrameClock, sizeof(gFrameClock));
    Write(&gFrameTicks, sizeof(gFrameTicks));
    Write(&gFrame, sizeof(gFrame));
    ClockTicks nGameClock = totalclock;
    Write(&nGameClock, sizeof(nGameClock));
    Write(&gLevelTime, sizeof(gLevelTime));
    Write(&gPaused, sizeof(gPaused));
    Write(&gbAdultContent, sizeof(gbAdultContent));
    Write(baseWall, sizeof(baseWall[0])*numwalls);
    Write(baseSprite, sizeof(baseSprite[0])*nNumSprites);
    Write(baseFloor, sizeof(baseFloor[0])*numsectors);
    Write(baseCeil, sizeof(baseCeil[0])*numsectors);
    Write(velFloor, sizeof(velFloor[0])*numsectors);
    Write(velCeil, sizeof(velCeil[0])*numsectors);
    Write(&gHitInfo, sizeof(gHitInfo));
    Write(&byte_1A76C6, sizeof(byte_1A76C6));
    Write(&byte_1A76C8, sizeof(byte_1A76C8));
    Write(&byte_1A76C7, sizeof(byte_1A76C7));
    Write(&byte_19AE44, sizeof(byte_19AE44));
    Write(gStatCount, sizeof(gStatCount));
    Write(nextXSprite, sizeof(nextXSprite));
    Write(nextXWall, sizeof(nextXWall));
    Write(nextXSector, sizeof(nextXSector));
    for (int nSprite = 0; nSprite < kMaxSprites; nSprite++)
    {
        int nXSprite = sprite[nSprite].extra;
        if (nXSprite > 0)
            Write(&xsprite[nXSprite], sizeof(XSPRITE));
    }
    for (int nWall = 0; nWall < numwalls; nWall++)
    {
        int nXWall = wall[nWall].extra;
        if (nXWall > 0)
            Write(&xwall[nXWall], sizeof(XWALL));
    }
    for (int nSector = 0; nSector < numsectors; nSector++)
    {
        int nXSector = sector[nSector].extra;
        if (nXSector > 0)
            Write(&xsector[nXSector], sizeof(XSECTOR));
    }
    Write(xvel, nNumSprites*sizeof(xvel[0]));
    Write(yvel, nNumSprites*sizeof(yvel[0]));
    Write(zvel, nNumSprites*sizeof(zvel[0]));
    Write(&gMapRev, sizeof(gMapRev));
    Write(&gSongId, sizeof(gSkyCount));
    Write(&gFogMode, sizeof(gFogMode));
#ifdef NOONE_EXTENSIONS
    Write(&gModernMap, sizeof(gModernMap));
#endif
#ifdef YAX_ENABLE
    Write(&numyaxbunches, sizeof(numyaxbunches));
#endif
    psky_t skyInfo = *tileSetupSky(0);
    Write(&skyInfo, sizeof(skyInfo));
}

void LoadSavedInfo(void)
{
    const int nNameMin = strlen("##.sav"); // length offset for string numbers
    auto pList = klistpath((g_modDir[0] != '/') ? g_modDir : "./", "game00*.sav", BUILDVFS_FIND_FILE);
    for (auto pIterator = pList; pIterator != NULL; pIterator = pIterator->next)
    {
        int hFile = kopen4loadfrommod(pIterator->name, 0);
        if (hFile == -1)
            ThrowError("Error loading save file header.");
        int id = 0;
        short version = 0;
        short nSkill = 0, nSlot = 0;
        if ((uint32_t)kread(hFile, &id, sizeof(id)) != sizeof(id))
        {
            kclose(hFile);
            continue;
        }
        if (id != 0x5653424e/*'VSBN'*/)
        {
            kclose(hFile);
            continue;
        }
        kread(hFile, &version, sizeof(version));
        if (version != BYTEVERSION)
        {
            kclose(hFile);
            continue;
        }
        nSlot = strlen(pIterator->name);
        if (nSlot < nNameMin) // unexpected size, abort
        {
            kclose(hFile);
            continue;
        }
        nSlot = ((pIterator->name[nSlot-nNameMin] - '0') * 10) + (pIterator->name[nSlot-(nNameMin+1)] - '0');
        if (nSlot > kLoadSaveSlot10) // slot id too big, skip
        {
            kclose(hFile);
            continue;
        }
        kread(hFile, &nSkill, sizeof(nSkill));
        if ((uint32_t)kread(hFile, &gSaveGameOptions[nSlot], sizeof(gSaveGameOptions[0])) != sizeof(gSaveGameOptions[0]))
            ThrowError("Error reading save file.");
        LoadUpdateSaveGame(nSlot, nSkill);
        kclose(hFile);
    }
    klistfree(pList);
}

void LoadAutosavedInfo(void)
{
    const int nNameMin = strlen("#.sav"); // length offset for string numbers
    auto pList = klistpath((g_modDir[0] != '/') ? g_modDir : "./", "gameautosave*.sav", BUILDVFS_FIND_FILE);
    for (auto pIterator = pList; pIterator != NULL; pIterator = pIterator->next)
    {
        int hFile = kopen4loadfrommod(pIterator->name, 0);
        if (hFile == -1)
            ThrowError("Error loading save file header.");
        int id = 0;
        short version = 0;
        short nSkill = 0, nSlot = 0;
        if ((uint32_t)kread(hFile, &id, sizeof(id)) != sizeof(id))
        {
            kclose(hFile);
            continue;
        }
        if (id != 0x5653424e/*'VSBN'*/)
        {
            kclose(hFile);
            continue;
        }
        kread(hFile, &version, sizeof(version));
        if (version != BYTEVERSION)
        {
            kclose(hFile);
            continue;
        }
        nSlot = strlen(pIterator->name);
        if (nSlot < nNameMin) // unexpected size, abort
        {
            kclose(hFile);
            continue;
        }
        nSlot = kLoadSaveSlotAutosave + (pIterator->name[nSlot-nNameMin] - '0');
        if (nSlot > kLoadSaveSlotKey) // slot id too big, skip
        {
            kclose(hFile);
            continue;
        }
        kread(hFile, &nSkill, sizeof(nSkill));
        if ((uint32_t)kread(hFile, &gSaveGameOptions[nSlot], sizeof(gSaveGameOptions[0])) != sizeof(gSaveGameOptions[0]))
            ThrowError("Error reading save file.");
        LoadUpdateSaveGame(nSlot, nSkill);
        kclose(hFile);
    }
    klistfree(pList);
}

bool LoadSavedInCurrentSession(int nSlot)
{
    if ((nSlot < kLoadSaveSlot0) || (nSlot >= kMaxLoadSaveSlot))
        return false;
    if (gSaveGameOptions[nSlot].nEpisode != gGameOptions.nEpisode)
        return false;
    if (gSaveGameOptions[nSlot].nLevel != gGameOptions.nLevel)
        return false;
    if (gSaveGameOptions[nSlot].nDifficulty != gGameOptions.nDifficulty)
        return false;
    if (gSaveGameOptions[nSlot].nEnemyHealth != gGameOptions.nEnemyHealth)
        return false;
    if (gSaveGameOptions[nSlot].nEnemyQuantity != gGameOptions.nEnemyQuantity)
        return false;
    if (gSaveGameProfileSkill[nSlot] != gProfile[myconnectindex].skill)
        return false;
    if (gSaveGameOptions[nSlot].nEnemySpeed != gGameOptions.nEnemySpeed)
        return false;
    if (gSaveGameOptions[nSlot].bEnemyShuffle != gGameOptions.bEnemyShuffle)
        return false;
    if (gSaveGameOptions[nSlot].bPitchforkOnly != gGameOptions.bPitchforkOnly)
        return false;
    if (gSaveGameOptions[nSlot].uSpriteBannedFlags != gGameOptions.uSpriteBannedFlags)
        return false;
    if (gSaveGameOptions[nSlot].nRandomizerCheat != gGameOptions.nRandomizerCheat)
        return false;
    if (Bstrncmp(gSaveGameOptions[nSlot].zLevelName, gGameOptions.zLevelName, sizeof(gSaveGameOptions[nSlot].zLevelName)))
        return false;
    return true;
}

void LoadUpdateSaveGame(int nSlot, int nSkill)
{
    gSaveGameProfileSkill[nSlot] = nSkill;
    Bstrncpyz(strRestoreGameStrings[gSaveGameOptions[nSlot].nSaveGameSlot], gSaveGameOptions[nSlot].szUserGameName, sizeof(strRestoreGameStrings[gSaveGameOptions[nSlot].nSaveGameSlot]));

    char nDifficulty = gSaveGameOptions[nSlot].nDifficulty;
    if (gSaveGameOptions[nSlot].nDifficulty != gSaveGameOptions[nSlot].nEnemyHealth)
        nDifficulty = 5;
    else if (gSaveGameOptions[nSlot].nDifficulty != gSaveGameOptions[nSlot].nEnemyQuantity)
        nDifficulty = 5;
    else if (gSaveGameOptions[nSlot].nDifficulty != (char)gSaveGameProfileSkill[nSlot])
        nDifficulty = 5;
    else if (gSaveGameOptions[nSlot].nEnemySpeed)
        nDifficulty = 5;
    else if (gSaveGameOptions[nSlot].bEnemyShuffle)
        nDifficulty = 5;
    else if (gSaveGameOptions[nSlot].bPitchforkOnly)
        nDifficulty = 5;
    else if (gSaveGameOptions[nSlot].uSpriteBannedFlags)
        nDifficulty = 5;
    restoreGameDifficulty[gSaveGameOptions[nSlot].nSaveGameSlot] = nDifficulty;
}

static MyLoadSave *myLoadSave;


void LoadSaveSetup(void)
{
    void ActorLoadSaveConstruct(void);
    void AILoadSaveConstruct(void);
    void EndGameLoadSaveConstruct(void);
    void EventQLoadSaveConstruct(void);
    void LevelsLoadSaveConstruct(void);
    void MessagesLoadSaveConstruct(void);
    void MirrorLoadSaveConstruct(void);
    void PlayerLoadSaveConstruct(void);
    void SeqLoadSaveConstruct(void);
    void TriggersLoadSaveConstruct(void);
    void ViewLoadSaveConstruct(void);
    void WarpLoadSaveConstruct(void);
    void WeaponLoadSaveConstruct(void);
#ifdef NOONE_EXTENSIONS
    void nnExtLoadSaveConstruct(void);
#endif
    myLoadSave = new MyLoadSave();

    ActorLoadSaveConstruct();
    AILoadSaveConstruct();
    EndGameLoadSaveConstruct();
    EventQLoadSaveConstruct();
    LevelsLoadSaveConstruct();
    MessagesLoadSaveConstruct();
    MirrorLoadSaveConstruct();
    PlayerLoadSaveConstruct();
    SeqLoadSaveConstruct();
    TriggersLoadSaveConstruct();
    ViewLoadSaveConstruct();
    WarpLoadSaveConstruct();
    WeaponLoadSaveConstruct();
#ifdef NOONE_EXTENSIONS
    nnExtLoadSaveConstruct();
#endif
}