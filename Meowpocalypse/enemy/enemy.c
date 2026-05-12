#include "enemy.h"
#include "config.h"
#include "map.h"
#include "collision.h"
#include "player.h"
#include <stdlib.h>
#include <math.h>

enum { INACTIVE, ACTIVE };

ENEMY enemies[ENEMY_LIMIT];
STONE stones[STONE_LIMIT];

void InitEnemy() {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		enemies[i].isActive = INACTIVE;
		enemies[i].base.width = ENEMY_SIZE;
		enemies[i].base.height = ENEMY_SIZE;
		enemies[i].state = ENEMY_IDLE;
		enemies[i].shootTimer = 0;
	}
	for (int i = 0; i < STONE_LIMIT; i++) {
		stones[i].isActive = 0;
	}
}

void ClearEnemies() {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		enemies[i].isActive = INACTIVE;
	}
	for (int i = 0; i < STONE_LIMIT; i++) {
		stones[i].isActive = 0;
	}
}

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

		for (int i = 0; i < ENEMY_LIMIT; i++) {
			if (!enemies[i].isActive) {
				enemies[i].isActive = ACTIVE;
				enemies[i].base.x = spawnX;
				enemies[i].base.y = spawnY;
				enemies[i].state = ENEMY_IDLE;
				enemies[i].shootTimer = rand() % STONE_INTERVAL;
				spawned++;
				break;
				//enemies[i].base.hp = 
				//enemies[i].base.state = 
			}
		}
	}
}
void SpawnStone(float fromX, float fromY) {
	float dx = player.base.x - fromX;
	float dy = player.base.y - fromY;
	float dist = sqrtf(dx * dx + dy * dy);
	if (dist == 0) return;

	for (int i = 0; i < STONE_LIMIT; i++) {
		if (!stones[i].isActive) {
			stones[i].isActive = 1;
			stones[i].x = fromX;
			stones[i].y = fromY;
			stones[i].dx = (dx / dist) * STONE_SPEED;
			stones[i].dy = (dy / dist) * STONE_SPEED;
			break;
		}
	}
}

void UpdateEnemies() {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		if (!enemies[i].isActive) continue;

		float ex = enemies[i].base.x;
		float ey = enemies[i].base.y;
		float dx = player.base.x - ex;
		float dy = player.base.y - ey;
		float dist = sqrtf(dx * dx + dy * dy);

		if (dist < ENEMY_CHASE_RANGE) {
			// 잡몹 - 플레이어 근접
			enemies[i].state = ENEMY_CHASE;
			enemies[i].shootTimer = 0;

			if (dist > 0) {
				float nx = (dx / dist) * ENEMY_SPEED;
				float ny = (dy / dist) * ENEMY_SPEED;
				int half = ENEMY_SIZE / 2;

				float nextX = ex + nx;
				if (!IsTileWall(nextX - half, ey - half) &&
					!IsTileWall(nextX + half, ey - half) &&
					!IsTileWall(nextX - half, ey + half) &&
					!IsTileWall(nextX + half, ey + half)) enemies[i].base.x = nextX;

				float nextY = ey + ny;
				if (!IsTileWall(ex - half, nextY - half) &&
					!IsTileWall(ex + half, nextY - half) &&
					!IsTileWall(ex - half, nextY + half) &&
					!IsTileWall(ex + half, nextY + half)) enemies[i].base.y = nextY;
			}
		}
		else {
			// 원거리
			enemies[i].state = ENEMY_RANGED;
			enemies[i].shootTimer++;
			if (enemies[i].shootTimer >= STONE_INTERVAL) {
				SpawnStone(ex, ey);
				enemies[i].shootTimer = 0;
			}
		}
	}
	for (int i = 0; i < STONE_LIMIT; i++) {
		if (!stones[i].isActive) continue;

		stones[i].x += stones[i].dx;
		stones[i].y += stones[i].dy;

		if (IsTileWall(stones[i].x, stones[i].y)) {
			stones[i].isActive = 0;
			continue;
		}
		float dx = stones[i].x - player.base.x;
		float dy = stones[i].y - player.base.y;
		if (sqrtf(dx * dx + dy * dy) < PLAYER_SIZE / 1.5f) {
			stones[i].isActive = 0;
		}
	}
}