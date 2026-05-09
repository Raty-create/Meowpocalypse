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
	m->worldX = 0;
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
	InitDoor(m, 4, 29);
	InitDoor(m, 5, 29);
	InitDoor(m, 4, 30);
	InitDoor(m, 5, 30);
}

// 첫번째 복도
static void InitFirstHallWay(MAPDATA* m) {
	m->rows = FIRST_HALLWAYMAP_ROWS;
	m->cols = FIRST_HALLWAYMAP_COLS;
	m->worldX = FIRST_HALLWAYMAP_X;
	m->worldY = 0;
	m->doorCount = 0;


	for (int row = 0; row < m->rows; row++) {
		for (int col = 0; col < m->cols; col++) {
			if (row == 0 || row == m->rows - 1 ||
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
	InitDoor(m, m->rows / 2, m->cols - 2);
	InitDoor(m, m->rows / 2 + 1, m->cols - 2);
}

void InitMap(MAP_TYPE type) {
	MAPDATA* m = &maps[type];
	switch (type) {
	case MAP_WAITING: InitWaitingMap(m); break;
	case MAP_FIRST_HALLWAY: InitFirstHallWay(m); break;
	}
}

void InitAllMap() {
	InitMap(MAP_WAITING);
	InitMap(MAP_FIRST_HALLWAY);
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