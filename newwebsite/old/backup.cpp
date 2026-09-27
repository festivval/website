#include <iostream>
#include <fstream>
#include <string>
#include <vector>

static const size_t MAX_TAGS = 5;

std::string dashify(const std::string& s) {
	std::string r = "";
	for(const char& c : s) {
		if(c == '[' || c == ']') continue;
		r += (c == ' ')? '-' : c;
	}
	return r;
}

int write_head(std::ofstream& ofs, std::string& styleout, std::string& scriptout) {
	ofs << "<!DOCTYPE html>\n<html>\n<head>\n";
	ofs << "<link rel=\"stylesheet\" type=\"text/css\" href=\"" << styleout << "\">\n";
	ofs << "<script type=\"text/javascript\" src=\"https://code.jquery.com/jquery-4.0.0.slim.min.js\"></script>"\n;
	ofs << "<script type=\"text/javascript\" src=\"" << scriptout << "\"></script>"\n;
	ofs << "</head>\n";
	return 0;
}

int write_foot(std::ofstream& ofs) {
	ofs << "</body>\n</html>";
}

int get_items(std::string& line, std::vector<std::string> items, char open_char, char close_char, char end='\0') {
	size_t open = line.find(open_char);
	size_t close = line.find(close_char);
	while(open != std::string::npos) {
		if(close == std::string::npos) {
			std::cerr << "unclosed item\n";
			return 1;
		}
		
		items.push_back(line.substr(open, close-open+1));
		if(close == line.length()-1) break;
		open = line.find(open_char, close);
		close = line.find(close_char, close+1);
	}
	return 0;
}

int write_main(std::string& file, std::string& out) {
    // open files
    std::ifstream ifs(file);
    if(!ifs.is_open()) {
        std::cerr << "failed to open " << file << "\n";
        return 1;
    }
    std::ofstream ofs(out);
	
	// write head
	write_head(ofs, styleout, scriptout);

    std::string line;
    bool list = false;
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
        if(list) {
            // find indicators
            size_t org_open = line.find('[');
            size_t org_close = line.find(']');
            if(org_open == std::string::npos || org_close == std::string::npos) {
                std::cerr << "unclosed org tag\n";
                return 1;
            }
            size_t ink_open = line.find('(');
            size_t ink_close = line.find(')');
            if(((ink_open == std::string::npos) != (ink_close == std::string::npos)) || ink_close < ink_open) {
                std::cerr << "unclosed ink brackets\n";
            	return 1;
            }
            if(ink_close != std::string::npos && ink_close > org_open) {
                std::cerr << "ink after org tag\n";
                return 1;
            }
            size_t link_open = line.find('<');
            size_t link_close = line.find('>');
            if(((link_open == std::string::npos) != (link_close == std::string::npos)) || link_close < link_open) {
                std::cerr << "unclosed link brackets\n";
                return 1;
            }
            if(link_close != std::string::npos && link_close > org_open) {
                std::cerr << "link after org tag\n";
                return 1;
            }
            if((ink_close == std::string::npos) && (link_close != std::string::npos)) {
                std::cerr << "link without ink\n";
                return 1;
            }

            // name
            if(link_close == std::string::npos) {
                ofs << "    <li>" << line.substr(2, org_open-2) << "\n";
            } else {
                std::string link_url = line.substr(link_open+1, link_close-link_open-1);
                ofs << "    <li>" << line.substr(2, ink_open-2);
                ofs << "<a href=" << link_url <<">" << line.substr(ink_open+1, ink_close-ink_open-1) << "</a>";
                ofs << line.substr(link_close+1, org_open-link_close-1) << "\n";
            }
			std::string org = line.substr(org_open, org_close-org_open+1);
			std::string org_class = dashify(org);
            ofs << "        <span class=\"org " << org_class << "\">" << org << "</span>\n";
            // tags
            size_t star_open = line.find('*');
            size_t star_close = line.find('*', star_open+1);
            size_t i = 0;
            while(star_open != std::string::npos) {
                std::string tag = (star_close == std::string::npos)? line.substr(star_open+1, star_close-star_open) : line.substr(star_open+1, star_close-star_open-1);
                ofs << "        <span class=\"tag " << tag << "\">" << tag << "</span>\n";

                star_open = star_close;
                star_close = line.find('*', star_open+1);
                i++;
                if(i > MAX_TAGS) {
                    break;
                }
            }
            ofs << "    </li>\n";
        } else {
            ofs << "<h3>" << line << "</h3>\n";
        }
    }
    
	// write foot
	write_foot(ofs);
	ofs.close();

	return 0;
}

