#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "raylib.h"

#define SCR_W   1200
#define SCR_H   720

#include "pal.h"

#define ColorLerp(__a, __b, __t) ((Color) { \
    ((__a).r + (int)((__t) * ((__b).r - (__a).r) + 0.5)), \
    ((__a).g + (int)((__t) * ((__b).g - (__a).g) + 0.5)), \
    ((__a).b + (int)((__t) * ((__b).b - (__a).b) + 0.5)), \
    ((__a).a + (int)((__t) * ((__b).a - (__a).a) + 0.5)), \
})

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

void DrawPolyFilledConcave(const Vector2 *points, int numPoints, Color color);

void DrawCircleFilledOutline(Vector2 c, float r, Color fill, Color outline);

void DrawLineStripWithChromaBegin();
void DrawLineStripWithChromaAdd(Vector2 p, Vector2 q, Color c);
void DrawLineStripWithChromaEnd();

#include "easings.h"

#include "math_ext.h"

#endif
