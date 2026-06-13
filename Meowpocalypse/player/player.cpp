#include <Windows.h>
#include <math.h>

#include "player.h"
#include "config.h"
#include "map.h"
#include "camera.h"
#include "collision.h"
#include "bullet.h"
#include "input.h"
#include "sound.h"

PLAYER player;
IMAGE imgPlayerSprite;

// 초기 설정
void InitPlayer() {
	if (imgPlayerSprite.img.IsNull()) {
		LoadMyImage(&imgPlayerSprite, L"player_sprite.png");
	}

	int fw = imgPlayerSprite.width / 4;
	int fh = imgPlayerSprite.height / 29;

	player.base.x = player.base.hitBoxX = (float)(SCREEN_WIDTH / 2 + (TILE_SIZE / 2));
	player.base.y = player.base.hitBoxY = (float)(SCREEN_HEIGHT / 2 + (TILE_SIZE / 2));
	player.base.width = PLAYER_WIDTH;
	player.base.height = PLAYER_HEIGHT;
	player.base.hitBoxW = PLAYER_HITBOX_WIDTH;
	player.base.hitBoxH = PLAYER_HITBOX_HEIGHT;
	player.base.hp = PLAYER_HP;
	player.mp = PLAYER_MP;
	player.speed = 0;

	player.invincibleTimer = 0;

	player.skillQCooldown = 0;

	player.boostTimer = 0;
	player.boostCooldown = 0;
	player.fireTimer = 0;

	player.skillRCooldown = 0;

	player.itemOneCooldown = 0;
	player.itemTwoCooldown = 0;

	player.hpPotionCount = POTION_LIMIT;
	player.mpPotionCount = POTION_LIMIT;

	player.base.state = PLAYER_IDLE;
	player.base.direction = DIR_DOWN;

	player.base.kx = player.base.ky = 0;
	player.base.kTimer = 0;

	player.deathTimer = 0;

	InitAnimation(&player.anim, &imgPlayerSprite, fw, fh, 4, 10);
}

void ReleasePlayer() {
	ReleaseMyImage(&imgPlayerSprite);
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
	UpdateAnimation(&player.anim);
	UpdatePlayerTimers();

	if (player.base.state == PLAYER_DEAD) return;

	HandlePlayerInput();
	HandlePlayerMovement();
	HandlePlayerWallCollision();

	MapTransition();
}

// 사망 여부 및 HP 확인
void UpdatePlayerStatus() {
	// 사망 상태 처리
	if (player.base.state == PLAYER_DEAD) {
		player.anim.isLoop = FALSE;

		player.deathTimer--;
		if (player.deathTimer <= 0) {
			player.deathTimer = 0;

			g_UI.isPlayerDeadFadeOut = TRUE;
		}
		return;
	}

	player.anim.isLoop = TRUE;

	// 피가 0 이하이면 DEAD 상태로 전환
	if (player.base.hp <= 0) {
		player.base.hp = 0;
		player.base.state = PLAYER_DEAD;
		player.deathTimer = 0;
		player.invincibleTimer = 0; // 사망 시 무적 시간 초기화 (깜빡임 방지)
		SetAnimationFrame(&player.anim, 0);
		PlaySFX(SFX_PLAYER_DEAD);
		return;
	}

	player.base.dx = 0;
	player.base.dy = 0;
}

// 무적 시간, 스킬 쿨타임 등 타이머 갱신
void UpdatePlayerTimers() {
	static DWORD lastTime = GetTickCount64();
	DWORD currTime = GetTickCount64();

	int elapsed = (int)(currTime - lastTime);
	lastTime = currTime;

	// 60 FPS 기준(16.66ms)으로 몇 프레임이 지났어야 하는지 누적 계산
	static float accumulator = 0.0f;
	accumulator += (float)elapsed / (1000.0f / 60.0f);

	int ticks = (int)accumulator;
	accumulator -= (float)ticks;

	// 아직 1프레임 분량의 시간도 안 지났다면 생략
	if (ticks <= 0) return;

	// 무적 타이머 감소
	if (player.invincibleTimer > 0) {
		player.invincibleTimer -= ticks;

		if (player.invincibleTimer < 0) player.invincibleTimer = 0;
		// 무적 타이머가 동작 중일 때 (즉, 맞았을 때) HIT 상태 유지 (넉백 시간 동안)
		if (player.base.kTimer > 0) {
			player.base.state = PLAYER_HIT;
		}
	}

	// Q 스킬 - 쿨타임 타이머 감소
	if (player.skillQCooldown > 0) {
		player.skillQCooldown -= ticks;

		if (player.skillQCooldown < 0) player.skillQCooldown = 0;
	}

	// E 스킬 - 부스트 타이머 감소
	if (player.boostTimer > 0) {
		player.boostTimer -= ticks;

		if (player.boostTimer < 0) player.boostTimer = 0;
	}

	// E 스킬 - 쿨타임 타이머 감소
	if (player.boostCooldown > 0) {
		player.boostCooldown -= ticks;

		if (player.boostCooldown < 0) player.boostCooldown = 0;
	}

	// E 스킬 - 발사 타이머 감소
	if (player.fireTimer > 0) {
		player.fireTimer -= ticks;

		if (player.fireTimer < 0) player.fireTimer = 0;
	}

	// R 스킬 - 쿨타임 타이머 감소
	if (player.skillRCooldown > 0) {
		player.skillRCooldown -= ticks;

		if (player.skillRCooldown < 0) player.skillRCooldown = 0;
	}

	// 키보드 숫자 1(잃은 체력 비례 HP 회복) 아이템 - 쿨타임 타이머 감소
	if (player.itemOneCooldown > 0) {
		player.itemOneCooldown -= ticks;

		if (player.itemOneCooldown < 0) player.itemOneCooldown = 0;
	}

	// 키보드 숫자 2(MP 전체 회복) 아이템 - 쿨타임 타이머 감소
	if (player.itemTwoCooldown > 0) {
		player.itemTwoCooldown -= ticks;

		if (player.itemTwoCooldown < 0) player.itemTwoCooldown = 0;
	}
	}

