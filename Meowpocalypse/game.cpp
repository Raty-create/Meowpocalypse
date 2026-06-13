#include "game.h"

// 전역 렌더링 리소스
HDC g_hGameDC = NULL;
HBITMAP g_hGameBitmap = NULL;
HBITMAP g_hOldGameBitmap = NULL;

static float g_titleBgmVol = 0.0f;

static void StartTitleBGM(int volume) {
	PlayBGM(BGM_TITLE, TRUE, volume);  // 목표 볼륨 기록 (GetBGMTargetVolume이 이 값을 반환)
	SetBGMVolume(0);                   // 실제 재생은 0에서 시작
	g_titleBgmVol = 0.0f;              // swell 리셋
}

void InitGame() {
	currentMapType = MAP_WAITING;				// 시작 맵 설정

	camera.zoom = CAMERA_ZOOM_RATIO;
	camera.isIntroActive = INACTIVE;
	camera.introTimer = 0;

	// 가상 해상도용 DC 및 비트맵 초기화 (1920x1080 고정)
	HDC hDC = GetDC(NULL);
	g_hGameDC = CreateCompatibleDC(hDC);
	g_hGameBitmap = CreateCompatibleBitmap(hDC, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_hOldGameBitmap = (HBITMAP)SelectObject(g_hGameDC, g_hGameBitmap);
	ReleaseDC(NULL, hDC);

	InitAllMap();								// 맵
	SetDoorState(MAP_WAITING, DOOR_OPEN);		// 문
	InitRenderResources();						// 렌더링 리소스
	InitPlayer();								// 플레이어
	InitEnemy();								// 잡몹
	InitBoss();									// 보스
	InitBullet();								// 총알
	InitChuru();								// 츄르
	InitUI();									// UI
	InitSound();
	StartTitleBGM(300);
}

void ReleaseGame() {
	if (g_hGameDC) {
		SelectObject(g_hGameDC, g_hOldGameBitmap);
		DeleteObject(g_hGameBitmap);
		DeleteDC(g_hGameDC);
	}

	ReleasePlayer();
	ReleaseBullet();
	ReleaseEnemy();
	ReleaseBoss();
	ReleaseObstacles();
	ReleaseUI();
	ReleaseMap();
	ReleaseDoor();
	ReleaseShadow();
	ReleaseRenderResources();
	ReleaseSound();
}

void Update(HWND hWnd) {
	UpdateInput(hWnd);				// 입력 업데이트
	UpdateSound();

	// Fade Out - Fade In 효과 업데이트 (상태와 상관없이 매 프레임 수행)
	if (g_UI.isFadeOut) {
		g_UI.fadeAlpha += 0.02f;
		if (g_UI.fadeAlpha >= 1.0f) {
			g_UI.fadeAlpha = 1.0f;
			g_UI.isFadeOut = FALSE;
			g_UI.isFadeIn = TRUE;
			g_UI.gameState = INGAME;
			ReleaseTitle();
			PlayBGM(BGM_WAITING, TRUE, 400);
		}
	}
	else if (g_UI.isMapFadeOut) {
		g_UI.fadeAlpha += 0.04f;

		int target = GetBGMTargetVolume();
		int currentVol = (int)(target * (1.0f - g_UI.fadeAlpha));
		SetBGMVolume(currentVol); // 복도맵(800) -> 0으로 부드럽게 감소

		if (g_UI.fadeAlpha >= 1.0f) {
			g_UI.fadeAlpha = 1.0f;
			g_UI.isMapFadeOut = FALSE;
			g_UI.isMapFadeIn = TRUE;
			ExecuteMapTransition();
			player.base.state = PLAYER_IDLE;
		}
	}
	else if (g_UI.isPlayerDeadFadeOut) {
		g_UI.fadeAlpha += 0.003f;
		if (g_UI.fadeAlpha >= 0.7f) {
			g_UI.fadeAlpha = 0.7f;
			g_UI.isPlayerDeadFadeOut = FALSE;
			g_UI.gameState = GAMEOVER;
		}
	}
	else if (g_UI.isEndingFadeOut) {
		g_UI.fadeAlpha += 0.02f;

		int target = GetBGMTargetVolume();
		float volProgress = g_UI.fadeAlpha * 0.5f;
		SetBGMVolume((int)(target * (1.0f - volProgress)));

		if (g_UI.fadeAlpha >= 1.0f) {
			g_UI.fadeAlpha = 1.0f;
			g_UI.isEndingFadeOut = FALSE;
			g_UI.isEndingFadeIn = TRUE;

			LoadMyImage(&imgEnding, L"final_animation_sprite.png");
			InitEnding();
			g_UI.gameState = ENDING;
		}
	}
	else if (g_UI.isEndingToTitleFadeOut) {
		g_UI.fadeAlpha += 0.02f;
		if (g_UI.fadeAlpha >= 1.0f) {
			g_UI.fadeAlpha = 1.0f;
			g_UI.isEndingToTitleFadeOut = FALSE;
			g_UI.isEndingToTitleFadeIn = TRUE;

			ReleaseMyImage(&imgEnding);

			// 게임 상태 초기화 (전체 리소스를 해제하는 ReleaseGame() 대신 필요한 상태만 초기화)
			currentMapType = MAP_WAITING;
			g_hallwayStage = 0;

			camera.zoom = CAMERA_ZOOM_RATIO;
			camera.isIntroActive = INACTIVE;
			camera.introTimer = 0;

			InitAllMap();
			InitPlayer();
			InitEnemy();
			InitBoss();
			InitBullet();
			InitChuru();
			InitObstacles();

			// UI 상태 일부 초기화 (fadeAlpha는 유지해야 하므로 InitUI() 대신 수동 초기화)
			g_UI.hud.bossVisualHp = (float)BOSS_HP;
			g_UI.hud.showBossHp = FALSE;
			g_UI.hud.playerVisualHp = (float)PLAYER_HP;
			g_UI.hud.playerVisualMp = (float)PLAYER_MP;

			g_UI.gameState = TITLE;
			LoadMyImage(&g_UI.imgTitleBg, L"title_bg.png");
			LoadMyImage(&g_UI.imgMeowpocalypseTextLogo, L"meowpocalypse_logo.png");
			StartTitleBGM(100);
		}
	}
	else if (g_UI.isFadeIn) {
		g_UI.fadeAlpha -= 0.02f;
		if (g_UI.fadeAlpha <= 0.0f) {
			g_UI.fadeAlpha = 0.0f;
			g_UI.isFadeIn = FALSE;
		}
	}
	else if (g_UI.isMapFadeIn) {
		g_UI.fadeAlpha -= 0.04f;
		if (g_UI.fadeAlpha <= 0.0f) {
			g_UI.fadeAlpha = 0.0f;
			g_UI.isMapFadeIn = FALSE;
		}
	}
	else if (g_UI.isEndingFadeIn) {
		g_UI.fadeAlpha -= 0.02f;

		int target = GetBGMTargetVolume();
		float volProgress = 1.0f - (g_UI.fadeAlpha * 0.5f);
		SetBGMVolume((int)(target * (1.0f - volProgress)));

		if (g_UI.fadeAlpha <= 0.0f) {
			g_UI.fadeAlpha = 0.0f;
			g_UI.isEndingFadeIn = FALSE;
		}
	}
	else if (g_UI.isEndingToTitleFadeIn) {
		g_UI.fadeAlpha -= 0.02f;
		if (g_UI.fadeAlpha <= 0.0f) {
			g_UI.fadeAlpha = 0.0f;
			g_UI.isEndingToTitleFadeIn = FALSE;
		}
	}

	if (g_UI.gameState == TITLE) {
		if (g_UI.gameState == TITLE) {
			int target = GetBGMTargetVolume();
			if (g_titleBgmVol < (float)target) {
				g_titleBgmVol += (float)target / 180.0f;
				if (g_titleBgmVol > (float)target) g_titleBgmVol = (float)target;
				SetBGMVolume((int)g_titleBgmVol);
			}
			UpdateTitle();
		}
	}
	else if (g_UI.gameState == PAUSE) {
		UpdatePause();
	}
	else if (g_UI.gameState == KEY_GUIDE) {
		UpdateKeyGuide();
	}
	else if (g_UI.gameState == GAMEOVER) {
		UpdateGameOver();
		UpdateBoss();
	}
	else if (g_UI.gameState == INGAME) {
		// 맵 전환 또는 엔딩 페이드 아웃 중에는 모든 업데이트 중단
		if (g_UI.isMapFadeOut || g_UI.isEndingFadeOut) return;

		// 맵 전환 페이드 인 또는 카메라 인트로 중에도 보스와 HUD 애니메이션은 업데이트 되어야 함
		if (camera.isIntroActive == ACTIVE && (currentMapType == MAP_FIRST_BOSS || currentMapType == MAP_SECOND_BOSS || currentMapType == MAP_THIRD_BOSS)) {

			int target = 200;
			static float introVol = 0.0f;

			introVol += ((float)target / 180.0f);
			if (introVol > (float)target) introVol = (float)target;

			SetBGMVolume((int)introVol);

			if (camera.introTimer <= 0) {
				introVol = 0.0f;
			}

			else if (g_UI.isMapFadeIn) {
				UpdateBoss();
				UpdateBossHpBar();
				UpdateHpBar();
				UpdateMpBar();
				UpdateAnimation(&player.anim);
			}
		}
		else {
			// 실제 게임 플레이 중 업데이트
			UpdatePlayer();					// 플레이어 업데이트
			ShootSkillQ();					// Q 스킬 - 총알 세 방향 발사
			ShootBullet();					// 총알 발사
			ShootSkillR();					// R 스킬 - 츄르 던지기

			UpdateEnemies();				// 잡몹 업데이트
			SpawnBoss(currentMapType);		// 보스 스폰
			UpdateBoss();					// 보스 업데이트

			UpdateMapDoors();

			// 보스 HP HUD 표시 제어
			if (currentMapType == MAP_FIRST_BOSS || currentMapType == MAP_SECOND_BOSS || currentMapType == MAP_THIRD_BOSS) {
				if (boss.isActive) g_UI.hud.showBossHp = TRUE;
				else g_UI.hud.showBossHp = FALSE;
			}
			else {
				g_UI.hud.showBossHp = FALSE;
			}
			UpdateBossHpBar();				// 보스 HP 바 부드러운 감소 업데이트
			UpdateHpBar();					// 플레이어 HP 바 부드러운 감소 업데이트
			UpdateMpBar();					// 플레이어 MP 바 부드러운 감소 업데이트

			UpdateBullet();					// 총알 업데이트
			UpdateChuru();					// 츄르 업데이트
		}

		MAPDATA* m = &maps[currentMapType];									// 카메라(현재 맵 크기를 카메라로 전달)
		UpdateCamera(player.base.x, player.base.y, m->rows, m->cols);		// 카메라 업데이트
	}
	else if (g_UI.gameState == ENDING) {
		UpdateAnimation(&g_UI.endingAnim);
		if (g_UI.endingAnim.isEnd) {
			g_UI.isEndingToTitleFadeOut = TRUE;
		}
	}
}

void Render(HWND hWnd, HDC hDC) {
	RECT rt;
	GetClientRect(hWnd, &rt);
	int winW = rt.right;
	int winH = rt.bottom;

	if (winW <= 0 || winH <= 0) return;

	// 프레임 시작 시 가상 DC(g_hGameDC)를 검은색으로 초기화하여 이전 프레임 잔상 방지
	RECT screenRect = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	FillRect(g_hGameDC, &screenRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

	// 타이틀 화면 렌더링
	if (g_UI.gameState == TITLE) {
		RenderUI(g_hGameDC);
	}
	// 인게임 렌더링
	else if (g_UI.gameState == INGAME || g_UI.gameState == PAUSE || g_UI.gameState == KEY_GUIDE || g_UI.gameState == GAMEOVER) {
		// 모든 게임 그래픽을 가상 DC(g_hGameDC, 1920x1080 기준)에 먼저 그림
		RenderCurrentMap(g_hGameDC);
		RenderDoors(g_hGameDC);

		for (int i = 0; i < OBSTACLE_LIMIT; i++) {
			if (obstacles[i].isActive && obstacles[i].type == OBS_HAZARD) {
				RenderObjectShadow(g_hGameDC, obstacles[i].base.x, obstacles[i].base.y - 40.0f, (int)(obstacles[i].base.width * 1.7f));
				RenderSpecificObstacle(g_hGameDC, i);
			}
		}

		RenderDashWarning(g_hGameDC);
		RenderJumpWarning(g_hGameDC);
		RenderBossSkillEffect(g_hGameDC);

		if (camera.isIntroActive == INACTIVE) {
			RenderObjectShadow(g_hGameDC, player.base.x, player.base.y, (int)(player.base.width * 3.2f));
			for (int i = 0; i < ENEMY_LIMIT; i++) {
				if (enemies[i].isActive)
					RenderObjectShadow(g_hGameDC, enemies[i].base.x, enemies[i].base.y + 5.0f, (int)(enemies[i].base.width * 0.8f));
			}
			for (int i = 0; i < CHURU_MAX; i++) {
				if (churues[i].isActive)
					RenderObjectShadow(g_hGameDC, churues[i].x, churues[i].y - 10.0f, (int)(churues[i].width * 2.0f));
			}
		}
		if (boss.isActive) {
			RenderObjectShadow(g_hGameDC, boss.base.x, boss.base.y, (int)(boss.base.width * 1.1f));
		}
		
		for (int i = 0; i < OBSTACLE_LIMIT; i++) {
			if (obstacles[i].isActive) {
				if (obstacles[i].subType == SUB_TOWER_BROKEN) {
					RenderObjectShadow(g_hGameDC, obstacles[i].base.x + 2.0f, obstacles[i].base.y + 24.0f, obstacles[i].base.width);
				}
				else if (obstacles[i].subType == SUB_TOWER_TUNNEL) {
					RenderObjectShadow(g_hGameDC, obstacles[i].base.x, obstacles[i].base.y - 20.0f, (int)(obstacles[i].base.width * 2.0f));
				}
				else if (obstacles[i].subType == SUB_TOWER_TILTED) {
					RenderObjectShadow(g_hGameDC, obstacles[i].base.x - 3.0f, obstacles[i].base.y + 3.0f, (int)(obstacles[i].base.width * 1.4f));
				}
			}
		}

		RenderTask tasks[100];
		int taskCount = 0;

		// 플레이어 추가
		tasks[taskCount].y = player.base.y + (player.base.height / 1.5f);		// 무조건 발밑 기준
		tasks[taskCount].type = TYPE_PLAYER;
		taskCount++;

		// 잡몹 추가
		for (int i = 0; i < ENEMY_LIMIT; i++) {
			if (enemies[i].isActive) {
				tasks[taskCount].y = enemies[i].base.y + (enemies[i].base.height * 0.2f);
				tasks[taskCount].type = TYPE_ENEMY;
				tasks[taskCount].idx = i;
				taskCount++;
			}
		}

		// 보스 추가
		if (boss.isActive) {
			tasks[taskCount].y = boss.base.y + (boss.base.height / 2.5f);
			tasks[taskCount].type = TYPE_BOSS;
			taskCount++;
		}

		// 츄르 추가
		for (int i = 0; i < CHURU_MAX; i++) {
			if (churues[i].isActive) {
				tasks[taskCount].y = churues[i].y + (churues[i].height * 0.2f);
				tasks[taskCount].type = TYPE_CHURU;
				tasks[taskCount].idx = i;
				taskCount++;
			}
		}

		// 장애물 추가
		for (int i = 0; i < OBSTACLE_LIMIT; i++) {
			if (obstacles[i].isActive && obstacles[i].type == OBS_SOLID) {
				tasks[taskCount].y = obstacles[i].base.y + (obstacles[i].base.height * 0.3f);
				tasks[taskCount].type = TYPE_OBSTACLE;
				tasks[taskCount].idx = i;
				taskCount++;
			}
		}

		// 총알 추가
		for (int i = 0; i < BULLET_MAX; i++) {
			if (bullets[i].isActive) {
				tasks[taskCount].y = bullets[i].y + (bullets[i].height / 2.0f);
				tasks[taskCount].type = TYPE_PLAYER_BULLET;
				tasks[taskCount].idx = i;
				taskCount++;
			}
		}

		// 잡못 catpaw 추가
		for (int i = 0; i < CAT_PAW_LIMIT; i++) {
			if (catpaw[i].isActive) {
				tasks[taskCount].y = catpaw[i].y + (catpaw[i].height / 2.0f);
				tasks[taskCount].type = TYPE_CATPAW;
				tasks[taskCount].idx = i;
				taskCount++;
			}
		}

		qsort(tasks, taskCount, sizeof(RenderTask), CompareTasks);

		for (int i = 0; i < taskCount; i++) {
			switch (tasks[i].type) {
			case TYPE_PLAYER:
				RenderPlayer(g_hGameDC);								// 플레이어
				break;
			case TYPE_ENEMY:
				RenderSpecificEnemy(g_hGameDC, tasks[i].idx);			// 잡몹
				break;
			case TYPE_BOSS:
				RenderBoss(g_hGameDC);									// 보스
				break;
			case TYPE_CHURU:
				RenderSpecificChuru(g_hGameDC, tasks[i].idx);			// 츄르
				break;
			case TYPE_OBSTACLE:
				RenderSpecificObstacle(g_hGameDC, tasks[i].idx);		// 장애물
				break;
			case TYPE_PLAYER_BULLET:
				RenderSpecificBullets(g_hGameDC, tasks[i].idx);			// 총알
				break;
			case TYPE_CATPAW:
				RenderSpecificCatPaw(g_hGameDC, tasks[i].idx);			// 잡몹 젤리
				break;
			}
		}

		RenderBossPaws(g_hGameDC);								// 보스 젤리

		if (camera.isIntroActive == INACTIVE) {
			RenderPlayerHitBox(g_hGameDC);						// 플레이어 hitBox
			RenderEnemiesHitBox(g_hGameDC);						// 잡몹 hitBox
			RenderCatPawHitBox(g_hGameDC);						// 잡몹 젤리 hitBox
			RenderBossHitBox(g_hGameDC);						// 보스 hitBox
			RenderBossPawsHitBox(g_hGameDC);					// 보스 젤리 hitBox
			RenderBulletsHitBox(g_hGameDC);						// 총알 hitBox
			RenderObstaclesHitBox(g_hGameDC);					// 장애물 hitBox

			RenderUI(g_hGameDC);								// UI
		}
	}
	else if (g_UI.gameState == ENDING) {
		RenderAnimation(&g_UI.endingAnim, g_hGameDC, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	// Fade Out - Fade In (타이틀 -> 인게임)
	if (g_UI.isFadeOut || g_UI.isFadeIn || g_UI.fadeAlpha > 0.0f) {
		RenderFadeEffect(g_hGameDC);
	}

	// 게임 오버 버튼은 페이드 효과 위에
	if (g_UI.gameState == GAMEOVER) {
		RenderGameOver(g_hGameDC);
	}

	RenderCursor(g_hGameDC);

	// 화면 비율 계산 (16:9 기준 레터박스)
	float scaleX = (float)winW / SCREEN_WIDTH;
	float scaleY = (float)winH / SCREEN_HEIGHT;
	float scale = MIN(scaleX, scaleY);

	int destW = (int)(SCREEN_WIDTH * scale);
	int destH = (int)(SCREEN_HEIGHT * scale);
	int destX = (winW - destW) / 2;
	int destY = (winH - destH) / 2;

	// 가상 화면을 실제 화면 해상도에 맞춰 출력
	StretchBlt(hDC, destX, destY, destW, destH, g_hGameDC, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SRCCOPY);

	// [깜빡임 방지] 전체를 지우는 대신, 게임 화면이 그려지지 않는 '레터박스' 영역만 검은색으로 채움
	HBRUSH hBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);
	
	// 위쪽 바
	RECT rtTop = { 0, 0, winW, destY };
	FillRect(hDC, &rtTop, hBlack);
	// 아래쪽 바
	RECT rtBottom = { 0, destY + destH, winW, winH };
	FillRect(hDC, &rtBottom, hBlack);
	// 왼쪽 바
	RECT rtLeft = { 0, destY, destX, destY + destH };
	FillRect(hDC, &rtLeft, hBlack);
	// 오른쪽 바
	RECT rtRight = { destX + destW, destY, winW, destY + destH };
	FillRect(hDC, &rtRight, hBlack);
}