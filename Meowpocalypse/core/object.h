#pragma once

enum { STOP, MOVE };

typedef struct {
	float x, y;
	float dx, dy;
	int width, height;
	int hp;
	int state;
} OBJECT;