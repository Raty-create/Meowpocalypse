#include "game.h"
#include "player/player.h"
#include "enemy/enemy.h"
#include "enemy/boss.h"

void Update() {
	UpdatePlayer();
	UpdateEnemy();
	UpdateBullet();
}