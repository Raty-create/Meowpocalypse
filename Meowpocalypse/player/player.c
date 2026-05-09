#include <Windows.h>
#include <math.h>

#include "player.h"
#include "config.h"
#include "map.h"

PLAYER player;

// 초기 설정
void InitPlayer() {
	player.base.x = SCREEN_WIDTH / 2;
	player.base.y = SCREEN_HEIGHT / 2;
	player.base.width = PLAYER_SIZE;
	player.base.height = PLAYER_SIZE;
	player.base.hp = 200;
	player.mp = 100;
	player.base.state = STOP;
}

// 플레이어 - 벽 충돌 체크
int IsTileWall(float x, float y) {
	MAPDATA* m = &maps[currentMapType];

	int col = (int)((x - m->worldX) / TILE_SIZE);
	int row = (int)((y - m->worldY) / TILE_SIZE);

	if (row < 0 || row >= m->rows) return 1;
	if (col < 0 || col >= m->cols) return 1;

	return m->tiles[row][col] == TILE_WALL;
}

// 플레이어 업데이트
void UpdatePlayer() {
	player.base.dx = 0;
	player.base.dy = 0;

	if (GetAsyncKeyState('a') || GetAsyncKeyState('A')) player.base.dx = -2.8f;
	if (GetAsyncKeyState('d') || GetAsyncKeyState('D')) player.base.dx = 2.8f;
	if (GetAsyncKeyState('w') || GetAsyncKeyState('W')) player.base.dy = -2.8f;
	if (GetAsyncKeyState('s') || GetAsyncKeyState('S')) player.base.dy = 2.8f;

	float playerNextX = player.base.x + player.base.dx;
	float playerNextY = player.base.y + player.base.dy;

	int playerSizeHalf = PLAYER_SIZE / 2;
	if (!IsTileWall(playerNextX - playerSizeHalf, player.base.y - playerSizeHalf) &&
		!IsTileWall(playerNextX + playerSizeHalf, player.base.y - playerSizeHalf) &&
		!IsTileWall(playerNextX - playerSizeHalf, player.base.y + playerSizeHalf) &&
		!IsTileWall(playerNextX + playerSizeHalf, player.base.y + playerSizeHalf))
		player.base.x = playerNextX;

	if (!IsTileWall(player.base.x - playerSizeHalf, playerNextY - playerSizeHalf) &&
		!IsTileWall(player.base.x + playerSizeHalf, playerNextY - playerSizeHalf) &&
		!IsTileWall(player.base.x - playerSizeHalf, playerNextY + playerSizeHalf) &&
		!IsTileWall(player.base.x + playerSizeHalf, playerNextY + playerSizeHalf))
		player.base.y = playerNextY;
}