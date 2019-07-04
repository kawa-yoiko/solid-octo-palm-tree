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



// single source shortest path (without path counting)
// using Dijkstra algorithm
std::vector<double> Graph::sssp(unsigned source) const
{
	unsigned N = edge.size();
	std::vector<double> dist(N, INFINITY);
	std::vector<bool> used(N, false);
	typedef std::pair<double, unsigned> pq_t;
	priority_queue<pq_t, std::vector<pq_t>, std::greater<pq_t>> q;
	dist[source] = 0;
	q.push({0, source});
	for (unsigned i=0; i<N && !q.empty(); ++i)
	{
		unsigned u = q.top().second;
		q.pop();
		while (used[u])
		{
			if (q.empty()) return dist;
			u = q.top().second;
			q.pop();
		}
		used[u] = true;
		for (auto const& t: edge[u])
			if (dist[t.v] > dist[u] + t.w)
			{
				dist[t.v] = dist[u] + t.w;
				q.push({dist[t.v], t.v});
			}
	}
	return dist;
}




// single source shortest path (with path counting)
// using Dijkstra algorithm
std::vector<std::pair<double, unsigned>> Graph::sssp(unsigned source) const
{
	unsigned N = edge.size();
	std::vector<std::pair<double, unsigned>> dist(N, {INFINITY, 0});
	std::vector<bool> used(N, false);
	typedef std::pair<double, std::pair<unsigned, unsigned>> pq_t;
	priority_queue<pq_t, std::vector<pq_t>, std::greater<pq_t>> q;
	dist[source] = {0,1};
	q.push({0, source});
	for (unsigned i=0; i<N && !q.empty(); ++i)
	{
		unsigned u = q.top().second;
		q.pop();
		while (used[u])
		{
			if (q.empty()) return dist;
			u = q.top().second;
			q.pop();
		}
		used[u] = true;
		for (auto const& t: edge[u])
			if (dist[t.v] > dist[u] + t.w)
			{
				dist[t.v] = dist[u] + t.w;
				q.push({dist[t.v], t.v});
			}
	}
	return dist;
}


// closeness
void Graph::getCloseness()
{
	const unsigned N = edge.size();
	closeness = std::vector<double>(N,0);
	for (int v=0; v<N; ++v)
	{
		double sum = 0;
		for (auto const& p: d[v])
			sum += p.first;
		closeness[v] = 1.0 / sum;
	}
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
void Graph::getPagerank(unsigned nIter, bool normalize)
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

