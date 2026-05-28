#include <Windows.h>
#include "render.h"
#include "player.h"
#include "enemy.h"
#include "camera.h"
#include "map.h"
#include "boss.h"
#include "bullet.h"
#include "enum.h"

IMAGE imgShadow;

void InitRenderResources() {
	LoadMyImage(&imgShadow, L"DEFAULT_SHADOW.png");
}

HBRUSH hBrush, oldBrush;
HPEN hPen, oldPen;
COLORREF color;
int screenX, screenY;
POINT pt;

//타일 색상
COLORREF TileColor(int tileType, DOOR_STATE doorState) {
	switch (tileType) {
	case TILE_FLOOR: return RGB(153, 76, 0);
	case TILE_WALL: return RGB(0, 0, 0);
	case TILE_DOOR: return(doorState == DOOR_OPEN) ? RGB(0, 200, 0) : RGB(139, 69, 0);
	default: return RGB(0, 0, 0);
	}
}

//타일 그리기
void RenderTile(HDC hDC, int screenX, int screenY, COLORREF color) {
	hBrush = CreateSolidBrush(color);
	oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
	hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	oldPen = (HPEN)SelectObject(hDC, hPen);

	Rectangle(hDC, screenX, screenY, screenX + TILE_SIZE, screenY + TILE_SIZE);

	SelectObject(hDC, oldBrush);
	DeleteObject(hBrush);
	SelectObject(hDC, oldPen);
	DeleteObject(hPen);
}

// 맵 그리기
void RenderCurrentMap(HDC hDC) {
	MAPDATA* m = &maps[currentMapType];
	for (int row = 0; row < m->rows; row++) {
		for (int col = 0; col < m->cols; col++) {

			screenX = (int)(m->worldX + col * TILE_SIZE - camera.x);
			screenY = (int)(m->worldY + row * TILE_SIZE - camera.y);

			if (screenX + TILE_SIZE < 0 || screenX > SCREEN_WIDTH) continue;
			if (screenY + TILE_SIZE < 0 || screenY > SCREEN_HEIGHT) continue;

			int tileType = m->tiles[row][col];
			DOOR_STATE doorstate = DOOR_CLOSE;

			if (tileType == TILE_DOOR) {
				for (int d = 0; d < m->doorCount; d++) {
					if (m->doors[d].row == row && m->doors[d].col == col) {
						doorstate = m->doors[d].state;
						break;
					}
				}
			}

			RenderTile(hDC, screenX, screenY, TileColor(tileType, doorstate));
		}
	}
}

// 그림자 공용 함수
void RenderObjectShadow(HDC hDC, float x, float y, int objW) {
	if (imgShadow.img.IsNull()) return;

	int shadowX = (int)(x - camera.x);
	int shadowY = (int)(y - camera.y) + (objW / 3);		// 발밑 위치 오프셋

	int sw = (int)(objW * 0.6f);
	int sh = sw;

	DrawMyImage(&imgShadow, hDC, shadowX - sw / 2, shadowY - sh / 2, sw, sh, 0, 0, imgShadow.width, imgShadow.height);
}

// 플레이어
void RenderPlayer(HDC hDC) {
	// 무적 상태일 때 깜빡임 효과
	if (player.invincibleTimer > 0 && (player.invincibleTimer / 5) % 2 == 0) return;

	screenX = (int)(player.base.x - camera.x);
	screenY = (int)(player.base.y - camera.y);

	int finalRow = 0;

	switch (player.base.state) {
	case PLAYER_IDLE:
		finalRow = 0;
		break;
	case PLAYER_MOVE:
		switch (player.base.direction) {
		case DIR_DOWN:
			finalRow = 1;
			break;
		case DIR_UP:
			finalRow = 2;
			break;
		case DIR_LEFT:
		case DIR_UP_LEFT:
		case DIR_DOWN_LEFT:
			finalRow = 3;
			break;
		case DIR_RIGHT:
		case DIR_UP_RIGHT:
		case DIR_DOWN_RIGHT:
			finalRow = 4;
			break;
		}
		break;
	case PLAYER_HIT:
		finalRow = 5 + (int)player.base.direction;
		break;
	case PLAYER_SHOOT:
		finalRow = 13 + (int)player.base.direction;
		break;
	case PLAYER_DEAD:
		finalRow = 21 + (int)player.base.direction;
		break;
	default:
		finalRow = 0;
		break;
	}

	RenderAnimation(&player.anim, hDC, screenX, screenY, player.base.width * 6, (int)((float)player.base.height * 6.3f), finalRow);
}

