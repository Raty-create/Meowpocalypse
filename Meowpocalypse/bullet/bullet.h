#pragma once
#include "config.h"

typedef enum { INACTIVE, ACTIVE } BULLET_STATE;

typedef struct {
	float x, y;
	float dx, dy;
	int width, height;
	BULLET_STATE isActive;
} BULLET;

extern BULLET bullets[BULLET_MAX];

void InitBullet();
void ShootBullet(HWND hWnd);
void UpdateBullet();