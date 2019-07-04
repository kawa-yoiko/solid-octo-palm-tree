#pragma once

#include <vector>


class Graph
{
public:
	struct Edge
	{
		unsigned v;
		double w;
		Edge(unsigned _v, double _w) : v(_v), w(_w) { }
	};
	using std::vector;
	vector<vector<Edge>> edge;

	vector<vector<std::pair<double, int>>> d; // d[u][v] = {dist, num_shortest_path}
	vector<unsigned> color;
	vector<double> pagerank;
	vector<double> closeness;
	vector<double> betweenness;

	void compute();

private:
	void getPagerank(unsigned nIter, bool normalize = false);
	std::vector<double> betweenness() const;
	void getCloseness();
	std::vector<double> bf_betweenness() const;
	std::vector<double> sssp(unsigned source) const;
	std::vector<std::pair<double, unsigned>> sssp(unsigned source) const;

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


