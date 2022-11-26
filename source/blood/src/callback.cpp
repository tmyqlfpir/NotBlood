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

#include "build.h"
#include "common_game.h"

#include "actor.h"
#include "ai.h"
#include "blood.h"
#include "callback.h"
#include "config.h"
#include "db.h"
#include "dude.h"
#include "endgame.h"
#include "eventq.h"
#include "fx.h"
#include "gameutil.h"
#include "globals.h"
#include "levels.h"
#include "player.h"
#include "replace.h"
#include "seq.h"
#include "sfx.h"
#include "sound.h"
#include "trig.h"
#include "triggers.h"
#include "view.h"
#ifdef NOONE_EXTENSIONS
#include "nnexts.h"
#include "aiunicult.h"
#endif

void fxFlameLick(int nSprite) // 0
{
    spritetype *pSprite = &sprite[nSprite];
    int nXSprite = pSprite->extra;
    XSPRITE *pXSprite = &xsprite[nXSprite];
    int top, bottom;
    GetSpriteExtents(pSprite, &top, &bottom);
    for (int i = 0; i < 3; i++)
    {
        int nDist = (pSprite->xrepeat*(tilesiz[pSprite->picnum].x/2))>>3;
        int nAngle = Random(2048);
        int dx = mulscale30(nDist, Cos(nAngle));
        int dy = mulscale30(nDist, Sin(nAngle));
        int x = pSprite->x + dx;
        int y = pSprite->y + dy;
        int z = bottom-Random(bottom-top);
        spritetype *pFX = gFX.fxSpawn(FX_32, pSprite->sectnum, x, y, z);
        if (pFX)
        {
            xvel[pFX->index] = xvel[nSprite] + Random2(-dx);
            yvel[pFX->index] = yvel[nSprite] + Random2(-dy);
            zvel[pFX->index] = zvel[nSprite] - Random(0x1aaaa);
        }
    }
    if (pXSprite->burnTime > 0)
        evPost(nSprite, 3, 5, kCallbackFXFlameLick);
}

void Remove(int nSprite) // 1
{
    spritetype *pSprite = &sprite[nSprite];
    evKill(nSprite, 3);
    if (pSprite->extra > 0)
        seqKill(3, pSprite->extra);
    sfxKill3DSound(pSprite, 0, -1);
    DeleteSprite(nSprite);
}

void FlareBurst(int nSprite) // 2
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    spritetype *pSprite = &sprite[nSprite];
    int nAngle = getangle(xvel[nSprite], yvel[nSprite]);
    int nRadius = 0x55555;
    const int nFlares = !VanillaMode() ? (gFlareBurstCount + 1) * 4 : 8;
    for (int i = 0; i < nFlares; i++)
    {
        spritetype *pSpawn = actSpawnSprite(pSprite, 5);
        pSpawn->picnum = 2424;
        pSpawn->shade = -128;
        pSpawn->xrepeat = pSpawn->yrepeat = 32;
        pSpawn->type = kMissileFlareAlt;
        pSpawn->clipdist = 2;
        pSpawn->owner = pSprite->owner;
        int nAngle2 = (i<<11)/nFlares;
        int dx = 0;
        int dy = mulscale30r(nRadius, Sin(nAngle2));
        int dz = mulscale30r(nRadius, -Cos(nAngle2));
        if (i&1)
        {
            dy >>= 1;
            dz >>= 1;
        }
        RotateVector(&dx, &dy, nAngle);
        xvel[pSpawn->index] += dx;
        yvel[pSpawn->index] += dy;
        zvel[pSpawn->index] += dz;
        evPost(pSpawn->index, 3, 960, kCallbackRemove);
    }
    evPost(nSprite, 3, 0, kCallbackRemove);
}

void fxFlareSpark(int nSprite) // 3
{
    spritetype *pSprite = &sprite[nSprite];
    spritetype *pFX = gFX.fxSpawn(FX_28, pSprite->sectnum, pSprite->x, pSprite->y, pSprite->z);
    if (pFX)
    {
        xvel[pFX->index] = xvel[nSprite] + Random2(0x1aaaa);
        yvel[pFX->index] = yvel[nSprite] + Random2(0x1aaaa);
        zvel[pFX->index] = zvel[nSprite] - Random(0x1aaaa);
    }
    evPost(nSprite, 3, 4, kCallbackFXFlareSpark);
}