// 플레이어 hitBox
void RenderPlayerHitBox(HDC hDC) {
	screenX = (int)(player.base.hitBoxX - camera.x);
	screenY = (int)(player.base.hitBoxY - camera.y);

	hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	oldPen = (HPEN)SelectObject(hDC, hPen);
	oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

	Rectangle(hDC, screenX - player.base.hitBoxW / 2, screenY - player.base.hitBoxH / 2, screenX + player.base.hitBoxW / 2, screenY + player.base.hitBoxH / 2);

	SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);
	DeleteObject(hPen);
}

// 잡몹 그리기
void RenderEnemies(HDC hDC) {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		if (!enemies[i].isActive) continue;

		screenX = (int)(enemies[i].base.x - camera.x);
		screenY = (int)(enemies[i].base.y - camera.y);

		// 애니메이션 행 세트 결정 (각 세트는 8줄씩 차지)
		int baseRow = 0;
		BOOL isDeadState = FALSE; 
		switch (enemies[i].base.state) {
		case ENEMY_IDLE:
		case ENEMY_MOVE:
		case ENEMY_CHASE:
		case ENEMY_AGGRO:
			baseRow = 0;	// 대기/이동 세트 (0~7)
			break;
		case ENEMY_RANGED:
			baseRow = 8;	// 총 쏘기 세트 (8~15)
			break;
		case ENEMY_HIT:
			baseRow = 16;	// 피격 세트 (16~23)
			break;
		case ENEMY_MELEE:
			baseRow = 24;	// 근접 공격 세트 (24~31)
			break;
		case ENEMY_DEAD:
			baseRow = 32;	// 사망 세트 (32) - 방향 무시
			isDeadState = TRUE;
			break;
		default:
			baseRow = 0;
			break;
		}

		// 방향에 따른 오프셋 (사망 상태면 오프셋 0 고정)
		int dirOffset = 0;
		if (!isDeadState) {
			switch (enemies[i].base.direction) {
			case DIR_DOWN:       dirOffset = 0; break;
			case DIR_UP:         dirOffset = 1; break;
			case DIR_LEFT:       dirOffset = 2; break;
			case DIR_RIGHT:      dirOffset = 3; break;
			case DIR_UP_LEFT:    dirOffset = 4; break;
			case DIR_UP_RIGHT:   dirOffset = 5; break;
			case DIR_DOWN_LEFT:  dirOffset = 6; break;
			case DIR_DOWN_RIGHT: dirOffset = 7; break;
			default:             dirOffset = 0; break;
			}
		}

		int finalRow = baseRow + dirOffset;

		RenderAnimation(&enemies[i].anim, hDC, screenX, screenY, enemies[i].base.width, enemies[i].base.height, finalRow);
	}
}

// 잡몹 hitBox 그리기
void RenderEnemiesHitBox(HDC hDC) {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		if (!enemies[i].isActive) continue;

		screenX = (int)(enemies[i].base.hitBoxX - camera.x);
		screenY = (int)(enemies[i].base.hitBoxY - camera.y);

		hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		oldPen = (HPEN)SelectObject(hDC, hPen);
		oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

		Rectangle(hDC, screenX - enemies[i].base.hitBoxW / 2, screenY - enemies[i].base.hitBoxH / 3,
			screenX + enemies[i].base.hitBoxW / 2, screenY + enemies[i].base.hitBoxH / 2);

		SelectObject(hDC, oldPen);
		SelectObject(hDC, oldBrush);
		DeleteObject(hPen);
	}
}

