#pragma once

#include <Windows.h>
#include "object.h"
#include "animation.h"

typedef struct {
	OBJECT base;
	float speed;
	int mp;
	int invincibleTimer;
	int skillQCooldown;			// Q 스킬 - 쿨타임 타이머
	int boostTimer;				// E 스킬 - 부스트 지속 시간
	int boostCooldown;			// E 스킬 - 부스트 쿨타임
	int fireTimer;				// E 스킬 - 발사 쿨타임 타이머
	int skillRCooldown;			// R 스킬 - 쿨타임 타이머
	int itemOneCooldown;		// 키보드 숫자 1(잃은 체력 비례 HP 회복) 아이템 - 쿨타임 타이머
	int itemTwoCooldown;		// 키보드 숫자 2(MP 전체 회복) 아이템 - 쿨타임 타이머
	int hpPotionCount;			// HP 포션 개수
	int mpPotionCount;			// MP 포션 개수
	int deathTimer;
	ANIMATION anim;
} PLAYER;

extern PLAYER player;
extern IMAGE imgPlayerSprite;

void InitPlayer();
void ReleasePlayer();
void UpdatePlayer();
void UpdatePlayerStatus();
void UpdatePlayerTimers();
void HandlePlayerInput();
void HandlePlayerMovement();
void MapTransition();

BOOL ConsumeMP(int amount);