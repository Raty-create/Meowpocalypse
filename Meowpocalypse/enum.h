#pragma once

enum { INACTIVE, ACTIVE };

// GAME
typedef enum { TITLE, INGAME, PAUSE, KEY_GUIDE, GAMEOVER, ENDING } GAME_STATE;

// MAP
typedef enum { MAP_WAITING, MAP_HALLWAY, MAP_FIRST_BOSS, MAP_SECOND_BOSS, MAP_THIRD_BOSS, MAP_COUNT } MAP_TYPE;
typedef enum { DOOR_CLOSE, DOOR_OPEN } DOOR_STATE;

// DIRECTION
typedef enum { DIR_DOWN, DIR_UP, DIR_LEFT, DIR_RIGHT, DIR_UP_LEFT, DIR_UP_RIGHT, DIR_DOWN_LEFT, DIR_DOWN_RIGHT } DIRECTION;

// PLAYER
typedef enum { PLAYER_IDLE, PLAYER_MOVE, PLAYER_SHOOT, PLAYER_HIT, PLAYER_DEAD } PLAYER_STATE;

// ENEMY
typedef enum { ENEMY_IDLE, ENEMY_MOVE, ENEMY_CHASE, ENEMY_MELEE, ENEMY_RANGED, ENEMY_HIT, ENEMY_DEAD, ENEMY_AGGRO } ENEMY_STATE;

// BOSS
typedef enum { BOSS_IDLE, BOSS_MOVE, BOSS_CHASE, BOSS_HIT, BOSS_AGGRO, BOSS_DEAD } BOSS_STATE;
typedef enum { BOSS_DASH = 6, BOSS_MELEE, BOSS_THREE_WAY_CATPAW, BOSS_CIRCULAR_CATPAW, BOSS_JUMP, BOSS_RANDOM_CATPAW, BOSS_SPIRAL_CATPAW } BOSS_SKILL_STATE;

// CHURU
typedef enum { AIRBORNE, DROPPED } CHURU_STATE;

// OBSTACLE
typedef enum { 
    OBS_SOLID,          // 지나가지 못 하게 막는 장애물
    OBS_HAZARD          // 밟으면 상태 이상을 주는 장애물
} OBS_TYPE;

typedef enum {
    // SOLID 계열
    SUB_TOWER_BROKEN,
    SUB_TOWER_TUNNEL,
    SUB_TOWER_TILTED,

    // HAZARD 계열
    SUB_CYBER_SLIME
} OBS_SUBTYPE;

// SOUND
typedef enum { BGM_TITLE, BGM_WAITING, BGM_HALLWAY, BGM_BOSS1, BGM_BOSS2,  BGM_BOSS3, BGM_COUNT } BGM_TYPE;
typedef enum {
    SFX_PLAYER_SHOOT,                      // 총알 발사
    SFX_PLAYER_FOOTSTEP,                   // 플레이어 발소리
    SFX_PLAYER_HIT,                        // 플레이어 피격
    SFX_PLAYER_SKILL_DRINK,                // 스킬 사용 (HP 포션) (MP 포션)
    SFX_PLAYER_SKILL_Q,                    // 스킬 사용 (Q)
    SFX_PLAYER_SKILL_E,                    // 스킬 사용 (E)
    SFX_PLAYER_SKILL_R,                    // 스킬 사용 (R)
    SFX_PLAYER_DEAD,                       // 플레이어 사망

    SFX_BOSS_FOOT_STEP,                    // 보스 발소리
    SFX_BOSS_THREE_WAY_CATPAW,             // 보스 탄막(젤리) 발사
    SFX_BOSS_CIRCULAR_CATPAW,              // 보스 탄막(원형젤리) 발사
    SFX_BOSS_RANDOM_CATPAW,                // 보스 탄막(원형 * 10 젤리) 발사
    SFX_BOSS_SPIRAL_CATPAW,                // 보스 탄막(회오리 젤리) 발사
    SFX_BOSS_DASH,                         // 보스 대시
    SFX_BOSS_JUMP,                         // 보스 점프
    SFX_BOSS_JUMP_LAND,                    // 보스 착지

    SFX_ENEMY_HIT,                         // 잡몹 피격

    SFX_COUNT
} SFX_TYPE;

// SORT LAYER
typedef enum { TYPE_PLAYER, TYPE_ENEMY, TYPE_BOSS, TYPE_CHURU, TYPE_OBSTACLE, TYPE_PLAYER_BULLET, TYPE_CATPAW } RENDER_TYPE;