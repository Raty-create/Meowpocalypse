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
#include "camera.h"
#include "bullet.h"
#include "sound.h"

BOSS boss;
IMAGE imgBossSprite;
IMAGE imgBossSkillPattrenEffect;

BOSS_PAW bossPaws[BOSS_PAW_LIMIT];

DASH_WARNING dashWarn;
JUMP_WARNING jumpWarn;

void InitBoss() {
	if (imgBossSprite.img.IsNull()) {
		LoadMyImage(&imgBossSprite, L"boss_sprite.png");
	}
	if (imgBossSkillPattrenEffect.img.IsNull()) {
		LoadMyImage(&imgBossSkillPattrenEffect, L"boss_skill_pattern_effect.png");
	}

	int bFW = imgBossSprite.width / 5;
	int bFH = imgBossSprite.height / 42;
	int bspeFW = imgBossSkillPattrenEffect.width / 6;
	int bspeFH = imgBossSkillPattrenEffect.height / 3;

	boss.isActive = INACTIVE;
	boss.base.width = BOSS_WIDTH;
	boss.base.height = BOSS_HEIGHT;
	boss.base.hitBoxW = BOSS_HITBOX_WIDTH;
	boss.base.hitBoxH = BOSS_HITBOX_HEIGHT;
	boss.base.state = BOSS_IDLE;
	boss.base.dx = 0;
	boss.base.dy = 0;
	boss.base.hp = BOSS_HP;

	boss.invincibleTimer = 0;

	boss.attackTimer = BOSS_ATTACK_INTERVAL;
	boss.pawTimer = 0;

	boss.dashTimer = 0;
	boss.isDashing = 0;
	boss.dashDirX = 0;
	boss.dashDirY = 0;

	boss.moveTimer = BOSS_MOVE_INTERVAL / 3;
	boss.moveDirX = 0;
	boss.moveDirY = 0;

	boss.isJumping = 0;
	boss.jumpTimer = 0;
	boss.jumpDirX = 0;
	boss.jumpDirY = 0;
	boss.jumpPhase = 0;
	boss.jumpOffsetY = 0;

	boss.spiralTimer = 0;
	boss.spiralIndex = 0;
	boss.isSpiralActive = 0;

	boss.isRandomCircularActive = 0;
	boss.randomCircularPhase = 0;
	boss.randomCircularDelay = 0;
	boss.doubleDashPhase = 0;
	boss.doubleDashDelay = 0;
	boss.isAttacking = 0;
	boss.attackEndTimer = 0;

	boss.isEscaping = INACTIVE;

	for (int i = 0; i < BOSS_PAW_LIMIT; i++) {
		bossPaws[i].isActive = INACTIVE;
	}

	dashWarn.isActive = INACTIVE;
	dashWarn.timer = 0;

	jumpWarn.isActive = INACTIVE;
	jumpWarn.timer = 0;

	boss.skillChargeTimer = 0;
	boss.deathTimer = 0;

	InitAnimation(&boss.anim, &imgBossSprite, bFW, bFH, 5, 10);
	InitAnimation(&boss.effectAnim, &imgBossSkillPattrenEffect, bspeFW, bspeFH, 6, 0, FALSE);
}

void ReleaseBoss() {
	ReleaseMyImage(&imgBossSprite);
	ReleaseMyImage(&imgBossSkillPattrenEffect);
}

void SpawnBoss(MAP_TYPE type) {
	if (type == MAP_WAITING ||
		type == MAP_HALLWAY) return;

	// 죽은 상태거나 체력이 없으면 스폰하지 않음
	if (boss.base.state == BOSS_DEAD || boss.base.hp <= 0) return;

	MAPDATA* m = &maps[type];

	float spawnX = m->worldX + (BOSSMAP_COLS / 2) * TILE_SIZE + TILE_SIZE / 2;
	float spawnY = m->worldY + (BOSSMAP_ROWS / 2) * TILE_SIZE - TILE_SIZE * 3;

	// 1페이즈: 체력이 250 초과일 때만 MAP_FIRST_BOSS에서 스폰
	if (type == MAP_FIRST_BOSS && boss.base.hp > (BOSS_HP * 0.75)) {

		if (boss.isActive == INACTIVE) {
			boss.isActive = ACTIVE;
			boss.base.x = spawnX;
			boss.base.y = spawnY;
			boss.base.state = BOSS_IDLE;
			boss.base.dx = 0;
			boss.base.dy = 0;
			boss.isEscaping = INACTIVE;
			boss.jumpOffsetY = 0;
		}
	}

	// 2페이즈: 체력이 125 초과, 250 이하일 때만 MAP_SECOND_BOSS에서 스폰
	else if (type == MAP_SECOND_BOSS && boss.base.hp <= (BOSS_HP * 0.75) && boss.base.hp > (BOSS_HP * 0.5)) {

		if (boss.isActive == INACTIVE) {
			boss.isActive = ACTIVE;
			boss.base.x = spawnX;
			boss.base.y = spawnY;
			boss.base.state = BOSS_CHASE;
			boss.base.dx = 0;
			boss.base.dy = 0;
			boss.isEscaping = INACTIVE;
			boss.jumpOffsetY = 0;
			boss.invincibleTimer = 0;
			boss.skillChargeTimer = 0;

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
	else if (type == MAP_THIRD_BOSS && boss.base.hp <= (BOSS_HP * 0.5)) {

		if (boss.isActive == INACTIVE) {
			boss.isActive = ACTIVE;
			boss.base.x = spawnX;
			boss.base.y = spawnY;
			boss.base.state = BOSS_IDLE;
			boss.base.dx = 0;
			boss.base.dy = 0;
			boss.isEscaping = INACTIVE;
			boss.jumpOffsetY = 0;
			boss.invincibleTimer = 0;
			boss.skillChargeTimer = 0;

			// 상태 초기화
			boss.moveTimer = BOSS_MOVE_INTERVAL;
			boss.attackTimer = BOSS_ATTACK_INTERVAL;
			boss.isDashing = 0;
			boss.isJumping = 0;
			dashWarn.isActive = INACTIVE;
			jumpWarn.isActive = INACTIVE;

			boss.spiralTimer = 0;
			boss.spiralIndex = 0;
			boss.isSpiralActive = 0;

			boss.isRandomCircularActive = 0;
			boss.randomCircularPhase = 0;
			boss.randomCircularDelay = 0;
			boss.doubleDashPhase = 0;
			boss.doubleDashDelay = 0;
		}
	}
}

// 보스 스킬(3방향 젤리)
void SpawnBossPaws() {
	boss.isActive = ACTIVE;
	boss.isAttacking = 1;
	boss.base.state = BOSS_THREE_WAY_CATPAW;
	boss.attackEndTimer = BOSS_ATTACK_INTERVAL / 2; // 발사 후 이 프레임 동안 정지

	float dx = player.base.x - boss.base.x;
	float dy = player.base.y - boss.base.y;
	float len = sqrtf(dx * dx + dy * dy);

	if (len == 0) return;
	dx /= len;
	dy /= len;

	// 공격 방향 결정 (플레이어를 바라보게)
	float angle = atan2f(dy, dx) * 180.0f / PI;
	if (angle < 0) angle += 360.0f;

	if (angle >= 337.5f || angle < 22.5f) boss.base.direction = DIR_RIGHT;
	else if (angle >= 22.5f && angle < 67.5f) boss.base.direction = DIR_DOWN_RIGHT;
	else if (angle >= 67.5f && angle < 112.5f) boss.base.direction = DIR_DOWN;
	else if (angle >= 112.5f && angle < 157.5f) boss.base.direction = DIR_DOWN_LEFT;
	else if (angle >= 157.5f && angle < 202.5f) boss.base.direction = DIR_LEFT;
	else if (angle >= 202.5f && angle < 247.5f) boss.base.direction = DIR_UP_LEFT;
	else if (angle >= 247.5f && angle < 292.5f) boss.base.direction = DIR_UP;
	else if (angle >= 292.5f && angle < 337.5f) boss.base.direction = DIR_UP_RIGHT;

	SetAnimationFrame(&boss.anim, 0);

	float offsets[3] = { -0.1454f, 0.0f, 0.1454f };

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
				bossPaws[i].width = BOSS_PAW_WIDTH;
				bossPaws[i].height = BOSS_PAW_HEIGHT;
				bossPaws[i].hitBoxX = boss.base.x;
				bossPaws[i].hitBoxY = boss.base.y;
				bossPaws[i].hitBoxW = BOSS_PAW_HITBOX_W;
				bossPaws[i].hitBoxH = BOSS_PAW_HITBOX_H;

				int fw = imgProjectile.width / 4;
				int fh = imgProjectile.height / 18;
				bossPaws[i].dirRow = 17;
				InitAnimation(&bossPaws[i].anim, &imgProjectile, fw, fh, 1, 0, FALSE);
				
				break;
			}
		}
	}
}

