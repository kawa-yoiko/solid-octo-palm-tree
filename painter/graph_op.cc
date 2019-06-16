#include "graph_op.h"
#include "../graph_algorithm/graph.h"

extern "C" {
#include "global.h"
}

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <utility>

static int n, m;
static Graph g;

static int x, y, hw, hh;

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
    //FILE *f = fopen("../crawler/cavestory-processed.txt", "r");
    FILE *f = fopen("graph.txt", "r");
    if (!f) return;

    g.edge.clear();
    vert.clear();
    ::x = x;
    ::y = y;
    ::hw = hw;
    ::hh = hh;

    fscanf(f, "%d%d", &n, &m);
    g.edge.resize(n);
    vert.resize(n);

    char s[1024];
    fgets(s, sizeof s, f);  // Ignore a newline

    for (int i = 0; i < n; i++) {
        fgets(s, sizeof s, f);
        int len = strlen(s);
        while (len > 0 && isspace(s[len - 1])) len--;
        s[len] = '\0';
        vert[i].title = strdup(s);
        vert[i].x = cos(M_PI * 2 * i / n) * 50;
        vert[i].y = sin(M_PI * 2 * i / n) * 50;
        vert[i].c = LIME_3;
    }

    for (int i = 0, u, v, w = 1; i < m; i++) {
        //fscanf(f, "%d%d%d", &u, &v, &w);
        fscanf(f, "%d%d", &u, &v);
        g.edge[u].push_back(Graph::Edge(v, w));
        vert[u].deg++;
        vert[v].deg++;
    }

    fclose(f);
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

    // Integration
    for (int u = 0; u < n; u++) {
        if (isnan(vert[u].fx)) {
            vert[u].vx *= velocityDecay;
            vert[u].vy *= velocityDecay;
            vert[u].x += vert[u].vx;
            vert[u].y += vert[u].vy;
        } else {
            vert[u].vx = vert[u].vy = 0;
        }
    }
}

#if !defined(BARNES_HUT_TEST) && !defined(SIM_TEST)

void VerletDraw()
{
    DrawRectangle(x - hw, y - hh, hw * 2, hh * 2, Fade(GRAY_3, 0.8));

    DrawLineStripWithChromaBegin();
    for (int i = 0; i < n; i++) {
        Vector2 p = (Vector2){
            (float)(x + vert[i].x),
            (float)(y + vert[i].y)
        };
        for (const auto &e : g.edge[i]) {
            Vector2 q = (Vector2){
                (float)(x + vert[e.v].x),
                (float)(y + vert[e.v].y)
            };
            int dsq = (p.x - q.x) * (p.x - q.x) + (p.y - q.y) * (p.y - q.y);
            if (dsq < hw * hw / 2) {
                DrawLineStripWithChromaAdd(p, q,
                    sqrtf(dsq), 2.5,
                    dsq < hw * hw / 4 ? GRAY_6 :
                    Fade(GRAY_6, 2.0 - (float)dsq / (hw * hw / 4)));
            }
        }
    }
    DrawLineStripWithChromaEnd();

    DrawCirclesBegin(5, 6);
    for (int i = 0; i < n; i++) {
        DrawCirclesAdd((Vector2){
            (float)(x + vert[i].x),
            (float)(y + vert[i].y)
        }, LIME_8);
    }
    DrawCirclesEnd();
}

#endif

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
