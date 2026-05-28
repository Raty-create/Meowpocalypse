#pragma once

typedef struct {
	float x, y;
	int width, height; 
	float zoom;           // 현재 카메라 확대 비율 (1.0 = 기본, 2.0 = 2배 확대)
	int introTimer;       // 보스방 입장 연출 타이머 (프레임 단위)
	int isIntroActive;    // 현재 연출 중인지 여부 (INACTIVE, ACTIVE)
} CAMERA;

extern CAMERA camera;

void UpdateCamera(float plyerX, float playerY, int mapRows, int mapCols);