// 보스 스킬(원형 탄막)
void SpawnCircularPaws() {
	boss.isActive = ACTIVE;
	boss.isAttacking = 1;
	boss.base.state = BOSS_CIRCULAR_CATPAW;
	boss.base.direction = DIR_DOWN;
	boss.attackEndTimer = BOSS_ATTACK_INTERVAL / 2;

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
				bossPaws[j].width = BOSS_PAW_WIDTH;
				bossPaws[j].height = BOSS_PAW_HEIGHT;
				bossPaws[j].hitBoxX = boss.base.x;
				bossPaws[j].hitBoxY = boss.base.y;
				bossPaws[j].hitBoxW = BOSS_PAW_HITBOX_W;
				bossPaws[j].hitBoxH = BOSS_PAW_HITBOX_H;

				int fw = imgProjectile.width / 4;
				int fh = imgProjectile.height / 18;
				bossPaws[j].dirRow = 17;
				InitAnimation(&bossPaws[j].anim, &imgProjectile, fw, fh, 1, 0, FALSE);
				
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

	// 벽까지 거리 계산
	int halfW = (int)(BOSS_WIDTH * BOSS_DASH_SCALE / 2);
	int halfH = (int)(BOSS_HEIGHT * BOSS_DASH_SCALE / 2);
	int maxDist = DASH_WARN_TILES * TILE_SIZE;
	dashWarn.stopDist = maxDist;

	for (int dist = 1; dist <= maxDist; dist++) {
		float cx = dashWarn.startX + dashWarn.dirX * dist;
		float cy = dashWarn.startY + dashWarn.dirY * dist;

		if (IsTileBlocked(cx + dashWarn.perpX * halfW, cy + dashWarn.perpY * halfH) ||
			IsTileBlocked(cx - dashWarn.perpX * halfW, cy - dashWarn.perpY * halfH) ||
			IsTileBlocked(cx, cy)) {
			dashWarn.stopDist = dist;
			break;
		}
	}

	dashWarn.timer = DASH_WARN_INTERVAL;
	dashWarn.isActive = ACTIVE;

	boss.dashDirX = dashWarn.dirX;
	boss.dashDirY = dashWarn.dirY;
	boss.base.state = BOSS_DASH;

	// 방향 업데이트
	if (fabsf(dashWarn.dirX) > fabsf(dashWarn.dirY) * 2) {
		if (dashWarn.dirX > 0) boss.base.direction = DIR_RIGHT;
		else boss.base.direction = DIR_LEFT;
	}
	else if (fabsf(dashWarn.dirY) > fabsf(dashWarn.dirX) * 2) {
		if (dashWarn.dirY > 0) boss.base.direction = DIR_DOWN;
		else boss.base.direction = DIR_UP;
	}
	else {
		if (dashWarn.dirX > 0 && dashWarn.dirY > 0) boss.base.direction = DIR_DOWN_RIGHT;
		else if (dashWarn.dirX > 0 && dashWarn.dirY < 0) boss.base.direction = DIR_UP_RIGHT;
		else if (dashWarn.dirX < 0 && dashWarn.dirY > 0) boss.base.direction = DIR_DOWN_LEFT;
		else if (dashWarn.dirX < 0 && dashWarn.dirY < 0) boss.base.direction = DIR_UP_LEFT;
	}
}

// 점프스킬 착지 위치를 맵 안쪽으로 보정
void ClampBossLanding(float* x, float* y) {
	MAPDATA* m = &maps[currentMapType];
	int halfW = (int)(BOSS_WIDTH * BOSS_JUMP_SCALE / 2);
	int halfH = (int)(BOSS_HEIGHT * BOSS_JUMP_SCALE / 2);

	float minX = m->worldX + (float)(WALL_THICKNESS + 2) * TILE_SIZE + halfW;
	float maxX = m->worldX + (float)(m->cols - WALL_THICKNESS - 1) * TILE_SIZE - halfW - 1;
	float minY = m->worldY + (float)(WALL_THICKNESS + 2) * TILE_SIZE + halfH;
	float maxY = m->worldY + (float)(m->rows - WALL_THICKNESS - 1) * TILE_SIZE - halfH - 1;

	if (*x < minX) *x = minX;
	if (*x > maxX) *x = maxX;
	if (*y < minY) *y = minY;
	if (*y > maxY) *y = maxY;
}

// 보스 스킬(점프 경고)
void StartJumpWarning() {
	boss.base.direction = DIR_DOWN;
	float dx = player.base.x - boss.base.x;
	float dy = player.base.y - boss.base.y;
	float len = sqrtf(dx * dx + dy * dy);
	if (len == 0) return;

	jumpWarn.targetX = player.base.x;
	jumpWarn.targetY = player.base.y;

	ClampBossLanding(&jumpWarn.targetX, &jumpWarn.targetY);

	jumpWarn.timer = BOSS_JUMP_WARN_INTERVAL;
	jumpWarn.isActive = ACTIVE;

	boss.jumpDirX = dx / len;
	boss.jumpDirY = dy / len;
	boss.isJumping = ACTIVE;
	boss.jumpPhase = 0;
	boss.jumpOffsetY = 0;
	boss.base.state = BOSS_JUMP;
}

