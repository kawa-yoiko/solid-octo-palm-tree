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
	std::vector<double> pagerank(unsigned nIter, bool normalize = false) const;
	std::vector<double> betweenness() const;
	std::vector<double> closeness() const;
	std::vector<double> bf_betweenness() const;
	std::vector<double> sssp(unsigned source) const;
	void floyd() const;
	void tarjan() const;
};

