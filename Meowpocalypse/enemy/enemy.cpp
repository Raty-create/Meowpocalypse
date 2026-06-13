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

	// 애니메이션 초기화 (가로 11프레임, 세로 33줄)
	int fw = imgCatMove.width / 11;
	int fh = imgCatMove.height / 33;

	for (int i = 0; i < ENEMY_LIMIT; i++) {
		enemies[i].isActive = INACTIVE;
		enemies[i].base.width = ENEMY_WIDTH;
		enemies[i].base.height = ENEMY_HEIGHT;
		enemies[i].base.hitBoxW = ENEMY_HITBOX_WIDTH;
		enemies[i].base.hitBoxH = ENEMY_HITBOX_HEIGHT;
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
	int halfW = ENEMY_HITBOX_WIDTH / 2;
	int halfH = ENEMY_HITBOX_HEIGHT / 2;

	while (spawned < count && attempts < 1000) {
		attempts++;
		
		float spawnX = m->worldX + (rand() % (m->cols - ENEMY_COLS_SPAWN_MARGIN) + ENEMY_COLS_SPAWN_MARGIN) * TILE_SIZE + TILE_SIZE / 2.0f;
		float spawnY = m->worldY + (rand() % (m->rows - ENEMY_ROWS_SPAWN_MARGIN * 4) + ENEMY_ROWS_SPAWN_MARGIN) * TILE_SIZE + TILE_SIZE / 2.0f;

		if (IsTileBlocked(spawnX - halfW, spawnY - halfH) ||
			IsTileBlocked(spawnX + halfW, spawnY - halfH) ||
			IsTileBlocked(spawnX - halfW, spawnY + halfH) ||
			IsTileBlocked(spawnX + halfW, spawnY + halfH) ||
			IsObstacleAABBBlocked(spawnX, spawnY, ENEMY_HITBOX_WIDTH, ENEMY_HITBOX_HEIGHT)) continue;

		if (IsOverlapWithEnemy(spawnX, spawnY)) continue;

		for (int i = 0; i < ENEMY_LIMIT; i++) {
			if (!enemies[i].isActive) {
				enemies[i].isActive = ACTIVE;
				enemies[i].base.x = enemies[i].base.hitBoxX = spawnX;
				enemies[i].base.y = spawnY;
				enemies[i].base.hitBoxY = spawnY + (ENEMY_HEIGHT * 0.05f);
				enemies[i].speedMultiplier = ENEMY_SPEED_MULTIPLIER;
				enemies[i].base.state = ENEMY_IDLE;
				enemies[i].base.direction = DIR_DOWN;
				enemies[i].shootTimer = rand() % CAT_PAW_INTERVAL;
				enemies[i].moveTimer = rand() % ENEMY_MOVE_TIME;
				enemies[i].base.dx = 0;
				enemies[i].base.dy = 0;
				enemies[i].base.kx = 0;
				enemies[i].base.ky = 0;
				enemies[i].base.kTimer = 0;
				enemies[i].base.hp = ENEMY_HP;
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
	if (player.base.state == PLAYER_DEAD) return;

	float fromX = enemies[i].base.x;
	float fromY = enemies[i].base.y;

	float dx = player.base.x - fromX;
	float dy = player.base.y - fromY;

	float dist = sqrtf(dx * dx + dy * dy);
	if (dist == 0) return;

	// 공격 방향 결정 (플레이어를 바라보게)
	float angle = atan2f(dy, dx) * 180.0f / PI;
	if (angle < 0) angle += 360.0f;
	
	if (angle >= 337.5f || angle < 22.5f) enemies[i].base.direction = DIR_RIGHT;
	else if (angle >= 22.5f && angle < 67.5f) enemies[i].base.direction = DIR_DOWN_RIGHT;
	else if (angle >= 67.5f && angle < 112.5f) enemies[i].base.direction = DIR_DOWN;
	else if (angle >= 112.5f && angle < 157.5f) enemies[i].base.direction = DIR_DOWN_LEFT;
	else if (angle >= 157.5f && angle < 202.5f) enemies[i].base.direction = DIR_LEFT;
	else if (angle >= 202.5f && angle < 247.5f) enemies[i].base.direction = DIR_UP_LEFT;
	else if (angle >= 247.5f && angle < 292.5f) enemies[i].base.direction = DIR_UP;
	else if (angle >= 292.5f && angle < 337.5f) enemies[i].base.direction = DIR_UP_RIGHT;

	enemies[i].base.state = ENEMY_RANGED;
	enemies[i].anim.totalFrames = 5;
	enemies[i].attackTimer = ENEMY_ATTACK_TIME;
	SetAnimationFrame(&enemies[i].anim, 0);

	int fw = imgProjectile.width / 4;
	int fh = imgProjectile.height / 18;

	// 실제 총알 생성
	for (int j = 0; j < CAT_PAW_LIMIT; j++) {
		if (!catpaw[j].isActive) {
			catpaw[j].isActive = ACTIVE;
			catpaw[j].x = fromX;
			catpaw[j].y = fromY;
			catpaw[j].dx = (dx / dist) * CAT_PAW_SPEED;
			catpaw[j].dy = (dy / dist) * CAT_PAW_SPEED;
			catpaw[j].width = CAT_PAW_WIDTH;
			catpaw[j].height = CAT_PAW_HEIGHT;
			catpaw[j].hitBoxX = fromX;
			catpaw[j].hitBoxY = fromY;
			catpaw[j].hitBoxW = CAT_PAW_HITBOX_W;
			catpaw[j].hitBoxH = CAT_PAW_HITBOX_H;

			switch (enemies[i].base.direction) {
			case DIR_DOWN: catpaw[j].dirRow = 9; break;
			case DIR_UP: catpaw[j].dirRow = 10; break;
			case DIR_LEFT: catpaw[j].dirRow = 11; break;
			case DIR_RIGHT: catpaw[j].dirRow = 12; break;
			case DIR_UP_LEFT: catpaw[j].dirRow = 13; break;
			case DIR_UP_RIGHT: catpaw[j].dirRow = 14; break;
			case DIR_DOWN_LEFT: catpaw[j].dirRow = 15; break;
			case DIR_DOWN_RIGHT: catpaw[j].dirRow = 16; break;
			}

			InitAnimation(&catpaw[j].anim, &imgProjectile, fw, fh, 1, 0, FALSE);
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
			enemies[i].deathTimer = 0;
			enemies[i].isActive = INACTIVE;
		}
		return;
	}

	HandleEnemyCyberSlimeCollision(i);

	// 플레이어가 죽었으면 순찰 상태로 강제 전환
	if (player.base.state == PLAYER_DEAD) {
		enemies[i].base.state = ENEMY_IDLE;
		HandleEnemyPatrol(i);
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

	float ex = enemies[i].base.x;
	float ey = enemies[i].base.y;

	float toPlayerX = player.base.x - ex;
	float toPlayerY = player.base.y - ey;

	if (fabsf(toPlayerX) > fabsf(toPlayerY) * 2) {
		if (toPlayerX > 0) enemies[i].base.direction = DIR_RIGHT;
		else enemies[i].base.direction = DIR_LEFT;
	}
	else if (fabsf(toPlayerY) > fabsf(toPlayerX) * 2) {
		if (toPlayerY > 0) enemies[i].base.direction = DIR_DOWN;
		else enemies[i].base.direction = DIR_UP;
	}
	else {
		if (toPlayerX < 0 && toPlayerY < 0) enemies[i].base.direction = DIR_UP_LEFT;
		else if (toPlayerX > 0 && toPlayerY < 0) enemies[i].base.direction = DIR_UP_RIGHT;
		else if (toPlayerX < 0 && toPlayerY > 0) enemies[i].base.direction = DIR_DOWN_LEFT;
		else if (toPlayerX < 0 && toPlayerY < 0) enemies[i].base.direction = DIR_DOWN_RIGHT;
	}

	enemies[i].base.dx = enemies[i].base.kx;
	enemies[i].base.dy = enemies[i].base.ky;

	HandleEnemyWallCollision(i);

	enemies[i].base.kTimer--;

	// 넉백이 끝나는 순간 속도를 초기화하여 미끄러짐 방지 및 새로운 이동 유도
	if (enemies[i].base.kTimer <= 0) {
		enemies[i].base.kTimer = 0;
		enemies[i].base.dx = 0;
		enemies[i].base.dy = 0;
		enemies[i].moveTimer = 0;
	}
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
			nx = (dx / dist) * (ENEMY_SPEED * 1.5f * enemies[i].speedMultiplier);
			ny = (dy / dist) * (ENEMY_SPEED * 1.5f * enemies[i].speedMultiplier);
		}
		else if (dist < ENEMY_STOP_DISTANCE - 5.0f) {
			nx = -(dx / dist) * (ENEMY_SPEED * 0.5f * enemies[i].speedMultiplier);
			ny = -(dy / dist) * (ENEMY_SPEED * 0.5f * enemies[i].speedMultiplier);
		}

		float sepX = 0, sepY = 0;
		for (int j = 0; j < ENEMY_LIMIT; j++) {
			if (i == j || !enemies[j].isActive) continue;

			float diffX = enemies[i].base.x - enemies[j].base.x;
			float diffY = enemies[i].base.y - enemies[j].base.y;
			float eDist = sqrtf(diffX * diffX + diffY * diffY);

			if (eDist < ENEMY_WIDTH && eDist > 0) {
				float force = (ENEMY_WIDTH - eDist) / ENEMY_WIDTH;
				sepX += (diffX / eDist) * force * ENEMY_SPEED * enemies[i].speedMultiplier;
				sepY += (diffY / eDist) * force * ENEMY_SPEED * enemies[i].speedMultiplier;
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

		enemies[i].base.dx = finalNX;
		enemies[i].base.dy = finalNY;

		HandleEnemyWallCollision(i);
	}
}

// 적 순찰 (기본 이동)
void HandleEnemyPatrol(int i) {
	if (enemies[i].attackTimer <= 0) {
		enemies[i].attackTimer = 0;
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
		float currSpeed = ENEMY_SPEED * enemies[i].speedMultiplier;
		float diagSpeed = currSpeed * 0.7071f;
		
		switch (dir) {
		case 0: enemies[i].base.dx = currSpeed; enemies[i].base.dy = 0; break;
		case 1: enemies[i].base.dx = -currSpeed; enemies[i].base.dy = 0; break;
		case 2: enemies[i].base.dx = 0; enemies[i].base.dy = currSpeed; break;
		case 3: enemies[i].base.dx = 0; enemies[i].base.dy = -currSpeed; break;
		case 4: enemies[i].base.dx = -diagSpeed; enemies[i].base.dy = -diagSpeed; break;
		case 5: enemies[i].base.dx = diagSpeed; enemies[i].base.dy = -diagSpeed; break;
		case 6: enemies[i].base.dx = -diagSpeed; enemies[i].base.dy = diagSpeed; break;
		case 7: enemies[i].base.dx = diagSpeed; enemies[i].base.dy = diagSpeed; break;
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

	HandleEnemyWallCollision(i);
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
			nx = (dx / dist) * (ENEMY_SPEED * 1.5f * enemies[i].speedMultiplier);
			ny = (dy / dist) * (ENEMY_SPEED * 1.5f * enemies[i].speedMultiplier);
		}
		else if (dist < ENEMY_STOP_DISTANCE - 5.0f) {
			nx = -(dx / dist) * (ENEMY_SPEED * 0.5f * enemies[i].speedMultiplier);
			ny = -(dy / dist) * (ENEMY_SPEED * 0.5f * enemies[i].speedMultiplier);
		}
		
		float sepX = 0, sepY = 0;
		for (int j = 0; j < ENEMY_LIMIT; j++) {
			if (i == j || !enemies[j].isActive) continue;

			float diffX = enemies[i].base.x - enemies[j].base.x;
			float diffY = enemies[i].base.y - enemies[j].base.y;
			float eDist = sqrtf(diffX * diffX + diffY * diffY);

			if (eDist < ENEMY_WIDTH && eDist > 0) {
				float force = (ENEMY_WIDTH - eDist) / ENEMY_WIDTH;
				sepX += (diffX / eDist) * force * ENEMY_SPEED * enemies[i].speedMultiplier;
				sepY += (diffY / eDist) * force * ENEMY_SPEED * enemies[i].speedMultiplier;
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
		
		enemies[i].base.dx = finalNX;
		enemies[i].base.dy = finalNY;

		HandleEnemyWallCollision(i);
	}
}

// 잡몹 젤리 업데이트
void UpdateCatPaws() {
	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		if (!catpaw[i].isActive) continue;

		UpdateAnimation(&catpaw[i].anim);

		catpaw[i].x += catpaw[i].dx;
		catpaw[i].y += catpaw[i].dy;
		catpaw[i].hitBoxX += catpaw[i].dx;
		catpaw[i].hitBoxY += catpaw[i].dy;

		float catpawNextX = catpaw[i].x + catpaw[i].dx;
		float catpawNextY = catpaw[i].y + catpaw[i].dy;

		if (IsTileBlocked(catpaw[i].x, catpaw[i].y) ||
			IsTileBlocked(catpawNextX, catpaw[i].y) ||
			IsTileBlocked(catpaw[i].x, catpawNextY) ||
			IsTileBlocked(catpawNextX, catpawNextY) ||
			IsObstacleAABBBlocked(catpaw[i].hitBoxX, catpaw[i].hitBoxY, catpaw[i].hitBoxW, catpaw[i].hitBoxH)) {
			catpaw[i].isActive = INACTIVE;
			continue;
		}

		if (player.base.state != PLAYER_DEAD) {
			HandleCatPawPlayerCollision(&catpaw[i], &player);
		}

		for (int j = 0; j < BULLET_MAX; j++)
			HandleBulleCatPawCollision(&bullets[j], &catpaw[i]);
	}
}