#include "camera.h"
#include "config.h"

CAMERA camera;

void UpdateCamera(float playerX, float playerY, int mapRows, int mapCols) {
	camera.x = playerX - SCREEN_WIDTH / 2;
	camera.y = playerY - SCREEN_HEIGHT / 2;

	int mapW = mapCols * TILE_SIZE;
	int mapH = mapRows * TILE_SIZE;

	if (camera.x < 0) camera.x = 0;
	if (camera.x > mapW - SCREEN_WIDTH) camera.x = mapW - SCREEN_WIDTH;
	if (camera.y < 0) camera.y = 0;
	if (camera.y > mapH - SCREEN_HEIGHT) camera.y = mapH - SCREEN_HEIGHT;
}