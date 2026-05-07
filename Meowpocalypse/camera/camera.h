#pragma once

typedef struct {
	float x, y;
	int width, height;
} CAMERA;

extern CAMERA camera;

void UpdateCamera(float plyerX, float playerY, int mapRows, int mapCols);