// 잡몹 젤리
void RenderCatPaw(HDC hDC) {
	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		if (!catpaw[i].isActive) continue;

		screenX = (int)(catpaw[i].x - camera.x);
		screenY = (int)(catpaw[i].y - camera.y);

		RenderAnimation(&catpaw[i].anim, hDC, screenX, screenY, catpaw[i].width, catpaw[i].height, catpaw[i].dirRow);
	}
}

// 잡몹 젤리 hitBox
void RenderCatPawHitBox(HDC hDC) {
	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		if (!catpaw[i].isActive) continue;

		screenX = (int)(catpaw[i].hitBoxX - camera.x);
		screenY = (int)(catpaw[i].hitBoxY - camera.y);

		hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		oldPen = (HPEN)SelectObject(hDC, hPen);
		oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

		Rectangle(hDC, screenX - catpaw[i].hitBoxW / 2, screenY - catpaw[i].hitBoxH / 2,
			screenX + catpaw[i].hitBoxW / 2, screenY + catpaw[i].hitBoxH / 2);

		SelectObject(hDC, oldPen);
		SelectObject(hDC, oldBrush);
		DeleteObject(hPen);
	}
}

//보스 대시 경고 - Polygon으로 경고 영역 한 번에 그리기
void RenderDashWarning(HDC hDC) {
	if (dashWarn.isActive == INACTIVE) return;

	// 깜빡임: 10프레임 단위로 켜짐/꺼짐
	if ((dashWarn.timer / 10) % 2 == 0) return;

	int half = BOSS_SIZE / 2;

	// 경고 영역의 꼭짓점 4개 계산 (월드 → 화면 좌표)
	// 끝점 중심
	float ex = dashWarn.startX + dashWarn.dirX * dashWarn.stopDist;
	float ey = dashWarn.startY + dashWarn.dirY * dashWarn.stopDist;

	POINT pts[4];
	// P0: 시작 왼쪽 위
	pts[0].x = (int)(dashWarn.startX + dashWarn.perpX * half - camera.x);
	pts[0].y = (int)(dashWarn.startY + dashWarn.perpY * half - camera.y);
	// P1: 끝   왼쪽 위
	pts[1].x = (int)(ex + dashWarn.perpX * half - camera.x);
	pts[1].y = (int)(ey + dashWarn.perpY * half - camera.y);
	// P2: 끝   오른쪽 아래
	pts[2].x = (int)(ex - dashWarn.perpX * half - camera.x);
	pts[2].y = (int)(ey - dashWarn.perpY * half - camera.y);
	// P3: 시작 오른쪽 아래
	pts[3].x = (int)(dashWarn.startX - dashWarn.perpX * half - camera.x);
	pts[3].y = (int)(dashWarn.startY - dashWarn.perpY * half - camera.y);

	hBrush = CreateSolidBrush(RGB(255, 40, 40));
	oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
	hPen = CreatePen(PS_NULL, 0, 0); // 테두리 없음
	oldPen = (HPEN)SelectObject(hDC, hPen);

	Polygon(hDC, pts, 4); // 꼭짓점 4개로 경고 영역 한 번에 그리기

	SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);
	DeleteObject(hPen);
	DeleteObject(hBrush);
}

// 보스 점프 착지 경고 - 원형 표시
void RenderJumpWarning(HDC hDC) {
	if (jumpWarn.isActive == INACTIVE) return;

	// 깜빡임: 10프레임 단위
	if ((jumpWarn.timer / 10) % 2 == 0) return;

	int r = BOSS_JUMP_LAND_SIZE / 2;
	int sx = (int)(jumpWarn.targetX - camera.x);
	int sy = (int)(jumpWarn.targetY - camera.y);

	hBrush = CreateSolidBrush(RGB(255, 40, 40));
	oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
	hPen = CreatePen(PS_NULL, 0, 0);
	oldPen = (HPEN)SelectObject(hDC, hPen);

	Ellipse(hDC, sx - r, sy - r, sx + r, sy + r);

	SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);
	DeleteObject(hPen);
	DeleteObject(hBrush);
}

