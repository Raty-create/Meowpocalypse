#include <stdlib.h>
#include <math.h>

#include "enemy.h"
#include "config.h"
#include "map.h"
#include "collision.h"
#include "player.h"
#include "object.h"

ENEMY enemies[ENEMY_LIMIT];
CATPAW catpaw[CAT_PAW_LIMIT];

// 잡몹 생성
void InitEnemy() {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		enemies[i].isActive = INACTIVE;
		enemies[i].base.width = enemies[i].base.height = ENEMY_SIZE;
		enemies[i].base.hitBoxW = enemies[i].base.hitBoxH = ENEMY_HITBOX_SIZE;
		enemies[i].base.state = ENEMY_IDLE;
		enemies[i].base.direction = DIR_DOWN;
		enemies[i].shootTimer = 0;
		enemies[i].moveTimer = 0;
		enemies[i].base.dx = 0;
		enemies[i].base.dy = 0;
		enemies[i].base.kx = 0;
		enemies[i].base.ky = 0;
		enemies[i].base.kTimer = 0;
	}
	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		catpaw[i].isActive = INACTIVE;
	}
}

void ClearEnemies() {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		enemies[i].isActive = INACTIVE;
	}
	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		catpaw[i].isActive = INACTIVE;
	}
}

// 잡몹 스폰
void SpawnEnemy(MAP_TYPE type, int count) {
	if (type == MAP_WAITING ||
		type == MAP_FIRST_BOSS ||
		type == MAP_SECOND_BOSS ||
		type == MAP_THIRD_BOSS) return;

	MAPDATA* m = &maps[type];

	int spawned = 0;
	int attempts = 0;

	while (spawned < count && attempts < 1000) {
		attempts++;
		float spawnX = m->worldX + (rand() % (m->cols - ENEMY_COLS_SPAWN_MARGIN) + ENEMY_COLS_SPAWN_MARGIN) * TILE_SIZE;
		float spawnY = m->worldY + (rand() % (m->rows - ENEMY_ROWS_SPAWN_MARGIN) + ENEMY_ROWS_SPAWN_MARGIN) * TILE_SIZE;

		if (IsTileWall(spawnX, spawnY)) continue;
		if (IsOverlapWithEnemy(spawnX, spawnY)) continue;

		for (int i = 0; i < ENEMY_LIMIT; i++) {
			if (!enemies[i].isActive) {
				enemies[i].isActive = ACTIVE;
				enemies[i].base.x = enemies[i].base.hitBoxX = spawnX;
				enemies[i].base.y = enemies[i].base.hitBoxY = spawnY;
				enemies[i].base.state = ENEMY_IDLE;
				enemies[i].base.direction = DIR_DOWN;
				enemies[i].shootTimer = rand() % CAT_PAW_INTERVAL;
				enemies[i].moveTimer = rand() % ENEMY_MOVE;
				enemies[i].base.dx = 0;
				enemies[i].base.dy = 0;
				enemies[i].base.hp = ENEMY_HP;
				spawned++;
				break;
			}
		}
	}
}

// 잡몹 공격 생성
void SpawnCatPaw(float fromX, float fromY) {
	float dx = player.base.x - fromX;
	float dy = player.base.y - fromY;
	float dist = sqrtf(dx * dx + dy * dy);
	if (dist == 0) return;

	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		if (!catpaw[i].isActive) {
			catpaw[i].isActive = ACTIVE;
			catpaw[i].x = fromX;
			catpaw[i].y = fromY;
			catpaw[i].dx = (dx / dist) * CAT_PAW_SPEED;
			catpaw[i].dy = (dy / dist) * CAT_PAW_SPEED;
			break;
		}
	}
}

