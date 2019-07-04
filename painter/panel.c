#include "panel.h"
#include "global.h"

#include "rlgl.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

// top x/y; total w/h
static int tx = 0, ty = 0, tw = SCR_W, th = SCR_H;

#define Y1  (12)
#define Y2  (12 + th * 0.2)

#define Y_S1    (12 + th * 0.1)
#define W_S1    (tw / 2 - 18)
#define R_S1    10

#define Y_B1    (12 + th * 0.28)
#define H_B1    (th * 0.075)
static const char *T_B1[] = {
    "SSSP",
    "Betw. Centrality",
    "Clos. Centrality",
    "SCC",
    "PageRank"
};
#define N_B1    (sizeof T_B1 / sizeof T_B1[0])

static float slider1 = 0;
static float slider_mx = NAN;

static int button1 = 0;
static int button1_hover = -1;

void PanelSetDimensions(int x, int y, int w, int h)
{
    tx = x;
    ty = y;
    tw = w;
    th = h;
}

static inline int InButton(int x, int y)
{
    if (x >= 12 && x <= tw - 12 && y >= Y_B1 && y < Y_B1 + H_B1 * N_B1) {
        return (y - Y_B1) / H_B1;
    } else {
        return -1;
    }
}

bool PanelMousePress(int x, int y)
{
    x -= tx;
    y -= ty;
    if (x < 0 || x >= tw || y < 0 || y >= th) return false;

    if (Vector2Distance(
        (Vector2){x, y}, (Vector2){tw / 2 + W_S1 * slider1, Y_S1}
    ) <= R_S1) {
        slider_mx = slider1 - (float)(x - tw / 2) / W_S1;
    }

    button1_hover = InButton(x, y);
    if (button1_hover != -1) button1 = button1_hover;

    return true;
}

void PanelMouseMove(int x, int y)
{
    x -= tx;
    y -= ty;

    if (!isnan(slider_mx)) {
        slider1 = slider_mx + (float)(x - tw / 2) / W_S1;
        slider1 = Clamp(slider1, -1, 1);
    }

    button1_hover = InButton(x, y);
}

void PanelMouseRelease()
{
    slider_mx = NAN;
}

float PanelGetSlider1()
{
    return powf(10, slider1);
}

void PanelDraw()
{
    DrawRectangle(tx, ty, tw, th, Fade(GRAY_2, 0.8));
    rlglDraw(); // Workaround for raylib#891

    DrawTextEx(font, "First-paragraph weight",
        (Vector2){tx + 12, ty + Y1},
        36, 0, GRAY_8);

    DrawLineEx(
        (Vector2){tx + tw / 2 - W_S1, ty + Y_S1},
        (Vector2){tx + tw / 2 + W_S1, ty + Y_S1},
        4, GRAY_4);
    DrawCircleV(
        (Vector2){tx + tw / 2 + W_S1 * slider1, ty + Y_S1},
        R_S1, LIME_6);

    char s[8];
    snprintf(s, sizeof s, "%.2f", PanelGetSlider1());
    Vector2 sz = MeasureTextEx(font, s, 24, 0);
    float x = tx + tw / 2 + W_S1 * slider1 - sz.x / 2;
    x = Clamp(x, tx + 12, tx + tw - 12 - sz.x);
    DrawTextEx(font, s,
        (Vector2){x, ty + Y_S1 + 12},
        24, 0, GRAY_8);

    DrawTextEx(font, "Vertex colour",
        (Vector2){tx + 12, ty + Y2},
        36, 0, GRAY_8);
    rlglDraw();

    for (int i = 0; i < N_B1; i++) {
        if (button1 == i) {
            DrawRectangle(
                tx + 12, ty + Y_B1 + H_B1 * i,
                tw - 24, H_B1, Fade(LIME_5, 0.8));
        } else if (button1_hover == i) {
            DrawRectangle(
                tx + 12, ty + Y_B1 + H_B1 * i,
                tw - 24, H_B1, Fade(LIME_6, 0.2));
        }
        DrawLineEx(
            (Vector2){tx + 12, ty + Y_B1 + H_B1 * i},
            (Vector2){tx + tw - 12, ty + Y_B1 + H_B1 * i},
            2, GRAY_4);
        rlglDraw();
        Vector2 sz = MeasureTextEx(font, T_B1[i], 32, 0);
        DrawTextEx(font, T_B1[i], (Vector2){
            tx + tw / 2 - sz.x / 2,
            ty + Y_B1 + H_B1 * (i + 0.5) - sz.y / 2
        }, 32, 0, GRAY_8);
        rlglDraw();
    }
    DrawLineEx(
        (Vector2){tx + 12, ty + Y_B1 + H_B1 * N_B1},
        (Vector2){tx + tw - 12, ty + Y_B1 + H_B1 * N_B1},
        2, GRAY_4);
}
