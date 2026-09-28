all: genweb run

genweb: main.cpp
	g++ main.cpp -o genweb -std=c++20 -Wall -Werror

genweb2: second.cpp
	g++ second.cpp -o genweb2 -std=c++20 -Wall -Werror

run:
	./genweb in.txt style.txt index.html
