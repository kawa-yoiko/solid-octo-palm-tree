#include "raylib.h"
#include "global.h"
#include <stdlib.h>

#define REPO_TITLE "solid-octo-palm-tree"

static const Vector2 Leaf[16] = {
    {25, 25},
    {100, 25},
    {100, 100},
    {25, 100}
};
static Vector2 *LeafScaled = NULL;

Vector2 *GenerateAnchoredPoly(
    const Vector2 *points, int numPoints,
    Vector2 offset, Vector2 anchor, float scale)
{
    Vector2 *scaledPoints = (Vector2 *)malloc(sizeof(Vector2) * numPoints);
    for (int i = 0; i < numPoints; i++) {
        scaledPoints[i].x = offset.x + (points[i].x - anchor.x) * scale;
        scaledPoints[i].y = offset.y + (points[i].y - anchor.y) * scale;
    }
    return scaledPoints;
}

void DrawIcon(Vector2 offset, float scale, double t)
{
    LeafScaled = GenerateAnchoredPoly(Leaf, 16, offset, (Vector2){0.5, 1}, 25);

    DrawLineEx(
        (Vector2){offset.x - 200 * scale, offset.y},
        (Vector2){offset.x + 200 * scale, offset.y},
        3, DARKGRAY);
    DrawPolyExLines(Leaf, 4, DARKGRAY);
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
