#pragma once
#include <windows.h>

// 타일
void RenderTile(HDC mDC, int screenX, int screenY, COLORREF color);

// 대기방
void RenderWaitingMap(HDC mDC);
void RenderFirstHallWayMap(HDC mDC);

// 플레이어
void RenderPlayer(HDC mDC);