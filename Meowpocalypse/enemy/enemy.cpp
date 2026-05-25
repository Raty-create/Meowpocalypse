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
IMAGE imgCatMove;

// 잡몹 생성
void InitEnemy() {
	if (imgCatMove.img.IsNull()) {
		LoadMyImage(&imgCatMove, L"cat_sprite.png");
	}

	// 애니메이션 초기화 (가로 5프레임, 세로 33줄 중 한 줄은 렌더링 시 결정)
	// 이미지의 전체 크기를 프레임 수로 나눠서 fw, fh 계산
	int fw = imgCatMove.width / 11;
	int fh = imgCatMove.height / 33;

	for (int i = 0; i < ENEMY_LIMIT; i++) {
		enemies[i].isActive = INACTIVE;
		enemies[i].base.width = enemies[i].base.height = ENEMY_SIZE;
		enemies[i].base.hitBoxW = ENEMY_HITBOX_SIZE * 2;
		enemies[i].base.hitBoxH = ENEMY_HITBOX_SIZE * 2;
		enemies[i].base.state = ENEMY_IDLE;
		enemies[i].base.direction = DIR_DOWN;
		enemies[i].shootTimer = 0;
		enemies[i].moveTimer = 0;
		enemies[i].base.dx = 0;
		enemies[i].base.dy = 0;
		enemies[i].base.kx = 0;
		enemies[i].base.ky = 0;
		enemies[i].base.kTimer = 0;
		enemies[i].attackTimer = 0;

		InitAnimation(&enemies[i].anim, &imgCatMove, fw, fh, 5, 8);
	}
	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		catpaw[i].isActive = INACTIVE;
	}
}

void ReleaseEnemy() {
	ReleaseMyImage(&imgCatMove);
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
	int half = ENEMY_SIZE / 2;

	while (spawned < count && attempts < 1000) {
		attempts++;
		
		float spawnX = m->worldX + (rand() % (m->cols - ENEMY_COLS_SPAWN_MARGIN) + ENEMY_COLS_SPAWN_MARGIN) * TILE_SIZE + TILE_SIZE / 2.0f;
		float spawnY = m->worldY + (rand() % (m->rows - ENEMY_ROWS_SPAWN_MARGIN) + ENEMY_ROWS_SPAWN_MARGIN) * TILE_SIZE + TILE_SIZE / 2.0f;

		if (IsTileWall(spawnX - half, spawnY - half) ||
			IsTileWall(spawnX + half, spawnY - half) ||
			IsTileWall(spawnX - half, spawnY + half) ||
			IsTileWall(spawnX + half, spawnY + half)) continue;

		if (IsOverlapWithEnemy(spawnX, spawnY)) continue;

		for (int i = 0; i < ENEMY_LIMIT; i++) {
			if (!enemies[i].isActive) {
				enemies[i].isActive = ACTIVE;
				enemies[i].base.x = enemies[i].base.hitBoxX = spawnX;
				enemies[i].base.y = enemies[i].base.hitBoxY = spawnY;
				enemies[i].base.state = ENEMY_IDLE;
				enemies[i].base.direction = DIR_DOWN;
				enemies[i].shootTimer = rand() % CAT_PAW_INTERVAL;
				enemies[i].moveTimer = rand() % ENEMY_MOVE_TIMER;
				enemies[i].base.dx = 0;
				enemies[i].base.dy = 0;
				enemies[i].base.kx = 0;
				enemies[i].base.ky = 0;
				enemies[i].base.kTimer = 0;
				enemies[i].base.hp = ENEMY_HP;
				enemies[i].shootTimer = rand() % CAT_PAW_INTERVAL;
				enemies[i].moveTimer = rand() % ENEMY_MOVE_TIMER;
				enemies[i].deathTimer = 0;
				enemies[i].attackTimer = 0;
				SetAnimationFrame(&enemies[i].anim, 0);
				spawned++;
				break;
			}
		}
	}
}

