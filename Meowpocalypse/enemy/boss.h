#pragma once
#include "object.h"

typedef struct {
    OBJECT base;
    int isActive;
    int invincibleTimer;

    // 보스 공격
    int attackTimer;
    int pawTimer;

    //보스 스킬(대쉬)
    int dashTimer;
    int isDashing;
    float dashDirX;
    float dashDirY;
    
    // 보스 이동
    int moveTimer;
    float moveDirX;
    float moveDirY;

    // 보스 스킬(점프)
    int isJumping;
    int jumpTimer;
    float jumpDirX;
    float jumpDirY;
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

typedef struct {
    float targetX, targetY;  // 착지 예정 월드 좌표
    int   isActive;
    int   timer;              // 남은 경고 프레임
} JUMP_WARNING;

extern BOSS boss;
extern BOSS_PAW bossPaws[BOSS_PAW_LIMIT];
extern DASH_WARNING dashWarn;
extern JUMP_WARNING jumpWarn;

void InitBoss();
void SpawnBoss(MAP_TYPE type);
void UpdateBoss();