void fxFlareSparkLite(int nSprite) // 4
{
    spritetype *pSprite = &sprite[nSprite];
    spritetype *pFX = gFX.fxSpawn(FX_28, pSprite->sectnum, pSprite->x, pSprite->y, pSprite->z);
    if (pFX)
    {
        xvel[pFX->index] = xvel[nSprite] + Random2(0x1aaaa);
        yvel[pFX->index] = yvel[nSprite] + Random2(0x1aaaa);
        zvel[pFX->index] = zvel[nSprite] - Random(0x1aaaa);
    }
    evPost(nSprite, 3, 12, kCallbackFXFlareSparkLite);
}

void fxZombieBloodSpurt(int nSprite) // 5
{
    dassert(nSprite >= 0 && nSprite < kMaxSprites);
    spritetype *pSprite = &sprite[nSprite];
    int nXSprite = pSprite->extra;
    dassert(nXSprite > 0 && nXSprite < kMaxXSprites);
    XSPRITE *pXSprite = &xsprite[nXSprite];
    int top, bottom;
    GetSpriteExtents(pSprite, &top, &bottom);
    spritetype *pFX = gFX.fxSpawn(FX_27, pSprite->sectnum, pSprite->x, pSprite->y, top);
    if (pFX)
    {
        xvel[pFX->index] = xvel[nSprite] + Random2(0x11111);
        yvel[pFX->index] = yvel[nSprite] + Random2(0x11111);
        zvel[pFX->index] = zvel[nSprite] - 0x6aaaa;
    }
    if (pXSprite->data1 > 0)
    {
        evPost(nSprite, 3, 4, kCallbackFXZombieSpurt);
        pXSprite->data1 -= 4;
    }
    else if (pXSprite->data2 > 0)
    {
        evPost(nSprite, 3, 60, kCallbackFXZombieSpurt);
        pXSprite->data1 = 40;
        pXSprite->data2--;
    }
}

void fxBloodSpurt(int nSprite) // 6
{
    spritetype *pSprite = &sprite[nSprite];
    spritetype *pFX = gFX.fxSpawn(FX_27, pSprite->sectnum, pSprite->x, pSprite->y, pSprite->z);
    if (pFX)
    {
        pFX->ang = 0;
        xvel[pFX->index] = xvel[nSprite]>>8;
        yvel[pFX->index] = yvel[nSprite]>>8;
        zvel[pFX->index] = zvel[nSprite]>>8;
    }
    evPost(nSprite, 3, 6, kCallbackFXBloodSpurt);
}


void fxArcSpark(int nSprite) // 7
{
    spritetype* pSprite = &sprite[nSprite];
    spritetype* pFX = gFX.fxSpawn(FX_15, pSprite->sectnum, pSprite->x, pSprite->y, pSprite->z);
    if (pFX)
    {
        xvel[pFX->index] = xvel[nSprite] + Random2(0x10000);
        yvel[pFX->index] = yvel[nSprite] + Random2(0x10000);
        zvel[pFX->index] = zvel[nSprite] - Random(0x1aaaa);
    }
    evPost(nSprite, 3, 3, kCallbackFXArcSpark);
}


