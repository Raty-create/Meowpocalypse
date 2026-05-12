#pragma once
#include <windows.h>

// 타일
void RenderTile(HDC mDC, int screenX, int screenY, COLORREF color);

// 맵
void RenderCurrentMap(HDC mDC);

// 플레이어
void RenderPlayer(HDC mDC);

// 총알
void RenderBullets(HDC mDC);