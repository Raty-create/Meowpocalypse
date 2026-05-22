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
    int deathTimer; // 사망 애니메이션용 타이머
} ENEMY;

extern ENEMY enemies[ENEMY_LIMIT];
extern CATPAW catpaw[CAT_PAW_LIMIT];

void InitEnemy();
void ClearEnemies();
void SpawnEnemy(MAP_TYPE type, int count);
void UpdateEnemies();

// 적 업데이트 관련 내부 함수
void UpdateEnemyState(int index);
void HandleEnemyKnockback(int index);
void HandleEnemyChase(int index, float dx, float dy, float dist);
void HandleEnemyPatrol(int index);
void HandleEnemyAggro(int index, float tx, float ty);
void UpdateCatPaws();