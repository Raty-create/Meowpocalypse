#include "config.h"
#include "game.h"
#include "player.h"
#include "render.h"
#include "camera.h"
#include "map.h"
#include "enemy.h"

void InitGame() {
	// 맵
	InitAllMap();
	// 플레이어
	InitPlayer();
	SetDoorState(MAP_WAITING, DOOR_OPEN);
	// 잡몹
	InitEnemy();
}

void Update() {
	// 플레이어
	UpdatePlayer();
	// 잡몹
	UpdateEnemies();
	// 카메라(현재 맵 크기를 카메라로 전달)
	MAPDATA* m = &maps[currentMapType];
	UpdateCamera(player.base.x, player.base.y, m->rows, m->cols);
}

void Render(HDC mDC) {
	// 현재 맵만 렌더링
	RenderCurrentMap(mDC);
	// 플레이어
	RenderPlayer(mDC);
	// 잡몹
	RenderEnemies(mDC);
	RenderStones(mDC);
}