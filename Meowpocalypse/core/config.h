#pragma once

#define PI 3.141592f
#define DEG_TO_RAD(deg) (deg * (PI / 180.0f))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define TILE_SIZE 32

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;

#define SCREEN_TILE_X (SCREEN_WIDTH / TILE_SIZE)
#define SCREEN_TILE_Y (SCREEN_HEIGHT / TILE_SIZE)

// MAP
#define TILE_FLOOR 0
#define TILE_WALL 1
#define TILE_DOOR 2

#define WAITINGMAP_ROWS (SCREEN_HEIGHT / TILE_SIZE)
#define WAITINGMAP_COLS (SCREEN_WIDTH / TILE_SIZE)

#define HALLWAYMAP_ROWS (int)(WAITINGMAP_ROWS * 1.5f)
#define HALLWAYMAP_COLS (int)(WAITINGMAP_COLS * 1.5f)

#define BOSSMAP_ROWS (int)(WAITINGMAP_ROWS * 2.0f)
#define BOSSMAP_COLS (int)(WAITINGMAP_COLS * 2.0f)

#define WAITING_X   0
#define HALLWAY1_X  (WAITINGMAP_COLS * TILE_SIZE)
#define BOSS1_X     (HALLWAY1_X + HALLWAYMAP_COLS * TILE_SIZE)
#define HALLWAY2_X  (BOSS1_X   + BOSSMAP_COLS    * TILE_SIZE)
#define BOSS2_X     (HALLWAY2_X + HALLWAYMAP_COLS * TILE_SIZE)
#define HALLWAY3_X  (BOSS2_X   + BOSSMAP_COLS    * TILE_SIZE)
#define BOSS3_X     (HALLWAY3_X + HALLWAYMAP_COLS * TILE_SIZE)

#define MAX_MAP_ROWS 512
#define MAX_MAP_COLS 512
#define MAX_DOORS 16

// PLAYER
#define PLAYER_SIZE 32
#define PLAYER_HITBOX_SIZE 28
#define PLAYER_SPEED 14.8f
#define PLAYER_INVINCIBLE_TIME 60

// ENEMY
#define ENEMY_SIZE 32
#define ENEMY_HITBOX_SIZE 28
#define ENEMY_HP 30
#define ENEMY_LIMIT 1

#define ENEMY_COLS_SPAWN_MARGIN 20
#define ENEMY_ROWS_SPAWN_MARGIN 5

#define ENEMY_MOVE 120

#define ENEMY_CHASE_RANGE 200.0f
#define ENEMY_SPEED 2.5f
#define ENEMY_CONTACT_DAMAGE 5
#define ENEMY_STOP_DISTANCE 30.0f

#define CAT_PAW_SIZE 25
#define CAT_PAW_SPEED 5.0f
#define CAT_PAW_DAMAGE 10
#define CAT_PAW_LIMIT (ENEMY_LIMIT * 2)
#define CAT_PAW_INTERVAL 180

// BOSS
#define BOSS_SIZE (TILE_SIZE * 6)
#define BOSS_HP 500
#define BOSS_PAW_SIZE 64
#define BOSS_HITBOX_SIZE (BOSS_SIZE - 20)
#define BOSS_PAW_SPEED 10.0f
#define BOSS_PAW_INTERVAL 180
#define BOSS_PAW_LIMIT 50
#define BOSS_CIRCULARPAWS_COUNT 32		// 원형 탄막 개수

#define DASH_WARN_TILES 100
#define DASH_WARN_INTERVAL 100
#define DASH_SPEED 18.0f
#define DASH_INTERVAL 50

#define BOSS_ATTACK_INTERVAL 60

#define BOSS_MOVE_SPEED 3.0f
#define BOSS_MOVE_INTERVAL 180

#define BOSS_CHASE_SPEED 5.0f

#define BOSS_JUMP_WARN_INTERVAL 100
#define BOSS_JUMP_LAND_SIZE (BOSS_SIZE * 2)

#define BOSS_CONTACT_DAMAGE    20
#define BOSS_DASH_KNOCKBACK    18.0f
#define BOSS_JUMP_KNOCKBACK    15.0f
#define BOSS_NORMAL_KNOCKBACK  10.0f

#define BOSS_SPIRAL_COUNT      36      // 회오리 한 바퀴 발사 수
#define BOSS_SPIRAL_INTERVAL   4       // 발사 간격 (프레임)
#define BOSS_SPIRAL_ROTATIONS  3       // 총 회전 수

#define BOSS_DOUBLE_DASH_DELAY  10     // 첫 대시 종료 후 두 번째 대시까지 딜레이 (프레임)


// BULLET
#define BULLET_MAX 50
#define BULLET_SPEED 8.0f
#define BULLET_DAMAGE 10
#define KNOCKBACK_FORCE 6.0f
#define KNOCKBACK_TIME 10

// PLAYER SKILL & FIRE RATE
#define PLAYER_FIRE_COOLDOWN 15				// 기본 발사 쿨타임 (프레임)

#define SKILL_Q_MP 20
#define SKILL_Q_COOLDOWN 300				// Q 스킬 쿨타임 (5초)

#define SKILL_E_MP 10
#define BOOST_DURATION 300					// 부스트 지속 시간 (5초)
#define BOOST_COOLDOWN 600					// 부스트 쿨타임 (10초)
#define BOOST_FIRE_RATE_MULTIPLIER 0.5f		// 부스트 시 공격 쿨타임 배율 (낮을수록 빠름)
#define BOOST_SPEED_MULTIPLIER 1.5f			// 부스트 시 이동속도 배율

#define SKILL_R_MP 30
#define SKILL_R_COOLDOWN 480				// R 스킬 쿨타임 (8초)
#define CHURU_SPEED 10.0f
#define CHURU_MAX_DIST 300.0f
#define CHURU_DURATION 300					// 바닥에 머무는 시간 (5초)
#define CHURU_AGGRO_RANGE 400.0f			// 어그로 범위
#define CHURU_MAX 5