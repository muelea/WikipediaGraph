#ifndef SHORTEST_PATH_H
#define SHORTEST_PATH_H

#include <mpi.h>
#include "dynatomicvec.hpp"
#include <vector>

/*Builds two vectors including all information needed for the graphsearch
links_in_article_vec: Vector with ID of each article neighbor; Vector as a 
whole is orderd by article id*/
void build_small_graph(std::vector<std::int64_t> &links_in_article_vec,
	std::vector<std::int64_t> &offset, std::int64_t &links_in_article_vec_size,
	std::int64_t &offset_size, std::map<std::string, Article> &art_list_map);

/*MPI included
Sends offset and links_in_article_vec vectors to all processes and */
void send_map_info(std::int64_t rank, std::int64_t &map_size, 
	std::int64_t &links_in_article_vec_size, std::int64_t &offset_size, 
	std::vector<std::int64_t> &links_in_article_vec,
	std::vector<std::int64_t> &offset, MPI_Comm comm);

/*Shortest path search from one article to another article*/
void shortest_path_vec_vec(std::int64_t i, std::int64_t j, int ds, 
	DynAtomicArray<std::int64_t> &D, std::int64_t indexD, std::int64_t size_map, 
	std::vector<std::int64_t> &links, std::vector<std::int64_t> &offset);

/* Since DynAtomicArray can't be send via MPI atomic_D (in which the shortest
path is stored) is copied to a std::vector;
called in Graph_search*/ 
void copy_vec(DynAtomicArray<std::int64_t> &atomic_D, 
	std::vector<std::int64_t> &D);

/*Calculates shortest path between all articles; The DynAtomicArray is copied
to a std::vector which will be returned by the function*/
std::vector<std::int64_t> Graph_search(std::int64_t rank, std::int64_t map_size, 
	std::int64_t size, std::vector<std::int64_t> &graph_info, 
	std::vector<std::int64_t> &links_in_article_vec,
	std::vector<std::int64_t> &offset);

/*MPI included
Since processes can have different numbers of articles the Gatherv() function
must be used. Therefore the receivecounters and displacements are calculated*/
void recv_disp_calc(std::int64_t map_size, std::int64_t size, 
	std::vector<int> &recvcount, std::vector<int> &displs);

/*After Graph search has finished, all shortest path data was collected in 
D_complete. Copy_shortest_path_to_map copies all information into the map;
called in Gather_data()*/
void copy_shortest_path_to_map(std::map <std::string, Article> &art_list_map, 
	std::int64_t size, std::vector<std::int64_t> &D_complete);

/*MPI included
Gathers data from processes to Host and copies the vector with all shortest 
paths into the map*/
void Gather_data(std::int64_t size, std::int64_t rank, 
	std::int64_t map_size, std::vector<std::int64_t> &D, 
	std::map <std::string, Article> &art_list_map, MPI_Comm comm);

/*Print path (strings) between two articles */
void print_path(Article& art1, Article& art2, 
	std::map <std::string, Article> &art_list_map);

#endif