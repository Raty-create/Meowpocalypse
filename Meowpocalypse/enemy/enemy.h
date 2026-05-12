#pragma once
#include "object.h"
#include "config.h"
#include "map.h"

typedef enum{ ENEMY_IDLE, ENEMY_CHASE, ENEMY_RANGED} ENEMY_STATE;

typedef struct {
	float x, y;
	float dx, dy;
	int isActive;
}STONE;

typedef struct {
	OBJECT base;
	int isActive;
	ENEMY_STATE state;
	int shootTimer;
} ENEMY;

extern ENEMY enemies[ENEMY_LIMIT];
extern STONE stones[STONE_LIMIT];

void InitEnemy();
void ClearEnemies();
void SpawnEnemy(MAP_TYPE type, int count);
void UpdateEnemies();