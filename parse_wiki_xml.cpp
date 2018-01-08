#include <fstream>
#include <locale>
#include <string>
#include <vector>

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "parse_wiki_xml.hpp"

// https://akrzemi1.wordpress.com/2011/07/13/parsing-xml-with-boost/
using article_map = std::map <std::string, Article>;

void read_title_text_wiki_map(
	std::istream& xml_file, 
	std::map <std::string, Article>& art_list_map) {
	using boost::property_tree::ptree;

	ptree pt;

	read_xml(xml_file, pt);

	std::int64_t i = 0;

	BOOST_FOREACH(ptree::value_type const& wiki_children, 
		pt.get_child("mediawiki")) {

		if (wiki_children.first == "page") {
			Article new_article;			
			new_article.title = wiki_children.second.get<std::string>("title");

			BOOST_FOREACH(ptree::value_type const& page_children, 
				wiki_children.second.get_child("revision")) {
				if (page_children.first == "text") {
					new_article.text = page_children.second.data();
					break;
				}
			}

			new_article.id = i++;
			art_list_map.insert (
				std::pair<std::string, Article>(new_article.title, 
					new_article));
		}
	}
}