#ifndef _EASINGS_H_
#define _EASINGS_H_

#include "raymath.h"

static inline float Min(float x, float y) { return x < y ? x : y; }
static inline float Max(float x, float y) { return x > y ? x : y; }

static inline float EaseQuadIn(float x) { return x * x; }
static inline float EaseQuadOut(float x) { return 1 - EaseQuadIn(1 - x); }

static inline float EaseCubicIn(float x) { return x * x * x; }
static inline float EaseCubicOut(float x) { return 1 - EaseCubicIn(1 - x); }

static inline float EaseElasticOut(float x, float p)
{
    return exp(-10 * x) * sin((x * 2 - p / 2) * M_PI / p) + 1;
}

#endif
