#include "config.h"
#include "game.h"
#include "player.h"
#include "render.h"
#include "camera.h"
#include "map.h"

void InitGame() {
	// 대기방
	InitWaitingRoom();
	InitFirstHallWay();
	// 플레이어
	InitPlayer();
}

void Update() {
	// 플레이어
	UpdatePlayer();
	// 카메라
	UpdateCamera(player.base.x, player.base.y, WAITINGMAP_ROWS, WAITINGMAP_COLS);
}

void Render(HDC mDC) {
	// 대기방
	RenderWaitingMap(mDC);
	RenderFirstHallWayMap(mDC);
	// 플레이어
	RenderPlayer(mDC);
}