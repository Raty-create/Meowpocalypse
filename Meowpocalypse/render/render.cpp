#include "render.h"

IMAGE imgShadow;
IMAGE imgMapTiles[5];
IMAGE imgDoors;

HDC g_hFadeDC = NULL;
HBITMAP g_hFadeBitmap = NULL;

// 레이어 정렬에 사용할 qsort를 위한 비교 함수
int CompareTasks(const void* a, const void* b) {
	RenderTask* taskA = (RenderTask*)a;
	RenderTask* taskB = (RenderTask*)b;

	if (taskA->y < taskB->y) return -1;
	if (taskA->y > taskB->y) return 1;
	return 0;
}

void InitRenderResources() {
	LoadMyImage(&imgShadow, L"SHADOW.png");

	LoadMyImage(&imgMapTiles[MAP_WAITING], L"map_waiting.png");
	LoadMyImage(&imgMapTiles[MAP_HALLWAY], L"hallway.png");
	LoadMyImage(&imgMapTiles[MAP_FIRST_BOSS], L"first_boss_map.png");
	LoadMyImage(&imgMapTiles[MAP_SECOND_BOSS], L"second_boss_map.png");
	LoadMyImage(&imgMapTiles[MAP_THIRD_BOSS], L"third_boss_map.png");

	LoadMyImage(&imgDoors, L"doors.png");

	// Fade/Dimmed 효과용 1x1 검은색 비트맵 미리 생성
	HDC hDC = GetDC(NULL);

	g_hFadeDC = CreateCompatibleDC(hDC);
	g_hFadeBitmap = CreateCompatibleBitmap(hDC, 1, 1);

	SelectObject(g_hFadeDC, g_hFadeBitmap);
	SetPixel(g_hFadeDC, 0, 0, RGB(0, 0, 0));

	ReleaseDC(NULL, hDC);
}

void ReleaseRenderResources() {
	if (g_hFadeDC) {
		DeleteDC(g_hFadeDC);
		g_hFadeDC = NULL;
	}
	if (g_hFadeBitmap) {
		DeleteObject(g_hFadeBitmap);
		g_hFadeBitmap = NULL;
	}
}

void ReleaseMap() {
	ReleaseMyImage(&imgMapTiles[MAP_WAITING]);
	ReleaseMyImage(&imgMapTiles[MAP_HALLWAY]);
	ReleaseMyImage(&imgMapTiles[MAP_FIRST_BOSS]);
	ReleaseMyImage(&imgMapTiles[MAP_SECOND_BOSS]);
	ReleaseMyImage(&imgMapTiles[MAP_THIRD_BOSS]);
}

void ReleaseDoor() {
	ReleaseMyImage(&imgDoors);
}

void ReleaseShadow() {
	ReleaseMyImage(&imgShadow);
}

HBRUSH hBrush, oldBrush;
HPEN hPen, oldPen;
COLORREF color;
int screenX, screenY;
POINT pt;

//타일 색상
COLORREF TileColor(int tileType, DOOR_STATE doorState) {
	switch (tileType) {
	case TILE_FLOOR: return RGB(153, 76, 0);
	case TILE_WALL: return RGB(0, 0, 0);
	case TILE_DOOR: return(doorState == DOOR_OPEN) ? RGB(0, 200, 0) : RGB(139, 69, 0);
	case TILE_OBSTACLE: return RGB(255, 0, 0);
	default: return RGB(0, 0, 0);
	}
}

//타일 그리기 (성능 최적화)
void RenderTile(HDC hDC, int screenX, int screenY, COLORREF color) {
	// CreateSolidBrush 대신 FillRect 사용 (시스템 브러시 활용 가능 시)
	HBRUSH hBrush = CreateSolidBrush(color);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
	
	// 테두리 없는 사각형 그리기 (펜 생성 방지)
	SelectObject(hDC, GetStockObject(NULL_PEN));

	Rectangle(hDC, screenX, screenY, screenX + TILE_SIZE + 1, screenY + TILE_SIZE + 1);

	SelectObject(hDC, oldBrush);
	DeleteObject(hBrush);
}

// 맵 그리기
void RenderCurrentMap(HDC hDC) {
	MAPDATA* m = &maps[currentMapType];
	IMAGE* pMapImg = &imgMapTiles[currentMapType];

	// 화면 전체를 검은색으로 먼저 채워 여백 처리
	hBrush = CreateSolidBrush(RGB(0, 0, 0));
	oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
	hPen = CreatePen(PS_NULL, 0, 0);
	oldPen = (HPEN)SelectObject(hDC, hPen);

	// 화면보다 약간 크게 그려서 테두리 잔상을 확실히 방지
	Rectangle(hDC, -10, -10, SCREEN_WIDTH + 10, SCREEN_HEIGHT + 10);

	SelectObject(hDC, oldBrush); DeleteObject(hBrush);
	SelectObject(hDC, oldPen); DeleteObject(hPen);

	// 이동 가능한 영역(바닥/문)의 범위(Bounding Box)를 계산
	int minC = m->cols, maxC = 0, minR = m->rows, maxR = 0;
	BOOL foundMovable = FALSE;

	for (int r = 0; r < m->rows; r++) {
		for (int c = 0; c < m->cols; c++) {
			if (m->tiles[r][c] == TILE_FLOOR || m->tiles[r][c] == TILE_OBSTACLE) {
				if (c < minC) minC = c;
				if (c > maxC) maxC = c;
				if (r < minR) minR = r;
				if (r > maxR) maxR = r;
				foundMovable = TRUE;
			}
		}
	}

	// 배경 이미지가 있다면 계산된 이동 가능 영역에만 출력
	if (!pMapImg->img.IsNull() && foundMovable) {
		int startX = (int)((m->worldX + minC * TILE_SIZE - camera.x) * camera.zoom);
		int startY = (int)((m->worldY + minR * TILE_SIZE - camera.y) * camera.zoom);
		int drawW = (int)((maxC - minC + 1) * TILE_SIZE * camera.zoom);
		int totalAreaH = (int)((maxR - minR + 1) * TILE_SIZE * camera.zoom);

		// 이미지를 가로 너비(drawW)에 맞췄을 때의 비율(Scale) 계산
		/*float scale = (float)drawW / pMapImg->width;
		int scaledImgH = (int)(pMapImg->height * scale);*/

		DrawMyImage(pMapImg, hDC, startX, startY, drawW, totalAreaH, 0, 0, pMapImg->width, pMapImg->height);


/*		// 영역의 높이가 끝날 때까지 이미지를 위에서부터 아래로 반복해서 그림
		for (int currentY = 0; currentY < totalAreaH; currentY += scaledImgH) {
			int remainingH = totalAreaH - currentY;
			int currentDrawH = (remainingH < scaledImgH) ? remainingH : scaledImgH;

			// 소스 이미지에서 가져올 높이 계산 (마지막 조각은 잘려나갈 수 있음)
			int srcH = (int)(currentDrawH / scale);

			DrawMyImage(pMapImg, hDC, startX, startY + currentY, drawW, currentDrawH, 0, 0, pMapImg->width, srcH);
		}*/
	}
	else if (foundMovable) {
		// 이미지가 없는 경우 (또는 못 찾은 경우) 타일별로 색상 칠하기
		for (int row = minR; row <= maxR; row++) {
			for (int col = minC; col <= maxC; col++) {
				int tileType = m->tiles[row][col];
				if (tileType == TILE_WALL) continue;

				screenX = (int)((m->worldX + col * TILE_SIZE - camera.x) * camera.zoom);
				screenY = (int)((m->worldY + row * TILE_SIZE - camera.y) * camera.zoom);
				int scaledTileSize = (int)(TILE_SIZE * camera.zoom);

				if (screenX + scaledTileSize < 0 || screenX > SCREEN_WIDTH) continue;
				if (screenY + scaledTileSize < 0 || screenY > SCREEN_HEIGHT) continue;

				DOOR_STATE doorstate = DOOR_CLOSE;
				if (tileType == TILE_DOOR) {
					for (int d = 0; d < m->doorCount; d++) {
						if (m->doors[d].row == row && m->doors[d].col == col) {
							doorstate = m->doors[d].state;
							break;
                        }
                    }
                }

				hBrush = CreateSolidBrush(TileColor(tileType, doorstate));
				oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
				hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
				oldPen = (HPEN)SelectObject(hDC, hPen);

				Rectangle(hDC, screenX, screenY, screenX + scaledTileSize, screenY + scaledTileSize);

				SelectObject(hDC, oldBrush); DeleteObject(hBrush);
				SelectObject(hDC, oldPen); DeleteObject(hPen);
			}
		}
	}
}

