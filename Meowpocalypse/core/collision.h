#pragma once

#include "object.h"
#include "player.h"
#include "enemy.h"
#include "bullet.h"
#include "boss.h"
#include "obstacle.h"

BOOL IsObjectCollision(float ax, float ay, int aw, int ah, float bx, float by, int bw, int bh);		// AABB 충돌 체크 함수
BOOL IsTileBlocked(float x, float y);
BOOL IsObstacleBlocked(float x, float y);
BOOL IsPointInsideObstacle(float x, float y, OBSTACLE* ob);

int IsPlayerOnDoor();
int IsPlayerOnHazard(PLAYER* p, OBSTACLE* ob);
int IsEnemyOnHazard(ENEMY* enemy, OBSTACLE* ob);
void HandlePlayerCyberSlimeCollision();
void HandleEnemyCyberSlimeCollision(int enemyIdx);

void HandlePlayerWallCollision();
void HandleEnemyWallCollision(int enemyIdx);
int IsOverlapWithEnemy(float x, float y);
int HandleBulletEnemyCollision(BULLET* bullet, ENEMY* enemy);
int HandleBulletBossCollision(BULLET* bullet, BOSS* boss);
int HandleBulleCatPawCollision(BULLET* bullet, CATPAW* catpaw);
int HandleChuruBossCollision(CHURU* churues, BOSS* boss);
int HandleCatPawPlayerCollision(CATPAW* cp, PLAYER* p);
int HandleEnemyPlayerCollision(ENEMY* enemy, PLAYER* p);
int HandleBossPawPlayerCollision(BOSS_PAW* bp, PLAYER* p);

int HandleBossPlayerCollision(PLAYER* p);       // 일반: 반대 방향 넉백
int HandleBossDashPlayerCollision(PLAYER* p);   // 대시 중: 대시 방향 넉백, 보스는 계속 진행
int HandleBossJumpPlayerCollision(PLAYER* p);   // 점프 착지: 반대 방향 넉백