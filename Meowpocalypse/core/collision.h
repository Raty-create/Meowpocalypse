#pragma once

#include "object.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "boss.h"

int IsTileWall(float x, float y);
int IsPlayerOnDoor();
void HandlePlayerCollision();
int IsOverlapWithEnemy(float x, float y);
int IsObjectCollision(float ax, float ay, int aw, int ah, float bx, float by, int bw, int bh);
int HandleBulletEnemyCollision(BULLET* bullet, ENEMY* enemy);
int HandleBulletBossCollision(BULLET* bullet, BOSS* boss);
int HandleChuruBossCollision(CHURU* churues, BOSS* boss);
int HandleCatPawPlayerCollision(CATPAW* cp, PLAYER* p);
int HandleEnemyPlayerCollision(ENEMY* enemy, PLAYER* p);
int HandleBossPawPlayerCollision(BOSS_PAW* bp, PLAYER* p);

int HandleBossPlayerCollision(PLAYER* p);       // 일반: 반대 방향 넉백
int HandleBossDashPlayerCollision(PLAYER* p);   // 대시 중: 대시 방향 넉백, 보스는 계속 진행
int HandleBossJumpPlayerCollision(PLAYER* p);   // 점프 착지: 랜덤 4방향 넉백