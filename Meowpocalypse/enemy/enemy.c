#include "enemy.h"
#include "config.h"
#include "map.h"
#include "collision.h"
#include <stdlib.h>

enum { INACTIVE, ACTIVE };

ENEMY enemies[ENEMY_LIMIT];

void InitEnemy() {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		enemies[i].isActive = INACTIVE;
		enemies[i].base.width = ENEMY_SIZE;
		enemies[i].base.height = ENEMY_SIZE;
	}
}

void ClearEnemies() {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		enemies[i].isActive = INACTIVE;
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
		int spawnX = m->worldX + (rand() % m->cols) * TILE_SIZE;
		int spawnY = m->worldY + (rand() % m->rows) * TILE_SIZE;

		if (IsTileWall(spawnX, spawnY)) continue;

		for (int i = 0; i < ENEMY_LIMIT; i++) {
			if (!enemies[i].isActive) {
				enemies[i].isActive = ACTIVE;
				enemies[i].base.x = spawnX;
				enemies[i].base.y = spawnY;
				spawned++;
				break;
				//enemies[i].base.hp = 
				//enemies[i].base.state = 
			}
		}
	}
}