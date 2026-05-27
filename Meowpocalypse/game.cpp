#include "config.h"
#include "game.h"
#include "player.h"
#include "render.h"
#include "camera.h"
#include "map.h"
#include "enemy.h"
#include "bullet.h"
#include "boss.h"
#include "enum.h"
#include "input.h"

int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;

void InitGame() {
	currentMapType = MAP_WAITING;				// 시작 맵 설정
	InitAllMap();								// 맵
	SetDoorState(MAP_WAITING, DOOR_OPEN);		// 문
	InitRenderResources();						// 렌더링 리소스(그림자)
	InitPlayer();								// 플레이어
	InitEnemy();								// 잡몹
	InitBoss();									// 보스
	InitBullet();								// 총알
	InitChuru();								// 츄르
}

void ReleaseGame() {
	ReleasePlayer();
	ReleaseBullet();
	ReleaseEnemy();
}

void Update(HWND hWnd) {
	UpdateInput(hWnd);				// 입력 업데이트
	UpdatePlayer();					// 플레이어 업데이트
	ShootSkillQ();					// Q 스킬 - 총알 세 방향 발사
	UpdateEnemies();				// 잡몹 업데이트
	SpawnBoss(currentMapType);		// 보스 스폰
	UpdateBoss();					// 보스 업데이트
	UpdateBullet();					// 총알 업데이트
	ShootBullet();					// 총알 발사
	UpdateChuru();					// 츄르 업데이트
	ShootSkillR();					// R 스킬 - 츄르 던지기
	MAPDATA* m = &maps[currentMapType];									// 카메라(현재 맵 크기를 카메라로 전달)
	UpdateCamera(player.base.x, player.base.y, m->rows, m->cols);		// 카메라 업데이트
}

void Render(HWND hWnd, HDC hDC) {
	RECT rt;
	GetClientRect(hWnd, &rt);

	HDC hMemDC = CreateCompatibleDC(hDC);
	HBITMAP hMemBitmap = CreateCompatibleBitmap(hDC, rt.right, rt.bottom);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hMemBitmap);

	RenderCurrentMap(hMemDC);				// 현재 맵만 렌더링

	// 그림자
	RenderObjectShadow(hMemDC, player.base.x, player.base.y, player.base.width * 3);
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		if (enemies[i].isActive)
			RenderObjectShadow(hMemDC, enemies[i].base.x, enemies[i].base.y, enemies[i].base.width);
	}

	RenderPlayer(hMemDC);					// 플레이어
	RenderPlayerHitBox(hMemDC);				// 플레이어 히트박스

	RenderEnemies(hMemDC);					// 잡몹
	RenderEnemiesHitBox(hMemDC);			// 잡몹 히트박스
	RenderCatPaw(hMemDC);					// 잡몹 젤리

	RenderBoss(hMemDC);						// 보스
	RenderBossHitBox(hMemDC);				// 보스 히트박스
	RenderBossPaws(hMemDC);					// 보스 젤리

	RenderBullets(hMemDC);					// 총알
	RenderChuru(hMemDC);					// 츄르

	BitBlt(hDC, 0, 0, rt.right, rt.bottom, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC, hOldBitmap);
	DeleteDC(hMemDC);
	DeleteObject(hMemBitmap);
}