// 문
void RenderDoors(HDC hDC) {
	MAPDATA* m = &maps[currentMapType];

	if (m->doorCount == 0) return;

	int centerIdx = m->doorCount / 2;
	float doorWorldX = m->worldX + m->doors[centerIdx].col * TILE_SIZE;
	float doorWorldY = m->worldY + m->doors[centerIdx].row * TILE_SIZE;

	int srcYRow = 0;
	float mapRenderOffsetX = 0.0f;					// 맵마다 문 이미지의 좌우 위치를 미세 조정하는 오프셋 (픽셀 단위)
	float mapRenderOffsetY = 0.0f;					// 맵마다 문 이미지의 상하 위치를 미세 조정하는 오프셋 (픽셀 단위)

	if (currentMapType == MAP_WAITING) {
		srcYRow = 0;
		mapRenderOffsetX = 15.0f;
		mapRenderOffsetY = -15.0f;
	}
	else if (currentMapType == MAP_FIRST_BOSS) {
		srcYRow = 1;
		mapRenderOffsetX = 15.0f;
		mapRenderOffsetY = -60.0f;
	}
	else if (currentMapType == MAP_SECOND_BOSS) {
		srcYRow = 2;
		mapRenderOffsetX = 17.0f;
		mapRenderOffsetY = -123.0f;
	}
	else if (currentMapType == MAP_THIRD_BOSS) {
		srcYRow = 3;
		mapRenderOffsetX = 15.0f;
		mapRenderOffsetY = -100.0f;
	}
	else if (currentMapType == MAP_HALLWAY) {
		srcYRow = 4;
		mapRenderOffsetX = 16.0f;
		mapRenderOffsetY = -32.0f;
	}

	int screenX = (int)((doorWorldX + mapRenderOffsetX - camera.x) * camera.zoom);
	int screenY = (int)((doorWorldY + m->doorOffsetY + mapRenderOffsetY - camera.y) * camera.zoom);

	int fw = imgDoors.width / 13;
	int fh = imgDoors.height / 5;

	int srcX = m->doorAnimFrame * fw;
	int srcY = srcYRow * fh;

	int drawW = (int)(fw * camera.zoom);
	int drawH = (int)(fh * camera.zoom);

	DrawMyImage(&imgDoors, hDC, screenX - drawW / 2, screenY - drawH / 2, drawW, drawH, srcX, srcY, fw, fh);
}

// 그림자 공용 함수
void RenderObjectShadow(HDC hDC, float x, float y, int objW) {
	if (imgShadow.img.IsNull()) return;

	int shadowX = (int)((x - camera.x) * camera.zoom);
	int shadowY = (int)((y - camera.y + (float)objW / 3.3f) * camera.zoom);

	int sw = (int)(objW * 0.5f * camera.zoom);
	int sh = (int)(sw / 1.5f);

	DrawMyImage(&imgShadow, hDC, shadowX - sw / 2, shadowY - sh / 2, sw, sh, 0, 0, imgShadow.width, imgShadow.height);
}

// 플레이어
void RenderPlayer(HDC hDC) {
	if (player.invincibleTimer > 0 && (player.invincibleTimer / 5) % 2 == 0) return;

	// zoom을 적용한 스크린 좌표 및 크기 변환
	screenX = (int)((player.base.x - camera.x) * camera.zoom);
	screenY = (int)((player.base.y - camera.y) * camera.zoom);
	int sw = (int)(player.base.width * 6.0f * camera.zoom);
	int sh = (int)(player.base.height * 6.0f * camera.zoom);

	int finalRow = 0;

	switch (player.base.state) {
	case PLAYER_IDLE:
		finalRow = 0;
		break;
	case PLAYER_MOVE:
		switch (player.base.direction) {
		case DIR_DOWN:
			finalRow = 1;
			break;
		case DIR_UP:
			finalRow = 2;
			break;
		case DIR_LEFT:
		case DIR_UP_LEFT:
		case DIR_DOWN_LEFT:
			finalRow = 3;
			break;
		case DIR_RIGHT:
		case DIR_UP_RIGHT:
		case DIR_DOWN_RIGHT:
			finalRow = 4;
			break;
		}
		break;
	case PLAYER_HIT:
		finalRow = 5 + (int)player.base.direction;
		break;
	case PLAYER_SHOOT:
		finalRow = 13 + (int)player.base.direction;
		break;
	case PLAYER_DEAD:
		finalRow = 21 + (int)player.base.direction;
		break;
	default:
		finalRow = 0;
		break;
	}

	RenderAnimation(&player.anim, hDC, screenX, screenY, sw, sh, finalRow);
}

// 플레이어 hitBox
void RenderPlayerHitBox(HDC hDC) {
	screenX = (int)((player.base.hitBoxX - camera.x) * camera.zoom);
	screenY = (int)((player.base.hitBoxY - camera.y) * camera.zoom);
	int sw = (int)(player.base.hitBoxW * camera.zoom);
	int sh = (int)(player.base.hitBoxH * camera.zoom);

	hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	oldPen = (HPEN)SelectObject(hDC, hPen);
	oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

	Rectangle(hDC, screenX - sw / 2, screenY - sh / 2, screenX + sw / 2, screenY + sh / 2);

	SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);
	DeleteObject(hPen);
}

// 잡몹 그리기
void RenderEnemies(HDC hDC) {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		if (!enemies[i].isActive) continue;

		screenX = (int)((enemies[i].base.x - camera.x) * camera.zoom);
		screenY = (int)((enemies[i].base.y - camera.y) * camera.zoom);
		int sw = (int)(enemies[i].base.width * camera.zoom);
		int sh = (int)(enemies[i].base.height * camera.zoom);

		// 애니메이션 행 세트 결정 (각 세트는 8줄씩 차지)
		int baseRow = 0;
		BOOL isDeadState = FALSE; 
		switch (enemies[i].base.state) {
		case ENEMY_IDLE:
		case ENEMY_MOVE:
		case ENEMY_CHASE:
		case ENEMY_AGGRO:
			baseRow = 0;	// 대기/이동 세트 (0~7)
			break;
		case ENEMY_RANGED:
			baseRow = 8;	// 총 쏘기 세트 (8~15)
			break;
		case ENEMY_HIT:
			baseRow = 16;	// 피격 세트 (16~23)
			break;
		case ENEMY_MELEE:
			baseRow = 24;	// 근접 공격 세트 (24~31)
			break;
		case ENEMY_DEAD:
			baseRow = 32;	// 사망 세트 (32) - 방향 무시
			isDeadState = TRUE;
			break;
		default:
			baseRow = 0;
			break;
		}

		// 방향에 따른 오프셋 (사망 상태면 오프셋 0 고정)
		int dirOffset = 0;
		if (!isDeadState) {
			switch (enemies[i].base.direction) {
			case DIR_DOWN:       dirOffset = 0; break;
			case DIR_UP:         dirOffset = 1; break;
			case DIR_LEFT:       dirOffset = 2; break;
			case DIR_RIGHT:      dirOffset = 3; break;
			case DIR_UP_LEFT:    dirOffset = 4; break;
			case DIR_UP_RIGHT:   dirOffset = 5; break;
			case DIR_DOWN_LEFT:  dirOffset = 6; break;
			case DIR_DOWN_RIGHT: dirOffset = 7; break;
			default:             dirOffset = 0; break;
			}
		}

		int finalRow = baseRow + dirOffset;

		RenderAnimation(&enemies[i].anim, hDC, screenX, screenY, sw, sh, finalRow);
	}
}

