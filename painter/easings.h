#ifndef _EASINGS_H_
#define _EASINGS_H_

#include "raymath.h"

inline float Min(float x, float y) { return x < y ? x : y; }
inline float Max(float x, float y) { return x > y ? x : y; }

inline float EaseQuadIn(float x) { return x * x; }
inline float EaseQuadOut(float x) { return 1 - EaseQuadIn(1 - x); }

inline float EaseCubicIn(float x) { return x * x * x; }
inline float EaseCubicOut(float x) { return 1 - EaseCubicIn(1 - x); }

inline float EaseElasticOut(float x, float p)
{
    return exp(-10 * x) * sin((x * 2 - p / 2) * M_PI / p) + 1;
}

#endif
