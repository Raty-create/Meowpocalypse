#pragma once
#include "object.h"
#include "config.h"
#include "map.h"

typedef struct {
	OBJECT base;
	int isActive;
} ENEMY;

extern ENEMY enemies[ENEMY_LIMIT];

void InitEnemy();
void SpawnEnemy(MAP_TYPE type, int count);