#pragma once

#include "config.h"
#include "enum.h"
#include "object.h"
#include "map.h"
#include "image.h"

typedef struct {
	OBJECT base;
	OBS_TYPE type;					// 대분류(기능)
	OBS_SUBTYPE subType;			// 소분류(외형)
	BOOL isActive;
} OBSTACLE;

extern OBSTACLE obstacles[OBSTACLE_LIMIT];
extern IMAGE imgObstacles;

void InitObstacles();
void SpawnObstacles(OBS_TYPE type, OBS_SUBTYPE subType, float x, float y);
void ReleaseObstacles();
void UpdateObstacles();