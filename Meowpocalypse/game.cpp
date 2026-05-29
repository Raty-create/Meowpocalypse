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

// 전역 렌더링 리소스
HDC g_hGameDC = NULL;
HBITMAP g_hGameBitmap = NULL;
HBITMAP g_hOldGameBitmap = NULL;

void InitGame() {
	currentMapType = MAP_WAITING;				// 시작 맵 설정

	camera.zoom = 1.0f;
	camera.isIntroActive = INACTIVE;
	camera.introTimer = 0;

	// 가상 해상도용 DC 및 비트맵 초기화 (1920x1080 고정)
	HDC hDC = GetDC(NULL);
	g_hGameDC = CreateCompatibleDC(hDC);
	g_hGameBitmap = CreateCompatibleBitmap(hDC, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_hOldGameBitmap = (HBITMAP)SelectObject(g_hGameDC, g_hGameBitmap);
	ReleaseDC(NULL, hDC);

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
	if (g_hGameDC) {
		SelectObject(g_hGameDC, g_hOldGameBitmap);
		DeleteObject(g_hGameBitmap);
		DeleteDC(g_hGameDC);
	}

	ReleasePlayer();
	ReleaseBullet();
	ReleaseEnemy();
}

void Update(HWND hWnd) {
	UpdateInput(hWnd);				// 입력 업데이트

	// 카메라 인트로 중에는 플레이어가 움직이지 못하게 막거나 업데이트를 제한
	if (camera.isIntroActive == INACTIVE) {
		UpdatePlayer();				// 플레이어 업데이트
		ShootSkillQ();				// Q 스킬 - 총알 세 방향 발사
		ShootBullet();				// 총알 발사
		ShootSkillR();				// R 스킬 - 츄르 던지기
	}

	UpdateEnemies();				// 잡몹 업데이트
	SpawnBoss(currentMapType);		// 보스 스폰
	UpdateBoss();					// 보스 업데이트
	UpdateBullet();					// 총알 업데이트
	UpdateChuru();					// 츄르 업데이트
	MAPDATA* m = &maps[currentMapType];									// 카메라(현재 맵 크기를 카메라로 전달)
	UpdateCamera(player.base.x, player.base.y, m->rows, m->cols);		// 카메라 업데이트
}

void Render(HWND hWnd, HDC hDC) {
	RECT rt;
	GetClientRect(hWnd, &rt);
	int winW = rt.right;
	int winH = rt.bottom;

	if (winW <= 0 || winH <= 0) return;

	// 모든 게임 그래픽을 가상 DC(g_hGameDC, 1920x1080)에 먼저 그림
	RenderCurrentMap(g_hGameDC);

	if (camera.isIntroActive == INACTIVE) {
		RenderObjectShadow(g_hGameDC, player.base.x, player.base.y, player.base.width * 3);
		for (int i = 0; i < ENEMY_LIMIT; i++) {
			if (enemies[i].isActive)
				RenderObjectShadow(g_hGameDC, enemies[i].base.x, enemies[i].base.y, enemies[i].base.width);
		}
		for (int i = 0; i < CHURU_MAX; i++) {
			if (churues[i].isActive)
				RenderObjectShadow(g_hGameDC, churues[i].x, churues[i].y, (int)((float)churues[i].width * 1.6f));
		}
	}

	RenderPlayer(g_hGameDC);
	RenderPlayerHitBox(g_hGameDC);

	RenderEnemies(g_hGameDC);
	RenderEnemiesHitBox(g_hGameDC);
	RenderCatPaw(g_hGameDC);
	RenderCatPawHitBox(g_hGameDC);

	RenderBoss(g_hGameDC);
	RenderBossHitBox(g_hGameDC);
	RenderBossPaws(g_hGameDC);

	RenderChuru(g_hGameDC);

	RenderBullets(g_hGameDC);
	RenderBulletsHitBox(g_hGameDC);

	// 화면 비율 계산 (16:9 기준 레터박스)
	float scaleX = (float)winW / SCREEN_WIDTH;
	float scaleY = (float)winH / SCREEN_HEIGHT;
	float scale = MIN(scaleX, scaleY);

	int destW = (int)(SCREEN_WIDTH * scale);
	int destH = (int)(SCREEN_HEIGHT * scale);
	int destX = (winW - destW) / 2;
	int destY = (winH - destH) / 2;

	// 최종 출력을 위한 임시 메모리 DC (창 크기)
	HDC hMemDC = CreateCompatibleDC(hDC);
	HBITMAP hMemBitmap = CreateCompatibleBitmap(hDC, winW, winH);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hMemBitmap);

	// 레터박스 영역을 검은색으로 채우기
	HBRUSH hBlack = CreateSolidBrush(RGB(0, 0, 0));
	FillRect(hMemDC, &rt, hBlack);
	DeleteObject(hBlack);

	// 가상 화면을 실제 윈도우 크기에 맞춰 중앙에 확대/축소 복사
	StretchBlt(hMemDC, destX, destY, destW, destH, g_hGameDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SRCCOPY);

	// 완성된 이미지가 모여있는 메모리 DC(hMemDC)를 실제 화면(hDC)으로 고속 복사
	BitBlt(hDC, 0, 0, winW, winH, hMemDC, 0, 0, SRCCOPY);

	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hMemBitmap);
	DeleteDC(hMemDC);
}