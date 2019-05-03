#include "raylib.h"
#include "global.h"
#include <math.h>
#include <stdlib.h>

#define REPO_TITLE "solid-octo-palm-tree"

static Vector2 Trunk[16] = {
    {0.55, 1},      // 0
    {0.55, 0.6},
    {0.5, 0.45},
    {0.45, 0.3},    // 1
    {0, 0},
    {0, 0},
    {0.55, 0.25},   // 2
};
static Vector2 *TrunkScaled = NULL;

static const Vector2 Leaf[16] = {
    {0, 0},
    {0, 1},
    {1, 1},
    {1, 0}
};
static Vector2 *LeafScaled = NULL;

void DrawIcon(Vector2 offset, float scale, double t)
{
    GenerateAnchoredPoly(&LeafScaled, Leaf, 16, offset, (Vector2){0.5, 1}, 25);

    GenerateAnchoredBezier(&TrunkScaled, Trunk, 1, 24, offset, (Vector2){0.5, 1}, 120);

    DrawLineEx(
        (Vector2){offset.x - 200 * scale, offset.y},
        (Vector2){offset.x + 200 * scale, offset.y},
        3, DARKGRAY);
    DrawPolyFilledConvex(LeafScaled, 4, DARKGRAY);
    DrawPolyFilledConcave(TrunkScaled, 25, DARKGRAY);
}

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
            DrawIcon((Vector2){SCR_W * 2 / 3, SCR_H / 3}, 1, GetTime());
            DrawTextEx(font, REPO_TITLE, pos, 64, 0, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
