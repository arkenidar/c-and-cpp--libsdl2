all:
	g++ multi_drag.cpp -o multi_drag $(shell pkg-config --cflags --libs SDL2)