#include "camera.h"
#include "config.h"
#include "map.h"
#include "boss.h"
#include "enum.h"

CAMERA camera;

void UpdateCamera(float playerX, float playerY, int mapRows, int mapCols) {
	MAPDATA* m = &maps[currentMapType];

	int mapW = mapCols * TILE_SIZE;
	int mapH = mapRows * TILE_SIZE;

	float targetCamX = playerX;
	float targetCamY = playerY;

	// 보스방 입장 연출 처리
	if (camera.isIntroActive == ACTIVE) {
		camera.introTimer--;

		// 3초(약 180프레임) 중 처음 2초(120프레임) 동안은 보스를 강하게 클로즈업
		if (camera.introTimer > 60) {
			camera.zoom = CAMERA_ZOOM_RATIO + 0.5f;
			targetCamX = boss.base.x; // 플레이어가 아닌 보스를 조준
			targetCamY = boss.base.y;
		}
		// 마지막 1초(60프레임) 동안은 1.5배에서 CAMERA_ZOOM_RATIO로 천천히 줌아웃
		else if (camera.introTimer > 0) {
			float t = (float)camera.introTimer / 60.0f; // CAMERA_ZOOM_RATIO -> 0.0
			camera.zoom = CAMERA_ZOOM_RATIO + (0.5f * t);            // 1.5f -> CAMERA_ZOOM_RATIO 로 변함

			// 카메라도 보스에게서 플레이어에게로 천천히 이동
			targetCamX = playerX + (boss.base.x - playerX) * t;
			targetCamY = playerY + (boss.base.y - playerY) * t;
		}
		else {
			// 연출 종료
			camera.isIntroActive = INACTIVE;
			camera.zoom = CAMERA_ZOOM_RATIO;
		}
	}
	else {
		// 보스 인트로 연출이 아닐 때는 일반 줌 CAMERA_ZOOM_RATIO 고정
		camera.zoom = CAMERA_ZOOM_RATIO;
	}

	// 확대율(zoom)이 적용된 카메라 가상 화면의 해상도 크기 계산
	float scaledScreenWidth = (float)SCREEN_WIDTH / camera.zoom;
	float scaledScreenHeight = (float)SCREEN_HEIGHT / camera.zoom;

	if (mapW <= scaledScreenWidth) {
		camera.x = m->worldX + (mapW - scaledScreenWidth) / 2.0f;
	}
	else {
		camera.x = targetCamX - (scaledScreenWidth / 2.0f);
		if (camera.x < m->worldX) {
			camera.x = m->worldX;
		}
		// 제한식을 SCREEN_WIDTH가 아닌 scaledScreenWidth로 검사해야 화면이 튕기지 않음.
		if (camera.x > m->worldX + mapW - scaledScreenWidth) {
			camera.x = m->worldX + mapW - scaledScreenWidth;
		}
	}

	if (mapH <= scaledScreenHeight) {
		camera.y = m->worldY + (mapH - scaledScreenHeight) / 2.0f;
	}
	else {
		camera.y = targetCamY - (scaledScreenHeight / 2.0f);
		if (camera.y < m->worldY) {
			camera.y = m->worldY;
		}
		// 제한식을 scaledScreenHeight로 검사해야 화면이 튕기지 않음.
		if (camera.y > m->worldY + mapH - scaledScreenHeight) {
			camera.y = m->worldY + mapH - scaledScreenHeight;
		}
	}
}