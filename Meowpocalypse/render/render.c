#include "render.h"
#include "player.h"

void RenderPlayer(HDC mDC) {
	Rectangle(mDC, (int)player.base.x, (int)player.base.y, (int)player.base.x + player.base.width, (int)player.base.y + player.base.height);
}