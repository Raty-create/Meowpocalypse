#pragma once
#include "config.h"

typedef struct {
	float x, y;
	float dx, dy;
	int width, height;
	int isActive;
} BULLET;

extern BULLET bullets[BULLET_MAX];

void InitBullet();
void ShootBullet(HWND hWnd);
void UpdateBullet();