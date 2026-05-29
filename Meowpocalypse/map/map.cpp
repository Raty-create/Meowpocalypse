#include "map.h"
#include "config.h"
#include "player.h"
#include "enemy.h"
#include "camera.h"
#include "collision.h"

MAP_TYPE currentMapType = MAP_WAITING;
MAPDATA maps[7];

void InitDoor(MAPDATA* m, int row, int col) {
	if (m->doorCount >= MAX_DOORS) return;
	if (row < 0 || row >= MAX_MAP_ROWS || col < 0 || col >= MAX_MAP_COLS) return;

	m->tiles[row][col] = TILE_DOOR;
	m->doors[m->doorCount].row = row;
	m->doors[m->doorCount].col = col;
	m->doors[m->doorCount].state = DOOR_CLOSE;
	m->doorCount++;
}

// 대기방
void InitWaitingMap(MAPDATA* m) {
	m->rows = MIN(WAITINGMAP_ROWS, MAX_MAP_ROWS);
	m->cols = MIN(WAITINGMAP_COLS, MAX_MAP_COLS);
	m->worldX = WAITING_X;
	m->worldY = 0;
	m->doorCount = 0;

	for (int row = 0; row < m->rows; row++) {
		for (int col = 0; col < m->cols; col++) {
			if (row <= 5 || col <= 15 ||
				row >= m->rows - 5 || col >= m->cols - 15)
				// 벽
				m->tiles[row][col] = TILE_WALL;
			else
				// 바닥
				m->tiles[row][col] = TILE_FLOOR;
		}
	}
	// 문
	int doorRow = m->rows - 25;
	int doorCol = m->cols / 2;

	if (doorRow >= 0 && doorRow + 1 < MAX_MAP_ROWS && doorCol + 1 >= 0 && doorCol + 1 < MAX_MAP_COLS) {
		m->tiles[doorRow][doorCol] = TILE_DOOR;
		m->tiles[doorRow][doorCol + 1] = TILE_DOOR;
	}
	
	InitDoor(m, doorRow, doorCol);
	InitDoor(m, doorRow, doorCol + 1);
}

// 복도맵
void InitHallWayMap(MAPDATA* m, int worldX) {
	m->rows = MIN(HALLWAYMAP_ROWS, MAX_MAP_ROWS);
	m->cols = MIN(HALLWAYMAP_COLS, MAX_MAP_COLS);
	m->worldX = (float)worldX;
	m->worldY = 0;
	m->doorCount = 0;


	for (int row = 0; row < m->rows; row++) {
		for (int col = 0; col < m->cols; col++) {
			if (row <= 2 || row >= m->rows - 5 ||
				col <= 5 || col >= m->cols - 10) {
				//벽
				m->tiles[row][col] = TILE_WALL;
			}
			else {
				//바닥
				m->tiles[row][col] = TILE_FLOOR;
			}
		}
	}
	// 문
	int doorRow = m->rows - 44;
	int doorCol = m->cols / 2;

	if (doorRow >= 0 && doorRow + 1 < MAX_MAP_ROWS && doorCol + 1 >= 0 && doorCol + 1 < MAX_MAP_COLS) {
		m->tiles[doorRow][doorCol] = TILE_DOOR;
		m->tiles[doorRow][doorCol + 1] = TILE_DOOR;
	}

	InitDoor(m, doorRow, doorCol);
	InitDoor(m, doorRow, doorCol + 1);
}

// 보스맵
void InitBossMap(MAPDATA* m, int worldX) {
	m->rows = MIN(BOSSMAP_ROWS, MAX_MAP_ROWS);
	m->cols = MIN(BOSSMAP_COLS, MAX_MAP_COLS);
	m->worldX = (float)worldX;
	m->worldY = 0;
	m->doorCount = 0;

	for (int row = 0; row < m->rows; row++) {
		for (int col = 0; col < m->cols; col++) {
			if (row <= 3 || row >= m->rows - 5 ||
				col <= 0 || col >= m->cols - 10)
				// 벽
				m->tiles[row][col] = TILE_WALL;
			else
				// 바닥
				m->tiles[row][col] = TILE_FLOOR;
		}
	}
	// 문
	int doorRow = m->rows - 44;
	int doorCol = m->cols / 2;

	if (doorRow >= 0 && doorRow + 1 < MAX_MAP_ROWS && doorCol + 1 >= 0 && doorCol + 1 < MAX_MAP_COLS) {
		m->tiles[doorRow][doorCol] = TILE_DOOR;
		m->tiles[doorRow][doorCol + 1] = TILE_DOOR;
	}

	InitDoor(m, doorRow, doorCol);
	InitDoor(m, doorRow, doorCol + 1);
}

