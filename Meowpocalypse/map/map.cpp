#include "map.h"
#include "config.h"
#include "player.h"
#include "enemy.h"
#include "camera.h"
#include "collision.h"
#include "sound.h"

#include <stdlib.h>

MAP_TYPE currentMapType = MAP_WAITING;
MAPDATA maps[MAP_COUNT];
int g_hallwayStage = 0;

void InitDoor(MAPDATA* m, int row, int col) {
	if (m->doorCount >= MAX_DOORS) return;
	if (row < 0 || row >= MAX_MAP_ROWS || col < 0 || col >= MAX_MAP_COLS) return;

	m->tiles[row][col] = TILE_DOOR;
	m->doors[m->doorCount].row = row;
	m->doors[m->doorCount].col = col;
	m->doors[m->doorCount].state = DOOR_OPEN;
	m->doorCount++;
}

void SetObstacleRect(MAPDATA* m, int rowStart, int colStart, int rowEnd, int colEnd) {
	for (int row = rowStart; row <= rowEnd; row++) {
		for (int col = colStart; col <= colEnd; col++) {
			if (row < 0 || row >= m->rows || col < 0 || col >= m->cols) continue;
			m->tiles[row][col] = TILE_OBSTACLE;
		}
	}
}

// 대기방
void InitWaitingMap(MAPDATA* m) {
	m->rows = MIN(WAITINGMAP_ROWS, MAX_MAP_ROWS);
	m->cols = MIN(WAITINGMAP_COLS, MAX_MAP_COLS);
	m->worldX = 0;
	m->worldY = WAITING_Y;
	m->doorCount = 0;

	for (int row = 0; row < m->rows; row++) {
		for (int col = 0; col < m->cols; col++) {
			if (row <= WALL_THICKNESS || col <= WALL_THICKNESS ||
				row >= m->rows - WALL_THICKNESS || col >= m->cols - WALL_THICKNESS)
				// 벽
				m->tiles[row][col] = TILE_WALL;
			else
				// 바닥
				m->tiles[row][col] = TILE_FLOOR;
		}
	}
	// 위
	SetObstacleRect(m, WALL_THICKNESS + 1, WALL_THICKNESS + 1, WALL_THICKNESS + 4, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4);
	SetObstacleRect(m, WALL_THICKNESS + 1, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, WALL_THICKNESS + 4, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, WALL_THICKNESS + 4, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 6 , WALL_THICKNESS + 5, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4);
	SetObstacleRect(m, WALL_THICKNESS + 4, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, WALL_THICKNESS + 5, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 6);

	// 아래
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 5, WALL_THICKNESS + 1, m->rows - WALL_THICKNESS - 1, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 6, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 6, m->rows - WALL_THICKNESS - 5, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 3);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 6, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 3, m->rows - WALL_THICKNESS - 5, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 6);

	// 좌
	SetObstacleRect(m, WALL_THICKNESS, WALL_THICKNESS + 1, m->rows - WALL_THICKNESS - 1, WALL_THICKNESS + 2);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 2, WALL_THICKNESS + 2, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 2, WALL_THICKNESS + 3);

	// 우
	SetObstacleRect(m, WALL_THICKNESS, m->cols - WALL_THICKNESS - 2, m->rows - WALL_THICKNESS - 1, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 2, m->cols - WALL_THICKNESS - 3, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 2, m->cols - WALL_THICKNESS - 2);

	// 문
	int doorRow = WALL_THICKNESS + 4;
	int doorCol = ((m->cols - ( 2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS;

	if (doorRow >= 0 && doorRow + 1 < MAX_MAP_ROWS && doorCol - 1 >= 0 && doorCol + 1 < MAX_MAP_COLS) {

		m->tiles[doorRow][doorCol - 2] = TILE_DOOR;
		m->tiles[doorRow][doorCol - 1] = TILE_DOOR;
		m->tiles[doorRow][doorCol] = TILE_DOOR;
		m->tiles[doorRow][doorCol + 1] = TILE_DOOR;
		m->tiles[doorRow][doorCol + 2] = TILE_DOOR;
	}

	InitDoor(m, doorRow, doorCol - 2);
	InitDoor(m, doorRow, doorCol - 1);
	InitDoor(m, doorRow, doorCol);
	InitDoor(m, doorRow, doorCol + 1);
	InitDoor(m, doorRow, doorCol + 2);
}

// 복도맵
void InitHallWayMap(MAPDATA* m, int worldY) {
	m->rows = MIN(HALLWAYMAP_ROWS, MAX_MAP_ROWS);
	m->cols = MIN(HALLWAYMAP_COLS, MAX_MAP_COLS);
	m->worldX = 0;
	m->worldY = (float)worldY;
	m->doorCount = 0;


	for (int row = 0; row < m->rows; row++) {
		for (int col = 0; col < m->cols; col++) {
			if (row <= WALL_THICKNESS || row >= m->rows - WALL_THICKNESS ||
				col <= WALL_THICKNESS || col >= m->cols - WALL_THICKNESS) {
				//벽
				m->tiles[row][col] = TILE_WALL;
			}
			else {
				//바닥
				m->tiles[row][col] = TILE_FLOOR;
			}
		}
	}

	// 좌
	SetObstacleRect(m, WALL_THICKNESS + 1, WALL_THICKNESS + 1, m->rows - WALL_THICKNESS - 1, WALL_THICKNESS + 9);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 5, WALL_THICKNESS + 9, m->rows - WALL_THICKNESS - 1, WALL_THICKNESS + 10);
	SetObstacleRect(m, (m->rows - WALL_THICKNESS - (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2)) - 9, WALL_THICKNESS + 9, (m->rows - WALL_THICKNESS - (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2)) + 8, WALL_THICKNESS + 10);
	SetObstacleRect(m, (m->rows - WALL_THICKNESS - (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2)) - 4, WALL_THICKNESS + 10, (m->rows - WALL_THICKNESS - (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2)) - 1, WALL_THICKNESS + 11);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 1, WALL_THICKNESS + 9, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, WALL_THICKNESS + 10);
	SetObstacleRect(m, (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2) + WALL_THICKNESS - 6, WALL_THICKNESS + 9, (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2) + WALL_THICKNESS + 11, WALL_THICKNESS + 10);
	SetObstacleRect(m, (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2) + WALL_THICKNESS + 3, WALL_THICKNESS + 10, (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2) + WALL_THICKNESS + 6, WALL_THICKNESS + 11);
	SetObstacleRect(m, WALL_THICKNESS + 1, WALL_THICKNESS + 9, WALL_THICKNESS + 6, WALL_THICKNESS + 10);

	// 우
	SetObstacleRect(m, WALL_THICKNESS + 1, m->cols - WALL_THICKNESS - 9, m->rows - WALL_THICKNESS - 1, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 5, m->cols - WALL_THICKNESS - 10, m->rows - WALL_THICKNESS - 1, m->cols - WALL_THICKNESS - 9);
	SetObstacleRect(m, (m->rows - WALL_THICKNESS - (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2)) - 9, m->cols - WALL_THICKNESS - 10, (m->rows - WALL_THICKNESS - (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2)) + 8, m->cols - WALL_THICKNESS - 9);
	SetObstacleRect(m, (m->rows - WALL_THICKNESS - (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2)) - 4, m->cols - WALL_THICKNESS - 11, (m->rows - WALL_THICKNESS - (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2)) - 1, m->cols - WALL_THICKNESS - 10);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 1, m->cols - WALL_THICKNESS - 10, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, m->cols - WALL_THICKNESS - 9);
	SetObstacleRect(m, (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2) + WALL_THICKNESS - 6, m->cols - WALL_THICKNESS - 10, (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2) + WALL_THICKNESS + 11, m->cols - WALL_THICKNESS - 9);
	SetObstacleRect(m, (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2) + WALL_THICKNESS + 3, m->cols - WALL_THICKNESS - 11, (((m->rows - (2 * WALL_THICKNESS)) / 2) / 2) + WALL_THICKNESS + 6, m->cols - WALL_THICKNESS - 10);
	SetObstacleRect(m, WALL_THICKNESS + 1, m->cols - WALL_THICKNESS - 10, WALL_THICKNESS + 6, m->cols - WALL_THICKNESS - 9);

	// 문
	int doorRow = WALL_THICKNESS + 1;
	int doorCol = ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS;

	if (doorRow >= 0 && doorRow + 1 < MAX_MAP_ROWS && doorCol - 1 >= 0 && doorCol + 1 < MAX_MAP_COLS) {

		m->tiles[doorRow][doorCol - 1] = TILE_DOOR;
		m->tiles[doorRow][doorCol] = TILE_DOOR;
		m->tiles[doorRow][doorCol + 1] = TILE_DOOR;
	}

	InitDoor(m, doorRow, doorCol - 1);
	InitDoor(m, doorRow, doorCol);
	InitDoor(m, doorRow, doorCol + 1);
}