void fxDynPuff(int nSprite) // 8
{
    int nCallbackTicks = 12;
    spritetype *pSprite = &sprite[nSprite];
    if (zvel[nSprite])
    {
        const bool bSmokeTrail3D = gSmokeTrail3D && !VanillaMode() && (gGameOptions.nGameType == kGameTypeSinglePlayer) && actSpriteOwnerIsDude(pSprite) && ((pSprite->type == kThingArmedTNTStick) || (pSprite->type == kThingArmedTNTBundle) || (pSprite->type == kThingArmedSpray));
        if (bSmokeTrail3D) // feature is single-player only (causes desync)
        {
            const int nTile = 3436;
            const int frames = picanm[nTile].num;
            const int frameoffset = qanimateoffs(nTile, 32768+nSprite);
            const int angleOffset = frameoffset * (kAngMask / (frames+1));
            int angle = (pSprite->ang+((-angleOffset)-kAng90))&kAngMask;
            if (actSpriteOwnerIsPlayer(pSprite)) // chances are if an enemy is throwing tnt at the player, the angle will be inverted - so rotate by 180 degrees
                angle = (angle+kAng180)&kAngMask;
            int nDist = (pSprite->xrepeat*(tilesiz[pSprite->picnum+frameoffset].x/2))>>3;
            nDist += nDist>>2;
            int x = pSprite->x + mulscale30(nDist, Cos(angle));
            int y = pSprite->y + mulscale30(nDist, Sin(angle));
            int z = pSprite->z + mulscale30(nDist<<3, Cos(angle));
            spritetype *pFX = gFX.fxSpawn(FX_7, pSprite->sectnum, x, y, z);
            if (pFX)
            {
                xvel[pFX->index] = xvel[nSprite] + mulscale30(nDist<<10, Cos(angle));
                yvel[pFX->index] = yvel[nSprite] + mulscale30(nDist<<10, Sin(angle));
                zvel[pFX->index] = zvel[nSprite];
                nCallbackTicks = 10;
            }
        }
        else // original
        {
            int nDist = (pSprite->xrepeat*(tilesiz[pSprite->picnum].x/2))>>2;
            int x = pSprite->x + mulscale30(nDist, Cos(pSprite->ang-kAng90));
            int y = pSprite->y + mulscale30(nDist, Sin(pSprite->ang-kAng90));
            int z = pSprite->z;
            spritetype *pFX = gFX.fxSpawn(FX_7, pSprite->sectnum, x, y, z);
            if (pFX)
            {
                xvel[pFX->index] = xvel[nSprite];
                yvel[pFX->index] = yvel[nSprite];
                zvel[pFX->index] = zvel[nSprite];
            }
        }
    }
    evPost(nSprite, 3, nCallbackTicks, kCallbackFXDynPuff);
}

void Respawn(int nSprite) // 9
{
    spritetype *pSprite = &sprite[nSprite];
    dassert(pSprite->extra > 0 && pSprite->extra < kMaxXSprites);
    XSPRITE *pXSprite = &xsprite[pSprite->extra];
    
    if (pSprite->statnum != kStatRespawn && pSprite->statnum != kStatThing) {
        OSD_Printf("Sprite #%d is not on Respawn or Thing list\n", nSprite);
        return;
    } else if (!(pSprite->flags & kHitagRespawn)) {
        OSD_Printf("Sprite #%d does not have the respawn attribute\n", nSprite);
        return;
    }

    switch (pXSprite->respawnPending) {
        case 1: {
            int nTime = mulscale16(actGetRespawnTime(pSprite), 0x4000);
            pXSprite->respawnPending = 2;
            evPost(nSprite, 3, nTime, kCallbackRespawn);
            break;
        }
        case 2: {
            int nTime = mulscale16(actGetRespawnTime(pSprite), 0x2000);
            pXSprite->respawnPending = 3;
            evPost(nSprite, 3, nTime, kCallbackRespawn);
            break;
        }
        case 3: {
            dassert(pSprite->owner != kStatRespawn);
            dassert(pSprite->owner >= 0 && pSprite->owner < kMaxStatus);
            ChangeSpriteStat(nSprite, pSprite->owner);
            pSprite->type = pSprite->inittype;
            pSprite->owner = -1;
            pSprite->flags &= ~kHitagRespawn;
            xvel[nSprite] = yvel[nSprite] = zvel[nSprite] = 0;
            pXSprite->respawnPending = 0;
            pXSprite->burnTime = 0;
            pXSprite->isTriggered = 0;
            if (IsDudeSprite(pSprite)) {
                if (!VanillaMode()) // remove a kill
                    gKillMgr.RemoveKill(pSprite);
                int nType = pSprite->type-kDudeBase;
                pSprite->x = baseSprite[nSprite].x;
                pSprite->y = baseSprite[nSprite].y;
                pSprite->z = baseSprite[nSprite].z;
                pSprite->cstat |= 0x1101;
                #ifdef NOONE_EXTENSIONS
                if (!gModernMap || pXSprite->sysData2 <= 0) pXSprite->health = dudeInfo[pSprite->type - kDudeBase].startHealth << 4;
                else pXSprite->health = ClipRange(pXSprite->sysData2 << 4, 1, 65535);

                switch (pSprite->type) {
                    default:
                        pSprite->clipdist = getDudeInfo(nType + kDudeBase)->clipdist;
                        if (gSysRes.Lookup(getDudeInfo(nType + kDudeBase)->seqStartID, "SEQ"))
                            seqSpawn(getDudeInfo(nType + kDudeBase)->seqStartID, 3, pSprite->extra, -1);
                        break;
                    case kDudeModernCustom:
                        seqSpawn(genDudeSeqStartId(pXSprite), 3, pSprite->extra, -1);
                        break;
                }
                
                // return dude to the patrol state
                if (gModernMap && pXSprite->dudeFlag4) {
                    pXSprite->data3 = 0;
                    pXSprite->target = -1;
                }
                #else
                pSprite->clipdist = getDudeInfo(nType + kDudeBase)->clipdist;
                pXSprite->health = getDudeInfo(nType + kDudeBase)->startHealth << 4;
                if (gSysRes.Lookup(getDudeInfo(nType + kDudeBase)->seqStartID, "SEQ"))
                    seqSpawn(getDudeInfo(nType + kDudeBase)->seqStartID, 3, pSprite->extra, -1);
                #endif
                aiInitSprite(pSprite);
                pXSprite->key = 0;
            } else if (pSprite->type == kThingTNTBarrel) {
                pSprite->cstat |= CSTAT_SPRITE_BLOCK | CSTAT_SPRITE_BLOCK_HITSCAN;
                pSprite->cstat &= (unsigned short)~CSTAT_SPRITE_INVISIBLE;
            }

            gFX.fxSpawn(FX_29, pSprite->sectnum, pSprite->x, pSprite->y, pSprite->z);
            sfxPlay3DSound(pSprite, 350, -1, 0);
            break;
        }
    }
}

