#pragma once

#include <Windows.h>
#include <math.h>

#include "config.h"
#include "enum.h"
#include "input.h"
#include "image.h"
#include "animation.h"
#include "player.h"
#include "boss.h"
#include "game.h"

typedef struct {
	float x, y;
	int width, height;
	int srcX, srcY;
	int srcW, srcH;
	int isActive;
	BOOL isHovered;
} UI_ELEMENT;

typedef struct {
	UI_ELEMENT titleBg;
	UI_ELEMENT titleLogo;
	UI_ELEMENT startButton;
	UI_ELEMENT exitButton;
} TITLE_DATA;

typedef struct {
	UI_ELEMENT menuBg;
	UI_ELEMENT menuButton[4];		// Play, Key Guide, Title, Exit
} PAUSE_DATA;

typedef struct {
	UI_ELEMENT keyGuideUI;
} KEY_GUIDE_DATA;

typedef struct {
	UI_ELEMENT reStartButton;
	UI_ELEMENT exitButton;
} GAMEOVER_DATA;

typedef struct {
	UI_ELEMENT hpBarFrame;
	UI_ELEMENT hpBar;
	float playerVisualHp;
	UI_ELEMENT mpBarFrame;
	UI_ELEMENT mpBar;
	float playerVisualMp;

	UI_ELEMENT skill_item_sq[5];
	UI_ELEMENT skill_item_ban_sq[5];
	UI_ELEMENT skill_Icon[3];		// Q, E, R 아이콘

	UI_ELEMENT hpPotion;
	UI_ELEMENT mpPotion;

	UI_ELEMENT bossHpBarFrame;
	UI_ELEMENT bossHpBar;
	float bossVisualHp;
	BOOL showBossHp;
	UI_ELEMENT bossEmblem;

	UI_ELEMENT logo_Icon;
} HUD_DATA;

typedef struct {
	GAME_STATE gameState;		// 0: Title, 1: InGame, 2: Pause, 3: GameOver

	IMAGE imgTitleBg;
	IMAGE imgMeowpocalypseTextLogo;
	TITLE_DATA title;

	IMAGE imgUISheet;
	HUD_DATA hud;

	PAUSE_DATA pause;

	IMAGE imgKeyGuide;
	KEY_GUIDE_DATA keyGuide;

	GAMEOVER_DATA gameover;

	HFONT hTitleStartExitFont;	// 타이틀 Start 및 Exit 폰트
	HFONT hTitleHoverFont;		// 타이틀 마우스 호버용 큰 폰트
	HFONT hCooldownFont;		// 스킬 쿨타임용 폰트
	HFONT hItemCountFont;		// 아이템 개수용 폰트
	HFONT hGameOverFont;		// 게임 오버 대형 폰트

	float fadeAlpha;			// 0.0f(투명) ~ 1.0f (검정)
	BOOL isFadeOut;				// 타이틀 -> 검정 (Fade Out)
	BOOL isFadeIn;				// 검정 -> 인게임 (Fade In)

	BOOL isMapFadeOut;			// 맵 전환 시작 (Fade Out)
	BOOL isMapFadeIn;			// 맵 전환 완료 (Fade In)

	BOOL isPlayerDeadFadeOut;	// 플레이어 사망 시 전환 시작 (Fade Out)

	ANIMATION endingAnim;
	BOOL isEndingFadeOut;
	BOOL isEndingFadeIn;

	BOOL isEndingToTitleFadeOut;
	BOOL isEndingToTitleFadeIn;
} UI_SYSTEM;

extern UI_SYSTEM g_UI;
extern IMAGE imgEnding;

void InitUI();
void InitEnding();

void UpdateTitle();
void UpdatePause();
void UpdateKeyGuide();
void UpdateGameOver();
void UpdateHpBar();						// 플레이어 HP 바 부드러운 감소 업데이트
void UpdateMpBar();						// 플레이어 MP 바 부드러운 감소 업데이트
void UpdateBossHpBar();					// 보스 HP 바 부드러운 감소 업데이트

void ReleaseTitle();
void ReleaseUI();

// TITLE
void TitleBg();
void TextLogo();
void GameStartBar();
void GameExitBar();

// INGAME
void HpMpBarFrame();
void HpBarUI();
void MpBarUI();
void SkillAndItemSq();
void SkillAndItemBanSq();
void SkillUI();
void HpPotionUI();
void MpPotionUI();
void LogoIconUI();						// GUI 로고

void BossHpBarFrame();
void BossHpBarUI();
void BossEmblem();

// PAUSE
void PauseMenuBg();
void PauseMenuButton();

// KEY_GUIDE
void KeyGuideUI();

// GAMEOVER
void GameOverRestartBar();
void GameOverExitBar();