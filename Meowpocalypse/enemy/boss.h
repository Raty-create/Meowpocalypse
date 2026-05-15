#pragma once
#include "object.h"

typedef struct {
    OBJECT base;
    int isActive;
} BOSS;

extern BOSS boss;

void InitBoss();
void SpawnBoss(MAP_TYPE type);