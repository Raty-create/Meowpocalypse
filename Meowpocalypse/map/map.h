#pragma once

#include "config.h"

typedef enum { MAP_WAITING, MAP_FIRST_HALLWAY, MAP_FIRST_BOSS, MAP_SECOND_HALLWAY, MAP_SECOND_BOSS, MAP_THIRD_HALLWAY, MAP_THIRD_BOSS } MAP_TYPE;
extern MAP_TYPE currentMapType;

typedef enum {
    DOOR_CLOSE,
    DOOR_OPEN
} DOOR_STATE;

typedef struct {
    int row;
    int col;
    DOOR_STATE state;
} DOOR_INFO;

typedef struct {
    int tiles[MAX_MAP_ROWS][MAX_MAP_COLS];
    int rows;
    int cols;
    int worldX;
    int worldY;
    DOOR_INFO doors[MAX_DOORS];
    int doorCount;
} MAPDATA;

extern MAPDATA maps[7];

MAP_TYPE GetNextMap(MAP_TYPE type);

void GetSpawnPos(MAP_TYPE type, float* outX, float* outY);

void InitMap(MAP_TYPE type);
void InitAllMap();

void SetDoorState(MAP_TYPE type, DOOR_STATE state);
void UpdateDoors(MAP_TYPE type, int allEnemiesDead);