#ifndef BAG_OF_WORDS_H
#define BAG_OF_WORDS_H

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <Eigen/Sparse>

#include "wikiarticle.hpp"

typedef Eigen::Triplet<double> T;

// adds element to triple vector for each word in article
// triple is used to build sparse matrix
void wmap_pos_dict (
	std::map<std::string, size_t>&,
	//std::set<std::string>&,
	std::map<std::string, std::int64_t>&,
	std::vector<T>&,
	std::int64_t);

// iterate over every article and create bag of words matrix
// using triple vector
void create_word_matrix(
	Eigen::SparseMatrix<double>&,
	std::map <std::string, Article>&,
	std::map<std::string, std::int64_t>&);

// calculates the inverse sum of each column
// inverse is required for normalization 
// multiplication with inverse is "cheaper" than dividing by sum 
void calc_col_sums(
	Eigen::SparseMatrix<double>&,
	Eigen::VectorXd&);

// calculates the inverse sum of each row
// inverse is required for normalization 
// multiplication with inverse is "cheaper" than dividing by sum
void calc_row_sums(
	Eigen::SparseMatrix<double>&,
	Eigen::VectorXd&);

// calculate dot product of each normalized column (describing article)
// with every single other column
void compare_all_text(
	Eigen::SparseMatrix<double>&);

// normalize each element of matrix by dividing 
// by its row_sum and col_sum
void norm_mat(
	Eigen::SparseMatrix<double>&,
	Eigen::VectorXd&,
	Eigen::VectorXd&);

// dot product of all column combinations
// result is an upper triangular matrix without elements on diagonal
void dot_prod(
	Eigen::SparseMatrix<double>&,
	Eigen::SparseMatrix<double>&);

// wrapper for all previous functions
// requires initialized sparese matrix dict.size x art_map.size
void bag_of_words(
	Eigen::SparseMatrix<double>&,
	Eigen::SparseMatrix<double>&);

#endif