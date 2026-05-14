#include <Windows.h>
#include <math.h>

#include "player.h"
#include "config.h"
#include "map.h"
#include "camera.h"
#include "collision.h"
#include "bullet.h"

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

// 플레이어 업데이트
void UpdatePlayer() {
	player.base.dx = 0;
	player.base.dy = 0;

	if (GetAsyncKeyState('a') || GetAsyncKeyState('A')) player.base.dx = -PLAYER_SPEED;
	if (GetAsyncKeyState('d') || GetAsyncKeyState('D')) player.base.dx = PLAYER_SPEED;
	if (GetAsyncKeyState('w') || GetAsyncKeyState('W')) player.base.dy = -PLAYER_SPEED;
	if (GetAsyncKeyState('s') || GetAsyncKeyState('S')) player.base.dy = PLAYER_SPEED;

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

	MapTransition();
}