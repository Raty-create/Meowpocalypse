#pragma once
#include "object.h"

typedef struct {
	OBJECT base;
	int type;
	int isActive;
} BOSS;

extern BOSS boss;