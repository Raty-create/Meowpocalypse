#pragma once

#include <atlimage.h>

typedef struct {
	CImage img;
	int width;
	int height;
} IMAGE;

void LoadMyImage(IMAGE* image, LPCTSTR filePath);
void ReleaseMyImage(IMAGE* image);
void DrawMyImage(IMAGE* image, HDC hDC, int x, int y, int dw, int dh, int sx, int sy, int sw, int sh);