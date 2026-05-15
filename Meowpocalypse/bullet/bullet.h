#pragma once

#include <Windows.h>
#include "config.h"

typedef struct {
	float x, y;
	float dx, dy;
	int width, height;
	int damage;
	int isActive;
} BULLET;

extern BULLET bullets[BULLET_MAX];

void InitBullet();
void ShootBullet(HWND hWnd);
void UpdateBullet();