#include "graph.h"


// multi source shortest path (with path counting)
// using Floyd-Warshall algorithm
void Graph::floyd()
{
	int n = edge.size();
	auto d = vector(n, vector(n, std::make_pair(INFINITY, 0)));
	for (int i=0; i<n; ++i)
		d[i][i] = {0,1};
	for (int i=0; i<n; ++i)
		for (auto const& p: edge[i])
		{
			if (p.w < d[i][p.v].first)
				d[i][p.v] = {p.w, 0};
			if (p.w == d[i][p.v].first)
				d[i][p.v].second += 1;
		}
	for (int k=0; k<n; ++k)
		for (int i=0; i<n; ++i) if (i!=k)
			for (int j=0; j<n; ++j) if (j!=k)
			{
				if (d[i][j].first > d[i][k].first + d[k][j].first)
					d[i][j] = {d[i][k].first + d[k][j].first, 0};
				if (d[i][j].first == d[i][k].first + d[k][j].first)
					d[i][j].second += d[i][k].second * d[k][j].second;
			}
	return d;
}




// closeness
std::vector<double> Graph::closeness() const
{
	const unsigned N = edge.size();
	std::vector<double> C(N,0);
	for (int v=0; v<N; ++v)
	{
		double sum = 0;
		for (double dist: d[v])
			sum += dist;
		C[v] = 1.0 / sum;
	}
	return C;
}




std::vector<double> Graph::bf_betweenness()
{
	static const double eps = 1e-9;
	floyd();
	const unsigned n = edge.size();
	std::vector<double> C(n);
	for (int i=0; i<n; ++i)
		for (int u=0; u<n; ++u)
			for (int v=0; v<n; ++v)
				if (u!=i && v!=i &&
				std::abs(d[u][i].first + d[i][v].first - d[u][v].first) < eps)
					C[i] += (double) d[u][i].second * d[i][v].second / d[u][v].second;
	return C;
}




// pagerank (without smoothing)
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
		for (unsigned i=0; i<N; ++i)
			for (auto const& p: trans[i])
				next[p.v] += p.w * curr[i];
		curr = std::move(next);
	}
	if (normalize) delete(&trans);
	return curr;
}



namespace NSTarjanAlgorithm
{
	std::vector<int> low, dfn, color;
	int dfsTime;
	std::stack<int> S;

	void dfs(int x)
	{
		low[x] = dfn[x] = ++dfsTime;
		inStack = true;
		S.push(x);
		for (auto const& e: edge[x])
		{
			if (!dfn[e.v])
			{
				dfs(e.v);
				low[x] = min(low[x], low[e.v]);
			}
			else if (inStack[e.v])
				low[x] = min(low[x], dfn[e.v]);
		}
		if (dfn[x] == low[x])
		{
			inStack[x] = false;
			component = {x};
			color[x] = ++col_num;
			while (S.top() != x)
			{
				component.push_back(S.top());
				inStack[S.top()] = false;
				S.pop();
			}
			S.pop();
		}
	}
}



void Graph::tarjan()
{
	using namespace NSTarjanAlgorithm;
	dfsTime = 0;
	const int n = edge.size();
	low = dfn = color = vector<int>(n,0);
	for (int i=0; i<n; ++i)
	{
		if (dfn[i] == 0)
			tarjan(i);
	}
	this->color = color;
}


