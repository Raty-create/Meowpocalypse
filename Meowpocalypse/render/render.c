#include "render.h"
#include "player.h"
#include "camera.h"
#include "map.h"

HBRUSH hBrush, oldBrush;
HPEN hPen, oldPen;
COLORREF color;
int screenX, screenY;

// 맵
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

void RenderWaitingMap(HDC mDC) {
	for (int row = 0; row < WAITINGMAP_ROWS; row++) {
		for (int col = 0; col < WAITINGMAP_COLS; col++) {

			screenX = col * TILE_SIZE - (int)camera.x;
			screenY = row * TILE_SIZE - (int)camera.y;

			if (screenX + TILE_SIZE < 0 || screenX > SCREEN_WIDTH) continue;
			if (screenY + TILE_SIZE < 0 || screenY > SCREEN_HEIGHT) continue;

			if (currentMap.tiles[row][col] == TILE_FLOOR) color = RGB(153, 76, 0);
			else if (currentMap.tiles[row][col] == TILE_WALL) color = RGB(0, 0, 0);
			else if (currentMap.tiles[row][col] == TILE_DOOR) color = RGB(0, 0, 255);

			RenderTile(mDC, screenX, screenY, color);
		}
	}
}

void RenderFirstHallWayMap(HDC mDC) {
	for (int row = 0; row < FIRST_HALLWAYMAP_ROWS; row++) {
		for (int col = 0; col < FIRST_HALLWAYMAP_COLS; col++) {
			screenX = FIRST_HALLWAYMAP_X + col * TILE_SIZE - (int)camera.x;
			screenY = row * TILE_SIZE - (int)camera.y;

			if (screenX + TILE_SIZE < 0 || screenX > SCREEN_WIDTH) continue;
			if (screenY + TILE_SIZE < 0 || screenY > SCREEN_HEIGHT) continue;

			if (firsthallwayMap.tiles[row][col] == TILE_WALL) color = RGB(0, 0, 0);
			else if (firsthallwayMap.tiles[row][col] == TILE_FLOOR) color = RGB(153, 76, 0);

			RenderTile(mDC, screenX, screenY, color);
		}
	}
}

// 플레이어
void RenderPlayer(HDC mDC) {
	screenX = (int)(player.base.x - camera.x);
	screenY = (int)(player.base.y - camera.y);

	Rectangle(mDC, screenX - player.base.width / 2, screenY - player.base.height / 2, screenX + player.base.width / 2, screenY + player.base.height / 2);
}