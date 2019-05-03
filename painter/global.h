#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#define SCR_W   800
#define SCR_H   480

void GenerateAnchoredPoly(
    Vector2 **outPoints,
    const Vector2 *points, int numPoints,
    Vector2 offset, Vector2 anchor, float scale);

void GenerateAnchoredBezier(
    Vector2 **outPoints,
    const Vector2 *points, int numSegs, int numDivs,
    Vector2 offset, Vector2 anchor, float scale);

void DrawPolyFilledConvex(const Vector2 *points, int numPoints, Color color);

#endif
