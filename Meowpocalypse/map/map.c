#include "map.h"
#include "config.h"

WAITINGMAP currentMap;
FIRSTHALLWAYMAP firsthallwayMap;

//대기방
void InitWaitingRoom() {
	for (int row = 0; row < WAITINGMAP_ROWS; row++) {
		for (int col = 0; col < WAITINGMAP_COLS; col++) {
			if (row <= 5 || col <= 15 ||
				row > WAITINGMAP_ROWS - 6 || col > WAITINGMAP_COLS - 16) {
				// 벽
				currentMap.tiles[row][col] = TILE_WALL;
			}
			else {
				// 바닥
				currentMap.tiles[row][col] = TILE_FLOOR;
			}
		}
	}
	currentMap.tiles[4][29] = TILE_DOOR;
	currentMap.tiles[5][29] = TILE_DOOR;
	currentMap.tiles[4][30] = TILE_DOOR;
	currentMap.tiles[5][30] = TILE_DOOR;
}
// 첫번째 복도
void InitFirstHallWay() {
	for (int row = 0; row < FIRST_HALLWAYMAP_ROWS; row++) {
		for (int col = 0; col < FIRST_HALLWAYMAP_COLS; col++) {
			if (row == 0 || row == FIRST_HALLWAYMAP_ROWS - 1 ||
				col == 0 || col == FIRST_HALLWAYMAP_COLS - 1) {
				//벽
				firsthallwayMap.tiles[row][col] = TILE_WALL;
			}
			else {
				//바닥
				firsthallwayMap.tiles[row][col] = TILE_FLOOR;
			}
		}
	}
}