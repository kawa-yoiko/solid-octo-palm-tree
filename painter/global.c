#include "global.h"

Font font;
Font font_alt;

void PalmTreeSetup()
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);

    InitWindow(SCR_W, SCR_H, "Visualizer: solid-octo-palm-tree");
    SetTargetFPS(60);

    font = LoadFontEx("imprima.ttf", 128, 0, 0);
    font_alt = LoadFontEx("imprima.ttf", 144, 0, 0);
    GenTextureMipmaps(&font.texture);
    SetTextureFilter(font.texture, FILTER_POINT);
}
