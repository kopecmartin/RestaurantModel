CC = g++

FLAGS = -std=c++98 -Wall -Wextra -pedantic
LIBS = -lm -lsimlib

all: main.cpp
	$(CC) $(FLAGS) $(LIBS) main.cpp -o main

run:
	./main

clean:
	rm -f main
