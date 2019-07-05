#include "graph_op.h"
#include "../graph_algorithm/graph.h"

extern "C" {
#include "global.h"
#include "panel.h"
}

#include "raymath.h"
#include "rlgl.h"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <thread>
#include <utility>
#include <vector>

static int n, m;
static std::vector<std::vector<int>> outFirst, outOther;

static Graph g;

static std::mutex gMutex;
static std::atomic<bool> gStarted(false), gSignal(false), gRedraw(false);

static int x, y, hw, hh;
static float scale = 1;
static float sx = 0, sy = 0;

struct GraphVertex {
    char *title;
    int deg;
    float x, y, vx, vy, fx, fy;
    Color c;
    GraphVertex()
      : title(nullptr),
        deg(0),
        x(0), y(0), vx(0), vy(0), fx(NAN), fy(NAN),
        c({0, 0, 0, 0}) { }
    ~GraphVertex() { if (title) free(title); }
};

static std::vector<GraphVertex> vert;

static int mode = 0;
static const int MODE_SSSP = 0;
static const int MODE_BC = 1;
static const int MODE_CC = 2;
static const int MODE_SCC = 3;
static const int MODE_PR = 4;
static float firstParSlider = 0;
static float firstParWeight = 1;
static int iconOpacity = 0;
static const int ICON_OPACITY_MAX = 64;
static const int ICON_OPACITY_INC = 2;

#include "barnes_hut.hh"

static std::vector<Color> fromColour, targetColour;
static double colourTransitionStart = -1, colourTransitionDur = 1;

static inline Color GetColour(int id)
{
    double rate = Clamp(
        (GetTime() - colourTransitionStart) / colourTransitionDur,
        0, 1);
    Color c1 = fromColour[id];
    Color c2 = targetColour[id];
    return (Color){
        (unsigned char)Lerp(c1.r, c2.r, rate),
        (unsigned char)Lerp(c1.g, c2.g, rate),
        (unsigned char)Lerp(c1.b, c2.b, rate),
        255
    };
}

static inline void RegisterColourTransition(double dur)
{
    for (int i = 0; i < n; i++) fromColour[i] = GetColour(i);
    colourTransitionStart = GetTime();
    colourTransitionDur = dur;
}

static inline Graph BuildGraph()
{
    Graph g;
    g.edge.resize(n);
    for (int u = 0; u < n; u++) {
        int p = outFirst[u].size(),
            q = outOther[u].size();
        float w = p * firstParWeight + q;
        for (int v : outFirst[u])
            g.edge[u].push_back({(unsigned int)v, w / firstParWeight});
        for (int v : outOther[u])
            g.edge[u].push_back({(unsigned int)v, w});
    }
    g.compute();
    return g;
}

void InitGraph(const char *dataset, int x, int y, int hw, int hh)
{
    char path[64];
    snprintf(path, sizeof path, "%s-processed.txt", dataset);

    FILE *f = fopen(path, "r");
    if (!f) return;

    g.edge.clear();
    vert.clear();
    ::x = x;
    ::y = y;
    ::hw = hw;
    ::hh = hh;

    fscanf(f, "%d%d", &n, &m);
    int _n = n;
    outFirst.resize(n);
    outOther.resize(n);
    vert.resize(n);

    char s[1024];
    fgets(s, sizeof s, f);  // Ignore a newline

    const float radius = 10;
    const float angle = M_PI * (3 - sqrtf(5));

    for (int i = 0; i < _n; i++) {
        fgets(s, sizeof s, f);
        int len = strlen(s);
        while (len > 0 && isspace(s[len - 1])) len--;
        s[len] = '\0';
        if (i >= n) continue;
        vert[i].title = strdup(s);
        vert[i].x = cos(angle * i) * radius * sqrtf(i);
        vert[i].y = sin(angle * i) * radius * sqrtf(i);
        vert[i].c = LIME_3;
    }

    for (int i = 0, u, v, w; i < m; i++) {
        fscanf(f, "%d%d%d", &u, &v, &w);
        if (u >= n || v >= n) continue;
        vert[u].deg++;
        vert[v].deg++;
        (w ? outFirst : outOther)[u].push_back(v);
    }

    fclose(f);

    Graph g1 = BuildGraph();
    g = g1;

    fromColour.resize(n);
    targetColour.resize(n);
    for (int i = 0; i < n; i++)
        targetColour[i] = ColorFromHSV((Vector3){190, 0, 1});
    RegisterColourTransition(0.5);
    for (int i = 0; i < n; i++)
        targetColour[i] = ColorFromHSV((Vector3){165, 0.7, 0.6});
}

static float alpha, alphaMin, alphaDecay, alphaTarget;
static float velocityDecay;

