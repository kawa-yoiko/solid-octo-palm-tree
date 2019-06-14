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
    float x, y, vx, vy;
    Color c;
    GraphVertex()
      : title(nullptr), x(0), y(0), vx(0), vy(0), c({0, 0, 0, 0}) { }
    ~GraphVertex() { if (title) free(title); }
};

static std::vector<GraphVertex> vertices;

void InitGraph(int x, int y, int hw, int hh)
{
    FILE *f = fopen("../crawler/cavestory-processed.txt", "r");
    if (!f) return;

    g.edge.clear();
    vertices.clear();
    ::x = x;
    ::y = y;
    ::hw = hw;
    ::hh = hh;

    fscanf(f, "%d%d", &n, &m);
    g.edge.resize(n);
    vertices.resize(n);

    char s[1024];
    fgets(s, sizeof s, f);  // Ignore a newline

    for (int i = 0; i < n; i++) {
        fgets(s, sizeof s, f);
        int len = strlen(s);
        while (len > 0 && isspace(s[len - 1])) len--;
        s[len] = '\0';
        vertices[i].title = strdup(s);
        vertices[i].x = sin(M_PI * 2 * i / n) * 400;
        vertices[i].y = cos(M_PI * 2 * i / n) * 400;
        vertices[i].c = LIME_3;
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
    const float ALPHA = 1.f;

    // Link force
    for (int u = 0; u < n; u++)
        for (const auto &e : g.edge[u]) {
            int v = e.v;
            float dx = (vertices[v].x + vertices[v].vx) - (vertices[u].x + vertices[u].vx);
            float dy = (vertices[v].y + vertices[v].vy) - (vertices[u].y + vertices[u].vy);
            float l = sqrtf(dx * dx + dy * dy);
            if (fabsf(l) <= 1e-6) l = 1e-6;
            //l = (l - 30) / l * ALPHA * 1;
            l -= 180;
            if (l <= 5) continue;
            l = 1.0 / l;
            dx *= l;
            dy *= l;
            float b = 0.5;
            vertices[v].vx -= dx * b;
            vertices[v].vy -= dy * b;
            vertices[u].vx += dx * (1 - b);
            vertices[u].vy += dy * (1 - b);
        }

    // Repulsive force

    for (int u = 0; u < n; u++) {
        vertices[u].x += vertices[u].vx * dt;
        vertices[u].y += vertices[u].vy * dt;
    }
}

void VerletDraw()
{
    DrawRectangle(x - hw, y - hh, hw * 2, hh * 2, Fade(GRAY_3, 0.8));

    DrawLineStripWithChromaBegin();
    for (int i = 0; i < n; i++) {
        Vector2 p = (Vector2){
            (float)(x + vertices[i].x),
            (float)(y + vertices[i].y)
        };
        for (const auto &e : g.edge[i]) {
            Vector2 q = (Vector2){
                (float)(x + vertices[e.v].x),
                (float)(y + vertices[e.v].y)
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
            (float)(x + vertices[i].x),
            (float)(y + vertices[i].y)
        }, LIME_8);
    }
    DrawCirclesEnd();
}