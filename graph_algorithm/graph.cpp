#include "graph.h"
#include <algorithm>
#include <cmath>
#include <queue>

using std::vector;



// single source shortest path (without path counting)
// using Dijkstra algorithm
vector<std::pair<double, unsigned>> Graph::sssp(unsigned source) const
{
	unsigned N = edge.size();
	vector<double> dist(N, INFINITY);
	vector<unsigned> cnt(N, 0);
	vector<bool> used(N, false);
	struct pq_t
	{
		unsigned v, cnt;
		double w;
	};
	struct pq_cmp
	{
		bool operator() (const pq_t& a, const pq_t& b)
		{
			return a.w > b.w || (a.w == b.w && a.cnt < b.cnt);
		}
	};
	std::priority_queue<pq_t, std::vector<pq_t>, pq_cmp> q;
	dist[source] = 0;
	cnt[source] = 1;
	q.push({source, 1, 0});
	for (unsigned i=0; i<N && !q.empty(); ++i)
	{
		unsigned u = q.top().v;
		q.pop();
		while (used[u])
		{
			if (q.empty()) break;
			u = q.top().v;
			q.pop();
		}
		if (used[u]) break;
		used[u] = true;
		for (auto const& t: edge[u])
		{
			if (dist[t.v] > dist[u] + t.w)
			{
				dist[t.v] = dist[u] + t.w;
				cnt[t.v] = cnt[u];
				q.push({t.v, cnt[t.v], dist[t.v]});
			}
			else if (dist[t.v] == dist[u] + t.w)
			{
				cnt[t.v] += cnt[u];
				q.push({t.v, cnt[t.v], dist[t.v]});
			}
		}
	}
	vector<std::pair<double, unsigned>> t;
	for (int i=0; i<N; ++i)
		t.push_back({dist[i], cnt[i]});
	return t;
}




// compute betweenness using Brandes algorithm
void Graph::getBetweenness()
{
	int n = edge.size();
	vector<int> P[n];
	betweenness = std::vector<double>(n,0);

	for (int s=0; s<n; ++s)
	{
		for (auto& p: P) p.clear();
		for (int v=0; v<n; ++v)
			for (auto const& e: edge[v])
				if (d[s][e.v].first == d[s][v].first + e.w)
					P[e.v].push_back(v);
		vector<std::pair<double, unsigned>> S;
		for (int i=0; i<n; ++i)
			if (!isinf(d[s][i].first))
				S.push_back({d[s][i].first, i});
		std::sort(S.begin(), S.end(), std::greater<std::pair<double, unsigned>>());
		// S: non-increasing dist {dist, node}
		double delta[n];
		std::fill_n(delta, n, 0);
		for (auto const& p: S)
		{
			int w = p.second;
			for (int v: P[w])
				delta[v] += d[s][v].second / d[s][w].second * (1 + delta[w]);
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
	for (int v=0; v<N; ++v)
	{
		double sum = 0;
		for (int i=0; i<N; ++i)
			if (i!=v)
				sum += 1.0 / d[v][i].first;
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
	tarjan();
	const int n = edge.size();
	d = decltype(d)(n);
	for (int i=0; i<n; ++i)
		d[i] = sssp(i);
	getBetweenness();
	getCloseness();
	getPagerank(15);
	normalize(betweenness);
	normalize(closeness);
	normalize(pagerank);
}


namespace NSTarjanAlgorithm
{
	std::vector<unsigned> low, dfn;
	std::vector<bool> inStack;
	int dfsTime, col_num;
	std::stack<int> S;
	void dfs(const Graph& G, int x);
}
