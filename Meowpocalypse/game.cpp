#include "config.h"
#include "game.h"
#include "player.h"
#include "render.h"
#include "camera.h"
#include "map.h"
#include "enemy.h"
#include "bullet.h"
#include "boss.h"
#include "enum.h"
#include "input.h"

int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;

void InitGame() {
	currentMapType = MAP_WAITING;				// 시작 맵 설정
	InitAllMap();								// 맵
	SetDoorState(MAP_WAITING, DOOR_OPEN);		// 문
	InitPlayer();								// 플레이어
	InitEnemy();								// 잡몹
	InitBoss();									// 보스
	InitBullet();								// 총알
	InitChuru();								// 츄르
}

void Update(HWND hWnd) {
	UpdateInput(hWnd);				// 입력 업데이트
	UpdatePlayer();					// 플레이어 업데이트
	ShootSkillQ();					// Q 스킬 - 총알 세 방향 발사
	UpdateEnemies();				// 잡몹 업데이트
	SpawnBoss(currentMapType);		// 보스 스폰
	UpdateBoss();					// 보스 업데이트
	UpdateBullet();					// 총알 업데이트
	ShootBullet();					// 총알 발사
	UpdateChuru();					// 츄르 업데이트
	ShootSkillR();					// R 스킬 - 츄르 던지기
	MAPDATA* m = &maps[currentMapType];									// 카메라(현재 맵 크기를 카메라로 전달)
	UpdateCamera(player.base.x, player.base.y, m->rows, m->cols);		// 카메라 업데이트
}

void Render(HDC hDC) {
	RenderCurrentMap(hDC);				// 현재 맵만 렌더링
	RenderPlayer(hDC);					// 플레이어
	RenderPlayerHitBox(hDC);			// 플레이어 히트박스
	RenderEnemies(hDC);					// 잡몹
	RenderEnemiesHitBox(hDC);			// 잡몹 히트박스
	RenderCatPaw(hDC);					// 잡몹 젤리
	RenderBoss(hDC);					// 보스
	RenderBossHitBox(hDC);				// 보스 히트박스
	RenderBossPaws(hDC);				// 보스 젤리
	RenderBullets(hDC);					// 총알
	RenderChuru(hDC);					// 츄르
}