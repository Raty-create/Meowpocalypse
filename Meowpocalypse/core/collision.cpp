#include "map.h"
#include "config.h"
#include "collision.h"
#include "player.h"
#include "enemy.h"
#include "boss.h"

#include <math.h>

// AABB 충돌 체크
int IsObjectCollision(float ax, float ay, int aw, int ah, float bx, float by, int bw, int bh) {
	return (ax - aw / 2 < bx + bw / 2 &&
		ax + aw / 2 > bx - bw / 2 &&
		ay - ah / 2 < by + bh / 2 &&
		ay + ah / 2 > by - bh / 2);
}

// 플레이어 - 벽 충돌 체크
int IsTileWall(float x, float y) {
	MAPDATA* m = &maps[currentMapType];

	int col = (int)((x - m->worldX) / TILE_SIZE);
	int row = (int)((y - m->worldY) / TILE_SIZE);

	if (row < 0 || row >= m->rows) return 1;
	if (col < 0 || col >= m->cols) return 1;

	int tile = m->tiles[row][col];
	return tile == TILE_WALL;
}

// 플레이어 - 문
int IsPlayerOnDoor() {
	MAPDATA* m = &maps[currentMapType];

	// 플레이어의 4개 모서리 체크
	int halfW = PLAYER_WIDTH / 2;
	int halfH = PLAYER_HEIGHT;
	float point[4][2] = {
		{player.base.x - halfW + 1, player.base.y + halfH + 1},
		{player.base.x + halfW - 1, player.base.y + halfH + 1},
		{player.base.x - halfW + 1, player.base.y + halfH - 1},
		{player.base.x + halfW - 1, player.base.y + halfH - 1}
	};

	for (int p = 0; p < 4; p++) {
		int col = (int)((point[p][0] - m->worldX) / TILE_SIZE);
		int row = (int)((point[p][1] - m->worldY) / TILE_SIZE);

		if (row < 0 || row >= m->rows || col < 0 || col >= m->cols) continue;
		if (m->tiles[row][col] != TILE_DOOR) continue;

		for (int d = 0; d < m->doorCount; d++) {
			if (m->doors[d].row == row && m->doors[d].col == col && m->doors[d].state == DOOR_OPEN) {
				return 1;
			}
		}
	}
	return 0;
}

// 플레이어 - 벽과의 충돌 처리
void HandlePlayerWallCollision() {
	float playerNextX = player.base.x + player.base.dx;
	float playerNextY = player.base.y + player.base.dy;

	int playerHalfW = PLAYER_WIDTH / 2;
	int playerHalfH = PLAYER_HEIGHT;
	if (!IsTileWall(playerNextX - playerHalfW, player.base.y + playerHalfH) &&
		!IsTileWall(playerNextX + playerHalfW, player.base.y + playerHalfH))
		player.base.x = player.base.hitBoxX = playerNextX;

	if (!IsTileWall(player.base.x - playerHalfW, playerNextY + playerHalfH) &&
		!IsTileWall(player.base.x + playerHalfW, playerNextY + playerHalfH))
		player.base.y = player.base.hitBoxY = playerNextY;
}

// 잡몹 - 잡몹 충돌 체크
int IsOverlapWithEnemy(float x, float y) {
	for (int j = 0; j < ENEMY_LIMIT; j++) {
		if (!enemies[j].isActive) continue;
		float dx = enemies[j].base.x - x;
		float dy = enemies[j].base.y - y;
		if (sqrtf(dx * dx + dy * dy) < ENEMY_WIDTH) return 1;
	}
	return 0;
}

