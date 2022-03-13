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
#include <stdlib.h>
#include <string.h>

#include "build.h"
#include "common_game.h"

#include "actor.h"
#include "db.h"
#include "gameutil.h"
#include "globals.h"
#include "tile.h"
#include "trig.h"
#include "warp.h"

POINT2D baseWall[kMaxWalls];
POINT3D baseSprite[kMaxSprites];
int baseFloor[kMaxSectors];
int baseCeil[kMaxSectors];
int velFloor[kMaxSectors];
int velCeil[kMaxSectors];
short gUpperLink[kMaxSectors];
short gLowerLink[kMaxSectors];
HITINFO gHitInfo;

static bool AreSectorsNeighborsDepthCheck(int sect1, int sect2, int depth, bool checkRORSectors, char *pSectBit)
{
    if (checkRORSectors) // room over room sector checking
    {
        const int nSprite[2] = {gUpperLink[sect1], gLowerLink[sect1]};
        for (int i = 0; i < 2; i++)
        {
            if (!spriRangeIsFine(nSprite[i])) // invalid sprite, skip
                continue;
            const int nLink = sprite[nSprite[i]].owner;
            if (!spriRangeIsFine(nLink)) // invalid sprite, skip
                continue;
            const int nextSect = sprite[nLink].sectnum;
            if (!sectRangeIsFine(nextSect)) // invalid sector, skip
                continue;
            const bool floorLinked = i == 0;
            const int nextSectPic = floorLinked ? sector[nextSect].ceilingpicnum : sector[nextSect].floorpicnum;
            if ((nextSectPic < 4080) || (nextSectPic > 4095)) // if other room is not open air, skip
                continue;
            if (TestBitString(pSectBit, nextSect)) // if we've already checked this sector, skip
                continue;
            if (nextSect == sect2) // howdy neighbor
                return 1;
            if (depth > 0)
            {
                SetBitString(pSectBit, nextSect); // set the next sector as checked
                if (AreSectorsNeighborsDepthCheck(nextSect, sect2, depth, checkRORSectors, pSectBit))
                    return 1;
            }
        }
    }
    const int nStartWall = sector[sect1].wallptr;
    const int nEndWall = nStartWall + sector[sect1].wallnum;
    for (int i = nStartWall; i < nEndWall; i++) // check all walls
    {
        const int nextSect = wall[i].nextsector;
        if (nextSect < 0) // if next wall isn't linked to a sector, skip
            continue;
        if (!sectRangeIsFine(nextSect)) // invalid sector, skip
            continue;
        if (TestBitString(pSectBit, nextSect)) // if we've already checked this sector, skip
            continue;
        if (nextSect == sect2) // howdy neighbor
            return 1;
        if (depth > 0)
        {
            SetBitString(pSectBit, nextSect); // set the next sector as checked
            if (AreSectorsNeighborsDepthCheck(nextSect, sect2, depth-1, checkRORSectors, pSectBit))
                return 1;
        }
    }
    return 0;
}

bool AreSectorsNeighbors(int sect1, int sect2, int depth, bool checkSameSector, bool checkRORSectors)
{
    if (checkSameSector)
    {
        if (sect1 == sect2) // same sector
            return 1;
    }
    dassert(sect1 >= 0 && sect1 < kMaxSectors);
    dassert(sect2 >= 0 && sect2 < kMaxSectors);
    char sectbits[(kMaxSectors+7)>>3];
    memset(sectbits, 0, sizeof(sectbits));
    int srcSect, dstSect;
    if (sector[sect1].wallnum < sector[sect2].wallnum)
    {
        srcSect = sect1;
        dstSect = sect2;
    }
    else
    {
        srcSect = sect2;
        dstSect = sect1;
    }
    SetBitString(sectbits, srcSect); // set the source sector as checked
    return AreSectorsNeighborsDepthCheck(srcSect, dstSect, depth, checkRORSectors, sectbits);
}

bool FindSector(int nX, int nY, int nZ, int *nSector)
{
    int32_t nZFloor, nZCeil;
    dassert(*nSector >= 0 && *nSector < kMaxSectors);
    if (inside(nX, nY, *nSector))
    {
        getzsofslope(*nSector, nX, nY, &nZCeil, &nZFloor);
        if (nZ >= nZCeil && nZ <= nZFloor)
        {
            return 1;
        }
    }
    walltype *pWall = &wall[sector[*nSector].wallptr];
    for (int i = sector[*nSector].wallnum; i > 0; i--, pWall++)
    {
        int nOSector = pWall->nextsector;
        if (nOSector >= 0 && inside(nX, nY, nOSector))
        {
            getzsofslope(nOSector, nX, nY, &nZCeil, &nZFloor);
            if (nZ >= nZCeil && nZ <= nZFloor)
            {
                *nSector = nOSector;
                return 1;
            }
        }
    }
    for (int i = 0; i < numsectors; i++)
    {
        if (inside(nX, nY, i))
        {
            getzsofslope(i, nX, nY, &nZCeil, &nZFloor);
            if (nZ >= nZCeil && nZ <= nZFloor)
            {
                *nSector = i;
                return 1;
            }
        }
    }
    return 0;
}

bool FindSector(int nX, int nY, int *nSector)
{
    dassert(*nSector >= 0 && *nSector < kMaxSectors);
    if (inside(nX, nY, *nSector))
    {
        return 1;
    }
    walltype *pWall = &wall[sector[*nSector].wallptr];
    for (int i = sector[*nSector].wallnum; i > 0; i--, pWall++)
    {
        int nOSector = pWall->nextsector;
        if (nOSector >= 0 && inside(nX, nY, nOSector))
        {
            *nSector = nOSector;
            return 1;
        }
    }
    for (int i = 0; i < numsectors; i++)
    {
        if (inside(nX, nY, i))
        {
            *nSector = i;
            return 1;
        }
    }
    return 0;
}

