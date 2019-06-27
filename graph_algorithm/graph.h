#pragma once

#include <vector>

struct Graph
{
	struct Edge
	{
		unsigned v;
		double w;
		Edge(unsigned _v, double _w) : v(_v), w(_w) { }
	};
	std::vector<std::vector<Edge>> edge;

	std::vector<std::vector<unsigned>> tarjan() const;
	std::vector<double> sssp(unsigned source) const;
	std::vector<double> pagerank(unsigned nIter, bool normalize = false) const;
	std::vector<double> betweenness() const;
	std::vector<double> closeness() const;
};

