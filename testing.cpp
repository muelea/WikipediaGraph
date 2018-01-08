#include "gtest/gtest.h"
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <atomic>

#include "bag_of_words.hpp"
#include "dynatomicvec.hpp"
#include "parse_wiki_xml.hpp"
#include "shortest_path.hpp"
#include "wikiarticle.hpp"

class ArticleTest : public ::testing::Test
{
protected:
	virtual void SetUp(){
	}
	virtual void TearDown() {
	}
};


TEST_F(ArticleTest, links_in_article_length) {
std::ifstream input("../kill-host.xml");
std::map <std::string, Article> art_list_map;	
read_title_text_wiki_map(input, art_list_map);

	art_list_map["Aaron Goldberg"].Find_links_in_article_pointer(art_list_map);
	EXPECT_EQ(art_list_map["Aaron Goldberg"].links_in_article_pointer.size(),4);
}

TEST_F(ArticleTest, Find_links_in_article_complete) {
	std::ifstream input("../kill-host.xml");
	std::map <std::string, Article> art_list_map;	
	read_title_text_wiki_map(input, art_list_map);
	Article& art = art_list_map["Aaron Goldberg"];
	art.Find_links_in_article_pointer(art_list_map);

	std::vector<std::string> links_manual = 
	{"Joshua Redman", "Joshua Redman", "Daniel Dennett", "Joshua Redman"};

	
	for (size_t i = 0; i < links_manual.size(); ++i) {
  		EXPECT_EQ((art.links_in_article_pointer[i])\
  			->title, links_manual[i]) << "links_in_article and \
  				link_in_article_manual differ at index " << i;

	}
}

/*Check if shortest path between two Articles is correct*/
TEST_F(ArticleTest, shortest_path) {
	std::ifstream input("../wikipedia_love.xml");
	std::map <std::string, Article> art_list_map;	
	read_title_text_wiki_map(input, art_list_map);

	std::vector<std::int64_t> links_in_article_vec;
	std::vector<std::int64_t> offset;
	std::int64_t links_in_article_vec_size;
	std::int64_t offset_size;
	std::int64_t map_size;

	std::map<std::string, std::int64_t> dict;
	#ifndef IS_MIC
		find_wiki_link_map(art_list_map, dict);
	#endif

	build_small_graph(links_in_article_vec, offset, 
		links_in_article_vec_size, offset_size, art_list_map);
	
	DynAtomicArray<std::int64_t> atomic_D((std::int64_t)(art_list_map.size() *\
		art_list_map.size()));
	atomic_D.setTo(-1);

	shortest_path_vec_vec(art_list_map["Bisexuality"].id, 
		art_list_map["Puppy love"].id, 0, atomic_D, 
		art_list_map["Bisexuality"].id, art_list_map.size(), 
		links_in_article_vec, offset);

	EXPECT_EQ(atomic_D[art_list_map["Bisexuality"].id * art_list_map.size() + 
		art_list_map["Puppy love"].id], 3);

}

// BAG OF WORDS TESTS

TEST(Col_sum_Test, check_correct_col_sums) {
	// define test matrix
	// 1 0
	// 0 3
	// 4 0

	std::vector<int> row_vec {0, 2, 1};
	std::vector<int> col_vec {0, 0, 1};
	std::vector<double> value_vec {1, 4, 3};

	std::vector<T> triple_list;

	for(size_t i = 0; i < value_vec.size(); ++i) 
		triple_list.push_back(T(row_vec[i], col_vec[i], value_vec[i]));

	Eigen::SparseMatrix<double> mat(3, 2);
	mat.setFromTriplets(triple_list.begin(), triple_list.end());

	Eigen::VectorXd col_sums(2);
	calc_col_sums(mat, col_sums);

	// define expected result of calc_col_cums
	// 0.2000
	// 0.3333

	Eigen::VectorXd col_sums_match(2);
	col_sums_match << 1 / 5.0, 1 / 3.0;

	for(size_t i = 0; i < col_sums_match.size(); ++i)
		EXPECT_EQ(col_sums[i], col_sums_match[i]);
}

TEST(Row_sum_Test, check_correct_row_sums) {
	// define test matrix
	// 1 0
	// 0 3
	// 4 0

	std::vector<int> row_vec {0, 2, 1};
	std::vector<int> col_vec {0, 0, 1};
	std::vector<double> value_vec {1, 4, 3};

	std::vector<T> triple_list;

	for(size_t i = 0; i < value_vec.size(); ++i) 
		triple_list.push_back(T(row_vec[i], col_vec[i], value_vec[i]));

	Eigen::SparseMatrix<double> mat(3, 2);
	mat.setFromTriplets(triple_list.begin(), triple_list.end());

	Eigen::VectorXd row_sums(2);
	calc_row_sums(mat, row_sums);

	// define expected result of row_col_cums
	// 1.0000
	// 0.3333
	// 0.2500

	Eigen::VectorXd row_sums_match(3);
	row_sums_match << 1 / 1.0, 1 / 3.0, 1 / 4.0;

	for(size_t i = 0; i < row_sums_match.size(); ++i)
		EXPECT_EQ(row_sums[i], row_sums_match[i]);
}

