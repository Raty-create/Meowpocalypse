#include <Windows.h>
#include <math.h>

#include "config.h"
#include "bullet.h"
#include "player.h"
#include "camera.h"
#include "collision.h"
#include "enum.h"
#include "input.h"

BULLET bullets[BULLET_MAX];
CHURU churues[CHURU_MAX];
IMAGE imgProjectile;

void InitBullet() {
	if (imgProjectile.img.IsNull()) {
		LoadMyImage(&imgProjectile, L"projectile.png");
	}

	for (int i = 0; i < BULLET_MAX; i++) {
		bullets[i].isActive = INACTIVE;
		bullets[i].width = BULLET_WIDTH;
		bullets[i].height = BULLET_HEIGHT;
		bullets[i].hitBoxW = BULLET_HITBOX_WIDTH;
		bullets[i].hitBoxH = BULLET_HITBOX_HEIGHT;
	}
}

void ReleaseBullet() {
	ReleaseMyImage(&imgProjectile);
}

// 총알 업데이트
void UpdateBullet() {
	for (int i = 0; i < BULLET_MAX; i++) {
		if (bullets[i].isActive == INACTIVE) continue;

		bullets[i].x += bullets[i].dx;
		bullets[i].y += bullets[i].dy;
		bullets[i].hitBoxX += bullets[i].dx;
		bullets[i].hitBoxY += bullets[i].dy;

		// 적과의 충돌 체크
		for (int j = 0; j < ENEMY_LIMIT; j++) {
			if (enemies[j].isActive == ACTIVE && enemies[j].base.state != ENEMY_DEAD) {
				if (HandleBulletEnemyCollision(&bullets[i], &enemies[j])) {
					// 충돌 시 해당 총알은 더 이상 처리할 필요 없음
					break;
				}
			}
		}

		if (bullets[i].isActive == ACTIVE) {
			HandleBulletBossCollision(&bullets[i], &boss);
		}

		// 이미 적과 충돌하여 비활성화되었다면 벽 충돌 체크는 건너뜀
		BOOL insidePlayerBody = IsObjectCollision(
			bullets[i].x, bullets[i].y, BULLET_HITBOX_WIDTH, BULLET_HITBOX_HEIGHT,
			player.base.x, player.base.y, PLAYER_WIDTH, PLAYER_HEIGHT * 2);

		if (!insidePlayerBody) {
			int halfW = BULLET_HITBOX_WIDTH / 2;
			int halfH = BULLET_HITBOX_HEIGHT / 2;

			if (IsTileBlocked(bullets[i].x - halfW, bullets[i].y - halfH) ||
				IsTileBlocked(bullets[i].x + halfW, bullets[i].y - halfH) ||
				IsTileBlocked(bullets[i].x - halfW, bullets[i].y + halfH) ||
				IsTileBlocked(bullets[i].x + halfW, bullets[i].y + halfH))
				bullets[i].isActive = INACTIVE;
		}
	}
}