void CalcFrameRate(void)
{
    static int ticks[64];
    static int index;
    if (ticks[index] != gFrameClock)
    {
        gFrameRate = (120*64)/((int)gFrameClock-ticks[index]);
        ticks[index] = (int)gFrameClock;
    }
    index = (index+1) & 63;
}

bool CheckProximity(spritetype *pSprite, int nX, int nY, int nZ, int nSector, int nDist)
{
    dassert(pSprite != NULL);
    int oX = klabs(nX-pSprite->x)>>4;
    if (oX >= nDist) return 0;

    int oY = klabs(nY-pSprite->y)>>4;
    if (oY >= nDist) return 0;

    int oZ = klabs(nZ-pSprite->z)>>8;
    if (oZ >= nDist) return 0;

    if (approxDist(oX, oY) >= nDist) return 0;

    int bottom, top;
    GetSpriteExtents(pSprite, &top, &bottom);
    if (cansee(pSprite->x, pSprite->y, pSprite->z, pSprite->sectnum, nX, nY, nZ, nSector))
        return 1;
    if (cansee(pSprite->x, pSprite->y, bottom, pSprite->sectnum, nX, nY, nZ, nSector))
        return 1;
    if (cansee(pSprite->x, pSprite->y, top, pSprite->sectnum, nX, nY, nZ, nSector))
        return 1;
    return 0;
}

bool CheckProximityPoint(int nX1, int nY1, int nZ1, int nX2, int nY2, int nZ2, int nDist)
{
    int oX = klabs(nX2-nX1)>>4;
    if (oX >= nDist)
        return 0;
    int oY = klabs(nY2-nY1)>>4;
    if (oY >= nDist)
        return 0;
    if (nZ2 != nZ1)
    {
        int oZ = klabs(nZ2-nZ1)>>8;
        if (oZ >= nDist)
            return 0;
    }
    if (approxDist(oX, oY) >= nDist) return 0;
    return 1;
}

bool CheckProximityWall(int nWall, int x, int y, int nDist)
{
    int x1 = wall[nWall].x;
    int y1 = wall[nWall].y;
    int x2 = wall[wall[nWall].point2].x;
    int y2 = wall[wall[nWall].point2].y;

    nDist <<= 4;
    if (x1 < x2)
    {
        if (x <= x1 - nDist || x >= x2 + nDist)
        {
            return 0;
        }
    }
    else
    {
        if (x <= x2 - nDist || x >= x1 + nDist)
        {
            return 0;
        }
        if (x1 == x2)
        {
            int px1 = x - x1;
            int py1 = y - y1;
            int px2 = x - x2;
            int py2 = y - y2;
            int dist1 = px1 * px1 + py1 * py1;
            int dist2 = px2 * px2 + py2 * py2;
            if (y1 < y2)
            {
                if (y <= y1 - nDist || y >= y2 + nDist)
                {
                    return 0;
                }
                if (y < y1)
                {
                    return dist1 < nDist * nDist;
                }
                if (y > y2)
                {
                    return dist2 < nDist * nDist;
                }
            }
            else
            {
                if (y <= y2 - nDist || y >= y1 + nDist)
                {
                    return 0;
                }
                if (y < y2)
                {
                    return dist2 < nDist * nDist;
                }
                if (y > y1)
                {
                    return dist1 < nDist * nDist;
                }
            }
            return 1;
        }
    }
    if (y1 < y2)
    {
        if (y <= y1 - nDist || y >= y2 + nDist)
        {
            return 0;
        }
    }
    else
    {
        if (y <= y2 - nDist || y >= y1 + nDist)
        {
            return 0;
        }
        if (y1 == y2)
        {
            int px1 = x - x1;
            int py1 = y - y1;
            int px2 = x - x2;
            int py2 = y - y2;
            int check1 = px1 * px1 + py1 * py1;
            int check2 = px2 * px2 + py2 * py2;
            if (x1 < x2)
            {
                if (x <= x1 - nDist || x >= x2 + nDist)
                {
                    return 0;
                }
                if (x < x1)
                {
                    return check1 < nDist * nDist;
                }
                if (x > x2)
                {
                    return check2 < nDist * nDist;
                }
            }
            else
            {
                if (x <= x2 - nDist || x >= x1 + nDist)
                {
                    return 0;
                }
                if (x < x2)
                {
                    return check2 < nDist * nDist;
                }
                if (x > x1)
                {
                    return check1 < nDist * nDist;
                }
            }
        }
    }

    int dx = x2 - x1;
    int dy = y2 - y1;
    int px = x - x2;
    int py = y - y2;
    int side = px * dx + dy * py;
    if (side >= 0)
    {
        return px * px + py * py < nDist * nDist;
    }
    px = x - x1;
    py = y - y1;
    side = px * dx + dy * py;
    if (side <= 0)
    {
        return px * px + py * py < nDist * nDist;
    }
    int check1 = px * dy - dx * py;
    int check2 = dy * dy + dx * dx;
    return check1 * check1 < check2 * nDist * nDist;
}

int GetWallAngle(int nWall)
{
    int nWall2 = wall[nWall].point2;
    return getangle(wall[nWall2].x - wall[nWall].x, wall[nWall2].y - wall[nWall].y);
}

void GetWallNormal(int nWall, int *pX, int *pY)
{
    dassert(nWall >= 0 && nWall < kMaxWalls);
    int nWall2 = wall[nWall].point2;
    int dX = -(wall[nWall2].y - wall[nWall].y);
    dX >>= 4;
    int dY = wall[nWall2].x - wall[nWall].x;
    dY >>= 4;
    int nLength = ksqrt(dX*dX+dY*dY);
    if (nLength <= 0)
        nLength = 1;
    *pX = divscale16(dX, nLength);
    *pY = divscale16(dY, nLength);
}

