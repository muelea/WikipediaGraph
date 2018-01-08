#ifndef IS_MIC

#include <algorithm>
#include <iterator>
#include <map>
#include <string>

#include <boost/regex.hpp>

#include "wikiarticle.hpp"


void handle_pipe(std::string& string) {
	boost::regex r_prev_pipe(".+(?<=\\|)");
	boost::smatch pipe_clean;
	boost::regex_search(string, pipe_clean, r_prev_pipe);

	string = (pipe_clean.str()).substr(0, (pipe_clean.str()).size() - 1);
}

void handle_yolo(std::string& string) {
	boost::regex r_prev_pipe(".+(?<=#)");
	boost::smatch pipe_clean;
	boost::regex_search(string, pipe_clean, r_prev_pipe);

	string = (pipe_clean.str()).substr(0, (pipe_clean.str()).size() - 1);
}

void cut_sq_brackets(std::string& word) {
	word = word.substr(2, (word).size() - 4);
}

void Article::Find_links_in_article_pointer(
	std::map <std::string, Article>& art_list_map){

	std::string pattern_link("(?<!'')(\\[\\[(.*?)\\]\\])(?<!'')");
	boost::regex r_link(pattern_link);

	std::string pattern_file("(\\[\\[File:)|(\\[\\[Datei:)");
	boost::regex r_file(pattern_file);

	std::string pattern_image("(\\[\\[Image:)|(\\[\\[Bild:)");
	boost::regex r_image(pattern_image);

	std::string pattern_category("(\\[\\[Category:)|(\\[\\[Kategorie:)");
	boost::regex r_category(pattern_category);

	std::string pattern_wiki("\\[\\[(..|:wikt|wikt|Wikipedia):");
	boost::regex r_wiki(pattern_wiki);

 	boost::sregex_iterator next(text.begin(), text.end(), r_link);

 	boost::sregex_iterator end;

 	bool push_category = 0;

 	while(next != end) {
 		boost::smatch match = *next;

 		std::string link = match.str();

 		std::string match_str;

 		if(boost::regex_search(link, r_category)) {
			// check if link is a category
 			push_category = 1;

			match_str = match.str();

		} else if (boost::regex_search(link, r_file)) {
			// check if link is a picture that might contain additional links

			// get rid of first initial [[ to find possible 
			// link in file description
			link = link.substr(2);
			boost::smatch link_in_file;
			
			if(boost::regex_search(link, link_in_file, r_link)) {
				match_str = link_in_file.str();
			} else {
				next++;
				continue;
			}

		} else if (boost::regex_search(link, r_image)) {
			// check if link is a picture that might contain additional links

			// get rid of first initial [[ to find possible 
			// link in file description
			link = link.substr(2);
			boost::smatch link_in_image;
			
			if(boost::regex_search(link, link_in_image, r_link)) {
				match_str = link_in_image.str();
			} else {
				next++;
				continue;
			}

		} else if (boost::regex_search(link, r_wiki)) {
			// check if link is internal wiki link, or link to wiktionary
			// or strange links with two literals after category section
			// delete them all!

			next++;
			continue;

		} else {
			// just a boring link
			match_str = match.str();
		}

		cut_sq_brackets(match_str);

		boost::regex r_pipe("\\|");
		if(boost::regex_search(match_str, r_pipe))
			handle_pipe(match_str);

		boost::regex r_yolo("#");
		if(boost::regex_search(match_str, r_yolo)) {
			handle_yolo(match_str);
			if (!match_str.size()) {
				next++;
				continue;
			}
		}

 		//Pointer version
		std::map<std::string, Article>::iterator it;
 		it = art_list_map.find(match_str);
 		if((it != art_list_map.end()) && !push_category){
 			links_in_article.push_back(match_str);
	 		links_in_article_pointer.push_back(&(art_list_map[match_str]));
	 	} 

		if (push_category) {
			categorys_in_article.push_back(match_str.substr(9));
			push_category = 0;
		}

 		next++;
 	}
}

void lower_string(std::string& string) {
	for(auto &i : string)
		i = std::tolower(i);
}

void Article::Count_words_in_article(
	std::map<std::string, std::int64_t>& dict) {
	std::string pattern_word("[a-zA-Z]+");

	boost::regex r_word(pattern_word);

	boost::sregex_iterator word_next(text.begin(), text.end(), r_word);
 	boost::sregex_iterator end;

	while(word_next != end) {
 		boost::smatch match = *word_next;

 		std::string word = match.str();

		lower_string(word);
		
		dict[word];

		++word_count[word];

 		word_next++;
 	} 	
}

void find_wiki_link_map(
	std::map <std::string, Article> &art_list_map,
	std::map<std::string, std::int64_t>& dict) {

	std::vector<Article*> map_iter;
	for(auto it = art_list_map.begin(); it != art_list_map.end(); ++it)
		map_iter.push_back(&(it->second));


	#pragma omp parallel
	{
		std::map<std::string, std::int64_t> private_dict;
		
		#pragma omp for
		for(std::size_t i = 0; i < map_iter.size(); ++i) {
			map_iter[i]->id = i;

			map_iter[i]->Count_words_in_article(private_dict);
			map_iter[i]->Find_links_in_article_pointer(art_list_map);
		}

		#pragma omp critical
		dict.insert(private_dict.begin(), private_dict.end());
	}
}

#endif  // IS_MIC