// 총알 발사
void ShootBullet() {
	if (player.base.state == PLAYER_DEAD) return;

	// 마우스 왼쪽 버튼이 눌려 있을 때 (isLButtonDown으로 변경하여 자동 연사 지원)
	if (g_Input.isLButtonDown) {
		player.base.state = PLAYER_SHOOT;

		POINT pt = g_Input.mouseWorldPos;

		// 플레이어의 스크린 좌표
		float playerScreenX = player.base.x - camera.x;
		float playerScreenY = player.base.y - camera.y;

		// 마우스 방향 벡터
		float dirX = pt.x - playerScreenX;
		float dirY = pt.y - playerScreenY;

		float length = sqrtf(dirX * dirX + dirY * dirY);
		if (length == 0) return;

		// 마우스 방향에 따른 플레이어 방향 업데이트
		float angle = atan2f(dirY, dirX) * 180.0f / PI;
		if (angle < 0) angle += 360.0f;

		if (angle >= 337.5f || angle < 22.5f) player.base.direction = DIR_RIGHT;
		else if (angle >= 22.5f && angle < 67.5f) player.base.direction = DIR_DOWN_RIGHT;
		else if (angle >= 67.5f && angle < 112.5f) player.base.direction = DIR_DOWN;
		else if (angle >= 112.5f && angle < 157.5f) player.base.direction = DIR_DOWN_LEFT;
		else if (angle >= 157.5f && angle < 202.5f) player.base.direction = DIR_LEFT;
		else if (angle >= 202.5f && angle < 247.5f) player.base.direction = DIR_UP_LEFT;
		else if (angle >= 247.5f && angle < 292.5f) player.base.direction = DIR_UP;
		else if (angle >= 292.5f && angle < 337.5f) player.base.direction = DIR_UP_RIGHT;

		// 발사 쿨타임 체크
		if (player.fireTimer > 0) return;

		for (int i = 0; i < BULLET_MAX; i++) {
			if (bullets[i].isActive == INACTIVE) {
				int fw = imgProjectile.width / 4;
				int fh = imgProjectile.height / 17;

				bullets[i].x = player.base.x;
				bullets[i].y = player.base.y;
				bullets[i].hitBoxX = player.base.x;
				bullets[i].hitBoxY = player.base.y;
				bullets[i].dx = (dirX / length) * BULLET_SPEED;
				bullets[i].dy = (dirY / length) * BULLET_SPEED;
				bullets[i].isActive = ACTIVE;

				InitAnimation(&bullets[i].anim, &imgProjectile, fw, fh, 1, 0, FALSE);
				bullets[i].dirRow = 0;

				// 발사 쿨타임 설정
				float cooldown = PLAYER_FIRE_COOLDOWN;
				if (player.boostTimer > 0) {
					cooldown *= BOOST_FIRE_RATE_MULTIPLIER;
				}
				player.fireTimer = (int)cooldown;

				break;
			}
		}
	}
}

// 플레이어 Q 입력 시 세 방향 총알 발사
void ShootSkillQ() {
	if (!g_Input.isQPressed) return;

	// Q 스킬 - 쿨타임이 0일 때만 가능
	if (g_Input.isQPressed && player.skillQCooldown <= 0) {
		if (ConsumeMP(SKILL_Q_MP)) {
			int bulletFired = 0;
			float spreadAngle = DEG_TO_RAD(15.0f);

			POINT pt = g_Input.mouseWorldPos;
			float playerScreenX = player.base.x - camera.x;
			float playerScreenY = player.base.y - camera.y;

			float baseDirX = pt.x - playerScreenX;
			float baseDirY = pt.y - playerScreenY;
			float baseAngle = (float)atan2f(baseDirY, baseDirX);

			for (int i = 0; i < BULLET_MAX && bulletFired < 3; i++) {
				if (bullets[i].isActive == INACTIVE) {
					int fw = imgProjectile.width / 4;
					int fh = imgProjectile.height / 17;

					float currAngle = baseAngle + (bulletFired - 1) * spreadAngle;

					bullets[i].x = player.base.x;
					bullets[i].y = player.base.y;
					bullets[i].hitBoxX = player.base.x;
					bullets[i].hitBoxY = player.base.y;
					bullets[i].dx = cosf(currAngle) * BULLET_SPEED;
					bullets[i].dy = sinf(currAngle) * BULLET_SPEED;
					bullets[i].isActive = ACTIVE;
					bullets[i].damage = BULLET_DAMAGE;

					InitAnimation(&bullets[i].anim, &imgProjectile, fw, fh, 1, 0, FALSE);
					bullets[i].dirRow = 0;

					bulletFired++;
				}
			}

			player.skillQCooldown = SKILL_Q_COOLDOWN;
		}
	}
}

// 플레이어 R 입력 시 츄르 던지기
void InitChuru() {
	for (int i = 0; i < CHURU_MAX; i++) {
		churues[i].isActive = INACTIVE;
		churues[i].width = CHURU_WIDTH;
		churues[i].height = CHURU_HEIGHT;
	}
}

