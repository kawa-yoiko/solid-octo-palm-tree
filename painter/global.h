#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "raylib.h"

#define SCR_W   1200
#define SCR_H   720

// global.c

extern Font font;

void PalmTreeSetup();

// shapes_ext.c

void GenerateAnchoredPoly(
    Vector2 **outPoints,
    const Vector2 *points, int numPoints,
    Vector2 offset, Vector2 anchor, float scale);

void GenerateAnchoredBezier(
    Vector2 **outPoints,
    const Vector2 *points, int numSegs, int numDivs,
    Vector2 offset, Vector2 anchor,
    float scaleX, float scaleY, float rotation);

void DrawPolyFilledConvex(const Vector2 *points, int numPoints, Color color);

void DrawCircleFilledOutline(Vector2 c, float r, Color fill, Color outline);

#include "easings.h"

#include "math_ext.h"

#endif
