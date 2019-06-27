#ifndef _GRAPH_H_
#define _GRAPH_H_

#ifdef __cplusplus
extern "C" {
#endif

void InitGraph(int x, int y, int hw, int hh);

void VerletTick();
void VerletDraw();

void VerletResetRate();
void VerletSetFix(int id, float x, float y);
void VerletCancelFix(int id);

void VerletMousePress(int px, int py);
void VerletMouseMove(int px, int py);
void VerletMouseRelease();
void VerletChangeScale(int wheel, int px, int py);

#ifdef __cplusplus
}
#endif

#endif