// 잡몹 이동 및 공격 업데이트
void UpdateEnemies() {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		if (!enemies[i].isActive) continue;

		// 사망 상태 처리
		if (enemies[i].base.state == ENEMY_DEAD) {
			enemies[i].deathTimer--;
			if (enemies[i].deathTimer <= 0) {
				enemies[i].isActive = INACTIVE;
			}
			continue; // 사망 중에는 이동/공격 안함
		}

		float ex = enemies[i].base.x;
		float ey = enemies[i].base.y;

		// 넉백 및 히트 상태 처리
		if (enemies[i].base.kTimer > 0) {
			enemies[i].base.state = ENEMY_HIT; // 넉백 중에는 HIT 상태

			float nextX = ex + enemies[i].base.kx;
			float nextY = ey + enemies[i].base.ky;
			int half = ENEMY_SIZE / 2;

			// 넉백 중에는 플레이어(공격자) 쪽을 바라봄
			float toPlayerX = player.base.x - ex;
			float toPlayerY = player.base.y - ey;

			if (fabsf(toPlayerX) > fabsf(toPlayerY)) {
				if (toPlayerX > 0) enemies[i].base.direction = DIR_RIGHT;
				else enemies[i].base.direction = DIR_LEFT;
			}
			else {
				if (toPlayerY > 0) enemies[i].base.direction = DIR_DOWN;
				else enemies[i].base.direction = DIR_UP;
			}

			if (!IsTileWall(nextX - half, ey - half) &&
				!IsTileWall(nextX + half, ey - half) &&
				!IsTileWall(nextX - half, ey + half) &&
				!IsTileWall(nextX + half, ey + half)) enemies[i].base.x = enemies[i].base.hitBoxX = nextX;

			if (!IsTileWall(ex - half, nextY - half) &&
				!IsTileWall(ex + half, nextY - half) &&
				!IsTileWall(ex - half, nextY + half) &&
				!IsTileWall(ex + half, nextY + half)) enemies[i].base.y = enemies[i].base.hitBoxY = nextY;

			enemies[i].base.kTimer--;
			continue; // 넉백 중에는 일반 이동 안함
		}

		float dx = player.base.x - ex;
		float dy = player.base.y - ey;
		float dist = sqrtf(dx * dx + dy * dy);

		if (enemies[i].base.state == ENEMY_IDLE) {
			enemies[i].moveTimer++;
		}

		if (dist < ENEMY_CHASE_RANGE) {
			// 잡몹 - 플레이어 근접
			if (dist < ENEMY_STOP_DISTANCE + 5.0f) {
				enemies[i].base.state = ENEMY_MELEE;
			}
			else {
				enemies[i].base.state = ENEMY_CHASE;
			}
			enemies[i].shootTimer = 0;

			// 플레이어와 충돌 체크 (근접 데미지)
			HandleEnemyPlayerCollision(&enemies[i], &player);

			if (dist > 0) {
				// 플레이어를 향한 기본 추격 벡터 (일정 거리 유지)
				float nx = 0, ny = 0;
				if (dist > ENEMY_STOP_DISTANCE) {
					nx = (dx / dist) * (ENEMY_SPEED * 1.5f);
					ny = (dy / dist) * (ENEMY_SPEED * 1.5f);
				}
				else if (dist < ENEMY_STOP_DISTANCE - 5.0f) {
					nx = -(dx / dist) * (ENEMY_SPEED * 0.5f);
					ny = -(dy / dist) * (ENEMY_SPEED * 0.5f);
				}

				// 다른 적들과의 겹침 방지 (Separation)
				float sepX = 0, sepY = 0;
				for (int j = 0; j < ENEMY_LIMIT; j++) {
					if (i == j || !enemies[j].isActive) continue;

					float diffX = ex - enemies[j].base.x;
					float diffY = ey - enemies[j].base.y;
					float eDist = sqrtf(diffX * diffX + diffY * diffY);

					// 적들끼리 일정 거리보다 가까워지면 반발력 발생
					if (eDist < ENEMY_SIZE && eDist > 0) {
						float force = (ENEMY_SIZE - eDist) / ENEMY_SIZE;
						sepX += (diffX / eDist) * force * ENEMY_SPEED;
						sepY += (diffY / eDist) * force * ENEMY_SPEED;
					}
				}

				// 최종 이동 벡터 합산
				float finalNX = nx + sepX;
				float finalNY = ny + sepY;

				// 방향 업데이트
				if (finalNX > 0) {
					if (finalNY > 0) enemies[i].base.direction = DIR_DOWN_RIGHT;
					else if (finalNY < 0) enemies[i].base.direction = DIR_UP_RIGHT;
					else enemies[i].base.direction = DIR_RIGHT;
				}
				else if (finalNX < 0) {
					if (finalNY > 0) enemies[i].base.direction = DIR_DOWN_LEFT;
					else if (finalNY < 0) enemies[i].base.direction = DIR_UP_LEFT;
					else enemies[i].base.direction = DIR_LEFT;
				}
				else {
					if (finalNY > 0) enemies[i].base.direction = DIR_DOWN;
					else if (finalNY < 0) enemies[i].base.direction = DIR_UP;
				}

				int half = ENEMY_SIZE / 2;
				float nextX = ex + finalNX;
				float nextY = ey + finalNY;

				if (!IsTileWall(nextX - half, ey - half) &&
					!IsTileWall(nextX + half, ey - half) &&
					!IsTileWall(nextX - half, ey + half) &&
					!IsTileWall(nextX + half, ey + half)) enemies[i].base.x = enemies[i].base.hitBoxX = nextX;

				if (!IsTileWall(ex - half, nextY - half) &&
					!IsTileWall(ex + half, nextY - half) &&
					!IsTileWall(ex - half, nextY + half) &&
					!IsTileWall(ex + half, nextY + half)) enemies[i].base.y = enemies[i].base.hitBoxY = nextY;
			}
		}
		else {
			// 원거리
			enemies[i].base.state = ENEMY_RANGED;
			enemies[i].shootTimer++;
			if (enemies[i].shootTimer >= CAT_PAW_INTERVAL) {
				SpawnCatPaw(ex, ey);
				enemies[i].shootTimer = 0;
			}
			enemies[i].moveTimer--;
			if (enemies[i].moveTimer <= 0) {
				enemies[i].moveTimer = 120 + rand() % 60; // 120~180프레임 (2~3초)

				int dir = rand() % 4;
				switch (dir) {
				case 0: enemies[i].base.dx = ENEMY_SPEED; enemies[i].base.dy = 0; enemies[i].base.direction = DIR_RIGHT; break;
				case 1: enemies[i].base.dx = -ENEMY_SPEED; enemies[i].base.dy = 0; enemies[i].base.direction = DIR_LEFT; break;
				case 2: enemies[i].base.dx = 0; enemies[i].base.dy = ENEMY_SPEED; enemies[i].base.direction = DIR_DOWN; break;
				case 3: enemies[i].base.dx = 0; enemies[i].base.dy = -ENEMY_SPEED; enemies[i].base.direction = DIR_UP; break;
				}
			}

			// 벽 충돌 체크 후 이동
			int half = ENEMY_SIZE / 2;

			float nextX = ex + enemies[i].base.dx;
			if (!IsTileWall(nextX - half, ey - half) &&
				!IsTileWall(nextX + half, ey - half) &&
				!IsTileWall(nextX - half, ey + half) &&
				!IsTileWall(nextX + half, ey + half))
				enemies[i].base.x = enemies[i].base.hitBoxX = nextX;
			else
				enemies[i].moveTimer = 0; // 벽 충돌 시 즉시 방향 전환

			float nextY = ey + enemies[i].base.dy;
			if (!IsTileWall(ex - half, nextY - half) &&
				!IsTileWall(ex + half, nextY - half) &&
				!IsTileWall(ex - half, nextY + half) &&
				!IsTileWall(ex + half, nextY + half))
				enemies[i].base.y = enemies[i].base.hitBoxY = nextY;
			else
				enemies[i].moveTimer = 0; // 벽 충돌 시 즉시 방향 전환
		}
	}

	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		if (!catpaw[i].isActive) continue;

		catpaw[i].x += catpaw[i].dx;
		catpaw[i].y += catpaw[i].dy;

		float catpawNextX = catpaw[i].x + catpaw[i].dx;
		float catpawNextY = catpaw[i].y + catpaw[i].dy;

		if (IsTileWall(catpaw[i].x, catpaw[i].y) || IsTileWall(catpawNextX, catpaw[i].y) || IsTileWall(catpaw[i].x, catpawNextY) || IsTileWall(catpawNextX, catpawNextY)) {
			catpaw[i].isActive = INACTIVE;
			continue;
		}

		HandleCatPawPlayerCollision(&catpaw[i], &player);
	}

	// 모든 적이 죽었는지 체크하여 문 열기
	if (currentMapType != MAP_WAITING &&
		currentMapType != MAP_FIRST_BOSS &&
		currentMapType != MAP_SECOND_BOSS &&
		currentMapType != MAP_THIRD_BOSS) {
		int activeEnemyCount = 0;
		for (int i = 0; i < ENEMY_LIMIT; i++) {
			if (enemies[i].isActive) {
				activeEnemyCount++;
			}
		}

		if (activeEnemyCount == 0) {
			SetDoorState(currentMapType, DOOR_OPEN);
		}
	}
}