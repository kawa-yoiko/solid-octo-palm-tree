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

#ifdef __cplusplus
}
#endif

#endif
