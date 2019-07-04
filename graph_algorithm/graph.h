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
	std::vector<std::vector<Edge>> edge;

	std::vector<std::vector<std::pair<double, int>>> d; // d[u][v] = {dist, num_shortest_path}
	std::vector<unsigned> _color;
	std::vector<double> _pagerank;
	std::vector<double> _closeness;
	std::vector<double> _betweenness;

	void compute();

//private:
	std::vector<double> pagerank(unsigned nIter, bool normalize = false) const;
	std::vector<double> betweenness() const;
	std::vector<double> closeness() const;
	std::vector<double> bf_betweenness() const;
	std::vector<double> sssp(unsigned source) const;
	void floyd() const;
	void tarjan() const;
};