// 총알 - 적 충돌 처리
int HandleBulletEnemyCollision(BULLET* bullet, ENEMY* enemy) {
	if (!bullet->isActive || !enemy->isActive) return 0;

	if (IsObjectCollision(bullet->hitBoxX, bullet->hitBoxY, bullet->hitBoxW, bullet->hitBoxH,
		enemy->base.hitBoxX, enemy->base.hitBoxY, enemy->base.hitBoxW, enemy->base.hitBoxH)) {

		bullet->isActive = INACTIVE;
		enemy->base.hp -= BULLET_DAMAGE;

		if (enemy->base.hp <= 0) {
			enemy->base.hp = 0;
			enemy->base.state = ENEMY_DEAD;
			enemy->deathTimer = ENEMY_DEATH_TIME;
		}
		else {
			enemy->base.state = ENEMY_HIT;
		}

		// 넉백 적용: 총알의 이동 방향으로 밀려남
		float dist = sqrtf(bullet->dx * bullet->dx + bullet->dy * bullet->dy);
		if (dist > 0) {
			enemy->base.kx = (bullet->dx / dist) * KNOCKBACK_FORCE;
			enemy->base.ky = (bullet->dy / dist) * KNOCKBACK_FORCE;
			enemy->base.kTimer = KNOCKBACK_TIME;
		}

		return 1;
	}
	return 0;
}

// 잡몹 젤리 - 플레이어 충돌 처리
int HandleCatPawPlayerCollision(CATPAW* cp, PLAYER* p) {
	if (!cp->isActive || p->invincibleTimer > 0) return 0;

	// CatPaw의 크기는 설정값 사용
	if (IsObjectCollision(cp->hitBoxX, cp->hitBoxY, cp->hitBoxW, cp->hitBoxH,
		p->base.hitBoxX, p->base.hitBoxY, p->base.hitBoxW, p->base.hitBoxH)) {

		cp->isActive = INACTIVE;
		p->base.hp -= CAT_PAW_DAMAGE;
		p->invincibleTimer = PLAYER_INVINCIBLE_TIME;

		if (p->base.hp <= 0) {
			p->base.hp = 0;
			p->base.state = PLAYER_DEAD;
			p->deathTimer = PLAYER_DEATH_TIME;
		}
		else {
			p->base.state = PLAYER_HIT;
		}

		// 넉백 적용: 투사체의 이동 방향으로 밀려남
		float dist = sqrtf(cp->dx * cp->dx + cp->dy * cp->dy);
		if (dist > 0) {
			p->base.kx = (cp->dx / dist) * KNOCKBACK_FORCE;
			p->base.ky = (cp->dy / dist) * KNOCKBACK_FORCE;
			p->base.kTimer = KNOCKBACK_TIME;
		}

		if (p->base.hp < 0) p->base.hp = 0;
		return 1;
	}
	return 0;
}

// 적 - 플레이어 충돌 처리 (근접 데미지)
int HandleEnemyPlayerCollision(ENEMY* enemy, PLAYER* p) {
	if (!enemy->isActive || p->invincibleTimer > 0) return 0;

	if (IsObjectCollision(enemy->base.hitBoxX, enemy->base.hitBoxY, enemy->base.hitBoxW, enemy->base.hitBoxH,
		p->base.hitBoxX, p->base.hitBoxY, p->base.hitBoxW, p->base.hitBoxH)) {

		p->base.hp -= ENEMY_CONTACT_DAMAGE;
		p->invincibleTimer = PLAYER_INVINCIBLE_TIME;

		if (p->base.hp <= 0) {
			p->base.hp = 0;
			p->base.state = PLAYER_DEAD;
			p->deathTimer = PLAYER_DEATH_TIME;
		}
		else {
			p->base.state = PLAYER_HIT;
		}

		// 넉백 적용: 적 -> 플레이어 방향으로 밀려남
		float dx = p->base.x - enemy->base.x;
		float dy = p->base.y - enemy->base.y;
		float dist = sqrtf(dx * dx + dy * dy);

		if (dist > 0) {
			p->base.kx = (dx / dist) * KNOCKBACK_FORCE;
			p->base.ky = (dy / dist) * KNOCKBACK_FORCE;
			p->base.kTimer = KNOCKBACK_TIME;
		}

		if (p->base.hp < 0) p->base.hp = 0;
		return 1;
	}
	return 0;
}

