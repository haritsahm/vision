#ifndef GRAPH_IMPLEMENTATION
#define GRAPH_IMPLEMENTATION

// #include <cstdlib>
// #include <cstring>
// #include <set>
// #include <iostream>


//using ////////std::cout;
namespace graph_implementation
{
	struct Ad_matrix
	{
		bool is_present;
		double edge_cost;
	};

	template <class Item>
	class Graph
	{
	public:
		//member constants
		static const std::size_t MAXIMUM=500; //to be defined in terms of no of obstacles....****
		//constructor
		Graph()
		{
			many_vertices=0;
			no_path_flag=0;
			for(std::size_t i = 0; i < MAXIMUM; i++)
			{
				for(std::size_t j = 0; j < MAXIMUM; j++)
				{
					edges[i][j].is_present = false;
					onCircle[i][j] = false;
				}
			}
		}
		//modification member functions
		std::size_t add_vertex(const Item& label);
		void add_edge(std::size_t source,std::size_t target,double cost, bool isOnCircle = false);
		void update_vertex(const Item& label,std::size_t index);
		void remove_edge(std::size_t source,std::size_t target);
		//const member functions
		std::size_t size() const {return many_vertices;}
		bool is_edge(std::size_t source,std::size_t target);
		bool is_onCircle(std::size_t source,std::size_t target);
		Item operator [] (std::size_t vertex);
		void initialize();
		std::size_t getClosestUnmarkedNode();
		void dijkstra();
		void output();
		void printPath(std:: size_t node);
		std::size_t returnPathPoint(std::size_t node);
		void cleartree();
		void debug_filewrite();
		int no_path_flag;//set to 0 initially,-1 if path cant be made.
		
	private:
		Ad_matrix edges[MAXIMUM][MAXIMUM];
		bool onCircle[MAXIMUM][MAXIMUM];
		Item labels[MAXIMUM];
		std::size_t many_vertices;
		double distance[MAXIMUM];
		bool mark[MAXIMUM];
		std::size_t predecessor[MAXIMUM];
		std::size_t source;
		
	};

	template <class Item>
	const std::size_t Graph<Item>::MAXIMUM;

	template <class Item>
	void Graph<Item>::add_edge(std::size_t source,std::size_t target,double cost, bool isOnCircle)
	{
		 assert(source<size());
		 assert(target<size());
		// if(source>=size())
		// {	
		// 	no_path_flag=-1;
			
		// 	////////std::cout<<"\ncrashed\n";
		// 	//sleep(1000);
		// 		}
		// if(target>=size())
		// {	
		// 	no_path_flag=-1;
			
		// 	////////std::cout<<"\ncrashed\n";
		// 	//sleep(1000);
	
		// }
		edges[source][target].is_present=true;
		onCircle[source][target] = isOnCircle;
		edges[source][target].edge_cost=cost;
	}

	template <class Item>
	std::size_t Graph<Item>::add_vertex(const Item& label)
	{
		std::size_t new_vertex_number;
		std::size_t other_number;
		
		assert(size()<MAXIMUM);
		new_vertex_number=many_vertices;
		++many_vertices;
		for(other_number=0;other_number<many_vertices;++other_number)
		{
			edges[other_number][new_vertex_number].is_present=false;
			edges[new_vertex_number][other_number].is_present=false;
			edges[other_number][new_vertex_number].edge_cost=0;
			edges[new_vertex_number][other_number].edge_cost=0;
		}
		labels[new_vertex_number]=label;
		return new_vertex_number;
	}

	template <class Item>
	void Graph<Item>::update_vertex(const Item& label,std::size_t index)
	{
		labels[index]=label;
	}


	template <class Item>
	bool Graph<Item>::is_edge(std::size_t source,std::size_t target)
	{
		assert(source<size());
		assert(target<size());
		// if(source>=size())
		// {	
		// 	no_path_flag=-1;
			
		// 	////////std::cout<<"\ncrashed\n";
		// 	//sleep(1000);
		// 	return edges[0][0].is_present;
		// }
		// if(source>=size())
		// {	
		// 	no_path_flag=-1;
			
		// 	////////std::cout<<"\ncrashed\n";
		// 	//sleep(1000);
		// 	return edges[0][0].is_present;

		// }
		
		return edges[source][target].is_present;
	}