// 잡몹 공격 생성
void SpawnCatPaw(int i) {
	float fromX = enemies[i].base.x;
	float fromY = enemies[i].base.y;

	float dx = player.base.x - fromX;
	float dy = player.base.y - fromY;
	float dist = sqrtf(dx * dx + dy * dy);
	if (dist == 0) return;

	// 공격 방향 결정 (플레이어를 바라보게)
	if (fabsf(dx) > fabsf(dy)) {
		if (dx > 0) enemies[i].base.direction = DIR_RIGHT;
		else enemies[i].base.direction = DIR_LEFT;
	}
	else {
		if (dy > 0) enemies[i].base.direction = DIR_DOWN;
		else enemies[i].base.direction = DIR_UP;
	}

	float ratio = (dy != 0) ? fabsf(dx / dy) : 100.0f;
	if (ratio > DEG_TO_RAD(22.5f) && ratio < DEG_TO_RAD(67.5f)) {
		if (dx > 0 && dy > 0) enemies[i].base.direction = DIR_DOWN_RIGHT;
		else if (dx > 0 && dy < 0) enemies[i].base.direction = DIR_UP_RIGHT;
		else if (dx < 0 && dy > 0) enemies[i].base.direction = DIR_DOWN_LEFT;
		else if (dx < 0 && dy < 0) enemies[i].base.direction = DIR_UP_LEFT;
	}

	enemies[i].base.state = ENEMY_RANGED;
	enemies[i].anim.totalFrames = 5;
	enemies[i].attackTimer = 30;
	SetAnimationFrame(&enemies[i].anim, 0);

	// 실제 총알 생성
	for (int j = 0; j < CAT_PAW_LIMIT; j++) {
		if (!catpaw[j].isActive) {
			catpaw[j].isActive = ACTIVE;
			catpaw[j].x = fromX;
			catpaw[j].y = fromY;
			catpaw[j].dx = (dx / dist) * CAT_PAW_SPEED;
			catpaw[j].dy = (dy / dist) * CAT_PAW_SPEED;
			break;
		}
	}
}

// 잡몹 이동 및 공격 업데이트
void UpdateEnemies() {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		if (!enemies[i].isActive) continue;
		UpdateEnemyState(i);
		UpdateAnimation(&enemies[i].anim);
	}

	UpdateCatPaws();

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

void UpdateEnemyState(int i) {
	// 사망 상태 처리
	if (enemies[i].base.state == ENEMY_DEAD) {
		enemies[i].anim.totalFrames = 11;
		enemies[i].anim.isLoop = FALSE;

		enemies[i].deathTimer--;
		if (enemies[i].deathTimer <= 0) {
			enemies[i].isActive = INACTIVE;
		}
		return;
	}

	enemies[i].anim.totalFrames = 5;
	enemies[i].anim.isLoop = TRUE;

	// 공격 모션 타이머 처리
	if (enemies[i].attackTimer > 0) {
		enemies[i].attackTimer--;
		if (enemies[i].attackTimer <= 0) {
			enemies[i].base.state = ENEMY_IDLE;
		}
		else {
			return;
		}
	}

	// 넉백 및 히트 상태 처리
	if (enemies[i].base.kTimer > 0) {
		HandleEnemyKnockback(i);
		return;
	}

	float ex = enemies[i].base.x;
	float ey = enemies[i].base.y;
	float dx = player.base.x - ex;
	float dy = player.base.y - ey;
	float dist = sqrtf(dx * dx + dy * dy);

	// 츄르 어그로 체크 (플레이어 추격보다 우선순위 높음)
	for (int j = 0; j < CHURU_MAX; j++) {
		if (churues[j].isActive == ACTIVE && churues[j].isDropped == DROPPED) {
			float cdx = churues[j].x - ex;
			float cdy = churues[j].y - ey;
			float cDist = sqrtf(cdx * cdx + cdy * cdy);

			if (cDist < CHURU_AGGRO_RANGE) {
				enemies[i].base.state = ENEMY_AGGRO;
				HandleEnemyAggro(i, churues[j].x, churues[j].y);
				return;
			}
		}
	}

	if (enemies[i].base.state == ENEMY_IDLE) {
		enemies[i].moveTimer++;
	}

	if (dist < ENEMY_CHASE_RANGE) {
		HandleEnemyChase(i, dx, dy, dist);
	}
	else {
		HandleEnemyPatrol(i);
	}
}

