#include "global.h"
#include "graph_op.h"
#include "panel.h"

#include "rlgl.h"

#include <math.h>
#include <stdbool.h>
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

static bool isPanelPressed;

void DrawMainScreen()
{
    ClearBackground(GRAY_2);

#define quq "Nothing for now   o(' ~ ')o"
    Vector2 sz = MeasureTextEx(font, quq, 64, 0);
    DrawTextEx(font, quq,
        (Vector2){(SCR_W - sz.x) / 2, (SCR_H - sz.y) / 2},
        64, 0, LIGHTGRAY);
#undef quq

    if (IsMouseButtonPressed(0)) {
        isPanelPressed = PanelMousePress(GetMouseX() * 2, GetMouseY() * 2);
        if (!isPanelPressed)
            VerletMousePress(GetMouseX() * 2, GetMouseY() * 2);
    }

    if (IsMouseButtonDown(0) && !isPanelPressed)
        VerletResetRate();

    int wheel = GetMouseWheelMove();
    if (wheel != 0)
        VerletChangeScale(wheel, GetMouseX() * 2, GetMouseY() * 2);

    if (IsMouseButtonDown(0) && isPanelPressed)
        PanelMouseMove(GetMouseX() * 2, GetMouseY() * 2);
    VerletMouseMove(GetMouseX() * 2, GetMouseY() * 2);

    if (IsMouseButtonReleased(0)) {
        if (isPanelPressed) PanelMouseRelease();
        else VerletMouseRelease();
    }

    VerletTick();
    VerletDraw();
    PanelDraw();
}