// 보스맵
void InitFirstBossMap(MAPDATA* m, int worldY) {
	m->rows = MIN(BOSSMAP_ROWS, MAX_MAP_ROWS);
	m->cols = MIN(BOSSMAP_COLS, MAX_MAP_COLS);
	m->worldX = 0;
	m->worldY = (float)worldY;
	m->doorCount = 0;

	for (int row = 0; row < m->rows; row++) {
		for (int col = 0; col < m->cols; col++) {
			if (row <= WALL_THICKNESS || row >= m->rows - WALL_THICKNESS ||
				col <= WALL_THICKNESS || col >= m->cols - WALL_THICKNESS)
				// 벽
				m->tiles[row][col] = TILE_WALL;
			else
				// 바닥
				m->tiles[row][col] = TILE_FLOOR;
		}
	}

	// 좌
	SetObstacleRect(m, WALL_THICKNESS + 1, WALL_THICKNESS + 1, m->rows - WALL_THICKNESS - 1, WALL_THICKNESS + 4);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 14, WALL_THICKNESS + 4, m->rows - WALL_THICKNESS - 1, WALL_THICKNESS + 8);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 15, WALL_THICKNESS + 4, m->rows - WALL_THICKNESS - 14, WALL_THICKNESS + 7);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 16, WALL_THICKNESS + 4, m->rows - WALL_THICKNESS - 15, WALL_THICKNESS + 6);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 17, WALL_THICKNESS + 4, m->rows - WALL_THICKNESS - 16, WALL_THICKNESS + 5);
	SetObstacleRect(m, (((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS) - 5, WALL_THICKNESS + 4, (((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS) + 4, WALL_THICKNESS + 8);
	SetObstacleRect(m, WALL_THICKNESS + 1, WALL_THICKNESS + 4, WALL_THICKNESS + 14, WALL_THICKNESS + 8);
	SetObstacleRect(m, WALL_THICKNESS + 15, WALL_THICKNESS + 4, WALL_THICKNESS + 16, WALL_THICKNESS + 7);
	SetObstacleRect(m, WALL_THICKNESS + 16, WALL_THICKNESS + 4, WALL_THICKNESS + 17, WALL_THICKNESS + 6);
	SetObstacleRect(m, WALL_THICKNESS + 17, WALL_THICKNESS + 4, WALL_THICKNESS + 18, WALL_THICKNESS + 5);

	// 우
	SetObstacleRect(m, WALL_THICKNESS + 1, m->cols - WALL_THICKNESS - 4, m->rows - WALL_THICKNESS - 1, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 14, m->cols - WALL_THICKNESS - 8, m->rows - WALL_THICKNESS - 1, m->cols - WALL_THICKNESS - 4);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 15, m->cols - WALL_THICKNESS - 7, m->rows - WALL_THICKNESS - 14, m->cols - WALL_THICKNESS - 4);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 16, m->cols - WALL_THICKNESS - 6, m->rows - WALL_THICKNESS - 15, m->cols - WALL_THICKNESS - 4);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 17, m->cols - WALL_THICKNESS - 5, m->rows - WALL_THICKNESS - 16, m->cols - WALL_THICKNESS - 4);
	SetObstacleRect(m, (((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS) - 5, m->cols - WALL_THICKNESS - 8, (((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS) + 4, m->cols - WALL_THICKNESS - 4);
	SetObstacleRect(m, WALL_THICKNESS + 1, m->cols - WALL_THICKNESS - 8, WALL_THICKNESS + 14, m->cols - WALL_THICKNESS - 4);
	SetObstacleRect(m, WALL_THICKNESS + 15, m->cols - WALL_THICKNESS - 7, WALL_THICKNESS + 16, m->cols - WALL_THICKNESS - 4);
	SetObstacleRect(m, WALL_THICKNESS + 16, m->cols - WALL_THICKNESS - 6, WALL_THICKNESS + 17, m->cols - WALL_THICKNESS - 4);
	SetObstacleRect(m, WALL_THICKNESS + 17, m->cols - WALL_THICKNESS - 5, WALL_THICKNESS + 18, m->cols - WALL_THICKNESS - 4);
	
	// 위 
	SetObstacleRect(m, WALL_THICKNESS + 1, WALL_THICKNESS + 1, WALL_THICKNESS + 5, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 3);
	SetObstacleRect(m, WALL_THICKNESS + 1, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 3, WALL_THICKNESS + 5, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, WALL_THICKNESS + 6, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 9, WALL_THICKNESS + 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 5);
	SetObstacleRect(m, WALL_THICKNESS + 5, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 10, WALL_THICKNESS + 6, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 9);
	SetObstacleRect(m, WALL_THICKNESS + 6, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 5, WALL_THICKNESS + 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 9);
	SetObstacleRect(m, WALL_THICKNESS + 5, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 9, WALL_THICKNESS + 6, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 10);
	SetObstacleRect(m, WALL_THICKNESS + 6, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 16, WALL_THICKNESS + 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 14);
	SetObstacleRect(m, WALL_THICKNESS + 5, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 17, WALL_THICKNESS + 6, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 16);
	SetObstacleRect(m, WALL_THICKNESS + 6, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 14, WALL_THICKNESS + 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 16);
	SetObstacleRect(m, WALL_THICKNESS + 5, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 16, WALL_THICKNESS + 6, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 17);
	SetObstacleRect(m, WALL_THICKNESS + 6, WALL_THICKNESS + 8, WALL_THICKNESS + 8, WALL_THICKNESS + 12);
	SetObstacleRect(m, WALL_THICKNESS + 6, m->cols - WALL_THICKNESS - 12, WALL_THICKNESS + 8, m->cols - WALL_THICKNESS - 8);

	// 아래
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 6, WALL_THICKNESS + 1, m->rows - WALL_THICKNESS - 1, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 6, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, m->rows - WALL_THICKNESS - 1, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 15, m->rows - WALL_THICKNESS - 6, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, m->rows - WALL_THICKNESS - 6, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 15);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 14, m->rows - WALL_THICKNESS - 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 8);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 8, m->rows - WALL_THICKNESS - 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 14);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, WALL_THICKNESS + 8, m->rows - WALL_THICKNESS - 7, WALL_THICKNESS + 12);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, m->cols - WALL_THICKNESS - 12, m->rows - WALL_THICKNESS - 7, m->cols - WALL_THICKNESS - 8);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 9, WALL_THICKNESS + 8, m->rows - WALL_THICKNESS - 8, WALL_THICKNESS + 11);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 9, m->cols - WALL_THICKNESS - 11, m->rows - WALL_THICKNESS - 8, m->cols - WALL_THICKNESS - 8);

	// 중앙
	SetObstacleRect(m, WALL_THICKNESS + 9, WALL_THICKNESS + 16, WALL_THICKNESS + 21, WALL_THICKNESS + 34);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 22, WALL_THICKNESS + 16, m->rows - WALL_THICKNESS - 10, WALL_THICKNESS + 34);
	SetObstacleRect(m, WALL_THICKNESS + 9, m->cols - 39, WALL_THICKNESS + 21, m->cols - WALL_THICKNESS - 16);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 22, m->cols - WALL_THICKNESS - 34, m->rows - WALL_THICKNESS - 10, m->cols - WALL_THICKNESS - 16);

	// 문
	int doorRow = WALL_THICKNESS + 5;
	int doorCol = ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS;

	if (doorRow >= 0 && doorRow + 1 < MAX_MAP_ROWS && doorCol - 1 >= 0 && doorCol + 1 < MAX_MAP_COLS) {

		m->tiles[doorRow][doorCol - 1] = TILE_DOOR;
		m->tiles[doorRow][doorCol] = TILE_DOOR;
		m->tiles[doorRow][doorCol + 1] = TILE_DOOR;
	}

	InitDoor(m, doorRow, doorCol - 1);
	InitDoor(m, doorRow, doorCol);
	InitDoor(m, doorRow, doorCol + 1);
}

