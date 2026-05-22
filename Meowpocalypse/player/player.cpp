#include <Windows.h>
#include <math.h>

#include "player.h"
#include "config.h"
#include "map.h"
#include "camera.h"
#include "collision.h"
#include "bullet.h"
#include "input.h"

PLAYER player;

// 초기 설정
void InitPlayer() {
	player.base.x = player.base.hitBoxX = (float)SCREEN_WIDTH / 2;
	player.base.y = player.base.hitBoxY = (float)SCREEN_HEIGHT / 2;
	player.base.width = player.base.height = PLAYER_SIZE;
	player.base.hitBoxW = player.base.hitBoxH = PLAYER_HITBOX_SIZE;
	player.base.hp = 200;
	player.mp = 100;

	player.invincibleTimer = 0;

	player.skillQCooldown = 0;
	player.skillRCooldown = 0;

	player.boostTimer = 0;
	player.boostCooldown = 0;
	player.fireTimer = 0;

	player.base.direction = DIR_DOWN;

	player.base.kx = player.base.ky = 0;
	player.base.kTimer = 0;
}

// MP를 소모하고 성공 여부를 반환하는 함수
BOOL ConsumeMP(int amount) {
	if (player.mp >= amount) {
		player.mp -= amount;
		return TRUE;	// 소모 성공
	}

	return FALSE;	// 소모 실패
}

// 플레이어 업데이트
void UpdatePlayer() {
	UpdatePlayerStatus();
	if (player.base.state == PLAYER_DEAD) return;

	UpdatePlayerTimers();
	HandlePlayerInput();
	HandlePlayerMovement();
	HandlePlayerCollision();

	MapTransition();
}

// 사망 여부 및 HP 확인
void UpdatePlayerStatus() {
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
}

// 무적 시간, 스킬 쿨타임 등 타이머 갱신
void UpdatePlayerTimers() {
	// 무적 타이머 감소
	if (player.invincibleTimer > 0) {
		player.invincibleTimer--;
		// 무적 타이머가 동작 중일 때 (즉, 맞았을 때) HIT 상태 유지 (넉백 시간 동안)
		if (player.base.kTimer > 0) {
			player.base.state = PLAYER_HIT;
		}
	}

	// Q 스킬 - 쿨타임 타이머 감소
	if (player.skillQCooldown > 0) {
		player.skillQCooldown--;
	}

	// E 스킬 - 부스트 타이머 감소
	if (player.boostTimer > 0) {
		player.boostTimer--;
	}

	// E 스킬 - 쿨타임 타이머 감소
	if (player.boostCooldown > 0) {
		player.boostCooldown--;
	}

	// E 스킬 - 발사 타이머 감소
	if (player.fireTimer > 0) {
		player.fireTimer--;
	}

	// R 스킬 - 쿨타임 타이머 감소
	if (player.skillRCooldown > 0) {
		player.skillRCooldown--;
	}
}

// 스킬 입력 처리
void HandlePlayerInput() {
	// E 스킬 - 부스트 활성화 (쿨타임이 0일 때만 가능)
	if (g_Input.isEPressed && player.boostTimer <= 0 && player.boostCooldown <= 0) {
		if (ConsumeMP(SKILL_E_MP)) {
			player.boostTimer = BOOST_DURATION;
			player.boostCooldown = BOOST_DURATION + BOOST_COOLDOWN; // 지속시간 + 쿨타임만큼 설정
		}
	}
}

// 넉백 및 방향 키에 따른 이동 로직 처리
void HandlePlayerMovement() {
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
		float moveX = g_Input.moveX;
		float moveY = g_Input.moveY;

		if (moveX != 0 || moveY != 0) {
			float length = sqrtf(moveX * moveX + moveY * moveY);
			float speed = PLAYER_SPEED;

			// 부스트 상태일 때 속도 증가
			if (player.boostTimer > 0) {
				speed *= BOOST_SPEED_MULTIPLIER;
			}

			player.base.dx = (moveX / length) * speed;
			player.base.dy = (moveY / length) * speed;

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
}