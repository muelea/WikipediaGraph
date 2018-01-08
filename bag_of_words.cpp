#include "bag_of_words.hpp"

// http://stackoverflow.com/questions/8691459/how-do-i-parallelize-a-for-loop-through-a-c-stdlist-using-openmp
// https://stackoverflow.com/questions/18669296/c-openmp-parallel-for-loop-alternatives-to-stdvector/18671256#18671256

void wmap_pos_dict (
	std::map<std::string, size_t>& word_count,
	std::map<std::string, std::int64_t>& dict,
	std::vector<T>& triple_list,
	std::int64_t col) {

	for(auto word : word_count) 
		triple_list.push_back(T(dict[word.first], col, word.second));

}

void create_word_matrix(
	Eigen::SparseMatrix<double>& mat,
	std::map <std::string, Article>& art_list_map,
	std::map<std::string, std::int64_t>& dict) {

	std::vector<T> triple_list;

	std::vector<Article*> art_iter;
	for(auto it = art_list_map.begin(); it != art_list_map.end(); ++it)
		art_iter.push_back(&(it->second));

	#pragma omp parallel shared(art_list_map, dict)
	{
		std::vector<T> private_triple_list;
		#pragma omp for 
		for(size_t i = 0;
			i < art_iter.size(); ++i) {
			wmap_pos_dict(art_iter[i]->word_count, dict, 
				private_triple_list, art_iter[i]->id);

		}
		#pragma omp critical
		triple_list.insert(triple_list.end(), 
			private_triple_list.begin(),
			private_triple_list.end());
	}

	mat.setFromTriplets(triple_list.begin(), triple_list.end());
}

void calc_col_sums(
	Eigen::SparseMatrix<double>& mat,
	Eigen::VectorXd& col_sums) {

	for (int k = 0; k < mat.cols(); ++k)
  		col_sums[k] = mat.col(k).sum();
	
	col_sums = col_sums.cwiseInverse();
}

void calc_row_sums(
	Eigen::SparseMatrix<double>& mat,
	Eigen::VectorXd& row_sums) {

	Eigen::VectorXd vec = Eigen::VectorXd::Ones(mat.cols());
	row_sums = mat * vec;
	row_sums = row_sums.cwiseInverse();
}

void norm_mat(
	Eigen::SparseMatrix<double>& mat,
	Eigen::VectorXd& col_sums,
	Eigen::VectorXd& row_sums) {

	#pragma omp parallel for
	for(int i = 0; i < mat.cols(); ++i)
		mat.col(i) = mat.col(i).cwiseProduct(row_sums) * \
		 col_sums.coeffRef(i);
}

void dot_prod(
	Eigen::SparseMatrix<double>& mat,
	Eigen::SparseMatrix<double>& result) {

	std::vector<T> triple_list;

	#pragma omp parallel
	{
		std::vector<T> private_triple_list;
		#pragma omp for
		for(int i = 0; i < mat.cols() - 1; ++i) {
			Eigen::SparseVector<double> value_vec(mat.cols() - i - 1);
			value_vec = mat.col(i).transpose() * \
				mat.rightCols(mat.cols() - i - 1);
			for(int k = 0; k < value_vec.size(); ++k) 
			  	private_triple_list.push_back(
			  		T(i, i + 1 + k, value_vec.coeffRef(k))
		  		);
		}

		#pragma omp critical
		triple_list.insert(triple_list.end(), 
			private_triple_list.begin(),
			private_triple_list.end());
	}

	result.setFromTriplets(triple_list.begin(), triple_list.end());
}

void bag_of_words(
	Eigen::SparseMatrix<double>& mat,
	Eigen::SparseMatrix<double>& result) {

	Eigen::VectorXd col_sums(mat.cols());
	calc_col_sums(mat, col_sums);

	Eigen::VectorXd row_sums(mat.rows());
	calc_row_sums(mat, row_sums);

	norm_mat(mat, col_sums, row_sums);

	dot_prod(mat, result);
}