#include <iostream>
#include <fstream>
#include <string>
#include <cctype>

static const std::string HEAD_FILE = "head.html";
static const std::string FOOT_FILE = "foot.html";
static const size_t MAX_TAGS = 5;

#define DEFT_MODE 0
#define HEAD_MODE 1
#define LIST_MODE 2
#define BLANK_MODE 3

int parse_mode(const std::string& line) {
    if(line.length() == 0) {
        return BLANK_MODE;
    }
    switch(line.at(0)) {
        case '+':
            return HEAD_MODE;
        case '-':
        case '*':
            return LIST_MODE;
        default:
            return DEFT_MODE;
    }
}

std::string munch_whitespace(const std::string line) {
    if(line.length()==0) {
        return line;
    }
    char c;
    size_t index = 0;
    do {
        c = line.at(index);
        index++;
    } while(std::isspace(c));
    return line.substr(index);
}

int parse_head(std::ofstream& ofs, std::string line) {
    line = munch_whitespace(line.substr(1));
    ofs << "<span class=\"list-head\">";
    ofs << line;
    ofs << "</span>\n";
    return 0;
}

int parse_list(std::ofstream& ofs, std::string line) {
    bool bold = (line.at(0)=="*")? 1:0;
    ofs << "    <li>";
    if(bold) {
        ofs << "<b>";
    }
    line = munch_whitespace(line.substr(1));
    for(size_t i = 0; i < line.length(); i++) {
        char c = line.at(i);
        switch(c) {
            case '(':
                size_t end = line.find(')', i);
                size_t link = line.find('<', i);
                if(end > link) {
                    std::err << "unbounded link caption: " << line << "\n";
                    return 1;
                }
                 
                break;
            case '[':
                if(bold) {
                    ofs << "</b>";
                    bold = false;
                }
                break;
            default:
                ofs << c;
        }
    }
    if(bold) {
        ofs << "</b>";
    }
    ofs << "\n";
    return 0;
}

int main(int args, char* argv[]) {
    // get input
    std::string file, out;
    if(args > 1) {
        file = argv[1];
    }
    if(args > 2) {
        out = argv[2];
    }
    if(args == 1 || file=="q") {
        std::cout << "< ";
        std::cin >> file;
    }
    if(args == 1 || out=="q") {
        std::cout << "> ";
        std::cin >> out;
    }
    if(out=="q") {
        out="out.html";
    }
    
    // open files
    std::ifstream ifs(file);
    if(!ifs.is_open()) {
        std::cerr << "failed to open " << file << "\n";
        exit(1);
    }
    std::ifstream head(HEAD_FILE);
    if(!head.is_open()) {
        std::cerr << "failed to open " << HEAD_FILE << "\n";
        exit(1);
    }
    std::ifstream foot(FOOT_FILE);
    if(!foot.is_open()) {
        std::cerr << "failed to open " << FOOT_FILE << "\n";
        exit(1);
    }
    std::ofstream ofs(out);
    std::string line;

    // write head
    while(std::getline(head, line)) {
        ofs << line << "\n";
    }
    head.close();

    // do stuff
    int prev_mode = 0;
    int mode = 0;
    int err = 0;
    while(std::getline(ifs, line)) {
        mode = parse_mode(line);
        if(prev_mode == LIST_MODE && mode != LIST_MODE) {
            ofs << "</ul>\n";
        } else if(prev_mode != LIST_MODE && mode == LIST_MODE) {
            ofs << "<ul>\n";
        }
        switch(mode) {
            case DEFT_MODE:
                ofs << line << "\n";
                break;
            case HEAD_MODE:
                err = parse_head(ofs, line);
                if(err) {
                    std::cerr << "error parsing list head: " << line << "\n";
                    exit(1);
                }
                break;
            case LIST_MODE:
                err = parse_list(ofs, line);
                if(err) {
                    std::cerr << "error parsing list: " << line << "\n";
                    exit(1);
                }
                break;
            case BLANK_MODE:
                ofs << "<br>\n";
                break;
            default:
                std::cerr << "invalid parse mode: " << line << "\n";
                exit(1);
        }
        prev_mode = mode;
    }
    if(mode==LIST_MODE) {
        ofs << "</ul>\n";
    }

    // write foot
    while(std::getline(foot, line)) {
        ofs << line << "\n";
    }
    foot.close();

    return 0;
}
