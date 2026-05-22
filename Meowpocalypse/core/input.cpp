#include "input.h"

INPUT_STATE g_Input;

void UpdateInput(HWND hWnd) {
	InputPlayerMovement();			// 플레이어 움직임 입력 (WASD)
	InputBulletShoot(hWnd);			// 마우스 입력 (총알 발사)
	InputSkill();					// 스킬 입력
}

// 플레이어 움직임 입력(WASD)
void InputPlayerMovement() {
	g_Input.moveX = 0;
	g_Input.moveY = 0;

	if (GetAsyncKeyState('a') || GetAsyncKeyState('A')) g_Input.moveX -= 1.0f;
	if (GetAsyncKeyState('d') || GetAsyncKeyState('D')) g_Input.moveX += 1.0f;
	if (GetAsyncKeyState('w') || GetAsyncKeyState('W')) g_Input.moveY -= 1.0f;
	if (GetAsyncKeyState('s') || GetAsyncKeyState('S')) g_Input.moveY += 1.0f;
}

// 마우스 입력(총알 발사)
void InputBulletShoot(HWND hWnd) {
	static BOOL prevLButtonDown = FALSE;
	BOOL currLButtonDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

	g_Input.isLButtonDown = currLButtonDown;
	g_Input.isLButtonPressed = (!prevLButtonDown && currLButtonDown);
	prevLButtonDown = currLButtonDown;

	GetCursorPos(&g_Input.mousePos);
	ScreenToClient(hWnd, &g_Input.mousePos);
}

// 플레이어 스킬 입력 감지
void InputSkill() {
	Input_Skill_Q();
	Input_Skill_E();
	Input_Skill_R();
}

// 플레이어 Q 스킬
void Input_Skill_Q() {
	static BOOL prevQPressed = FALSE;
	BOOL currQPressed = (GetAsyncKeyState('Q') & 0x8000) != 0;

	g_Input.isQPressed = (!prevQPressed && currQPressed);
	prevQPressed = currQPressed;
}

// 플레이어 E 스킬
void Input_Skill_E() {
	static BOOL prevEPressed = FALSE;
	BOOL currEPressed = (GetAsyncKeyState('E') & 0x8000) != 0;

	g_Input.isEPressed = (!prevEPressed && currEPressed);
	prevEPressed = currEPressed;
}

// 플레이어 R 스킬
void Input_Skill_R() {
	static BOOL prevRPressed = FALSE;
	BOOL currRPressed = (GetAsyncKeyState('R') & 0x8000) != 0;

	g_Input.isRPressed = (!prevRPressed && currRPressed);
	prevRPressed = currRPressed;
}