void PlayerBubble(int nSprite) // 10
{
    spritetype *pSprite = &sprite[nSprite];
    if (IsPlayerSprite(pSprite))
    {
        PLAYER *pPlayer = &gPlayer[pSprite->type-kDudePlayer1];
        dassert(pPlayer != NULL);
        if (!pPlayer->bubbleTime)
            return;
        int top, bottom;
        GetSpriteExtents(pSprite, &top, &bottom);
        for (int i = 0; i < (pPlayer->bubbleTime>>6); i++)
        {
            int nDist = (pSprite->xrepeat*(tilesiz[pSprite->picnum].x/2))>>2;
            int nAngle = Random(2048);
            int x = pSprite->x + mulscale30(nDist, Cos(nAngle));
            int y = pSprite->y + mulscale30(nDist, Sin(nAngle));
            int z = bottom-Random(bottom-top);
            spritetype *pFX = gFX.fxSpawn((FX_ID)(FX_23+Random(3)), pSprite->sectnum, x, y, z);
            if (pFX)
            {
                xvel[pFX->index] = xvel[nSprite] + Random2(0x1aaaa);
                yvel[pFX->index] = yvel[nSprite] + Random2(0x1aaaa);
                zvel[pFX->index] = zvel[nSprite] + Random2(0x1aaaa);
            }
        }
        evPost(nSprite, 3, 4, kCallbackPlayerBubble);
    }
}

void EnemyBubble(int nSprite) // 11
{
    spritetype *pSprite = &sprite[nSprite];
    int top, bottom;
    GetSpriteExtents(pSprite, &top, &bottom);
    for (int i = 0; i < (klabs(zvel[nSprite])>>18); i++)
    {
        int nDist = (pSprite->xrepeat*(tilesiz[pSprite->picnum].x/2))>>2;
        int nAngle = Random(2048);
        int x = pSprite->x + mulscale30(nDist, Cos(nAngle));
        int y = pSprite->y + mulscale30(nDist, Sin(nAngle));
        int z = bottom-Random(bottom-top);
        spritetype *pFX = gFX.fxSpawn((FX_ID)(FX_23+Random(3)), pSprite->sectnum, x, y, z);
        if (pFX)
        {
            xvel[pFX->index] = xvel[nSprite] + Random2(0x1aaaa);
            yvel[pFX->index] = yvel[nSprite] + Random2(0x1aaaa);
            zvel[pFX->index] = zvel[nSprite] + Random2(0x1aaaa);
        }
    }
    evPost(nSprite, 3, 4, kCallbackEnemeyBubble);
}