// 보스 랜덤 이동 처리 (벽 충돌 시 방향 전환)
void UpdateBossMove() {
	boss.base.state = BOSS_MOVE;

	// 방향 전환 타이머
	boss.moveTimer--;
	if (boss.moveTimer <= 0) {
		boss.moveTimer = BOSS_MOVE_INTERVAL;

		// 상하좌우 중 랜덤 선택
		int dir = rand() % 8;
		float diagSpeed = BOSS_MOVE_SPEED * 0.7071f;		// 0.7071 ≒ ( 1/√2 )
		switch (dir) {
		case 0: boss.moveDirX = 1; boss.moveDirY = 0; boss.base.direction = DIR_RIGHT; break;
		case 1: boss.moveDirX = -1; boss.moveDirY = 0; boss.base.direction = DIR_LEFT; break;
		case 2: boss.moveDirX = 0; boss.moveDirY = 1; boss.base.direction = DIR_DOWN; break;
		case 3: boss.moveDirX = 0; boss.moveDirY = -1; boss.base.direction = DIR_UP; break;
		case 4: boss.moveDirX = -1; boss.moveDirY = -1; boss.base.dx = -diagSpeed; boss.base.dy = -diagSpeed; boss.base.direction = DIR_UP_LEFT; break;
		case 5: boss.moveDirX = 1; boss.moveDirY = -1; boss.base.dx = diagSpeed; boss.base.dy = -diagSpeed; boss.base.direction = DIR_UP_RIGHT; break;
		case 6: boss.moveDirX = -1; boss.moveDirY = 1; boss.base.dx = -diagSpeed; boss.base.dy = diagSpeed; boss.base.direction = DIR_DOWN_LEFT; break;
		case 7: boss.moveDirX = 1; boss.moveDirY = 1; boss.base.dx = diagSpeed; boss.base.dy = diagSpeed; boss.base.direction = DIR_DOWN_RIGHT; break;
		}
	}

	int halfW = (int)(BOSS_WIDTH * BOSS_IDLE_SCALE / 2);
	int halfH = (int)(BOSS_HEIGHT * BOSS_IDLE_SCALE / 2);

	// X축 이동 + 벽 충돌 체크
	float nextX = boss.base.x + boss.moveDirX * BOSS_MOVE_SPEED;
	BOOL xBlocked = FALSE;

	if (boss.moveDirX > 0) {
		if (IsTileBlocked(nextX + halfW, boss.base.y - halfH + 2) || IsTileBlocked(nextX + halfW, boss.base.y + halfH - 2)) xBlocked = TRUE;
	}
	else if (boss.moveDirX < 0) {
		if (IsTileBlocked(nextX - halfW, boss.base.y - halfH + 2) || IsTileBlocked(nextX - halfW, boss.base.y + halfH - 2)) xBlocked = TRUE;
	}

	if (!xBlocked) {
		boss.base.x = nextX;
		boss.base.hitBoxX = nextX;
	}
	else {
		boss.moveDirX = -boss.moveDirX; // 벽이면 반대 방향
		boss.moveTimer = 0;
	}

	// Y축 이동 + 벽 충돌 체크
	float nextY = boss.base.y + boss.moveDirY * BOSS_MOVE_SPEED;
	BOOL yBlocked = FALSE;

	if (boss.moveDirY > 0) {
		if (IsTileBlocked(boss.base.x - halfW + 2, nextY + halfH) || IsTileBlocked(boss.base.x + halfW - 2, nextY + halfH)) yBlocked = TRUE;
	}
	else if (boss.moveDirY < 0) {
		if (IsTileBlocked(boss.base.x - halfW + 2, nextY - halfH) || IsTileBlocked(boss.base.x + halfW - 2, nextY - halfH)) yBlocked = TRUE;
	}

	if (!yBlocked) {
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
	boss.base.state = BOSS_CHASE;

	float dx = player.base.x - boss.base.x;
	float dy = player.base.y - boss.base.y;
	float len = sqrtf(dx * dx + dy * dy);
	if (len == 0) return;

	float nx = (dx / len) * BOSS_CHASE_SPEED;
	float ny = (dy / len) * BOSS_CHASE_SPEED;

	int halfW = (int)(BOSS_WIDTH * BOSS_IDLE_SCALE / 2);
	int halfH = (int)(BOSS_HEIGHT * BOSS_IDLE_SCALE / 2);

	// 방향 업데이트
	if (fabsf(dx) > fabsf(dy) * 2) {
		if (dx > 0) boss.base.direction = DIR_RIGHT;
		else boss.base.direction = DIR_LEFT;
	}
	else if (fabsf(dy) > fabsf(dx) * 2) {
		if (dy > 0) boss.base.direction = DIR_DOWN;
		else boss.base.direction = DIR_UP;
	}
	else {
		if (dx > 0 && dy > 0) boss.base.direction = DIR_DOWN_RIGHT;
		else if (dx > 0 && dy < 0) boss.base.direction = DIR_UP_RIGHT;
		else if (dx < 0 && dy > 0) boss.base.direction = DIR_DOWN_LEFT;
		else if (dx < 0 && dy < 0) boss.base.direction = DIR_UP_LEFT;
	}

	float nextX = boss.base.x + nx;
	BOOL xBlocked = FALSE;

	if (nx > 0) {
		if (IsTileBlocked(nextX + halfW, boss.base.y - halfH + 2) || IsTileBlocked(nextX + halfW, boss.base.y + halfH - 2)) xBlocked = TRUE;
	}
	else if (nx < 0) {
		if (IsTileBlocked(nextX - halfW, boss.base.y - halfH + 2) || IsTileBlocked(nextX - halfW, boss.base.y + halfH - 2)) xBlocked = TRUE;
	}

	if (!xBlocked) {
		boss.base.x = nextX;
		boss.base.hitBoxX = nextX;
	}

	float nextY = boss.base.y + ny;
	BOOL yBlocked = FALSE;

	if (ny > 0) {
		if (IsTileBlocked(boss.base.x - halfW + 2, nextY + halfH) || IsTileBlocked(boss.base.x + halfW - 2, nextY + halfH)) yBlocked = TRUE;
	}
	else if (ny < 0) {
		if (IsTileBlocked(boss.base.x - halfW + 2, nextY - halfH) || IsTileBlocked(boss.base.x + halfW - 2, nextY - halfH)) yBlocked = TRUE;
	}

	if (!yBlocked) {
		boss.base.y = nextY;
		boss.base.hitBoxY = nextY;
	}
}

// BOSSPAW 이동 + 충돌처리
void UpdateBossPaws() {
	int pawRadiusW = BOSS_PAW_WIDTH / 2;
	int pawRadiusH = BOSS_PAW_HEIGHT / 2;
	for (int i = 0; i < BOSS_PAW_LIMIT; i++) {
		if (!bossPaws[i].isActive) continue;

		bossPaws[i].x += bossPaws[i].dx;
		bossPaws[i].y += bossPaws[i].dy;
		bossPaws[i].hitBoxX += bossPaws[i].dx;
		bossPaws[i].hitBoxY += bossPaws[i].dy;

		float speed = sqrtf(bossPaws[i].dx * bossPaws[i].dx + bossPaws[i].dy * bossPaws[i].dy);
		if (speed == 0) {
			bossPaws[i].isActive = INACTIVE; continue;
		}

		float ndx = bossPaws[i].dx / speed;
		float ndy = bossPaws[i].dy / speed;

		float edgeX = bossPaws[i].x + ndx * pawRadiusW;
		float edgeY = bossPaws[i].y + ndy * pawRadiusH;

		if (IsTileBlocked(edgeX, edgeY)) {
			bossPaws[i].isActive = INACTIVE;
			continue;
		}

		// 플레이어가 살아있을 때만 충돌 체크
		if (player.base.state != PLAYER_DEAD) {
			HandleBossPawPlayerCollision(&bossPaws[i], &player);
		}
	}
}

// 페이즈 전환 및 보스 사망 처리
int CheckPhaseTransition() {

	// 이미 탈출 중이면 탈출 처리 계속
	if (boss.isEscaping == ACTIVE) {

		// 대기 중 (무적 상태로 바닥에 있음)
		if (boss.escapingDelay > 0) {
			boss.escapingDelay--;
			boss.invincibleTimer = 120; // 매 프레임 무적 갱신

			UpdateAnimation(&boss.anim);
			
			if (boss.anim.currFrame > 1) {
				SetAnimationFrame(&boss.anim, 0);
			}

			return 1;
		}

		SetAnimationFrame(&boss.anim, 2);

		float escapeSpeed = 20.0f;
		boss.jumpOffsetY -= escapeSpeed;

		// 화면 밖으로 충분히 올라갔으면(오프셋이 화면 높이 이상) 실제로 비활성화 + 문 열기
		if (boss.jumpOffsetY < -(float)(SCREEN_HEIGHT + BOSS_HEIGHT)) {
			boss.jumpOffsetY = 0;
			boss.isEscaping = INACTIVE;
			boss.escapingDelay = 0;
			boss.isActive = INACTIVE;
			boss.isSpiralActive = 0;
			boss.doubleDashPhase = 0;
			dashWarn.isActive = INACTIVE;
			jumpWarn.isActive = INACTIVE;

			if (currentMapType == MAP_FIRST_BOSS)
				SetDoorState(MAP_FIRST_BOSS, DOOR_OPEN);
			else if (currentMapType == MAP_SECOND_BOSS)
				SetDoorState(MAP_SECOND_BOSS, DOOR_OPEN);
		}
		return 1; // 탈출 중에는 다른 업데이트 차단
	}

	if (currentMapType == MAP_FIRST_BOSS && boss.base.hp <= (BOSS_HP * 0.75)) {
		boss.isEscaping = ACTIVE;
		boss.escapingDelay = 60;
		boss.jumpOffsetY = 0;
		boss.base.state = BOSS_JUMP;
		SetAnimationFrame(&boss.anim, 0);
		dashWarn.isActive = INACTIVE;
		jumpWarn.isActive = INACTIVE;
		return 1;
	}

	if (currentMapType == MAP_SECOND_BOSS && boss.base.hp <= (BOSS_HP * 0.5)) {
		boss.isEscaping = ACTIVE;
		boss.escapingDelay = 60;
		boss.jumpOffsetY = 0;
		boss.base.state = BOSS_JUMP;
		SetAnimationFrame(&boss.anim, 0);
		dashWarn.isActive = INACTIVE;
		jumpWarn.isActive = INACTIVE;
		return 1;
	}

	if (currentMapType == MAP_THIRD_BOSS && boss.base.hp <= 0) {
		boss.deathTimer = BOSS_DEATH_TIMER;
		boss.base.state = BOSS_DEAD;
		boss.jumpOffsetY = 0;					// 공중 사망 방지
		boss.isJumping = INACTIVE;
		boss.isDashing = INACTIVE;
		SetAnimationFrame(&boss.anim, 0);		// 사망 애니메이션 프레임 초기화
		dashWarn.isActive = INACTIVE;
		jumpWarn.isActive = INACTIVE;
		boss.isSpiralActive = 0;
		boss.doubleDashPhase = 0;
		SetDoorState(MAP_THIRD_BOSS, DOOR_OPEN);
		return 1;
	}
	return 0;
}

// 대시 실행 처리
void UpdateDash(int is3rdPhase) {
	int is2nd3rdPhase = (currentMapType == MAP_SECOND_BOSS || currentMapType == MAP_THIRD_BOSS);

	// [이동 중] 2~4프레임 (인덱스 1, 2, 3) 반복
	if (boss.anim.currFrame < 1 || boss.anim.currFrame > 3) {
		SetAnimationFrame(&boss.anim, 1);
	}

	HandleBossDashPlayerCollision(&player);

	float nx = boss.dashDirX * DASH_SPEED;
	float ny = boss.dashDirY * DASH_SPEED;
	float nextX = boss.base.x + nx;
	float nextY = boss.base.y + ny;

	int halfW = (int)(BOSS_WIDTH * BOSS_DASH_SCALE / 2);
	int halfH = (int)(BOSS_HEIGHT * BOSS_DASH_SCALE / 2);

	BOOL blocked = FALSE;
	// X축 충돌 체크 (이동 방향만)
	if (nx > 0) { // 오른쪽
		if (IsTileBlocked(nextX + halfW, boss.base.y - halfH + 2) || IsTileBlocked(nextX + halfW, boss.base.y + halfH - 2)) blocked = TRUE;
	}
	else if (nx < 0) { // 왼쪽
		if (IsTileBlocked(nextX - halfW, boss.base.y - halfH + 2) || IsTileBlocked(nextX - halfW, boss.base.y + halfH - 2)) blocked = TRUE;
	}

	// Y축 충돌 체크 (이동 방향만)
	if (!blocked) {
		if (ny > 0) { // 아래
			if (IsTileBlocked(boss.base.x - halfW + 2, nextY + halfH) || IsTileBlocked(boss.base.x + halfW - 2, nextY + halfH)) blocked = TRUE;
		}
		else if (ny < 0) { // 위
			if (IsTileBlocked(boss.base.x - halfW + 2, nextY - halfH) || IsTileBlocked(boss.base.x + halfW - 2, nextY - halfH)) blocked = TRUE;
		}
	}

	if (blocked) {
		// 벽 충돌 -> 대쉬 종료
		boss.isDashing = INACTIVE;
		boss.base.state = is2nd3rdPhase ? BOSS_CHASE : BOSS_IDLE;
		boss.attackTimer = BOSS_ATTACK_INTERVAL;
		boss.doubleDashPhase = 0; // 3페이즈 연속 대시도 종료

		// [도착 시] 5프레임 (인덱스 4) 고정
		SetAnimationFrame(&boss.anim, 4);

		// 다음 이동 방향으로 시선 변경
		float dx = player.base.x - boss.base.x;
		float dy = player.base.y - boss.base.y;
		if (is2nd3rdPhase) {
			if (fabsf(dx) > fabsf(dy) * 2) {
				if (dx > 0) boss.base.direction = DIR_RIGHT;
				else boss.base.direction = DIR_LEFT;
			}
			else if (fabsf(dy) > fabsf(dx) * 2) {
				if (dy > 0) boss.base.direction = DIR_DOWN;
				else boss.base.direction = DIR_UP;
			}
			else {
				if (dx > 0 && dy > 0) boss.base.direction = DIR_DOWN_RIGHT;
				else if (dx > 0 && dy < 0) boss.base.direction = DIR_UP_RIGHT;
				else if (dx < 0 && dy > 0) boss.base.direction = DIR_DOWN_LEFT;
				else if (dx < 0 && dy < 0) boss.base.direction = DIR_UP_LEFT;
			}
		}
	}
	else {
		boss.base.x = nextX;
		boss.base.y = nextY;
		boss.base.hitBoxX = nextX;
		boss.base.hitBoxY = nextY;

		HandleBossDashPlayerCollision(&player);

		boss.dashTimer--;

		if (boss.dashTimer <= 0) {
			boss.isDashing = INACTIVE;
			boss.base.state = is2nd3rdPhase ? BOSS_CHASE : BOSS_IDLE;

			if (is3rdPhase && boss.doubleDashPhase == 1) {
				boss.doubleDashPhase = 2;
				boss.doubleDashDelay = BOSS_DOUBLE_DASH_DELAY;
			}
			else {
				boss.attackTimer = BOSS_ATTACK_INTERVAL;
				boss.doubleDashPhase = 0;
			}

			// [도착 시] 5프레임 (인덱스 4) 고정
			SetAnimationFrame(&boss.anim, 4);

			// 다음 이동 방향으로 시선 변경
			float dx = player.base.x - boss.base.x;
			float dy = player.base.y - boss.base.y;
			if (is2nd3rdPhase) {
				if (fabsf(dx) > fabsf(dy) * 2) {
					if (dx > 0) boss.base.direction = DIR_RIGHT;
					else boss.base.direction = DIR_LEFT;
				}
				else if (fabsf(dy) > fabsf(dx) * 2) {
					if (dy > 0) boss.base.direction = DIR_DOWN;
					else boss.base.direction = DIR_UP;
				}
				else {
					if (dx > 0 && dy > 0) boss.base.direction = DIR_DOWN_RIGHT;
					else if (dx > 0 && dy < 0) boss.base.direction = DIR_UP_RIGHT;
					else if (dx < 0 && dy > 0) boss.base.direction = DIR_DOWN_LEFT;
					else if (dx < 0 && dy < 0) boss.base.direction = DIR_UP_LEFT;
				}
			}
		}
	}
}

// 대시 경고 카운트다운
void UpdateDashWarningCountdown() {
	// [준비 중] 1프레임 (인덱스 0) 반복
	if (boss.anim.currFrame > 0) {
		SetAnimationFrame(&boss.anim, 0);
	}

	dashWarn.timer--;
	if (dashWarn.timer <= 0) {
		dashWarn.isActive = INACTIVE;
		boss.isDashing = ACTIVE;
		boss.dashTimer = DASH_INTERVAL;
		PlaySFX(SFX_BOSS_DASH);
	}
}

void UpdateJumpLanding(int is2nd3rdPhase) {
	float jumpSpeed = 40.0f;

	if (boss.jumpPhase == 0) {
		if (boss.anim.currFrame > 1) {
			SetAnimationFrame(&boss.anim, 0);
		}

		boss.invincibleTimer = 10; // 점프 준비 중 무적

		if (jumpWarn.timer <= 60) {
			boss.jumpPhase = 1;

			PlaySFX(SFX_BOSS_JUMP);
		}

		jumpWarn.timer--;
	}
	// [Phase 1] 하늘 위로 빠르게 상승
	else if (boss.jumpPhase == 1) {
		SetAnimationFrame(&boss.anim, 2);

		boss.invincibleTimer = 10; // 상승 중 무적

		jumpWarn.timer--;
		boss.jumpOffsetY -= jumpSpeed;

		if (jumpWarn.timer <= 0) {
			boss.base.x = jumpWarn.targetX;
			boss.base.y = jumpWarn.targetY;
			boss.base.hitBoxX = jumpWarn.targetX;
			boss.base.hitBoxY = jumpWarn.targetY;

			boss.jumpPhase = 2;
		}
	}
	else if (boss.jumpPhase == 2) {
		SetAnimationFrame(&boss.anim, 3);

		boss.invincibleTimer = 10; // 하강 중 무적

		boss.jumpOffsetY += jumpSpeed;

		boss.base.x = jumpWarn.targetX;
		boss.base.y = jumpWarn.targetY;
		boss.base.hitBoxX = jumpWarn.targetX;
		boss.base.hitBoxY = jumpWarn.targetY;

		if (boss.jumpOffsetY >= 0) {
			boss.jumpOffsetY = 0; // 바닥 고정

			jumpWarn.isActive = INACTIVE;

			// 착지하는 순간 바로 종료하지 않고 Phase 3으로 넘김.
			boss.jumpPhase = 3;
			boss.invincibleTimer = 0; // 착지 시 무적 해제

			boss.attackEndTimer = 50;

			HandleBossJumpPlayerCollision(&player);

			PlaySFX(SFX_BOSS_JUMP_LAND);
		}
	}
	// [Phase 3] 착지 완료 후 땅에서 1초 동안 멈춰서 대기
	else if (boss.jumpPhase == 3) {
		SetAnimationFrame(&boss.anim, 4);

		boss.invincibleTimer = 0; // 대기 중에는 데미지 입음

		boss.attackEndTimer--;			// 1초 카운트다운

		// 대기하는 동안 플레이어가 밀치거나 움직여도 보스 본체 및 히트박스는 착지점 고정
		boss.base.dx = 0;
		boss.base.dy = 0;

		if (boss.attackEndTimer <= 0) {
			// 1초 대기가 완전히 끝나면 점프 스킬 전체를 종료.
			boss.isJumping = INACTIVE;
			jumpWarn.isActive = INACTIVE; // 빨간 경고 장판 완전히 끄기
			boss.jumpPhase = 0;           // 페이즈 초기화

			// 원래 상태(추적 또는 이동)로 복귀
			boss.base.state = is2nd3rdPhase ? BOSS_CHASE : BOSS_IDLE;
			boss.attackTimer = BOSS_ATTACK_INTERVAL;
		}
	}
}

void FireRandomCircularPhase(int phase) {
	int count = BOSS_CIRCULARPAWS_COUNT;
	float angleStep = (2.0f * PI) / count;
	float angleOffset = DEG_TO_RAD(phase * 30.0f); // 0도, 30도, 60도

	for (int i = 0; i < count; i++) {
		float currAngle = i * angleStep + angleOffset;

		for (int j = 0; j < BOSS_PAW_LIMIT; j++) {
			if (bossPaws[j].isActive == INACTIVE) {
				bossPaws[j].isActive = ACTIVE;
				bossPaws[j].x = boss.base.x;
				bossPaws[j].y = boss.base.y;
				bossPaws[j].dx = cosf(currAngle) * BOSS_PAW_SPEED;
				bossPaws[j].dy = sinf(currAngle) * BOSS_PAW_SPEED;
				bossPaws[j].width = BOSS_PAW_WIDTH;
				bossPaws[j].height = BOSS_PAW_HEIGHT;
				bossPaws[j].hitBoxX = boss.base.x;
				bossPaws[j].hitBoxY = boss.base.y;
				bossPaws[j].hitBoxW = BOSS_PAW_HITBOX_W;
				bossPaws[j].hitBoxH = BOSS_PAW_HITBOX_H;

				int fw = imgProjectile.width / 4;
				int fh = imgProjectile.height / 18;
				bossPaws[j].dirRow = 17;
				InitAnimation(&bossPaws[j].anim, &imgProjectile, fw, fh, 1, 0, FALSE);

				break;
			}
		}
	}
}

// 랜덤 원형 PAW 3연속 발사 업데이트 (매 프레임 호출)
void UpdateRandomCircularPaws(int is2nd3rdPhase) {
	if (!boss.isRandomCircularActive) return;

	// 딜레이 카운트다운 중
	if (boss.randomCircularDelay > 0) {
		boss.randomCircularDelay--;
		return;
	}

	// 현재 단계 발사
	FireRandomCircularPhase(boss.randomCircularPhase);
	boss.randomCircularPhase++;

	if (boss.randomCircularPhase >= 10) {
		// 3단계 완료 → 종료
		boss.isRandomCircularActive = 0;
		boss.randomCircularPhase = 0;
		boss.attackTimer = BOSS_ATTACK_INTERVAL;
		boss.base.state = is2nd3rdPhase ? BOSS_CHASE : BOSS_IDLE;
	}
	else {
		// 다음 단계까지 1초(60프레임) 대기
		boss.randomCircularDelay = 15;
	}
}

// 랜덤 원형 PAW 3연속 발사 시작
void SpawnRandomCircularPaws() {
	boss.isActive = ACTIVE;
	boss.isRandomCircularActive = 1;
	boss.base.state = BOSS_RANDOM_CATPAW;
	boss.base.direction = DIR_DOWN;
	boss.randomCircularPhase = 0;
	boss.randomCircularDelay = 0; // 첫 발사는 즉시
}

// 3페이즈 회오리 PAW: 프레임마다 호출, 내부 타이머로 순차 발사
void UpdateSpiralPaws(int is2nd3rdPhase) {
	if (!boss.isSpiralActive) return;

	int totalShots = BOSS_SPIRAL_COUNT * BOSS_SPIRAL_ROTATIONS;
	if (boss.spiralIndex >= totalShots) {
		boss.isSpiralActive = 0;
		boss.spiralIndex = 0;
		boss.attackTimer = BOSS_ATTACK_INTERVAL;
		boss.base.state = is2nd3rdPhase ? BOSS_CHASE : BOSS_IDLE;
		return;
	}

	boss.spiralTimer--;
	if (boss.spiralTimer > 0) return;
	boss.spiralTimer = BOSS_SPIRAL_INTERVAL;

	// 각도: 한 바퀴(2PI)를 BOSS_SPIRAL_COUNT 등분, 인덱스마다 누적
	float angle = ((float)boss.spiralIndex / BOSS_SPIRAL_COUNT) * (2.0f * PI);
	float vx = cosf(angle);
	float vy = sinf(angle);

	for (int i = 0; i < BOSS_PAW_LIMIT; i++) {
		if (bossPaws[i].isActive == INACTIVE) {
			bossPaws[i].isActive = ACTIVE;
			bossPaws[i].x = boss.base.x;
			bossPaws[i].y = boss.base.y;
			bossPaws[i].dx = vx * BOSS_PAW_SPEED;
			bossPaws[i].dy = vy * BOSS_PAW_SPEED;
			bossPaws[i].width = BOSS_PAW_WIDTH;
			bossPaws[i].height = BOSS_PAW_HEIGHT;
			bossPaws[i].hitBoxX = boss.base.x;
			bossPaws[i].hitBoxY = boss.base.y;
			bossPaws[i].hitBoxW = BOSS_PAW_HITBOX_W;
			bossPaws[i].hitBoxH = BOSS_PAW_HITBOX_H;

			int fw = imgProjectile.width / 4;
			int fh = imgProjectile.height / 18;
			bossPaws[i].dirRow = 17;
			InitAnimation(&bossPaws[i].anim, &imgProjectile, fw, fh, 1, 0, FALSE);

			break;
		}
	}
	boss.spiralIndex++;
}

// 3페이즈 회오리 PAW 시작
void StartSpiralPaws() {
	boss.isSpiralActive = 1;
	boss.base.state = BOSS_SPIRAL_CATPAW;
	boss.base.direction = DIR_DOWN;
	boss.spiralIndex = 0;
	boss.spiralTimer = 0;
}

// 3페이즈 첫 번째 대시 경고 시작 (doubleDashPhase 설정)
void StartDoubleDashWarning() {
	StartDashWarning();
	boss.doubleDashPhase = 1; // 첫 번째 대시 예약
}

// 패턴 선택
void SelectPattern(int is2nd3rdPhase) {

	int is3rdPhase = (currentMapType == MAP_THIRD_BOSS);

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
	else if (!is3rdPhase) {
		if (pattern <= 2) {
			SpawnBossPaws();
			boss.attackTimer = BOSS_ATTACK_INTERVAL;
		}
		else if (pattern <= 5) {
			boss.skillChargeTimer = BOSS_SKILL_CHARGE_TIME;
			boss.nextSkillState = BOSS_CIRCULAR_CATPAW;
			boss.base.state = BOSS_IDLE;

			SetAnimationFrame(&boss.effectAnim, 0);
		}
		else if (pattern == 6) {
			StartDashWarning();
		}
		else {
			StartJumpWarning();
		}
	}
	else {
		int pattern = rand() % 10;

		if (pattern <= 2) {
			SpawnBossPaws();
			boss.attackTimer = BOSS_ATTACK_INTERVAL;
		}
		else if (pattern <= 4) {
			boss.skillChargeTimer = BOSS_SKILL_CHARGE_TIME;
			boss.nextSkillState = BOSS_CIRCULAR_CATPAW;
			boss.base.state = BOSS_IDLE;

			SetAnimationFrame(&boss.effectAnim, 0);
		}
		else if (pattern <= 6) {
			boss.skillChargeTimer = BOSS_SKILL_CHARGE_TIME;
			boss.nextSkillState = BOSS_RANDOM_CATPAW;
			boss.base.state = BOSS_IDLE;

			SetAnimationFrame(&boss.effectAnim, 0);
		}
		else if (pattern == 7) {
			boss.skillChargeTimer = BOSS_SKILL_CHARGE_TIME;
			boss.nextSkillState = BOSS_SPIRAL_CATPAW;
			boss.base.state = BOSS_IDLE;

			SetAnimationFrame(&boss.effectAnim, 0);
		}
		else if (pattern == 8) {
			StartDoubleDashWarning();
		}
		else {
			StartJumpWarning();
		}
	}
}

// 보스 -> 츄르 쫓기
void HandleBossAggro(float tx, float ty) {
	float ex = boss.base.x;
	float ey = boss.base.y;
	float dx = tx - ex;
	float dy = ty - ey;
	float dist = sqrtf(dx * dx + dy * dy);

	if (dist > 0) {
		// 츄르를 향해 느린 속도로 이동
		float nx = 0, ny = 0;
		if (dist > 10.0f) {
			nx = (dx / dist) * (BOSS_MOVE_SPEED * 0.5f);
			ny = (dy / dist) * (BOSS_MOVE_SPEED * 0.5f);
		}

		int halfW = (int)(BOSS_WIDTH * BOSS_IDLE_SCALE / 2);
		int halfH = (int)(BOSS_HEIGHT * BOSS_IDLE_SCALE / 2);

		float nextX = boss.base.x + nx;
		BOOL xBlocked = FALSE;

		if (nx > 0) {
			if (IsTileBlocked(nextX + halfW, boss.base.y - halfH + 2) || IsTileBlocked(nextX + halfW, boss.base.y + halfH - 2)) xBlocked = TRUE;
		}
		else if (nx < 0) {
			if (IsTileBlocked(nextX - halfW, boss.base.y - halfH + 2) || IsTileBlocked(nextX - halfW, boss.base.y + halfH - 2)) xBlocked = TRUE;
		}

		if (!xBlocked) {
			boss.base.x = nextX;
			boss.base.hitBoxX = nextX;
		}

		float nextY = boss.base.y + ny;
		BOOL yBlocked = FALSE;

		if (ny > 0) {
			if (IsTileBlocked(boss.base.x - halfW + 2, nextY + halfH) || IsTileBlocked(boss.base.x + halfW - 2, nextY + halfH)) yBlocked = TRUE;
		}
		else if (ny < 0) {
			if (IsTileBlocked(boss.base.x - halfW + 2, nextY - halfH) || IsTileBlocked(boss.base.x + halfW - 2, nextY - halfH)) yBlocked = TRUE;
		}

		if (!yBlocked) {
			boss.base.y = nextY;
			boss.base.hitBoxY = nextY;
		}
	}
}

void UpdateBoss() {
	if (currentMapType == MAP_WAITING ||
		currentMapType == MAP_HALLWAY) return;

	UpdateBossPaws();

	// 사망 상태 처리
	if (boss.base.state == BOSS_DEAD) {
		boss.anim.isLoop = FALSE;

		UpdateAnimation(&boss.anim);

		boss.deathTimer--;
		if (boss.deathTimer <= 0) {
			boss.deathTimer = 0;
			boss.isActive = INACTIVE;
		}
		return;
	}

	if (boss.isActive == INACTIVE) return;

	if (boss.invincibleTimer > 0) boss.invincibleTimer--;

	if (boss.skillChargeTimer > 0) {
		boss.skillChargeTimer--;

		int frameIdx = (BOSS_SKILL_CHARGE_TIME - boss.skillChargeTimer) / BOSS_SKILL_CHARGE_UPDATE_FRAME;
		if (frameIdx > 5) frameIdx = 5;			// 마지막이 되면 6프레임(인덱스 5) 고정

		SetAnimationFrame(&boss.effectAnim, frameIdx);
		UpdateAnimation(&boss.anim);

		if (boss.skillChargeTimer <= 0) {
			boss.skillChargeTimer = 0;

			SetAnimationFrame(&boss.effectAnim, 0);

			if (boss.nextSkillState == BOSS_CIRCULAR_CATPAW) {
				SpawnCircularPaws();
				boss.attackTimer = BOSS_ATTACK_INTERVAL;
			}
			else if (boss.nextSkillState == BOSS_RANDOM_CATPAW) {
				SpawnRandomCircularPaws();
				boss.attackTimer = BOSS_ATTACK_INTERVAL;
			}
			else if (boss.nextSkillState == BOSS_SPIRAL_CATPAW) {
				StartSpiralPaws();
				boss.attackTimer = BOSS_ATTACK_INTERVAL;
			}
		}

		return;
	}

	// 보스방 카메라 확대 연출 중에는 보스가 행동하지 않고 가만히 대기
	if (camera.isIntroActive == ACTIVE) {
		boss.base.state = BOSS_IDLE;
		UpdateAnimation(&boss.anim);
		return;
	}

	// 플레이어가 죽었어도 보스는 멈추지 않고 자유롭게 배회
	if (player.base.state == PLAYER_DEAD) {
		dashWarn.isActive = INACTIVE;
		jumpWarn.isActive = INACTIVE;
		boss.isDashing = 0;
		boss.isJumping = 0;
		boss.isSpiralActive = 0;
		boss.isRandomCircularActive = 0;
		boss.isAttacking = 0;
		boss.skillChargeTimer = 0;

		UpdateBossMove();
		UpdateAnimation(&boss.anim);
		return;
	}

	int is2nd3rdPhase = (currentMapType == MAP_SECOND_BOSS || currentMapType == MAP_THIRD_BOSS);
	int is3rdPhase = (currentMapType == MAP_THIRD_BOSS);

	if (CheckPhaseTransition()) return;

	UpdateAnimation(&boss.anim);

	// 3페이즈 회오리 PAW는 다른 스킬과 무관하게 프레임마다 처리
	if (is3rdPhase) UpdateSpiralPaws(is2nd3rdPhase);
	if (is3rdPhase) UpdateRandomCircularPaws(is2nd3rdPhase);

	if (boss.isDashing) {
		UpdateDash(is3rdPhase);
	}
	else if (dashWarn.isActive == ACTIVE) {
		UpdateDashWarningCountdown();
	}

	// 3페이즈 연속 대시: 첫 대시 완료 후 딜레이 -> 두 번째 대시 시작
	else if (is3rdPhase && boss.doubleDashPhase == 2) {
		boss.doubleDashDelay--;
		if (boss.doubleDashDelay <= 0) {
			// 현재 플레이어 방향으로 두 번째 대시 (경고 없이 즉시)
			float dx = player.base.x - boss.base.x;
			float dy = player.base.y - boss.base.y;
			float len = sqrtf(dx * dx + dy * dy);
			if (len > 0) {
				boss.dashDirX = dx / len;
				boss.dashDirY = dy / len;
			}

			// 방향 업데이트
			if (fabsf(dx) > fabsf(dy) * 2) {
				if (dx > 0) boss.base.direction = DIR_RIGHT;
				else boss.base.direction = DIR_LEFT;
			}
			else if (fabsf(dy) > fabsf(dx) * 2) {
				if (dy > 0) boss.base.direction = DIR_DOWN;
				else boss.base.direction = DIR_UP;
			}
			else {
				if (dx > 0 && dy > 0) boss.base.direction = DIR_DOWN_RIGHT;
				else if (dx > 0 && dy < 0) boss.base.direction = DIR_UP_RIGHT;
				else if (dx < 0 && dy > 0) boss.base.direction = DIR_DOWN_LEFT;
				else if (dx < 0 && dy < 0) boss.base.direction = DIR_UP_LEFT;
			}

			boss.isDashing = ACTIVE;
			boss.dashTimer = DASH_INTERVAL;
			boss.doubleDashPhase = 0;				// 두 번째 대시 시작, 이후 UpdateDash에서 완료 처리
			boss.base.state = BOSS_DASH;
			PlaySFX(SFX_BOSS_DASH);
		}
	}
	else if (boss.isJumping == ACTIVE) {
		UpdateJumpLanding(is2nd3rdPhase);
	}
	else if (boss.isSpiralActive) {
	}
	else if (boss.isRandomCircularActive) {
		// 랜덤 원형 3연속 발사 중 -> 이동 없음, 패턴선택 없음 (UpdateRandomCircularPaws가 처리)
	}
	// PAW 발사 직후 정지 구간
	else if (boss.isAttacking) {
		// Three-way 공격 중에는 첫 번째 프레임(인덱스 0)으로 고정
		if (boss.base.state == BOSS_THREE_WAY_CATPAW) {
			SetAnimationFrame(&boss.anim, 0);

			// 발사 직후 절반의 시간 동안만 플레이어를 바라보고, 그 이후에는 이동 방향을 바라보게 함
			if (boss.attackEndTimer < (BOSS_ATTACK_INTERVAL / 6)) {
				float dx = player.base.x - boss.base.x;
				float dy = player.base.y - boss.base.y;
				
				// 페이즈에 따라 추적 또는 랜덤 이동 방향 계산
				if (is2nd3rdPhase) {
					// 추적 방향으로 시선 변경
					if (fabsf(dx) > fabsf(dy) * 2) {
						if (dx > 0) boss.base.direction = DIR_RIGHT;
						else boss.base.direction = DIR_LEFT;
					}
					else if (fabsf(dy) > fabsf(dx) * 2) {
						if (dy > 0) boss.base.direction = DIR_DOWN;
						else boss.base.direction = DIR_UP;
					}
					else {
						if (dx > 0 && dy > 0) boss.base.direction = DIR_DOWN_RIGHT;
						else if (dx > 0 && dy < 0) boss.base.direction = DIR_UP_RIGHT;
						else if (dx < 0 && dy > 0) boss.base.direction = DIR_DOWN_LEFT;
						else if (dx < 0 && dy < 0) boss.base.direction = DIR_UP_LEFT;
					}
				}
				else {
					if (boss.moveDirX == 1 && boss.moveDirY == 0) boss.base.direction = DIR_RIGHT;
					else if (boss.moveDirX == -1 && boss.moveDirY == 0) boss.base.direction = DIR_LEFT;
					else if (boss.moveDirX == 0 && boss.moveDirY == 1) boss.base.direction = DIR_DOWN;
					else if (boss.moveDirX == 0 && boss.moveDirY == -1) boss.base.direction = DIR_UP;
					else if (boss.moveDirX == -1 && boss.moveDirY == -1) boss.base.direction = DIR_UP_LEFT;
					else if (boss.moveDirX == 1 && boss.moveDirY == -1) boss.base.direction = DIR_UP_RIGHT;
					else if (boss.moveDirX == -1 && boss.moveDirY == 1) boss.base.direction = DIR_DOWN_LEFT;
					else if (boss.moveDirX == 1 && boss.moveDirY == 1) boss.base.direction = DIR_DOWN_RIGHT;
				}
			}
		}

		boss.attackEndTimer--;
		if (boss.attackEndTimer <= 0) {
			boss.isAttacking = 0;
			boss.attackTimer = BOSS_ATTACK_INTERVAL;
		}
		// 정지 중: 이동/패턴선택 안 함
	}

	else {
		// 츄르 어그로 체크
		BOOL aggroFound = FALSE;
		if (currentMapType == MAP_FIRST_BOSS || currentMapType == MAP_SECOND_BOSS || currentMapType == MAP_THIRD_BOSS) {
			for (int j = 0; j < CHURU_MAX; j++) {
				if (churues[j].isActive == ACTIVE && churues[j].isDropped == DROPPED) {
					float cdx = churues[j].x - boss.base.x;
					float cdy = churues[j].y - boss.base.y;
					float cDist = sqrtf(cdx * cdx + cdy * cdy);

					if (cDist < CHURU_AGGRO_RANGE) {
						boss.base.state = BOSS_AGGRO;

						if (fabsf(cdx) > fabsf(cdy) * 2) {
							if (cdx > 0) boss.base.direction = DIR_RIGHT;
							else boss.base.direction = DIR_LEFT;
						}
						else if (fabsf(cdy) > fabsf(cdx) * 2) {
							if (cdy > 0) boss.base.direction = DIR_DOWN;
							else boss.base.direction = DIR_UP;
						}
						else {
							if (cdx > 0 && cdy > 0) boss.base.direction = DIR_DOWN_RIGHT;
							else if (cdx > 0 && cdy < 0) boss.base.direction = DIR_UP_RIGHT;
							else if (cdx < 0 && cdy > 0) boss.base.direction = DIR_DOWN_LEFT;
							else if (cdx < 0 && cdy < 0) boss.base.direction = DIR_UP_LEFT;
						}

						HandleBossAggro(churues[j].x, churues[j].y);
						aggroFound = TRUE;
						break;
					}
				}
			}
		}

		if (!aggroFound) {
			if (is2nd3rdPhase)
				UpdateBossChase();
			else
				UpdateBossMove();
		}

		HandleBossPlayerCollision(&player);

		boss.attackTimer--;
		if (boss.attackTimer <= 0)
			SelectPattern(is2nd3rdPhase);
	}
}