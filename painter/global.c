#include "global.h"

Font font;

void PalmTreeSetup()
{
    SetConfigFlags(FLAG_VSYNC_HINT);

    InitWindow(SCR_W, SCR_H, "Visualizer");
    SetTargetFPS(60);

    font = LoadFontEx("imprima.ttf", 64, 0, 0);
    GenTextureMipmaps(&font.texture);
    SetTextureFilter(font.texture, FILTER_POINT);
}