// 잡몹 hitBox 그리기
void RenderEnemiesHitBox(HDC hDC) {
	for (int i = 0; i < ENEMY_LIMIT; i++) {
		if (!enemies[i].isActive) continue;

		screenX = (int)((enemies[i].base.hitBoxX - camera.x) * camera.zoom);
		screenY = (int)((enemies[i].base.hitBoxY - camera.y) * camera.zoom);
		int sw = (int)(enemies[i].base.hitBoxW * camera.zoom);
		int sh = (int)(enemies[i].base.hitBoxH * camera.zoom);

		hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		oldPen = (HPEN)SelectObject(hDC, hPen);
		oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

		Rectangle(hDC, screenX - sw / 2, screenY - sh / 3, screenX + sw / 2, screenY + sh / 2);

		SelectObject(hDC, oldPen);
		SelectObject(hDC, oldBrush);
		DeleteObject(hPen);
	}
}

// 레이어 정렬을 위한 특정 잡몹 그리기
void RenderSpecificEnemy(HDC hDC, int idx) {
	screenX = (int)((enemies[idx].base.x - camera.x) * camera.zoom);
	screenY = (int)((enemies[idx].base.y - camera.y) * camera.zoom);
	int sw = (int)(enemies[idx].base.width * camera.zoom);
	int sh = (int)(enemies[idx].base.height * camera.zoom);

	// 애니메이션 행 세트 결정 (각 세트는 8줄씩 차지)
	int baseRow = 0;
	BOOL isDeadState = FALSE;
	switch (enemies[idx].base.state) {
	case ENEMY_IDLE:
	case ENEMY_MOVE:
	case ENEMY_CHASE:
	case ENEMY_AGGRO:
		baseRow = 0;	// 대기/이동 세트 (0~7)
		break;
	case ENEMY_RANGED:
		baseRow = 8;	// 총 쏘기 세트 (8~15)
		break;
	case ENEMY_HIT:
		baseRow = 16;	// 피격 세트 (16~23)
		break;
	case ENEMY_MELEE:
		baseRow = 24;	// 근접 공격 세트 (24~31)
		break;
	case ENEMY_DEAD:
		baseRow = 32;	// 사망 세트 (32) - 방향 무시
		isDeadState = TRUE;
		break;
	default:
		baseRow = 0;
		break;
	}

	// 방향에 따른 오프셋 (사망 상태면 오프셋 0 고정)
	int dirOffset = 0;
	if (!isDeadState) {
		switch (enemies[idx].base.direction) {
		case DIR_DOWN:       dirOffset = 0; break;
		case DIR_UP:         dirOffset = 1; break;
		case DIR_LEFT:       dirOffset = 2; break;
		case DIR_RIGHT:      dirOffset = 3; break;
		case DIR_UP_LEFT:    dirOffset = 4; break;
		case DIR_UP_RIGHT:   dirOffset = 5; break;
		case DIR_DOWN_LEFT:  dirOffset = 6; break;
		case DIR_DOWN_RIGHT: dirOffset = 7; break;
		default:             dirOffset = 0; break;
		}
	}

	int finalRow = baseRow + dirOffset;

	RenderAnimation(&enemies[idx].anim, hDC, screenX, screenY, sw, sh, finalRow);
}

// 잡몹 젤리
void RenderCatPaw(HDC hDC) {
	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		if (!catpaw[i].isActive) continue;

		screenX = (int)((catpaw[i].x - camera.x) * camera.zoom);
		screenY = (int)((catpaw[i].y - camera.y) * camera.zoom);
		int sw = (int)(catpaw[i].width * camera.zoom);
		int sh = (int)(catpaw[i].height * camera.zoom);

		RenderAnimation(&catpaw[i].anim, hDC, screenX, screenY, sw, sh, catpaw[i].dirRow);
	}
}

// 레이어 정렬을 위한 특정 잡몹 젤리 그리기
void RenderSpecificCatPaw(HDC hDC, int idx) {
	screenX = (int)((catpaw[idx].x - camera.x) * camera.zoom);
	screenY = (int)((catpaw[idx].y - camera.y) * camera.zoom);
	int sw = (int)(catpaw[idx].width * camera.zoom);
	int sh = (int)(catpaw[idx].height * camera.zoom);

	RenderAnimation(&catpaw[idx].anim, hDC, screenX, screenY, sw, sh, catpaw[idx].dirRow);
}

// 잡몹 젤리 hitBox
void RenderCatPawHitBox(HDC hDC) {
	for (int i = 0; i < CAT_PAW_LIMIT; i++) {
		if (!catpaw[i].isActive) continue;

		screenX = (int)((catpaw[i].hitBoxX - camera.x) * camera.zoom);
		screenY = (int)((catpaw[i].hitBoxY - camera.y) * camera.zoom);
		int sw = (int)(catpaw[i].hitBoxW * camera.zoom);
		int sh = (int)(catpaw[i].hitBoxH * camera.zoom);

		hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		oldPen = (HPEN)SelectObject(hDC, hPen);
		oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

		Rectangle(hDC, screenX - sw / 2, screenY - sh / 2, screenX + sw / 2, screenY + sh / 2);

		SelectObject(hDC, oldPen);
		SelectObject(hDC, oldBrush);
		DeleteObject(hPen);
	}
}

// 보스 대시 경고 - Polygon으로 경고 영역 한 번에 그리기
void RenderDashWarning(HDC hDC) {
	if (dashWarn.isActive == INACTIVE) return;

	// 깜빡임: 10프레임 단위로 켜짐/꺼짐 (시작 시점에 바로 그려지도록 조정)
	if ((dashWarn.timer / 10) % 2 != 0) return;

	// 경고 영역의 너비 (히트박스 크기 기준, 방향에 상관없이 일관되게 적용)
	float dashWidth = (BOSS_HITBOX_WIDTH > BOSS_HITBOX_HEIGHT) ? BOSS_HITBOX_WIDTH : BOSS_HITBOX_HEIGHT;
	float halfW = dashWidth / 2.5f;

	// 경고 영역의 끝점 중심
	float ex = dashWarn.startX + dashWarn.dirX * dashWarn.stopDist;
	float ey = dashWarn.startY + dashWarn.dirY * dashWarn.stopDist;

	POINT pts[4];
	// P0: 시작 왼쪽 위
	pts[0].x = (int)((dashWarn.startX + dashWarn.perpX * halfW - camera.x) * camera.zoom);
	pts[0].y = (int)((dashWarn.startY + dashWarn.perpY * halfW - camera.y) * camera.zoom);
	// P1: 끝   왼쪽 위
	pts[1].x = (int)((ex + dashWarn.perpX * halfW - camera.x) * camera.zoom);
	pts[1].y = (int)((ey + dashWarn.perpY * halfW - camera.y) * camera.zoom);
	// P2: 끝   오른쪽 아래
	pts[2].x = (int)((ex - dashWarn.perpX * halfW - camera.x) * camera.zoom);
	pts[2].y = (int)((ey - dashWarn.perpY * halfW - camera.y) * camera.zoom);
	// P3: 시작 오른쪽 아래
	pts[3].x = (int)((dashWarn.startX - dashWarn.perpX * halfW - camera.x) * camera.zoom);
	pts[3].y = (int)((dashWarn.startY - dashWarn.perpY * halfW - camera.y) * camera.zoom);

	hBrush = CreateSolidBrush(RGB(255, 40, 40));
	oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
	hPen = CreatePen(PS_NULL, 0, 0); // 테두리 없음
	oldPen = (HPEN)SelectObject(hDC, hPen);

	Polygon(hDC, pts, 4); // 꼭짓점 4개로 경고 영역 한 번에 그리기

	SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);
	DeleteObject(hPen);
	DeleteObject(hBrush);
}

