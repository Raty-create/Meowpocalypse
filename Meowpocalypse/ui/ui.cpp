#include "ui.h"

UI_SYSTEM g_UI;

void InitUI() {
	LoadMyImage(&g_UI.imgUISheet, L"ui.png");
	LoadMyImage(&g_UI.imgTitleBg, L"title_bg.png");
	LoadMyImage(&g_UI.imgMeowpocalypseTextLogo, L"meowpocalypse_logo.png");

	// 폰트 파일 로드
	AddFontResourceEx(L"upheavtt.ttf", FR_PRIVATE, NULL);
	AddFontResourceEx(L"neodgm_code.ttf", FR_PRIVATE, NULL);
	AddFontResourceEx(L"Galmuri11.ttf", FR_PRIVATE, NULL);

	// 폰트 생성
	g_UI.hTitleStartExitFont = CreateFont(32, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, L"Upheaval TT (BRK)");
	g_UI.hTitleHoverFont = CreateFont(42, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, L"Upheaval TT (BRK)");
	g_UI.hCooldownFont = CreateFont(48, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, L"Neo둥근모");
	g_UI.hItemCountFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, L"Galmuri11 Regular");

	TitleBg();
	TextLogo();
	GameStartBar();
	GameExitBar();

	HpMpBarFrame();
	HpBarUI();
	MpBarUI();
	SkillAndItemSq();
	SkillAndItemBanSq();
	SkillUI();
	HpPotionUI();
	MpPotionUI();
	LogoIconUI();

	PauseMenuBg();
	//PausePlayButton();

	g_UI.gameState = TITLE;
}

void ReleaseTitle() {
	ReleaseMyImage(&g_UI.imgTitleBg);
	ReleaseMyImage(&g_UI.imgMeowpocalypseTextLogo);

	if (g_UI.hTitleStartExitFont) {
		DeleteObject(g_UI.hTitleStartExitFont);
		g_UI.hTitleStartExitFont = NULL;
	}
	if (g_UI.hTitleHoverFont) {
		DeleteObject(g_UI.hTitleHoverFont);
		g_UI.hTitleHoverFont = NULL;
	}

	RemoveFontResourceEx(L"upheavtt.ttf", FR_PRIVATE, NULL);
}

void ReleaseUI() {
	ReleaseMyImage(&g_UI.imgUISheet);
	
	// 타이틀 리소스가 아직 남아있다면 해제 (ReleaseTitle에서 NULL 처리를 하므로 안전하기 함)
	ReleaseTitle();

	if (g_UI.hCooldownFont) {
		DeleteObject(g_UI.hCooldownFont);
		g_UI.hCooldownFont = NULL;
	}
	if (g_UI.hItemCountFont) {
		DeleteObject(g_UI.hItemCountFont);
		g_UI.hItemCountFont = NULL;
	}

	RemoveFontResourceEx(L"neodgm_code.ttf", FR_PRIVATE, NULL);
	RemoveFontResourceEx(L"Galmuri11.ttf", FR_PRIVATE, NULL);
}

