#ifndef _MATH_EXT_H_
#define _MATH_EXT_H_

static inline Vector2 Vector2Rotate(const Vector2 p, const Vector2 o, float rad)
{
    float x1 = p.x - o.x;
    float y1 = p.y - o.y;
    float x2 = x1 * cos(rad) - y1 * sin(rad);
    float y2 = x1 * sin(rad) + y1 * cos(rad);
    return (Vector2){x2 + o.x, y2 + o.y};
}

#endif