// 보스 점프 착지 경고 - 타원형 표시
void RenderJumpWarning(HDC hDC) {
	if (jumpWarn.isActive == INACTIVE) return;

	// 깜빡임: 10프레임 단위
	if ((jumpWarn.timer / 10) % 2 == 0) return;

	// 타원형 반지름 설정 (충돌 로직과 동일한 비율)
	int rx = (int)(BOSS_JUMP_LAND_SIZE * BOSS_JUMP_LAND_SIZE_W_RATIO * camera.zoom);
	int ry = (int)(BOSS_JUMP_LAND_SIZE * BOSS_JUMP_LAND_SIZE_H_RATIO * camera.zoom);
	int sx = (int)((jumpWarn.targetX - camera.x) * camera.zoom);
	int sy = (int)((jumpWarn.targetY + 40.0f - camera.y) * camera.zoom);

	hBrush = CreateSolidBrush(RGB(255, 40, 40));
	oldBrush = (HBRUSH)SelectObject(hDC, hBrush);
	hPen = CreatePen(PS_NULL, 0, 0);
	oldPen = (HPEN)SelectObject(hDC, hPen);

	Ellipse(hDC, sx - rx, sy - ry, sx + rx, sy + ry);

	SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);
	DeleteObject(hPen);
	DeleteObject(hBrush);
}

// 보스
void RenderBoss(HDC hDC) {
	if (!boss.isActive) return;

	// zoom을 적용한 스크린 좌표 및 크기 변환
	screenX = (int)((boss.base.x - camera.x) * camera.zoom);
	screenY = (int)((boss.base.y - camera.y + boss.jumpOffsetY) * camera.zoom);
	int bw = (int)(boss.base.width * camera.zoom);
	int bh = (int)(boss.base.height * camera.zoom);

	//  DEAD일 때는 크기를 더 키움
	if (boss.base.state == BOSS_DEAD) {
		bw = (int)(boss.base.width * BOSS_DEAD_SCALE * camera.zoom);
		bh = (int)(boss.base.height * BOSS_DEAD_SCALE * camera.zoom);
	}

	// 대쉬 중일 때는 크기를 더 키움
	if (boss.base.state == BOSS_DASH) {
		bw = (int)(boss.base.width * BOSS_DASH_SCALE * camera.zoom);
		bh = (int)(boss.base.height * BOSS_DASH_SCALE * camera.zoom);
	}

	// 점프 중일 때는 크기를 더 키움
	if (boss.base.state == BOSS_JUMP) {
		bw = (int)(boss.base.width * BOSS_JUMP_SCALE * camera.zoom);
		bh = (int)(boss.base.height * BOSS_JUMP_SCALE * camera.zoom);
	}

	// 보스 본체 그리기
	int finalRow = 0;
	BOOL isDeadState = FALSE;

	switch (boss.base.state) {
	case BOSS_IDLE:
		finalRow = 0;
		break;
	case BOSS_MOVE:
	case BOSS_CHASE:
	case BOSS_AGGRO:
		switch (boss.base.direction) {
		case DIR_DOWN:
			finalRow = 1;
			break;
		case DIR_UP:
			finalRow = 2;
			break;
		case DIR_LEFT:
			finalRow = 3;
			break;
		case DIR_RIGHT:
			finalRow = 4;
			break;
		case DIR_UP_LEFT:
			finalRow = 5;
			break;
		case DIR_UP_RIGHT:
			finalRow = 6;
			break;
		case DIR_DOWN_LEFT:
			finalRow = 7;
			break;
		case DIR_DOWN_RIGHT:
			finalRow = 8;
			break;
		}
		break;
	case BOSS_DASH:
		finalRow = 17 + (int)boss.base.direction;
		break;
	case BOSS_MELEE:
		finalRow = 25 + (int)boss.base.direction;
		break;
	case BOSS_THREE_WAY_CATPAW:	
		finalRow = 34 + (int)boss.base.direction;
		break;
	case BOSS_JUMP:
		finalRow = 33;
		break;
	case BOSS_DEAD:
		finalRow = 9 + (int)boss.base.direction;
		isDeadState = TRUE;
		break;
	default:
		finalRow = 0;
		break;
	}

	RenderAnimation(&boss.anim, hDC, screenX, screenY, bw, bh, finalRow);
}

// 보스 스킬 이펙트
void RenderBossSkillEffect(HDC hDC) {
	if (!boss.isActive || boss.skillChargeTimer <= 0) return;

	// zoom을 적용한 스크린 좌표 및 크기 변환
	screenX = (int)((boss.base.x - camera.x) * camera.zoom);
	screenY = (int)((boss.base.y - camera.y + boss.jumpOffsetY) * camera.zoom);
	int bw = (int)(boss.base.width * camera.zoom);
	int bh = (int)(boss.base.height * camera.zoom);

	// 탄막 스킬 이펙트 그리기
	int ew = (int)(boss.base.width * BOSS_SKILL_EFFECT_SCALE * camera.zoom);
	int eh = (int)(boss.base.height * BOSS_SKILL_EFFECT_SCALE * camera.zoom);

	float offsetX = 0.0f;
	float offsetY = 0.0f;

	int effectRow = 0;

	if (boss.nextSkillState == BOSS_CIRCULAR_CATPAW) {
		effectRow = 0;
		offsetX = -10.0f;
	}
	else if (boss.nextSkillState == BOSS_RANDOM_CATPAW) {
		effectRow = 1;
		offsetX = -10.0f;
		offsetY = -15.0f;
	}
	else if (boss.nextSkillState == BOSS_SPIRAL_CATPAW) {
		effectRow = 2;
		offsetX = -15.0f;
		offsetY = -10.0f;
	}

	int finalEffX = screenX + (int)(offsetX * camera.zoom);
	int finalEffY = screenY + (int)(offsetY * camera.zoom);

	RenderAnimation(&boss.effectAnim, hDC, finalEffX, finalEffY, ew, eh, effectRow);
}

// 보스 hitBox
void RenderBossHitBox(HDC hDC) {

	if (!boss.isActive) return;
	if (boss.isEscaping == ACTIVE) return;
	if (boss.isJumping == ACTIVE && boss.jumpPhase < 3) return;

	screenX = (int)((boss.base.hitBoxX - camera.x) * camera.zoom);
	screenY = (int)((boss.base.hitBoxY - camera.y) * camera.zoom);
	int sw = (int)(boss.base.hitBoxW * camera.zoom);
	int sh = (int)(boss.base.hitBoxH * camera.zoom);

	hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	oldPen = (HPEN)SelectObject(hDC, hPen);
	oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

	Rectangle(hDC, screenX - sw / 2, screenY - sh / 2, screenX + sw / 2, screenY + sh / 2);

	SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);
	DeleteObject(hPen);
}

// 보스 CatPaw
void RenderBossPaws(HDC hDC) {
	for (int i = 0; i < BOSS_PAW_LIMIT; i++) {
		if (bossPaws[i].isActive == INACTIVE) continue;
		
		screenX = (int)((bossPaws[i].x - camera.x) * camera.zoom);
		screenY = (int)((bossPaws[i].y - camera.y) * camera.zoom);
		int rw = (int)(bossPaws[i].width * BOSS_PAW_SCALE * camera.zoom);
		int rh = (int)(bossPaws[i].height * BOSS_PAW_SCALE * camera.zoom);

		RenderAnimation(&bossPaws[i].anim, hDC, screenX, screenY, rw, rh, bossPaws[i].dirRow);
	}
}

