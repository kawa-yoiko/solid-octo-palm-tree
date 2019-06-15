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
    float x, y, vx, vy, ax, ay, ax0, ay0, fx, fy;
    Color c;
    GraphVertex()
      : title(nullptr),
        x(0), y(0), vx(0), vy(0), ax(0), ay(0), ax0(0), ay0(0),
        fx(NAN), fy(NAN),
        c({0, 0, 0, 0}) { }
    ~GraphVertex() { if (title) free(title); }
};

static std::vector<GraphVertex> vert;

#include "barnes_hut.hh"

#ifndef BARNES_HUT_TEST

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
    g.edge.resize(n);
    vert.resize(n);

    char s[1024];
    fgets(s, sizeof s, f);  // Ignore a newline

    int perm[n];
    for (int i = 0; i < n; i++) perm[i] = i;
    for (int i = 1; i < n; i++)
        for (int j = 101; j <= 130; j++) {
            int r = j * (j + 3) + 100000 / j + 1928374655 % (j * j * (j + 24) + 9997);
            std::swap(perm[i], perm[r % (i + 1)]);
        }
    for (int i = 0; i < n; i++) printf("%d%c", perm[i], i == n - 1 ? '\n' : ' ');

    for (int i = 0; i < n; i++) {
        fgets(s, sizeof s, f);
        int len = strlen(s);
        while (len > 0 && isspace(s[len - 1])) len--;
        s[len] = '\0';
        vert[i].title = strdup(s);
        vert[i].x = sin(M_PI * 2 * perm[i] / n) * 400;
        vert[i].y = cos(M_PI * 2 * perm[i] / n) * 400;
        vert[i].c = LIME_3;
    }

    for (int i = 0, u, v, w = 1; i < m; i++) {
        //fscanf(f, "%d%d%d", &u, &v, &w);
        fscanf(f, "%d%d", &u, &v);
        g.edge[u].push_back(Graph::Edge(v, w));
    }

    fclose(f);
}

static float _rate = 1.0;
static int _tick = 120;

void VerletResetRate()
{
    _rate = 1.0;
    _tick = 120;
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

void VerletTick()
{
    if (_rate <= 0.05) return;

    const float dt = 1.f / 60;
    const float ALPHA = 0.5f;
    const float BETA = 300.f;
    const float GAMMA = 0.06f;

    // Integration (1)
    for (int u = 0; u < n; u++) {
        if (isnan(vert[u].fx)) {
            vert[u].x += (vert[u].vx + vert[u].ax / 2 * dt) * dt;
            vert[u].y += (vert[u].vy + vert[u].ay / 2 * dt) * dt;
            vert[u].ax0 = vert[u].ax;
            vert[u].ay0 = vert[u].ay;
            vert[u].ax = vert[u].ay = 0;
        } else {
            vert[u].x = vert[u].fx;
            vert[u].y = vert[u].fy;
            vert[u].ax0 = vert[u].ax = vert[u].vx =
            vert[u].ay0 = vert[u].ay = vert[u].vy = 0;
        }
    }

    // Link force
    for (int u = 0; u < n; u++)
        for (const auto &e : g.edge[u]) {
            int v = e.v;
            float dx = vert[v].x - vert[u].x;
            float dy = vert[v].y - vert[u].y;
            float l = sqrtf(dx * dx + dy * dy);
            if (fabsf(l) <= 1e-6) l = 1e-6;
            float rate = (l - 90) / l;
            dx *= rate * ALPHA;
            dy *= rate * ALPHA;
            float b = 0.5;
            vert[v].ax -= dx * b;
            vert[v].ay -= dy * b;
            vert[u].ax += dx * (1 - b);
            vert[u].ay += dy * (1 - b);
        }

    // Repulsive force
    BarnesHut::Rebuild(n);
    for (int u = 0; u < n; u++) {
        auto f = BarnesHut::Get(vert[u].x, vert[u].y, _rate >= 0.5 ? 0.9 : 2);
        vert[u].ax += f.first * BETA;
        vert[u].ay += f.second * BETA;
    }

    // Radial force
    for (int u = 0; u < n; u++) {
        float l = sqrtf(vert[u].x * vert[u].x + vert[u].y * vert[u].y);
        if (fabsf(l) <= 1e-6) l = 1e-6;
        float ux = vert[u].x / l;
        float uy = vert[u].y / l;
        float diff = (360 - l);
        vert[u].ax += diff * ux * GAMMA;
        vert[u].ay += diff * uy * GAMMA;
    }

    // Integration (2)
    for (int u = 0; u < n; u++) {
        if (isnan(vert[u].fx)) {
            vert[u].vx += (vert[u].ax + vert[u].ax0) / 2 * dt;
            vert[u].vy += (vert[u].ay + vert[u].ay0) / 2 * dt;
            vert[u].vx *= _rate;
            vert[u].vy *= _rate;
            vert[u].ax *= _rate;
            vert[u].ay *= _rate;
        } else {
            vert[u].ax = vert[u].ay = 0;
        }
    }

    if (_tick > 0) _tick--; else _rate *= 0.99;

    // Centre "force"
    for (int u = 0; u < n; u++) {
    }
}

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
