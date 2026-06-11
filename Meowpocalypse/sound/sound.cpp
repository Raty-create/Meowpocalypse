#include "sound.h"
#include "config.h"
#include "map.h"

#include <mmsystem.h>
#include <stdio.h>
#include <wchar.h>

// BGM 파일 경로 테이블
static const wchar_t* g_bgmPath[BGM_COUNT] = {
    L"Title.mp3",                         // BGM_TITLE
    L"Waiting Map.mp3",                   // BGM_WAITING
    L"HallWayMap.mp3",                    // BGM_HALLWAY
    L"First Boss Map.mp3",                // BGM_BOSS1
    L"Second Boss Map.mp3",               // BGM_BOSS2
    L"Third Boss Map.mp3",                // BGM_BOSS3
};

// SFX 파일 경로 테이블
static const wchar_t* g_sfxPath[SFX_COUNT] = {
    L"Shoot.mp3",                           // SFX_PLAYER_SHOOT
    L"FootStep.mp3",                        // SFX_PLAYER_FOOTSTEP
    L"Player_Hit.mp3",                      // SFX_PLAYER_HIT
    L"skill_drink.mp3",                     // SFX_PLAYER_SKILL HP, MP
    L"skill_q.mp3",                         // SFX_PLAYER_SKILL Q
    L"skill_e.mp3",                         // SFX_PLAYER_SKILL E
    L"skill_r.mp3",                         // SFX_PLAYER_SKILL R
    L"player_dead.mp3",                     // SFX_PLAYER_DEAD

    L"boss_footstep.mp3",                   // 보스 발소리
    L"boss_three_way_catpaw.mp3",           // 보스 탄막(젤리) 발사
    L"boss_circular_catpaw.mp3",            // 보스 탄막(원형젤리) 발사
    L"boss_random_catpaw.mp3",              // 보스 탄막(원형 * 10 젤리) 발사
    L"boss_spiral_catpaw.mp3",              // 보스 탄막(회오리 젤리) 발사
    L"Boss_Dash.mp3",                       // SFX_BOSS_DASH
    L"Boss_Jump.mp3",                       // SFX_BOSS_JUMP
    L"Boss_Jump_Land.mp3",                  // SFX_BOSS_JUMP_Land

    L"Enemy_Hit.mp3",                       // SFX_ENEMY_HIT
};

static wchar_t g_bgmAlias[32] = L"";    // 현재 열려있는 BGM 의 alias ("" 면 없음)
static int g_bgmActive = 0;      // BGM 재생 중 여부
static int g_bgmLoop = 0;      // 루프 여부
static int g_bgmGrace = 0;      // 재생 직후 루프 오작동 방지 카운트
static int g_bgmPaused = 0;

static int g_bgmTargetVolume = 400;
static int g_sfxVolume = 200;

static int g_sfxLoaded[SFX_COUNT][SFX_VOICES];  // 채널별 open 성공 여부
static int g_sfxNextVoice[SFX_COUNT];           // 다음에 사용할 채널 (라운드 로빈)

// MCI 명령 전송 헬퍼. 성공하면 1, 실패하면 0.
static int mci(const wchar_t* cmd, wchar_t* ret = NULL, int retSize = 0) {
    return (mciSendStringW(cmd, ret, retSize, NULL) == 0);
}

// SFX 채널 alias 생성 ("sfx_<종류>_<채널>")
static void MakeSfxAlias(wchar_t* out, int type, int voice) {
    swprintf(out, 32, L"sfx_%d_%d", type, voice);
}

// 파일 열기: MP3 우선(type mpegvideo), 실패하면 확장자 자동 인식으로 재시도.
static int OpenMedia(const wchar_t* path, const wchar_t* alias) {
    wchar_t cmd[512];
    swprintf(cmd, 512, L"open \"%s\" type mpegvideo alias %s", path, alias);
    if (mci(cmd)) return 1;

    swprintf(cmd, 512, L"open \"%s\" alias %s", path, alias);
    return mci(cmd);
}

void InitSound() {
    for (int t = 0; t < SFX_COUNT; t++) {
        g_sfxNextVoice[t] = 0;
        for (int v = 0; v < SFX_VOICES; v++) {
            wchar_t alias[32];
            MakeSfxAlias(alias, t, v);

            // 파일을 미리 열어둔다. 파일이 없으면 그냥 재생만 안됨.
            g_sfxLoaded[t][v] = OpenMedia(g_sfxPath[t], alias);

            if (g_sfxLoaded[t][v]) {
                wchar_t cmd[64];
                swprintf(cmd, 64, L"setaudio %s volume to %d", alias, g_sfxVolume);
                mci(cmd);
            }
        }
    }
}

void ReleaseSound() {
    if (g_bgmAlias[0]) {
        wchar_t cmd[64];
        swprintf(cmd, 64, L"close %s", g_bgmAlias);
        mci(cmd);
        g_bgmAlias[0] = 0;
    }
    g_bgmActive = 0;

    for (int t = 0; t < SFX_COUNT; t++)
        for (int v = 0; v < SFX_VOICES; v++)
            if (g_sfxLoaded[t][v]) {
                wchar_t alias[32], cmd[64];
                MakeSfxAlias(alias, t, v);
                swprintf(cmd, 64, L"close %s", alias);
                mci(cmd);
                g_sfxLoaded[t][v] = 0;
            }

    mciSendStringW(L"close all", NULL, 0, NULL);  // 혹시 남은 장치 정리
}

