target: main.cpp
	g++ main.cpp -Wall -Werror -pedantic -std=gnu++14 && ./a.out "main.asm" "exe"

clear:
	-rm a.out 2>/dev/null
