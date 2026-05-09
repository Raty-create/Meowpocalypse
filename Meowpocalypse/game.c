#include "config.h"
#include "game.h"
#include "player.h"
#include "render.h"
#include "camera.h"
#include "map.h"

void InitGame() {
	// 시작 맵 설정
	currentMapType = MAP_WAITING;
	// 맵
	InitAllMap();
	// 플레이어
	InitPlayer();
}

void Update() {
	// 플레이어
	UpdatePlayer();
	// 카메라(현재 맵 크기를 카메라로 전달)
	MAPDATA* m = &maps[currentMapType];
	UpdateCamera(player.base.x, player.base.y, m->rows, m->cols);
}

void Render(HDC mDC) {
	// 현재 맵만 렌더링
	RenderCurrentMap(mDC);
	// 플레이어
	RenderPlayer(mDC);
}