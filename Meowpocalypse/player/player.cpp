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
	player.base.x = player.base.hitBoxX = SCREEN_WIDTH / 2;
	player.base.y = player.base.hitBoxY = SCREEN_HEIGHT / 2;
	player.base.width = player.base.height = PLAYER_SIZE;
	player.base.hitBoxW = player.base.hitBoxH = PLAYER_HITBOX_SIZE;
	player.base.hp = 200;
	player.mp = 100;
	player.invincibleTimer = 0;
	player.base.kx = player.base.ky = 0;
	player.base.kTimer = 0;
}

// 플레이어 업데이트
void UpdatePlayer() {
	player.base.dx = 0;
	player.base.dy = 0;

	// 무적 타이머 감소
	if (player.invincibleTimer > 0) player.invincibleTimer--;

	// 넉백 처리
	if (player.base.kTimer > 0) {
		player.base.dx = player.base.kx;
		player.base.dy = player.base.ky;
		player.base.kTimer--;
	}
	else {
		if (GetAsyncKeyState('a') || GetAsyncKeyState('A')) player.base.dx = -PLAYER_SPEED;
		if (GetAsyncKeyState('d') || GetAsyncKeyState('D')) player.base.dx = PLAYER_SPEED;
		if (GetAsyncKeyState('w') || GetAsyncKeyState('W')) player.base.dy = -PLAYER_SPEED;
		if (GetAsyncKeyState('s') || GetAsyncKeyState('S')) player.base.dy = PLAYER_SPEED;
	}

	float playerNextX = player.base.x + player.base.dx;
	float playerNextY = player.base.y + player.base.dy;

	int playerSizeHalf = PLAYER_SIZE / 2;
	if (!IsTileWall(playerNextX - playerSizeHalf, player.base.y - playerSizeHalf) &&
		!IsTileWall(playerNextX + playerSizeHalf, player.base.y - playerSizeHalf) &&
		!IsTileWall(playerNextX - playerSizeHalf, player.base.y + playerSizeHalf) &&
		!IsTileWall(playerNextX + playerSizeHalf, player.base.y + playerSizeHalf))
		player.base.x = player.base.hitBoxX = playerNextX;

	if (!IsTileWall(player.base.x - playerSizeHalf, playerNextY - playerSizeHalf) &&
		!IsTileWall(player.base.x + playerSizeHalf, playerNextY - playerSizeHalf) &&
		!IsTileWall(player.base.x - playerSizeHalf, playerNextY + playerSizeHalf) &&
		!IsTileWall(player.base.x + playerSizeHalf, playerNextY + playerSizeHalf))
		player.base.y = player.base.hitBoxY = playerNextY;

	MapTransition();
}