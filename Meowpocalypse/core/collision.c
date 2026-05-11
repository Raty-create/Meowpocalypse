#include "map.h"
#include "config.h"
#include "player.h"
#include "collision.h"

// 플레이어 - 벽 충돌 체크
int IsTileWall(float x, float y) {
	MAPDATA* m = &maps[currentMapType];

	int col = (int)((x - m->worldX) / TILE_SIZE);
	int row = (int)((y - m->worldY) / TILE_SIZE);

	if (row < 0 || row >= m->rows) return 1;
	if (col < 0 || col >= m->cols) return 1;

	int tile = m->tiles[row][col];
	return tile == TILE_WALL;
}
// 플레이어 - 문
int IsPlayerOnDoor() {
	MAPDATA* m = &maps[currentMapType];

	float point[4][2] = {
		{player.base.x - PLAYER_SIZE / 2 + 1, player.base.y - PLAYER_SIZE / 2 + 1},
		{player.base.x - PLAYER_SIZE / 2 - 1, player.base.y - PLAYER_SIZE / 2 + 1},
		{player.base.x - PLAYER_SIZE / 2 + 1, player.base.y - PLAYER_SIZE / 2 - 1},
		{player.base.x - PLAYER_SIZE / 2 - 1, player.base.y - PLAYER_SIZE / 2 - 1}
	};

	for (int p = 0; p < 4; p++) {
		int col = (int)((point[p][0] - m->worldX) / TILE_SIZE);
		int row = (int)((point[p][1] - m->worldY) / TILE_SIZE);

		if (row < 0 || row >= m->rows || col < 0 || col >= m->cols) continue;
		if (m->tiles[row][col] != TILE_DOOR) continue;

		for (int d = 0; d < m->doorCount; d++) {
			if (m->doors[d].row == row && m->doors[d].col == col && m->doors[d].state == DOOR_OPEN) {
				return 1;
			}
		}
	}
	return 0;
}