void CounterCheck(int nSector) // 12
{
    dassert(nSector >= 0 && nSector < kMaxSectors);
    if (sector[nSector].type != kSectorCounter) return;
    if (sector[nSector].extra <= 0) return;
    
    XSECTOR *pXSector = &xsector[sector[nSector].extra];
    int nReq = pXSector->waitTimeA; int nType = pXSector->data; int nCount = 0;
    if (!nType || !nReq) return;
    
    for (int nSprite = headspritesect[nSector]; nSprite >= 0; nSprite = nextspritesect[nSprite]) {
        if (sprite[nSprite].type == nType) nCount++;
    }
        
    if (nCount < nReq) {
        evPost(nSector, 6, 5, kCallbackCounterCheck);
        return;
    } else {
        //pXSector->waitTimeA = 0; //do not reset necessary objects counter to zero
        trTriggerSector(nSector, pXSector, kCmdOn, kCauserGame);
        pXSector->locked = 1; //lock sector, so it can be opened again later
    }
}


void FinishHim(int nSprite) // 13
{
    spritetype* pSprite = &sprite[nSprite];
    int nXSprite = pSprite->extra;
    XSPRITE* pXSprite = &xsprite[nXSprite];
    if (IsPlayerSprite(pSprite) && playerSeqPlaying(&gPlayer[pSprite->type - kDudePlayer1], 16) && pXSprite->target == gMe->nSprite)
        sndStartSample(3313, -1, 1, 0);
}

void fxBloodBits(int nSprite) // 14
{
    spritetype *pSprite = &sprite[nSprite];
    int ceilZ, ceilHit, floorZ, floorHit;
    GetZRange(pSprite, &ceilZ, &ceilHit, &floorZ, &floorHit, pSprite->clipdist, CLIPMASK0);
    int top, bottom;
    GetSpriteExtents(pSprite, &top, &bottom);
    pSprite->z += floorZ-bottom;
    int nAngle = Random(2048);
    int nDist = Random(16)<<4;
    int x = pSprite->x+mulscale28(nDist, Cos(nAngle));
    int y = pSprite->y+mulscale28(nDist, Sin(nAngle));
    int nSector = pSprite->sectnum;
    if (gGameOptions.bSectorBehavior && !VanillaMode()) // check sector when creating splatter in random directions
    {
        if (!FindSector(x, y, pSprite->z, &nSector)) // could not find valid sector, delete fx
        {
            gFX.fxFree(nSprite);
            return;
        }
        if ((sector[nSector].floorpicnum >= 4080) && (sector[nSector].floorpicnum <= 4095)) // if sector is open air, delete fx
        {
            gFX.fxFree(nSprite);
            return;
        }
        GetZRangeAtXYZ(x, y, pSprite->z, nSector, &ceilZ, &ceilHit, &floorZ, &floorHit, pSprite->clipdist, CLIPMASK0); // get new floor position of changed sector
    }
    gFX.fxSpawn(FX_48, nSector, x, y, pSprite->z);
    if (pSprite->ang == 1024)
    {
        int nChannel = 28+(pSprite->index&2);
        dassert(nChannel < 32);
        sfxPlay3DSound(pSprite, 385, nChannel, 1);
    }
    if (Chance(0x5000))
    {
        if (!gGameOptions.bSectorBehavior || VanillaMode()) // restore sector (resolves optimization bug)
            nSector = pSprite->sectnum;
        spritetype *pFX = gFX.fxSpawn(FX_36, nSector, x, y, floorZ-64);
        if (pFX)
        {
            pFX->ang = nAngle;
            if (gGameOptions.bSectorBehavior && !VanillaMode())
            {
                floorZ = getflorzofslope(pFX->sectnum, pFX->x, pFX->y); // get more accurate z position of sector
                if (klabs(pFX->z-floorZ) <= (32<<8)) // if sprite is less than one meter from floor, set to sector height and test for slope
                {
                    pFX->z = floorZ;
                    if ((sector[pFX->sectnum].floorstat&2) && (sector[pFX->sectnum].floorheinum != 0)) // align sprite to slope
                    {
                        walltype *pWall1 = &wall[sector[pFX->sectnum].wallptr];
                        walltype *pWall2 = &wall[pWall1->point2];
                        spriteSetSlope(pFX->index, sector[pFX->sectnum].floorheinum);
                        pFX->ang = getangle(pWall2->x-pWall1->x, pWall2->y-pWall1->y)+kAng270;
                    }
                }
            }
        }
    }
    gFX.fxFree(nSprite);
}


