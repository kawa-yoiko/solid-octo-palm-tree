#include "graph_op.h"
#include "../graph_algorithm/graph.h"

extern "C" {
#include "global.h"
}

#include "rlgl.h"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <utility>
#include <vector>

static int n, m;
static Graph g;

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

#include "barnes_hut.hh"

void InitGraph(int x, int y, int hw, int hh)
{
    FILE *f = fopen("../crawler/cavestory-processed.txt", "r");
    //FILE *f = fopen("graph.txt", "r");
    if (!f) return;

    g.edge.clear();
    vert.clear();
    ::x = x;
    ::y = y;
    ::hw = hw;
    ::hh = hh;

    fscanf(f, "%d%d", &n, &m);
    int _n = n;
    n = 300;
    g.edge.resize(n);
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

    for (int i = 0, u, v, w = 1; i < m; i++) {
        //fscanf(f, "%d%d%d", &u, &v, &w);
        fscanf(f, "%d%d", &u, &v);
        if (u >= n || v >= n) continue;
        g.edge[u].push_back(Graph::Edge(v, 0));
        vert[u].deg++;
        vert[v].deg++;
    }

    fclose(f);

    // Normalize
    for (int u = 0; u < n; u++)
        for (auto &e : g.edge[u])
            e.w = g.edge[u].size();

    g.compute();
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
    for (int u = 0; u < n; u++)
        for (const auto &e : g.edge[u]) {
            int v = e.v;
            float dx = (vert[v].x + vert[v].vx) - (vert[u].x + vert[u].vx);
            float dy = (vert[v].y + vert[v].vy) - (vert[u].y + vert[u].vy);
            if (fabsf(dx) <= 1e-6) dx = jiggle();
            if (fabsf(dy) <= 1e-6) dy = jiggle();
            float l = sqrtf(dx * dx + dy * dy);
            l = (l - 30) / l * alpha * linkStrength(u, v);
            dx *= l;
            dy *= l;
            float b = linkBias(u, v);
            vert[v].vx -= dx * b;
            vert[v].vy -= dy * b;
            vert[u].vx += dx * (1 - b);
            vert[u].vy += dy * (1 - b);
        }

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
static double selTime = 0;
static double releaseTime = -INFINITY;
static std::vector<double> selSSSP;
static const double SEL_FADE_IN_T = 0.15;
static const double SEL_FADE_OUT_T = 0.3;

#if !defined(BARNES_HUT_TEST) && !defined(SIM_TEST)

void VerletDraw()
{
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
                    sqrtf(dsq), 2.5,
                    dsq < hw * hw ? GRAY_4 :
                    Fade(GRAY_4, 2.0 - (float)dsq / (hw * hw)));
            }
        }
    }
    DrawLineStripWithChromaEnd();

    double t = GetTime();
    float radius = 5 * Lerp(scale, 1, 0.875);
    DrawCirclesBegin(radius, 6);
    for (int i = 0; i < n; i++) {
        Vector2 p = (Vector2){
            (float)(x + sx + vert[i].x * scale),
            (float)(y + sy + vert[i].y * scale)
        };
        if (p.x > -radius && p.x < SCR_W + radius &&
            p.y > -radius && p.y < SCR_H + radius)
        {
            float h = 165, s = 0.7, v = 0.6;
            if (selVert >= 0 || t < releaseTime + SEL_FADE_OUT_T) {
                Color c1 = ORANGE_6;
                double lerp = (t < selTime + SEL_FADE_IN_T) ?
                    (t - selTime) / SEL_FADE_IN_T :
                    (t > releaseTime) ?
                    1 - (t - releaseTime) / SEL_FADE_OUT_T : 1;
                //double z = Clamp(4.0 / selSSSP[i], 0, 1);
                double z = selSSSP[i];
                z = 1 - pow(1 - z, 10);
                lerp *= z;
                h = Lerp(h, 30, lerp);
                s = Lerp(s, 0.8, lerp);
                v = Lerp(v, 1, lerp);
            }
            Color c = ColorFromHSV((Vector3){h, s, v});
            DrawCirclesAdd(p, c);
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
        DrawRectangleV(pos, size, Fade(GRAY_4, alpha));
        DrawTextEx(font, vert[hoverID].title, pos, 32, 0, Fade(GRAY_8, alpha));
    }
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
        /*auto sssp = g.d[id];
        selSSSP.clear();
        for (auto p : sssp) selSSSP.push_back(p.first);*/
        selSSSP = g.betweenness;
        //for (int i = 0; i < n; i++)
        //    printf("%d %d %.4f\n", id, i, selSSSP[i]);
        selTime = GetTime();
        releaseTime = INFINITY;
        px0 = vert[id].x - px;
        py0 = vert[id].y - py;
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
        releaseTime = std::max(selTime + SEL_FADE_IN_T, GetTime());
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
