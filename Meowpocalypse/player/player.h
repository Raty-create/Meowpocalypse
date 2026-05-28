#pragma once

#include <Windows.h>
#include "object.h"
#include "animation.h"

typedef struct {
	OBJECT base;
	int mp;
	int invincibleTimer;
	int skillQCooldown;			// Q 스킬 - 쿨타임 타이머
	int boostTimer;				// E 스킬 - 부스트 지속 시간
	int boostCooldown;			// E 스킬 - 부스트 쿨타임
	int fireTimer;				// E 스킬 - 발사 쿨타임 타이머
	int skillRCooldown;			// R 스킬 - 쿨타임 타이머
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