#include "raylib.h"
#include "global.h"
#include <math.h>
#include <stdlib.h>

#define REPO_TITLE "solid-octo-palm-tree"

static Vector2 Trunk[10] = {
    {0.7, 1},       // 0
    {0.65, 0.55},
    {0.6, 0.35},
    {0.55, 0.25},   // 1
    {0.55, 0.25},
    {0.5, 0.225},
    {0.45, 0.3},    // 2
    {0.5, 0.45},
    {0.55, 0.6},
    {0.55, 1},      // 3
};
static Vector2 *TrunkScaled = NULL;

static const Vector2 LeafL[7] = {
    {1, 0.5},
    {0.5, 0},
    {-0.1, 0.4},
    {0, 0.5},
    {-0.1, 0.7},
    {0.5, 0.3},
    {1, 0.5}
};
static const Vector2 LeafR[7] = {
    {0, 0.5},
    {0.5, 0.15},
    {1.05, 0.8},
    {1, 0.5},
    {1.1, 0.2},
    {0.1, -0.05},
    {0, 0.5}
};
static Vector2 *Leaf1Scaled = NULL;
static Vector2 *Leaf2Scaled = NULL;
static Vector2 *Leaf3Scaled = NULL;
static Vector2 *Leaf4Scaled = NULL;
static Vector2 *Leaf5Scaled = NULL;

void DrawIcon(Vector2 offset, float scale, double t)
{
    GenerateAnchoredBezier(&TrunkScaled, Trunk, 3, 12,
        offset, (Vector2){0.5, 1}, 120 * scale, 0);
    GenerateAnchoredBezier(&Leaf1Scaled, LeafL, 2, 12,
        (Vector2){offset.x + 2 * scale, offset.y - 86 * scale},
        (Vector2){1, 0.5}, 70 * scale, 0);
    GenerateAnchoredBezier(&Leaf2Scaled, LeafL, 2, 12,
        (Vector2){offset.x + 7 * scale, offset.y - 86 * scale},
        (Vector2){1, 0.5}, 80 * scale, -0.4);
    GenerateAnchoredBezier(&Leaf3Scaled, LeafR, 2, 12,
        (Vector2){offset.x - 6 * scale, offset.y - 82 * scale},
        (Vector2){0, 0.5}, 80 * scale, +0.1);
    GenerateAnchoredBezier(&Leaf4Scaled, LeafR, 2, 12,
        (Vector2){offset.x - 6 * scale, offset.y - 82 * scale},
        (Vector2){0, 0.5}, 60 * scale, -0.4);

    DrawLineEx(
        (Vector2){offset.x - 200 * scale, offset.y},
        (Vector2){offset.x + 200 * scale, offset.y},
        3, DARKGRAY);
    DrawPolyFilledConcave(TrunkScaled, 37, DARKGRAY);
    DrawPolyFilledConcave(Leaf1Scaled, 25, DARKGRAY);
    DrawPolyFilledConcave(Leaf2Scaled, 25, DARKGRAY);
    DrawPolyFilledConcave(Leaf3Scaled, 25, DARKGRAY);
    DrawPolyFilledConcave(Leaf4Scaled, 25, DARKGRAY);
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
