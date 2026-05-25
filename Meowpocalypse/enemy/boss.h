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
    
    // 보스 스킬 (회오리)
    int spiralTimer;       // 회오리 PAW 발사 간격 타이머
    int spiralIndex;       // 현재 회오리 발사 인덱스 (0 ~ 총 발사 수)
    int isSpiralActive;    // 회오리 발사 중 여부

    // 보스 스킬 (연속 대쉬)
    int doubleDashPhase;   // 0=대시 안 함, 1=첫 번째 대시 완료 대기, 2=두 번째 대시 중
    int doubleDashDelay;   // 두 번째 대시 시작 전 딜레이 타이머

    // PAW 발사 중 이동 정지
    int isAttacking;       // 1이면 PAW 발사 직후 정지 중
    int attackEndTimer;    // 0이 되면 이동 재개
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