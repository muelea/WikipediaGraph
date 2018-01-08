#include <atomic>
#include <chrono>
#include <iostream>
#include <sstream>

#include <mpi.h>

#include "bag_of_words.hpp"
#include "MPI_class.hpp"
#include "parse_wiki_xml.hpp"
#include "shortest_path.hpp"
#include "wikiarticle.hpp"

using std::chrono::duration;
using std::chrono::system_clock;


int main(int argc, char** argv) {
	if (argc != 2) {
		std::cout << "./main <wiki_dump.xml>" << std::endl;
		return -1;
	}
	try {
		MPIManager mpiMan(argc, argv);
		MPI_Comm comm = MPI_COMM_WORLD;
		int size, rank;
		MPI_Comm_size(comm, &size);
		MPI_Comm_rank(comm, &rank);

		//Information send to processes
		std::vector<std::int64_t> links_in_article_vec;
		std::vector<std::int64_t> offset;
		std::int64_t links_in_article_vec_size;
		std::int64_t offset_size;

		//Map of articles
		std::map <std::string, Article> art_list_map;
		std::int64_t map_size;

		Eigen::SparseMatrix<double> result_bow;

		if(rank == 0){

			std::string input_string = argv[1];

			// std::ifstream input("../wikipedia_liebe.xml");
			// std::ifstream input("../wikipedia_love.xml");
			// std::ifstream input("../Wikipedia-english-men.xml");
			// std::ifstream input("../Wikipedia-cornell.xml");
			// std::ifstream input("../dewiki-latest-pages-articles.xml");

			std::ifstream input(input_string);
			std::cout << std::endl << "Read xml ..." << std::endl;
			read_title_text_wiki_map(input, art_list_map);

			std::cout << "Filter graph information (links; categories; words)..." 
					  << std::endl;				
			std::map<std::string, std::int64_t> dict;
			#ifndef IS_MIC
				find_wiki_link_map(art_list_map, dict);
			#endif

			map_size = art_list_map.size();

			std::cout << std::endl << "Wikipedia information:" << std::endl
					  << "\t articles in wiki dump: \t" 
					  << map_size << std::endl;

			// enumerate each word in map with index
			// required to identify position of a word in dict to place
			// necessary to place each word in the bag of words matrix
			std::int64_t n = 0;
			for(auto &word : dict) 
				word.second = n++;

			// BAG OF WORDS
			std::cout << std::endl << "Start bag of words ..." << std::endl;
			Eigen::SparseMatrix<double> bag_matrix(dict.size(), 
				art_list_map.size());

			std::cout << "Create word matrix ..." << std::endl;
			create_word_matrix(bag_matrix, art_list_map, dict);

			result_bow.resize(bag_matrix.cols(), bag_matrix.cols());

			std::cout << "Calculate bag of words ..." << std::endl;
			auto start_bow = system_clock::now();

			bag_of_words(bag_matrix, result_bow);

			auto end_bow = system_clock::now();
			const double time_bow = \
				duration<double>(end_bow - start_bow).count();

			// BAG OF WORDS SUMMARY
			std::cout << std::endl << "Bag of words summary:" << std::endl		  
					  << "\t words in dictionary: \t \t" 
					  << dict.size() << std::endl
			 		  << "\t calculated dot products: \t" 
					  << result_bow.nonZeros() << std::endl
					  << "\t time bag of words: \t \t"
					  << time_bow << " s" << std::endl << std::endl;
		
			std::cout << "Prepare graph to send via MPI ..." << std::endl;
			build_small_graph(links_in_article_vec, offset, 
				links_in_article_vec_size, offset_size, art_list_map);
		}
		
		//Graph info is send to processes
		MPI_Barrier(comm);
		if(rank == 0) std::cout << "Send graph via MPI ..." << std::endl;
		send_map_info(rank, map_size, links_in_article_vec_size, offset_size, 
			links_in_article_vec, offset, comm);

		//Information about graph and graph search which will be printed 
		std::vector<std::int64_t> graph_info(4, 0);

		MPI_Barrier(comm);
		auto start_sp = system_clock::now();
		if(rank == 0) std::cout << "Start graph search ..." << std::endl;
		std::vector<std::int64_t> D = Graph_search(rank, map_size, size, 
			graph_info,links_in_article_vec,offset);
		auto end_sp = system_clock::now();
		const double time_sp = duration<double>(end_sp-start_sp).count();

		//Gather data from processes
		MPI_Barrier(comm);
		if(rank == 0) std::cout << "Gather shortest path data ..." << std::endl;
		Gather_data(size, rank, map_size, D, art_list_map, comm);

		std::vector<std::int64_t> graphinfo(4);
		MPI_Reduce(graph_info.data(), graphinfo.data(), 4, MPI_INT64_T, MPI_SUM, 
			0, comm);

		if(rank == 0){
			std::cout << std::endl << "Shortest path summary:" << std::endl
					  << "\t searched pairs:  \t\t" 
			 		  << graphinfo[0] << std::endl
					  << "\t articles without path: \t" 
					  << graphinfo[3] << std::endl
			 		  << "\t average distance: \t\t" 
					  << (double)graphinfo[1]/graphinfo[2] << std::endl
					  << "\t time shortest path: \t \t"
					  << time_sp << " s" << std::endl;
		}
		
	} catch (std::exception const& e) {
	std::cerr << e.what() << std::endl;
	return -1;
	}
	return 0;
}