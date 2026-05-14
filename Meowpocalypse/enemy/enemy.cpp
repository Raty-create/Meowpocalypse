#include <stdlib.h>
#include <math.h>

#include "enemy.h"
#include "config.h"
#include "map.h"
#include "collision.h"
#include "player.h"
#include "object.h"

ENEMY enemies[ENEMY_LIMIT];
CATPAW catpaw[CAT_PAW_LIMIT];

// 잡몹 생성
void InitEnemy() {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		enemies[i].isActive = INACTIVE;
		enemies[i].base.width = ENEMY_SIZE;
		enemies[i].base.height = ENEMY_SIZE;
		enemies[i].base.state = ENEMY_IDLE;
		enemies[i].shootTimer = 0;
		enemies[i].moveTimer = 0;
		enemies[i].base.dx = 0;
		enemies[i].base.dy = 0;
	}
	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		catpaw[i].isActive = INACTIVE;
	}
}

void ClearEnemies() {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		enemies[i].isActive = INACTIVE;
	}
	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		catpaw[i].isActive = INACTIVE;
	}
}

// 잡몹 스폰
void SpawnEnemy(MAP_TYPE type, int count) {
	if (type == MAP_WAITING ||
		type == MAP_FIRST_BOSS ||
		type == MAP_SECOND_BOSS ||
		type == MAP_THIRD_BOSS) return;

	MAPDATA* m = &maps[type];

	int spawned = 0;
	int attempts = 0;

	while(spawned < count && attempts < 1000){
		attempts++;
		int spawnX = m->worldX + (rand() % (m->cols - ENEMY_COLS_SPAWN_MARGIN) + ENEMY_COLS_SPAWN_MARGIN) * TILE_SIZE;
		int spawnY = m->worldY + (rand() % (m->rows - ENEMY_ROWS_SPAWN_MARGIN) + ENEMY_ROWS_SPAWN_MARGIN) * TILE_SIZE;

		if (IsTileWall(spawnX, spawnY)) continue;
		if (IsOverlapWithEnemy(spawnX, spawnY)) continue;

		for (int i = 0; i < ENEMY_LIMIT; i++) {
			if (!enemies[i].isActive) {
				enemies[i].isActive = ACTIVE;
				enemies[i].base.x = spawnX;
				enemies[i].base.y = spawnY;
				enemies[i].base.state = ENEMY_IDLE;
				enemies[i].shootTimer = rand() % CAT_PAW_INTERVAL;
				enemies[i].moveTimer = rand() % ENEMY_MOVE;
				enemies[i].base.dx = 0;
				enemies[i].base.dy = 0;
				spawned++;
				break;
				//enemies[i].base.hp = 
			}
		}
	}
}

// 잡몹 공격 생성
void SpawnCatPaw(float fromX, float fromY) {
	float dx = player.base.x - fromX;
	float dy = player.base.y - fromY;
	float dist = sqrtf(dx * dx + dy * dy);
	if (dist == 0) return;

	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		if (!catpaw[i].isActive) {
			catpaw[i].isActive = ACTIVE;
			catpaw[i].x = fromX;
			catpaw[i].y = fromY;
			catpaw[i].dx = (dx / dist) * CAT_PAW_SPEED;
			catpaw[i].dy = (dy / dist) * CAT_PAW_SPEED;
			break;
		}
	}
}

// 잡몹 이동 및 공격 업데이트
void UpdateEnemies() {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		if (!enemies[i].isActive) continue;

		float ex = enemies[i].base.x;
		float ey = enemies[i].base.y;
		float dx = player.base.x - ex;
		float dy = player.base.y - ey;
		float dist = sqrtf(dx * dx + dy * dy);

		if (enemies[i].base.state == ENEMY_IDLE) {
			enemies[i].moveTimer++;
		}

		if (dist < ENEMY_CHASE_RANGE) {
			// 잡몹 - 플레이어 근접
			enemies[i].base.state = ENEMY_CHASE;
			enemies[i].shootTimer = 0;

			if (dist > 0) {
				float nx = (dx / dist) * (ENEMY_SPEED * 1.5f);
				float ny = (dy / dist) * (ENEMY_SPEED * 1.5f);
				int half = ENEMY_SIZE / 2;

				float nextX = ex + nx;
				float nextY = ey + ny;
				if (!IsTileWall(nextX - half, ey - half) &&
					!IsTileWall(nextX + half, ey - half) &&
					!IsTileWall(nextX - half, ey + half) &&
					!IsTileWall(nextX + half, ey + half)) enemies[i].base.x = nextX;

				if (!IsTileWall(ex - half, nextY - half) &&
					!IsTileWall(ex + half, nextY - half) &&
					!IsTileWall(ex - half, nextY + half) &&
					!IsTileWall(ex + half, nextY + half)) enemies[i].base.y = nextY;
			}
		}
		else {
			// 원거리
			enemies[i].base.state = ENEMY_RANGED;
			enemies[i].shootTimer++;
			if (enemies[i].shootTimer >= CAT_PAW_INTERVAL) {
				SpawnCatPaw(ex, ey);
				enemies[i].shootTimer = 0;
			}
			enemies[i].moveTimer--;
			if (enemies[i].moveTimer <= 0) {
				enemies[i].moveTimer = 120 + rand() % 60; // 120~180프레임 (2~3초)

				int dir = rand() % 4;
				switch (dir) {
				case 0: enemies[i].base.dx = ENEMY_SPEED; enemies[i].base.dy = 0; break;
				case 1: enemies[i].base.dx = -ENEMY_SPEED; enemies[i].base.dy = 0;  break;
				case 2: enemies[i].base.dx = 0; enemies[i].base.dy = ENEMY_SPEED; break;
				case 3: enemies[i].base.dx = 0; enemies[i].base.dy = -ENEMY_SPEED; break;
				}
			}

			// 벽 충돌 체크 후 이동
			int half = ENEMY_SIZE / 2;

			float nextX = ex + enemies[i].base.dx;
			if (!IsTileWall(nextX - half, ey - half) &&
				!IsTileWall(nextX + half, ey - half) &&
				!IsTileWall(nextX - half, ey + half) &&
				!IsTileWall(nextX + half, ey + half))
				enemies[i].base.x = nextX;
			else
				enemies[i].moveTimer = 0; // 벽 충돌 시 즉시 방향 전환

			float nextY = ey + enemies[i].base.dy;
			if (!IsTileWall(ex - half, nextY - half) &&
				!IsTileWall(ex + half, nextY - half) &&
				!IsTileWall(ex - half, nextY + half) &&
				!IsTileWall(ex + half, nextY + half))
				enemies[i].base.y = nextY;
			else
				enemies[i].moveTimer = 0; // 벽 충돌 시 즉시 방향 전환
		}
	}

	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		if (!catpaw[i].isActive) continue;

		catpaw[i].x += catpaw[i].dx;
		catpaw[i].y += catpaw[i].dy;

		if (IsTileWall(catpaw[i].x, catpaw[i].y)) {
			catpaw[i].isActive = 0;
			continue;
		}
		float dx = catpaw[i].x - player.base.x;
		float dy = catpaw[i].y - player.base.y;
		if (sqrtf(dx * dx + dy * dy) < PLAYER_SIZE / 1.5f) {
			catpaw[i].isActive = 0;
		}
	}
}