#pragma once
#include "object.h"
#include "config.h"
#include "map.h"
#include "enum.h"

typedef struct {
    float x, y;
    float dx, dy;
    int isActive;
} CATPAW;

typedef struct {
    OBJECT base;
    int isActive;
    int shootTimer;
    int moveTimer;
} ENEMY;

extern ENEMY enemies[ENEMY_LIMIT];
extern CATPAW catpaw[CAT_PAW_LIMIT];

void InitEnemy();
void ClearEnemies();
void SpawnEnemy(MAP_TYPE type, int count);
void UpdateEnemies();