void VerletResetRate()
{
    alpha = 1.0;
    alphaMin = 0.001;
    alphaDecay = 1 - powf(alphaMin, 1.0 / 300);
    alphaTarget = 0;
    velocityDecay = 0.6;
}

void VerletSetFix(int id, float x, float y)
{
    vert[id].fx = x;
    vert[id].fy = y;
}

void VerletCancelFix(int id)
{
    VerletSetFix(id, NAN, NAN);
}

static inline float jiggle()
{
    return (float)rand() / RAND_MAX * 2e-6 - 1e-6;
}

static inline float linkStrength(int u, int v)
{
    return 1.f / std::min(vert[u].deg, vert[v].deg);
}

static inline float linkBias(int u, int v)
{
    return (float)vert[u].deg / (vert[u].deg + vert[v].deg);
}

void VerletTick()
{
    if (alpha < 1e-2) return;
    alpha += (alphaTarget - alpha) * alphaDecay;

    // Link force
    gMutex.lock();
    for (int u = 0; u < n; u++)
        for (const auto &e : g.edge[u]) {
            int v = e.v;
            float dx = (vert[v].x + vert[v].vx) - (vert[u].x + vert[u].vx);
            float dy = (vert[v].y + vert[v].vy) - (vert[u].y + vert[u].vy);
            if (fabsf(dx) <= 1e-6) dx = jiggle();
            if (fabsf(dy) <= 1e-6) dy = jiggle();
            float l = sqrtf(dx * dx + dy * dy);
            l = (l - 45) / l * alpha * linkStrength(u, v) / e.w;
            dx *= l;
            dy *= l;
            float b = linkBias(u, v);
            vert[v].vx -= dx * b;
            vert[v].vy -= dy * b;
            vert[u].vx += dx * (1 - b);
            vert[u].vy += dy * (1 - b);
        }
    gMutex.unlock();

    // Repulsive force
    /*for (int u = 0; u < n; u++)
        for (int v = u + 1; v < n; v++) {
            float dx = vert[v].x - vert[u].x;
            float dy = vert[v].y - vert[u].y;
            if (fabsf(dx) <= 1e-6) dx = jiggle();
            if (fabsf(dy) <= 1e-6) dy = jiggle();
            float l = sqrtf(dx * dx + dy * dy);
            dx /= l;
            dy /= l;
            if (l <= 5) l = 5;
            vert[u].vx -= dx / (l * l) * alpha * 30;
            vert[u].vy -= dy / (l * l) * alpha * 30;
            vert[v].vx += dx / (l * l) * alpha * 30;
            vert[v].vy += dy / (l * l) * alpha * 30;
        }*/
    float theta = 0.7 + (1 - alpha) * 0.5;
    BarnesHut::Rebuild(n);
    for (int u = 0; u < n; u++) {
        auto f = BarnesHut::Get(vert[u].x, vert[u].y, theta);
        vert[u].vx += f.first * alpha * 30;
        vert[u].vy += f.second * alpha * 30;
    }

    // Integration
    for (int u = 0; u < n; u++) {
        if (isnan(vert[u].fx)) {
            vert[u].vx *= velocityDecay;
            vert[u].vy *= velocityDecay;
            vert[u].x += vert[u].vx;
            vert[u].y += vert[u].vy;
        } else {
            vert[u].x = vert[u].fx;
            vert[u].y = vert[u].fy;
            vert[u].vx = vert[u].vy = 0;
        }
    }

    // Keep CM at the centre
    float sx = 0, sy = 0;
    for (int u = 0; u < n; u++) {
        sx += vert[u].x;
        sy += vert[u].y;
    }
    sx /= n;
    sy /= n;
    for (int u = 0; u < n; u++) {
        vert[u].x -= sx;
        vert[u].y -= sy;
    }
}

static int hoverID = -1;
static double hoverTime = 0;
static double unhoverTime = -INFINITY;
static double lastHoverAlpha = 0;
static float lastHoverX = NAN, lastHoverY = NAN;
static Vector2 lastHoverSize;
static const double HOVER_FADE_IN_T = 0.2;
static const double HOVER_FADE_OUT_T = 0.4;

static int selVert = -1, px0, py0;
static const double SEL_FADE_IN_T = 0.15;
static const double SEL_FADE_OUT_T = 0.3;

static const double SWITCH_FADE_T = 0.15;

#if !defined(BARNES_HUT_TEST) && !defined(SIM_TEST)