// 총알 - 보스 충돌 처리 (넉백 없음)
int HandleBulletBossCollision(BULLET* bullet, BOSS* boss) {
	if (!bullet->isActive || boss->isActive == INACTIVE) return 0;

	// 보스 히트박스 = base.x/y 기준 BOSS_SIZE 크기
	if (IsObjectCollision(bullet->hitBoxX, bullet->hitBoxY, bullet->hitBoxW, bullet->hitBoxH,
		boss->base.hitBoxX, boss->base.hitBoxY, boss->base.hitBoxW, boss->base.hitBoxH)) {

		bullet->isActive = INACTIVE;
		boss->base.hp -= BULLET_DAMAGE;

		if (boss->base.hp <= 0) {
			boss->base.hp = 0;
			boss->base.state = BOSS_DEAD;
		}
		return 1;
	}
	return 0;
}

// 총알 - 잡몹 젤리 충돌 처리
int HandleBulleCatPawCollision(BULLET* bullet, CATPAW* catpaw) {
	if (!bullet->isActive || catpaw->isActive == INACTIVE) return 0;

	if (IsObjectCollision(bullet->hitBoxX, bullet->hitBoxY, bullet->hitBoxW, bullet->hitBoxH,
		catpaw->hitBoxX, catpaw->hitBoxY, catpaw->hitBoxW, catpaw->hitBoxH)) {

		catpaw->isActive = INACTIVE;

		return 1;
	}
	return 0;
}

// 츄르 - 보스 충돌 처리
int HandleChuruBossCollision(CHURU* churues, BOSS* boss) {
	if (!churues->isActive || boss->isActive == INACTIVE) return 0;

	if (IsObjectCollision(churues->x, churues->y, churues->width, churues->height,
		boss->base.hitBoxX, boss->base.hitBoxY, boss->base.hitBoxW, boss->base.hitBoxH)) {

		churues->isActive = INACTIVE;

		return 1;
	}
	return 0;
}

// 보스 PAW - 플레이어 충돌 처리 (catpaw와 동일 방식)
int HandleBossPawPlayerCollision(BOSS_PAW* bp, PLAYER* p) {
	if (!bp->isActive || p->invincibleTimer > 0) return 0;

	if (IsObjectCollision(bp->x, bp->y, BOSS_PAW_SIZE, BOSS_PAW_SIZE,
		p->base.hitBoxX, p->base.hitBoxY, p->base.hitBoxW, p->base.hitBoxH)) {

		bp->isActive = INACTIVE;
		p->base.hp -= CAT_PAW_DAMAGE;
		p->invincibleTimer = PLAYER_INVINCIBLE_TIME;

		if (p->base.hp <= 0) {
			p->base.hp = 0;
			p->base.state = PLAYER_DEAD;
			p->deathTimer = PLAYER_DEATH_TIME;
		}
		else {
			p->base.state = PLAYER_HIT;
		}

		// 넉백: 발사체 방향으로 밀려남
		float dist = sqrtf(bp->dx * bp->dx + bp->dy * bp->dy);
		if (dist > 0) {
			p->base.kx = (bp->dx / dist) * KNOCKBACK_FORCE;
			p->base.ky = (bp->dy / dist) * KNOCKBACK_FORCE;
			p->base.kTimer = KNOCKBACK_TIME;
		}

		if (p->base.hp < 0) p->base.hp = 0;
		return 1;
	}
	return 0;
}

