#pragma once

#include "image.h"

 typedef struct {
     IMAGE* pImg;        // 연결된 스프라이트 시트 이미지
     int currFrame;      // 현재 재생 중인 프레임 번호
     int totalFrames;    // 전체 프레임 수
     int frameWidth;     // 프레임 하나의 가로 크기
     int frameHeight;    // 프레임 하나의 세로 크기
     int frameTimer;     // 시간 누적용 카운터
     int frameDelay;     // 프레임 전환 속도 (값이 클수록 느림)
     BOOL isLoop;        // 반복 재생 여부
     BOOL isEnd;         // 애니메이션 종료 여부 (반복이 아닐 때)
} ANIMATION;

void InitAnimation(ANIMATION* anim, IMAGE* img, int fw, int fh, int total, int delay, BOOL loop = TRUE);
void UpdateAnimation(ANIMATION* anim);
void RenderAnimation(ANIMATION* anim, HDC hDC, int x, int y, int dw, int dh, int row = 0);          // 중심점 기준
void SetAnimationFrame(ANIMATION* anim, int frame);