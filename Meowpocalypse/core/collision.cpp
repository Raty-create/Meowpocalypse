#include "map.h"
#include "config.h"
#include "collision.h"
#include "player.h"
#include "enemy.h"
#include "boss.h"
#include "sound.h"

#include <math.h>

// AABB 충돌 체크
BOOL IsObjectCollision(float ax, float ay, int aw, int ah, float bx, float by, int bw, int bh) {
	return (ax - aw / 2 < bx + bw / 2 &&
		ax + aw / 2 > bx - bw / 2 &&
		ay - ah / 2 < by + bh / 2 &&
		ay + ah / 2 > by - bh / 2);
}

// 플레이어 - 벽 및 장애물 충돌 체크
BOOL IsTileBlocked(float x, float y) {
	MAPDATA* m = &maps[currentMapType];

	int col = (int)((x - m->worldX) / TILE_SIZE);
	int row = (int)((y - m->worldY) / TILE_SIZE);

	if (row < 0 || row >= m->rows) return 1;
	if (col < 0 || col >= m->cols) return 1;

	int tile = m->tiles[row][col];
	return (tile == TILE_WALL || tile == TILE_OBSTACLE);
}

// 장애물에 막혀있는지 체크
BOOL IsObstacleBlocked(float x, float y) {
	for (int i = 0; i < OBSTACLE_LIMIT; i++) {
		if (!obstacles[i].isActive) continue;

		if (obstacles[i].type == OBS_SOLID) {
			float ohx = obstacles[i].base.hitBoxX;
			float ohy = obstacles[i].base.hitBoxY;
			float ohw = obstacles[i].base.hitBoxW / 2.0f;
			float ohh = obstacles[i].base.hitBoxH / 2.0f;

			if (x > ohx - ohw && x < ohx + ohw && y > ohy - ohh && y < ohy + ohh) {
				return TRUE;	// 막힘
			}
		}
	}

	return FALSE;	// 안 막힘
}

// 장애물 AABB 충돌 체크 (한 번에 검사)
BOOL IsObstacleAABBBlocked(float x, float y, int w, int h) {
	for (int i = 0; i < OBSTACLE_LIMIT; i++) {
		if (!obstacles[i].isActive) continue;

		if (obstacles[i].type == OBS_SOLID) {
			if (IsObjectCollision(x, y, w, h,
				obstacles[i].base.hitBoxX, obstacles[i].base.hitBoxY,
				obstacles[i].base.hitBoxW, obstacles[i].base.hitBoxH)) {
				return TRUE; // 막힘
			}
		}
	}
	return FALSE; // 안 막힘
}


