#pragma once
#include "object.h"

#define ENEMY_LIMIT 50

typedef struct {
	OBJECT base;
	int isActive;
} ENEMY;

extern ENEMY enemies[ENEMY_LIMIT];