void UpdateTitle(HWND hWnd) {
	// 텍스트 로고 둥둥 뜨는 효과
	static float logoTimer = 0.0f;
	logoTimer += 0.08f;

	float floatingOffset = sinf(logoTimer) * 10.0f;

	g_UI.title.titleLogo.y = (SCREEN_HEIGHT / 3.0f) + floatingOffset;

	// Start 버튼 스케일링
	const int MIN_W = 240;
	const int MAX_W = 300;
	const float RATIO = 76.0f / MIN_W;		// height 비율

	UI_ELEMENT* startBnt = &g_UI.title.startButton;
	BOOL isStartHover = (g_Input.mousePos.x >= startBnt->x - startBnt->width / 2 && g_Input.mousePos.x <= startBnt->x + startBnt->width / 2 &&
		g_Input.mousePos.y >= startBnt->y - startBnt->height / 2 && g_Input.mousePos.y <= startBnt->y + startBnt->height / 2);

	if (isStartHover) {
		if (startBnt->width < MAX_W) {
			startBnt->width = (int)(startBnt->width * 1.1f) + 1;
			if (startBnt->width > MAX_W) startBnt->width = MAX_W;
			startBnt->height = (int)(startBnt->width * RATIO);
		}
		if (g_Input.isLButtonDown) {					// Start 좌클릭 - Fade Out
			g_UI.isFadeOut = TRUE;
		}
	}
	else if (startBnt->width > MIN_W) {
		startBnt->width = (int)(startBnt->width / 1.1f);
		if (startBnt->width < MIN_W) startBnt->width = MIN_W;
		startBnt->height = (int)(startBnt->width * RATIO);
	}

	// Exit 버튼 스케일링
	UI_ELEMENT* exitBnt = &g_UI.title.exitButton;
	bool isExitHover = (g_Input.mousePos.x >= exitBnt->x - exitBnt->width / 2 && g_Input.mousePos.x <= exitBnt->x + exitBnt->width / 2 &&
		g_Input.mousePos.y >= exitBnt->y - exitBnt->height / 2 && g_Input.mousePos.y <= exitBnt->y + exitBnt->height / 2);

	if (isExitHover) {
		if (exitBnt->width < MAX_W) {
			exitBnt->width = (int)(exitBnt->width * 1.1f) + 1;
			if (exitBnt->width > MAX_W) exitBnt->width = MAX_W;
			exitBnt->height = (int)(exitBnt->width * RATIO);
		}
		if (g_Input.isLButtonDown) {
			PostQuitMessage(0);
		}
	}
	else if (exitBnt->width > MIN_W) {
		exitBnt->width = (int)(exitBnt->width / 1.1f);
		if (exitBnt->width < MIN_W) exitBnt->width = MIN_W;
		exitBnt->height = (int)(exitBnt->width * RATIO);
	}
}

// TITLE
//
//
//
//
// 타이틀 배경
void TitleBg() {
	// 타이틀 배경 원본 크기
	g_UI.title.titleBg.srcX = 0;
	g_UI.title.titleBg.srcY = 0;
	g_UI.title.titleBg.srcW = 1920;
	g_UI.title.titleBg.srcH = 1080;

	// 타이틀 배경 위치 및 크기 조절
	g_UI.title.titleBg.x = SCREEN_WIDTH / 2;
	g_UI.title.titleBg.y = SCREEN_HEIGHT / 2;
	g_UI.title.titleBg.width = 1920;
	g_UI.title.titleBg.height = 1080;
}

// 텍스트 로고
void TextLogo() {
	// 텍스트 로고 원본 크기
	g_UI.title.titleLogo.srcX = 0;
	g_UI.title.titleLogo.srcY = 0;
	g_UI.title.titleLogo.srcW = 2500;
	g_UI.title.titleLogo.srcH = 630;

	// 텍스트 로고 위치 및 크기 조절
	g_UI.title.titleLogo.x = SCREEN_WIDTH / 2;
	g_UI.title.titleLogo.y = SCREEN_HEIGHT / 2;
	g_UI.title.titleLogo.width = 1000;
	g_UI.title.titleLogo.height = 252;
}

// 게임 시작 버튼
void GameStartBar() {
	// 게임 시작 버튼 원본 크기
	g_UI.title.startButton.srcX = 15;
	g_UI.title.startButton.srcY = 203;
	g_UI.title.startButton.srcW = 240;
	g_UI.title.startButton.srcH = 76;

	// 게임 시작 버튼 위치 및 크기 조절
	g_UI.title.startButton.x = SCREEN_WIDTH / 2;
	g_UI.title.startButton.y = SCREEN_HEIGHT * TITLE_START_HEIGHT_MARGIN;
	g_UI.title.startButton.width = 240;
	g_UI.title.startButton.height = 76;
}

