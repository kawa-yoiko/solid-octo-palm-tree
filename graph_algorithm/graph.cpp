#include "graph.h"
#include <cmath>
#include <stack>

using std::vector;


// multi source shortest path (with path counting)
// using Floyd-Warshall algorithm
void Graph::floyd()
{
	int n = edge.size();
	d = vector<vector<std::pair<double, int>>>
		(n, vector<std::pair<double, int>>(n, std::make_pair(INFINITY, 0)));
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
}




// closeness
void Graph::getCloseness()
{
	const unsigned N = edge.size();
	closeness = std::vector<double>(N,0);
	for (int v=0; v<N; ++v)
	{
		double sum = 0;
		for (int i=0; i<N; ++i)
			if (i!=v) sum += 1.0 / p[i].first;
		closeness[v] = sum;
	}
	double maxC = 0;
	for (int i=0; i<N; ++i)
		maxC = std::max(maxC, closeness[i]);
	for (int i=0; i<N; ++i)
		closeness[i] /= maxC;	
}




void Graph::bf_betweenness()
{
	static const double eps = 1e-9;
	floyd();
	const unsigned n = edge.size();
	betweenness = std::vector<double>(n,0);
	for (int i=0; i<n; ++i)
		for (int u=0; u<n; ++u)
			for (int v=0; v<n; ++v)
				if (u!=i && v!=i &&
				std::abs(d[u][i].first + d[i][v].first - d[u][v].first) < eps)
					betweenness[i] += (double) d[u][i].second * d[i][v].second / d[u][v].second;
	double maxC = 0;
	for (int i=0; i<n; ++i)
		maxC = std::max(maxC, betweenness[i]);
	for (int i=0; i<n; ++i)
		betweenness[i] /= maxC;	
}




// pagerank (without smoothing)
void Graph::getPagerank(unsigned nIter)
{
	unsigned N = edge.size();
	std::vector<double> curr(N, 1.0/N);
	std::vector<double> next;
	while (nIter--)
	{
		next = std::vector<double>(N, 0);
		for (unsigned i=0; i<N; ++i)
			for (auto const& p: edge[i])
				next[p.v] += 1.0/p.w * curr[i];
		curr = std::move(next);
	}
	pagerank = curr;
}



namespace NSTarjanAlgorithm
{
	void dfs(Graph& G, int x)
	{
		low[x] = dfn[x] = ++dfsTime;
		inStack[x] = true;
		S.push(x);
		for (auto const& e: G.edge[x])
		{
			if (!dfn[e.v])
			{
				dfs(G, e.v);
				low[x] = std::min(low[x], low[e.v]);
			}
			else if (inStack[e.v])
				low[x] = std::min(low[x], dfn[e.v]);
		}
		if (dfn[x] == low[x])
		{
			inStack[x] = false;
			G.color[x] = ++col_num;
			while (S.top() != x)
			{
				G.color[S.top()] = col_num;
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
	dfsTime = col_num = 0;
	const int n = edge.size();
	low = dfn = color = vector<unsigned>(n, 0);
	inStack = vector<bool>(n, false);
	for (int i=0; i<n; ++i)
	{
		if (dfn[i] == 0)
			dfs(*this, i);
	}
}


void Graph::compute()
{
	tarjan();
	floyd();
	getCloseness();
	bf_betweenness();
	getPagerank(15);
}


namespace NSTarjanAlgorithm
{
	std::vector<unsigned> low, dfn;
	std::vector<bool> inStack;
	int dfsTime, col_num;
	std::stack<int> S;
	void dfs(const Graph& G, int x);
}

