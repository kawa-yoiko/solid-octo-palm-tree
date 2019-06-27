#ifndef _PANEL_H_
#define _PANEL_H_

#include <stdbool.h>

void PanelSetDimensions(int x, int y, int w, int h);

bool PanelMousePress(int x, int y);
void PanelMouseMove(int x, int y);
void PanelMouseRelease();

void PanelDraw();

#endif
