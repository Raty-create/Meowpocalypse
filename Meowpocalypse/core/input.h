#pragma once

#include <Windows.h>
#include "camera.h"
#include "bullet.h"
#include "ui.h"

typedef struct {
	float moveX, moveY;						// 플레이어의 움직임(WASD)
	BOOL isLButtonDown;						// 좌클릭(단일 총알 발사)
	BOOL isLButtonPressed;
	BOOL isQPressed;						// 플레이어 Q
	BOOL isEPressed;						// 플레이어 E
	BOOL isRPressed;						// 플레이어 R
	BOOL isOnePressed;						// 키보드 숫자 1
	BOOL isTwoPressed;						// 키보드 숫자 2
	POINT mousePos;							// 현재 마우스 커서의 위치
} INPUT_STATE;

extern INPUT_STATE g_Input;

void UpdateInput(HWND hWnd);

void InputEsc();

void InputPlayerMovement();
void InputBulletShoot(HWND hWnd);

void InputSkill();
void Input_Skill_Q();
void Input_Skill_E();
void Input_Skill_R();

void InputNum();
void Input_Num_One();
void Input_Num_Two();