#include "raylib.h"
#include "global.h"

#define REPO_TITLE "solid-octo-palm-tree"

int main(int argc, char *argv[])
{
    SetConfigFlags(FLAG_VSYNC_HINT);

    InitWindow(SCR_W, SCR_H, "Visualizer");
    SetTargetFPS(60);

    Font font = LoadFontEx("imprima.ttf", 64, 0, 0);
    GenTextureMipmaps(&font.texture);
    SetTextureFilter(font.texture, FILTER_POINT);

    while (!WindowShouldClose()) {
        Vector2 sz = MeasureTextEx(font, REPO_TITLE, 64, 0);
        Vector2 pos = {(SCR_W - sz.x) / 2, (SCR_H - sz.y) / 2};

        BeginDrawing();
            ClearBackground((Color){240, 255, 249});
            DrawTextEx(font, REPO_TITLE, pos, 64, 0, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
