#include "config.h"
#include "game.h"
#include "player.h"
#include "enemy.h"
#include "boss.h"
#include "render.h"

void InitGame() {
	InitPlayer();
}

void Update() {
	UpdatePlayer();
}

void Render(HDC mDC) {
	RenderPlayer(mDC);
}