#include "graph.h"



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


// compute betweenness using Brandes' algorithm
void Graph::getBetweenness()
{
	
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