// 보스 - 플레이어 일반 충돌 (반대 방향 넉백)
int HandleBossPlayerCollision(PLAYER* p) {
	if (boss.isActive == INACTIVE || p->invincibleTimer > 0) return 0;

	if (IsObjectCollision(boss.base.hitBoxX, boss.base.hitBoxY, boss.base.hitBoxW, boss.base.hitBoxH,
		p->base.hitBoxX, p->base.hitBoxY, p->base.hitBoxW, p->base.hitBoxH)) {

		p->base.hp -= BOSS_CONTACT_DAMAGE;
		p->invincibleTimer = PLAYER_INVINCIBLE_TIME;

		if (p->base.hp <= 0) {
			p->base.hp = 0;
			p->base.state = PLAYER_DEAD;
			p->deathTimer = PLAYER_DEATH_TIME;
		}
		else p->base.state = PLAYER_HIT;

		// 보스 → 플레이어 방향의 반대로 밀려남
		float dx = p->base.x - boss.base.x;
		float dy = p->base.y - boss.base.y;
		float dist = sqrtf(dx * dx + dy * dy);
		if (dist > 0) {
			p->base.kx = (dx / dist) * 10.0f;
			p->base.ky = (dy / dist) * BOSS_NORMAL_KNOCKBACK;
			p->base.kTimer = KNOCKBACK_TIME;
		}

		if (p->base.hp < 0) p->base.hp = 0;
		return 1;
	}
	return 0;
}

// 보스 대시 중 - 플레이어 충돌 (대시 방향으로 넉백, 보스는 계속 진행)
int HandleBossDashPlayerCollision(PLAYER* p) {
	if (boss.isActive == INACTIVE || p->invincibleTimer > 0) return 0;

	if (IsObjectCollision(boss.base.hitBoxX, boss.base.hitBoxY, boss.base.hitBoxW, boss.base.hitBoxH,
		p->base.hitBoxX, p->base.hitBoxY, p->base.hitBoxW, p->base.hitBoxH)) {

		p->base.hp -= BOSS_CONTACT_DAMAGE;
		p->invincibleTimer = PLAYER_INVINCIBLE_TIME;

		if (p->base.hp <= 0) {
			p->base.hp = 0;
			p->base.state = PLAYER_DEAD;
			p->deathTimer = PLAYER_DEATH_TIME;
		}
		else p->base.state = PLAYER_HIT;

		// 대시 방향으로 강하게 밀려남 (보스는 계속 진행 - 넉백 적용 없음)
		p->base.kx = boss.dashDirX * BOSS_DASH_KNOCKBACK;
		p->base.ky = boss.dashDirY * BOSS_DASH_KNOCKBACK;
		p->base.kTimer = KNOCKBACK_TIME;

		if (p->base.hp < 0) p->base.hp = 0;
		return 1;
	}
	return 0;
}

// 보스 점프 착지 - 플레이어 충돌 (랜덤 4방향 넉백)
int HandleBossJumpPlayerCollision(PLAYER* p) {
	if (boss.isActive == INACTIVE || p->invincibleTimer > 0) return 0;

	// 착지 범위: BOSS_JUMP_LAND_SIZE 크기의 원형 영역
	float dx = p->base.x - boss.base.x;
	float dy = p->base.y - boss.base.y;
	float dist = sqrtf(dx * dx + dy * dy);

	if (dist < BOSS_JUMP_LAND_SIZE / 2) {
		p->base.hp -= BOSS_CONTACT_DAMAGE;
		p->invincibleTimer = PLAYER_INVINCIBLE_TIME;

		if (p->base.hp <= 0) {
			p->base.hp = 0;
			p->base.state = PLAYER_DEAD;
			p->deathTimer = PLAYER_DEATH_TIME;
		}
		else p->base.state = PLAYER_HIT;

		// 랜덤 4방향 넉백
		int dir = rand() % 4;
		switch (dir) {
		case 0: p->base.kx = BOSS_JUMP_KNOCKBACK; p->base.ky = 0; break; // 오른쪽
		case 1: p->base.kx = -BOSS_JUMP_KNOCKBACK; p->base.ky = 0; break; // 왼쪽
		case 2: p->base.kx = 0; p->base.ky = BOSS_JUMP_KNOCKBACK; break; // 아래
		case 3: p->base.kx = 0; p->base.ky = -BOSS_JUMP_KNOCKBACK; break; // 위
		}
		p->base.kTimer = KNOCKBACK_TIME;

		if (p->base.hp < 0) p->base.hp = 0;
		return 1;
	}
	return 0;
}