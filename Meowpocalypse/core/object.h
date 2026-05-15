#pragma once

typedef struct {
	float x, y;
	float dx, dy;
	int width, height;
	float hitBoxX, hitBoxY;
	int hitBoxW, hitBoxH;
	int hp;
	int state;
	float kx, ky; // knockback direction/force
	int kTimer;   // knockback timer
} OBJECT;