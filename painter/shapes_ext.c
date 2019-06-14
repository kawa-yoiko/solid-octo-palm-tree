#include "rlgl.h"
#include <stdbool.h>
#include <stdlib.h>

void GenerateAnchoredPoly(
    Vector2 **outPoints,
    const Vector2 *points, int numPoints,
    Vector2 offset, Vector2 anchor, float scale)
{
    Vector2 *_outPoints = *outPoints;
    if (_outPoints == NULL)
        _outPoints = (Vector2 *)malloc(sizeof(Vector2) * numPoints);

    for (int i = 0; i < numPoints; i++) {
        _outPoints[i].x = offset.x + (points[i].x - anchor.x) * scale;
        _outPoints[i].y = offset.y + (points[i].y - anchor.y) * scale;
    }

    *outPoints = _outPoints;
}

static inline Vector2 bezierInterpolate(
    const Vector2 A, const Vector2 B, const Vector2 C, const Vector2 D, float t)
{
#define interpolate(__u, __v) (Vector2){ \
    __u.x * (1 - t) + __v.x * t, \
    __u.y * (1 - t) + __v.y * t \
}
    Vector2 AB = interpolate(A, B);
    Vector2 BC = interpolate(B, C);
    Vector2 CD = interpolate(C, D);
    Vector2 ABC = interpolate(AB, BC);
    Vector2 BCD = interpolate(BC, CD);
    Vector2 ABCD = interpolate(ABC, BCD);
    return ABCD;
#undef interpolate
}

// Points:
// [End1] [Ctrl1A] [Ctrl1B] [End2] [Ctrl2A] [Ctrl2B] [End3]
void GenerateAnchoredBezier(
    Vector2 **outPoints,
    const Vector2 *points, int numSegs, int numDivs,
    Vector2 offset, Vector2 anchor,
    float scaleX, float scaleY, float rotation)
{
    Vector2 *_outPoints = *outPoints;
    if (_outPoints == NULL)
        _outPoints = (Vector2 *)malloc(sizeof(Vector2) * (numSegs * numDivs + 1));

    for (int i = 0; i < numSegs; i++)
        for (int j = 0; j < numDivs + (i == numSegs - 1 ? 1 : 0); j++) {
            int idx = i * numDivs + j;
            Vector2 p = bezierInterpolate(
                points[i * 3], points[i * 3 + 1],
                points[i * 3 + 2], points[i * 3 + 3], (float)j / numDivs);
            float x1 = p.x - anchor.x, x2;
            float y1 = p.y - anchor.y;
            if (rotation != 0) {
                x2 = x1 * cos(rotation) - y1 * sin(rotation);
                y1 = x1 * sin(rotation) + y1 * cos(rotation);
                x1 = x2;
            }
            _outPoints[idx].x = offset.x + x1 * scaleX;
            _outPoints[idx].y = offset.y + y1 * scaleY;
        }

    *outPoints = _outPoints;
}

void DrawPolyFilledConvex(const Vector2 *points, int numPoints, Color color)
{
    if (numPoints < 3) return;
    rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        for (int i = 2; i < numPoints; i++) {
            rlVertex2f(points[0].x, points[0].y);
            rlVertex2f(points[i - 1].x, points[i - 1].y);
            rlVertex2f(points[i].x, points[i].y);
        }
    rlEnd();
}

static inline float cross(const Vector2 A, const Vector2 B, const Vector2 C)
{
    return (B.x - A.x) * (C.y - A.y) - (C.x - A.x) * (B.y - A.y);
}

static inline void rlAddTriangle(Vector2 A, Vector2 B, Vector2 C)
{
    if (cross(A, B, C) <= 0) {
        rlVertex2f(A.x, A.y);
        rlVertex2f(B.x, B.y);
        rlVertex2f(C.x, C.y);
    } else {
        rlVertex2f(B.x, B.y);
        rlVertex2f(A.x, A.y);
        rlVertex2f(C.x, C.y);
    }
}