void DrawIcon(Vector2 offset, float scale, float t)
{
    float rate;

    rate = EaseQuadOut(Min(t, 0.5) * 2);
    DrawLineEx(
        (Vector2){offset.x - 200 * scale * rate, offset.y},
        (Vector2){offset.x + 200 * scale * rate, offset.y},
        3, GRAY_8);

    rate = EaseElasticOut(Clamp(t, 0.3, 1.3) - 0.3, 0.3);
    GenerateAnchoredBezier(&TrunkScaled, Trunk, 3, 12,
        offset, (Vector2){0.5, 1}, 120 * scale, 120 * scale * rate, 0);
    DrawPolyFilledConcave(TrunkScaled, 37, GRAY_8);

    const int OvO = 54;

    rate = EaseCubicOut((Clamp(t, 1.05, 1.25) - 1.05) * 5);
    if (rate > 1e-4) {
        LeafL[1].y = 0;
        GenerateAnchoredBezier(&Leaf1Scaled, LeafL, 2, OvO,
            (Vector2){offset.x + 2 * scale, offset.y - 86 * scale},
            (Vector2){1, 0.5}, 70 * scale * rate, 70 * scale * rate,
            0 + easeCycle(t, 2.51, 0.025));
        DrawPolyFilledConcave(Leaf1Scaled, OvO * 2 + 1, GRAY_8);
    }

    rate = EaseCubicOut((Clamp(t, 0.8, 1.05) - 0.8) * 4);
    if (rate > 1e-4) {
        LeafL[1].y = -0.04 + easeCycle(t, 3.2, 0.02);
        GenerateAnchoredBezier(&Leaf2Scaled, LeafL, 2, OvO,
            (Vector2){offset.x + 7 * scale, offset.y - 86 * scale},
            (Vector2){1, 0.5}, 80 * scale * rate, 80 * scale * rate,
            -0.4 + easeCycle(t + 3, 3.51, 0.035));
        DrawPolyFilledConcave(Leaf2Scaled, OvO * 2 + 1, GRAY_8);
    }

    rate = EaseCubicOut((Clamp(t, 0.92, 1.17) - 0.92) * 4);
    if (rate > 1e-4) {
        LeafR[5].y = easeCycle(t, 2.2, 0.02);
        GenerateAnchoredBezier(&Leaf3Scaled, LeafR, 2, OvO,
            (Vector2){offset.x - 6 * scale, offset.y - 82 * scale},
            (Vector2){0, 0.5}, 80 * scale * rate, 80 * scale * rate,
            +0.1 + easeCycle(t + 1, 3.38, 0.04));
        DrawPolyFilledConcave(Leaf3Scaled, OvO * 2 + 1, GRAY_8);
    }

    rate = EaseCubicOut((Clamp(t, 1.23, 1.41) - 1.23) * 5);
    if (rate > 1e-4) {
        LeafR[5].y = -0.05;
        GenerateAnchoredBezier(&Leaf4Scaled, LeafR, 2, OvO,
            (Vector2){offset.x - 6 * scale, offset.y - 82 * scale},
            (Vector2){0, 0.5}, 60 * scale * rate, 60 * scale * rate,
            -0.4 + easeCycle(t + 2, 2.71, 0.03));
        DrawPolyFilledConcave(Leaf4Scaled, OvO * 2 + 1, GRAY_8);
    }

    t -= 2.5;
    if (t > 1e-5) {
        Vector2 p1 = {offset.x - 10 * scale, offset.y - 86 * scale};
        Vector2 p2 = {offset.x + 6 * scale, offset.y - 84 * scale};
        Vector2 p3 = {offset.x - 1 * scale, offset.y - 77 * scale};

        if (t >= 0.9) {
            Vector2 o = {offset.x, offset.y - 83 * scale};
            rate = (t - 0.9) * (t - 0.9) * 360 * scale;
            p1 = Vector2Add(p1, (Vector2){-rate * 0.866, -rate * 0.5});
            p2 = Vector2Add(p2, (Vector2){+rate * 0.866, -rate * 0.5});
            p3 = Vector2Add(p3, (Vector2){0, +rate});
            p1 = Vector2Rotate(p1, o, rate / 100);
            p2 = Vector2Rotate(p2, o, rate / 100);
            p3 = Vector2Rotate(p3, o, rate / 100);
            DrawTriangle(p2, p1, p3, GRAY_2);
            DrawLineEx(p1, p2, 3, GRAY_8);
            DrawLineEx(p2, p3, 3, GRAY_8);
            DrawLineEx(p3, p1, 3, GRAY_8);
        }

        rate = EaseElasticOut(Min(t, 0.25) * 4, 0.8);
        DrawCircleFilledOutline(p1, 8 * rate * scale, GRAY_2, GRAY_8);
        rate = EaseElasticOut((Clamp(t, 0.07, 0.35) - 0.07) * 4, 0.8);
        DrawCircleFilledOutline(p2, 8 * rate * scale, GRAY_2, GRAY_8);
        rate = EaseElasticOut((Clamp(t, 0.12, 0.35) - 0.12) * 4, 0.8);
        DrawCircleFilledOutline(p3, 8 * rate * scale, GRAY_2, GRAY_8);

        if (t >= 3) {
            DrawMainScreen();
            if (t < 3.25) {
                rlglDraw();
                DrawRectangle(0, 0, SCR_W, SCR_H, Fade(GRAY_2, (3.25 - t) * 4));
            }
        }
    }
}

static inline void DrawStartupScreen()
{
    ClearBackground(LIME_1);

    float t = GetTime() + 5.5;
    if (t < 0.3) return;
    t -= 0.3;

    float rate = EaseCubicOut((Clamp(t, 1.7, 2.2) - 1.7) * 2);

    Vector2 sz = MeasureTextEx(font, REPO_TITLE, 64, 0);
    DrawTextEx(font, REPO_TITLE,
        (Vector2){(SCR_W - sz.x) / 2, SCR_H * (0.52 + rate * 0.02)},
        64, 0, Fade(GRAY_8, rate));

    DrawIcon((Vector2){SCR_W / 2, SCR_H * (0.58 - rate * 0.05)}, SCR_W / 900.0, t);
}

int main(int argc, char *argv[])
{
    PalmTreeSetup();
    InitGraph(SCR_W * 0.35, SCR_H * 0.5, SCR_W * 0.35, SCR_H * 0.5);
    VerletResetRate();

    PanelSetDimensions(SCR_W * 0.7, 0, SCR_W * 0.3, SCR_H);

    while (!WindowShouldClose()) {
        BeginDrawing();
            if (GetTime() + 5.5 <= 6)
                DrawStartupScreen();
            else
                DrawMainScreen();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
