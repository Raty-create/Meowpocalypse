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
JUMP_WARNING jumpWarn;

void InitBoss() {
	boss.isActive = INACTIVE;
	boss.base.width = BOSS_SIZE;
	boss.base.height = BOSS_SIZE;
	boss.base.state = BOSS_IDLE;
	boss.base.dx = 0;
	boss.base.dy = 0;
	boss.base.hp = BOSS_HP;

	boss.base.hitBoxW = boss.base.hitBoxH = BOSS_HITBOX_SIZE;
	boss.invincibleTimer = 0;

	boss.attackTimer = BOSS_ATTACK_INTERVAL;
	boss.pawTimer = 0;

	boss.dashTimer = 0;
	boss.isDashing = 0;
	boss.dashDirX = 0;
	boss.dashDirY = 0;

	boss.moveTimer = BOSS_MOVE_INTERVAL;
	boss.moveDirX = 0;
	boss.moveDirY = 0;

	boss.isJumping = 0;
	boss.jumpTimer = 0;
	boss.jumpDirX = 0;
	boss.jumpDirY = 0;

	for (int i = 0; i < BOSS_PAW_LIMIT; i++) {
		bossPaws[i].isActive = INACTIVE;
	}

	dashWarn.isActive = INACTIVE;
	dashWarn.timer = 0;

	jumpWarn.isActive = INACTIVE;
	jumpWarn.timer = 0;
}

