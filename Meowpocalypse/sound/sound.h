#pragma once

#include <windows.h>
#include "enum.h"

#pragma comment(lib, "winmm.lib")   // MCI(mciSendString) 사용을 위한 링크

void InitSound();                       // 게임 시작 시 1회 (InitGame 안에서)
void ReleaseSound();                    // 게임 종료 시 1회 (ReleaseGame 안에서)
void UpdateSound();                     // 매 프레임 호출 (Update 안에서) - BGM 루프 처리

void PlayBGM(BGM_TYPE type, BOOL loop, int volume); // 스테이지 배경음악 재생 (기존 BGM 은 자동 정지)
int GetBGMTargetVolume();
void StopBGM();                         // 배경음악 정지
void PauseBGM();    // 현재 위치에서 일시정지
void ResumeBGM();   // 멈춘 위치부터 이어서 재생

void PlaySFX(SFX_TYPE type);            // 효과음 재생 (겹쳐서 재생됨)

void SetBGMVolume(int volume);          // 0 ~ 1000
void SetSFXVolume(int volume);          // 0 ~ 1000
