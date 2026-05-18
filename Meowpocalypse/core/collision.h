#pragma once

#include "object.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"

int IsTileWall(float x, float y);
int IsPlayerOnDoor();
int IsOverlapWithEnemy(float x, float y);
int IsObjectCollision(float ax, float ay, int aw, int ah, float bx, float by, int bw, int bh);
int HandleBulletEnemyCollision(BULLET* bullet, ENEMY* enemy);
int HandleCatPawPlayerCollision(CATPAW* cp, PLAYER* p);
int HandleEnemyPlayerCollision(ENEMY* enemy, PLAYER* p);