TEST(Norm_mat_Test, check_correct_norm_mat) {
	// define test matrix
	// (col and rows sums doesnt fit matrix values)
	// (simplyfied for testing)

	// ---------------------------
	//    matrix   | row_sum_value
	// 20       40 | 1 / 1 
	//  8        0 | 1 / 2
	// 24      144 | 1 / 3
	// ----------------------------
	// 1 / 2 | 1 / 4  col_sum_value

	std::vector<int> row_vec {0, 1, 2, 0, 2};
	std::vector<int> col_vec {0, 0, 0, 1, 1};
	std::vector<double> value_vec {20, 8, 24, 40, 144};

	std::vector<T> triple_list;

	for(size_t i = 0; i < value_vec.size(); ++i) 
		triple_list.push_back(T(row_vec[i], col_vec[i], value_vec[i]));

	Eigen::SparseMatrix<double> mat(3, 2);
	mat.setFromTriplets(triple_list.begin(), triple_list.end());

	Eigen::VectorXd col_sums(2);
	col_sums << 1 / 2.0, 1 / 4.0;

	Eigen::VectorXd row_sums(3);
	row_sums << 1 / 1.0, 1 / 2.0, 1 / 3.0;

	Eigen::SparseMatrix<double> result(3, 2);
	norm_mat(mat, col_sums, row_sums);

	// define expected results of dot prod
	// 10 10 
	//  2  0
	//  4 12  

	std::vector<int> row_result_vec {0, 1, 2, 0, 2};
	std::vector<int> col_result_vec {0, 0, 0, 1, 1};
	std::vector<double> value_result_vec {10, 2, 4, 10, 12};

	std::vector<T> triple_list_result;

	for(size_t i = 0; i < value_result_vec.size(); ++i) 
		triple_list_result.push_back(T(
			row_result_vec[i], 
			col_result_vec[i], 
			value_result_vec[i]));

	Eigen::SparseMatrix<double> result_match(3, 2);
	result_match.setFromTriplets(
		triple_list_result.begin(), 
		triple_list_result.end());	

	for(size_t i = 0; i < result_match.rows(); ++i)
		for(size_t j = 0; j < result_match.cols(); ++j)
			EXPECT_EQ(mat.coeffRef(i, j), result_match.coeffRef(i, j));
}

TEST(dot_prod_Test, check_correct_dot_prod) {
	// define test matrix
	// 2.5 4.0 0.0
	// 0.0 3.0 0.0
	// 2.0 1.5 2.0

	std::vector<int> row_vec {0, 3, 0, 1, 3, 2, 3};
	std::vector<int> col_vec {0, 0, 1, 1, 1, 2, 2};
	std::vector<double> value_vec {2.5, 2, 4, 3, 1.5, 2, 2};

	std::vector<T> triple_list;

	for(size_t i = 0; i < value_vec.size(); ++i) 
		triple_list.push_back(T(row_vec[i], col_vec[i], value_vec[i]));

	Eigen::SparseMatrix<double> mat(4, 3);
	mat.setFromTriplets(triple_list.begin(), triple_list.end());

	Eigen::SparseMatrix<double> result(3, 3);
	dot_prod(mat, result);

	// define expected results of dot prod
	// 0.0 13.0 2.0
	// 0.0  0.0 1.5
	// 0.0  0.0 0.0

	std::vector<int> row_result_vec {0, 0, 1};
	std::vector<int> col_result_vec {1, 2, 2};
	std::vector<double> value_result_vec {13, 4, 3};

	std::vector<T> triple_list_result;

	for(size_t i = 0; i < value_result_vec.size(); ++i) 
		triple_list_result.push_back(T(
			row_result_vec[i], 
			col_result_vec[i], 
			value_result_vec[i]));

	Eigen::SparseMatrix<double> result_match(3, 3);
	result_match.setFromTriplets(
		triple_list_result.begin(), 
		triple_list_result.end());	

	for(size_t i = 0; i < result_match.rows(); ++i)
		for(size_t j = 0; j < result_match.cols(); ++j)
			EXPECT_EQ(result.coeffRef(i, j), result_match.coeffRef(i, j));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}