	template <class Item>
	bool Graph<Item>::is_onCircle(std::size_t source,std::size_t target)
	{
		assert(source<size());
		assert(target<size());
		// if(target>=size())
		// {	
		// 	no_path_flag=-1;
			
		// 	////////std::cout<<"\ncrashed\n";
		// 	//sleep(1000);
		// 	return onCircle[0][0];
		// }
		// if(source>=size())
		// {	
		// 	no_path_flag=-1;
			
		// 	////////std::cout<<"\ncrashed\n";
		// 	//sleep(1000);
		// 	return onCircle[0][0];
		// }
		return onCircle[source][target];
	}


	template <class Item>
	Item Graph<Item>::operator [] (std::size_t vertex)
	{
		assert(vertex<size());
		// if(vertex>=size())
		// {	
		// 	no_path_flag=-1;
			
		// 	////////std::cout<<"\ncrashed\n";
		// 	//sleep(1000);
		// 	return labels[0];
		// }
		return labels[vertex];
	}

	template <class Item>
	void Graph<Item>::remove_edge(std::size_t source,std::size_t target)
	{
		assert(source<size());
		assert(target<size());
		// if(source>=size())
		// {	
		// 	no_path_flag=-1;
			
		// 	////////std::cout<<"\ncrashed\n";
		// 	//sleep(1000);
			
		// }
		// if(target>=size())
		// {	
		// 	no_path_flag=-1;
			
		// 	////////std::cout<<"\ncrashed\n";
		// 	//sleep(1000);
			
		// }
		edges[source][target].is_present=false;
		edges[source][target].edge_cost=0;
	}

	template <class Item>
	void Graph<Item> :: initialize()
	{
		for(std::size_t i=0;i<many_vertices;i++)
		{
			mark[i]=false;
			predecessor[i]=-1;
			distance[i]=DBL_MAX;
		}
		distance[source]=0;
	}

	template <class Item>
	std::size_t Graph<Item> :: getClosestUnmarkedNode()
	{
		double minDistance=DBL_MAX;
		std::size_t closestUnmarkedNode;
		for(std::size_t i=0;i<many_vertices;i++)
		{
			if((!mark[i]) && (minDistance >= distance[i]))
			{
				minDistance=distance[i];
				closestUnmarkedNode=i;
			}
		}
		return closestUnmarkedNode;
	}

	template<class Item>
	void Graph<Item> :: dijkstra()
	{
		
		source=0;
		
		initialize();
		
		int minDistance=INT_MAX;
		std::size_t closestUnmarkedNode;
		int count=0;
		
		while(count<many_vertices)
		{
			closestUnmarkedNode=getClosestUnmarkedNode();
			////////std::cout<<"closestUnmarkedNode "<<closestUnmarkedNode<<std::endl;
			mark[closestUnmarkedNode]=true;
			for(std::size_t i=0;i<many_vertices;i++)
			{
				if((!mark[i]) && (edges[closestUnmarkedNode][i].edge_cost>0) && edges[closestUnmarkedNode][i].is_present==true)
				{
					////////std::cout<<"cutting through the firset if are "<<i<<std::endl;
					if(distance[i]>(distance[closestUnmarkedNode]+edges[closestUnmarkedNode][i].edge_cost))
					{
						distance[i]=(distance[closestUnmarkedNode]+edges[closestUnmarkedNode][i].edge_cost);
						predecessor[i]=closestUnmarkedNode;
						////////std::cout<<"predecessor[i] "<<predecessor[i]<<std::endl;
					}
				}
			}
			count++;
		}
		//////////std::cout<<"printing path\n";
		std::size_t hello;
		hello=1;
		printPath(hello);
	}

	template <class Item>
	void Graph<Item> :: printPath(std::size_t node)
	{
		std::cout<<"1\n";
		while(node!=0)
		{
			//////////std::cout<<"genorai\n";
			std::cout<<""<<predecessor[node]<<"\n";
			////////std::cout<<"\n";	
			node=predecessor[node];
		}
	}

	template <class Item>
	std::size_t Graph<Item> :: returnPathPoint(std::size_t node)
	{
		return predecessor[node];
	}


	template <class Item>
	void Graph<Item> ::cleartree()
	{
		many_vertices=0;
			for(std::size_t i = 0; i < MAXIMUM; i++)
			{
				for(std::size_t j = 0; j < MAXIMUM; j++)
				{
					edges[i][j].is_present = false;
					onCircle[i][j] = false;
				}
			}
	}

	template <class Item>
	void Graph<Item> ::debug_filewrite()
	{	
		FILE *file_pointer;
		file_pointer=fopen("path_debug.bin","ab+");
		assert(file_pointer);
		fwrite(edges,MAXIMUM*MAXIMUM,1,file_pointer);
		fclose(file_pointer);

	}

}

#endif