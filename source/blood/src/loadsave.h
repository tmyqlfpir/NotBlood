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
#pragma once
#include <stdio.h>
#include "levels.h"

// load save slot ids
enum
{
    kLoadSaveNull         = -1,
    kLoadSaveSlot0        = 0,
    kLoadSaveSlot1        = 1,
    kLoadSaveSlot2        = 2,
    kLoadSaveSlot3        = 3,
    kLoadSaveSlot4        = 4,
    kLoadSaveSlot5        = 5,
    kLoadSaveSlot6        = 6,
    kLoadSaveSlot7        = 7,
    kLoadSaveSlot8        = 8,
    kLoadSaveSlot9        = 9,

    kLoadSaveSlot10       = 10,
    kLoadSaveSlotQuick    = kLoadSaveSlot10,

    kLoadSaveSlotAutosave = 11,
    kLoadSaveSlotSpawn    = kLoadSaveSlotAutosave,
    kLoadSaveSlotKey      = 12,
    kMaxLoadSaveSlot      = 13,
};

class LoadSave {
public:
    static LoadSave head;
    static FILE *hSFile;
    static int hLFile;
    LoadSave *prev;
    LoadSave *next;
    LoadSave() {
        prev = head.prev;
        prev->next = this;
        next = &head;
        next->prev = this;
    }
    LoadSave(int dummy)
    {
        UNREFERENCED_PARAMETER(dummy);
        next = prev = this;
    }
    //~LoadSave() { }
    virtual void Save(void);
    virtual void Load(void);
    void Read(void *, int);
    void Write(void const *, int);
    static void LoadGame(char *);
    static void SaveGame(char *);
};

extern unsigned int gSavedOffset;
extern GAMEOPTIONS gSaveGameOptions[kMaxLoadSaveSlot];
extern int gSaveGameProfileSkill[kMaxLoadSaveSlot];
extern char *gSaveGamePic[kMaxLoadSaveSlot];
bool LoadSavedInCurrentSession(int nSlot);
void LoadUpdateSaveGame(int nSlot, int nSkill);
void LoadSavedInfo(void);
void LoadAutosavedInfo(void);
void LoadSaveSetup(void);
