#include "input.h"
#include "config.h"
#include "sound.h"
#include "enum.h"
#include "player.h"

INPUT_STATE g_Input;

void UpdateInput(HWND hWnd) {
	InputEsc();						// 키보드 Esc 입력
	InputPlayerMovement();			// 플레이어 움직임 입력 (WASD)
	InputBulletShoot(hWnd);			// 마우스 입력 (총알 발사)
	InputSkill();					// 스킬 입력
	InputNum();						// 키보드 숫자 입력
}

// 키보드 ESC
void InputEsc() {
	static BOOL prevEscPressed = FALSE;
	BOOL currEscPressed = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;

	if (!prevEscPressed && currEscPressed) {
		if (g_UI.gameState == INGAME) {
			// 카메라 연출 중 ESC 막기
			if (camera.isIntroActive == ACTIVE || g_UI.isPlayerDeadFadeOut) return;

			g_UI.gameState = PAUSE;
			// PauseBGM();
		}
		else if (g_UI.gameState == PAUSE) {
			g_UI.gameState = INGAME;
			// ResumeBGM();
		}
	}
	prevEscPressed = currEscPressed;
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

	// 가상 해상도 및 레터박스 보정
	RECT rt;
	GetClientRect(hWnd, &rt);
	int winW = rt.right;
	int winH = rt.bottom;

	if (winW > 0 && winH > 0) {
		float scaleX = (float)winW / SCREEN_WIDTH;
		float scaleY = (float)winH / SCREEN_HEIGHT;
		float scale = (scaleX < scaleY) ? scaleX : scaleY;

		int destW = (int)(SCREEN_WIDTH * scale);
		int destH = (int)(SCREEN_HEIGHT * scale);
		int destX = (winW - destW) / 2;
		int destY = (winH - destH) / 2;

		// UI용 마우스 좌표 (가상 해상도 1920x1080 기준, 줌 미적용)
		g_Input.mousePos.x = (long)((g_Input.mousePos.x - destX) / scale);
		g_Input.mousePos.y = (long)((g_Input.mousePos.y - destY) / scale);

		// 게임용 마우스 좌표 (카메라 줌 적용)
		g_Input.mouseWorldPos.x = (long)((g_Input.mousePos.x) / camera.zoom);
		g_Input.mouseWorldPos.y = (long)((g_Input.mousePos.y) / camera.zoom);
	}
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
	
	if (g_Input.isQPressed && player.skillQCooldown  <= 0 && player.mp >= SKILL_Q_MP && camera.isIntroActive == INACTIVE && g_UI.gameState != ENDING && 
		!g_UI.isFadeOut && !g_UI.isMapFadeOut && !g_UI.isPlayerDeadFadeOut && !g_UI.isEndingFadeOut && !g_UI.isEndingToTitleFadeOut &&
		!g_UI.isFadeIn && !g_UI.isMapFadeIn && !g_UI.isEndingFadeIn && !g_UI.isEndingToTitleFadeIn) {
		PlaySFX(SFX_PLAYER_SKILL_Q);
	}
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
	
	if (g_Input.isRPressed && player.skillRCooldown <= 0 && player.mp >= SKILL_R_MP && camera.isIntroActive == INACTIVE && g_UI.gameState != ENDING &&
		!g_UI.isFadeOut && !g_UI.isMapFadeOut && !g_UI.isPlayerDeadFadeOut && !g_UI.isEndingFadeOut && !g_UI.isEndingToTitleFadeOut &&
		!g_UI.isFadeIn && !g_UI.isMapFadeIn && !g_UI.isEndingFadeIn && !g_UI.isEndingToTitleFadeIn) {
		PlaySFX(SFX_PLAYER_SKILL_R);
	}
}

// 플레이어 키보드 숫자 입력 감지
void InputNum() {
	Input_Num_One();
	Input_Num_Two();
}

// 키보드 숫자 1 아이템
void Input_Num_One() {
	static BOOL prevOnePressed = FALSE;
	BOOL currOnePressed = (GetAsyncKeyState('1') & 0x8000) != 0;

	g_Input.isOnePressed = (!prevOnePressed && currOnePressed);
	prevOnePressed = currOnePressed;
}

// 키보드 숫자 2 아이템
void Input_Num_Two() {
	static BOOL prevTwoPressed = FALSE;
	BOOL currTwoPressed = (GetAsyncKeyState('2') & 0x8000) != 0;

	g_Input.isTwoPressed = (!prevTwoPressed && currTwoPressed);
	prevTwoPressed = currTwoPressed;
}