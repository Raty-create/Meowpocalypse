#include "camera.h"
#include "config.h"
#include "map.h"

CAMERA camera;

void UpdateCamera(float playerX, float playerY, int mapRows, int mapCols) {
	MAPDATA* m = &maps[currentMapType];

	int mapW = mapCols * TILE_SIZE;
	int mapH = mapRows * TILE_SIZE;

	// X축: 카메라가 맵보다 작다면 중앙 고정, 크면 플레이어 추적
	if (mapW <= SCREEN_WIDTH) {
		camera.x = m->worldX + (mapW - SCREEN_WIDTH) / 2.0f;
	}
	else {
		camera.x = playerX - SCREEN_WIDTH / 2;
		if (camera.x < m->worldX) camera.x = m->worldX;
		if (camera.x > m->worldX + mapW - SCREEN_WIDTH) camera.x = m->worldX + mapW - SCREEN_WIDTH;
	}

	// Y축: 카메라가 맵보다 작다면 중앙 고정, 크면 플레이어 추적
	if (mapH <= SCREEN_HEIGHT) {
		camera.y = m->worldY + (mapH - SCREEN_HEIGHT) / 2.0f;
	}
	else {
		camera.y = playerY - SCREEN_HEIGHT / 2;
		if (camera.y < m->worldY) camera.y = m->worldY;
		if (camera.y > m->worldY + mapH - SCREEN_HEIGHT) camera.y = m->worldY + mapH - SCREEN_HEIGHT;
	}
}