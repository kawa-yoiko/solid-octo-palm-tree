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
	std::vector<double> last(N, 1.0/N);
	std::vector<double> next(N, 0);
	while (nIter--)
	{
#pragma omp parallel for collapse(2)
		for (unsigned i=0; i<N; ++i)
			for (auto const& p: trans[i])
				next[p.v] += p.w * last[i];
		std::swap(last, next);
	}
	if (normalize) delete(&trans);
	return last;
}