void SpawnBoss(MAP_TYPE type) {
	if (type == MAP_WAITING ||
		type == MAP_FIRST_HALLWAY ||
		type == MAP_SECOND_HALLWAY ||
		type == MAP_THIRD_HALLWAY) return;

	MAPDATA* m = &maps[type];

	float spawnX = m->worldX + (BOSSMAP_COLS / 2) * TILE_SIZE;
	float spawnY = m->worldY + (BOSSMAP_ROWS / 2) * TILE_SIZE;

	// 죽은 상태라면 스폰하지 않음
	if (boss.base.state == BOSS_DEAD) return;

	// 1페이즈: 체력이 250 초과일 때만 MAP_FIRST_BOSS에서 스폰
	if (type == MAP_FIRST_BOSS && boss.base.hp > BOSS_HP / 2) {
		if (boss.isActive == INACTIVE) {
			boss.isActive = ACTIVE;
			boss.base.x = spawnX;
			boss.base.y = spawnY;
			boss.base.state = BOSS_IDLE;
			boss.base.dx = 0;
			boss.base.dy = 0;
		}
	}

	// 2페이즈: 체력이 125 초과, 250 이하일 때만 MAP_SECOND_BOSS에서 스폰
	else if (type == MAP_SECOND_BOSS && boss.base.hp <= BOSS_HP / 2 && boss.base.hp > (BOSS_HP / 2) / 2) {

		if (boss.isActive == INACTIVE) {
			boss.isActive = ACTIVE;
			boss.base.x = spawnX;
			boss.base.y = spawnY;
			boss.base.state = BOSS_CHASE;
			boss.base.dx = 0;
			boss.base.dy = 0;

			// 상태 초기화
			boss.moveTimer = BOSS_MOVE_INTERVAL;
			boss.attackTimer = BOSS_ATTACK_INTERVAL;
			boss.isDashing = 0;
			boss.isJumping = 0;
			dashWarn.isActive = INACTIVE;
			jumpWarn.isActive = INACTIVE;

			for (int i = 0; i < BOSS_PAW_LIMIT; i++) {
				bossPaws[i].isActive = INACTIVE;
			}
		}
	}

	// 3페이즈: 체력이 125 이하일 때 MAP_THIRD_BOSS에서 스폰
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

// 보스 스킬(3방향 발바닥)
void SpawnBossPaws() {

	boss.isActive = ACTIVE;
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

// 보스 스킬(원형 탄막)
void SpawnCircularPaws() {
	boss.isActive = ACTIVE;

	int count = BOSS_CIRCULARPAWS_COUNT;
	float angleStep = (2.0f * PI) / count;

	for (int i = 0; i < count; i++) {
		float currAngle = i * angleStep;

		for (int j = 0; j < BOSS_PAW_LIMIT; j++) {
			if (bossPaws[j].isActive == INACTIVE) {
				bossPaws[j].isActive = ACTIVE;
				bossPaws[j].x = boss.base.x;
				bossPaws[j].y = boss.base.y;
				bossPaws[j].dx = cosf(currAngle) * BOSS_PAW_SPEED;
				bossPaws[j].dy = sinf(currAngle) * BOSS_PAW_SPEED;
				break;
			}
		}
	}
}

// 보스 스킬(대쉬 경고)
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

// 보스 스킬(점프 경고)
void StartJumpWarning() {
	float dx = player.base.x - boss.base.x;
	float dy = player.base.y - boss.base.y;
	float len = sqrtf(dx * dx + dy * dy);
	if (len == 0) return;

	jumpWarn.targetX = player.base.x;
	jumpWarn.targetY = player.base.y;
	jumpWarn.timer = BOSS_JUMP_WARN_INTERVAL;
	jumpWarn.isActive = ACTIVE;

	boss.jumpDirX = dx / len;
	boss.jumpDirY = dy / len;
	boss.isJumping = ACTIVE;
	boss.base.state = BOSS_JUMP;
}

// 보스 랜덤 이동 처리 (벽 충돌 시 방향 전환)
void UpdateBossMove() {
	int half = BOSS_SIZE / 2;

	// 방향 전환 타이머
	boss.moveTimer--;
	if (boss.moveTimer <= 0) {
		boss.moveTimer = BOSS_MOVE_INTERVAL;

		// 상하좌우 중 랜덤 선택
		int dir = rand() % 8;
		switch (dir) {
		case 0: boss.moveDirX = 1; boss.moveDirY = 0; boss.base.direction = DIR_RIGHT; break;
		case 1: boss.moveDirX = -1; boss.moveDirY = 0; boss.base.direction = DIR_LEFT; break;
		case 2: boss.moveDirX = 0; boss.moveDirY = 1; boss.base.direction = DIR_DOWN; break;
		case 3: boss.moveDirX = 0; boss.moveDirY = -1; boss.base.direction = DIR_UP; break;
		case 4: boss.moveDirX = -1; boss.moveDirY = -1; boss.base.direction = DIR_UP_LEFT; break;
		case 5: boss.moveDirX = 1; boss.moveDirY = -1; boss.base.direction = DIR_UP_RIGHT; break;
		case 6: boss.moveDirX = -1; boss.moveDirY = 1; boss.base.direction = DIR_DOWN_LEFT; break;
		case 7: boss.moveDirX = 1; boss.moveDirY = 1; boss.base.direction = DIR_DOWN_RIGHT; break;
		}
	}

	// X축 이동 + 벽 충돌 체크
	float nextX = boss.base.x + boss.moveDirX * BOSS_MOVE_SPEED;
	if (!IsTileWall(nextX - half, boss.base.y - half) &&
		!IsTileWall(nextX + half, boss.base.y - half) &&
		!IsTileWall(nextX - half, boss.base.y + half) &&
		!IsTileWall(nextX + half, boss.base.y + half)) {
		boss.base.x = nextX;
		boss.base.hitBoxX = nextX;
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
		boss.base.hitBoxY = nextY;
	}
	else {
		boss.moveDirY = -boss.moveDirY;
		boss.moveTimer = 0;
	}
}

// 보스(플레이어 추적 이동)
void UpdateBossChase() {
	int half = BOSS_SIZE / 2;

	float dx = player.base.x - boss.base.x;
	float dy = player.base.y - boss.base.y;
	float len = sqrtf(dx * dx + dy * dy);
	if (len == 0) return;

	float nx = (dx / len) * BOSS_CHASE_SPEED;
	float ny = (dy / len) * BOSS_CHASE_SPEED;

	float nextX = boss.base.x + nx;
	if (!IsTileWall(nextX - half, boss.base.y - half) &&
		!IsTileWall(nextX + half, boss.base.y - half) &&
		!IsTileWall(nextX - half, boss.base.y + half) &&
		!IsTileWall(nextX + half, boss.base.y + half)) {
		boss.base.x = nextX;
		boss.base.hitBoxX = nextX;
	}

	float nextY = boss.base.y + ny;
	if (!IsTileWall(boss.base.x - half, nextY - half) &&
		!IsTileWall(boss.base.x + half, nextY - half) &&
		!IsTileWall(boss.base.x - half, nextY + half) &&
		!IsTileWall(boss.base.x + half, nextY + half)) {
		boss.base.y = nextY;
		boss.base.hitBoxY = nextY;
	}
}

// BOSSPAW 이동 + 충돌처리
static void UpdateBossPaws() {
	int pawRadius = BOSS_PAW_SIZE / 2;
	for (int i = 0; i < BOSS_PAW_LIMIT; i++) {
		if (!bossPaws[i].isActive) continue;

		bossPaws[i].x += bossPaws[i].dx;
		bossPaws[i].y += bossPaws[i].dy;

		float speed = sqrtf(bossPaws[i].dx * bossPaws[i].dx + bossPaws[i].dy * bossPaws[i].dy);
		if (speed == 0) {
			bossPaws[i].isActive = INACTIVE; continue;
		}

		float ndx = bossPaws[i].dx / speed;
		float ndy = bossPaws[i].dy / speed;

		float edgeX = bossPaws[i].x + ndx * pawRadius;
		float edgeY = bossPaws[i].y + ndy * pawRadius;

		if (IsTileWall(edgeX, edgeY)) {
			bossPaws[i].isActive = INACTIVE;
			continue;
		}

		HandleBossPawPlayerCollision(&bossPaws[i], &player);
	}
}

// 페이즈 전환 및 보스 사망 처리
static int CheckPhaseTransition() {
	if (currentMapType == MAP_FIRST_BOSS && boss.base.hp <= BOSS_HP / 2) {
		boss.isActive = INACTIVE;
		dashWarn.isActive = INACTIVE;
		jumpWarn.isActive = INACTIVE;
		SetDoorState(MAP_FIRST_BOSS, DOOR_OPEN);
		return 1;
	}

	if (currentMapType == MAP_SECOND_BOSS && boss.base.hp <= (BOSS_HP / 2) / 2) {
		boss.isActive = INACTIVE;
		dashWarn.isActive = INACTIVE;
		jumpWarn.isActive = INACTIVE;
		SetDoorState(MAP_SECOND_BOSS, DOOR_OPEN);
		return 1;
	}
	return 0;
}

// 대시 실행 처리
static void UpdateDash(int is2nd3rdPhase) {
	float nextX = boss.base.x + boss.dashDirX * DASH_SPEED;
	float nextY = boss.base.y + boss.dashDirY * DASH_SPEED;
	int half = BOSS_SIZE / 2;

	if (IsTileWall(nextX - half, nextY - half) ||
		IsTileWall(nextX + half, nextY - half) ||
		IsTileWall(nextX - half, nextY + half) ||
		IsTileWall(nextX + half, nextY + half)) {
		boss.isDashing = INACTIVE;
		boss.base.state = is2nd3rdPhase ? BOSS_CHASE : BOSS_IDLE;
		boss.attackTimer = BOSS_ATTACK_INTERVAL;
	}
	else {
		boss.base.x = nextX;
		boss.base.y = nextY;
		boss.base.hitBoxX = nextX;
		boss.base.hitBoxY = nextY;
		boss.dashTimer--;

		if (boss.dashTimer <= 0) {
			boss.isDashing = INACTIVE;
			boss.base.state = is2nd3rdPhase ? BOSS_CHASE : BOSS_IDLE;
			boss.attackTimer = BOSS_ATTACK_INTERVAL;
		}
	}
}

// 대시 경고 카운트다운
static void UpdateDashWarningCountdown() {
	dashWarn.timer--;
	if (dashWarn.timer <= 0) {
		dashWarn.isActive = INACTIVE;
		boss.isDashing = ACTIVE;
		boss.dashTimer = DASH_INTERVAL;
	}
}

// 점프 경고 카운트다운 + 착지
static void UpdateJumpLanding(int is2nd3rdPhase) {
	jumpWarn.timer--;
	if (jumpWarn.timer <= 0) {
		boss.base.x = jumpWarn.targetX;
		boss.base.y = jumpWarn.targetY;
		boss.base.hitBoxX = jumpWarn.targetX;
		boss.base.hitBoxY = jumpWarn.targetY;
		jumpWarn.isActive = INACTIVE;
		boss.isJumping = INACTIVE;
		boss.base.state = is2nd3rdPhase ? BOSS_CHASE : BOSS_IDLE;
		boss.attackTimer = BOSS_ATTACK_INTERVAL;
	}
}

// 패턴 선택
static void SelectPattern(int is2nd3rdPhase) {
	int pattern = rand() % 8;

	if (!is2nd3rdPhase) {
		if (pattern <= 6) {
			SpawnBossPaws();
			boss.attackTimer = BOSS_ATTACK_INTERVAL;
		}
		else {
			StartDashWarning();
		}
	}
	else {
		if (pattern <= 2) {
			SpawnBossPaws();
			boss.attackTimer = BOSS_ATTACK_INTERVAL;
		}
		else if (pattern <= 5) {
			SpawnCircularPaws();
			boss.attackTimer = BOSS_ATTACK_INTERVAL;
		}
		else if (pattern == 6) {
			StartDashWarning();
		}
		else {
			StartJumpWarning();
		}
	}
}

void UpdateBoss() {
	if (currentMapType == MAP_WAITING ||
		currentMapType == MAP_FIRST_HALLWAY ||
		currentMapType == MAP_SECOND_HALLWAY ||
		currentMapType == MAP_THIRD_HALLWAY) return;

	UpdateBossPaws();

	if (boss.isActive == INACTIVE) return;

	int is2nd3rdPhase = (currentMapType == MAP_SECOND_BOSS || currentMapType == MAP_THIRD_BOSS);

	if (CheckPhaseTransition()) return;

	if (boss.isDashing) {
		UpdateDash(is2nd3rdPhase);
	}
	else if (dashWarn.isActive == ACTIVE) {
		UpdateDashWarningCountdown();
	}
	else if (boss.isJumping == ACTIVE && jumpWarn.isActive == ACTIVE) {
		UpdateJumpLanding(is2nd3rdPhase);
	}
	else {
		if (is2nd3rdPhase)
			UpdateBossChase();
		else
			UpdateBossMove();

		HandleBossPlayerCollision(&player);

		boss.attackTimer--;
		if (boss.attackTimer <= 0)
			SelectPattern(is2nd3rdPhase);
	}
}