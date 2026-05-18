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
	player.base.direction = DIR_DOWN;
	player.base.kx = player.base.ky = 0;
	player.base.kTimer = 0;
}

// 플레이어 업데이트
void UpdatePlayer() {
	// 이미 죽은 상태면 업데이트 중단
	if (player.base.state == PLAYER_DEAD) return;

	// 피가 0 이하이면 DEAD 상태로 전환
	if (player.base.hp <= 0) {
		player.base.hp = 0;
		player.base.state = PLAYER_DEAD;
		return;
	}

	player.base.dx = 0;
	player.base.dy = 0;

	// 무적 타이머 감소
	if (player.invincibleTimer > 0) {
		player.invincibleTimer--;
		// 무적 타이머가 동작 중일 때 (즉, 맞았을 때) HIT 상태 유지 (넉백 시간 동안)
		if (player.base.kTimer > 0) {
			player.base.state = PLAYER_HIT;
		}
	}

	// 넉백 처리
	if (player.base.kTimer > 0) {
		player.base.dx = player.base.kx;
		player.base.dy = player.base.ky;
		player.base.kTimer--;

		// 넉백 시에는 공격이 온 방향(넉백의 반대 방향)을 바라봄
		if (fabsf(player.base.kx) > fabsf(player.base.ky)) {
			if (player.base.kx > 0) player.base.direction = DIR_LEFT;
			else player.base.direction = DIR_RIGHT;
		}
		else {
			if (player.base.ky > 0) player.base.direction = DIR_UP;
			else player.base.direction = DIR_DOWN;
		}
	}
	else {
		float moveX = 0, moveY = 0;
		if (GetAsyncKeyState('a') || GetAsyncKeyState('A')) moveX -= 1.0f;
		if (GetAsyncKeyState('d') || GetAsyncKeyState('D')) moveX += 1.0f;
		if (GetAsyncKeyState('w') || GetAsyncKeyState('W')) moveY -= 1.0f;
		if (GetAsyncKeyState('s') || GetAsyncKeyState('S')) moveY += 1.0f;

		if (moveX != 0 || moveY != 0) {
			float length = sqrtf(moveX * moveX + moveY * moveY);
			player.base.dx = (moveX / length) * PLAYER_SPEED;
			player.base.dy = (moveY / length) * PLAYER_SPEED;

			if (moveX > 0) {
				if (moveY > 0) player.base.direction = DIR_DOWN_RIGHT;
				else if (moveY < 0) player.base.direction = DIR_UP_RIGHT;
				else player.base.direction = DIR_RIGHT;
			}
			else if (moveX < 0) {
				if (moveY > 0) player.base.direction = DIR_DOWN_LEFT;
				else if (moveY < 0) player.base.direction = DIR_UP_LEFT;
				else player.base.direction = DIR_LEFT;
			}
			else {
				if (moveY > 0) player.base.direction = DIR_DOWN;
				else if (moveY < 0) player.base.direction = DIR_UP;
			}

			player.base.state = PLAYER_MOVE;
		}
		else {
			player.base.dx = 0;
			player.base.dy = 0;
			player.base.state = PLAYER_IDLE;
		}
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