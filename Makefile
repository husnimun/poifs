all: main.cpp poi.o poi_fs.o
	g++ main.cpp poi.o poi_fs.o -D_FILE_OFFSET_BITS=64 `pkg-config fuse --cflags --libs` -o mount-poi

poi.o : poi.h poi.cpp
	g++ -Wall -c poi.cpp -D_FILE_OFFSET_BITS=64

poi_fs.o : poi_fs.h poi_fs.cpp
	g++ -Wall -c poi_fs.cpp -D_FILE_OFFSET_BITS=64

clean:
	rm *~
	
clear:
	rm *.o
