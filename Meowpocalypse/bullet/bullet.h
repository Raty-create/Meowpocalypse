#pragma once

#include <Windows.h>

#include "config.h"
#include "enum.h"

typedef struct {
	float x, y;
	float dx, dy;
	int width, height;
	int damage;
	int isActive;
} BULLET;

typedef struct {
	float x, y;
	float dx, dy;
	float startX, startY;
	int width, height;
	int isActive;
	CHURU_STATE isDropped;			// 바닥에 떨어졌는지 여부 (0: 날아가는 중, 1: 바닥)
	int activeTimer;				// 바닥에 머무는 시간 타이머
} CHURU;

extern BULLET bullets[BULLET_MAX];
extern CHURU churues[CHURU_MAX];

void InitBullet();
void ShootBullet();
void UpdateBullet();

void ShootSkillQ();
void InitChuru();
void UpdateChuru();
void ShootSkillR();