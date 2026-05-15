#pragma once
#include <windows.h>

// 타일
void RenderTile(HDC mDC, int screenX, int screenY, COLORREF color);

// 맵
void RenderCurrentMap(HDC mDC);

// 플레이어
void RenderPlayer(HDC mDC);
void RenderPlayerHitBox(HDC mDC);

// 잡몹
void RenderEnemies(HDC mDC);
void RenderEnemiesHitBox(HDC mDC);
// 잡몹 공격
void RenderCatPaw(HDC mDC);

// 보스
void RenderBoss(HDC mDC);

// 총알
void RenderBullets(HDC mDC);