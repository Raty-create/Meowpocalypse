#pragma once
#include "object.h"
#include "image.h"
#include "animation.h"

typedef struct {
    OBJECT base;
    int isActive;
    int invincibleTimer;

    // 보스 공격
    int attackTimer;
    int pawTimer;

    //보스 스킬(대쉬)
    int dashTimer;
    int isDashing;
    float dashDirX;
    float dashDirY;
    
    // 보스 이동
    int moveTimer;
    float moveDirX;
    float moveDirY;

    // 보스 스킬(점프)
    int isJumping;
    int jumpTimer;
    float jumpDirX;
    float jumpDirY;
    int jumpPhase;                      // 0=올라가는 중, 1=내려오는 중
    float jumpOffsetY;                  // 렌더링용 Y 오프셋 (음수=위로 올라감

    int isEscaping;                     // 페이즈 전환 시 보스의 이동
    int escapingDelay;                  // 탈출 전 무적 대기 타이머
    
    // 보스 스킬 (회오리)
    int spiralTimer;                    // 회오리 PAW 발사 간격 타이머
    int spiralIndex;                    // 현재 회오리 발사 인덱스 (0 ~ 총 발사 수)
    int isSpiralActive;                 // 회오리 발사 중 여부

    // 보스 스킬 (랜덤 원형 3연속)
    int isRandomCircularActive;         // 랜덤 원형 3연속 발사 중 여부
    int randomCircularPhase;            // 현재 발사 단계 (0, 1, 2)
    int randomCircularDelay;            // 다음 단계까지 남은 딜레이 타이머

    // 보스 스킬 (연속 대쉬)
    int doubleDashPhase;                // 0=대시 안 함, 1=첫 번째 대시 완료 대기, 2=두 번째 대시 중
    int doubleDashDelay;                // 두 번째 대시 시작 전 딜레이 타이머

    // PAW 발사 중 이동 정지
    int isAttacking;                    // 1이면 PAW 발사 직후 정지 중
    int attackEndTimer;                 // 0이 되면 이동 재개

    int skillChargeTimer;               // 기 모으는 타이머
    int nextSkillState;                 // 기 모은 후 실행할 스킬 상태

    int deathTimer;

    ANIMATION anim;
    ANIMATION effectAnim;               // 탄막 스킬 이펙트 전용 애니메이션
} BOSS;

typedef struct {
    float x, y;
    float dx, dy;
    int width, height;
    float hitBoxX, hitBoxY;
    int hitBoxW, hitBoxH;
    int isActive;
    int dirRow;
    ANIMATION anim;
} BOSS_PAW;

typedef struct {
    float startX, startY;
    float dirX, dirY;
    float perpX, perpY;
    int stopDist;
    int isActive;
    int timer;
} DASH_WARNING;

typedef struct {
    float targetX, targetY;             // 착지 예정 월드 좌표
    int   isActive;
    int   timer;                        // 남은 경고 프레임
} JUMP_WARNING;

extern BOSS boss;
extern IMAGE imgBossSprite;
extern IMAGE imgBossSkillPattrenEffect;

extern BOSS_PAW bossPaws[BOSS_PAW_LIMIT];

extern DASH_WARNING dashWarn;
extern JUMP_WARNING jumpWarn;

void InitBoss();
void SpawnBoss(MAP_TYPE type);
void UpdateBoss();
void ReleaseBoss();


void UpdateBossMove();                                       // 보스 랜덤 이동 처리 (벽 충돌 시 방향 전환)
void UpdateBossChase();                                      // 보스(플레이어 추적 이동)
void UpdateBossPaws();                                       // BOSSPAW 이동 + 충돌처리

void SpawnBossPaws();                                        // 보스 스킬(3방향 젤리)

void SpawnCircularPaws();                                    // 보스 스킬(원형 탄막)

void StartDashWarning();                                     // 보스 스킬(대쉬 경고)
void UpdateDash(int is3rdPhase);                             // 대시 실행 처리
void UpdateDashWarningCountdown();                           // 대시 경고 카운트다운
void StartDoubleDashWarning();                               // 3페이즈 첫 번째 대시 경고 시작 (doubleDashPhase 설정)

void ClampBossLanding(float* x, float* y);                   // 보스 점프 스킬 위치 보정
void StartJumpWarning();                                     // 보스 스킬(점프 경고)
void UpdateJumpLanding(int is2nd3rdPhase);                   // 보스 점프 착지

void FireRandomCircularPhase(int phase);                     // 보스 랜덤탄막
void UpdateRandomCircularPaws(int is2nd3rdPhase);            // 보스 랜덤탄막 업데이트
void SpawnRandomCircularPaws();                              // 랜덤탄막 발사 시작


void UpdateSpiralPaws(int is2nd3rdPhase);                    // 회오리 PAW
void StartSpiralPaws();                                      // 회오리 PAW 발사 시작

void SelectPattern(int is2nd3rdPhase);                       // 패턴 선택

int CheckPhaseTransition();                                  // 페이즈 전환 및 보스 사망 처리