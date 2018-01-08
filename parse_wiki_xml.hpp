#ifndef PARSE_WIKI_XML_HPP_
#define PARSE_WIKI_XML_HPP_

#include <fstream>

#include "wikiarticle.hpp"

// Structure of Wikipedia XML dump

// <mediawiki>
// 	<page>
// 		<title>Page1</title>
// 		<revision>
// 			<text> Page Text ... </text>
// 		</revision>
// 	</page>
// 	<page>
// 		<title>Page2</title>
// 		<revision>
// 			<text> Bla bla ..</text>
// 		</revision>
// 	</page>
// 	<page>
// 		<title>Page3</title>
// 		<revision>
// 			<text> ... </text>
// 		</revision>
// 	</page>
// </mediawiki>

// boost parser creates following tree for file above

//						mediawiki
//			/ 				|					\
//		page 		  	   page 				page
// 		/  \			   /  \					/  \
// 	title  revison	 	    ...				title  revision
//			  | 			 |						  |
//			 text 			... 					 text


// function reads xml file and creates boost::ptree
// after creation of ptree function traveres tree and
// creats an element in a map for each page
// key of each map element is the title
// element of map is class Article (defined in wikiarticle.hpp)
// text of the page is saved in Article 
void read_title_text_wiki_map(
	std::istream& xml_file, 
	std::map <std::string, Article>& art_list_map);

#endif