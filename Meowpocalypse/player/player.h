#pragma once
#include "object.h"

typedef struct {
	OBJECT base;
	int mp;
} PLAYER;

extern PLAYER player;
