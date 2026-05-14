#include "boss.h"
#include "config.h"
#include "map.h"

typedef enum { INACTIVE, ACTIVE } BOSS_STATE;

BOSS boss;

void InitBoss() {
	boss.isActive = INACTIVE;
}

void SpawnBoss(MAP_TYPE type) {
	MAPDATA* m = &maps[type];

	if (type == MAP_WAITING ||
		type == MAP_FIRST_HALLWAY ||
		type == MAP_SECOND_HALLWAY ||
		type == MAP_THIRD_HALLWAY) return;

	if (!boss.isActive) {
		boss.isActive = ACTIVE;
		boss.base.x = m->worldX + 50;
		boss.base.y = m->worldY + 50;
		boss.base.width = BOSS_SIZE;
		boss.base.height = BOSS_SIZE;
		//boss.base.hp = 
		//boss.base.state = 
	}
}