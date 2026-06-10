#pragma once

#include <Windows.h>
#include <math.h>

#include "enum.h"

// 레이어 정렬을 위한 구조체
typedef struct {
	float y;
	RENDER_TYPE type;
	int idx;
} RenderTask;

// 레이어 정렬에 사용할 qsort를 위한 비교 함수
int CompareTasks(const void* a, const void* b);

// 그림자/맵/문 등
void InitRenderResources();
void RenderObjectShadow(HDC hDC, float x, float y, int objW);

// 자원 해제
void ReleaseRenderResources();
void ReleaseMap();
void ReleaseDoor();
void ReleaseShadow();

// 타일
void RenderTile(HDC hDC, int screenX, int screenY, COLORREF color);

// 맵
void RenderCurrentMap(HDC hDC);

// 문
void RenderDoors(HDC hDC);

// 플레이어
void RenderPlayer(HDC hDC);
void RenderPlayerHitBox(HDC hDC);

// 잡몹
void RenderEnemies(HDC hDC);
void RenderEnemiesHitBox(HDC hDC);

void RenderSpecificEnemy(HDC hDC, int idx);				// 레이어 정렬 시 사용(모든 잡몹을 순회할 필요없이 특정 잡몹의 인덱스만 받음)

// 잡몹 공격
void RenderCatPaw(HDC hDC);
void RenderCatPawHitBox(HDC hDC);

// 보스
void RenderBoss(HDC hDC);
void RenderBossSkillEffect(HDC hDC);
void RenderBossHitBox(HDC hDC);
void RenderBossPaws(HDC hDC);
void RenderBossPawsHitBox(HDC hDC);
void RenderDashWarning(HDC hDC);
void RenderJumpWarning(HDC hDC);

// 총알
void RenderBullets(HDC hDC);
void RenderBulletsHitBox(HDC hDC);

// 츄르
void RenderChuru(HDC hDC);

void RenderSpecificChuru(HDC hDC, int idx);				// 레이어 정렬 시 사용(모든 잡몹을 순회할 필요없이 특정 츄르의 인덱스만 받음)

// UI
void RenderUI(HDC hDC);
void RenderTitle(HDC hDC);
void RenderHUD(HDC hDC);
void RenderBossHUD(HDC hDC);
void RenderPause(HDC hDC);
void RenderKeyGuide(HDC hDC);
void RenderGameOver(HDC hDC);

void RenderDimmedBackground(HDC hDC);
void RenderFadeEffect(HDC hDC);