all: main.cpp poi.o poifs.o
	g++ main.cpp poi.o poifs.o -D_FILE_OFFSET_BITS=64 `pkg-config fuse --cflags --libs` -o mount-poi

poi.o : poi.hpp poi.cpp
	g++ -Wall -c poi.cpp -D_FILE_OFFSET_BITS=64

poifs.o : poifs.hpp poifs.cpp
	g++ -Wall -c poifs.cpp -D_FILE_OFFSET_BITS=64
	
clean:
	rm *~
	
clear:
	rm *.o