void fxTeslaAlt(int nSprite) // 15
{
    spritetype* pSprite = &sprite[nSprite];
    spritetype* pFX = gFX.fxSpawn(FX_49, pSprite->sectnum, pSprite->x, pSprite->y, pSprite->z);
    if (pFX)
    {
        xvel[pFX->index] = xvel[nSprite] + Random2(0x1aaaa);
        yvel[pFX->index] = yvel[nSprite] + Random2(0x1aaaa);
        zvel[pFX->index] = zvel[nSprite] - Random(0x1aaaa);
    }
    evPost(nSprite, 3, 3, kCallbackFXTeslaAlt);
}


int tommySleeveSnd[] = { 608, 609, 611 }; // unused?
int sawedOffSleeveSnd[] = { 610, 612 };

void fxBouncingSleeve(int nSprite) // 16
{
    spritetype* pSprite = &sprite[nSprite]; int ceilZ, ceilHit, floorZ, floorHit;
    GetZRange(pSprite, &ceilZ, &ceilHit, &floorZ, &floorHit, pSprite->clipdist, CLIPMASK0);
    int top, bottom; GetSpriteExtents(pSprite, &top, &bottom);
    pSprite->z += floorZ - bottom;
    
    int zv = zvel[nSprite] - velFloor[pSprite->sectnum];
    
    if (zvel[nSprite] == 0) sleeveStopBouncing(pSprite);
    else if (zv > 0) {
        actFloorBounceVector((int*)& xvel[nSprite], (int*)& yvel[nSprite], &zv, pSprite->sectnum, 0x9000);
        zvel[nSprite] = zv;
        if (velFloor[pSprite->sectnum] == 0 && klabs(zvel[nSprite]) < 0x20000)  {
            sleeveStopBouncing(pSprite);
            return;
        }

        int nChannel = 28 + (pSprite->index & 2);
        dassert(nChannel < 32);
        
        // tommy sleeve
        if (pSprite->type >= FX_37 && pSprite->type <= FX_39) {
            Random(3); 
            sfxPlay3DSound(pSprite, 608 + Random(2), nChannel, 1);
        
        // sawed-off sleeve
        } else {
            sfxPlay3DSound(pSprite, sawedOffSleeveSnd[Random(2)], nChannel, 1);
        }
    }   

}


void sleeveStopBouncing(spritetype* pSprite) {
    xvel[pSprite->index] = yvel[pSprite->index] = zvel[pSprite->index] = 0;
    if (pSprite->extra > 0) seqKill(3, pSprite->extra);
    sfxKill3DSound(pSprite, -1, -1);

    switch (pSprite->type) {
    case FX_37:
    case FX_38:
    case FX_39:
        pSprite->picnum = 2465;
        break;
    case FX_40:
    case FX_41:
    case FX_42:
        pSprite->picnum = 2464;
        break;
    }

    pSprite->type = FX_51; // static spent casing
    pSprite->xrepeat = pSprite->yrepeat = 10;
    if (gGameOptions.bSectorBehavior && !VanillaMode()) // offset into ground so casings can be dragged across sectors
    {
        pSprite->z = 0;
        int top, bottom;
        GetSpriteExtents(pSprite, &top, &bottom);
        pSprite->z = getflorzofslope(pSprite->sectnum, pSprite->x, pSprite->y) - bottom;
    }
}


void returnFlagToBase(int nSprite) // 17
{
    spritetype* pSprite = &sprite[nSprite];
    if (pSprite->owner >= 0 && pSprite->owner < kMaxSprites)
    {
        spritetype* pOwner = &sprite[pSprite->owner];
        XSPRITE* pXOwner = &xsprite[pOwner->extra];
        switch (pSprite->type) {
            case kItemFlagA:
                trTriggerSprite(pOwner->index, pXOwner, kCmdOn, pOwner->index);
                sndStartSample(8003, 255, 2, 0);
                gBlueFlagDropped = false;
                viewSetMessage("Blue Flag returned to base.");
                break;
            case kItemFlagB:
                trTriggerSprite(pOwner->index, pXOwner, kCmdOn, pOwner->index);
                sndStartSample(8002, 255, 2, 0);
                gRedFlagDropped = false;
                viewSetMessage("Red Flag returned to base.");
                break;
        }
    }
    evPost(pSprite->index, 3, 0, kCallbackRemove);
}

