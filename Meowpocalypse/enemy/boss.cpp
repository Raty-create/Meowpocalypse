#include <stdlib.h>
#include <math.h>

#include "enemy.h"
#include "config.h"
#include "map.h"
#include "collision.h"
#include "player.h"
#include "object.h"
#include "boss.h"
#include "enum.h"

BOSS boss;
BOSS_PAW bossPaws[BOSS_PAW_LIMIT];
DASH_WARNING dashWarn;

void InitBoss() {
		boss.isActive = INACTIVE;
		boss.base.width = BOSS_SIZE;
		boss.base.height = BOSS_SIZE;
		boss.base.state = BOSS_IDLE;
		boss.base.dx = 0;
		boss.base.dy = 0;
		boss.base.hp = BOSS_HP;
		boss.attackTimer = BOSS_ATTACK_INTERVAL;
		boss.pawTimer = 0;
		boss.dashTimer = 0;
		boss.isDashing = 0;
		boss.dashDirX = 0;
		boss.dashDirY = 0;

		for (int i = 0; i < BOSS_PAW_LIMIT; i++) {
			bossPaws[i].isActive = INACTIVE;
		}

		dashWarn.isActive = INACTIVE;
		dashWarn.timer = 0;
}

void SpawnBoss(MAP_TYPE type) {
	if (type == MAP_WAITING ||
		type == MAP_FIRST_HALLWAY ||
		type == MAP_SECOND_HALLWAY ||
		type == MAP_THIRD_HALLWAY) return;

	MAPDATA* m = &maps[type];

	float spawnX = m->worldX + (BOSSMAP_COLS / 2) * TILE_SIZE;
	float spawnY = m->worldY + (BOSSMAP_ROWS / 2) * TILE_SIZE;

	if (type == MAP_FIRST_BOSS && boss.base.hp == BOSS_HP) {

		if (boss.isActive == INACTIVE) {
			boss.isActive = ACTIVE;
			boss.base.x = spawnX;
			boss.base.y = spawnY;
			boss.base.state = BOSS_IDLE;
			boss.base.dx = 0;
			boss.base.dy = 0;
		}
	}
	else if (type == MAP_SECOND_BOSS && boss.base.hp <= BOSS_HP / 2) {

		if (boss.isActive == INACTIVE) {
			boss.isActive = ACTIVE;
			boss.base.x = spawnX;
			boss.base.y = spawnY;
			boss.base.state = BOSS_IDLE;
			boss.base.dx = 0;
			boss.base.dy = 0;
		}
	}
	else if (type == MAP_THIRD_BOSS && boss.base.hp <= (BOSS_HP / 2) / 2) {

		if (boss.isActive == INACTIVE) {
			boss.isActive = ACTIVE;
			boss.base.x = spawnX;
			boss.base.y = spawnY;
			boss.base.state = BOSS_IDLE;
			boss.base.dx = 0;
			boss.base.dy = 0;
		}
	}
}

void SpawnBossPaws() {

	boss.isActive = BOSS_THREE_WAY_CATPAW;
	float dx = player.base.x - boss.base.x;
	float dy = player.base.y - boss.base.y;
	float len = sqrtf(dx * dx + dy * dy);
	
	if (len == 0) return;
	dx /= len;
	dy /= len;

	float offsets[3] = { -0.7854f, 0.0f, 0.7854f };

	for (int d = 0; d < 3; d++) {
		float angle = atan2f(dy, dx) + offsets[d];
		float vx = cosf(angle);
		float vy = sinf(angle);

		for (int i = 0; i < BOSS_PAW_LIMIT; i++) {
			if (bossPaws[i].isActive == INACTIVE) {
				bossPaws[i].isActive = ACTIVE;
				bossPaws[i].x = boss.base.x;
				bossPaws[i].y = boss.base.y;
				bossPaws[i].dx = vx * BOSS_PAW_SPEED;
				bossPaws[i].dy = vy * BOSS_PAW_SPEED;
				break;
			}
		}
	}
}

void StartDashWarning() {

	float dx = player.base.x - boss.base.x;
	float dy = player.base.y - boss.base.y;
	float len = sqrtf(dx * dx + dy * dy);

	if (len == 0) return;

	dashWarn.startX = boss.base.x;
	dashWarn.startY = boss.base.y;
	dashWarn.dirX = dx / len;
	dashWarn.dirY = dy / len;

	// 수직 방향 벡터 계산해서 저장 (render에서 재계산 불필요)
	dashWarn.perpX = -dashWarn.dirY;
	dashWarn.perpY = dashWarn.dirX;

	// 벽까지 거리 계산 - render.cpp 대신 여기서 한 번만 계산
	int half = BOSS_SIZE / 2;
	int maxDist = DASH_WARN_TILES * TILE_SIZE;
	dashWarn.stopDist = maxDist;

	for (int dist = 1; dist <= maxDist; dist++) {
		float cx = dashWarn.startX + dashWarn.dirX * dist;
		float cy = dashWarn.startY + dashWarn.dirY * dist;

		if (IsTileWall(cx + dashWarn.perpX * half, cy + dashWarn.perpY * half) ||
			IsTileWall(cx - dashWarn.perpX * half, cy - dashWarn.perpY * half) ||
			IsTileWall(cx, cy)) {
			dashWarn.stopDist = dist;
			break;
		}
	}

	dashWarn.timer = DASH_WARN_INTERVAL;
	dashWarn.isActive = ACTIVE;

	boss.dashDirX = dashWarn.dirX;
	boss.dashDirY = dashWarn.dirY;
	boss.base.state = BOSS_DASH;
}

