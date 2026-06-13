#include "input.h"
#include "bullet.h"

BOOL g_isCheatActive = FALSE;

void Input_Cheat_C() {
	static BOOL prevCPressed = FALSE;
	BOOL currCPressed = (GetAsyncKeyState('C') & 0x8000) != 0;

	if (!prevCPressed && currCPressed) {
		g_isCheatActive = !g_isCheatActive;
		// 즉시 반영
		for (int i = 0; i < BULLET_MAX; i++) {
			bullets[i].damage = g_isCheatActive ? 20050 : BULLET_DAMAGE;
		}
	}
	prevCPressed = currCPressed;
}