void fxPodBloodSpray(int nSprite) // 18
{
    spritetype* pSprite = &sprite[nSprite];
    spritetype* pFX;
    if (pSprite->type == 53)
        pFX = gFX.fxSpawn(FX_53, pSprite->sectnum, pSprite->x, pSprite->y, pSprite->z);
    else
        pFX = gFX.fxSpawn(FX_54, pSprite->sectnum, pSprite->x, pSprite->y, pSprite->z);
    if (pFX)
    {
        pFX->ang = 0;
        xvel[pFX->index] = xvel[nSprite] >> 8;
        yvel[pFX->index] = yvel[nSprite] >> 8;
        zvel[pFX->index] = zvel[nSprite] >> 8;
    }
    evPost(nSprite, 3, 6, kCallbackFXPodBloodSpray);
}

void fxPodBloodSplat(int nSprite) // 19
{
    spritetype *pSprite = &sprite[nSprite];
    int ceilZ, ceilHit, floorZ, floorHit;
    GetZRange(pSprite, &ceilZ, &ceilHit, &floorZ, &floorHit, pSprite->clipdist, CLIPMASK0);
    int top, bottom;
    GetSpriteExtents(pSprite, &top, &bottom);
    pSprite->z += floorZ-bottom;
    int nAngle = Random(2048);
    int nDist = Random(16)<<4;
    int x = pSprite->x+mulscale28(nDist, Cos(nAngle));
    int y = pSprite->y+mulscale28(nDist, Sin(nAngle));
    if (pSprite->ang == 1024 && pSprite->type == 53)
    {
        int nChannel = 28+(pSprite->index&2);
        dassert(nChannel < 32);
        sfxPlay3DSound(pSprite, 385, nChannel, 1);
    }
    spritetype *pFX = NULL;
    if (pSprite->type == 53 || pSprite->type == kThingPodGreenBall)
    {
        if (Chance(0x500) || pSprite->type == kThingPodGreenBall)
            pFX = gFX.fxSpawn(FX_55, pSprite->sectnum, x, y, floorZ-64);
        if (pFX)
            pFX->ang = nAngle;
    }
    else
    {
        pFX = gFX.fxSpawn(FX_32, pSprite->sectnum, x, y, floorZ-64);
        if (pFX)
            pFX->ang = nAngle;
    }
    gFX.fxFree(nSprite);
}



void LeechStateTimer(int nSprite) // 20
{
    spritetype *pSprite = &sprite[nSprite];
    if (pSprite->statnum == kStatThing && !(pSprite->flags & 32)) {
        switch (pSprite->type) {
            case kThingDroppedLifeLeech:
            #ifdef NOONE_EXTENSIONS
            case kModernThingEnemyLifeLeech:
            #endif
                xsprite[pSprite->extra].stateTimer = 0;
                break;
        }
    }
}

void sub_76A08(spritetype *pSprite, spritetype *pSprite2, PLAYER *pPlayer) // ???
{
    int top, bottom;
    int nSprite = pSprite->index;
    GetSpriteExtents(pSprite, &top, &bottom);
    pSprite->x = pSprite2->x;
    pSprite->y = pSprite2->y;
    pSprite->z = sector[pSprite2->sectnum].floorz-(bottom-pSprite->z);
    pSprite->ang = pSprite2->ang;
    ChangeSpriteSect(nSprite, pSprite2->sectnum);
    sfxPlay3DSound(pSprite2, 201, -1, 0);
    xvel[nSprite] = yvel[nSprite] = zvel[nSprite] = 0;
    viewBackupSpriteLoc(nSprite, pSprite);
    if (pPlayer)
    {
        playerResetInertia(pPlayer);
        pPlayer->zViewVel = pPlayer->zWeaponVel = 0;
    }
}

