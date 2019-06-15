namespace BarnesHut {

static const float THETA = 0.9;

// Quadtree node

struct Node {
    int child[4];
    float w;
    float cx, cy, tot;
};

std::vector<Node> t;
int tsz;

std::vector<int> seq;

static inline int Quadrant(float x, float y)
{
    // There is no need to follow methematical conventions
    //return x >= 0 ?
    //    y >= 0 ? 0 : 3 :
    //    y >= 0 ? 1 : 2;
    return ((int)(x >= 0) << 1) | (y >= 0);
}

static inline int Build(int l, int r, float x1, float y1, float x2, float y2)
{
    // Create a new node
    if (tsz == t.size()) t.resize(t.size() * 3 / 2 + 1);
    int u = tsz++;

    t[u].w = x2 - x1;
    t[u].tot = r - l;   // Assumes equal mass for all bodies

    // Node information
    float xtot = 0, ytot = 0;
    for (int i = l; i < r; i++) {
        if (vert[seq[i]].x < x1 || vert[seq[i]].x > x2 ||
            vert[seq[i]].y < y1 || vert[seq[i]].y > y2)
        {
            puts("> <");
        }
        xtot += vert[seq[i]].x;
        ytot += vert[seq[i]].y;
    }

    if (r - l <= 1) {
        for (int i = 0; i < 4; i++) t[u].child[i] = -1;
        t[u].cx = xtot;
        t[u].cy = ytot;
        return u;
    }

    t[u].cx /= (r - l);
    t[u].cy /= (r - l);

    // Partition
    float x0 = (x1 + x2) / 2;
    float y0 = (y1 + y2) / 2;

    int cnt[4] = {0, 0, 0, 0};
    for (int i = l; i < r; i++) {
        int q = Quadrant(vert[seq[i]].x - x0, vert[seq[i]].y - y0);
        cnt[q] += 1;
    }
    for (int i = 0, s = 0; i < 4; i++) {
        int t = cnt[i];
        cnt[i] = s;
        s += t;
    }
    int tmp[r - l];
    for (int i = l; i < r; i++) {
        int q = Quadrant(vert[seq[i]].x - x0, vert[seq[i]].y - y0);
        tmp[cnt[q]++] = seq[i];
    }
    for (int i = l; i < r; i++) seq[i] = tmp[i - l];

    // Recurse
    t[u].child[0] = Build(l, l + cnt[0], x1, y1, x0, y0);
    t[u].child[1] = Build(l + cnt[0], l + cnt[1], x1, y0, x0, y2);
    t[u].child[2] = Build(l + cnt[1], l + cnt[2], x0, y1, x2, y0);
    t[u].child[3] = Build(l + cnt[2], r, x0, y0, x2, y2);

    return u;
}

static inline void Rebuild(int n)
{
    seq.resize(n);
    for (int i = 0; i < n; i++) seq[i] = i;

    float x1 = INFINITY, y1 = INFINITY;
    float x2 = -INFINITY, y2 = -INFINITY;
    for (int i = 0; i < n; i++) {
        x1 = std::min(x1, vert[i].x);
        y1 = std::min(y1, vert[i].y);
        x2 = std::max(x2, vert[i].x);
        y2 = std::max(y2, vert[i].y);
    }

    t.resize(16);
    tsz = 0;
    Build(0, n, x1, y1, x2, y2);
}

float qx, qy;

static inline std::pair<float, float> Traverse(int u)
{
    float dsq =
        (qx - t[u].cx) * (qx - t[u].cx) +
        (qy - t[u].cy) * (qy - t[u].cy);
    if (fabsf(dsq) <= 1e-6) return {0, 0};
    if (t[u].w * t[u].w / dsq < THETA * THETA || t[u].child[0] == -1) {
        // Single body w.r.t. the query point
        float d32 = dsq * sqrtf(dsq);
        float unit = t[u].tot / d32;
        return {
            unit * (qx - t[u].cx),
            unit * (qy - t[u].cy)
        };
    }

    // Recurse otherwise
    std::pair<float, float> result(0, 0);
    for (int i = 0; i < 4; i++) {
        auto f = Traverse(t[u].child[i]);
        result.first += f.first;
        result.second += f.second;
    }
    return result;
}

static inline std::pair<float, float> Get(float x, float y)
{
    qx = x; qy = y;
    return Traverse(0);
}

}

// g++ graph_op.cc -Og -g -std=c++11 -DBARNES_HUT_TEST
#ifdef BARNES_HUT_TEST

int main()
{
    srand(0);
    int n = 20;
    vert.resize(n);
    for (int i = 0; i < n; i++) {
        vert[i].x = rand() % 1001 - 500;
        vert[i].y = rand() % 1001 - 500;
        printf("(%.0f, %.0f)\n", vert[i].x, vert[i].y);
    }

    BarnesHut::Rebuild(n);

    for (int u = 0; u < n; u++) {
        float fx = 0, fy = 0;
        for (int v = 0; v < n; v++) if (u != v) {
            float dx = vert[v].x - vert[u].x;
            float dy = vert[v].y - vert[u].y;
            float dsq = dx * dx + dy * dy;
            float d32 = dsq * sqrtf(dsq);
            fx -= dx / d32;
            fy -= dy / d32;
        }
        printf("%.6f %.6f\n", fx, fy);

        auto approx = BarnesHut::Get(vert[u].x, vert[u].y);
        printf("%.6f %.6f\n", approx.first, approx.second);
    }

    return 0;
}

#endif