void VerletDraw()
{
    float newSlider = PanelGetSlider();
    if (newSlider != firstParSlider) {
        firstParSlider = newSlider;
        firstParWeight = pow(10, firstParSlider);
        if (!gStarted.load()) {
            gStarted.store(true);
            gSignal.store(true);
            auto mutex = &gMutex;
            auto started = &gStarted;
            auto signal = &gSignal;
            auto redraw = &gRedraw;
            std::thread t = std::thread([mutex, started, signal, redraw] {
                do {
                    signal->store(false);
                    Graph g1 = BuildGraph();
                    mutex->lock();
                    g = g1;
                    mutex->unlock();
                    redraw->store(true);
                } while (signal->load());
                started->store(false);
            });
            t.detach();
        }
    }

    gMutex.lock();
    int newMode = PanelGetMode();
    if (newMode != mode || gRedraw.load()) {
        gRedraw.store(false);
        mode = newMode;
        RegisterColourTransition(SWITCH_FADE_T);
        switch (mode) {
        case MODE_SSSP:
            for (int i = 0; i < n; i++)
                targetColour[i] = ColorFromHSV((Vector3){165, 0.7, 0.6});
            break;
        case MODE_BC:
        case MODE_CC:
        case MODE_PR: {
            std::vector<double> &val =
                mode == MODE_BC ? g.betweenness :
                mode == MODE_CC ? g.closeness :
                g.pagerank;
            double exp =
                mode == MODE_BC ? 8 :
                mode == MODE_CC ? 1 : 3;
            for (int i = 0; i < n; i++)
                targetColour[i] = ColorFromHSV(Vector3Lerp(
                    (Vector3){165, 0.7, 0.6},
                    (Vector3){ 30, 0.8, 1.0},
                    1 - pow(1 - val[i], exp)));
            break;
        }
        case MODE_SCC:
            for (int i = 0; i < n; i++)
                targetColour[i] = ColorFromHSV((Vector3){
                    360.0f / g.color_count * g.color[i], 0.6, 0.95
                });
            break;
        }
    }

    DrawRectangle(x - hw, y - hh, hw * 2, hh * 2, Fade(GRAY_3, 0.8));

    DrawLineStripWithChromaBegin();
    for (int i = 0; i < n; i++) {
        Vector2 p = (Vector2){
            (float)(x + sx + vert[i].x * scale),
            (float)(y + sy + vert[i].y * scale)
        };
        for (const auto &e : g.edge[i]) {
            Vector2 q = (Vector2){
                (float)(x + sx + vert[e.v].x * scale),
                (float)(y + sy + vert[e.v].y * scale)
            };
            int dsq = (p.x - q.x) * (p.x - q.x) + (p.y - q.y) * (p.y - q.y);
            dsq *= scale;   // not scale * scale because we'd like to show more
            if (dsq < hw * hw * 2) {
                DrawLineStripWithChromaAdd(p, q,
                    sqrtf(dsq), 3,
                    dsq < hw * hw ? GRAY_4 :
                    Fade(GRAY_4, 2.0 - (float)dsq / (hw * hw)));
            }
        }
    }
    DrawLineStripWithChromaEnd();
    gMutex.unlock();

    double t = GetTime();
    float radius = 5 * Lerp(scale, 1, 0.875);
    DrawCirclesBegin(radius, 12);
    for (int i = 0; i < n; i++) {
        Vector2 p = (Vector2){
            (float)(x + sx + vert[i].x * scale),
            (float)(y + sy + vert[i].y * scale)
        };
        if (p.x > -radius && p.x < SCR_W + radius &&
            p.y > -radius && p.y < SCR_H + radius)
        {
            DrawCirclesAdd(p, GetColour(i));
        }
    }
    DrawCirclesEnd();

    // Tooltip
    rlglDraw();
    if (hoverID != -1 && t < unhoverTime + HOVER_FADE_OUT_T) {
        float alpha = (t < hoverTime + HOVER_FADE_IN_T) ?
            Lerp(lastHoverAlpha, 1, (t - hoverTime) / HOVER_FADE_IN_T) :
            (t > unhoverTime) ? (1 - (t - unhoverTime) / HOVER_FADE_OUT_T) : 1;
        Vector2 size = MeasureTextEx(font, vert[hoverID].title, 32, 0);
        Vector2 pos = (Vector2){
            x + sx + vert[hoverID].x * scale,
            y + sy + vert[hoverID].y * scale
        };
        if (t < hoverTime + HOVER_FADE_IN_T && !isnan(lastHoverX)) {
            size = Vector2Lerp(
                lastHoverSize,
                size, EaseExpOut((t - hoverTime) / HOVER_FADE_IN_T));
            pos = Vector2Lerp(
                (Vector2){lastHoverX, lastHoverY},
                pos, EaseExpOut((t - hoverTime) / HOVER_FADE_IN_T));
        }
        DrawRectangleV(pos,
            (Vector2){size.x + 8, size.y + 2},
            Fade(GRAY_4, alpha));
        DrawTextEx(font, vert[hoverID].title,
            (Vector2){pos.x + 4, pos.y + 1},
            32, 0, Fade(GRAY_8, alpha));
    }

    DrawIcon((Vector2){SCR_W * 0.92, SCR_H + 3}, SCR_W / 1200.0, GetTime() * 2 + 200);
}