struct CyclicList {
    int cap;
    struct CyclicListNode {
        struct CyclicListNode *next, *prev;
    } *nodes, *head;
};

static struct CyclicList
    poly = { 0 }, convex = { 0 }, ear = { 0 }, reflex = { 0 };

static void CyclicListInit(struct CyclicList *ls, int sz)
{
    if (ls->cap < sz) {
        ls->nodes = (struct CyclicListNode *)realloc(
            ls->nodes, sizeof(struct CyclicListNode) * (sz + 1));
        ls->cap = sz;
    }
    ls->head = &ls->nodes[sz];
    ls->head->next = ls->head->prev = ls->head;
    // next == NULL marks a node not in the list
    for (int i = 0; i < sz; ++i) ls->nodes[i].next = NULL;
}

static void CyclicListInsert(struct CyclicList *ls, int idx)
{
    ls->nodes[idx].next = ls->head->next;
    ls->nodes[idx].prev = ls->head;
    ls->head->next->prev = &ls->nodes[idx];
    ls->head->next = &ls->nodes[idx];
}

static bool CyclicListContains(const struct CyclicList *ls, int idx)
{
    return (ls->nodes[idx].next != NULL);
}

static void CyclicListRemove(struct CyclicList *ls, int idx)
{
    ls->nodes[idx].next->prev = ls->nodes[idx].prev;
    ls->nodes[idx].prev->next = ls->nodes[idx].next;
    ls->nodes[idx].next = NULL;
}

#define CyclicListEach(__ls, __var) \
    (struct CyclicListNode *__node = (__ls).head->next; \
    ((__var) = __node - (__ls).nodes, __node != (__ls).head); \
    __node = __node->next)

#define PrevPolyVertex(__u) \
    ((poly.nodes[__u].prev == poly.head ? \
    poly.head->prev : poly.nodes[__u].prev) - poly.nodes)
#define NextPolyVertex(__u) \
    ((poly.nodes[__u].next == poly.head ? \
    poly.head->next : poly.nodes[__u].next) - poly.nodes)

static inline void CheckEarAndUpdate(const Vector2 *p, int u)
{
    bool isIn = CyclicListContains(&ear, u);
    int v = PrevPolyVertex(u),
        w = NextPolyVertex(u);
    int i;
    for CyclicListEach(reflex, i)
        if (CheckCollisionPointTriangle(p[i], p[u], p[v], p[w])) {
            // Not an ear!
            if (isIn) CyclicListRemove(&ear, u);
            return;
        }
    // An ear
    if (!isIn) CyclicListInsert(&ear, u);
}

// https://www.geometrictools.com/Documentation/TriangulationByEarClipping.pdf
// O(n^2)
static void Triangulate(const Vector2 *p, int n)
{
    CyclicListInit(&poly, n);
    CyclicListInit(&convex, n);
    CyclicListInit(&ear, n);
    CyclicListInit(&reflex, n);

    int i;

    // Add in reverse order so that list traversal order is polygon order
    for (i = n - 1; i >= 0; --i) {
        CyclicListInsert(&poly, i);
        // The input is counter-clockwise, but the Y-axis is flipped
        // So for a vertex to be convex, the cross product should be negative
        if (cross(p[(i + n - 1) % n], p[i], p[(i + 1) % n]) <= 0)
            CyclicListInsert(&convex, i);
        else
            CyclicListInsert(&reflex, i);
    }

    for CyclicListEach(convex, i) CheckEarAndUpdate(p, i);

    for (int cnt = 0; cnt < n - 3; cnt++) {
        int u = ear.head->next - ear.nodes,
            v = PrevPolyVertex(u),
            w = NextPolyVertex(u);
        rlAddTriangle(p[v], p[u], p[w]);
        CyclicListRemove(&poly, u);
        CyclicListRemove(&convex, u);
        CyclicListRemove(&ear, u);
        // convex -> convex
        // reflex -> reflex | convex
        int vv = PrevPolyVertex(v),
            ww = NextPolyVertex(w);
        if (CyclicListContains(&reflex, v)) {
            if (cross(p[vv], p[v], p[w]) <= 0) {    // Note the flipped Y-axis
                CyclicListRemove(&reflex, v);
                CyclicListInsert(&convex, v);
                CheckEarAndUpdate(p, v);
            }
        } else {
            CheckEarAndUpdate(p, v);
        }
        if (CyclicListContains(&reflex, w)) {
            if (cross(p[v], p[w], p[ww]) <= 0) {
                CyclicListRemove(&reflex, w);
                CyclicListInsert(&convex, w);
                CheckEarAndUpdate(p, w);
            }
        } else {
            CheckEarAndUpdate(p, w);
        }
    }
    int u = ear.head->next - ear.nodes,
        v = PrevPolyVertex(u),
        w = NextPolyVertex(u);
    rlAddTriangle(p[u], p[v], p[w]);
}

