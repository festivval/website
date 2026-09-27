#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>

static const size_t MAX_TAGS = 5;
std::set<std::string> tags;
std::map<std::string, std::vector<std::string>> groups;

std::string dashify(const std::string& s) {
	if(s.length() == 0) return s;

	// trim
	size_t begin(0), end;
	for(const char& c : s) {
		if(c != ' ') break;
		begin++;
	}
	for(end = s.length()-1; end > 0; end--) {
		if(s.at(end) != ' ') break;
	}

	// remove dashes
	std::string r = "";
	for(const char& c : s.substr(begin, end-begin+1)) {
		if(c == '[' || c == ']' || c=='*') continue;
		r += (c == ' ')? '-' : c;
	}
	return r;
}
void dashify(std::vector<std::string>& strs) {
	for(size_t i = 0; i < strs.size(); i++) {
		strs[i] = dashify(strs[i]);
	}
}

std::string linkify(const std::string& line) {
	size_t ink_open = line.find('(');
	size_t ink_close = line.find(')');
	size_t link_open = line.find('<');
	size_t link_close = line.find('>');

	// error checking
	if(ink_open == std::string::npos || ink_close == std::string::npos || link_open == std::string::npos || link_close == std::string::npos) return line;
	if(ink_close < ink_open) {
		std::cerr << "unclosed ink brackets\n";
		return line;
	}
	if(link_close < link_open) {
		std::cerr << "unclosed link brackets\n";
		return line;
	}

	std::string pre_link = line.substr(0, ink_open);
	std::string link_url = line.substr(link_open+1, link_close-link_open-1);
	std::string link_txt = line.substr(ink_open+1, ink_close-ink_open-1);
	std::string post_link = line.substr(link_close+1);
	return pre_link + "<a href=" + link_url + ">" +  link_txt + "</a>" + post_link;
}

int write_head(std::ofstream& ofs, std::string& styleout, std::string& scriptout) {
	ofs << "<!DOCTYPE html>\n<html>\n<head>\n";
	ofs << "<link rel=\"stylesheet\" type=\"text/css\" href=\"" << styleout << "\">\n";
	ofs << "<link rel=\"stylesheet\" type=\"text/css\" href=\"" << "sidebar.css" << "\">\n";
	ofs << "<script type=\"text/javascript\" src=\"https://code.jquery.com/jquery-4.0.0.slim.min.js\"></script>\n";
	ofs << "<script type=\"text/javascript\" src=\"" << scriptout << "\"></script>\n";
	ofs << "<script type=\"text/javascript\" src=\"script.js\"></script>\n";
	ofs << "</head>\n";
	return 0;
}

int write_foot(std::ofstream& ofs) {
	ofs << "</body>\n</html>";
	return 0;
}

int get_items(std::string& line, std::vector<std::string>& items, size_t start, char open_char, char close_char) {
	size_t open = (open_char == '\0')? start : line.find(open_char, start);
	size_t close = (close_char == '\0') ? line.find(open_char, start+1) : line.find(close_char, start);
	while(open != std::string::npos) {
		if(close == std::string::npos) {
			if(open_char == '\0' || close_char == '\0') {
				items.push_back(line.substr(open));
				break;
			}
			std::cerr << "unclosed item\n";
			return 1;
		}
		
		if(close_char == '\0') close--;
		items.push_back(line.substr(open, close-open+1));
		if(close == line.length()-1) break;
		open = (open_char == '\0')? close+1 : line.find(open_char, close);
		close = (close_char == '\0')? line.find(open_char, open+1): line.find(close_char, close+1);
	}

	return 0;
}

int write_filters(std::ofstream& ofs) {
	ofs << "<div id=\"sidebar\"><details>\n";
	ofs << "    <summary>Filter subject</summary>\n";
	ofs << "    <button type=\"button\" id=\"all-filters\">select all</button>\n";
	ofs << "    <button type=\"button\" id=\"nil-filters\">deselect all</button>\n";
	for(auto& g : groups) {
		if(g.first == "") continue;
		std::string g_class = g.first + "-group";
		ofs << "    <div>\n";
		ofs << "        <input type=\"checkbox\" class=\"filter\" id=\"" << g_class << "\" name=\"" << g_class << "\" checked />\n";
    	ofs << "        <label for=\"" << g_class << "\">" << g.first << "</label>\n";
		ofs << "    </div>\n";
	}
	ofs << "</details>\n";
	
	ofs << "<details>\n";
	ofs << "    <summary>Filter tool</summary>\n";
	ofs << "    <button type=\"button\" id=\"all-tags\">select all</button>\n";
	ofs << "    <button type=\"button\" id=\"nil-tags\">deselect all</button>\n";
	for(auto& g : tags) {
		if(g == "") continue;
		std::string g_class = g;
		ofs << "    <div>\n";
		ofs << "        <input type=\"checkbox\" class=\"tfilter\" id=\"" << g_class << "\" name=\"" << g << "\" checked />\n";
    	ofs << "        <label for=\"" << g_class << "\">" << g << "</label>\n";
		ofs << "    </div>\n";
	}
	ofs << "</details></div>\n";
	return 0;
}

