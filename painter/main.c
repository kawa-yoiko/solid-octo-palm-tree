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

static Vector2 LeafL[7] = {
    {1, 0.5},
    {0.5, 0},
    {-0.1, 0.4},
    {0, 0.5},
    {-0.1, 0.7},
    {0.5, 0.3},
    {1, 0.5}
};
static Vector2 LeafR[7] = {
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

static inline float easeCycle(float x, float period, float amplitude)
{
    return sinf(x / period * 2 * M_PI) * amplitude;
}

void DrawIcon(Vector2 offset, float scale, float t)
{
    float rate;

    rate = EaseQuadOut(Min(t, 0.5) * 2);
    DrawLineEx(
        (Vector2){offset.x - 200 * scale * rate, offset.y},
        (Vector2){offset.x + 200 * scale * rate, offset.y},
        3, DARKGRAY);

    rate = EaseElasticOut(Clamp(t, 0.3, 1.3) - 0.3, 0.3);
    GenerateAnchoredBezier(&TrunkScaled, Trunk, 3, 12,
        offset, (Vector2){0.5, 1}, 120 * scale, 120 * scale * rate, 0);
    DrawPolyFilledConcave(TrunkScaled, 37, DARKGRAY);

    rate = EaseCubicOut((Clamp(t, 1.05, 1.25) - 1.05) * 5);
    if (rate > 1e-4) {
        LeafL[1].y = 0;
        GenerateAnchoredBezier(&Leaf1Scaled, LeafL, 2, 12,
            (Vector2){offset.x + 2 * scale, offset.y - 86 * scale},
            (Vector2){1, 0.5}, 70 * scale * rate, 70 * scale * rate,
            0 + easeCycle(t, 2.51, 0.025));
        DrawPolyFilledConcave(Leaf1Scaled, 25, DARKGRAY);
    }

    rate = EaseCubicOut((Clamp(t, 0.8, 1.05) - 0.8) * 4);
    if (rate > 1e-4) {
        LeafL[1].y = -0.04 + easeCycle(t, 3.2, 0.02);
        GenerateAnchoredBezier(&Leaf2Scaled, LeafL, 2, 12,
            (Vector2){offset.x + 7 * scale, offset.y - 86 * scale},
            (Vector2){1, 0.5}, 80 * scale * rate, 80 * scale * rate,
            -0.4 + easeCycle(t + 3, 3.51, 0.035));
        DrawPolyFilledConcave(Leaf2Scaled, 25, DARKGRAY);
    }

    rate = EaseCubicOut((Clamp(t, 0.92, 1.17) - 0.92) * 4);
    if (rate > 1e-4) {
        LeafR[5].y = easeCycle(t, 2.2, 0.02);
        GenerateAnchoredBezier(&Leaf3Scaled, LeafR, 2, 12,
            (Vector2){offset.x - 6 * scale, offset.y - 82 * scale},
            (Vector2){0, 0.5}, 80 * scale * rate, 80 * scale * rate,
            +0.1 + easeCycle(t + 1, 3.38, 0.04));
        DrawPolyFilledConcave(Leaf3Scaled, 25, DARKGRAY);
    }

    rate = EaseCubicOut((Clamp(t, 1.23, 1.41) - 1.23) * 5);
    if (rate > 1e-4) {
        LeafR[5].y = -0.05;
        GenerateAnchoredBezier(&Leaf4Scaled, LeafR, 2, 12,
            (Vector2){offset.x - 6 * scale, offset.y - 82 * scale},
            (Vector2){0, 0.5}, 60 * scale * rate, 60 * scale * rate,
            -0.4 + easeCycle(t + 2, 2.71, 0.03));
        DrawPolyFilledConcave(Leaf4Scaled, 25, DARKGRAY);
    }
}

static inline void DrawStartupScreen()
{
    float t = GetTime();
    if (t < 0.3) return;
    t -= 0.3;

    float rate = EaseCubicOut((Clamp(t, 1.7, 2.2) - 1.7) * 2);
    DrawIcon((Vector2){SCR_W / 2, SCR_H * (0.58 - rate * 0.05)}, SCR_W / 900.0, t);

    Vector2 sz = MeasureTextEx(font, REPO_TITLE, 64, 0);
    DrawTextEx(font, REPO_TITLE,
        (Vector2){(SCR_W - sz.x) / 2, SCR_H * (0.52 + rate * 0.02)},
        64, 0, Fade(DARKGRAY, rate));
}

int main(int argc, char *argv[])
{
    PalmTreeSetup();

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground((Color){240, 255, 249});
            DrawStartupScreen();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