// 게임 종료 버튼
void GameExitBar() {
	// 게임 종료 버튼 원본 크기
	g_UI.title.exitButton.srcX = 15;
	g_UI.title.exitButton.srcY = 299;
	g_UI.title.exitButton.srcW = 240;
	g_UI.title.exitButton.srcH = 76;

	// 게임 종료 버튼 위치 및 크기 조절
	g_UI.title.exitButton.x = SCREEN_WIDTH / 2;
	g_UI.title.exitButton.y = SCREEN_HEIGHT * TITLE_EXIT_HEIGHT_MARGIN;
	g_UI.title.exitButton.width = 240;
	g_UI.title.exitButton.height = 76;
}





// INGAME
//
//
// 
// 
// HP - MP 바 배경 프레임
void HpMpBarFrame() {
	// HP - MP 바 배경 프레임 원본 크기
	g_UI.hud.hpBarFrame.srcX = g_UI.hud.mpBarFrame.srcX = 16;
	g_UI.hud.hpBarFrame.srcY = g_UI.hud.mpBarFrame.srcY = 19;
	g_UI.hud.hpBarFrame.srcW = g_UI.hud.mpBarFrame.srcW = 128;
	g_UI.hud.hpBarFrame.srcH = g_UI.hud.mpBarFrame.srcH = 26;

	// HP 바 배경 프레임 위치 및 크기 조절
	g_UI.hud.hpBarFrame.x = SCREEN_WIDTH / 2 - HPBAR_FRAME_WIDTH_MARGIN;
	g_UI.hud.hpBarFrame.y = SCREEN_HEIGHT - HPBAR_FRAME_HEIGHT_MARGIN;
	g_UI.hud.hpBarFrame.width = 256;
	g_UI.hud.hpBarFrame.height = 52;

	// MP 바 배경 프레임 위치 및 크기 조절
	g_UI.hud.mpBarFrame.x = SCREEN_WIDTH / 2 + MPBAR_FRAME_WIDTH_MARGIN;
	g_UI.hud.mpBarFrame.y = SCREEN_HEIGHT - MPBAR_FRAME_HEIGHT_MARGIN;;
	g_UI.hud.mpBarFrame.width = 256;
	g_UI.hud.mpBarFrame.height = 52;
}

// HP 바
void HpBarUI() {
	// HP 바 원본 크기
	g_UI.hud.hpBar.srcX = 24;
	g_UI.hud.hpBar.srcY = 91;
	g_UI.hud.hpBar.srcW = 112;
	g_UI.hud.hpBar.srcH = 10;

	// HP 바 위치 및 크기 조절
	g_UI.hud.hpBar.x = SCREEN_WIDTH / 2 - HPBAR_WIDTH_MARGIN;
	g_UI.hud.hpBar.y = SCREEN_HEIGHT - HPBAR_HEIGHT_MARGIN;
	g_UI.hud.hpBar.width = 224;
	g_UI.hud.hpBar.height = 20;
}

// MP 바
void MpBarUI() {
	// MP 바 원본 크기
	g_UI.hud.mpBar.srcX = 24;
	g_UI.hud.mpBar.srcY = 155;
	g_UI.hud.mpBar.srcW = 112;
	g_UI.hud.mpBar.srcH = 10;

	// MP 바 위치 및 크기 조절
	g_UI.hud.mpBar.x = SCREEN_WIDTH / 2 + MPBAR_WIDTH_MARGIN;
	g_UI.hud.mpBar.y = SCREEN_HEIGHT - MPBAR_HEIGHT_MARGIN;
	g_UI.hud.mpBar.width = 224;
	g_UI.hud.mpBar.height = 20;
}

