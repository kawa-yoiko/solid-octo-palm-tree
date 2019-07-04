#pragma once

#include <vector>
#include <stack>


class Graph
{
public:
	struct Edge
	{
		unsigned v;
		double w;
		Edge(unsigned _v, double _w) : v(_v), w(_w) { }
	};
	std::vector<std::vector<Edge>> edge;

	std::vector<std::vector<double>> d; // d[u][v] = dist
	int color_count;
	std::vector<unsigned> color;
	std::vector<double> pagerank;
	std::vector<double> closeness;
	std::vector<double> betweenness;

	void compute();

private:
	void getPagerank(unsigned nIter);
	void getBetweenness();
	void getCloseness();
	std::vector<double> sssp(unsigned source) const;
	void tarjan();
};



namespace NSTarjanAlgorithm
{
	extern std::vector<unsigned> low, dfn;
	extern std::vector<bool> inStack;
	extern int dfsTime, col_num;
	extern std::stack<int> S;
	extern void dfs(const Graph& G, int x);
}