// 스킬 및 아이템 입력 처리
void HandlePlayerInput() {
	// E 스킬 - 부스트 활성화 (쿨타임이 0일 때만 가능)
	if (g_Input.isEPressed && player.boostTimer <= 0 && player.boostCooldown <= 0) {
		if (ConsumeMP(SKILL_E_MP)) {
			player.boostTimer = BOOST_DURATION;
			player.boostCooldown = BOOST_DURATION + BOOST_COOLDOWN; // 지속시간 + 쿨타임만큼 설정
		}
	}

	// 키보드 숫자 1 아이템 - HP 회복 (잃은 체력의 30%)
	if (g_Input.isOnePressed && player.itemOneCooldown <= 0 && player.hpPotionCount > 0) {
		int missingHp = PLAYER_HP - player.base.hp;
		PlaySFX(SFX_PLAYER_SKILL_DRINK);
		if (missingHp > 0) {
			player.itemOneCooldown = ITEM_ONE_COOLDOWN;
			player.hpPotionCount--;

			int healAmount = (int)(missingHp * 0.3f);
			if (healAmount < 1) healAmount = 1;

			player.base.hp += healAmount;
			if (player.base.hp > PLAYER_HP) player.base.hp = PLAYER_HP;
		}
		else {
			player.itemOneCooldown = ITEM_ONE_COOLDOWN;
			player.hpPotionCount--;
		}
	}

	// 키보드 숫자 2 아이템 - MP 전체 회복
	if (g_Input.isTwoPressed && player.itemTwoCooldown <= 0 && player.mpPotionCount > 0) {
		int missingMp = PLAYER_MP - player.mp;
		PlaySFX(SFX_PLAYER_SKILL_DRINK);
		if (missingMp > 0) {
			player.itemTwoCooldown = ITEM_TWO_COOLDOWN;
			player.mpPotionCount--;

			int chargeAmount = (int)(missingMp * 0.7f);
			if (chargeAmount < 1) chargeAmount = 1;

			player.mp += chargeAmount;
			if (player.mp > PLAYER_MP) player.mp = PLAYER_MP;
		}
		else {
			player.itemTwoCooldown = ITEM_TWO_COOLDOWN;
			player.mpPotionCount--;
		}
	}
}

// 넉백 및 방향 키에 따른 이동 로직 처리
void HandlePlayerMovement() {
	if (player.base.state == PLAYER_DEAD) return;

	// 넉백 처리
	if (player.base.kTimer > 0) {
		player.base.dx = player.base.kx;
		player.base.dy = player.base.ky;
		player.base.kTimer--;

		// 넉백 시에는 공격이 온 방향(넉백의 반대 방향)을 바라봄
		if (fabsf(player.base.kx) > fabsf(player.base.ky) * 2) {
			if (player.base.kx > 0) player.base.direction = DIR_LEFT;
			else player.base.direction = DIR_RIGHT;
		}
		else if (fabsf(player.base.ky) > fabsf(player.base.kx) * 2) {
			if (player.base.ky > 0) player.base.direction = DIR_UP;
			else player.base.direction = DIR_DOWN;
		}
		else {
			if (player.base.kx < 0 && player.base.ky < 0) player.base.direction = DIR_DOWN_RIGHT;
			else if (player.base.kx > 0 && player.base.ky < 0) player.base.direction = DIR_DOWN_LEFT;
			else if (player.base.kx < 0 && player.base.ky > 0) player.base.direction = DIR_UP_RIGHT;
			else if (player.base.kx > 0 && player.base.ky > 0) player.base.direction = DIR_UP_LEFT;
		}
	}
	else {
		float moveX = g_Input.moveX;
		float moveY = g_Input.moveY;

		if (moveX != 0 || moveY != 0) {

			static int footstepTimer = 0;
			if (g_Input.moveX != 0 || g_Input.moveY != 0) {
				if (footstepTimer <= 0) {
					PlaySFX(SFX_PLAYER_FOOTSTEP);
					footstepTimer = 18;   // 약 18프레임마다
				}
			}
			if (footstepTimer > 0) footstepTimer--;

			float length = sqrtf(moveX * moveX + moveY * moveY);
			
			HandlePlayerCyberSlimeCollision();

			player.base.dx = (moveX / length) * player.speed;
			player.base.dy = (moveY / length) * player.speed;

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