void DrawPolyFilledConcave(const Vector2 *points, int numPoints, Color color)
{
    if (numPoints < 3) return;
    rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        Triangulate(points, numPoints);
    rlEnd();
}

void DrawCircleFilledOutline(Vector2 c, float r, Color fill, Color outline)
{
    DrawCircleV(c, r + 3, outline);
    DrawCircleV(c, r, fill);
}

void DrawLineStripWithChromaBegin()
{
    rlBegin(RL_TRIANGLES);
}

void DrawLineStripWithChromaAdd(Vector2 p, Vector2 q, float d, float s, Color c)
{
    if (rlCheckBufferLimit(6)) {
        rlEnd();
        rlglDraw();
        rlBegin(RL_TRIANGLES);
    }

    float dx = (q.x - p.x) / d, dy = (q.y - p.y) / d;

    rlColor4ub(c.r, c.g, c.b, c.a);
    rlVertex2f(q.x + dy * s / 2, q.y - dx * s / 2);
    rlColor4ub(c.r, c.g, c.b, c.a);
    rlVertex2f(p.x + dy * s / 2, p.y - dx * s / 2);
    rlColor4ub(c.r, c.g, c.b, c.a);
    rlVertex2f(p.x - dy * s / 2, p.y + dx * s / 2);

    rlColor4ub(c.r, c.g, c.b, c.a);
    rlVertex2f(p.x - dy * s / 2, p.y + dx * s / 2);
    rlColor4ub(c.r, c.g, c.b, c.a);
    rlVertex2f(q.x - dy * s / 2, q.y + dx * s / 2);
    rlColor4ub(c.r, c.g, c.b, c.a);
    rlVertex2f(q.x + dy * s / 2, q.y - dx * s / 2);
}

void DrawLineStripWithChromaEnd()
{
    rlEnd();
}

static int circleSegs;
static float sinTable[64], cosTable[64];

// Modified from DrawCircleSector()
void DrawCircleSectorBatch(Vector2 center, Color color)
{
    if (rlCheckBufferLimit(3 * circleSegs)) {
        rlEnd();
        rlglDraw();
        rlBegin(RL_TRIANGLES);
    }

    //rlBegin(RL_TRIANGLES);
        for (int i = 0; i < circleSegs; i++)
        {
            rlColor4ub(color.r, color.g, color.b, color.a);

            rlVertex2f(center.x, center.y);
            rlVertex2f(center.x + sinTable[i], center.y + cosTable[i]);
            rlVertex2f(center.x + sinTable[i + 1], center.y + cosTable[i + 1]);
        }
    //rlEnd();
}

void DrawCirclesBegin(float r, int segs)
{
    circleSegs = segs;
    for (int i = 0; i <= segs; i++) {
        sinTable[i] = sinf(M_PI * 2 / segs * i) * r;
        cosTable[i] = cosf(M_PI * 2 / segs * i) * r;
    }
    rlBegin(RL_TRIANGLES);
}

void DrawCirclesAdd(Vector2 p, Color c)
{
    DrawCircleSectorBatch(p, c);
}

void DrawCirclesEnd()
{
    rlEnd();
}
