#include "graph_op.h"
#include "../graph_algorithm/graph.h"
#include "global.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

static int n, m;
static Graph g;

static int x, y, hw, hh;

struct GraphVertex {
    char *title;
    double x, y, vx, vy;
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
        vertices[i].x = i * 3;
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
}

void VerletDraw()
{
    DrawRectangle(x - hw, y - hh, hw * 2, hh * 2, Fade(GRAY_3, 0.8));

    for (int i = 0; i < n; i++) {
        DrawCircle(x + vertices[i].x, y + vertices[i].y, 5, LIME_8);
    }
}
