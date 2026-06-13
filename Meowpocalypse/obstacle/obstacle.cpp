#include "obstacle.h"

OBSTACLE obstacles[OBSTACLE_LIMIT];
IMAGE imgObstacles;

void InitObstacles() {
	if (imgObstacles.img.IsNull()) {
		LoadMyImage(&imgObstacles, L"obstacles.png");
	}

	for (int i = 0; i < OBSTACLE_LIMIT; i++) {
		obstacles[i].isActive = INACTIVE;
	}

	// 복도맵 장애물 배치
	if (currentMapType == MAP_HALLWAY) {
		float hallwayH = (float)HALLWAYMAP_ROWS * TILE_SIZE;
		float marginX = (float)(WALL_THICKNESS + 15) * TILE_SIZE;

		// "벽" 기능을 하는 "부서진 타워 모형" 생성
		SpawnObstacles(OBS_SOLID, SUB_TOWER_BROKEN, marginX, HALLWAY_Y + (hallwayH * 0.25f));
		SpawnObstacles(OBS_SOLID, SUB_TOWER_BROKEN, SCREEN_WIDTH - marginX * 1.8f, HALLWAY_Y + (hallwayH * 0.7f));

		// "벽" 기능을 하는 "터널 모형" 생성
		SpawnObstacles(OBS_SOLID, SUB_TOWER_TUNNEL, SCREEN_WIDTH - marginX, HALLWAY_Y + (hallwayH * 0.6f));
		SpawnObstacles(OBS_SOLID, SUB_TOWER_TUNNEL, marginX * 1.4f, HALLWAY_Y + (hallwayH * 0.3f));

		// "벽" 기능을 하는 "경사 모형" 생성
		SpawnObstacles(OBS_SOLID, SUB_TOWER_TILTED, SCREEN_WIDTH - marginX * 1.2f, HALLWAY_Y + (hallwayH * 0.75f));
		SpawnObstacles(OBS_SOLID, SUB_TOWER_TILTED, marginX * 1.1f, HALLWAY_Y + (hallwayH * 0.45f));
		SpawnObstacles(OBS_SOLID, SUB_TOWER_TILTED, SCREEN_WIDTH - marginX * 1.1f, HALLWAY_Y + (hallwayH * 0.15f));

		// "상태 이상" 기능을 하는 "사이버 끈끈이" 생성
		SpawnObstacles(OBS_HAZARD, SUB_CYBER_SLIME, SCREEN_WIDTH - marginX * 1.3f, HALLWAY_Y + (hallwayH * 0.8f));
		SpawnObstacles(OBS_HAZARD, SUB_CYBER_SLIME, marginX * 1.3f, HALLWAY_Y + (hallwayH * 0.53f));
		SpawnObstacles(OBS_HAZARD, SUB_CYBER_SLIME, SCREEN_WIDTH - marginX * 1.5f, HALLWAY_Y + (hallwayH * 0.2f));
		SpawnObstacles(OBS_HAZARD, SUB_CYBER_SLIME, SCREEN_WIDTH - marginX * 1.1f, HALLWAY_Y + (hallwayH * 0.34f));
	}
}

void ReleaseObstacles() {
	ReleaseMyImage(&imgObstacles);
}

void SpawnObstacles(OBS_TYPE type, OBS_SUBTYPE subType, float x, float y) {
	for (int i = 0; i < OBSTACLE_LIMIT; i++) {
		if (!obstacles[i].isActive) {
			obstacles[i].isActive = ACTIVE;
			obstacles[i].type = type;
			obstacles[i].subType = subType;
			obstacles[i].base.x = x;
			obstacles[i].base.y = y;

			switch (subType) {
			case SUB_TOWER_BROKEN:
				obstacles[i].base.width = CAT_TOWER_BROKEN_WIDTH;
				obstacles[i].base.height = CAT_TOWER_BROKEN_HEIGHT;
				obstacles[i].base.hitBoxW = CAT_TOWER_BROKEN_HITBOX_W;
				obstacles[i].base.hitBoxH = CAT_TOWER_BROKEN_HITBOX_H;
				break;

			case SUB_TOWER_TUNNEL:
				obstacles[i].base.width = CAT_TOWER_TUNNEL_WIDTH;
				obstacles[i].base.height = CAT_TOWER_TUNNEL_HEIGHT;
				obstacles[i].base.hitBoxW = CAT_TOWER_TUNNEL_HITBOX_W;
				obstacles[i].base.hitBoxH = CAT_TOWER_TUNNEL_HITBOX_H;
				break;

			case SUB_TOWER_TILTED:
				obstacles[i].base.width = CAT_TOWER_TILTED_WIDTH;
				obstacles[i].base.height = CAT_TOWER_TILTED_HEIGHT;
				obstacles[i].base.hitBoxW = CAT_TOWER_TILTED_HITBOX_W;
				obstacles[i].base.hitBoxH = CAT_TOWER_TILTED_HITBOX_H;
				break;

			case SUB_CYBER_SLIME:
				obstacles[i].base.width = CYBER_SLIME_WIDTH;
				obstacles[i].base.height = CYBER_SLIME_HEIGHT;
				obstacles[i].base.hitBoxW = CYBER_SLIME_HITBOX_W;
				obstacles[i].base.hitBoxH = CYBER_SLIME_HITBOX_H;
				break;
			}

			obstacles[i].base.hitBoxX = obstacles[i].base.x;
			if (obstacles[i].subType != SUB_CYBER_SLIME) {
				obstacles[i].base.hitBoxY = obstacles[i].base.y + (obstacles[i].base.height / 2.0f) - (obstacles[i].base.hitBoxH / 2.0f);
			}
			else if(obstacles[i].subType == SUB_CYBER_SLIME){
				obstacles[i].base.hitBoxY = obstacles[i].base.y;
			}

			return;
		}
	}
}

void UpdateObstacles() {
	// 필요 시 장애물 상태 업데이트 로직 추가 (예: 파괴 등)
}