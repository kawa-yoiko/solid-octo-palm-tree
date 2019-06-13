#ifndef _GRAPH_H_
#define _GRAPH_H_

#ifdef __cplusplus
extern "C" {
#endif

void InitGraph(int x, int y, int hw, int hh);

void VerletTick();
void VerletDraw();

#ifdef __cplusplus
}
#endif

#endif