int write_style(std::string style, std::string styleout) {
	ifstream ifs(style);
    if(!ifs.is_open()) {
        std::cerr << "failed to open " << style << "\n";
        return 1;
    }
    ofstream ofs(styleout);

	bool item = false;
	int group = 0;
	std::vector<std::string> group_items;
	std::string group_value;
    while(std::getline(ifs, line)) {
		if(line.empty()) continue;

		if(line.at(0) == '#') continue;

		size_t group_open = line.find('*');
		if(group == 0 && group_open != std::string::npos) {
			if(item) ofs << "}\n";
			item = false;
			group = 1;
		}
		if(group == 1) {
			size_t org_open = line.find('[');
        	size_t org_close = line.find(']');

			while(org_open != std::string::npos) {
				if(org_close == std::string::npos) {
                	std::cerr << "unclosed org tag\n";
                	return 1;
				}
				group_items.push_back(dashify(line.substr(org_open+1, org_close-org_open-1)));
				if(org_close == line.length()-1) break;
				org_open = line.find('[', org_close);
				org_close = line.find(']', org_close+1);
			}
			
			size_t attr_find = (line.rfind(']') == std::string::npos)? 0 : line.rfind(']')+1;
			if(line.find(':', attr_find) != std::string::npos) group = 2;
		}
		if(group == 2 || group == 3) {
			size_t attr_open;
			if(group == 2) {
				attr_open = line.find(']');
				attr_open = (attr_open == std::string::npos)? 0 : attr_open+1;
				group = 3;
			} else {
				attr_open = 0;
			}
			size_t attr_close = line.find(':', attr_open);
			size_t value_close = line.find(';', attr_close);

			while(attr_close != std::string::npos) {
				std::string attr_value = (value_close == std::string::npos)? (line.substr(attr_open)+';') : line.substr(attr_open, value_close-attr_open+1);
				group_value += "    " + attr_value + "\n";

				if(value_close == std::string::npos) break;

				attr_open = value_close+1;
				attr_close = line.find(':', attr_open);
				value_close = line.find(';', attr_close);
			}

			if(line.find('*') != std::string::npos) {
				for(const std::string& s : group_items) {
					ofs << "." << s << " {\n" << group_value << "}\n";
				}
				group_items.clear();
				group_value = "";
				group = 0;
			}
		}
		if(group != 0) continue;

		size_t org_open = line.find('[');
        size_t org_close = line.find(']');
		if(org_open != std::string::npos) {
			if(org_close == std::string::npos) {
                std::cerr << "unclosed org tag\n";
                return 1;
			}
			
			if(item) { ofs << "}\n";}
			item = true;
			
			ofs << "." << dashify(line.substr(org_open+1, org_close-org_open-1)) << " {\n";
			if(org_close == line.length()-1) continue;
		}

		size_t attr_open = org_close+1;
		size_t attr_close = line.find(':', attr_open);
		size_t value_close = line.find(';', attr_close);
		while(attr_close != std::string::npos) {
			std::string attr_value = (value_close == std::string::npos)? (line.substr(attr_open)+';') : line.substr(attr_open, value_close-attr_open+1);
			ofs << "    " << attr_value << "\n";

			if(value_close == std::string::npos) break;

			attr_open = value_close+1;
			attr_close = line.find(':', attr_open);
			value_close = line.find(';', attr_close);
		}
	}
	if(item) {
		ofs << "}";
	}
	if(group != 0) {
		std::cerr << "unclosed group\n";
		return 1;
	}
	ifs.close();
	ofs.close();
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
	scriptout = (args >= 6)? argv[5] : "script.js";
    
    // write index
	err = write_main(file, out);
	if(err) exit(1);

	// write styles
	err = write_style(style, styleout);
	if(err) exit(1);

    return 0;
}