// 보스 CatPaw hitBox
void RenderBossPawsHitBox(HDC hDC) {
	for (int i = 0; i < BOSS_PAW_LIMIT; i++) {
		if (bossPaws[i].isActive == INACTIVE) continue;

		screenX = (int)((bossPaws[i].hitBoxX - camera.x) * camera.zoom);
		screenY = (int)((bossPaws[i].hitBoxY - camera.y) * camera.zoom);
		int sw = (int)(bossPaws[i].hitBoxW * camera.zoom);
		int sh = (int)(bossPaws[i].hitBoxH * camera.zoom);

		hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		oldPen = (HPEN)SelectObject(hDC, hPen);
		oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

		Rectangle(hDC, screenX - sw / 2, screenY - sh / 2, screenX + sw / 2, screenY + sh / 2);

		SelectObject(hDC, oldPen);
		SelectObject(hDC, oldBrush);
		DeleteObject(hPen);
	}
}

// 총알
void RenderBullets(HDC hDC) {
	for (int i = 0; i < BULLET_MAX; i++) {
		if (bullets[i].isActive == INACTIVE) continue;

		screenX = (int)((bullets[i].x - camera.x) * camera.zoom);
		screenY = (int)((bullets[i].y - camera.y) * camera.zoom);
		int sw = (int)(bullets[i].width * camera.zoom);
		int sh = (int)(bullets[i].height * camera.zoom);

		RenderAnimation(&bullets[i].anim, hDC, screenX, screenY, sw, sh, bullets[i].dirRow);
	}
}

// 레이어 정렬을 위한 특정 총알 그리기
void RenderSpecificBullets(HDC hDC, int idx) {
	screenX = (int)((bullets[idx].x - camera.x) * camera.zoom);
	screenY = (int)((bullets[idx].y - camera.y) * camera.zoom);
	int sw = (int)(bullets[idx].width * camera.zoom);
	int sh = (int)(bullets[idx].height * camera.zoom);

	RenderAnimation(&bullets[idx].anim, hDC, screenX, screenY, sw, sh, bullets[idx].dirRow);
}

// 총알 hitBox
void RenderBulletsHitBox(HDC hDC) {
	for (int i = 0; i < BULLET_MAX; i++) {
		if (!bullets[i].isActive) continue;

		screenX = (int)((bullets[i].hitBoxX - camera.x) * camera.zoom);
		screenY = (int)((bullets[i].hitBoxY - camera.y) * camera.zoom);
		int sw = (int)(bullets[i].hitBoxW * camera.zoom);
		int sh = (int)(bullets[i].hitBoxH * camera.zoom);

		hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		oldPen = (HPEN)SelectObject(hDC, hPen);
		oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

		Rectangle(hDC, screenX - sw / 2, screenY - sh / 2, screenX + sw / 2, screenY + sh / 2);

		SelectObject(hDC, oldPen);
		SelectObject(hDC, oldBrush);
		DeleteObject(hPen);
	}
}

// 츄르
void RenderChuru(HDC hDC) {
	for (int i = 0; i < CHURU_MAX; i++) {
		if (churues[i].isActive == INACTIVE) continue;

		screenX = (int)((churues[i].x - camera.x) * camera.zoom);
		screenY = (int)((churues[i].y - camera.y) * camera.zoom);
		int sw = (int)(churues[i].width * camera.zoom);
		int sh = (int)(churues[i].height * camera.zoom);

		RenderAnimation(&churues[i].anim, hDC, screenX, screenY, sw, sh, churues[i].dirRow);
	}
}

// 레이어 정렬을 위한 특정 츄르 그리기
void RenderSpecificChuru(HDC hDC, int idx) {
	screenX = (int)((churues[idx].x - camera.x) * camera.zoom);
	screenY = (int)((churues[idx].y - camera.y) * camera.zoom);
	int sw = (int)(churues[idx].width * camera.zoom);
	int sh = (int)(churues[idx].height * camera.zoom);

	RenderAnimation(&churues[idx].anim, hDC, screenX, screenY, sw, sh, churues[idx].dirRow);
}

// 레이어 정렬을 위한 특정 장애물 그리기
void RenderSpecificObstacle(HDC hDC, int idx) {
	if (!obstacles[idx].isActive) return;
	
	int imgW = imgObstacles.width / 4;
	int imgH = imgObstacles.height;
	int srcX = (int)obstacles[idx].subType * imgW;

	screenX = (int)((obstacles[idx].base.x - camera.x) * camera.zoom);
	screenY = (int)((obstacles[idx].base.y - camera.y) * camera.zoom);
	int sw = (int)(obstacles[idx].base.width * camera.zoom);
	int sh = (int)(obstacles[idx].base.height * camera.zoom);

	DrawMyImage(&imgObstacles, hDC, screenX - sw / 2, screenY - sh / 2, sw, sh, srcX, 0, imgW, imgH);
}

// 장애물 hitBox
void RenderObstaclesHitBox(HDC hDC) {
	for (int i = 0; i < OBSTACLE_LIMIT; i++) {
		if (!obstacles[i].isActive) continue;

		screenX = (int)((obstacles[i].base.hitBoxX - camera.x) * camera.zoom);
		screenY = (int)((obstacles[i].base.hitBoxY - camera.y) * camera.zoom);
		int sw = (int)(obstacles[i].base.hitBoxW * camera.zoom);
		int sh = (int)(obstacles[i].base.hitBoxH * camera.zoom);

		hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
		oldPen = (HPEN)SelectObject(hDC, hPen);
		oldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));

		Rectangle(hDC, screenX - sw / 2, screenY - sh / 2, screenX + sw / 2, screenY + sh / 2);

		SelectObject(hDC, oldPen);
		SelectObject(hDC, oldBrush);
		DeleteObject(hPen);
	}
}

// UI
void RenderUI(HDC hDC) {
	if (g_UI.gameState == TITLE) {
		RenderTitle(hDC);					// 타이틀 화면 그리기
	}
	else if (g_UI.gameState == INGAME) {
		RenderHUD(hDC);						// 인게임 HUD 그리기
	}
	else if (g_UI.gameState == PAUSE) {
		RenderDimmedBackground(hDC);		// 배경 어둡게 처리
		RenderPause(hDC);
	}
	else if (g_UI.gameState == KEY_GUIDE) {
		RenderDimmedBackground(hDC);		// 배경 어둡게 처리
		RenderKeyGuide(hDC);
	}
}