// 보스
void RenderBoss(HDC hDC) {
	if (!boss.isActive) return;

	RenderDashWarning(hDC);
	RenderJumpWarning(hDC);

	if (boss.isJumping == ACTIVE) return;

	COLORREF bossColor = boss.isDashing ? RGB(255, 220, 0) : RGB(0, 255, 0);
	hBrush = CreateSolidBrush(bossColor);
	oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
	
	screenX = (int)(boss.base.x - camera.x);
	screenY = (int)(boss.base.y - camera.y);

	Rectangle(hDC, screenX - BOSS_SIZE / 2, screenY - BOSS_SIZE / 2, screenX + BOSS_SIZE / 2, screenY + BOSS_SIZE / 2);

	SelectObject(hDC, oldBrush);
	DeleteObject(hBrush);
}

void RenderBossHitBox(HDC hDC) {

	if (!boss.isActive) return;
	if (boss.isJumping == ACTIVE) return;

	screenX = (int)(boss.base.hitBoxX - camera.x);
	screenY = (int)(boss.base.hitBoxY - camera.y);

	hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	oldPen = (HPEN)SelectObject(hDC, hPen);
	oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

	Rectangle(hDC, screenX - boss.base.hitBoxW / 2, screenY - boss.base.hitBoxH / 2, screenX + boss.base.hitBoxW / 2, screenY + boss.base.hitBoxH / 2);

	SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);
	DeleteObject(hPen);
}

//보스 CAT_PAW
void RenderBossPaws(HDC hDC) {
	hBrush = CreateSolidBrush(RGB(255, 140, 0));  // 주황색으로 잡몹 PAW와 구분
	oldBrush = (HBRUSH)SelectObject(hDC, hBrush);

	for (int i = 0; i < BOSS_PAW_LIMIT; i++) {
		if (bossPaws[i].isActive == INACTIVE) continue;
		screenX = (int)(bossPaws[i].x - camera.x);
		screenY = (int)(bossPaws[i].y - camera.y);
		Ellipse(hDC,
			screenX - BOSS_PAW_SIZE / 2, screenY - BOSS_PAW_SIZE / 2,
			screenX + BOSS_PAW_SIZE / 2, screenY + BOSS_PAW_SIZE / 2);
	}
	SelectObject(hDC, oldBrush);
	DeleteObject(hBrush);
}

// 총알
void RenderBullets(HDC hDC) {
	for (int i = 0; i < BULLET_MAX; i++) {
		if (bullets[i].isActive == INACTIVE) continue;

		screenX = (int)(bullets[i].x - camera.x);
		screenY = (int)(bullets[i].y - camera.y);

		RenderAnimation(&bullets[i].anim, hDC, screenX, screenY, bullets[i].width, bullets[i].height, bullets[i].dirRow);
	}
}

// 총알 hitBox
void RenderBulletsHitBox(HDC hDC) {
	for (int i = 0; i < BULLET_MAX; i++) {
		if (!bullets[i].isActive) continue;

		screenX = (int)(bullets[i].hitBoxX - camera.x);
		screenY = (int)(bullets[i].hitBoxY - camera.y);

		hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		oldPen = (HPEN)SelectObject(hDC, hPen);
		oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

		Rectangle(hDC, screenX - bullets[i].hitBoxW / 2, screenY - bullets[i].hitBoxH / 2,
			screenX + bullets[i].hitBoxW / 2, screenY + bullets[i].hitBoxH / 2);

		SelectObject(hDC, oldPen);
		SelectObject(hDC, oldBrush);
		DeleteObject(hPen);
	}
}

// 츄르
void RenderChuru(HDC hDC) {
	for (int i = 0; i < CHURU_MAX; i++) {
		if (churues[i].isActive == INACTIVE) continue;

		screenX = (int)(churues[i].x - camera.x);
		screenY = (int)(churues[i].y - camera.y);

		RenderAnimation(&churues[i].anim, hDC, screenX, screenY, churues[i].width, churues[i].height, churues[i].dirRow);
	}
}