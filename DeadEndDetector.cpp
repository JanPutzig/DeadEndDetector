#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <list>

using std::vector;
using std::map;

struct Vertex
{
	//ids of all vertices connected by an edge
	vector<int> neighbors;

	//id of this vertex
	int id;

	//initializes vertex with id and empty vector neighbors
	Vertex(int id):id(id)
	{
		neighbors = vector<int>(0);
	}

	Vertex()
	{
		id = -1;
		neighbors = vector<int>(0);
	}
};

typedef vector<Vertex> Graph;
typedef map<int, Vertex> Subgraph;
typedef std::pair<int, int> Sign;


Graph readFile(std::string file)
{
	//initialize file_reader
	std::ifstream inputFile;
	inputFile.open(file);

	//read size of graph
	int m, n;
	inputFile >> n >> m;

	//initialize graph
	Graph result = Graph(n);
	for (int i = 0; i < n; i++)
	{
		result[i] = Vertex(i);
	}

	//read m lines, the edges of the graph
	int v, w;
	for (int i = 0; i < m; i++)
	{
		//read edge
		inputFile >> v >> w;

		//add adjacency to vertices, offset input to work in range 0 to n-1
		result[v-1].neighbors.push_back(w-1);
		result[w-1].neighbors.push_back(v-1);
	}

	//cleanup and return
	inputFile.close();
	return result;
}

//remove vertex id if it is a leaf
//continue to trim its ancestors if they thereby turn into leafs 
bool trim(Subgraph& g, Subgraph::iterator& it)
{
	int id = it->first;
	//check if vertex is a leaf
	if (g[id].neighbors.size() == 1)
	{
		//find its parent
		int parent_id  = g[id].neighbors[0];

		//delete the connecting edge
		g[parent_id].neighbors.erase(std::find(g[parent_id].neighbors.begin(), g[parent_id].neighbors.end(), id));

		//delete the leaf
		it = g.erase(it);

		//check the parent
		Subgraph::iterator nextIt = g.find(parent_id);
		trim(g, nextIt);
		return true;
	}
	return false;
}

//comparison function for lexicographical order of signs
bool lexiComp(const Sign& a, const Sign& b)
{
	if (a.first != b.first) {
		return a < b;
	}
	else
	{
		return a.second < b.second;
	}
}

int main() {
	//input prompt
	std::cout << "Please designate the input file: \n";

	//get filename
	std::string file;
	std::cin >> file;

	//generate Graph
	Graph complete = readFile(file);

	//initialize corresponding vector of flags
	vector<char> visited = vector<char>(complete.size(),false);

	//initialize result as empty
	std::list<Sign> result = std::list<Sign>();

	int minimum_id = 0;
	//while unprocessed vertices remain
	while(minimum_id < complete.size())
	{
		//initialize empty subgraph, and tree flag
		Subgraph g = Subgraph();
		bool tree = true;

		//find next unprocessed vertex
		while(visited[minimum_id])
		{
			++minimum_id;
			if (minimum_id == complete.size())
			{
				break;
			}
		}

		if (minimum_id == complete.size())
		{
			break;
		}

		//add next unprocessed vertex to g
		int root = minimum_id++;
		g[root] = complete[root];
		visited[root] = true;

		//BFS to build a copy of the connected component and determine if it is a tree
		std::list<Vertex> bfs = std::list<Vertex>();
		bfs.push_front(g[root]);
		while (!bfs.empty())
		{	
			Vertex curr = bfs.front();
			bfs.pop_front();
			int processed_neighbors = 0;
			//add unprocessed neighbors to g, flag them as processed
			//count previously processed neighbors
			for (vector<int>::iterator it = curr.neighbors.begin(); it != curr.neighbors.end(); it++)
			{
				int tmp = *it;
				if (!visited[tmp])
				{
					g[tmp] = complete[tmp];
					bfs.push_back(g[tmp]);
					visited[tmp] = true;
				}
				else
				{
					++processed_neighbors;
				}
			}

			//if more than one neighbor has already been processed, it is no tree
			if (processed_neighbors > 1)
			{
				tree = false;
			}
		}
		//find positions of dead-end signs for this component
		//for a tree, all leaves receive a dead-end sign on their end of the street
		if (tree)
		{
			for (Subgraph::iterator it = g.begin(); it !=  g.end(); it++)
			{
				if (it->second.neighbors.size() == 1)
				{
					result.push_back(Sign(it->first,it->second.neighbors[0]));
				}
			}
		}
		/*
		for graphs with a cycle, signs are added where a street leads
		from a vertex in a cycle or on a path connecting two cycles
		to a vertex for which neither is true
		
		to identify these, repeatedly remove all leaves from the component, until no leaves remain
		then any street leading from a vertex in the remainder outside of the remainder receives a sign
		*/
		else
		{
			Subgraph trimmedG = g;
			for (Subgraph::iterator it = trimmedG.begin(); it != trimmedG.end();)
			{
				//remove leaves/trees hanging off
				if (!trim(trimmedG, it))
				{
					it++;
				}
			}
			//add all pairs of remaining nodes with removed neighbors to the result
			for (Subgraph::iterator it = g.begin(); it != g.end(); it++)
			{
				//if it points to a vertex outside tmpG
				if (trimmedG.find(it->first) == trimmedG.end())
				{
					//search its neighbors
					for (vector<int>::iterator findIt = it->second.neighbors.begin(); findIt != it->second.neighbors.end(); findIt++)
					{
						//for a vertex in tmpG
						if (trimmedG.find(*findIt) != trimmedG.end())
						{
							//add a Sign at the end inside the trimmed Graph
							result.push_back(Sign(*findIt, it->first));
						}
					}
				}
			}
		}
	}
	//sort result according to specifications
	result.sort(lexiComp);
	//output result
	int k = result.size();
	std::cout << k << "\n";
	for (int i = 0; i < k; i++)
	{
		std::cout << result.front().first+1 << " " << result.front().second+1 << "\n";
		result.pop_front();
	}
	return 0;
}