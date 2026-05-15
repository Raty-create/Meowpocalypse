#include <Windows.h>
#include "render.h"
#include "player.h"
#include "enemy.h"
#include "camera.h"
#include "map.h"
#include "boss.h"
#include "bullet.h"

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
	}
}

//타일 그리기
void RenderTile(HDC mDC, int screenX, int screenY, COLORREF color) {
	hBrush = CreateSolidBrush(color);
	oldBrush = (HBRUSH)SelectObject(mDC, hBrush);
	hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	oldPen = (HPEN)SelectObject(mDC, hPen);

	Rectangle(mDC, screenX, screenY, screenX + TILE_SIZE, screenY + TILE_SIZE);

	SelectObject(mDC, oldBrush);
	DeleteObject(hBrush);
	SelectObject(mDC, oldPen);
	DeleteObject(hPen);
}

// 맵 그리기
void RenderCurrentMap(HDC mDC) {
	MAPDATA* m = &maps[currentMapType];
	for (int row = 0; row < m->rows; row++) {
		for (int col = 0; col < m->cols; col++) {

			screenX = m->worldX + col * TILE_SIZE - (int)camera.x;
			screenY = m->worldY + row * TILE_SIZE - (int)camera.y;

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

			RenderTile(mDC, screenX, screenY, TileColor(tileType, doorstate));
		}
	}
}

// 플레이어
void RenderPlayer(HDC mDC) {
	// 무적 상태일 때 깜빡임 효과
	if (player.invincibleTimer > 0 && (player.invincibleTimer / 5) % 2 == 0) return;

	screenX = (int)(player.base.x - camera.x);
	screenY = (int)(player.base.y - camera.y);

	Rectangle(mDC, screenX - player.base.width / 2, screenY - player.base.height / 2, screenX + player.base.width / 2, screenY + player.base.height / 2);
}

// 플레이어 hitBox
void RenderPlayerHitBox(HDC mDC) {
	screenX = (int)(player.base.hitBoxX - camera.x);
	screenY = (int)(player.base.hitBoxY - camera.y);

	hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	oldPen = (HPEN)SelectObject(mDC, hPen);
	oldBrush = (HBRUSH)SelectObject(mDC, GetStockObject(NULL_BRUSH));

	Rectangle(mDC, screenX - player.base.hitBoxW / 2, screenY - player.base.hitBoxH / 2, screenX + player.base.hitBoxW / 2, screenY + player.base.hitBoxH / 2);

	SelectObject(mDC, oldPen);
	SelectObject(mDC, oldBrush);
	DeleteObject(hPen);
}

// 잡몹 그리기
void RenderEnemies(HDC mDC) {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		if (!enemies[i].isActive) continue;

		screenX = (int)(enemies[i].base.x - camera.x);
		screenY = (int)(enemies[i].base.y - camera.y);

		hBrush = CreateSolidBrush(enemies[i].base.state == ENEMY_CHASE ? RGB(200, 30, 0) : RGB(0, 30, 200));
		oldBrush = (HBRUSH)SelectObject(mDC, hBrush);

		Rectangle(mDC, screenX - enemies[i].base.width / 2, screenY - enemies[i].base.height / 2,
			screenX + enemies[i].base.width / 2, screenY + enemies[i].base.height / 2);

		SelectObject(mDC, oldBrush);
		DeleteObject(hBrush);
	}
}

// 잡몹 hitBox 그리기
void RenderEnemiesHitBox(HDC mDC) {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		if (!enemies[i].isActive) continue;

		screenX = (int)(enemies[i].base.hitBoxX - camera.x);
		screenY = (int)(enemies[i].base.hitBoxY - camera.y);

		hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		oldPen = (HPEN)SelectObject(mDC, hPen);
		oldBrush = (HBRUSH)SelectObject(mDC, GetStockObject(NULL_BRUSH));

		Rectangle(mDC, screenX - enemies[i].base.hitBoxW / 2, screenY - enemies[i].base.hitBoxH / 2,
			screenX + enemies[i].base.hitBoxW / 2, screenY + enemies[i].base.hitBoxH / 2);

		SelectObject(mDC, oldPen);
		SelectObject(mDC, oldBrush);
		DeleteObject(hPen);
	}
}

// 잡몹 돌던지기 그리기
void RenderCatPaw(HDC mDC) {
	hBrush = CreateSolidBrush(RGB(255, 0, 0));
	oldBrush = (HBRUSH)SelectObject(mDC, hBrush);
	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		if (!catpaw[i].isActive) continue;
		screenX = (int)(catpaw[i].x - camera.x);
		screenY = (int)(catpaw[i].y - camera.y);
		Ellipse(mDC, screenX - CAT_PAW_SIZE / 2, screenY - CAT_PAW_SIZE / 2,
			screenX + CAT_PAW_SIZE / 2, screenY + CAT_PAW_SIZE / 2);
	}
	SelectObject(mDC, oldBrush);
	DeleteObject(hBrush);
}

// 총알
void RenderBullets(HDC mDC) {
	for (int i = 0; i < BULLET_MAX; i++) {
		if (bullets[i].isActive == INACTIVE) continue;

		screenX = (int)(bullets[i].x - camera.x);
		screenY = (int)(bullets[i].y - camera.y);

		Ellipse(mDC, screenX - bullets[i].width, screenY - bullets[i].height, screenX + bullets[i].width, screenY + bullets[i].height);
	}
}