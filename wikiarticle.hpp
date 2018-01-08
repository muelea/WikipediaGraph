#ifndef WIKIARTICLE_H
#define WIKIARTICLE_H

#include <atomic>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

class Article
{
public:
	std::string title;
	std::string text;
	std::int64_t id;

	std::vector<std::string> links_in_article;
	std::vector<Article *> links_in_article_pointer;

	//http://stackoverflow.com/questions/11725413/correct-way-to-initialize-vector-member-variable
	//Article() : weg(100) {}
	std::vector<std::int64_t> weg;

	std::vector<std::pair<std::string, std::int64_t>> words_in_article;
	std::vector<std::string> categorys_in_article;

	std::map<std::string, size_t> word_count;


	// filter all links and categories in text
	// links in Wikipedia XML dump are marked by:
	// 		[[link]] 
	// e.g.: "Text bla bla [[link1]] shit chat ... [[link2]] ..."
	void Find_links_in_article_pointer(
		std::map <std::string, Article>&);

	// filter each word in article 
	// and adds new words to dictionary 
	void Count_words_in_article(
		std::map<std::string, std::int64_t>&);
};

typedef std::vector<Article> Article_list;

// certain links dont use their related page title
// function filters page title of these links
void handle_pipe(std::string&);

// certain links contain links to chapter of linked page
// function filters linked page (ignores chapter link)
void handle_yolo(std::string&);

void lower_string(std::string&);

void cut_sq_brackets(std::string&);

// execute search for links and words for 
// each article text on whole map
// iterates over whole map
// and executes functions 
//	-	Count_words_in_article
//	- 	Find_links_in_article_pointer
void find_wiki_link_map(
	std::map <std::string, Article>&, 
	std::map<std::string, std::int64_t>&);

#endif