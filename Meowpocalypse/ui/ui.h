#pragma once

#include <Windows.h>

#include "image.h"

typedef struct {
	float x, y;
	int width, height;
	int isActive;
} UI_ELEMENT;

typedef struct {
	UI_ELEMENT hpBar;
	UI_ELEMENT mpBar;
	UI_ELEMENT skill_Icon[3];		// Q, E, R 아이콘 위치
	UI_ELEMENT bossHpBar;
	BOOL showBossHp;
} HUD_DATA;

typedef struct {
	HUD_DATA hud;

	IMAGE imgHpBarFrame;
	IMAGE imgSkill_Icon[3];

	int gameState;		// 0: Title, 1: InGame, 2: Pause, 3: GameOver
} UI_SYSTEM;

extern UI_SYSTEM g_UI;

/*void InitUI();
void UpdateUI();
void ReleaseUI();*/