void InitSecondBossMap(MAPDATA* m, int worldY) {
	m->rows = MIN(BOSSMAP_ROWS, MAX_MAP_ROWS);
	m->cols = MIN(BOSSMAP_COLS, MAX_MAP_COLS);
	m->worldX = 0;
	m->worldY = (float)worldY;
	m->doorCount = 0;

	for (int row = 0; row < m->rows; row++) {
		for (int col = 0; col < m->cols; col++) {
			if (row <= WALL_THICKNESS || row >= m->rows - WALL_THICKNESS ||
				col <= WALL_THICKNESS || col >= m->cols - WALL_THICKNESS)
				// 벽
				m->tiles[row][col] = TILE_WALL;
			else
				// 바닥
				m->tiles[row][col] = TILE_FLOOR;
		}
	}

	// 위
	SetObstacleRect(m, WALL_THICKNESS + 1, WALL_THICKNESS + 1, WALL_THICKNESS + 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4);
	SetObstacleRect(m, WALL_THICKNESS + 1, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, WALL_THICKNESS + 8, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, WALL_THICKNESS + 8, WALL_THICKNESS + 1, WALL_THICKNESS + 9, WALL_THICKNESS + 21);
	SetObstacleRect(m, WALL_THICKNESS + 8, m->cols - WALL_THICKNESS - 21, WALL_THICKNESS + 9, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, WALL_THICKNESS + 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 7, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4);
	SetObstacleRect(m, WALL_THICKNESS + 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 7);
	SetObstacleRect(m, WALL_THICKNESS + 9, WALL_THICKNESS + 1, WALL_THICKNESS + 10, WALL_THICKNESS + 20);
	SetObstacleRect(m, WALL_THICKNESS + 9, m->cols - WALL_THICKNESS - 20, WALL_THICKNESS + 10, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 5, WALL_THICKNESS + 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, WALL_THICKNESS + 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 5);
	SetObstacleRect(m, WALL_THICKNESS + 10, WALL_THICKNESS + 14, WALL_THICKNESS + 11, WALL_THICKNESS + 18);
	SetObstacleRect(m, WALL_THICKNESS + 10, m->cols - WALL_THICKNESS - 18, WALL_THICKNESS + 11, m->cols - WALL_THICKNESS - 14);
	SetObstacleRect(m, WALL_THICKNESS + 11, WALL_THICKNESS + 15, WALL_THICKNESS + 12, WALL_THICKNESS + 17);
	SetObstacleRect(m, WALL_THICKNESS + 11, m->cols - WALL_THICKNESS - 17, WALL_THICKNESS + 12, m->cols - WALL_THICKNESS - 15);
	SetObstacleRect(m, WALL_THICKNESS + 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 16, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 15);
	SetObstacleRect(m, WALL_THICKNESS + 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 23, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 22);
	SetObstacleRect(m, WALL_THICKNESS + 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 30, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 29);
	SetObstacleRect(m, WALL_THICKNESS + 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 15, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 16);
	SetObstacleRect(m, WALL_THICKNESS + 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 22, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 23);
	SetObstacleRect(m, WALL_THICKNESS + 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 29, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 30);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 3, WALL_THICKNESS + 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 3);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 3, WALL_THICKNESS + 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 3);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 11, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 11);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 11, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 11);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 10, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 10);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 10, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 10);

	// 아래
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 7, WALL_THICKNESS + 1, m->rows - WALL_THICKNESS - 1, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, WALL_THICKNESS + 1, m->rows - WALL_THICKNESS - 7, WALL_THICKNESS + 21);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, m->cols - WALL_THICKNESS - 21, m->rows - WALL_THICKNESS - 7, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4, m->rows - WALL_THICKNESS - 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, m->rows - WALL_THICKNESS - 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 9, WALL_THICKNESS + 1, m->rows - WALL_THICKNESS - 8, WALL_THICKNESS + 20);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 9, m->cols - WALL_THICKNESS - 20, m->rows - WALL_THICKNESS - 8, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, WALL_THICKNESS + 14, m->rows - WALL_THICKNESS - 9, WALL_THICKNESS + 18);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, m->cols - WALL_THICKNESS - 18, m->rows - WALL_THICKNESS - 9, m->cols - WALL_THICKNESS - 14);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 11, WALL_THICKNESS + 15, m->rows - WALL_THICKNESS - 10, WALL_THICKNESS + 17);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 11, m->cols - WALL_THICKNESS - 17, m->rows - WALL_THICKNESS - 10, m->cols - WALL_THICKNESS - 15);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 7, m->rows - WALL_THICKNESS - 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 6);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 16, m->rows - WALL_THICKNESS - 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 15);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 23, m->rows - WALL_THICKNESS - 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 22);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 30, m->rows - WALL_THICKNESS - 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 29);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 6, m->rows - WALL_THICKNESS - 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 7);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 15, m->rows - WALL_THICKNESS - 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 16);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 22, m->rows - WALL_THICKNESS - 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 23);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 29, m->rows - WALL_THICKNESS - 7, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 30);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 11, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 11);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 11, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 11);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 10, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 10);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 10, m->rows - WALL_THICKNESS - 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 10);

	// 좌
	SetObstacleRect(m, WALL_THICKNESS + 1, WALL_THICKNESS + 1, m->rows - WALL_THICKNESS - 1, WALL_THICKNESS + 7);
	SetObstacleRect(m, WALL_THICKNESS + 9, WALL_THICKNESS + 7,WALL_THICKNESS + 18, WALL_THICKNESS + 8);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 18, WALL_THICKNESS + 7, m->rows - WALL_THICKNESS - 9, WALL_THICKNESS + 8);
	SetObstacleRect(m, WALL_THICKNESS + 9, WALL_THICKNESS + 8, WALL_THICKNESS + 10, WALL_THICKNESS + 9);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, WALL_THICKNESS + 8, m->rows - WALL_THICKNESS - 9, WALL_THICKNESS + 9);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4, WALL_THICKNESS + 7, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, WALL_THICKNESS + 8);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 3, WALL_THICKNESS + 8, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 3, WALL_THICKNESS + 10);

	// 우
	SetObstacleRect(m, WALL_THICKNESS + 1, m->cols - WALL_THICKNESS - 7, m->rows - WALL_THICKNESS - 1, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, WALL_THICKNESS + 9, m->cols - WALL_THICKNESS - 8, WALL_THICKNESS + 18, m->cols - WALL_THICKNESS - 7);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 18, m->cols - WALL_THICKNESS - 8, m->rows - WALL_THICKNESS - 9, m->cols - WALL_THICKNESS - 7);
	SetObstacleRect(m, WALL_THICKNESS + 9, m->cols - WALL_THICKNESS - 9, WALL_THICKNESS + 10, m->cols - WALL_THICKNESS - 8);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, m->cols - WALL_THICKNESS - 9, m->rows - WALL_THICKNESS - 9, m->cols - WALL_THICKNESS - 8);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4, m->cols - WALL_THICKNESS - 8, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, m->cols - WALL_THICKNESS - 7);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 3, m->cols - WALL_THICKNESS - 10, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 3, m->cols - WALL_THICKNESS - 8);


	// 문
	int doorRow = WALL_THICKNESS + 9;
	int doorCol = ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS;

	if (doorRow >= 0 && doorRow + 1 < MAX_MAP_ROWS && doorCol - 1 >= 0 && doorCol + 1 < MAX_MAP_COLS) {

		m->tiles[doorRow][doorCol - 1] = TILE_DOOR;
		m->tiles[doorRow][doorCol] = TILE_DOOR;
		m->tiles[doorRow][doorCol + 1] = TILE_DOOR;
	}

	InitDoor(m, doorRow, doorCol - 1);
	InitDoor(m, doorRow, doorCol);
	InitDoor(m, doorRow, doorCol + 1);
}

