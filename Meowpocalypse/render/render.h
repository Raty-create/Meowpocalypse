#pragma once

#include <Windows.h>
#include <math.h>

// 타일
void RenderTile(HDC hDC, int screenX, int screenY, COLORREF color);

// 맵
void RenderCurrentMap(HDC hDC);

// 그림자
void InitRenderResources();
void RenderObjectShadow(HDC hDC, float x, float y, int objW);

// 플레이어
void RenderPlayer(HDC hDC);
void RenderPlayerHitBox(HDC hDC);

// 잡몹
void RenderEnemies(HDC hDC);
void RenderEnemiesHitBox(HDC hDC);

// 잡몹 공격
void RenderCatPaw(HDC hDC);
void RenderCatPawHitBox(HDC hDC);

// 보스
void RenderBoss(HDC hDC);
void RenderBossPaws(HDC hDC);
void RenderDashWarning(HDC hDC);
void RenderJumpWarning(HDC hDC);
void RenderBossHitBox(HDC hDC);

// 총알
void RenderBullets(HDC hDC);
void RenderBulletsHitBox(HDC hDC);

// 츄르
void RenderChuru(HDC hDC);

// UI
void RenderUI(HDC hDC);
void RenderTitle(HDC hDC);
void RenderHUD(HDC hDC);
void RenderPause(HDC hDC);

void RenderFadeEffect(HDC hDC);