#endif

static inline void FindNearest(int px, int py, int &id, float &dsq)
{
    id = -1;
    dsq = INFINITY;
    for (int u = 0; u < n; u++) {
        float cur = (px - vert[u].x) * (px - vert[u].x)
            + (py - vert[u].y) * (py - vert[u].y);
        if (dsq > cur) {
            dsq = cur;
            id = u;
        }
    }
}

void VerletMousePress(int px, int py)
{
    px = (px - ::x - sx) / scale;
    py = (py - ::y - sy) / scale;
    int id;
    float nearest;
    FindNearest(px, py, id, nearest);
    if (nearest <= 10 * 10) {
        selVert = id;
        if (mode == MODE_SSSP) {
            RegisterColourTransition(SEL_FADE_IN_T);
            gMutex.lock();
            for (int i = 0; i < n; i++) {
                double z = Clamp(4 / g.d[selVert][i].first, 0, 1);
                targetColour[i] = ColorFromHSV(Vector3Lerp(
                    (Vector3){165, 0.7, 0.6},
                    (Vector3){ 30, 0.8, 1.0},
                    1 - pow(1 - z, 10)
                ));
            }
            gMutex.unlock();
        }
        px0 = vert[id].x - px;
        py0 = vert[id].y - py;
        VerletResetRate();
    } else {
        selVert = -2;
        px0 = px;
        py0 = py;
    }
}

void VerletMouseMove(int px, int py)
{
    int px1 = px, py1 = py;
    px = (px - ::x - sx) / scale;
    py = (py - ::y - sy) / scale;

    if (selVert >= 0) {
        VerletSetFix(selVert, px + px0, py + py0);
        VerletResetRate();
    } else if (selVert == -2) {
        float dx = (px - px0) * scale;
        float dy = (py - py0) * scale;
        sx += dx;
        sy += dy;
        px = (px1 - ::x - sx) / scale;
        py = (py1 - ::y - sy) / scale;
        px0 = px;
        py0 = py;
    }

    int id;
    float nearest;
    FindNearest(px, py, id, nearest);
    double t = GetTime();
    if (nearest <= 10 * 10) {
        if (hoverID != id || !isinf(unhoverTime)) {
            lastHoverAlpha = Clamp(1 - (t - unhoverTime) / HOVER_FADE_OUT_T, 0, 1);
            if (hoverID >= 0 && lastHoverAlpha > 1e-6) {
                lastHoverX = ::x + sx + vert[hoverID].x * scale;
                lastHoverY = ::y + sy + vert[hoverID].y * scale;
                lastHoverSize = MeasureTextEx(font, vert[hoverID].title, 32, 0);
            } else {
                lastHoverX = NAN;
                lastHoverY = NAN;
            }
            hoverID = id;
            hoverTime = GetTime();
        }
        unhoverTime = INFINITY;
    } else if (isinf(unhoverTime)) {
        unhoverTime = std::max(hoverTime + HOVER_FADE_IN_T, t);
    }
}

void VerletMouseRelease()
{
    if (selVert >= 0) {
        VerletCancelFix(selVert);
        if (mode == MODE_SSSP) {
            RegisterColourTransition(SEL_FADE_IN_T);
            for (int i = 0; i < n; i++) {
                targetColour[i] = ColorFromHSV((Vector3){165, 0.7, 0.6});
            }
        }
    }
    selVert = -1;
}

void VerletChangeScale(int wheel, int px, int py)
{
    px = (px - ::x - sx) / scale;
    py = (py - ::y - sy) / scale;
    float newScale = Clamp(scale + (float)wheel / 16, 1, 8);
    // ::x + sx + px * scale == ::x + sx' + px * scale'
    sx = sx + px * (scale - newScale);
    sy = sy + py * (scale - newScale);
    scale = newScale;
}

// g++ graph_op.cc -Og -g -std=c++11 -DSIM_TEST
#ifdef SIM_TEST
int main()
{
    InitGraph(100, 100, 100, 100);
    VerletResetRate();
    for (int i = 0; i < 10; i++) {
        VerletTick();
        printf("\n== %d ==\n", i);
        for (int j = 0; j < n; j++)
            printf("%.4f %.4f %.4f %.4f\n",
                vert[j].x, vert[j].y, vert[j].vx, vert[j].vy);
    }
}
#endif
