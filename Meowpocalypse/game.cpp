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
#include "ui.h"

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
	InitUI();									// UI
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
	ReleaseUI();
}

void Update(HWND hWnd) {
	UpdateInput(hWnd);				// 입력 업데이트

	// Fade Out - Fade In 효과 업데이트 (상태와 상관없이 매 프레임 수행)
	if (g_UI.isFadeOut) {
		g_UI.fadeAlpha += 0.02f;
		if (g_UI.fadeAlpha >= 1.0f) {
			g_UI.fadeAlpha = 1.0f;
			g_UI.isFadeOut = FALSE;
			g_UI.isFadeIn = TRUE;
			g_UI.gameState = INGAME;
			ReleaseTitle();
		}
	}
	else if (g_UI.isFadeIn) {
		g_UI.fadeAlpha -= 0.02f;
		if (g_UI.fadeAlpha <= 0.0f) {
			g_UI.fadeAlpha = 0.0f;
			g_UI.isFadeIn = FALSE;
		}
	}

	if (g_UI.gameState == TITLE) {
		UpdateTitle(hWnd);
	}
	else if (g_UI.gameState == PAUSE) {
		UpdatePause(hWnd);
	}
	else if (g_UI.gameState == KEY_GUIDE) {
		UpdateKeyGuide(hWnd);
	}
	else if (g_UI.gameState == INGAME) {
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
}

void Render(HWND hWnd, HDC hDC) {
	RECT rt;
	GetClientRect(hWnd, &rt);
	int winW = rt.right;
	int winH = rt.bottom;

	if (winW <= 0 || winH <= 0) return;

	// 프레임 시작 시 가상 DC(g_hGameDC)를 검은색으로 초기화하여 이전 프레임 잔상 방지
	RECT screenRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	FillRect(g_hGameDC, &screenRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

	// 타이틀 화면 렌더링
	if (g_UI.gameState == TITLE) {
		RenderUI(g_hGameDC);
	}
	// 인게임 렌더링
	else if (g_UI.gameState == INGAME || g_UI.gameState == PAUSE || g_UI.gameState == KEY_GUIDE) {
		// 모든 게임 그래픽을 가상 DC(g_hGameDC, 1920x1080 기준)에 먼저 그림
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

		RenderPlayer(g_hGameDC);								// 플레이어
		RenderPlayerHitBox(g_hGameDC);							// 플레이어 hitBox

		RenderEnemies(g_hGameDC);								// 잡몹
		RenderEnemiesHitBox(g_hGameDC);							// 잡몹 hitBox
		RenderCatPaw(g_hGameDC);								// 잡몹 젤리
		RenderCatPawHitBox(g_hGameDC);							// 잡몹 젤리 hitBox

		RenderBoss(g_hGameDC);									// 보스
		RenderBossHitBox(g_hGameDC);							// 보스 hitBox
		RenderBossPaws(g_hGameDC);								// 보스 젤리

		RenderChuru(g_hGameDC);									// 츄르

		RenderBullets(g_hGameDC);								// 총알
		RenderBulletsHitBox(g_hGameDC);							// 총알 hitBox

		if (camera.isIntroActive == INACTIVE) {
			RenderUI(g_hGameDC);								// UI
		}
	}

	// Fade Out - Fade In (타이틀 -> 인게임)
	if (g_UI.isFadeOut || g_UI.isFadeIn || g_UI.fadeAlpha > 0.0f) {
		RenderFadeEffect(g_hGameDC);
	}

	// 화면 비율 계산 (16:9 기준 레터박스)
	float scaleX = (float)winW / SCREEN_WIDTH;
	float scaleY = (float)winH / SCREEN_HEIGHT;
	float scale = MIN(scaleX, scaleY);

	int destW = (int)(SCREEN_WIDTH * scale);
	int destH = (int)(SCREEN_HEIGHT * scale);
	int destX = (winW - destW) / 2;
	int destY = (winH - destH) / 2;

	// 가상 화면을 실제 화면 해상도에 맞춰 출력
	StretchBlt(hDC, destX, destY, destW, destH, g_hGameDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SRCCOPY);

	// [깜빡임 방지] 전체를 지우는 대신, 게임 화면이 그려지지 않는 '레터박스' 영역만 검은색으로 채움
	HBRUSH hBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);
	
	// 위쪽 바
	RECT rtTop = { 0, 0, winW, destY };
	FillRect(hDC, &rtTop, hBlack);
	// 아래쪽 바
	RECT rtBottom = { 0, destY + destH, winW, winH };
	FillRect(hDC, &rtBottom, hBlack);
	// 왼쪽 바
	RECT rtLeft = { 0, destY, destX, destY + destH };
	FillRect(hDC, &rtLeft, hBlack);
	// 오른쪽 바
	RECT rtRight = { destX + destW, destY, winW, destY + destH };
	FillRect(hDC, &rtRight, hBlack);
}