bool IntersectRay(int wx, int wy, int wdx, int wdy, int x1, int y1, int z1, int x2, int y2, int z2, int *ix, int *iy, int *iz)
{
    int dX = x1 - x2;
    int dY = y1 - y2;
    int dZ = z1 - z2;
    int side = wdx * dY - wdy * dX;
    int dX2 = x1 - wx;
    int dY2 = y1 - wy;
    int check1 = dX2 * dY - dY2 * dX;
    int check2 = wdx * dY2 - wdy * dX2;
    if (side >= 0)
    {
        if (!side)
            return 0;
        if (check1 < 0)
            return 0;
        if (check2 < 0 || check2 >= side)
            return 0;
    }
    else
    {
        if (check1 > 0)
            return 0;
        if (check2 > 0 || check2 <= side)
            return 0;
    }
    int nScale = divscale16(check2, side);
    *ix = x1 + mulscale16(dX, nScale);
    *iy = y1 + mulscale16(dY, nScale);
    *iz = z1 + mulscale16(dZ, nScale);
    return 1;
}

int HitScan(spritetype *pSprite, int z, int dx, int dy, int dz, unsigned int nMask, int nRange)
{
    dassert(pSprite != NULL);
    dassert(dx != 0 || dy != 0);
    gHitInfo.hitsect = -1;
    gHitInfo.hitwall = -1;
    gHitInfo.hitsprite = -1;
    int x = pSprite->x;
    int y = pSprite->y;
    int nSector = pSprite->sectnum;
    int bakCstat = pSprite->cstat;
    pSprite->cstat &= ~256;
    if (nRange)
    {
        hitscangoal.x = x + mulscale30(nRange << 4, Cos(pSprite->ang));
        hitscangoal.y = y + mulscale30(nRange << 4, Sin(pSprite->ang));
    }
    else
    {
        hitscangoal.x = hitscangoal.y = 0x1ffffff;
    }
    vec3_t pos = { x, y, z };
    hitdata_t hitData;
    hitData.xyz.z = gHitInfo.hitz;
    hitscan(&pos, nSector, dx, dy, dz << 4, &hitData, nMask);
    gHitInfo.hitsect = hitData.sect;
    gHitInfo.hitwall = hitData.wall;
    gHitInfo.hitsprite = hitData.sprite;
    gHitInfo.hitx = hitData.xyz.x;
    gHitInfo.hity = hitData.xyz.y;
    gHitInfo.hitz = hitData.xyz.z;
    hitscangoal.x = hitscangoal.y = 0x1ffffff;
    pSprite->cstat = bakCstat;
    if (gHitInfo.hitsprite >= kMaxSprites || gHitInfo.hitwall >= kMaxWalls || gHitInfo.hitsect >= kMaxSectors)
        return -1;
    if (gHitInfo.hitsprite >= 0)
        return 3;
    if (gHitInfo.hitwall >= 0)
    {
        if (wall[gHitInfo.hitwall].nextsector == -1)
            return 0;
        int nZCeil, nZFloor;
        getzsofslope(wall[gHitInfo.hitwall].nextsector, gHitInfo.hitx, gHitInfo.hity, &nZCeil, &nZFloor);
        if (gHitInfo.hitz <= nZCeil || gHitInfo.hitz >= nZFloor)
            return 0;
        return 4;
    }
    if (gHitInfo.hitsect >= 0)
        return 1 + (z < gHitInfo.hitz);
    return -1;
}