// 적 넉백
void HandleEnemyKnockback(int i) {
	enemies[i].base.state = ENEMY_HIT;
	//SetAnimationFrame(&enemies[i].anim, 0);

	float ex = enemies[i].base.x;
	float ey = enemies[i].base.y;
	float nextX = ex + enemies[i].base.kx;
	float nextY = ey + enemies[i].base.ky;
	int half = ENEMY_SIZE / 2;

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
}

// 적 -> 플레이어 쫓기
void HandleEnemyChase(int i, float dx, float dy, float dist) {
	if (dist < ENEMY_STOP_DISTANCE + 5.0f) {
		enemies[i].base.state = ENEMY_MELEE;
	}
	else {
		enemies[i].base.state = ENEMY_CHASE;
	}
	enemies[i].shootTimer = 0;

	HandleEnemyPlayerCollision(&enemies[i], &player);

	if (dist > 0) {
		float nx = 0, ny = 0;
		if (dist > ENEMY_STOP_DISTANCE) {
			nx = (dx / dist) * (ENEMY_SPEED * 1.5f);
			ny = (dy / dist) * (ENEMY_SPEED * 1.5f);
		}
		else if (dist < ENEMY_STOP_DISTANCE - 5.0f) {
			nx = -(dx / dist) * (ENEMY_SPEED * 0.5f);
			ny = -(dy / dist) * (ENEMY_SPEED * 0.5f);
		}

		float sepX = 0, sepY = 0;
		for (int j = 0; j < ENEMY_LIMIT; j++) {
			if (i == j || !enemies[j].isActive) continue;

			float diffX = enemies[i].base.x - enemies[j].base.x;
			float diffY = enemies[i].base.y - enemies[j].base.y;
			float eDist = sqrtf(diffX * diffX + diffY * diffY);

			if (eDist < ENEMY_SIZE && eDist > 0) {
				float force = (ENEMY_SIZE - eDist) / ENEMY_SIZE;
				sepX += (diffX / eDist) * force * ENEMY_SPEED;
				sepY += (diffY / eDist) * force * ENEMY_SPEED;
			}
		}

		float finalNX = nx + sepX;
		float finalNY = ny + sepY;

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
		float ex = enemies[i].base.x;
		float ey = enemies[i].base.y;
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

// 적 순찰 (기본 이동)
void HandleEnemyPatrol(int i) {
	float ex = enemies[i].base.x;
	float ey = enemies[i].base.y;

	if (enemies[i].attackTimer <= 0) {
		enemies[i].base.state = ENEMY_MOVE;
	}

	enemies[i].shootTimer++;
	if (enemies[i].shootTimer >= CAT_PAW_INTERVAL) {
		SpawnCatPaw(i);
		enemies[i].shootTimer = 0;
		return;
	}

	enemies[i].moveTimer--;
	if (enemies[i].moveTimer <= 0) {
		enemies[i].moveTimer = 120 + rand() % 60;

		int dir = rand() % 8;
		float diagSpeed = ENEMY_SPEED * 0.7071f;		// 0.7071 ≒ ( 1/√2 )
		switch (dir) {
		case 0: enemies[i].base.dx = ENEMY_SPEED; enemies[i].base.dy = 0; enemies[i].base.direction = DIR_RIGHT; break;
		case 1: enemies[i].base.dx = -ENEMY_SPEED; enemies[i].base.dy = 0; enemies[i].base.direction = DIR_LEFT; break;
		case 2: enemies[i].base.dx = 0; enemies[i].base.dy = ENEMY_SPEED; enemies[i].base.direction = DIR_DOWN; break;
		case 3: enemies[i].base.dx = 0; enemies[i].base.dy = -ENEMY_SPEED; enemies[i].base.direction = DIR_UP; break;
		case 4: enemies[i].base.dx = -diagSpeed; enemies[i].base.dy = -diagSpeed; enemies[i].base.direction = DIR_UP_LEFT; break;
		case 5: enemies[i].base.dx = diagSpeed; enemies[i].base.dy = -diagSpeed; enemies[i].base.direction = DIR_UP_RIGHT; break;
		case 6: enemies[i].base.dx = -diagSpeed; enemies[i].base.dy = diagSpeed; enemies[i].base.direction = DIR_DOWN_LEFT; break;
		case 7: enemies[i].base.dx = diagSpeed; enemies[i].base.dy = diagSpeed; enemies[i].base.direction = DIR_DOWN_RIGHT; break;
		}
	}

	if (enemies[i].attackTimer <= 0) {
		float adx = enemies[i].base.dx;
		float ady = enemies[i].base.dy;

		if (adx > 0) {
			if (ady > 0) enemies[i].base.direction = DIR_DOWN_RIGHT;
			else if (ady < 0) enemies[i].base.direction = DIR_UP_RIGHT;
			else enemies[i].base.direction = DIR_RIGHT;
		}
		else if (adx < 0) {
			if (ady > 0) enemies[i].base.direction = DIR_DOWN_LEFT;
			else if (ady < 0) enemies[i].base.direction = DIR_UP_LEFT;
			else enemies[i].base.direction = DIR_LEFT;
		}
		else {
			if (ady > 0) enemies[i].base.direction = DIR_DOWN;
			else if (ady < 0) enemies[i].base.direction = DIR_UP;
		}
	}

	int half = ENEMY_SIZE / 2;
	float nextX = ex + enemies[i].base.dx;
	if (!IsTileWall(nextX - half, ey - half) &&
		!IsTileWall(nextX + half, ey - half) &&
		!IsTileWall(nextX - half, ey + half) &&
		!IsTileWall(nextX + half, ey + half))
		enemies[i].base.x = enemies[i].base.hitBoxX = nextX;
	else
		enemies[i].moveTimer = 0;

	float nextY = ey + enemies[i].base.dy;
	if (!IsTileWall(ex - half, nextY - half) &&
		!IsTileWall(ex + half, nextY - half) &&
		!IsTileWall(ex - half, nextY + half) &&
		!IsTileWall(ex + half, nextY + half))
		enemies[i].base.y = enemies[i].base.hitBoxY = nextY;
	else
		enemies[i].moveTimer = 0;
}

// 적 -> 츄르 쫓기
void HandleEnemyAggro(int i, float tx, float ty) {
	float ex = enemies[i].base.x;
	float ey = enemies[i].base.y;
	float dx = tx - ex;
	float dy = ty - ey;
	float dist = sqrtf(dx * dx + dy * dy);

	if (dist > 0) {
		float nx = 0, ny = 0;
		if (dist > ENEMY_STOP_DISTANCE) {
			nx = (dx / dist) * (ENEMY_SPEED * 1.5f);
			ny = (dy / dist) * (ENEMY_SPEED * 1.5f);
		}
		else if (dist < ENEMY_STOP_DISTANCE - 5.0f) {
			nx = -(dx / dist) * (ENEMY_SPEED * 0.5f);
			ny = -(dy / dist) * (ENEMY_SPEED * 0.5f);
		}
		
		float sepX = 0, sepY = 0;
		for (int j = 0; j < ENEMY_LIMIT; j++) {
			if (i == j || !enemies[j].isActive) continue;

			float diffX = enemies[i].base.x - enemies[j].base.x;
			float diffY = enemies[i].base.y - enemies[j].base.y;
			float eDist = sqrtf(diffX * diffX + diffY * diffY);

			if (eDist < ENEMY_SIZE && eDist > 0) {
				float force = (ENEMY_SIZE - eDist) / ENEMY_SIZE;
				sepX += (diffX / eDist) * force * ENEMY_SPEED;
				sepY += (diffY / eDist) * force * ENEMY_SPEED;
			}
		}

		float finalNX = nx + sepX;
		float finalNY = ny + sepY;

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

// 잡몹 젤리 업데이트
void UpdateCatPaws() {
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
}