// 보스 랜덤 이동 처리 (벽 충돌 시 방향 전환)
void UpdateBossMove() {
	int half = BOSS_SIZE / 2;

	// 방향 전환 타이머
	boss.moveTimer--;
	if (boss.moveTimer <= 0) {
		boss.moveTimer = BOSS_MOVE_INTERVAL;

		// 상하좌우 중 랜덤 선택
		int dir = rand() % 4;
		switch (dir) {
		case 0: boss.moveDirX = 1; boss.moveDirY = 0; boss.base.direction = DIR_RIGHT; break;
		case 1: boss.moveDirX = -1; boss.moveDirY = 0; boss.base.direction = DIR_LEFT; break;
		case 2: boss.moveDirX = 0; boss.moveDirY = 1; boss.base.direction = DIR_DOWN; break;
		case 3: boss.moveDirX = 0; boss.moveDirY = -1; boss.base.direction = DIR_UP; break;
		}
	}

	// X축 이동 + 벽 충돌 체크
	float nextX = boss.base.x + boss.moveDirX * BOSS_MOVE_SPEED;
	if (!IsTileWall(nextX - half, boss.base.y - half) &&
		!IsTileWall(nextX + half, boss.base.y - half) &&
		!IsTileWall(nextX - half, boss.base.y + half) &&
		!IsTileWall(nextX + half, boss.base.y + half)) {
		boss.base.x = nextX;
	}
	else {
		boss.moveDirX = -boss.moveDirX; // 벽이면 반대 방향
		boss.moveTimer = 0;
	}

	// Y축 이동 + 벽 충돌 체크
	float nextY = boss.base.y + boss.moveDirY * BOSS_MOVE_SPEED;
	if (!IsTileWall(boss.base.x - half, nextY - half) &&
		!IsTileWall(boss.base.x + half, nextY - half) &&
		!IsTileWall(boss.base.x - half, nextY + half) &&
		!IsTileWall(boss.base.x + half, nextY + half)) {
		boss.base.y = nextY;
	}
	else {
		boss.moveDirY = -boss.moveDirY;
		boss.moveTimer = 0;
	}
}

void UpdateBoss() {
	if (currentMapType == MAP_WAITING ||
		currentMapType == MAP_FIRST_HALLWAY ||
		currentMapType == MAP_SECOND_HALLWAY ||
		currentMapType == MAP_THIRD_HALLWAY) return;

	// 대시 실행 중
	if (boss.isDashing) {

		float nextX = boss.base.x + boss.dashDirX * DASH_SPEED;
		float nextY = boss.base.y + boss.dashDirY * DASH_SPEED;

		// 보스 가장자리 4코너로 벽 충돌 체크
		int half = BOSS_SIZE / 2;

		if (IsTileWall(nextX - half, nextY - half) ||
			IsTileWall(nextX + half, nextY - half) ||
			IsTileWall(nextX - half, nextY + half) ||
			IsTileWall(nextX + half, nextY + half)) {
			boss.isDashing = INACTIVE;
			boss.base.state = BOSS_IDLE;
			boss.attackTimer = BOSS_ATTACK_INTERVAL;
		}

		else {
			boss.base.x = nextX;
			boss.base.y = nextY;
			boss.dashTimer--;
			
			if (boss.dashTimer <= 0) {
				boss.isDashing = INACTIVE;
				boss.base.state = BOSS_IDLE;
				boss.attackTimer = BOSS_ATTACK_INTERVAL;
			}
		}
	}

	// 경고 카운트다운 → 대시 실행
	else if (dashWarn.isActive == ACTIVE) {

		dashWarn.timer--;
		if (dashWarn.timer <= 0) {
			dashWarn.isActive = INACTIVE;
			boss.isDashing = ACTIVE;
			boss.dashTimer = DASH_INTERVAL;
		}
	}

	// 패턴 선택 타이머
	else {
		UpdateBossMove();

		boss.attackTimer--;
		if (boss.attackTimer <= 0) {
			// 0: CAT_PAW 발사,  1: 대시 경고
			int pattern = rand() % 8;

			if (pattern <= 6) {
				SpawnBossPaws();
				boss.attackTimer = BOSS_ATTACK_INTERVAL;
			}
			else {
				StartDashWarning();
				// attackTimer는 대시 종료 후 리셋
			}
		}
	}

	// CatPaw 이동
	int pawRadius = BOSS_PAW_SIZE / 2;
	for (int i = 0; i < BOSS_PAW_LIMIT; i++) {
		if (!bossPaws[i].isActive) continue;

		bossPaws[i].x += bossPaws[i].dx;
		bossPaws[i].y += bossPaws[i].dy;

		// 이동 방향 단위벡터 계산
		float speed = sqrtf(bossPaws[i].dx * bossPaws[i].dx + bossPaws[i].dy * bossPaws[i].dy);
		if (speed == 0) { bossPaws[i].isActive = INACTIVE; continue; }
		float ndx = bossPaws[i].dx / speed;
		float ndy = bossPaws[i].dy / speed;

		// 원의 앞쪽 테두리 좌표: 중심 + 방향 * 반지름
		float edgeX = bossPaws[i].x + ndx * pawRadius;
		float edgeY = bossPaws[i].y + ndy * pawRadius;

		if (IsTileWall(edgeX, edgeY)) {
			bossPaws[i].isActive = INACTIVE;
		}
	}
}
