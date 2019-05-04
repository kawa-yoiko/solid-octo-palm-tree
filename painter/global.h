#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include "raylib.h"

#define SCR_W   1200
#define SCR_H   720

extern Font font;

void PalmTreeSetup();

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

#include "easings.h"

#endif
