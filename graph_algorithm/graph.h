#pragma once

#include <vector>

struct Graph
{
	struct Edge
	{
		unsigned v;
		double w;
	};
	std::vector<std::vector<Edge>> edge;

	std::vector<std::vector<unsigned>> tarjan();
	std::vector<double> sssp(unsigned source);
	std::vector<double> pagerank();
	std::vector<double> betweenness();
	std::vector<double> closeness();
};

