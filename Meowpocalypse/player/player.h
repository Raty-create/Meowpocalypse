#pragma once

#include "object.h"

typedef struct {
	OBJECT base;
	int mp;
	int invincibleTimer;
} PLAYER;

extern PLAYER player;

void InitPlayer();
void UpdatePlayer();
void MapTransition();