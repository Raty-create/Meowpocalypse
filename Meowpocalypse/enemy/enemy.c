#include "enemy.h"
#include "config.h"

enum { DEACTIVATE, ACTIVE };

ENEMY enemies[ENEMY_LIMIT];

void InitEnemy() {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		enemies[i].isActive = DEACTIVATE;
	}
}

void SpawnEnemy() {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		if (!enemies[i].isActive) {
			enemies[i].isActive = ACTIVE;
			//enemies[i].base.x = 
			//enemies[i].base.y = 
			enemies[i].base.width = ENEMY_SIZE;
			enemies[i].base.height = ENEMY_SIZE;
			//enemies[i].base.hp = 
			//enemies[i].base.state = 
		}
	}
}