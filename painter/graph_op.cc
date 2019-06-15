#include "graph_op.h"
#include "../graph_algorithm/graph.h"

extern "C" {
#include "global.h"
}

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>

static int n, m;
static Graph g;

static int x, y, hw, hh;

struct GraphVertex {
    char *title;
    float x, y, vx, vy, ax, ay, ax0, ay0;
    Color c;
    GraphVertex()
      : title(nullptr),
        x(0), y(0), vx(0), vy(0), ax(0), ay(0), ax0(0), ay0(0),
        c({0, 0, 0, 0}) { }
    ~GraphVertex() { if (title) free(title); }
};

static std::vector<GraphVertex> vert;

void InitGraph(int x, int y, int hw, int hh)
{
    FILE *f = fopen("../crawler/cavestory-processed.txt", "r");
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
        vert[i].x = sin(M_PI * 2 * i / n) * 400;
        vert[i].y = cos(M_PI * 2 * i / n) * 400;
        vert[i].c = LIME_3;
    }

    for (int i = 0, u, v, w = 1; i < m; i++) {
        //fscanf(f, "%d%d%d", &u, &v, &w);
        fscanf(f, "%d%d", &u, &v);
        g.edge[u].push_back(Graph::Edge(v, w));
    }

    fclose(f);
}

void VerletTick()
{
    const float dt = 1.f / 60;
    const float ALPHA = 1.f / 4;
    const float BETA = 20.f;

    // Integration (1)
    for (int u = 0; u < n; u++) {
        vert[u].x += (vert[u].vx + vert[u].ax / 2 * dt) * dt;
        vert[u].y += (vert[u].vy + vert[u].ay / 2 * dt) * dt;
        vert[u].ax0 = vert[u].ax;
        vert[u].ay0 = vert[u].ay;
        vert[u].ax = vert[u].ay = 0;
    }

    // Link force
    for (int u = 0; u < n; u++)
        for (const auto &e : g.edge[u]) {
            int v = e.v;
            float dx = (vert[v].x + vert[v].vx) - (vert[u].x + vert[u].vx);
            float dy = (vert[v].y + vert[v].vy) - (vert[u].y + vert[u].vy);
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
    for (int u = 0; u < n; u++)
        for (int v = 0; v < n; v++) {
            float dx = (vert[v].x + vert[v].vx) - (vert[u].x + vert[u].vx);
            float dy = (vert[v].y + vert[v].vy) - (vert[u].y + vert[u].vy);
            float dsq = dx * dx + dy * dy;
            float d32 = dsq * sqrtf(dsq);
            if (fabsf(d32) <= 1) d32 = 1;
            dx *= BETA / d32;
            dy *= BETA / d32;
            vert[u].ax -= dx;
            vert[u].ay -= dy;
            vert[v].ax += dx;
            vert[v].ay += dy;
        }

    // Integration (2)
    for (int u = 0; u < n; u++) {
        vert[u].vx += (vert[u].ax + vert[u].ax0) / 2 * dt;
        vert[u].vy += (vert[u].ay + vert[u].ay0) / 2 * dt;
    }

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
