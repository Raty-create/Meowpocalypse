#pragma once

#include "config.h"

typedef enum { MAP_WAITING, MAP_FIRST_HALLWAY, MAP_FIRST_BOSS, MAP_SECOND_HALLWAY, MAP_SECOND_BOSS, MAP_THIRD_HALLWAY, MAP_THIRD_BOSS } MAP_TYPE;
extern MAP_TYPE currentMapType;

typedef struct {
    int tiles[WAITINGMAP_ROWS][WAITINGMAP_COLS];
    int width;
    int height;
} WAITINGMAP;

typedef struct {
    int tiles[FIRST_HALLWAYMAP_ROWS][FIRST_HALLWAYMAP_COLS];
} FIRSTHALLWAYMAP;

extern WAITINGMAP currentMap;
extern FIRSTHALLWAYMAP firsthallwayMap;

void InitWaitingRoom();
void InitFirstHallWay();