void InitThirdBossMap(MAPDATA* m, int worldY) {
	m->rows = MIN(BOSSMAP_ROWS, MAX_MAP_ROWS);
	m->cols = MIN(BOSSMAP_COLS, MAX_MAP_COLS);
	m->worldX = 0;
	m->worldY = (float)worldY;
	m->doorCount = 0;

	for (int row = 0; row < m->rows; row++) {
		for (int col = 0; col < m->cols; col++) {
			if (row <= WALL_THICKNESS || row >= m->rows - WALL_THICKNESS ||
				col <= WALL_THICKNESS || col >= m->cols - WALL_THICKNESS)
				// 벽
				m->tiles[row][col] = TILE_WALL;
			else
				// 바닥
				m->tiles[row][col] = TILE_FLOOR;
		}
	}

	// 위
	SetObstacleRect(m, WALL_THICKNESS + 1, WALL_THICKNESS + 1, WALL_THICKNESS + 8, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4);
	SetObstacleRect(m, WALL_THICKNESS + 1, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, WALL_THICKNESS + 8, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, WALL_THICKNESS + 8, WALL_THICKNESS + 1, WALL_THICKNESS + 9, WALL_THICKNESS + 20);
	SetObstacleRect(m, WALL_THICKNESS + 8, m->cols - WALL_THICKNESS - 20, WALL_THICKNESS + 9, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, WALL_THICKNESS + 9, WALL_THICKNESS + 12, WALL_THICKNESS + 11, WALL_THICKNESS + 16);
	SetObstacleRect(m, WALL_THICKNESS + 9, m->cols - WALL_THICKNESS - 16, WALL_THICKNESS + 11, m->cols - WALL_THICKNESS - 12);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 6, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 6);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 17, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 16);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 24, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 23);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 31, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 30);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 16, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 17);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 23, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 24);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 30, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 31);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4, m->cols - WALL_THICKNESS - 8, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, m->cols - WALL_THICKNESS - 7);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 3, m->cols - WALL_THICKNESS - 10, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 3, m->cols - WALL_THICKNESS - 8);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 11, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 11);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 11, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 11);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 12, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 12);
	SetObstacleRect(m, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 12, WALL_THICKNESS + 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 12);

	// 아래
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 9, WALL_THICKNESS + 1, m->rows - WALL_THICKNESS - 1, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, WALL_THICKNESS + 1, m->rows - WALL_THICKNESS - 9, WALL_THICKNESS + 20);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, m->cols - WALL_THICKNESS - 20, m->rows - WALL_THICKNESS - 9, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 11, WALL_THICKNESS + 1, m->rows - WALL_THICKNESS - 10, WALL_THICKNESS + 19);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 11, m->cols - WALL_THICKNESS - 19, m->rows - WALL_THICKNESS - 10, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 12, WALL_THICKNESS + 12, WALL_THICKNESS + m->rows - WALL_THICKNESS - 11, WALL_THICKNESS + 16);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 12, m->cols - WALL_THICKNESS - 16, WALL_THICKNESS + m->rows - WALL_THICKNESS - 11, m->cols - WALL_THICKNESS - 12);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4, m->rows - WALL_THICKNESS - 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4, m->rows - WALL_THICKNESS - 9, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 4);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 17, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 16);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 24, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 23);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 31, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 30);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 16, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 17);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 23, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 24);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 30, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 31);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 11, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 11);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 11, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 11);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 12, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 12);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 12, m->rows - WALL_THICKNESS - 10, ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 12);

	// 좌
	SetObstacleRect(m, WALL_THICKNESS + 1, WALL_THICKNESS + 1, m->rows - WALL_THICKNESS - 1, WALL_THICKNESS + 4);
	SetObstacleRect(m, WALL_THICKNESS + 1, WALL_THICKNESS + 4, WALL_THICKNESS + 18, WALL_THICKNESS + 5);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 19, WALL_THICKNESS + 4, m->rows - WALL_THICKNESS - 1, WALL_THICKNESS + 5);
	SetObstacleRect(m, WALL_THICKNESS + 1, WALL_THICKNESS + 5, WALL_THICKNESS + 17, WALL_THICKNESS + 6);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 18, WALL_THICKNESS + 5, m->rows - WALL_THICKNESS - 1, WALL_THICKNESS + 6);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 5, WALL_THICKNESS + 5, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 3, WALL_THICKNESS + 6);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4, WALL_THICKNESS + 7, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 2, WALL_THICKNESS + 7);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 3, WALL_THICKNESS + 8, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 1, WALL_THICKNESS + 8);
	
	// 우
	SetObstacleRect(m, WALL_THICKNESS + 1, m->cols - WALL_THICKNESS - 4, m->rows - WALL_THICKNESS - 1, m->cols - WALL_THICKNESS - 1);
	SetObstacleRect(m, WALL_THICKNESS + 1, m->cols - WALL_THICKNESS - 5, WALL_THICKNESS + 18, m->cols - WALL_THICKNESS - 4);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 19, m->cols - WALL_THICKNESS - 5, m->rows - WALL_THICKNESS - 1, m->cols - WALL_THICKNESS - 4);
	SetObstacleRect(m, WALL_THICKNESS + 1, m->cols - WALL_THICKNESS - 6, WALL_THICKNESS + 17, m->cols - WALL_THICKNESS - 5);
	SetObstacleRect(m, m->rows - WALL_THICKNESS - 18, m->cols - WALL_THICKNESS - 6, m->rows - WALL_THICKNESS - 1, m->cols - WALL_THICKNESS - 5);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 5, m->cols - WALL_THICKNESS - 6, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 3, m->cols - WALL_THICKNESS - 5);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 4, m->cols - WALL_THICKNESS - 7, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 2, m->cols - WALL_THICKNESS - 7);
	SetObstacleRect(m, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS - 3, m->cols - WALL_THICKNESS - 8, ((m->rows - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS + 1, m->cols - WALL_THICKNESS - 8);

	// 문
	int doorRow = WALL_THICKNESS + 9;
	int doorCol = ((m->cols - (2 * WALL_THICKNESS)) / 2) + WALL_THICKNESS;

	if (doorRow >= 0 && doorRow + 1 < MAX_MAP_ROWS && doorCol - 1 >= 0 && doorCol + 1 < MAX_MAP_COLS) {

		m->tiles[doorRow][doorCol - 1] = TILE_DOOR;
		m->tiles[doorRow][doorCol] = TILE_DOOR;
		m->tiles[doorRow][doorCol + 1] = TILE_DOOR;
	}

	InitDoor(m, doorRow, doorCol - 1);
	InitDoor(m, doorRow, doorCol);
	InitDoor(m, doorRow, doorCol + 1);
}

void InitMap(MAP_TYPE type) {
	MAPDATA* m = &maps[type];

	m->doorAnimTimer = 0.0f;
	m->doorAnimFrame = 0;
	m->doorOffsetY = 0.0f;

	switch (type) {
	case MAP_WAITING: InitWaitingMap(m); break;
	case MAP_HALLWAY: InitHallWayMap(m, HALLWAY1_Y); break;
	case MAP_FIRST_BOSS: InitFirstBossMap(m, BOSS1_Y); break;
	case MAP_SECOND_BOSS: InitSecondBossMap(m, BOSS2_Y); break;
	case MAP_THIRD_BOSS: InitThirdBossMap(m, BOSS3_Y); break;
	}
}

void SetDoorState(MAP_TYPE type, DOOR_STATE state) {
	MAPDATA* m = &maps[type];

	if (m->doors[0].state == state) return;

	for (int i = 0; i < m->doorCount; i++) {
		m->doors[i].state = state;
		m->tiles[m->doors[i].row][m->doors[i].col] = (state == DOOR_OPEN) ? TILE_DOOR : TILE_WALL;
	}

	// 상태가 열리거나 닫힐 때 모두 프레임과 타이머를 초기화
	m->doorAnimTimer = 0.0f;
	m->doorAnimFrame = 0;
	m->doorOffsetY = 0.0f;
}

void InitAllMap() {
	g_hallwayStage = 0;
	for (int i = 0; i < MAP_COUNT; i++) {
		InitMap((MAP_TYPE)i);
		if (i != MAP_WAITING) SetDoorState((MAP_TYPE)i, DOOR_CLOSE);
	}
	SetDoorState(MAP_WAITING, DOOR_OPEN);
}

void UpdateDoors(MAP_TYPE type, int allEnemiesDead) {
	SetDoorState(type, allEnemiesDead ? DOOR_OPEN : DOOR_CLOSE);
}

void UpdateMapDoors() {
	MAPDATA* m = &maps[currentMapType];

	if (m->doorCount == 0 || m->doors->state == DOOR_CLOSE) return;

	m->doorAnimTimer += 1.0f;

	if (currentMapType == MAP_WAITING || currentMapType == MAP_HALLWAY || currentMapType == MAP_FIRST_BOSS) {
		if ((int)m->doorAnimTimer % 5 == 0) {
			m->doorAnimFrame++;

			if (currentMapType == MAP_WAITING) {
				if (m->doorAnimFrame >= MAP_WAITING_DOOR_MAX_FRAME) m->doorAnimFrame = MAP_WAITING_DOOR_MAX_FRAME;
			}
			else if (currentMapType == MAP_HALLWAY) {
				if (m->doorAnimFrame >= MAP_HALLWAY_DOOR_MAX_FRAME) m->doorAnimFrame = MAP_HALLWAY_DOOR_MAX_FRAME;
			}
			else if (currentMapType == MAP_FIRST_BOSS) {
				if (m->doorAnimFrame >= MAP_FIRST_BOSS_MAX_DOOR_FRAME) m->doorAnimFrame = MAP_FIRST_BOSS_MAX_DOOR_FRAME;
			}
		}
	}
	else if (currentMapType == MAP_SECOND_BOSS || currentMapType == MAP_THIRD_BOSS) {
		m->doorAnimFrame = 0;

		if (m->doorOffsetY > -150.0f) {
			m->doorOffsetY -= 3.0f;
		}
	}
}

MAP_TYPE GetNextMap(MAP_TYPE type) {
	switch (type) {
	case MAP_WAITING:     return MAP_HALLWAY;
	case MAP_HALLWAY:
		if (g_hallwayStage == 0)      return MAP_FIRST_BOSS;
		else if (g_hallwayStage == 1) return MAP_SECOND_BOSS;
		else                          return MAP_THIRD_BOSS;
	case MAP_FIRST_BOSS:  return MAP_HALLWAY;
	case MAP_SECOND_BOSS: return MAP_HALLWAY;
	case MAP_THIRD_BOSS:  return MAP_THIRD_BOSS;   // 마지막
	default: return type;
	}
}

void PlayStageBGM(MAP_TYPE type) {
	switch (type) {
	case MAP_HALLWAY:     PlayBGM(BGM_HALLWAY, TRUE, 100); break;
	case MAP_FIRST_BOSS:  PlayBGM(BGM_BOSS1, TRUE, 200);   break;
	case MAP_SECOND_BOSS: PlayBGM(BGM_BOSS2, TRUE, 800);   break;
	case MAP_THIRD_BOSS:  PlayBGM(BGM_BOSS3, TRUE, 900);   break;
	}
}


// 맵 전환
void MapTransition() {
	if (!IsPlayerOnDoor()) return;

	MAP_TYPE prevMap = currentMapType;
	MAP_TYPE nextMap = GetNextMap(prevMap);
	if (nextMap == prevMap) return;

	// 보스 클리어 후 떠날 때 다음 단계로
	if (prevMap == MAP_FIRST_BOSS)       g_hallwayStage = 1;
	else if (prevMap == MAP_SECOND_BOSS) g_hallwayStage = 2;

	camera.zoom = CAMERA_ZOOM_RATIO;
	currentMapType = nextMap;
	PlayStageBGM(currentMapType);

	if (currentMapType != MAP_WAITING)
		SetDoorState(currentMapType, DOOR_CLOSE);

	float spawnX, spawnY;
	GetSpawnPos(nextMap, &spawnX, &spawnY);
	player.base.x = player.base.hitBoxX = spawnX;
	player.base.y = player.base.hitBoxY = spawnY;

	if (currentMapType == MAP_FIRST_BOSS || currentMapType == MAP_SECOND_BOSS || currentMapType == MAP_THIRD_BOSS) {
		camera.isIntroActive = ACTIVE;
		camera.introTimer = 180;
		camera.zoom = 2.0f;
	}

	MAPDATA* m = &maps[currentMapType];
	UpdateCamera(player.base.x, player.base.y, m->rows, m->cols);
	UpdateCamera(player.base.hitBoxX, player.base.hitBoxY, m->rows, m->cols);

	ClearEnemies();
	int spawnCount = 0;
	if (currentMapType == MAP_HALLWAY) {
		spawnCount = 15 + rand() % 11;   // 15 ~ 25 랜덤
	}
	SpawnEnemy(currentMapType, spawnCount);
}

void GetSpawnPos(MAP_TYPE type, float* outX, float* outY) {
	MAPDATA* m = &maps[type];
	*outX = m->worldX + (m->cols / 2) * TILE_SIZE + TILE_SIZE / 2;	
	if (type == MAP_FIRST_BOSS || type == MAP_SECOND_BOSS || type == MAP_THIRD_BOSS) {
		*outY = m->worldY + (m->rows - 16) * TILE_SIZE;
	}
	else {
		*outY = m->worldY + (m->rows - 10) * TILE_SIZE;
	}
}