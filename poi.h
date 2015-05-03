/**
 * File: poi.h
 * Modification of ccfs.hpp by Ahmad Zaky and simplefs.hpp by Faiz Ilham
 */
#ifndef POI_H
#define POI_H

/** Definisi library **/
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <stdexcept>
#include <string>
#include <fstream>
using namespace std;

/** Definisi tipe **/
typedef unsigned short ptr_block;

/** Konstanta **/
/* Konstanta ukuran */
#define BLOCK_SIZE 512
#define N_BLOCK 65536
#define ENTRY_SIZE 32
#define DATA_POOL_OFFSET 257
/* Konstanta untuk pointer blok pada Allocation Table */
#define EMPTY_BLOCK 0x0000
#define END_BLOCK 0xFFFF


/**
 * Kelas poi: kelas yang mendefinisikan keseluruhan filesystem
 */
class poi{
public:
/* Method */	
	/* konstruktor & destruktor */
	poi();
	~poi();
	
	/* Buat file *.poi baru */
	void create(const char *filename);
	void initHeader(const char *_volumeName);
	void initDataPool();

	/* Baca file *.poi */
	void load(const char *filename);
	void readHeader();
	
	/* Tulis header *.poi */
	void writeVolumeInformation();
	void writeAllocationTable(ptr_block position);
	
	/* Pengalokasian block */
	void setNextBlock(ptr_block position, ptr_block next);
	ptr_block allocateBlock();
	void freeBlock(ptr_block position);
	
	/* Baca/tulis block */
	int readBlock(ptr_block position, char *buffer, int size, int offset = 0);
	int writeBlock(ptr_block position, const char *buffer, int size, int offset = 0);

/* Attributes */
	fstream poiFile;				// File .poi
	ptr_block nextBlock[N_BLOCK];	// Array pointer blok
	string volumeName;				// Nama volume
	int available;					// Jumlah slot yang masih kosong
	int firstEmpty;					// slot pertama yang masih kosong
	time_t mount_time;				// Waktu mounting
};


/**
 * Kelas entry
 */
class Entry {
public:
/* Method */
	/* Penanganan Entry */
	Entry();
	Entry(ptr_block _position, unsigned char _offset);
	Entry nextEntry();
	Entry getEntry(const char *path);
	Entry getNewEntry(const char *path);
	Entry getNextEmptyEntry();
	
	/* Penanganan Entry kosong */
	void makeEmpty();
	int isEmpty();
	
	/* Setter atribut Entry */
	string getName();
	unsigned char getAttr();
	short getTime();
	short getDate();
	ptr_block getIndex();
	int getSize();

	/* Setter atribut Entry */
	void setName(const char* name);
	void setAttr(const unsigned char attr);
	void setTime(const short time);
	void setDate(const short date);
	void setIndex(const ptr_block index);
	void setSize(const int size);
	
	/* Penganangan waktu */
	time_t getDateTime();
	void setCurrentDateTime();

	/* Tulis Entry ke file *.poi */
	void write();
	
/* Attributes */
	char data[ENTRY_SIZE];	// Data sesungguhnya dari Wntry
	ptr_block position;		// Posisi blok
	unsigned char offset;	// Offset dalam satu blok (0..15)
};

#endif