// 특정 좌표(x, y)가 장애물 히트박스 안에 있는지 체크
BOOL IsPointInsideObstacle(float x, float y, OBSTACLE* ob) {
	if (!ob->isActive) return FALSE;

    float ohx = ob->base.hitBoxX;
	float ohy = ob->base.hitBoxY;
    float ohw = ob->base.hitBoxW / 2.0f;
    float ohh = ob->base.hitBoxH / 2.0f;

	return (x > ohx - ohw && x < ohx + ohw && y > ohy - ohh && y < ohy + ohh);
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

// 플레이어가 사이버 끈끈이 위에 있는지 체크
int IsPlayerOnHazard(PLAYER* p, OBSTACLE* ob) {
	float footX = p->base.x;
	float footY = p->base.y + (p->base.height / 1.5f);
	
	return IsPointInsideObstacle(footX, footY, ob);
}

// 잡몹이 사이버 끈끈이 위에 있는지 체크
int IsEnemyOnHazard(ENEMY* enemy, OBSTACLE* ob) {
	if (!enemy->isActive) return FALSE;

	float footX = enemy->base.x;
	float footY = enemy->base.y + (enemy->base.height * 0.2f);

	return IsPointInsideObstacle(footX, footY, ob);
}

// 플레이어 - 사이버 끈끈이 충돌 처리
void HandlePlayerCyberSlimeCollision() {
	player.speed = PLAYER_SPEED; // 기본 속도로 초기화

	if (player.boostTimer > 0) {
		player.speed *= BOOST_SPEED_MULTIPLIER;
	}

	for (int i = 0; i < OBSTACLE_LIMIT; i++) {
		if (!obstacles[i].isActive || obstacles[i].type != OBS_HAZARD) continue;

		if (IsPlayerOnHazard(&player, &obstacles[i])) {
			if (obstacles[i].subType == SUB_CYBER_SLIME) {
				player.speed *= 0.25f; // 끈끈이 효과 적용
				break; 
			}
		}
	}
}

// 잡몹 - 사이버 끈끈이 충돌 처리
void HandleEnemyCyberSlimeCollision(int enemyIdx) {
	enemies[enemyIdx].speedMultiplier = ENEMY_SPEED_MULTIPLIER; // 기본 배율로 초기화

	for (int i = 0; i < OBSTACLE_LIMIT; i++) {
		if (!obstacles[i].isActive || obstacles[i].type != OBS_HAZARD) continue;

		if (IsEnemyOnHazard(&enemies[enemyIdx], &obstacles[i])) {
			if (obstacles[i].subType == SUB_CYBER_SLIME) {
				enemies[enemyIdx].speedMultiplier = 0.4f;
				break;
			}
		}
	}
}

// 플레이어 - 벽/장애물과의 충돌 처리
void HandlePlayerWallCollision() {
	float playerNextX = player.base.x + player.base.dx;
	float playerNextY = player.base.y + player.base.dy;

	int playerHalfW = PLAYER_WIDTH / 2;
	int playerHalfH = PLAYER_HEIGHT / 2;
	if (!IsTileBlocked(playerNextX - playerHalfW, player.base.y + playerHalfH) &&
		!IsTileBlocked(playerNextX + playerHalfW, player.base.y + playerHalfH) &&
		!IsObstacleBlocked(playerNextX - playerHalfW, player.base.y + playerHalfH) &&
		!IsObstacleBlocked(playerNextX + playerHalfW, player.base.y + playerHalfH))
		player.base.x = player.base.hitBoxX = playerNextX;

	if (!IsTileBlocked(player.base.x - playerHalfW, playerNextY + playerHalfH) &&
		!IsTileBlocked(player.base.x + playerHalfW, playerNextY + playerHalfH) &&
		!IsObstacleBlocked(player.base.x - playerHalfW, playerNextY + playerHalfH) &&
		!IsObstacleBlocked(player.base.x + playerHalfW, playerNextY + playerHalfH))
		player.base.y = player.base.hitBoxY = playerNextY;
}

// 잡몹 - 벽/장애물과의 충돌 처리
void HandleEnemyWallCollision(int i) {
	if (!enemies[i].isActive || enemies[i].base.state == ENEMY_DEAD) return;

	float ex = enemies[i].base.x;
	float ey = enemies[i].base.y;
	float nextX = ex + enemies[i].base.dx;
	float nextY = ey + enemies[i].base.dy;

	int halfW = ENEMY_HITBOX_WIDTH / 2;
	int halfH = ENEMY_HITBOX_HEIGHT / 2;

	// X축 이동 체크
	if (!IsTileBlocked(nextX - halfW, ey + halfH) &&
		!IsTileBlocked(nextX + halfW, ey + halfH) &&
		!IsObstacleBlocked(nextX - halfW, ey + halfH) &&
		!IsObstacleBlocked(nextX + halfW, ey + halfH)) {
		enemies[i].base.x = enemies[i].base.hitBoxX = nextX;
	}
	else {
		if (enemies[i].base.state == ENEMY_MOVE) enemies[i].moveTimer = 0;
	}

	// Y축 이동 체크
	if (!IsTileBlocked(ex - halfW, nextY + halfH) &&
		!IsTileBlocked(ex + halfW, nextY + halfH) &&
		!IsObstacleBlocked(ex - halfW, nextY + halfH) &&
		!IsObstacleBlocked(ex + halfW, nextY + halfH)) {
		enemies[i].base.y = nextY;
		enemies[i].base.hitBoxY = nextY + (ENEMY_HEIGHT * 0.05f);
	}
	else {
		if (enemies[i].base.state == ENEMY_MOVE) enemies[i].moveTimer = 0;
	}
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
			PlaySFX(SFX_ENEMY_HIT);
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
			PlaySFX(SFX_PLAYER_HIT);
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
			PlaySFX(SFX_PLAYER_HIT);
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
	if (!bullet->isActive || boss->isActive == INACTIVE || boss->invincibleTimer > 0) return 0;

	// 보스 히트박스 = base.x/y 기준 BOSS_SIZE 크기
	if (IsObjectCollision(bullet->hitBoxX, bullet->hitBoxY, bullet->hitBoxW, bullet->hitBoxH,
		boss->base.hitBoxX, boss->base.hitBoxY, boss->base.hitBoxW, boss->base.hitBoxH)) {

		bullet->isActive = INACTIVE;
		boss->base.hp -= BULLET_DAMAGE;

		if (boss->base.hp <= 0) {
			boss->base.hp = 0;

			// 총알을 맞은 방향(발사체 반대 방향)을 바라보게 설정
			float tdx = -bullet->dx;
			float tdy = -bullet->dy;

			if (fabsf(tdx) > fabsf(tdy) * 2) {
				if (tdx > 0) boss->base.direction = DIR_RIGHT;
				else boss->base.direction = DIR_LEFT;
			}
			else if (fabsf(tdy) > fabsf(tdx) * 2) {
				if (tdy > 0) boss->base.direction = DIR_DOWN;
				else boss->base.direction = DIR_UP;
			}
			else {
				if (tdx > 0 && tdy > 0) boss->base.direction = DIR_DOWN_RIGHT;
				else if (tdx > 0 && tdy < 0) boss->base.direction = DIR_UP_RIGHT;
				else if (tdx < 0 && tdy > 0) boss->base.direction = DIR_DOWN_LEFT;
				else if (tdx < 0 && tdy < 0) boss->base.direction = DIR_UP_LEFT;
			}
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

	if (IsObjectCollision(bp->x, bp->y, BOSS_PAW_HITBOX_W, BOSS_PAW_HITBOX_H,
		p->base.hitBoxX, p->base.hitBoxY, p->base.hitBoxW, p->base.hitBoxH)) {

		bp->isActive = INACTIVE;
		p->base.hp -= BOSS_PAW_DAMAGE;
		p->invincibleTimer = PLAYER_INVINCIBLE_TIME;

		if (p->base.hp <= 0) {
			p->base.hp = 0;
			p->base.state = PLAYER_DEAD;
			p->deathTimer = PLAYER_DEATH_TIME;
		}
		else {
			p->base.state = PLAYER_HIT;
			PlaySFX(SFX_PLAYER_HIT);
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
		else {
			p->base.state = PLAYER_HIT;
			PlaySFX(SFX_PLAYER_HIT);
		}

		// 보스 -> 플레이어 방향의 반대로 밀려남
		float dx = p->base.x - boss.base.x;
		float dy = p->base.y - boss.base.y;
		float dist = sqrtf(dx * dx + dy * dy);
		if (dist > 0) {
			p->base.kx = (dx / dist) * BOSS_NORMAL_KNOCKBACK;
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

		p->base.hp -= BOSS_DASH_DAMAGE;
		p->invincibleTimer = PLAYER_INVINCIBLE_TIME;

		if (p->base.hp <= 0) {
			p->base.hp = 0;
			p->base.state = PLAYER_DEAD;
			p->deathTimer = PLAYER_DEATH_TIME;
		}
		else {
			p->base.state = PLAYER_HIT;
			PlaySFX(SFX_PLAYER_HIT);
		}

		// 대시 방향으로 강하게 밀려남 (보스는 계속 진행 - 넉백 적용 없음)
		p->base.kx = boss.dashDirX * BOSS_DASH_KNOCKBACK;
		p->base.ky = boss.dashDirY * BOSS_DASH_KNOCKBACK;
		p->base.kTimer = KNOCKBACK_TIME;

		if (p->base.hp < 0) p->base.hp = 0;
		return 1;
	}
	return 0;
}

// 보스 점프 착지 - 플레이어 충돌 (옆으로 긴 타원형 범위 체크)
int HandleBossJumpPlayerCollision(PLAYER* p) {
	if (boss.isActive == INACTIVE || p->invincibleTimer > 0 || boss.invincibleTimer > 0) return 0;

	// 타원형 범위 설정 (가로로 긴 형태)
	float rx = BOSS_JUMP_LAND_SIZE * BOSS_JUMP_LAND_SIZE_W_RATIO;  // 가로 반지름
	float ry = BOSS_JUMP_LAND_SIZE * BOSS_JUMP_LAND_SIZE_H_RATIO;  // 세로 반지름

	float playerBottomY = p->base.y + (p->base.height / 2.0f);

	float dx = p->base.x - boss.base.x;
	float dy = playerBottomY - (boss.base.y + JUMPWARN_Y_OFFSET);

	float marginX = p->base.hitBoxW * 0.5f;

	float finalRx = rx + marginX;

	// 타원 방정식: (x/rx)^2 + (y/ry)^2 < 1
	if ((dx * dx) / (finalRx * finalRx) + (dy * dy) / (ry * ry) < 1.0f) {
		p->base.hp -= BOSS_JUMP_DAMAGE;
		p->invincibleTimer = PLAYER_INVINCIBLE_TIME;

		if (p->base.hp <= 0) {
			p->base.hp = 0;
			p->base.state = PLAYER_DEAD;
			p->deathTimer = PLAYER_DEATH_TIME;
		}
		else {
			p->base.state = PLAYER_HIT;
			PlaySFX(SFX_PLAYER_HIT);
		}

		// 보스 -> 플레이어 방향의 반대로 밀려남
		float dx = p->base.x - boss.base.x;
		float dy = p->base.y - boss.base.y;
		float dist = sqrtf(dx * dx + dy * dy);
		if (dist > 0) {
			p->base.kx = (dx / dist) * BOSS_JUMP_KNOCKBACK;
			p->base.ky = (dy / dist) * BOSS_JUMP_KNOCKBACK;
			p->base.kTimer = KNOCKBACK_TIME;
		}

		if (p->base.hp < 0) p->base.hp = 0;
		return 1;
	}
	return 0;
}