void PlayBGM(BGM_TYPE type, BOOL loop, int volume) {
    if (type < 0 || type >= BGM_COUNT) return;

    g_bgmTargetVolume = volume;

    // 같은 곡을 또 틀라고 하면 무시 (끊기지 않게)
    wchar_t wantAlias[32];
    swprintf(wantAlias, 32, L"bgm_%d", type);
    if (g_bgmActive && wcscmp(g_bgmAlias, wantAlias) == 0) return;

    // 기존 BGM 정리
    if (g_bgmAlias[0]) {
        wchar_t cmd[64];
        swprintf(cmd, 64, L"close %s", g_bgmAlias);
        mci(cmd);
        g_bgmAlias[0] = 0;
    }
    g_bgmActive = 0;

    // 새 BGM 열기
    wcscpy_s(g_bgmAlias, 32, wantAlias);
    if (!OpenMedia(g_bgmPath[type], g_bgmAlias)) {
        g_bgmAlias[0] = 0;  // 파일이 없으면 조용히 종료
        return;
    }

    wchar_t cmd[64];
    swprintf(cmd, 64, L"setaudio %s volume to %d", g_bgmAlias, volume);
    mci(cmd);

    swprintf(cmd, 64, L"play %s from 0", g_bgmAlias);
    mci(cmd);

    g_bgmActive = 1;
    g_bgmLoop = loop ? 1 : 0;
    g_bgmGrace = 30;   // 재생 직후 잠깐은 루프 체크를 건너뜀
}

int GetBGMTargetVolume() {
    return g_bgmTargetVolume;
}

void StopBGM() {
    g_bgmActive = 0;
    g_bgmLoop = 0;
    if (g_bgmAlias[0]) {
        wchar_t cmd[64];
        swprintf(cmd, 64, L"stop %s", g_bgmAlias);
        mci(cmd);
    }
}

void PauseBGM() {
    if (!g_bgmActive || g_bgmAlias[0] == 0 || g_bgmPaused) return;
    wchar_t cmd[64];
    swprintf(cmd, 64, L"pause %s", g_bgmAlias);
    mci(cmd);
    g_bgmPaused = 1;
}

void ResumeBGM() {
    if (!g_bgmActive || g_bgmAlias[0] == 0 || !g_bgmPaused) return;
    wchar_t cmd[64];
    swprintf(cmd, 64, L"resume %s", g_bgmAlias);
    mci(cmd);
    g_bgmPaused = 0;
    g_bgmGrace = 30;   // resume 직후 루프 오작동 방지
}

// 매 프레임 호출. MP3 는 MCI 자체 루프가 안정적이지 않아서,
// 곡이 끝났는지(mode == stopped) 확인 후 처음부터 다시 재생한다.
void UpdateSound() {
    if (!g_bgmActive || !g_bgmLoop || g_bgmAlias[0] == 0) return;
    if (g_bgmPaused) return;

    if (g_bgmGrace > 0) { g_bgmGrace--; return; }

    wchar_t cmd[64], ret[32] = L"";
    swprintf(cmd, 64, L"status %s mode", g_bgmAlias);
    if (mci(cmd, ret, 32)) {
        if (wcscmp(ret, L"stopped") == 0) {
            swprintf(cmd, 64, L"play %s from 0", g_bgmAlias);
            mci(cmd);
            g_bgmGrace = 30;
        }
    }
}

// 효과음 재생. 채널을 라운드 로빈으로 돌려가며 재생해서
// 같은 효과음도 겹쳐 들리고, 다른 효과음/BGM 과도 동시에 남.
void PlaySFX(SFX_TYPE type) {
    if (type < 0 || type >= SFX_COUNT) return;

    int v = g_sfxNextVoice[type];
    g_sfxNextVoice[type] = (v + 1) % SFX_VOICES;

    if (!g_sfxLoaded[type][v]) return;

    wchar_t alias[32], cmd[64];
    MakeSfxAlias(alias, type, v);
    swprintf(cmd, 64, L"play %s from 0", alias);  // 처음부터 다시 재생
    mci(cmd);
}

void SetBGMVolume(int volume) {
    if (volume < 0)    volume = 0;
    if (volume > 1000) volume = 1000;

    if (g_bgmAlias[0]) {
        wchar_t cmd[64];
        swprintf(cmd, 64, L"setaudio %s volume to %d", g_bgmAlias, volume);
        mci(cmd);
    }
}

//void SetSFXVolume(int volume) {
//    if (volume < 0)    volume = 0;
//    if (volume > 1000) volume = 1000;
//    g_sfxVolume = volume;
//
//    for (int t = 0; t < SFX_COUNT; t++)
//        for (int v = 0; v < SFX_VOICES; v++)
//            if (g_sfxLoaded[t][v]) {
//                wchar_t alias[32], cmd[64];
//                MakeSfxAlias(alias, t, v);
//                swprintf(cmd, 64, L"setaudio %s volume to %d", alias, volume);
//                mci(cmd);
//            }
//}