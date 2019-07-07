#include "graph.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <chrono>

using std::vector;



// single source shortest path (without path counting)
// using SPFA algorithm
vector<double> Graph::sssp(unsigned source) const
{
	int n = edge.size();
	vector<double> d(n, INFINITY);
	bool inq[n];
	memset(inq, 0, sizeof inq);
	d[source] = 0;
	std::queue<int> q;
	q.push(source);
	inq[source] = 1;
	while (!q.empty())
	{
		unsigned u = q.front();
		q.pop();
		inq[u] = 0;
		for (auto const& e: edge[u])
		{
			if (d[e.v] > d[u] + e.w)
			{
				d[e.v] = d[u] + e.w;
				if (!inq[e.v])
				{
					q.push(e.v);
					inq[e.v] = 1;
				}
			}
		}
	}
	return d;
}



// compute betweenness using Brandes algorithm
void Graph::getBetweenness()
{
	unsigned n = edge.size();
	vector<unsigned> P[n]; // prev on shortest path
	unsigned cnt[n]; // shortest path count
	betweenness = std::vector<double>(n,0);

#pragma omp parallel for private(P,cnt) reduction(+:betweenness)
	for (unsigned s=0; s<n; ++s)
	{
		// compute P
		for (auto& p: P) p.clear();
		for (unsigned v=0; v<n; ++v)
			if (!std::isinf(dist[s][v]))
				for (auto const& e: edge[v])
					if (dist[s][e.v] == dist[s][v] + e.w)
						P[e.v].push_back(v);

		// S: sorted {dist, node}
		typedef std::pair<double, unsigned> s_val;
		vector<s_val> S;
		for (unsigned i=0; i<n; ++i)
			if (!std::isinf(dist[s][i]))
				S.push_back({dist[s][i], i});
		std::sort(S.begin(), S.end());

		// compute cnt
		memset(cnt, 0, sizeof cnt);
		cnt[s] = 1;
		for (auto const& p: S)
			for (unsigned v: P[p.second])
				cnt[p.second] += cnt[v];

		double delta[n];
		std::fill_n(delta, n, 0);
		// make S non-increasing
		std::reverse(S.begin(), S.end());
		for (auto const& p: S)
		{
			int w = p.second;
			for (int v: P[w])
				delta[v] += cnt[v] / cnt[w] * (1 + delta[w]);
			if (w != s)
				betweenness[w] += delta[w];
		}
	}
}



// closeness
void Graph::getCloseness()
{
	const unsigned N = edge.size();
	closeness = std::vector<double>(N,0);
#pragma omp parallel for
	for (int v=0; v<N; ++v)
	{
		double sum = 0;
		for (int i=0; i<N; ++i)
			if (i!=v)
				sum += 1.0 / dist[v][i];
		closeness[v] = sum;
	}
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
	color_count = col_num;
}



void normalize(std::vector<double>& a)
{
	double mu = 1.0 / *std::max_element(a.begin(), a.end());
	for (double& f: a)
		f *= mu;
}


void Graph::compute()
{
	auto start = std::chrono::system_clock::now();
	tarjan();
	const int n = edge.size();
	dist = vector<vector<double>>(n);
#pragma omp parallel for
	for (int i=0; i<n; ++i)
		dist[i] = sssp(i);
	getBetweenness();
	getCloseness();
	getPagerank(15);
	normalize(betweenness);
	normalize(closeness);
	normalize(pagerank);
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end-start;
	std::cout << "Graph::compute() takes " << elapsed_seconds.count() << "s\n";
}


namespace NSTarjanAlgorithm
{
	std::vector<unsigned> low, dfn;
	std::vector<bool> inStack;
	int dfsTime, col_num;
	std::stack<int> S;
	void dfs(const Graph& G, int x);
}