// 타이틀 UI
void RenderTitle(HDC hDC) {
	// 배경 애니메이션 (UpdateTitle에서 업데이트 수행함)
	DrawMyImage(&g_UI.imgTitleBg, hDC, (int)g_UI.title.titleBg.x - g_UI.title.titleBg.width / 2, (int)g_UI.title.titleBg.y - g_UI.title.titleBg.height / 2, g_UI.title.titleBg.width, g_UI.title.titleBg.height,
		g_UI.title.titleBg.srcX, g_UI.title.titleBg.srcY, g_UI.title.titleBg.srcW, g_UI.title.titleBg.srcH);

	// 텍스트 로고
	DrawMyImage(&g_UI.imgMeowpocalypseTextLogo, hDC, (int)g_UI.title.titleLogo.x - g_UI.title.titleLogo.width / 2, (int)g_UI.title.titleLogo.y - g_UI.title.titleLogo.height / 2, g_UI.title.titleLogo.width, g_UI.title.titleLogo.height,
		g_UI.title.titleLogo.srcX, g_UI.title.titleLogo.srcY, g_UI.title.titleLogo.srcW, g_UI.title.titleLogo.srcH);

	// 버튼 호버 상태 체크 (텍스트 크기 결정을 위함)
	UI_ELEMENT* startBnt = &g_UI.title.startButton;
	UI_ELEMENT* exitBnt = &g_UI.title.exitButton;

	BOOL isStartHover = (g_Input.mousePos.x >= startBnt->x - startBnt->width / 2 && g_Input.mousePos.x <= startBnt->x + startBnt->width / 2 &&
		g_Input.mousePos.y >= startBnt->y - startBnt->height / 2 && g_Input.mousePos.y <= startBnt->y + startBnt->height / 2);
	BOOL isExitHover = (g_Input.mousePos.x >= exitBnt->x - exitBnt->width / 2 && g_Input.mousePos.x <= exitBnt->x + exitBnt->width / 2 &&
		g_Input.mousePos.y >= exitBnt->y - exitBnt->height / 2 && g_Input.mousePos.y <= exitBnt->y + exitBnt->height / 2);

	// Start 버튼 배경 프레임
	DrawMyImage(&g_UI.imgUISheet, hDC, (int)startBnt->x - startBnt->width / 2, (int)startBnt->y - startBnt->height / 2, startBnt->width, startBnt->height,
		startBnt->srcX, startBnt->srcY, startBnt->srcW, startBnt->srcH);

	// Exit 버튼 배경 프레임
	DrawMyImage(&g_UI.imgUISheet, hDC, (int)exitBnt->x - exitBnt->width / 2, (int)exitBnt->y - exitBnt->height / 2, exitBnt->width, exitBnt->height,
		exitBnt->srcX, exitBnt->srcY, exitBnt->srcW, exitBnt->srcH);

	// 타이틀 버튼 텍스트 (START / EXIT)
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(255, 255, 255));
	HFONT hOldFont = (HFONT)GetCurrentObject(hDC, OBJ_FONT);

	SIZE textSize;
	const wchar_t* startStr = L"START";
	const wchar_t* exitStr = L"EXIT";

	// START 텍스트 그리기
	if (isStartHover) SelectObject(hDC, g_UI.hTitleHoverFont);
	else SelectObject(hDC, g_UI.hTitleStartExitFont);
	
	GetTextExtentPoint32(hDC, startStr, lstrlen(startStr), &textSize);
	TextOut(hDC, (int)startBnt->x - (textSize.cx / 2), (int)(startBnt->y - (textSize.cy / 2.5f)), startStr, lstrlen(startStr));

	// EXIT 텍스트 그리기
	if (isExitHover) SelectObject(hDC, g_UI.hTitleHoverFont);
	else SelectObject(hDC, g_UI.hTitleStartExitFont);

	GetTextExtentPoint32(hDC, exitStr, lstrlen(exitStr), &textSize);
	TextOut(hDC, (int)exitBnt->x - (textSize.cx / 2), (int)(exitBnt->y - (textSize.cy / 2.5f)), exitStr, lstrlen(exitStr));

	SelectObject(hDC, hOldFont);
}

// 인게임 UI
void RenderHUD(HDC hDC) {
	// HP 바 배경 프레임
	DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.hud.hpBarFrame.x - g_UI.hud.hpBarFrame.width / 2, (int)g_UI.hud.hpBarFrame.y - g_UI.hud.hpBarFrame.height / 2, g_UI.hud.hpBarFrame.width, g_UI.hud.hpBarFrame.height,
		g_UI.hud.hpBarFrame.srcX, g_UI.hud.hpBarFrame.srcY, g_UI.hud.hpBarFrame.srcW, g_UI.hud.hpBarFrame.srcH);

	// HP 바
	float hpRatio = g_UI.hud.playerVisualHp / PLAYER_HP;
	int currHpW = (g_UI.hud.playerVisualHp > 0) ? (int)ceil(g_UI.hud.hpBar.width * hpRatio) : 0;
	int currHpSrcW = (g_UI.hud.playerVisualHp > 0) ? (int)ceil(g_UI.hud.hpBar.srcW * hpRatio) : 0;

	DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.hud.hpBar.x - g_UI.hud.hpBar.width / 2, (int)g_UI.hud.hpBar.y - g_UI.hud.hpBar.height / 2, currHpW, g_UI.hud.hpBar.height,
		g_UI.hud.hpBar.srcX, g_UI.hud.hpBar.srcY, currHpSrcW, g_UI.hud.hpBar.srcH);

	// MP 바 배경 프레임
	DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.hud.mpBarFrame.x - g_UI.hud.mpBarFrame.width / 2, (int)g_UI.hud.mpBarFrame.y - g_UI.hud.mpBarFrame.height / 2, g_UI.hud.mpBarFrame.width, g_UI.hud.mpBarFrame.height,
		g_UI.hud.mpBarFrame.srcX, g_UI.hud.mpBarFrame.srcY, g_UI.hud.mpBarFrame.srcW, g_UI.hud.mpBarFrame.srcH);

	// MP 바
	float mpRatio = g_UI.hud.playerVisualMp / PLAYER_MP;
	int currMpW = (g_UI.hud.playerVisualMp > 0) ? (int)ceil(g_UI.hud.mpBar.width * mpRatio) : 0;
	int currMpSrcW = (g_UI.hud.playerVisualMp > 0) ? (int)ceil(g_UI.hud.mpBar.srcW * mpRatio) : 0;

	DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.hud.mpBar.x - g_UI.hud.mpBar.width / 2, (int)g_UI.hud.mpBar.y - g_UI.hud.mpBar.height / 2, currMpW, g_UI.hud.mpBar.height,
		g_UI.hud.mpBar.srcX, g_UI.hud.mpBar.srcY, currMpSrcW, g_UI.hud.mpBar.srcH);

	// 스킬 - 아이템 아이콘 배경
	for (int i = 0; i < 5; i++) {
		DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.hud.skill_item_sq[i].x - g_UI.hud.skill_item_sq[i].width / 2, (int)g_UI.hud.skill_item_sq[i].y - g_UI.hud.skill_item_sq[i].height / 2, g_UI.hud.skill_item_sq[i].width, g_UI.hud.skill_item_sq[i].height,
			g_UI.hud.skill_item_sq[i].srcX, g_UI.hud.skill_item_sq[i].srcY, g_UI.hud.skill_item_sq[i].srcW, g_UI.hud.skill_item_sq[i].srcH);
	}

	// 스킬 아이콘
	for (int i = 0; i < 3; i++) {
		DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.hud.skill_Icon[i].x - g_UI.hud.skill_Icon[i].width / 2, (int)g_UI.hud.skill_Icon[i].y - g_UI.hud.skill_Icon[i].height / 2, g_UI.hud.skill_Icon[i].width, g_UI.hud.skill_Icon[i].height,
			g_UI.hud.skill_Icon[i].srcX, g_UI.hud.skill_Icon[i].srcY, g_UI.hud.skill_Icon[i].srcW, g_UI.hud.skill_Icon[i].srcH);
	}

	// HP 포션
	DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.hud.hpPotion.x - g_UI.hud.hpPotion.width / 2, (int)g_UI.hud.hpPotion.y - g_UI.hud.hpPotion.height / 2, g_UI.hud.hpPotion.width, g_UI.hud.hpPotion.height,
		g_UI.hud.hpPotion.srcX, g_UI.hud.hpPotion.srcY, g_UI.hud.hpPotion.srcW, g_UI.hud.hpPotion.srcH);

	// MP 포션
	DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.hud.mpPotion.x - g_UI.hud.mpPotion.width / 2, (int)g_UI.hud.mpPotion.y - g_UI.hud.mpPotion.height / 2, g_UI.hud.mpPotion.width, g_UI.hud.mpPotion.height,
		g_UI.hud.mpPotion.srcX, g_UI.hud.mpPotion.srcY, g_UI.hud.mpPotion.srcW, g_UI.hud.mpPotion.srcH);

	// 쿨타임 및 아이템 개수 표시 설정
	SetBkMode(hDC, TRANSPARENT);
	wchar_t textBuf[16];
	SIZE textSize;

	// 스킬 - 아이템 쿨타임
	HFONT hOldFont = (HFONT)SelectObject(hDC, g_UI.hCooldownFont);
	SetTextColor(hDC, RGB(255, 255, 255));

	int cooldowns[5] = {
		player.skillQCooldown,
		player.boostCooldown,
		player.skillRCooldown,
		player.itemOneCooldown,
		player.itemTwoCooldown
	};
	UI_ELEMENT* icons[5] = {
		&g_UI.hud.skill_Icon[0],
		&g_UI.hud.skill_Icon[1],
		&g_UI.hud.skill_Icon[2],
		&g_UI.hud.hpPotion,
		&g_UI.hud.mpPotion
	};

	for (int i = 0; i < 5; i++) {
		if (cooldowns[i] > 0) {
			// 스킬 - 아이템 아이콘 금지 배경
			DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.hud.skill_item_ban_sq[i].x - g_UI.hud.skill_item_ban_sq[i].width / 2, (int)g_UI.hud.skill_item_ban_sq[i].y - g_UI.hud.skill_item_ban_sq[i].height / 2, g_UI.hud.skill_item_ban_sq[i].width, g_UI.hud.skill_item_ban_sq[i].height,
				g_UI.hud.skill_item_ban_sq[i].srcX, g_UI.hud.skill_item_ban_sq[i].srcY, g_UI.hud.skill_item_ban_sq[i].srcW, g_UI.hud.skill_item_ban_sq[i].srcH);

			// 쿨타임 숫자
			swprintf_s(textBuf, L"%d", cooldowns[i] / 60 + 1);
			GetTextExtentPoint32(hDC, textBuf, lstrlen(textBuf), &textSize);
			TextOut(hDC, (int)icons[i]->x - (textSize.cx / 2), (int)icons[i]->y - (textSize.cy / 2), textBuf, lstrlen(textBuf));
		}
	}

	// 스킬 마나 소모량 텍스트
	SelectObject(hDC, g_UI.hItemCountFont);
	int skillMpCounts[3] = { SKILL_Q_MP, SKILL_E_MP, SKILL_R_MP };

	for (int i = 0; i < 3; i++) {
		swprintf_s(textBuf, L"%d", skillMpCounts[i]);
		TextOut(hDC, (int)g_UI.hud.skill_Icon[i].x, (int)(g_UI.hud.skill_Icon[i].y - g_UI.hud.skill_Icon[i].height / 2.2f), textBuf, lstrlen(textBuf));
	}

	SelectObject(hDC, hOldFont);

	// 아이템 개수
	SetTextColor(hDC, RGB(242, 232, 175));

	int potionCounts[2] = { player.hpPotionCount, player.mpPotionCount };
	UI_ELEMENT* potionIcons[2] = { &g_UI.hud.hpPotion, &g_UI.hud.mpPotion };

	for (int i = 0; i < 2; i++) {
		swprintf_s(textBuf, L"%d", potionCounts[i]);
		TextOut(hDC, (int)(potionIcons[i]->x + potionIcons[i]->width / 3.2f), (int)(potionIcons[i]->y - potionIcons[i]->height / 2.2f), textBuf, lstrlen(textBuf));
	}

	SelectObject(hDC, hOldFont);

	// GUI 로고
	DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.hud.logo_Icon.x - g_UI.hud.logo_Icon.width / 2, (int)g_UI.hud.logo_Icon.y - g_UI.hud.logo_Icon.height / 2, g_UI.hud.logo_Icon.width, g_UI.hud.logo_Icon.height,
		g_UI.hud.logo_Icon.srcX, g_UI.hud.logo_Icon.srcY, g_UI.hud.logo_Icon.srcW, g_UI.hud.logo_Icon.srcH);

	// 플레이어 HP/MP 텍스트
	SelectObject(hDC, g_UI.hItemCountFont);
	SetTextColor(hDC, RGB(255, 255, 255));

	wchar_t playerHpText[32];
	swprintf_s(playerHpText, L"%d / %d", player.base.hp, PLAYER_HP);
	GetTextExtentPoint32(hDC, playerHpText, lstrlen(playerHpText), &textSize);
	TextOut(hDC, (int)g_UI.hud.hpBar.x - (textSize.cx / 2), (int)(g_UI.hud.hpBar.y - (textSize.cy / 2) - 4), playerHpText, lstrlen(playerHpText));

	wchar_t playerMpText[32];
	swprintf_s(playerMpText, L"%d / %d", player.mp, PLAYER_MP);
	GetTextExtentPoint32(hDC, playerMpText, lstrlen(playerMpText), &textSize);
	TextOut(hDC, (int)g_UI.hud.mpBar.x - (textSize.cx / 2), (int)(g_UI.hud.mpBar.y - (textSize.cy / 2) - 4), playerMpText, lstrlen(playerMpText));

	SelectObject(hDC, hOldFont);
	RenderBossHUD(hDC);
}

