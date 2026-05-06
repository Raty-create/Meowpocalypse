#pragma once
#include "object.h"
#include "config.h"

typedef struct {
	OBJECT base;
	int isActive;
} ENEMY;

extern ENEMY enemies[ENEMY_LIMIT];