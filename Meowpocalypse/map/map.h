#pragma once

#include "config.h"
#include "enum.h"

typedef struct {
    int row;
    int col;
    DOOR_STATE state;
} DOOR_INFO;

typedef struct {
    int tiles[MAX_MAP_ROWS][MAX_MAP_COLS];
    int rows;
    int cols;
    float worldX;
    float worldY;
    DOOR_INFO doors[MAX_DOORS];
    int doorCount;
} MAPDATA;

extern MAP_TYPE currentMapType;
extern MAPDATA maps[7];

MAP_TYPE GetNextMap(MAP_TYPE type);

void GetSpawnPos(MAP_TYPE type, float* outX, float* outY);
void MapTransition();

void InitMap(MAP_TYPE type);
void InitAllMap();

void SetDoorState(MAP_TYPE type, DOOR_STATE state);
void UpdateDoors(MAP_TYPE type, int allEnemiesDead);

void ClearEnemies();