int VectorScan(spritetype *pSprite, int nOffset, int nZOffset, int dx, int dy, int dz, int nRange, int ac, vec3_t *adjustedRORPos)
{
    int nNum = 256;
    dassert(pSprite != NULL);
    gHitInfo.hitsect = -1;
    gHitInfo.hitwall = -1;
    gHitInfo.hitsprite = -1;
    int x1 = pSprite->x+mulscale30(nOffset, Cos(pSprite->ang+512));
    int y1 = pSprite->y+mulscale30(nOffset, Sin(pSprite->ang+512));
    int z1 = pSprite->z+nZOffset;
    int bakCstat = pSprite->cstat;
    pSprite->cstat &= ~256;
    int nSector = pSprite->sectnum;
    if (nRange)
    {
        hitscangoal.x = x1+mulscale30(nRange<<4, Cos(pSprite->ang));
        hitscangoal.y = y1+mulscale30(nRange<<4, Sin(pSprite->ang));
    }
    else
    {
        hitscangoal.x = hitscangoal.y = 0x1fffffff;
    }
    vec3_t pos = { x1, y1, z1 };
    hitdata_t hitData;
    hitData.xyz.z = gHitInfo.hitz;
    hitscan(&pos, nSector, dx, dy, dz << 4, &hitData, CLIPMASK1);
    gHitInfo.hitsect = hitData.sect;
    gHitInfo.hitwall = hitData.wall;
    gHitInfo.hitsprite = hitData.sprite;
    gHitInfo.hitx = hitData.xyz.x;
    gHitInfo.hity = hitData.xyz.y;
    gHitInfo.hitz = hitData.xyz.z;
    hitscangoal.x = hitscangoal.y = 0x1ffffff;
    pSprite->cstat = bakCstat;
    while (nNum--)
    {
        if (gHitInfo.hitsprite >= kMaxSprites || gHitInfo.hitwall >= kMaxWalls || gHitInfo.hitsect >= kMaxSectors)
            return -1;
        if (adjustedRORPos) // check using ror sector position offset instead of absolute sprite location
        {
            if (nRange && approxDist(gHitInfo.hitx - adjustedRORPos->x, gHitInfo.hity - adjustedRORPos->y) > nRange)
                return -1;      
        }
        else // original method - use sprite location (doesn't take into account ror sector position offset)
        {
            if (nRange && approxDist(gHitInfo.hitx - pSprite->x, gHitInfo.hity - pSprite->y) > nRange)
                return -1;            
        }
        if (gHitInfo.hitsprite >= 0)
        {
            spritetype *pOther = &sprite[gHitInfo.hitsprite];
            if ((pOther->flags & 8) && !(ac & 1))
                return 3;
            if ((pOther->cstat & 0x30) != 0)
                return 3;
            int nPicnum = pOther->picnum;
            if (tilesiz[nPicnum].x == 0 || tilesiz[nPicnum].y == 0)
                return 3;
            int height = (tilesiz[nPicnum].y*pOther->yrepeat)<<2;
            int otherZ = pOther->z;
            if (pOther->cstat & 0x80)
                otherZ += height / 2;
            int nOffset = picanm[nPicnum].yofs;
            if (nOffset)
                otherZ -= (nOffset*pOther->yrepeat)<<2;
            dassert(height > 0);
            int height2 = scale(otherZ-gHitInfo.hitz, tilesiz[nPicnum].y, height);
            if (!(pOther->cstat & 8))
                height2 = tilesiz[nPicnum].y-height2;
            if (height2 >= 0 && height2 < tilesiz[nPicnum].y)
            {
                int width = (tilesiz[nPicnum].x*pOther->xrepeat)>>2;
                width = (width*3)/4;
                int check1 = ((y1 - pOther->y)*dx - (x1 - pOther->x)*dy) / ksqrt(dx*dx+dy*dy);
                dassert(width > 0);
                int width2 = scale(check1, tilesiz[nPicnum].x, width);
                int nOffset = picanm[nPicnum].xofs;
                width2 += nOffset + tilesiz[nPicnum].x / 2;
                if (width2 >= 0 && width2 < tilesiz[nPicnum].x)
                {
                    char *pData = tileLoadTile(nPicnum);
                    if (pData[width2*tilesiz[nPicnum].y+height2] != (char)255)
                        return 3;
                }
            }
            int bakCstat = pOther->cstat;
            pOther->cstat &= ~256;
            gHitInfo.hitsect = -1;
            gHitInfo.hitwall = -1;
            gHitInfo.hitsprite = -1;
            x1 = gHitInfo.hitx;
            y1 = gHitInfo.hity;
            z1 = gHitInfo.hitz;
            pos = { x1, y1, z1 };
            hitData.xyz.z = gHitInfo.hitz;
            hitscan(&pos, pOther->sectnum,
                dx, dy, dz << 4, &hitData, CLIPMASK1);
            gHitInfo.hitsect = hitData.sect;
            gHitInfo.hitwall = hitData.wall;
            gHitInfo.hitsprite = hitData.sprite;
            gHitInfo.hitx = hitData.xyz.x;
            gHitInfo.hity = hitData.xyz.y;
            gHitInfo.hitz = hitData.xyz.z;
            pOther->cstat = bakCstat;
            continue;
        }
        if (gHitInfo.hitwall >= 0)
        {
            walltype *pWall = &wall[gHitInfo.hitwall];
            if (pWall->nextsector == -1)
                return 0;
            sectortype *pSector = &sector[gHitInfo.hitsect];
            sectortype *pSectorNext = &sector[pWall->nextsector];
            int nZCeil, nZFloor;
            getzsofslope(pWall->nextsector, gHitInfo.hitx, gHitInfo.hity, &nZCeil, &nZFloor);
            if (gHitInfo.hitz <= nZCeil)
                return 0;
            if (gHitInfo.hitz >= nZFloor)
            {
                if (!(pSector->floorstat&1) || !(pSectorNext->floorstat&1))
                    return 0;
                return 2;
            }
            if (!(pWall->cstat & 0x30))
                return 0;
            int nOffset;
            if (pWall->cstat & 4)
                nOffset = ClipHigh(pSector->floorz, pSectorNext->floorz);
            else
                nOffset = ClipLow(pSector->ceilingz, pSectorNext->ceilingz);
            nOffset = (gHitInfo.hitz - nOffset) >> 8;
            if (pWall->cstat & 256)
                nOffset = -nOffset;

            int nPicnum = pWall->overpicnum;
            int nSizX = tilesiz[nPicnum].x;
            int nSizY = tilesiz[nPicnum].y;
            if (!nSizX || !nSizY)
                return 0;

            int potX = nSizX == (1<<(picsiz[nPicnum]&15));
            int potY = nSizY == (1<<(picsiz[nPicnum]>>4));

            nOffset = (nOffset*pWall->yrepeat) / 8;
            nOffset += (nSizY*pWall->ypanning) / 256;
            int nLength = approxDist(pWall->x - wall[pWall->point2].x, pWall->y - wall[pWall->point2].y);
            int nHOffset;
            if (pWall->cstat & 8)
                nHOffset = approxDist(gHitInfo.hitx - wall[pWall->point2].x, gHitInfo.hity - wall[pWall->point2].y);
            else
                nHOffset = approxDist(gHitInfo.hitx - pWall->x, gHitInfo.hity - pWall->y);

            nHOffset = pWall->xpanning + ((nHOffset*pWall->xrepeat) << 3) / nLength;
            if (potX)
                nHOffset &= nSizX - 1;
            else
                nHOffset %= nSizX;
            if (potY)
                nOffset &= nSizY - 1;
            else
                nOffset %= nSizY;
            char *pData = tileLoadTile(nPicnum);
            int nPixel;
            if (potY)
                nPixel = (nHOffset<<(picsiz[nPicnum]>>4)) + nOffset;
            else
                nPixel = nHOffset*nSizY + nOffset;

            if (pData[nPixel] == (char)255)
            {
                int bakCstat = pWall->cstat;
                pWall->cstat &= ~64;
                int bakCstat2 = wall[pWall->nextwall].cstat;
                wall[pWall->nextwall].cstat &= ~64;
                gHitInfo.hitsect = -1;
                gHitInfo.hitwall = -1;
                gHitInfo.hitsprite = -1;
                x1 = gHitInfo.hitx;
                y1 = gHitInfo.hity;
                z1 = gHitInfo.hitz;
                pos = { x1, y1, z1 };
                hitData.xyz.z = gHitInfo.hitz;
                hitscan(&pos, pWall->nextsector,
                    dx, dy, dz << 4, &hitData, CLIPMASK1);
                gHitInfo.hitsect = hitData.sect;
                gHitInfo.hitwall = hitData.wall;
                gHitInfo.hitsprite = hitData.sprite;
                gHitInfo.hitx = hitData.xyz.x;
                gHitInfo.hity = hitData.xyz.y;
                gHitInfo.hitz = hitData.xyz.z;
                pWall->cstat = bakCstat;
                wall[pWall->nextwall].cstat = bakCstat2;
                continue;
            }
            return 4;
        }
        if (gHitInfo.hitsect >= 0)
        {
            int nSprite;
            if (dz > 0)
            {
                if (gUpperLink[gHitInfo.hitsect] < 0)
                    return 2;
                nSprite = gUpperLink[gHitInfo.hitsect];
            }
            else
            {
                if (gLowerLink[gHitInfo.hitsect] < 0)
                    return 1;
                nSprite = gLowerLink[gHitInfo.hitsect];
            }
            int nLink = sprite[nSprite].owner & 0x3fff;
            gHitInfo.hitsect = -1;
            gHitInfo.hitwall = -1;
            gHitInfo.hitsprite = -1;
            x1 = gHitInfo.hitx + sprite[nLink].x - sprite[nSprite].x;
            y1 = gHitInfo.hity + sprite[nLink].y - sprite[nSprite].y;
            z1 = gHitInfo.hitz + sprite[nLink].z - sprite[nSprite].z;
            if (adjustedRORPos && nRange) // only adjust if vector has range limit
            {
                adjustedRORPos->x = adjustedRORPos->x + sprite[nLink].x - sprite[nSprite].x;
                adjustedRORPos->y = adjustedRORPos->y + sprite[nLink].y - sprite[nSprite].y;
                adjustedRORPos->z = adjustedRORPos->z + sprite[nLink].z - sprite[nSprite].z;
            }
            pos = { x1, y1, z1 };
            hitData.xyz.z = gHitInfo.hitz;
            hitscan(&pos, sprite[nLink].sectnum, dx, dy, dz<<4, &hitData, CLIPMASK1);
            gHitInfo.hitsect = hitData.sect;
            gHitInfo.hitwall = hitData.wall;
            gHitInfo.hitsprite = hitData.sprite;
            gHitInfo.hitx = hitData.xyz.x;
            gHitInfo.hity = hitData.xyz.y;
            gHitInfo.hitz = hitData.xyz.z;
            continue;
        }
        return -1;
    }
    return -1;
}