int write_main(std::string& file, std::string& out, std::string& styleout, std::string& scriptout) {
	int err;
    bool list(false);
    std::string line;
	std::vector<std::string> items;

    // open files
    std::ifstream ifs(file);
    if(!ifs.is_open()) {
        std::cerr << "failed to open " << file << "\n";
        return 1;
    }
    std::ofstream ofs(out);
	
	// write head
	write_head(ofs, styleout, scriptout);

    while(std::getline(ifs, line)) {
        // empty line
        if(line.empty()) {
           ofs << "<br>\n";
           continue;
        }

        // set list mode
        if(!list && line.at(0)=='-') {
            ofs << "<ul>\n";
            list = true;
        } else if(list && line.at(0)!='-') {
            ofs << "</ul>\n";
            list = false;
        }
        
        // output text
		
		// FILTERS
		if(line.length() >= 2 && line.at(0) == '*' && line.at(1) == '*') {
			write_filters(ofs);
			continue;
		}

		// NON-LIST
		if(!list) {
            ofs << "<h3>" << line << "</h3>\n";
			continue;
		}
        
		// LIST

		// item
		size_t org_begin = line.find('[');
		size_t tag_begin = line.find('*');
		size_t item_end = (org_begin == std::string::npos)? ((tag_begin == std::string::npos)? line.length()-1 : tag_begin-2) : org_begin-2;
		ofs << "    <li>" << linkify(line.substr(2, item_end)) << "\n";

		// [org]
		items.clear();
		err = get_items(line, items, 0, '[', ']');
		if(err) return 1;
		for(const auto& org : items) {
			ofs << "        <span class=\"org " << dashify(org) << "\">" << org << "</span>\n";
		}
		
		// *tags
		items.clear();
		err = get_items(line, items, 0, '*', '\0');
		if(err) return 1;
		for(const auto& tag : items) {
			std::string dtag = dashify(tag);
			if(dtag.length() == 0) continue;
			tags.insert(dtag);
			ofs << "        <span class=\"tag " << dtag << "\">" << tag.substr(1) << "</span>\n";
		}
		// no tags marker
		if(items.empty()) {
			ofs << "        <span class=\"tag tagless\"></span>\n";
		}

		// end list item
		ofs << "    </li>\n";
    }
    
	// write foot
	write_foot(ofs);
	ofs.close();

	return 0;
}

