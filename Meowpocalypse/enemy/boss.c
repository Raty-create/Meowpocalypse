#include "boss.h"

#define BOSS_SIZE (32 * 3)

enum { DEACTIVATE, ACTIVE };

void InitBoss() {
	boss.isActive = DEACTIVATE;
}

void SpawnBoss() {
	if (!boss.isActive) {
		//boss.base.x = 
		//boss.base.y = 
		boss.base.width = BOSS_SIZE;
		boss.base.height = BOSS_SIZE;
		//boss.base.hp = 
		//boss.base.state = 
	}
}