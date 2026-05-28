#pragma once

#include <Windows.h>

#include "config.h"
#include "enum.h"
#include "animation.h"

typedef struct {
	float x, y;
	float dx, dy;
	int width, height;
	float hitBoxX, hitBoxY;
	int hitBoxW, hitBoxH;
	int damage;
	int isActive;
	int dirRow;
	ANIMATION anim;
} BULLET;

typedef struct {
	float x, y;
	float dx, dy;
	float startX, startY;
	int width, height;
	int isActive;
	CHURU_STATE isDropped;			// 바닥에 떨어졌는지 여부 (0: 날아가는 중, 1: 바닥)
	int activeTimer;				// 바닥에 머무는 시간 타이머
	int dirRow;
	ANIMATION anim;
} CHURU;

extern BULLET bullets[BULLET_MAX];
extern CHURU churues[CHURU_MAX];
extern IMAGE imgProjectile;

void InitBullet();
void ReleaseBullet();
void ShootBullet();
void UpdateBullet();

void ShootSkillQ();
void InitChuru();
void UpdateChuru();
void ShootSkillR();