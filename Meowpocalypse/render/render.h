#pragma once
#include <windows.h>

// 타일
void RenderTile(HDC hDC, int screenX, int screenY, COLORREF color);

// 맵
void RenderCurrentMap(HDC hDC);

// 플레이어
void RenderPlayer(HDC mDC);
void RenderPlayerHitBox(HDC hDC);

// 잡몹
void RenderEnemies(HDC hDC);
void RenderEnemiesHitBox(HDC hDC);
// 잡몹 공격
void RenderCatPaw(HDC hDC);

// 보스
void RenderBoss(HDC hDC);
void RenderBossPaws(HDC hDC);
void RenderDashWarning(HDC hDC);
void RenderJumpWarning(HDC hDC);
void RenderBossHitBox(HDC hDC);

// 총알
void RenderBullets(HDC hDC);

// 츄르
void RenderChuru(HDC hDC);