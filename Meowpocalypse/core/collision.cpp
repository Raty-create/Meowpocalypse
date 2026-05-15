#include "map.h"
#include "config.h"
#include "collision.h"
#include "player.h"
#include "enemy.h"

#include <math.h>

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

	float point[4][2] = {
		{player.base.x - PLAYER_SIZE / 2 + 1, player.base.y - PLAYER_SIZE / 2 + 1},
		{player.base.x - PLAYER_SIZE / 2 - 1, player.base.y - PLAYER_SIZE / 2 + 1},
		{player.base.x - PLAYER_SIZE / 2 + 1, player.base.y - PLAYER_SIZE / 2 - 1},
		{player.base.x - PLAYER_SIZE / 2 - 1, player.base.y - PLAYER_SIZE / 2 - 1}
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

// 잡몹-잡몹 충돌 처리
int IsOverlapWithEnemy(float x, float y) {
	for (int j = 0; j < ENEMY_LIMIT; j++) {
		if (!enemies[j].isActive) continue;
		float dx = enemies[j].base.x - x;
		float dy = enemies[j].base.y - y;
		if (sqrtf(dx * dx + dy * dy) < ENEMY_SIZE) return 1;
	}
	return 0;
}

int IsObjectCollision(float ax, float ay, int aw, int ah, float bx, float by, int bw, int bh) {
	return (ax - aw / 2 < bx + bw / 2 &&
		ax + aw / 2 > bx - bw / 2 &&
		ay - ah / 2 < by + bh / 2 &&
		ay + ah / 2 > by - bh / 2);
}

int HandleBulletEnemyCollision(BULLET* bullet, ENEMY* enemy) {
	if (!bullet->isActive || !enemy->isActive) return 0;

	if (IsObjectCollision(bullet->x, bullet->y, bullet->width, bullet->height,
		enemy->base.hitBoxX, enemy->base.hitBoxY, enemy->base.hitBoxW, enemy->base.hitBoxH)) {

		bullet->isActive = INACTIVE;
		enemy->base.hp -= BULLET_DAMAGE;

		// 넉백 적용: 총알의 이동 방향으로 밀려남
		float dist = sqrtf(bullet->dx * bullet->dx + bullet->dy * bullet->dy);
		if (dist > 0) {
			enemy->base.kx = (bullet->dx / dist) * KNOCKBACK_FORCE;
			enemy->base.ky = (bullet->dy / dist) * KNOCKBACK_FORCE;
			enemy->base.kTimer = KNOCKBACK_TIME;
		}

		if (enemy->base.hp <= 0) {
			enemy->isActive = INACTIVE;
		}
		return 1;
	}
	return 0;
}

int HandleCatPawPlayerCollision(CATPAW* cp, PLAYER* p) {
	if (!cp->isActive || p->invincibleTimer > 0) return 0;

	// CatPaw의 크기는 설정값 사용
	if (IsObjectCollision(cp->x, cp->y, CAT_PAW_SIZE, CAT_PAW_SIZE,
		p->base.hitBoxX, p->base.hitBoxY, p->base.hitBoxW, p->base.hitBoxH)) {

		cp->isActive = INACTIVE;
		p->base.hp -= CAT_PAW_DAMAGE;
		p->invincibleTimer = PLAYER_INVINCIBLE_TIME;

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