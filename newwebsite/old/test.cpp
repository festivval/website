#include <iostream>
#include <string>


int main() {
    std::string c = "cat";
    size_t mew = c.find("*");
    std::cout << c.substr(0, mew) << "\n";
    return 0;
}