int VectorScanROR(spritetype *pSprite, int nOffset, int nZOffset, int dx, int dy, int dz, int nRange, int ac, vec3_t *adjustedRORPos)
{
    // this function operates the same as VectorScan() but it'll check if initial starting position is clipping into a ror sector, as well as accumulating offsets for every ror sector traversal
    const vec3_t bakPos = pSprite->xyz;
    const short bakSect = pSprite->sectnum;
    bool restorePosSect = false;
    int x = pSprite->x+mulscale30(nOffset, Cos(pSprite->ang+512));
    int y = pSprite->y+mulscale30(nOffset, Sin(pSprite->ang+512));
    int z = pSprite->z+nZOffset;
    int nSector = pSprite->sectnum;
    const int cX = x-pSprite->x, cY = y-pSprite->y, cZ = z-pSprite->z;
    if (CheckLink(&x, &y, &z, &nSector)) // if hitscan start position is overlapping into ror sector, move sprite to ror sector
    {
        restorePosSect = true;
        pSprite->x = x-cX;
        pSprite->y = y-cY;
        pSprite->z = z-cZ;
        pSprite->sectnum = nSector;
    }
    if (adjustedRORPos)
        *adjustedRORPos = pSprite->xyz;
    int hit = VectorScan(pSprite, nOffset, nZOffset, dx, dy, dz, nRange, ac, adjustedRORPos);
    if (restorePosSect) // restore sprite position and sectnum
    {
        pSprite->xyz = bakPos;
        pSprite->sectnum = bakSect;
    }
    return hit;
}