void InitMap(MAP_TYPE type) {
	MAPDATA* m = &maps[type];
	switch (type) {
	case MAP_WAITING: InitWaitingMap(m); break;
	case MAP_FIRST_HALLWAY: InitHallWayMap(m, HALLWAY1_X); break;
	case MAP_FIRST_BOSS: InitBossMap(m, BOSS1_X); break;
	case MAP_SECOND_HALLWAY: InitHallWayMap(m, HALLWAY2_X); break;
	case MAP_SECOND_BOSS: InitBossMap(m, BOSS2_X); break;
	case MAP_THIRD_HALLWAY: InitHallWayMap(m, HALLWAY3_X); break;
	case MAP_THIRD_BOSS: InitBossMap(m, BOSS3_X); break;
	}
}

void SetDoorState(MAP_TYPE type, DOOR_STATE state) {
	MAPDATA* m = &maps[type];
	for (int i = 0; i < m->doorCount; i++) {
		m->doors[i].state = state;
		m->tiles[m->doors[i].row][m->doors[i].col] = (state == DOOR_OPEN) ? TILE_DOOR : TILE_WALL;
	}
}

void InitAllMap() {
	for (int i = 0; i < 7; i++) {
		InitMap((MAP_TYPE)i);
		// 대기방 외에는 문을 닫아둠
		if (i != MAP_WAITING) {
			SetDoorState((MAP_TYPE)i, DOOR_CLOSE);
		}
	}

	SetDoorState(MAP_WAITING, DOOR_OPEN);
}

void UpdateDoors(MAP_TYPE type, int allEnemiesDead) {
	SetDoorState(type, allEnemiesDead ? DOOR_OPEN : DOOR_CLOSE);
}

MAP_TYPE GetNextMap(MAP_TYPE type) {
	switch (type) {
	case MAP_WAITING: return MAP_FIRST_HALLWAY; break;
	case MAP_FIRST_HALLWAY: return MAP_FIRST_BOSS; break;
	case MAP_FIRST_BOSS: return MAP_SECOND_HALLWAY; break;
	case MAP_SECOND_HALLWAY: return MAP_SECOND_BOSS; break;
	case MAP_SECOND_BOSS: return MAP_THIRD_HALLWAY; break;
	case MAP_THIRD_HALLWAY: return MAP_THIRD_BOSS; break;
	default: return type;
	}
}

// 맵 전환
void MapTransition() {
	if (!IsPlayerOnDoor()) return;

	MAP_TYPE nextMap = GetNextMap(currentMapType);
	if (nextMap == currentMapType) return;

	currentMapType = nextMap;

	// 새 맵으로 갈 때 문을 일단 닫음 (대기방 제외)
	if (currentMapType != MAP_WAITING) {
		SetDoorState(currentMapType, DOOR_CLOSE);
	}

	float spawnX, spawnY;
	GetSpawnPos(nextMap, &spawnX, &spawnY);
	player.base.x = player.base.hitBoxX = spawnX;
	player.base.y = player.base.hitBoxY = spawnY;

	if (currentMapType == MAP_FIRST_BOSS || currentMapType == MAP_SECOND_BOSS || currentMapType == MAP_THIRD_BOSS) {
		camera.isIntroActive = ACTIVE;
		camera.introTimer = 180; // 6ms 타이머 기준 180프레임 = 약 1초 초반이므로 정확히 3초를 원하시면 300~400으로 조절 가능
		camera.zoom = 2.0f;
	}

	MAPDATA* m = &maps[currentMapType];
	UpdateCamera(player.base.x, player.base.y, m->rows, m->cols);
	UpdateCamera(player.base.hitBoxX, player.base.hitBoxY, m->rows, m->cols);

	ClearEnemies();
	int spawnCount = 0;
	switch (currentMapType) {
	case MAP_FIRST_HALLWAY: spawnCount = 15; break;
	case MAP_SECOND_HALLWAY: spawnCount = 20; break;
	case MAP_THIRD_HALLWAY: spawnCount = 25; break;
	}
	SpawnEnemy(currentMapType, spawnCount);
}

void GetSpawnPos(MAP_TYPE type, float* outX, float* outY) {
	MAPDATA* m = &maps[type];
	// 문 통과 후 맵 전환시 맵의 왼쪽 벽 근처에서 스폰
	*outX = m->worldX + TILE_SIZE * 15 - TILE_SIZE / 2;
	*outY = m->worldY + (m->rows / 2) * TILE_SIZE + TILE_SIZE / 2;
}