void DropVoodoo(int nSprite) // unused
{
    spritetype *pSprite = &sprite[nSprite];
    int nOwner = actSpriteOwnerToSpriteId(pSprite);
    if (nOwner < 0 || nOwner >= kMaxSprites)
    {
        evPost(nSprite, 3, 0, kCallbackRemove);
        return;
    }
    spritetype *pOwner = &sprite[nOwner];
    PLAYER *pPlayer;
    if (IsPlayerSprite(pOwner))
        pPlayer = &gPlayer[pOwner->type-kDudePlayer1];
    else
        pPlayer = NULL;
    if (!pPlayer)
    {
        evPost(nSprite, 3, 0, kCallbackRemove);
        return;
    }
    pSprite->ang = getangle(pOwner->x-pSprite->x, pOwner->y-pSprite->y);
    int nXSprite = pSprite->extra;
    if (nXSprite > 0)
    {
        XSPRITE *pXSprite = &xsprite[nXSprite];
        if (pXSprite->data1 == 0)
        {
            evPost(nSprite, 3, 0, kCallbackRemove);
            return;
        }
        int nSprite2, nNextSprite;
        for (nSprite2 = headspritestat[kStatDude]; nSprite2 >= 0; nSprite2 = nNextSprite)
        {
            nNextSprite = nextspritestat[nSprite2];
            if (nOwner == nSprite2)
                continue;
            spritetype *pSprite2 = &sprite[nSprite2];
            int nXSprite2 = pSprite2->extra;
            if (nXSprite2 > 0 && nXSprite2 < kMaxXSprites)
            {
                XSPRITE *pXSprite2 = &xsprite[nXSprite2];
                PLAYER *pPlayer2;
                if (IsPlayerSprite(pSprite2))
                    pPlayer2 = &gPlayer[pSprite2->type-kDudePlayer1];
                else
                    pPlayer2 = NULL;
                if (pXSprite2->health > 0 && (pPlayer2 || pXSprite2->key == 0))
                {
                    if (pPlayer2)
                    {
                        if (gGameOptions.nGameType == kGameTypeCoop)
                            continue;
                        if (gGameOptions.nGameType == kGameTypeTeams && pPlayer->teamId == pPlayer2->teamId)
                            continue;
                        int t = 0x8000/ClipLow(gNetPlayers-1, 1);
                        if (!powerupCheck(pPlayer2, kPwUpDeathMask))
                            t += ((3200-pPlayer2->armor[2])<<15)/3200;
                        if (Chance(t) || nNextSprite < 0)
                        {
                            int nDmg = actDamageSprite(nOwner, pSprite2, kDamageSpirit, pXSprite->data1<<4);
                            pXSprite->data1 = ClipLow(pXSprite->data1-nDmg, 0);
                            sub_76A08(pSprite2, pSprite, pPlayer2);
                            evPost(nSprite, 3, 0, kCallbackRemove);
                            return;
                        }
                    }
                    else
                    {
                        int vd = 0x2666;
                        switch (pSprite2->type)
                        {
                        case kDudeBoneEel:
                        case kDudeBat:
                        case kDudeRat:
                        case kDudeTinyCaleb:
                        case kDudeBeast:
                            vd = 0x147;
                            break;
                        case kDudeZombieAxeBuried:
                        case kDudePodGreen:
                        case kDudeTentacleGreen:
                        case kDudePodFire:
                        case kDudeTentacleFire:
                        case kDudePodMother:
                        case kDudeTentacleMother:
                        case kDudeCerberusTwoHead:
                        case kDudeCerberusOneHead:
                        case kDudeTchernobog:
                        case kDudeBurningInnocent:
                        case kDudeBurningCultist:
                        case kDudeBurningZombieAxe:
                        case kDudeBurningZombieButcher:
                        case kDudeCultistReserved:
                        case kDudeZombieAxeLaying:
                        case kDudeInnocent:
                        case kDudeBurningTinyCaleb:
                        case kDudeBurningBeast:
                            vd = 0;
                            break;
                        }
                        if (vd && (Chance(vd) || nNextSprite < 0))
                        {
                            sub_76A08(pSprite2, pSprite, NULL);
                            evPost(nSprite, 3, 0, kCallbackRemove);
                            return;
                        }
                    }
                }
            }
        }
        pXSprite->data1 = ClipLow(pXSprite->data1-1, 0);
        evPost(nSprite, 3, 0, kCallbackRemove);
    }
}

void(*gCallback[kCallbackMax])(int) =
{
    fxFlameLick,
    Remove,
    FlareBurst,
    fxFlareSpark,
    fxFlareSparkLite,
    fxZombieBloodSpurt,
    fxBloodSpurt,
    fxArcSpark,
    fxDynPuff,
    Respawn,
    PlayerBubble,
    EnemyBubble,
    CounterCheck,
    FinishHim,
    fxBloodBits,
    fxTeslaAlt,
    fxBouncingSleeve,
    returnFlagToBase,
    fxPodBloodSpray,
    fxPodBloodSplat,
    LeechStateTimer,
    DropVoodoo, // unused
    #ifdef NOONE_EXTENSIONS
    callbackUniMissileBurst, // the code is in nnexts.cpp
    callbackMakeMissileBlocking, // the code is in nnexts.cpp
    callbackGenDudeUpdate, // the code is in nnexts.cpp
    #endif
};
