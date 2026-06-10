#pragma once

#include <windows.h>

#include "config.h"
#include "player.h"
#include "render.h"
#include "camera.h"
#include "map.h"
#include "enemy.h"
#include "bullet.h"
#include "boss.h"
#include "enum.h"
#include "input.h"
#include "ui.h"
#include "sound.h"

void InitGame();
void Update(HWND hWnd);
void Render(HWND hWnd, HDC mDC);
void ReleaseGame();