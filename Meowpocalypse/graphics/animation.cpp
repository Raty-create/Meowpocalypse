#include "animation.h"

void InitAnimation(ANIMATION* anim, IMAGE* img, int fw, int fh, int total, int delay, BOOL loop) {
     anim->pImg = img;
     anim->frameWidth = fw;
     anim->frameHeight = fh;
     anim->totalFrames = total;
     anim->currFrame = 0;
     anim->frameTimer = 0;
     anim->frameDelay = delay;
     anim->isLoop = loop;
     anim->isEnd = FALSE;
}

void UpdateAnimation(ANIMATION* anim) {
    if (anim->isEnd) return;

    anim->frameTimer++;
    if (anim->frameTimer >= anim->frameDelay) {
        anim->frameTimer = 0;
        anim->currFrame++;
        
        if (anim->currFrame >= anim->totalFrames) {
            if (anim->isLoop) {
                anim->currFrame = 0;
            }
            else {
                anim->currFrame = anim->totalFrames - 1;
                anim->isEnd = TRUE;
            }
        }
    }
}

void RenderAnimation(ANIMATION* anim, HDC hDC, int x, int y, int dw, int dh, int row) {
    // 현재 프레임에 해당하는 이미지 소스 좌표 계산
    int sx = anim->currFrame * anim->frameWidth;
    int sy = row * anim->frameHeight;

    // dw, dh 크기로 스케일링하여 그림 (중심점 기준)
    DrawMyImage(anim->pImg, hDC, x - dw / 2, y - dh / 2, dw, dh, sx, sy, anim->frameWidth, anim->frameHeight);
}

void SetAnimationFrame(ANIMATION* anim, int frame) {
    if (frame >= 0 && frame < anim->totalFrames) {
        anim->currFrame = frame;
        anim->frameTimer = 0;
        anim->isEnd = FALSE;
    }
}