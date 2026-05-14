#include "config.h"
#include "game.h"
#include "player.h"
#include "render.h"
#include "camera.h"
#include "map.h"
#include "enemy.h"
#include "bullet.h"

void InitGame() {
	// 시작 맵 설정
	currentMapType = MAP_WAITING;
	// 맵
	InitAllMap();
	// 문
	SetDoorState(MAP_WAITING, DOOR_OPEN);
	// 플레이어
	InitPlayer();
	// 총알
	InitBullet();
	SetDoorState(MAP_WAITING, DOOR_OPEN);
	// 잡몹
	InitEnemy();
}

void Update(HWND hWnd) {
	// 플레이어
	UpdatePlayer();
	// 총알 발사
	ShootBullet(hWnd);
	// 총알
	UpdateBullet();
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
	// 총알
	RenderBullets(mDC);
	// 잡몹
	RenderEnemies(mDC);
	RenderCatPaw(mDC);
}