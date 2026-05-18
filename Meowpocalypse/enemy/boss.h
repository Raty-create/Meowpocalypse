#pragma once
#include "object.h"

typedef struct {
    OBJECT base;
    int isActive;
    int attackTimer;
    int pawTimer;
    int dashTimer;
    int isDashing;
    float dashDirX;
    float dashDirY;
    int moveTimer;
    float moveDirX;
    float moveDirY;
} BOSS;

typedef struct {
    float x, y;
    float dx, dy;
    int isActive;
} BOSS_PAW;

typedef struct {
    float startX, startY;
    float dirX, dirY;
    float perpX, perpY;
    int stopDist;
    int isActive;
    int timer;
} DASH_WARNING;

extern BOSS boss;
extern BOSS_PAW bossPaws[BOSS_PAW_LIMIT];
extern DASH_WARNING dashWarn;

void InitBoss();
void SpawnBoss(MAP_TYPE type);
void UpdateBoss();