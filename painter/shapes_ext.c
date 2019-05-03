#include "rlgl.h"
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
    Vector2 offset, Vector2 anchor, float scale)
{
    Vector2 *_outPoints = *outPoints;
    if (_outPoints == NULL)
        _outPoints = (Vector2 *)malloc(sizeof(Vector2) * (numSegs * numDivs + 1));

    for (int i = 0; i < numSegs; i++)
        for (int j = 0; j < numDivs + (i == numSegs - 1 ? 1 : 0); j++) {
            int idx = i * numDivs + j;
            _outPoints[idx] = bezierInterpolate(
                points[i * 3], points[i * 3 + 1],
                points[i * 3 + 2], points[i * 3 + 3], (float)j / numDivs);
            _outPoints[idx].x = offset.x + (_outPoints[idx].x - anchor.x) * scale;
            _outPoints[idx].y = offset.y + (_outPoints[idx].y - anchor.y) * scale;
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

// Incorrect
void DrawPolyFilledConcave(const Vector2 *points, int numPoints, Color color)
{
    if (numPoints < 3) return;
    rlBegin(RL_TRIANGLES);
        rlColor4ub(color.r, color.g, color.b, color.a);
        for (int i = 2; i < numPoints; i++) {
            rlAddTriangle(points[0], points[i - 1], points[i]);
        }
    rlEnd();
}
