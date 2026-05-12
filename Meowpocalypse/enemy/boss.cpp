#include "boss.h"
#include "config.h"

enum { INACTIVE, ACTIVE };

BOSS boss;

void InitBoss() {
	boss.isActive = INACTIVE;
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