void GetZRange(spritetype *pSprite, int *ceilZ, int *ceilHit, int *floorZ, int *floorHit, int nDist, unsigned int nMask, unsigned int nClipParallax)
{
    dassert(pSprite != NULL);
    int bakCstat = pSprite->cstat;
    int32_t nTemp1, nTemp2;
    pSprite->cstat &= ~257;
    getzrange_old(pSprite->x, pSprite->y, pSprite->z, pSprite->sectnum, (int32_t*)ceilZ, (int32_t*)ceilHit, (int32_t*)floorZ, (int32_t*)floorHit, nDist, nMask);
    if (((*floorHit) & 0xc000) == 0x4000)
    {
        int nSector = (*floorHit) & 0x3fff;
        if ((nClipParallax & PARALLAXCLIP_FLOOR) == 0 && (sector[nSector].floorstat & 1))
            *floorZ = 0x7fffffff;
        if (sector[nSector].extra > 0)
        {
            XSECTOR *pXSector = &xsector[sector[nSector].extra];
            *floorZ += pXSector->Depth << 10;
        }
        if (gUpperLink[nSector] >= 0)
        {
            int nSprite = gUpperLink[nSector];
            int nLink = sprite[nSprite].owner & 0x3fff;
            getzrange_old(pSprite->x+sprite[nLink].x-sprite[nSprite].x, pSprite->y+sprite[nLink].y-sprite[nSprite].y,
                pSprite->z+sprite[nLink].z-sprite[nSprite].z, sprite[nLink].sectnum, &nTemp1, &nTemp2, (int32_t*)floorZ, (int32_t*)floorHit,
                nDist, nMask);
            *floorZ -= sprite[nLink].z - sprite[nSprite].z;
        }
    }
    if (((*ceilHit) & 0xc000) == 0x4000)
    {
        int nSector = (*ceilHit) & 0x3fff;
        if ((nClipParallax & PARALLAXCLIP_CEILING) == 0 && (sector[nSector].ceilingstat & 1))
            *ceilZ = 0x80000000;
        if (gLowerLink[nSector] >= 0)
        {
            int nSprite = gLowerLink[nSector];
            int nLink = sprite[nSprite].owner & 0x3fff;
            getzrange_old(pSprite->x+sprite[nLink].x-sprite[nSprite].x, pSprite->y+sprite[nLink].y-sprite[nSprite].y,
                pSprite->z+sprite[nLink].z-sprite[nSprite].z, sprite[nLink].sectnum, (int32_t*)ceilZ, (int32_t*)ceilHit, &nTemp1, &nTemp2,
                nDist, nMask);
            *ceilZ -= sprite[nLink].z - sprite[nSprite].z;
        }
    }
    pSprite->cstat = bakCstat;
}

void GetZRangeAtXYZ(int x, int y, int z, int nSector, int *ceilZ, int *ceilHit, int *floorZ, int *floorHit, int nDist, unsigned int nMask, unsigned int nClipParallax)
{
    int32_t nTemp1, nTemp2;
    getzrange_old(x, y, z, nSector, (int32_t*)ceilZ, (int32_t*)ceilHit, (int32_t*)floorZ, (int32_t*)floorHit, nDist, nMask);
    if (((*floorHit) & 0xc000) == 0x4000)
    {
        int nSector = (*floorHit) & 0x3fff;
        if ((nClipParallax & PARALLAXCLIP_FLOOR) == 0 && (sector[nSector].floorstat & 1))
            *floorZ = 0x7fffffff;
        if (sector[nSector].extra > 0)
        {
            XSECTOR *pXSector = &xsector[sector[nSector].extra];
            *floorZ += pXSector->Depth << 10;
        }
        if (gUpperLink[nSector] >= 0)
        {
            int nSprite = gUpperLink[nSector];
            int nLink = sprite[nSprite].owner & 0x3fff;
            getzrange_old(x+sprite[nLink].x-sprite[nSprite].x, y+sprite[nLink].y-sprite[nSprite].y,
                z+sprite[nLink].z-sprite[nSprite].z, sprite[nLink].sectnum, &nTemp1, &nTemp2, (int32_t*)floorZ, (int32_t*)floorHit,
                nDist, nMask);
            *floorZ -= sprite[nLink].z - sprite[nSprite].z;
        }
    }
    if (((*ceilHit) & 0xc000) == 0x4000)
    {
        int nSector = (*ceilHit) & 0x3fff;
        if ((nClipParallax & PARALLAXCLIP_CEILING) == 0 && (sector[nSector].ceilingstat & 1))
            *ceilZ = 0x80000000;
        if (gLowerLink[nSector] >= 0)
        {
            int nSprite = gLowerLink[nSector];
            int nLink = sprite[nSprite].owner & 0x3fff;
            getzrange_old(x+sprite[nLink].x-sprite[nSprite].x, y+sprite[nLink].y-sprite[nSprite].y,
                z+sprite[nLink].z-sprite[nSprite].z, sprite[nLink].sectnum, (int32_t*)ceilZ, (int32_t*)ceilHit, &nTemp1, &nTemp2,
                nDist, nMask);
            *ceilZ -= sprite[nLink].z - sprite[nSprite].z;
        }
    }
}

int GetDistToLine(int x1, int y1, int x2, int y2, int x3, int y3)
{
    int check = (y1-y3)*(x3-x2);
    int check2 = (x1-x2)*(y3-y2);
    if (check2 > check)
        return -1;
    int v8 = dmulscale4(x1-x2,x3-x2,y1-y3,y3-y2);
    int vv = dmulscale4(x3-x2,x3-x2,y3-y2,y3-y2);
    int t1, t2;
    if (v8 <= 0)
    {
        t1 = x2;
        t2 = y2;
    }
    else if (vv > v8)
    {
        t1 = x2+scale(x3-x2,v8,vv);
        t2 = y2+scale(y3-y2,v8,vv);
    }
    else
    {
        t1 = x3;
        t2 = y3;
    }
    return approxDist(t1-x1, t2-y1);
}

unsigned int ClipMove(int *x, int *y, int *z, int *nSector, int xv, int yv, int wd, int cd, int fd, unsigned int nMask)
{
    int bakX = *x;
    int bakY = *y;
    int bakZ = *z;
    short bakSect = *nSector;
    unsigned int nRes = clipmove_old((int32_t*)x, (int32_t*)y, (int32_t*)z, &bakSect, xv<<14, yv<<14, wd, cd, fd, nMask);
    if (bakSect == -1)
    {
        *x = bakX; *y = bakY; *z = bakZ;
    }
    else
    {
        *nSector = bakSect;
    }
    return nRes;
}

