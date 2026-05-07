#pragma once

enum { STOP, MOVE };
typedef enum { MAP_WAITING, MAP_FIRST_HALLWAY, MAP_FIRST_BOSS, MAP_SECOND_HALLWAY, MAP_SECOND_BOSS, MAP_THIRD_HALLWAY, MAP_THIRD_BOSS };

typedef struct {
	float x, y;
	float dx, dy;
	int width, height;
	int hp;
	int state;
} OBJECT;