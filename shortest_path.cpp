#include <atomic>
#include <cmath>
#include <iostream>
#include <string>

#include <mpi.h>
#include <omp.h>

#include "wikiarticle.hpp"
#include "shortest_path.hpp"
#include "MPI_class.hpp"

void build_small_graph(std::vector<std::int64_t> &links_in_article_vec,
						std::vector<std::int64_t> &offset,
						std::int64_t &links_in_article_vec_size,
						std::int64_t &offset_size, 
						std::map<std::string, Article> &art_list_map){

	offset.resize(art_list_map.size()+1, 0);

	for(auto it = art_list_map.begin(); it != art_list_map.end(); ++it){
		Article& art = it->second;
		offset[art.id+1] = offset[art.id] + art.links_in_article_pointer.size();
		if(art.links_in_article_pointer.size() > 0){
			for(size_t a = 0; a < art.links_in_article_pointer.size(); ++a)
				links_in_article_vec.push_back(
					(art.links_in_article_pointer[a])->id);
		}
	}
	links_in_article_vec_size = links_in_article_vec.size();
	offset_size = offset.size();
}

void send_map_info(std::int64_t rank, std::int64_t &map_size, 
	std::int64_t &links_in_article_vec_size, std::int64_t &offset_size, 
	std::vector<std::int64_t> &links_in_article_vec,
	std::vector<std::int64_t> &offset, MPI_Comm comm){

		MPI_Bcast(&map_size, 1, MPI_INT64_T, 0, comm);
		MPI_Bcast(&links_in_article_vec_size, 1, MPI_INT64_T, 0, comm);
		MPI_Bcast(&offset_size, 1, MPI_INT64_T, 0, comm);

		if(rank != 0){
			links_in_article_vec.resize(links_in_article_vec_size);
			offset.resize(offset_size);
		}

		MPI_Bcast(links_in_article_vec.data(), links_in_article_vec_size,
			MPI_INT64_T, 0, comm);
		MPI_Bcast(offset.data(), offset_size, MPI_INT64_T, 0, comm);
}

void shortest_path_vec_vec(std::int64_t i, std::int64_t j, int ds,
	DynAtomicArray<std::int64_t> &D, std::int64_t indexD, std::int64_t size_map, 
	std::vector<int64_t> &links, std::vector<int64_t> &offset){

	int links_in_article_size = offset[i+1] - offset[i]; 

	#pragma omp parallel
	{
	for(int l = 0; l < links_in_article_size; ++l){
		int neighbor = links[offset[i] + l];
		std::int64_t value = D[indexD* size_map + neighbor];
		start :
		if(neighbor == j){
			if(value == -1){
 				if(!D[indexD * size_map + j].compare_exchange_weak(value, 
 												ds + 1)) goto start;
 			}else if(value > ds + 1){
 				if(!D[indexD * size_map + j].compare_exchange_weak(value, 
 												ds + 1)) goto start;
 			}
		}else{
			if(value <= ds + 1 && value != -1){
				continue;		
			}else{
				if(!D[indexD * size_map + neighbor].compare_exchange_weak(value,
														 ds + 1)) goto start;
				if(D[indexD * size_map + j] == -1 || D[indexD * size_map + j] > 
														ds + 2){
					#pragma omp task shared(D, links, offset)
					shortest_path_vec_vec(neighbor, j, ds + 1, D, indexD, 
										size_map ,links, offset);
				}
			}			
		}
	}
	}
}

void copy_vec(DynAtomicArray<std::int64_t> &atomic_D, 
	std::vector<std::int64_t> &D){

	#pragma omp for
	for(std::size_t i = 0; i < atomic_D.size(); ++i)D[i]  = atomic_D[i];
}

