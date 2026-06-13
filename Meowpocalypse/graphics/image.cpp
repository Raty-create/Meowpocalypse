#include "image.h"

void LoadMyImage(IMAGE* image, LPCTSTR filePath) {
	if (!image->img.IsNull()) image->img.Destroy();

	HRESULT hr = image->img.Load(filePath);
	if (FAILED(hr)) return;

	image->width = image->img.GetWidth();
	image->height = image->img.GetHeight();

	// 32비트 PNG (알파 채널)의 경우 투명도 전처리
	if (image->img.GetBPP() == 32) {
		for (int y = 0; y < image->height; y++) {
			for (int x = 0; x < image->width; x++) {
				unsigned char* argb = (unsigned char*)image->img.GetPixelAddress(x, y);
				unsigned char alpha = argb[3];
				if (alpha < 255) {
					argb[0] = (argb[0] * alpha + 127) / 255;
					argb[1] = (argb[1] * alpha + 127) / 255;
					argb[2] = (argb[2] * alpha + 127) / 255;
				}
			}
		}
	}
}

void ReleaseMyImage(IMAGE* image) {
	if (!image->img.IsNull()) image->img.Destroy();
	image->width = 0;
	image->height = 0;
}

void DrawMyImage(IMAGE* image, HDC hDC, int x, int y, int dw, int dh, int sx, int sy, int sw, int sh) {
	if (image->img.IsNull()) return;
	if (dw <= 0 || dh <= 0 || sw <= 0 || sh <= 0) return;

	image->img.Draw(hDC, x, y, dw, dh, sx, sy, sw, sh);
}