// 스킬 - 아이템 아이콘 배경
void SkillAndItemSq() {
	// 스킬 - 아이템 아이콘 배경 원본 크기
	for (int i = 0; i < 5; i++) {
		g_UI.hud.skill_item_sq[i].srcX = 160;
		g_UI.hud.skill_item_sq[i].srcY = 128;
		g_UI.hud.skill_item_sq[i].srcW = 61;
		g_UI.hud.skill_item_sq[i].srcH = 64;
	}

	// Q 스킬 아이콘 배경 위치 및 크기 조절
	g_UI.hud.skill_item_sq[0].x = SCREEN_WIDTH / 2 - Q_ICON_WIDTH_MARGIN;
	g_UI.hud.skill_item_sq[0].y = SCREEN_HEIGHT - Q_ICON_HEIGHT_MARGIN;
	g_UI.hud.skill_item_sq[0].width = 61;
	g_UI.hud.skill_item_sq[0].height = 64;

	// E 스킬 아이콘 배경 위치 및 크기 조절
	g_UI.hud.skill_item_sq[1].x = SCREEN_WIDTH / 2 - E_ICON_WIDTH_MARGIN;
	g_UI.hud.skill_item_sq[1].y = SCREEN_HEIGHT - E_ICON_HEIGHT_MARGIN;
	g_UI.hud.skill_item_sq[1].width = 61;
	g_UI.hud.skill_item_sq[1].height = 64;

	// R 스킬 아이콘 배경 위치 및 크기 조절
	g_UI.hud.skill_item_sq[2].x = SCREEN_WIDTH / 2 - R_ICON_WIDTH_MARGIN;
	g_UI.hud.skill_item_sq[2].y = SCREEN_HEIGHT - R_ICON_HEIGHT_MARGIN;
	g_UI.hud.skill_item_sq[2].width = 61;
	g_UI.hud.skill_item_sq[2].height = 64;

	// HP 포션 아이콘 배경 위치 및 크기 조절
	g_UI.hud.skill_item_sq[3].x = SCREEN_WIDTH / 2 + HP_POTION_WIDTH_MARGIN;
	g_UI.hud.skill_item_sq[3].y = SCREEN_HEIGHT - HP_POTION_HEIGHT_MARGIN;
	g_UI.hud.skill_item_sq[3].width = 61;
	g_UI.hud.skill_item_sq[3].height = 64;

	// MP 포션 아이콘 배경 위치 및 크기 조절
	g_UI.hud.skill_item_sq[4].x = SCREEN_WIDTH / 2 + MP_POTION_WIDTH_MARGIN;
	g_UI.hud.skill_item_sq[4].y = SCREEN_HEIGHT - MP_POTION_HEIGHT_MARGIN;
	g_UI.hud.skill_item_sq[4].width = 61;
	g_UI.hud.skill_item_sq[4].height = 64;
}