std::vector<std::int64_t> Graph_search(std::int64_t rank, std::int64_t map_size, 
	std::int64_t size, std::vector<std::int64_t> &graph_info, 
	std::vector<std::int64_t> &links_in_article_vec,
	std::vector<std::int64_t> &offset){

	//Calculate size of paires ech process has to find a shortest path inbetween
	std::int64_t add;
	rank < map_size%size ? add = 1 : add = 0;
	DynAtomicArray<std::int64_t> atomic_D(
		(std::int64_t)(map_size/size + add) * map_size);
	atomic_D.setTo(-1);

	#pragma omp parallel for
	for(std::int64_t i = rank; i < map_size; i += size){
		std::int64_t indexD = i/size;
		for(std::int64_t j = 0; j < map_size; ++j){
			if(i != j){
				shortest_path_vec_vec(i, j, 0, atomic_D, indexD, map_size, 
					links_in_article_vec, offset);
			}
			#pragma omp critical
			{
				++graph_info[0];

				if(atomic_D[indexD * map_size + j] != -1){
					graph_info[1] += atomic_D[indexD * map_size + j];
					++graph_info[2];
				}else{
					++graph_info[3];
				}
			}		
		}
	}

	rank < map_size%size ? add = 1 : add = 0;
	std::vector<std::int64_t> D((std::int64_t)(map_size/size + add) * map_size);
	copy_vec(atomic_D, D);

	return D;
}

void recv_disp_calc(std::int64_t map_size, std::int64_t size, 
	std::vector<int> &recvcount, std::vector<int> &displs){
	for(std::int64_t i = 0; i < size; ++i){
		i < map_size%size ? recvcount[i] = (map_size/size + 1) * \
		map_size : recvcount[i] = (map_size/size) * map_size;
		if(i < size-1) displs[i+1] += displs[i] + recvcount[i];
	}
}

void copy_shortest_path_to_map(std::map <std::string, Article> &art_list_map, 
					std::int64_t size, std::vector<std::int64_t> &D_complete){
	std::vector<std::int64_t> rank_start(size,0);
	for(std::int64_t i = 1; i < size; ++i){
		std::int64_t temp;

		i-1 < art_list_map.size()%size ? 
				temp = (art_list_map.size()/size + 1) * art_list_map.size() :
				temp = (art_list_map.size()/size) * art_list_map.size();

		rank_start[i] = rank_start[i-1] + temp;
	}

	std::int64_t it_counter = 0;
	std::int64_t j = 0;
	for(auto it = art_list_map.begin(); it != art_list_map.end(); ++it){
		std::int64_t D_pos = rank_start[it_counter%size];
 		(it->second).weg.insert((it->second).weg.begin(), D_complete.begin() + \
 			D_pos + j * art_list_map.size(), D_complete.begin() + D_pos + j * \
 			art_list_map.size() + art_list_map.size());
 		++it_counter;
 		if(it_counter%size == 0) ++j;		
	}
}

void Gather_data(std::int64_t size, std::int64_t rank, 
	std::int64_t map_size, std::vector<std::int64_t> &D, 
	std::map <std::string, Article> &art_list_map, MPI_Comm comm){

		std::int64_t add;
		rank < map_size%size ? add = 1 : add = 0;

	std::vector<std::int64_t> D_complete;
	if(rank == 0) {D_complete.resize(
		((map_size%size) * pow(map_size,2)/size + 1) + \
		((size - (map_size%size)) * pow(map_size,2)/size));
	}

	std::vector<int> recvcount(size);
	std::vector<int> displs(size, 0);
	recv_disp_calc(map_size, size, recvcount, displs);

	MPI_Barrier(comm);
	MPI_Gatherv(D.data(), (map_size/size + add) * map_size, MPI_INT64_T, 
		D_complete.data(), recvcount.data(), displs.data(), 
		MPI_INT64_T, 0, comm);

	if(rank == 0) copy_shortest_path_to_map(art_list_map, size, D_complete);
}

void print_path(Article& art1, Article& art2, 
				std::map <std::string, Article> &art_list_map){
	std::int64_t path_length = art1.weg[art2.id];
	std::int64_t path = path_length;
	std::int64_t dist = 0;
	std::cout << art2.title << " "<< art2.id << " " << path_length << std::endl;
	if(!(dist < path_length)) std::cout << "No path exists"<< std::endl;
	while(dist < path_length){
		for(size_t i = 0; i < art1.links_in_article_pointer.size(); ++i){
		if((path - 1 ) == art1.links_in_article_pointer[i]->weg[art2.id]) {
			std::cout << art1.links_in_article_pointer[i]->title << " -> ";
			art1 = art_list_map[art1.links_in_article_pointer[i]->title];
			--path;
		}else if(path -1 == 0){
			std::cout << art1.links_in_article_pointer[i]->title << std::endl;
		}else{
			std::cout << "Path search failed" << std::endl;
		}
		++dist;
		}
	}
}