void UpdateChuru() {
	for (int i = 0; i < CHURU_MAX; i++) {
		if (churues[i].isActive == INACTIVE) continue;

		UpdateAnimation(&churues[i].anim);

		if (churues[i].isDropped == AIRBORNE) {
			float nextX = churues[i].x + churues[i].dx;
			float nextY = churues[i].y + churues[i].dy;

			float margin = 5.0f;
			int hw = (int)(churues[i].width / 2 + margin);
			int hh = (int)(churues[i].height / 2 + margin);
			BOOL hitWall = FALSE;

			if (IsTileBlocked(nextX - hw, nextY - hh) || IsTileBlocked(nextX + hw, nextY - hh) ||
				IsTileBlocked(nextX - hw, nextY + hh) || IsTileBlocked(nextX + hw, nextY + hh)) {
				hitWall = TRUE;
			}

			// 최대 거리에 도달하거나 벽에 충돌 시 바닥에 떨어뜨림
			float distX = nextX - churues[i].startX;
			float distY = nextY - churues[i].startY;
			float dest = sqrtf(distX * distX + distY * distY);

			if (dest >= CHURU_MAX_DIST || hitWall) {
				churues[i].dx = 0;
				churues[i].dy = 0;
				churues[i].isDropped = DROPPED;
				churues[i].dirRow += 4;
				churues[i].activeTimer = CHURU_DURATION;
				SetAnimationFrame(&churues[i].anim, 0);
			}
			else {
				churues[i].x = nextX;
				churues[i].y = nextY;
			}

			// 공중 상태에서도 보스와 충돌 체크
			HandleChuruBossCollision(&churues[i], &boss);
		}
		else {
			churues[i].activeTimer--;

			// 바닥에 떨어진 상태에서도 보스와 충돌 체크
			HandleChuruBossCollision(&churues[i], &boss);

			if (churues[i].activeTimer <= 0)
				churues[i].isActive = INACTIVE;
		}
	}
}

void ShootSkillR() {
	if (!g_Input.isRPressed || (player.skillRCooldown > 0)) return;

	// R 스킬 - 쿨타임이 0일 때만 가능
	if (ConsumeMP(SKILL_R_MP)) {
		POINT pt = g_Input.mouseWorldPos;

		float playerScreenX = player.base.x - camera.x;
		float playerScreenY = player.base.y - camera.y;

		float dirX = pt.x - playerScreenX;
		float dirY = pt.y - playerScreenY;

		float length = sqrtf(dirX * dirX + dirY * dirY);
		if (length == 0) return;

		// 마우스 방향에 따른 플레이어 방향 업데이트
		float angle = atan2f(dirY, dirX) * 180.0f / PI;
		if (angle < 0) angle += 360.0f;

		if (angle >= 337.5f || angle < 22.5f) player.base.direction = DIR_RIGHT;
		else if (angle >= 22.5f && angle < 67.5f) player.base.direction = DIR_DOWN_RIGHT;
		else if (angle >= 67.5f && angle < 112.5f) player.base.direction = DIR_DOWN;
		else if (angle >= 112.5f && angle < 157.5f) player.base.direction = DIR_DOWN_LEFT;
		else if (angle >= 157.5f && angle < 202.5f) player.base.direction = DIR_LEFT;
		else if (angle >= 202.5f && angle < 247.5f) player.base.direction = DIR_UP_LEFT;
		else if (angle >= 247.5f && angle < 292.5f) player.base.direction = DIR_UP;
		else if (angle >= 292.5f && angle < 337.5f) player.base.direction = DIR_UP_RIGHT;

		for (int i = 0; i < CHURU_MAX; i++) {
			if (churues[i].isActive == INACTIVE) {
				int fw = imgProjectile.width / 4;
				int fh = imgProjectile.height / 17;

				churues[i].x = player.base.x;
				churues[i].y = player.base.y;
				churues[i].startX = player.base.x;
				churues[i].startY = player.base.y;
				churues[i].dx = (dirX / length) * CHURU_SPEED;
				churues[i].dy = (dirY / length) * CHURU_SPEED;
				churues[i].isActive = ACTIVE;
				churues[i].isDropped = AIRBORNE;

				switch (player.base.direction) {
				case DIR_DOWN:
				case DIR_DOWN_LEFT:
				case DIR_DOWN_RIGHT:
					churues[i].dirRow = 1;
					break;
				case DIR_UP:
				case DIR_UP_LEFT:
				case DIR_UP_RIGHT:
					churues[i].dirRow = 2;
					break;
				case DIR_LEFT:
					churues[i].dirRow = 3;
					break;
				case DIR_RIGHT:
					churues[i].dirRow = 4;
					break;
				}

				InitAnimation(&churues[i].anim, &imgProjectile, fw, fh, 4, 8);

				player.skillRCooldown = SKILL_R_COOLDOWN;

				break;
			}
		}
	}
}