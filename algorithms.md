# 算法实现



## 最短路

使用堆优化的Dijkstra算法

复杂度（单源最短路）：$O(m \log n)$



介数中心度 Betweenness centrality

定义：图中一点的介数中心度为最短路可经过该点的点对数量。

使用带权图上的Brandes算法

复杂度：$O(nm\log n)$

## 紧密中心度 Closeness centrality

定义：图中一点的紧密中心度为到所有点的最短路的 $-\alpha$ 次方之和的 $\alpha$ 次方。

复杂度：$O(nm\log n)$

## 强连通分量

定义：图的强连通分量为极大的内部点对间均可互相到达的子图。

Tarjan算法

复杂度：$O(n+m)$

## PageRank

复杂度：$O(km)$