// 보스 HP 바 HUD
void RenderBossHUD(HDC hDC) {
	if (!g_UI.hud.showBossHp || boss.isActive == INACTIVE) return;

	// 보스 HP 바 배경 프레임
	DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.hud.bossHpBarFrame.x - g_UI.hud.bossHpBarFrame.width / 2, (int)g_UI.hud.bossHpBarFrame.y - g_UI.hud.bossHpBarFrame.height / 2, g_UI.hud.bossHpBarFrame.width, g_UI.hud.bossHpBarFrame.height,
		g_UI.hud.bossHpBarFrame.srcX, g_UI.hud.bossHpBarFrame.srcY, g_UI.hud.bossHpBarFrame.srcW, g_UI.hud.bossHpBarFrame.srcH);

	// 보스 HP 바 (부드러운 감소 시각적 HP 사용)
	float hpRatio = g_UI.hud.bossVisualHp / BOSS_HP;
	if (hpRatio < 0) hpRatio = 0;
	if (hpRatio > 1.0f) hpRatio = 1.0f;

	int currHpW = (int)ceil(g_UI.hud.bossHpBar.width * hpRatio);
	int currHpSrcW = (int)ceil(g_UI.hud.bossHpBar.srcW * hpRatio);

	if (currHpW > 0) {
		DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.hud.bossHpBar.x - g_UI.hud.bossHpBar.width / 2, (int)g_UI.hud.bossHpBar.y - g_UI.hud.bossHpBar.height / 2, currHpW, g_UI.hud.bossHpBar.height,
			g_UI.hud.bossHpBar.srcX, g_UI.hud.bossHpBar.srcY, currHpSrcW, g_UI.hud.bossHpBar.srcH);
	}

	// 보스 엠블렘
	DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.hud.bossEmblem.x - g_UI.hud.bossEmblem.width / 2, (int)g_UI.hud.bossEmblem.y - g_UI.hud.bossEmblem.height / 2, g_UI.hud.bossEmblem.width, g_UI.hud.bossEmblem.height,
		g_UI.hud.bossEmblem.srcX, g_UI.hud.bossEmblem.srcY, g_UI.hud.bossEmblem.srcW, g_UI.hud.bossEmblem.srcH);

	// 보스 HP 텍스트
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(255, 255, 255));
	HFONT hOldFont = (HFONT)SelectObject(hDC, g_UI.hItemCountFont);

	wchar_t hpText[32];
	swprintf_s(hpText, L"%d / %d", boss.base.hp, BOSS_HP);

	SIZE textSize;
	GetTextExtentPoint32(hDC, hpText, lstrlen(hpText), &textSize);
	TextOut(hDC, (int)g_UI.hud.bossHpBar.x - (textSize.cx / 2), (int)(g_UI.hud.bossHpBar.y - (textSize.cy / 2) - 1), hpText, lstrlen(hpText));

	SelectObject(hDC, hOldFont);
}

