#include "graph.h"

std::vector<double> Graph::pagerank(unsigned nIter, bool normalize)
{
	decltype(edge)& trans = normalize? *new decltype(edge)(edge): edge;
	if (normalize)
	{
		auto trans = edge;
		for (auto& l: trans)
		{
			double sum = 0;
			for (auto const& p: l)
				sum += p.w;
			double normal = 1.0 / sum;
			for (auto& p: l)
				p.w *= normal;
		}
	}
	unsigned N = edge.size();
	std::vector<double> curr(N, 1.0/N);
	std::vector<double> next;
	while (nIter--)
	{
		next = std::vector<double>(N, 0);
#pragma omp parallel for collapse(2)
		for (unsigned i=0; i<N; ++i)
			for (auto const& p: trans[i])
				next[p.v] += p.w * curr[i];
		curr = std::move(next);
	}
	if (normalize) delete(&trans);
	return curr;
}

