#include <stdlib.h>
#include <math.h>

#include "enemy.h"
#include "config.h"
#include "map.h"
#include "collision.h"
#include "player.h"
#include "object.h"
#include "boss.h"

BOSS boss;

void InitBoss() {
		boss.isActive = INACTIVE;
		boss.base.width = BOSS_SIZE;
		boss.base.height = BOSS_SIZE;
		boss.base.state = BOSS_IDLE;
		boss.base.dx = 0;
		boss.base.dy = 0;
		boss.base.hp = BOSS_HP;
}

void SpawnBoss(MAP_TYPE type) {
	if (type == MAP_WAITING ||
		type == MAP_FIRST_HALLWAY ||
		type == MAP_SECOND_HALLWAY ||
		type == MAP_THIRD_HALLWAY) return;

	MAPDATA* m = &maps[type];

	float spawnX = m->worldX + (BOSSMAP_COLS / 2) * TILE_SIZE;
	float spawnY = m->worldY + (BOSSMAP_ROWS / 2) * TILE_SIZE;

	if (type == MAP_FIRST_BOSS && boss.base.hp == BOSS_HP) {

		if (boss.isActive == INACTIVE) {
			boss.isActive = ACTIVE;
			boss.base.x = spawnX;
			boss.base.y = spawnY;
			boss.base.state = BOSS_IDLE;
			boss.base.dx = 0;
			boss.base.dy = 0;
		}
	}
	else if (type == MAP_SECOND_BOSS && boss.base.hp == BOSS_HP / 2) {

		if (boss.isActive == INACTIVE) {
			boss.isActive = ACTIVE;
			boss.base.x = spawnX;
			boss.base.y = spawnY;
			boss.base.state = BOSS_IDLE;
			boss.base.dx = 0;
			boss.base.dy = 0;
		}
	}
	else if (type == MAP_THIRD_BOSS && boss.base.hp == (BOSS_HP / 2) / 2) {

		if (boss.isActive == INACTIVE) {
			boss.isActive = ACTIVE;
			boss.base.x = spawnX;
			boss.base.y = spawnY;
			boss.base.state = BOSS_IDLE;
			boss.base.dx = 0;
			boss.base.dy = 0;
		}
	}
}