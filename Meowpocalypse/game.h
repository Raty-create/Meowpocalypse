#pragma once
#include <windows.h>

void InitGame();
void Update(HWND hWnd);
void Render(HWND hWnd, HDC mDC);
void ReleaseGame();