// 퍼즈 UI
void RenderPause(HDC hDC) {
	// 메뉴 배경
	DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.pause.menuBg.x - g_UI.pause.menuBg.width / 2, (int)g_UI.pause.menuBg.y - g_UI.pause.menuBg.height / 2, g_UI.pause.menuBg.width, g_UI.pause.menuBg.height,
		g_UI.pause.menuBg.srcX, g_UI.pause.menuBg.srcY, g_UI.pause.menuBg.srcW, g_UI.pause.menuBg.srcH);

	// 메뉴 버튼 배경
	for (int i = 0; i < 4; i++) {
		DrawMyImage(&g_UI.imgUISheet, hDC, (int)g_UI.pause.menuButton[i].x - g_UI.pause.menuButton[i].width / 2, (int)g_UI.pause.menuButton[i].y - g_UI.pause.menuButton[i].height / 2, g_UI.pause.menuButton[i].width, g_UI.pause.menuButton[i].height,
			g_UI.pause.menuButton[i].srcX, g_UI.pause.menuButton[i].srcY, g_UI.pause.menuButton[i].srcW, g_UI.pause.menuButton[i].srcH);
	}

	// 퍼즈 메뉴 버튼 텍스트 (Play / Key Guide / Go Back Title / Exit)
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(255, 255, 255));
	HFONT hOldFont = (HFONT)GetCurrentObject(hDC, OBJ_FONT);

	SIZE textSize;
	const wchar_t* pauseMenuButtonsStr[] = { L"Play", L"Key Guide", L"Title", L"Exit" };

	for (int i = 0; i < 4; i++) {
		UI_ELEMENT* buttons = &g_UI.pause.menuButton[i];

		BOOL isButtonHover = (g_Input.mousePos.x >= buttons->x - buttons->width / 2 && g_Input.mousePos.x <= buttons->x + buttons->width / 2 &&
			g_Input.mousePos.y >= buttons->y - buttons->height / 2 && g_Input.mousePos.y <= buttons->y + buttons->height / 2);

		// 퍼즈 메뉴 버튼 텍스트 그리기
		if (isButtonHover) SelectObject(hDC, g_UI.hTitleHoverFont);
		else SelectObject(hDC, g_UI.hTitleStartExitFont);

		GetTextExtentPoint32(hDC, pauseMenuButtonsStr[i], lstrlen(pauseMenuButtonsStr[i]), &textSize);
		TextOut(hDC, (int)buttons->x - (textSize.cx / 2) - 4, (int)(buttons->y - (textSize.cy / 2.5f)) - 4, pauseMenuButtonsStr[i], lstrlen(pauseMenuButtonsStr[i]));
	}

	SelectObject(hDC, hOldFont);
}

// 키 가이드 UI
void RenderKeyGuide(HDC hDC) {
	// 키 가이드
	DrawMyImage(&g_UI.imgKeyGuide, hDC, (int)g_UI.keyGuide.keyGuideUI.x - g_UI.keyGuide.keyGuideUI.width / 2, (int)g_UI.keyGuide.keyGuideUI.y - g_UI.keyGuide.keyGuideUI.height / 2, g_UI.keyGuide.keyGuideUI.width, g_UI.keyGuide.keyGuideUI.height,
		g_UI.keyGuide.keyGuideUI.srcX, g_UI.keyGuide.keyGuideUI.srcY, g_UI.keyGuide.keyGuideUI.srcW, g_UI.keyGuide.keyGuideUI.srcH);
}

// 게임 오버 UI
void RenderGameOver(HDC hDC) {
	// 버튼 호버 상태 체크 (텍스트 크기 결정을 위함)
	UI_ELEMENT* reStartBnt = &g_UI.gameover.reStartButton;
	UI_ELEMENT* exitBnt = &g_UI.gameover.exitButton;

	BOOL isReStartHover = (g_Input.mousePos.x >= reStartBnt->x - reStartBnt->width / 2 && g_Input.mousePos.x <= reStartBnt->x + reStartBnt->width / 2 &&
		g_Input.mousePos.y >= reStartBnt->y - reStartBnt->height / 2 && g_Input.mousePos.y <= reStartBnt->y + reStartBnt->height / 2);
	BOOL isExitHover = (g_Input.mousePos.x >= exitBnt->x - exitBnt->width / 2 && g_Input.mousePos.x <= exitBnt->x + exitBnt->width / 2 &&
		g_Input.mousePos.y >= exitBnt->y - exitBnt->height / 2 && g_Input.mousePos.y <= exitBnt->y + exitBnt->height / 2);

	// Restart 버튼 배경 프레임
	DrawMyImage(&g_UI.imgUISheet, hDC, (int)reStartBnt->x - reStartBnt->width / 2, (int)reStartBnt->y - reStartBnt->height / 2, reStartBnt->width, reStartBnt->height,
		reStartBnt->srcX, reStartBnt->srcY, reStartBnt->srcW, reStartBnt->srcH);

	// Exit 버튼 배경 프레임
	DrawMyImage(&g_UI.imgUISheet, hDC, (int)exitBnt->x - exitBnt->width / 2, (int)exitBnt->y - exitBnt->height / 2, exitBnt->width, exitBnt->height,
		exitBnt->srcX, exitBnt->srcY, exitBnt->srcW, exitBnt->srcH);

	// 게임 오버 버튼 텍스트 (RESTART / EXIT)
	SetBkMode(hDC, TRANSPARENT);
	HFONT hOldFont = (HFONT)GetCurrentObject(hDC, OBJ_FONT);

	SIZE textSize;
	const wchar_t* gameoverStr = L"GAME OVER";
	const wchar_t* reStartStr = L"RESTART";
	const wchar_t* exitStr = L"EXIT";

	// GAME OVER 텍스트 그리기
	SetTextColor(hDC, RGB(220, 20, 60));
	SelectObject(hDC, g_UI.hGameOverFont);
	GetTextExtentPoint32(hDC, gameoverStr, lstrlen(gameoverStr), &textSize);
	TextOut(hDC, SCREEN_WIDTH / 2 - (textSize.cx / 2), SCREEN_HEIGHT / 2 - (textSize.cy * 2), gameoverStr, lstrlen(gameoverStr));

	// START 텍스트 그리기
	SetTextColor(hDC, RGB(255, 255, 255));

	if (isReStartHover) SelectObject(hDC, g_UI.hTitleHoverFont);
	else SelectObject(hDC, g_UI.hTitleStartExitFont);

	GetTextExtentPoint32(hDC, reStartStr, lstrlen(reStartStr), &textSize);
	TextOut(hDC, (int)reStartBnt->x - (textSize.cx / 2), (int)(reStartBnt->y - (textSize.cy / 1.9f)), reStartStr, lstrlen(reStartStr));

	// EXIT 텍스트 그리기
	if (isExitHover) SelectObject(hDC, g_UI.hTitleHoverFont);
	else SelectObject(hDC, g_UI.hTitleStartExitFont);

	GetTextExtentPoint32(hDC, exitStr, lstrlen(exitStr), &textSize);
	TextOut(hDC, (int)exitBnt->x - (textSize.cx / 2), (int)(exitBnt->y - (textSize.cy / 1.9f)), exitStr, lstrlen(exitStr));

	SelectObject(hDC, hOldFont);
}

// 마우스 커서 그리기
void RenderCursor(HDC hDC) {
	int cursorX = g_Input.mousePos.x;
	int cursorY = g_Input.mousePos.y;

	int sw = imgCursor.width;
	int sh = imgCursor.height;

	int dw = (int)(imgCursor.width / 2.0f);
	int dh = (int)(imgCursor.height / 2.0f);

	DrawMyImage(&imgCursor, hDC, cursorX, cursorY, dw, dh, 0, 0, sw, sh);
}

// 배경 어둡게(흐리게) 처리 (PAUSE 시)
void RenderDimmedBackground(HDC hDC) {
	if (!g_hFadeDC) return;

	BLENDFUNCTION bf;
	bf.AlphaFormat = 0;
	bf.BlendFlags = 0;
	bf.BlendOp = AC_SRC_OVER;
	bf.SourceConstantAlpha = 100;

	AlphaBlend(hDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, g_hFadeDC, 0, 0, 1, 1, bf);
}

// Fade Out - Fade In
void RenderFadeEffect(HDC hDC) {
	if (!g_hFadeDC || g_UI.fadeAlpha <= 0.0f) return;

	// AlphaBlend 설정
	BLENDFUNCTION bf;
	bf.AlphaFormat = 0;
	bf.BlendFlags = 0;
	bf.BlendOp = AC_SRC_OVER;

	BYTE alpha = (BYTE)(g_UI.fadeAlpha * 255.0f);
	bf.SourceConstantAlpha = alpha;

	AlphaBlend(hDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, g_hFadeDC, 0, 0, 1, 1, bf);
}