// 스킬 - 아이템 아이콘 금지 배경
void SkillAndItemBanSq() {
	// 스킬 - 아이템 아이콘 금지 배경 원본 크기
	for (int i = 0; i < 5; i++) {
		g_UI.hud.skill_item_ban_sq[i].srcX = 224;
		g_UI.hud.skill_item_ban_sq[i].srcY = 128;
		g_UI.hud.skill_item_ban_sq[i].srcW = 61;
		g_UI.hud.skill_item_ban_sq[i].srcH = 64;
	}

	// Q 스킬 아이콘 금지 배경 위치 및 크기 조절
	g_UI.hud.skill_item_ban_sq[0].x = SCREEN_WIDTH / 2 - Q_ICON_WIDTH_MARGIN;
	g_UI.hud.skill_item_ban_sq[0].y = SCREEN_HEIGHT - Q_ICON_HEIGHT_MARGIN;
	g_UI.hud.skill_item_ban_sq[0].width = 61;
	g_UI.hud.skill_item_ban_sq[0].height = 64;

	// E 스킬 아이콘 금지 배경 위치 및 크기 조절
	g_UI.hud.skill_item_ban_sq[1].x = SCREEN_WIDTH / 2 - E_ICON_WIDTH_MARGIN;
	g_UI.hud.skill_item_ban_sq[1].y = SCREEN_HEIGHT - E_ICON_HEIGHT_MARGIN;
	g_UI.hud.skill_item_ban_sq[1].width = 61;
	g_UI.hud.skill_item_ban_sq[1].height = 64;

	// R 스킬 아이콘 금지 배경 위치 및 크기 조절
	g_UI.hud.skill_item_ban_sq[2].x = SCREEN_WIDTH / 2 - R_ICON_WIDTH_MARGIN;
	g_UI.hud.skill_item_ban_sq[2].y = SCREEN_HEIGHT - R_ICON_HEIGHT_MARGIN;
	g_UI.hud.skill_item_ban_sq[2].width = 61;
	g_UI.hud.skill_item_ban_sq[2].height = 64;

	// HP 포션 아이콘 금지 배경 위치 및 크기 조절
	g_UI.hud.skill_item_ban_sq[3].x = SCREEN_WIDTH / 2 + HP_POTION_WIDTH_MARGIN;
	g_UI.hud.skill_item_ban_sq[3].y = SCREEN_HEIGHT - HP_POTION_HEIGHT_MARGIN;
	g_UI.hud.skill_item_ban_sq[3].width = 61;
	g_UI.hud.skill_item_ban_sq[3].height = 64;

	// MP 포션 아이콘 금지 배경 위치 및 크기 조절
	g_UI.hud.skill_item_ban_sq[4].x = SCREEN_WIDTH / 2 + MP_POTION_WIDTH_MARGIN;
	g_UI.hud.skill_item_ban_sq[4].y = SCREEN_HEIGHT - MP_POTION_HEIGHT_MARGIN;
	g_UI.hud.skill_item_ban_sq[4].width = 61;
	g_UI.hud.skill_item_ban_sq[4].height = 64;
}

// 스킬(Q, E, R)
void SkillUI() {
	// Q 스킬 아이콘 원본 크기
	g_UI.hud.skill_Icon[0].srcX = 160;
	g_UI.hud.skill_Icon[0].srcY = 0;
	g_UI.hud.skill_Icon[0].srcW = 64;
	g_UI.hud.skill_Icon[0].srcH = 64;

	// Q 스킬 아이콘 위치 및 크기 조절
	g_UI.hud.skill_Icon[0].x = SCREEN_WIDTH / 2 - Q_ICON_WIDTH_MARGIN;
	g_UI.hud.skill_Icon[0].y = SCREEN_HEIGHT - Q_ICON_HEIGHT_MARGIN;
	g_UI.hud.skill_Icon[0].width = 64;
	g_UI.hud.skill_Icon[0].height = 64;



	// E 스킬 아이콘 원본 크기
	g_UI.hud.skill_Icon[1].srcX = 224;
	g_UI.hud.skill_Icon[1].srcY = 0;
	g_UI.hud.skill_Icon[1].srcW = 64;
	g_UI.hud.skill_Icon[1].srcH = 64;

	// E 스킬 아이콘 위치 및 크기 조절
	g_UI.hud.skill_Icon[1].x = SCREEN_WIDTH / 2 - E_ICON_WIDTH_MARGIN;
	g_UI.hud.skill_Icon[1].y = SCREEN_HEIGHT - E_ICON_HEIGHT_MARGIN;
	g_UI.hud.skill_Icon[1].width = 64;
	g_UI.hud.skill_Icon[1].height = 64;



	// R 스킬 아이콘 원본 크기
	g_UI.hud.skill_Icon[2].srcX = 288;
	g_UI.hud.skill_Icon[2].srcY = 0;
	g_UI.hud.skill_Icon[2].srcW = 64;
	g_UI.hud.skill_Icon[2].srcH = 64;

	// R 스킬 아이콘 위치 및 크기 조절
	g_UI.hud.skill_Icon[2].x = SCREEN_WIDTH / 2 - R_ICON_WIDTH_MARGIN;
	g_UI.hud.skill_Icon[2].y = SCREEN_HEIGHT - R_ICON_HEIGHT_MARGIN;
	g_UI.hud.skill_Icon[2].width = 64;
	g_UI.hud.skill_Icon[2].height = 64;
}

