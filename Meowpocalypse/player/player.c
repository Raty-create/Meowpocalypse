#include <Windows.h>
#include "player.h"
#include "config.h"
#include "map.h"

PLAYER player;

void InitPlayer() {
	player.base.x = currentMap.width / 2;
	player.base.y = currentMap.height / 2;
	player.base.width = PLAYER_SIZE;
	player.base.height = PLAYER_SIZE;
	player.base.hp = 200;
	player.mp = 100;
	player.base.state = STOP;
}

void UpdatePlayer() {
	player.base.dx = 0;
	player.base.dy = 0;

	if (GetAsyncKeyState('a') || GetAsyncKeyState('A')) player.base.dx = -2.8f;
	if (GetAsyncKeyState('d') || GetAsyncKeyState('D')) player.base.dx = 2.8f;
	if (GetAsyncKeyState('w') || GetAsyncKeyState('W')) player.base.dy = -2.8f;
	if (GetAsyncKeyState('s') || GetAsyncKeyState('S')) player.base.dy = 2.8f;

	player.base.x += player.base.dx;
	player.base.y += player.base.dy;
}