int write_style(std::string& style, std::string& styleout) {
	int err;
	std::vector<std::string> items;
	std::vector<std::string> attrs;
	std::string name("");
	std::string line;

	std::ifstream ifs(style);
    if(!ifs.is_open()) {
        std::cerr << "failed to open " << style << "\n";
        return 1;
    }
	std::ofstream ofs(styleout);

    while(std::getline(ifs, line)) {
		if(line.empty()) continue;
		if(line.at(0) == '#') continue; // comment (ignore line)

		std::string name_new("");
		std::vector<std::string> items_new;
		std::vector<std::string> attrs_new;
		
		// error checking
		size_t name_open = line.find('<');
		size_t name_close = line.find('>');
		size_t item_open = line.find('[');
		size_t item_close = line.rfind(']');
		size_t attr_open = line.find(':');
		if(name_open != std::string::npos && item_open != std::string::npos && item_open < name_open) {
			std::cerr << "[class] before <name> on same line\n";
			return 1;
		}
		if(name_open != std::string::npos && attr_open != std::string::npos && attr_open < name_open) {
			std::cerr << "attribute before <name> on same line\n";
			return 1;
		}
		if(item_open != std::string::npos && attr_open != std::string::npos && attr_open < item_open) {
			std::cerr << "attribute before [class] on same line\n";
			return 1;
		}

		// set name
		if(name_open != std::string::npos && name_close != std::string::npos)
			name_new = line.substr(name_open+1, name_close-name_open-1);
		else
			name_new = "";

		// parse line
		err = get_items(line, items_new, item_open, '[', ']');
		if(err) return 1;
		err = 0;
		if(item_close == std::string::npos && (name_open == std::string::npos || name_close == std::string::npos)) {
			err = get_items(line, attrs_new, 0, '\0', ';');
		} else if(item_close != std::string::npos && item_close != line.length()-1) {
			err = get_items(line, attrs_new, item_close+1, '\0', ';');
		}
		if(err) return 1;

		// print to out
		if((!items_new.empty() || name_new.length() != 0) && !attrs.empty()) {
			std::string attr_str("");
			for(const auto& s : attrs) {
				attr_str += "    " + s + "\n";
			}
			for(const auto& s : items) {
				ofs << "." << dashify(s) << " {\n" << attr_str << "}\n";
			}
			
			dashify(items);
			if(name != "") groups.insert({name, items});
			
			name = (name_new.length() != 0)? name_new : "";
			items.clear();
			attrs.clear();
		}
		items.insert(items.end(), items_new.begin(), items_new.end());
		attrs.insert(attrs.end(), attrs_new.begin(), attrs_new.end());
	}
	if(!attrs.empty()) {
		std::string attr_str("");
		for(const auto& s : attrs) {
			attr_str += "    " + s + "\n";
		}
		for(const auto& s : items) {
			ofs << "." << dashify(s) << " {\n" << attr_str << "}\n";
		}
		
		dashify(items);
		if(name != "") groups.insert({name, items});
	}
	ifs.close();
	ofs.close();
	return 0;
}

int write_script(std::string& scriptout) {	
	std::ofstream ofs(scriptout);

	ofs << "const items = new Map([";
	for(auto i = groups.begin(); i != groups.end(); i++) {
		//std::cout << (*i).first << ": " << (*i).second[0] << "\n";
		ofs << "[\"" << (*i).first << "-group\", [";
		for(auto j = (*i).second.begin(); j != (*i).second.end(); j++) {
			ofs << "\"" << (*j);
			auto temp = j;
			temp++;
			ofs << ((temp == (*i).second.end())? "\"" : "\", ");
		}
		auto temp = i;
		temp++;
		ofs << ((temp == groups.end())? "]]" : "]], ");
	}
	ofs << "]);\n";

	ofs << "const tags = new Set([";
	for(auto i = tags.begin(); i != tags.end(); i++) {
		ofs << "\"" << (*i);
		auto temp = i;
		temp++;
		ofs << ((temp == tags.end())? "\"" : "\", ");
	}
	ofs << "]);\n";

	return 0;
}

int main(int args, char* argv[]) {
    // get input
	// usage: ./genweb input.txt style.txt output.html style.css
	// missing input files will trigger I/O
	// missing output files use default names
	int err;
    std::string file, style, out, styleout, scriptout;
    if(args >= 2) {
        file = argv[1];
    } else {
        std::cout << "parse file > ";
        std::cin >> file;
    }
	if(args >= 3) {
		style = argv[2];
	} else {
		std::cout << "stylesheet > ";
		std::cin >> style;
	}
	out = (args >= 4)? argv[3] : "out.html";
	styleout = (args >= 5)? argv[4] : "style.css";
	if(styleout == "sidebar.css") {
		std::cerr << "invalid styleout (sidebar style will be overwritten)\n";
		exit(1);
	}
	scriptout = (args >= 6)? argv[5] : "data.js";
	if(scriptout == "script.js") {
		std::cerr << "invalid scriptout (main script will be overwritten)\n";
		exit(1);
	}
	
	// write styles
	err = write_style(style, styleout);
	if(err) exit(1);

	/*for(auto& thing : groups) {
		std::cout << thing.first << ": ";
		for(auto& thing2 : thing.second)
			std::cout << thing2 << " ";
		std::cout << "\n";
	}*/
    
    // write index
	err = write_main(file, out, styleout, scriptout);
	if(err) exit(1);
	err = write_main(file, out, styleout, scriptout);
	if(err) exit(1);

	// write JS (data only)
	err = write_script(scriptout);
	if(err) exit(1);

    return 0;
}
