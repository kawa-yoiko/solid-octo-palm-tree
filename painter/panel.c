#include "panel.h"
#include "global.h"

// top x/y; total w/h
static int tx = 0, ty = 0, tw = SCR_W, th = SCR_H;

void PanelSetDimensions(int x, int y, int w, int h)
{
    tx = x;
    ty = y;
    tw = w;
    th = h;
}

bool PanelMousePress(int x, int y)
{
    x -= tx;
    y -= ty;
    if (x < 0 || x >= tw || y < 0 || y >= th) return false;
    return true;
}

void PanelMouseMove(int x, int y)
{
    x -= tx;
    y -= ty;
}

void PanelMouseRelease()
{
}

void PanelDraw()
{
    DrawRectangle(tx, ty, tw, th, Fade(GRAY_2, 0.8));
}
