#include "map.h"
#include "config.h"

MAP_TYPE currentMapType = MAP_WAITING;
MAPDATA maps[7];

static void InitDoor(MAPDATA* m, int row, int col) {
	if (m->doorCount >= MAX_DOORS) return;
	m->tiles[row][col] = TILE_DOOR;
	m->doors[m->doorCount].row = row;
	m->doors[m->doorCount].col = col;
	m->doors[m->doorCount].state = DOOR_CLOSE;
	m->doorCount++;
}
// 대기방
static void InitWaitingMap(MAPDATA* m) {
	m->rows = WAITINGMAP_ROWS;
	m->cols = WAITINGMAP_COLS;
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
	int doorRow = m->rows / 2;
	int doorCol = m->cols - 16;

	m->tiles[doorRow][doorCol+1] = TILE_DOOR;
	m->tiles[doorRow + 1][doorCol + 1] = TILE_DOOR;
	
	InitDoor(m, doorRow, doorCol);
	InitDoor(m, doorRow + 1, doorCol);
}

// 복도맵
static void InitHallWayMap(MAPDATA* m, int worldX) {
	m->rows = HALLWAYMAP_ROWS;
	m->cols = HALLWAYMAP_COLS;
	m->worldX = worldX;
	m->worldY = 0;
	m->doorCount = 0;


	for (int row = 0; row < m->rows; row++) {
		for (int col = 0; col < m->cols; col++) {
			if (row <= 7 || row >= m->rows - 7 ||
				col == 0 || col == m->cols - 1) {
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
	int doorRow = m->rows / 2;
	int doorCol = m->cols - 2;

	InitDoor(m, doorRow, doorCol);
	InitDoor(m, doorRow + 1, doorCol);
}

// 보스맵
static void InitBossMap(MAPDATA* m, int worldX) {
	m->rows = BOSSMAP_ROWS;
	m->cols = BOSSMAP_COLS;
	m->worldX = worldX;
	m->worldY = 0;
	m->doorCount = 0;

	for (int row = 0; row < m->rows; row++) {
		for (int col = 0; col < m->cols; col++) {
			if (row < 1 || row >= m->rows - 1 ||
				col < 1 || col >= m->cols - 1)
				// 벽
				m->tiles[row][col] = TILE_WALL;
			else
				// 바닥
				m->tiles[row][col] = TILE_FLOOR;
		}
	}
	// 문
	int doorRow = m->rows / 2;
	int doorCol = m->cols - 2;

	m->tiles[doorRow][doorCol + 1] = TILE_DOOR;
	m->tiles[doorRow + 1][doorCol + 1] = TILE_DOOR;

	InitDoor(m, doorRow, doorCol);
	InitDoor(m, doorRow + 1, doorCol);
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

void InitAllMap() {
	for (int i = 0; i < 7; i++) {
		InitMap((MAP_TYPE)i);
	}

	// 일단 모든 문 OPEN해놓음
	for (int i = 0; i < 7; i++) {
		SetDoorState((MAP_TYPE)i, DOOR_OPEN);
	}
}

void SetDoorState(MAP_TYPE type, DOOR_STATE state) {
	MAPDATA* m = &maps[type];
	for (int i = 0; i < m->doorCount; i++) {
		m->doors[i].state = state;
		m->tiles[m->doors[i].row][m->doors[i].col] = (state == DOOR_OPEN) ? TILE_DOOR : TILE_WALL;
	}
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
	}
}

void GetSpawnPos(MAP_TYPE type, float* outX, float* outY) {
	MAPDATA* m = &maps[type];
	// 문 통과 후 맵 전환시 맵의 왼쪽 벽 근처에서 스폰
	*outX = m->worldX + TILE_SIZE * 3;
	*outY = m->worldY + (m->rows / 2) * TILE_SIZE + TILE_SIZE / 2;
}