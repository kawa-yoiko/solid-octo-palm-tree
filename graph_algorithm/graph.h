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

	std::vector<std::vector<std::pair<double, int>>> d; // d[u][v] = {dist, num_shortest_path}
	std::vector<unsigned> color;
	std::vector<double> pagerank;
	std::vector<double> closeness;
	std::vector<double> betweenness;

	void compute();

private:
	void getPagerank(unsigned nIter, bool normalize = false);
	void getCloseness();
	void bf_betweenness();
	void floyd();
	void tarjan();
};


namespace NSTarjanAlgorithm
{
	std::vector<unsigned> low, dfn;
	std::vector<bool> inStack;
	int dfsTime, col_num;
	std::stack<int> S;
	void dfs(const Graph& G, int x);
}