unsigned int ClipMoveEDuke(spritetype *raySprite, int *x, int *y, int *z, int *nSector, int xv, int yv, int wd, int cd, int fd, unsigned int nMask)
{
    // while this function may look as hideous as any build engine internals, it's been carefully setup like a stack of cards
    // do not touch unless you know what you're doing, or are severely drunk
    // good levels to test with - the first breakable cave wall of CPE1M1 and DWE2M8's skull key room
    // providing a sprite will use additional raycast collision testing, otherwise this will behave like ClipMove()
    if ((xv == 0) && (yv == 0)) // not moving, don't bother continuing
        return 0;
    int origX = *x;
    int origY = *y;
    int origZ = *z;
    short updSect = *nSector, origSect = *nSector;
    const int bakCompat = enginecompatibilitymode;
    enginecompatibilitymode = ENGINE_EDUKE32; // improved clipmove accuracy
    unsigned int nRes = clipmove_old((int32_t*)x, (int32_t*)y, (int32_t*)z, &updSect, xv<<14, yv<<14, wd, cd, fd, nMask);
    enginecompatibilitymode = bakCompat; // restore
    if (updSect == -1) // clipped out of bounds, restore position
        *x = origX, *y = origY, *z = origZ;
    else
        *nSector = updSect;
    if ((nRes > 0) || (raySprite == NULL)) // if got a hit, or sprite is null, return
        return nRes;

    if ((*x == origX) && (*y == origY)) // if sprite got stuck, perform some special build guru meditation and unfuck the position with a horrible hack
    {
        *x += xv; // manually move position to the intended destination - hopefully face first into a wall
        *y += yv;
    }

    // we didn't hit shit, let's raycast and try again
    bool seekSector = false;
    vec3_t pos = {origX, origY, origZ};
    hitdata_t hitData;
    hitData.xyz = pos;
    hitscangoal.x = *x;
    hitscangoal.y = *y;
    hitscan(&pos, origSect, Cos(raySprite->ang)>>16, Sin(raySprite->ang)>>16, 0, &hitData, nMask);
    if (hitData.sprite >= kMaxSprites || hitData.wall >= kMaxWalls || hitData.sect >= kMaxSectors) // we didn't hit shit, get current sector and return
        seekSector = true;
    else if (hitData.sprite >= 0 || hitData.wall >= 0) // did we hit something, and was it a sprite/wall
    {
        const int distRay = approxDist(origX-hitData.xyz.x, origY-hitData.xyz.y);
        *x = hitData.xyz.x; // set to raycast position
        *y = hitData.xyz.y;
        *z = hitData.xyz.z;
        *nSector = hitData.sect;
        wd = raySprite->clipdist<<2; // use the original wd values to offset from wall/sprite
        if ((distRay - ((wd+8)<<1)) > 0) // if there is enough room to offset from ray's hit surface
        {
            *x -= mulscale30(wd+8, Cos(raySprite->ang)); // offset using walldist argument
            *y -= mulscale30(wd+8, Sin(raySprite->ang));
            seekSector = true; // we've moved the sprite, there might be a very small possibility that the sector is misaligned so check this
        }
        if (hitData.sprite >= 0)
            nRes = (hitData.sprite & 0x3FFF) | 0x8000;
        else
            nRes = (hitData.wall & 0x3FFF) | 0xC000;
    }
    if (seekSector) // find sector for sprite
    {
        if (!FindSector(*x, *y, *z, nSector)) // we couldn't find where we're supposed to be, just restore back to original position and return
            *nSector = -1;
        else if (!cansee(origX, origY, origZ, origSect, *x, *y, *z, *nSector)) // if the new updated sector is broken (this will happen when there are shared sectors in the same XYZ location such as DWE2M8's skull key room)
            *nSector = -1;
    }
    if (*nSector == -1) // if sector still failed, restore and pray to the build gods that everything will work out
    {
        *x = origX, *y = origY, *z = origZ;
        *nSector = origSect;
    }
    return nRes;
}

int GetClosestSectors(int nSector, int x, int y, int nDist, short *pSectors, char *pSectBit)
{
    char sectbits[(kMaxSectors+7)>>3];
    dassert(pSectors != NULL);
    memset(sectbits, 0, sizeof(sectbits));
    pSectors[0] = nSector;
    SetBitString(sectbits, nSector);
    int n = 1;
    int i = 0;
    if (pSectBit)
    {
        memset(pSectBit, 0, (kMaxSectors+7)>>3);
        SetBitString(pSectBit, nSector);
    }
    while (i < n)
    {
        const int nCurSector = pSectors[i];
        const int nStartWall = sector[nCurSector].wallptr;
        const int nEndWall = nStartWall + sector[nCurSector].wallnum;
        for (int j = nStartWall; j < nEndWall; j++)
        {
            const walltype *pWall = &wall[j];
            const int nNextSector = pWall->nextsector;
            if (nNextSector < 0)
                continue;
            if (TestBitString(sectbits, nNextSector))
                continue;
            SetBitString(sectbits, nNextSector);
            int dx = klabs(wall[pWall->point2].x - x)>>4;
            int dy = klabs(wall[pWall->point2].y - y)>>4;
            if (dx < nDist && dy < nDist)
            {
                if (approxDist(dx, dy) < nDist)
                {
                    if (pSectBit)
                        SetBitString(pSectBit, nNextSector);
                    pSectors[n++] = nNextSector;
                }
            }
        }
        i++;
    }
    pSectors[n] = -1;
    return n;
}

inline double SquareDist(double lx1, double ly1, double lx2, double ly2)
{
    double dx = lx2 - lx1;
    double dy = ly2 - ly1;
    return dx * dx + dy * dy;
}

