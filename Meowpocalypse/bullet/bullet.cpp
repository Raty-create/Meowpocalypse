#include <Windows.h>
#include <math.h>

#include "config.h"
#include "bullet.h"
#include "player.h"
#include "camera.h"
#include "collision.h"
#include "enum.h"

BULLET bullets[BULLET_MAX];

void InitBullet() {
	for (int i = 0; i < BULLET_MAX; i++) {
		bullets[i].isActive = INACTIVE;
		bullets[i].width = TILE_SIZE / 3;
		bullets[i].height = TILE_SIZE / 3;
	}
}

// 총알 발사
void ShootBullet(HWND hWnd) {
	static BOOL prevPressed = FALSE;

	BOOL currPressed = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;

	if (!prevPressed && currPressed) {
		for (int i = 0; i < BULLET_MAX; i++) {
			if (bullets[i].isActive == INACTIVE) {

				POINT pt;
				GetCursorPos(&pt);
				ScreenToClient(hWnd, &pt);

				// 플레이어의 스크린 좌표
				float playerScreenX = player.base.x - camera.x;
				float playerScreenY = player.base.y - camera.y;

				// 마우스 방향 벡터
				float dirX = pt.x - playerScreenX;
				float dirY = pt.y - playerScreenY;

				float length = sqrtf(dirX * dirX + dirY * dirY);
				if (length == 0) return;

				bullets[i].x = player.base.x;
				bullets[i].y = player.base.y;
				bullets[i].dx = (dirX / length) * BULLET_SPEED;
				bullets[i].dy = (dirY / length) * BULLET_SPEED;
				bullets[i].isActive = ACTIVE;
				break;
			}
		}
	}

	prevPressed = currPressed;
}

// 총알 업데이트
void UpdateBullet() {
	for (int i = 0; i < BULLET_MAX; i++) {
		if (bullets[i].isActive == INACTIVE) continue;

		bullets[i].x += bullets[i].dx;
		bullets[i].y += bullets[i].dy;

		// 적과의 충돌 체크
		for (int j = 0; j < ENEMY_LIMIT; j++) {
			if (enemies[j].isActive == ACTIVE) {
				if (HandleBulletEnemyCollision(&bullets[i], &enemies[j])) {
					// 충돌 시 해당 총알은 더 이상 처리할 필요 없음
					break;
				}
			}
		}

		// 이미 적과 충돌하여 비활성화되었다면 벽 충돌 체크는 건너뜀
		if (bullets[i].isActive == INACTIVE) continue;

		float bulletNextX = bullets[i].x + bullets[i].dx;
		float bulletNextY = bullets[i].y + bullets[i].dy;

		if (IsTileWall(bulletNextX, bullets[i].y))
			bullets[i].isActive = INACTIVE;

		if (IsTileWall(bullets[i].x, bulletNextY))
			bullets[i].isActive = INACTIVE;
	}
}