// HP 포션 아이콘
void HpPotionUI() {
	// HP 포션 아이콘 원본 크기
	g_UI.hud.hpPotion.srcX = 160;
	g_UI.hud.hpPotion.srcY = 64;
	g_UI.hud.hpPotion.srcW = 64;
	g_UI.hud.hpPotion.srcH = 64;

	// HP 포션 아이콘 위치 및 크기 조절
	g_UI.hud.hpPotion.x = SCREEN_WIDTH / 2 + HP_POTION_WIDTH_MARGIN;
	g_UI.hud.hpPotion.y = SCREEN_HEIGHT - HP_POTION_HEIGHT_MARGIN;
	g_UI.hud.hpPotion.width = 64;
	g_UI.hud.hpPotion.height = 64;
}

// MP 포션 아이콘
void MpPotionUI() {
	// MP 포션 아이콘 원본 크기
	g_UI.hud.mpPotion.srcX = 224;
	g_UI.hud.mpPotion.srcY = 64;
	g_UI.hud.mpPotion.srcW = 64;
	g_UI.hud.mpPotion.srcH = 64;

	// MP 포션 아이콘 위치 및 크기 조절
	g_UI.hud.mpPotion.x = SCREEN_WIDTH / 2 + MP_POTION_WIDTH_MARGIN;
	g_UI.hud.mpPotion.y = SCREEN_HEIGHT - MP_POTION_HEIGHT_MARGIN;
	g_UI.hud.mpPotion.width = 64;
	g_UI.hud.mpPotion.height = 64;
}

// GUI 로고 아이콘
void LogoIconUI() {
	// GUI 로고 아이콘 원본 크기
	g_UI.hud.logo_Icon.srcX = 288;
	g_UI.hud.logo_Icon.srcY = 64;
	g_UI.hud.logo_Icon.srcW = 64;
	g_UI.hud.logo_Icon.srcH = 64;

	// GUI 로고 아이콘 위치 및 크기 조절
	g_UI.hud.logo_Icon.x = SCREEN_WIDTH / 2;
	g_UI.hud.logo_Icon.y = SCREEN_HEIGHT - GUI_LOGO_HEIGHT_MARGIN;
	g_UI.hud.logo_Icon.width = 96;
	g_UI.hud.logo_Icon.height = 96;
}

// PAUSE
//
//
// 
// 
// 
// 퍼즈 메뉴 배경
void PauseMenuBg() {
	// 퍼즈 메뉴 배경 원본 크기
	g_UI.pause.menuBg.srcX = 0;
	g_UI.pause.menuBg.srcY = 385;
	g_UI.pause.menuBg.srcW = 96;
	g_UI.pause.menuBg.srcH = 150;

	// 퍼즈 메뉴 배경 위치 및 크기 조절
	g_UI.pause.menuBg.x = SCREEN_WIDTH / 2;
	g_UI.pause.menuBg.y = SCREEN_HEIGHT / 2;
	g_UI.pause.menuBg.width = g_UI.pause.menuBg.srcW * 4;
	g_UI.pause.menuBg.height = g_UI.pause.menuBg.srcH * 4;
}

void PausePlayButton() {
	// 게임 계속하기 버튼 원본 크기
	g_UI.pause.playButton.srcX = 0;
	g_UI.pause.playButton.srcY = 389;
	g_UI.pause.playButton.srcW = 120;
	g_UI.pause.playButton.srcH = 187;

	// 게임 계속하기 버튼 위치 및 크기 조절
	g_UI.pause.playButton.x = SCREEN_WIDTH / 2;
	g_UI.pause.playButton.y = SCREEN_HEIGHT / 2;
	g_UI.pause.playButton.width = 600;
	g_UI.pause.playButton.height = 935;
}