double SquareDistToWall(double px, double py, const walltype* pWall)
{
    double lx1 = pWall->x;
    double ly1 = pWall->y;
    double lx2 = wall[pWall->point2].x;
    double ly2 = wall[pWall->point2].y;

    double wall_length = SquareDist(lx1, ly1, lx2, ly2);

    if (wall_length == 0) return SquareDist(px, py, lx1, ly1);

    double t = ((px - lx1) * (lx2 - lx1) + (py - ly1) * (ly2 - ly1)) / wall_length;
    t = clamp(t, 0., 1.);
    return SquareDist(px, py, lx1 + t * (lx2 - lx1), ly1 + t * (ly2 - ly1));
}

int GetClosestSpriteSectors(int nSector, int x, int y, int nDist, short *pSectors, char *pSectBit, short *pWalls, bool newSectCheckMethod, bool newSectCheckMethodRaze)
{
    // by default this function fails with sectors that linked with wide spans, or there was more than one link to the same sector. for example...
    // E6M1: throwing TNT on the stone footpath while standing on the brown road will fail due to the start/end points of the span being too far away. it'll only do damage at one end of the road
    // E1M2: throwing TNT at the double doors while standing on the train platform
    // by setting newSectCheckMethod to true these issues will be resolved
    char sectbits[(kMaxSectors+7)>>3];
    dassert(pSectors != NULL);
    memset(sectbits, 0, sizeof(sectbits));
    pSectors[0] = nSector;
    SetBitString(sectbits, nSector);
    int n = 1, m = 0;
    int i = 0;
    if (pSectBit)
    {
        memset(pSectBit, 0, (kMaxSectors+7)>>3);
        SetBitString(pSectBit, nSector);
    }
    while (i < n) // scan through sectors
    {
        const int nCurSector = pSectors[i];
        const int nStartWall = sector[nCurSector].wallptr;
        const int nEndWall = nStartWall + sector[nCurSector].wallnum;
        for (int j = nStartWall; j < nEndWall; j++) // scan each wall of current sector for new sectors
        {
            const walltype *pWall = &wall[j];
            const int nNextSector = pWall->nextsector;
            if (nNextSector < 0) // if next wall isn't linked to a sector, skip
                continue;
            if (TestBitString(sectbits, nNextSector)) // if we've already checked this sector, skip
                continue;
            bool setSectBit = true;
            bool withinRange = false;
            if (!newSectCheckMethod) // original method
            {
                withinRange = CheckProximityWall(pWall->point2, x, y, nDist);
            }
            else // new method
            {
                if (newSectCheckMethodRaze) // raze - just straight up calculate the distance
                {
                    const double nDist4sq = 256. * nDist * nDist; // (nDist * 16)^2 - * 16 to account for build's 28.4 fixed point format
                    const double dist1 = SquareDistToWall(x, y, pWall);
                    withinRange = dist1 <= nDist4sq;
                    setSectBit = false; // check every wall, every time
                }
                else // notblood - first test edges and then wall span midpoints
                {
                    for (int k = (j+1); k < nEndWall; k++) // scan through the rest of the sector's walls
                    {
                        if (wall[k].nextsector == nNextSector) // if the next walls still reference the sector, then don't flag the sector as checked (yet)
                        {
                            setSectBit = false;
                            break;
                        }
                    }
                    const int nWallA = j;
                    const int nWallB = wall[nWallA].point2;
                    int x1 = wall[nWallA].x, y1 = wall[nWallA].y;
                    int x2 = wall[nWallB].x, y2 = wall[nWallB].y;
                    int point1Dist = approxDist(x-x1, y-y1); // setup edge distance needed for below loop (determines which point to shift closer to center)
                    int point2Dist = approxDist(x-x2, y-y2);
                    int nLength = approxDist(x1-x2, y1-y2);
                    const int nDist4 = nDist<<4;
                    nLength = ClipRange(nLength / (nDist4+(nDist4>>1)), 1, 4); // always test midpoint at least once, and never split more than 4 times
                    for (int k = 0; true; k++) // check both points of wall and subdivide span into smaller chunks towards target
                    {
                        withinRange = (point1Dist < nDist4) || (point2Dist < nDist4); // check if both points of span is within radius
                        if (withinRange)
                            break;
                        if (k == nLength) // reached end
                            break;
                        const int xcenter = (x1+x2)>>1, ycenter = (y1+y2)>>1;
                        if (point1Dist < point2Dist) // shift closest side of wall towards target point, and refresh point distance values
                        {
                            x2 = xcenter, y2 = ycenter;
                            point2Dist = approxDist(x-x2, y-y2);
                        }
                        else
                        {
                            x1 = xcenter, y1 = ycenter;
                            point1Dist = approxDist(x-x1, y-y1);
                        }
                    }
                }
            }
            if (withinRange) // if new sector is within range, set to current sector and test walls
            {
                setSectBit = true; // sector is within range, set the sector as checked
                if (pSectBit)
                    SetBitString(pSectBit, nNextSector);
                pSectors[n++] = nNextSector;
                if (pWalls && pWall->extra > 0)
                {
                    XWALL *pXWall = &xwall[pWall->extra];
                    if (pXWall->triggerVector && !pXWall->isTriggered && !pXWall->state)
                        pWalls[m++] = j;
                }
            }
            if (setSectBit)
                SetBitString(sectbits, nNextSector);
        }
        i++;
    }
    pSectors[n] = -1;
    if (pWalls)
    {
        pWalls[m] = -1;
    }
    return n;
}

int picWidth(short nPic, short repeat) {
    return ClipLow((tilesiz[nPic].x * repeat) << 2, 0);
}

int picHeight(short nPic, short repeat) {
    return ClipLow((tilesiz[nPic].y * repeat) << 2, 0);
}


