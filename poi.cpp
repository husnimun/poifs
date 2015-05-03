/**
 * File: poi.cpp
 * Modification of ccfs.cpp by Ahmad Zaky and simplefs.cpp by Faiz Ilham
 */
#include "poi.h"

/** Variabel global filesystem *.poi **/
extern poi filesystem;


/**                 *
 * BAGIAN KELAS poi *
 *                 **/
/** Konstruktor */
poi::poi(){
	time(&mount_time);
}

/** Destruktor */
poi::~poi(){
	poiFile.close();
}

/** Buat file *.poi baru */
void poi::create(const char *volumeName){
	// Buka file dengan mode input-output, binary dan truncate (untuk membuat file baru) 
	poiFile.open(volumeName, fstream::in | fstream::out | fstream::binary | fstream::trunc);
	
	// Inisialisasi Volume Information & Allocation Table
	initHeader(volumeName);
	
	// Inisialisasi Data Pool 
	initDataPool();
	
	// Tutup file
	poiFile.close();
}

/** Baca file *.poi */
void poi::load(const char *volumeName){
	// Buka file dengan mode input-output dan binary 
	poiFile.open(volumeName, fstream::in | fstream::out | fstream::binary);
	
	// Cek keberadaan file 
	if (!poiFile.is_open()){
		poiFile.close();
		throw runtime_error("File not found");
	}
	
	// Baca Volume Information & Allocation Table
	readHeader();
}

/** Inisialisasi header file system */
void poi::initHeader(const char* _volumename){
	/* Inisialisasi Volume Information */
	// Buffer untuk menulis ke file
	char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE);
	
	// Magic string "poi!" 
	memcpy(buffer + 0x00, "poi!", 4);
	
	// Nama volume 
	volumeName = string(_volumename);
	memcpy(buffer + 0x04, _volumename, strlen(_volumename));
	
	// Kapasitas filesystem, dalam little endian 
	int capacity = N_BLOCK;
	memcpy(buffer + 0x24, (char*)&capacity, 4);
	
	// Jumlah blok yang belum terpakai, dalam little endian 
	available = N_BLOCK - 1;
	memcpy(buffer + 0x28, (char*)&available, 4);
	
	// Indeks blok pertama yang bebas, dalam little endian 
	firstEmpty = 1;
	memcpy(buffer + 0x2C, (char*)&firstEmpty, 4);

	// set byte ke 0x50 dengan null (otomatis)
	
	// Magic string "!iop" 
	memcpy(buffer + 0x1FC, "!iop", 4);
	
	// Tulis ke file
	poiFile.write(buffer, BLOCK_SIZE);


	/* Inisialisasi Allocation Table */
	// Allocation Table untuk root 
	short dataAloc = 0xFFFF;
	poiFile.write((char*)&dataAloc, sizeof(short));
	
	// Allocation Table untuk lainnya (default kosong) 
	dataAloc = 0;
	for (int i = 1; i < N_BLOCK; i++) {
		poiFile.write((char*)&dataAloc, sizeof(short));
	}
}

/** Inisialisasi Data Pool */
void poi::initDataPool() {
	// Buffer untuk menulis ke file
	char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE);
	
	// Inisialisasi blok kosong
	for (int i = 0; i < N_BLOCK; i++) {
		poiFile.write(buffer, BLOCK_SIZE);
	}
}

/** Baca header file *.poi & kopi ke memori */
void poi::readHeader(){
	/* Baca Volume Information */
	char buffer[BLOCK_SIZE];
	poiFile.seekg(0);
	
	// Baca keseluruhan Volume Information 
	poiFile.read(buffer, BLOCK_SIZE);
	
	// Cek magic string 
	if (string(buffer, 4) != "poi!") {
		poiFile.close();
		throw runtime_error("File is not a valid poi file");
	}

	// Baca kapasitas total
	int capacity = N_BLOCK;
	memcpy((char*)&capacity, buffer + 0x24, 4);
	
	// Baca kapasitas yang masih tersedia
	memcpy((char*)&available, buffer + 0x28, 4);
	
	// Baca pointer ke block yang kosong pertama
	memcpy((char*)&firstEmpty, buffer + 0x2C, 4);
	

	/* Baca Allocation Table */
	// Pindah ke awal Allocation Table, geser 512 byte
	char allocbuffer[3];
	poiFile.seekg(0x200);
	
	// Baca keseluruhan Allocation Table 
	for (int i = 0; i < N_BLOCK; i++) {
		poiFile.read(allocbuffer, 2);
		memcpy((char*)&nextBlock[i], allocbuffer, 2);
	}

}

/* Tulis Volume Information ke file *.poi */
void poi::writeVolumeInformation() {
	// Geser ke awal file
	poiFile.seekp(0x00);
	
	// Memori buffer untuk menulis ke file 
	char buffer[BLOCK_SIZE];
	memset(buffer, 0, BLOCK_SIZE);
	
	// Magic string "poi!" 
	memcpy(buffer + 0x00, "poi!", 4);
	
	// Nama volume 
	memcpy(buffer + 0x04, volumeName.c_str(), volumeName.length());
	
	// Kapasitas filesystem, dalam little endian 
	int capacity = N_BLOCK;
	memcpy(buffer + 0x24, (char*)&capacity, 4);
	
	// Jumlah blok yang belum terpakai, dalam little endian 
	memcpy(buffer + 0x28, (char*)&available, 4);
	
	// Indeks blok pertama yang bebas, dalam little endian 
	memcpy(buffer + 0x2C, (char*)&firstEmpty, 4);
	
	// String "!iop" 
	memcpy(buffer + 0x1FC, "!iop", 4);
	
	// Tulis dari memori ke file
	poiFile.write(buffer, BLOCK_SIZE);
}

/** Tulis Volume Information ke file *.poi */
void poi::writeAllocationTable(ptr_block position) {
	// Geser ke tempat pointer ke blok yang diinginkan
	poiFile.seekp(BLOCK_SIZE + sizeof(ptr_block) * position);
	
	// Tulis ke file *.poi
	poiFile.write((char*)&nextBlock[position], sizeof(ptr_block));
}

/** Atur pointer ke blok berikutnya pada Allocation Table */
void poi::setNextBlock(ptr_block position, ptr_block next) {
	nextBlock[position] = next;
	writeAllocationTable(position);
}

/** Dapatkan pointer ke blok yang kosong berikutnya */
ptr_block poi::allocateBlock() {
	// Cari blok yang kosong
	ptr_block result = firstEmpty;
	setNextBlock(result, END_BLOCK);
	while (nextBlock[firstEmpty] != 0x0000) {
		firstEmpty++;
	}
	available--;

	// Tulis VolumeInformation ke file *.poi
	writeVolumeInformation();
	return result;
}

/** Bebaskan blok */
void poi::freeBlock(ptr_block position) {
	// Penanganan blok kosong
	if (position == EMPTY_BLOCK) {
		return;
	}

	// Bebaskan blok
	while (position != END_BLOCK) {
		ptr_block temp = nextBlock[position];
		setNextBlock(position, EMPTY_BLOCK);
		position = temp;
		available--;
	}

	// Tulis VolumeInformation ke file *.poi
	writeVolumeInformation();
}

/** Baca isi dari block sebesar size */
int poi::readBlock(ptr_block position, char *buffer, int size, int offset) {
	// Penanganan blok terakhir 
	if (position == END_BLOCK) {
		return 0;
	}

	// Pengananan jika offset melebihi ukuran blok
	if (offset >= BLOCK_SIZE) {
		return readBlock(nextBlock[position], buffer, size, offset - BLOCK_SIZE);
	}
	
	// Baca data sesungguhnya dari Data Pool
	poiFile.seekg(BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset);
	int size_now = size;
	if (offset + size_now > BLOCK_SIZE) {
		size_now = BLOCK_SIZE - offset;
	}
	poiFile.read(buffer, size_now);
	
	// Pengananan jika ukuran yang harus dibaca melebihi ukuran blok
	if (offset + size > BLOCK_SIZE) {
		return size_now + readBlock(nextBlock[position], buffer + BLOCK_SIZE, offset + size - BLOCK_SIZE);
	}

	return size_now;
}

/** Tulis blok ke file *.poi */
int poi::writeBlock(ptr_block position, const char *buffer, int size, int offset) {
	// Penanganan blok terakhir 
	if (position == END_BLOCK) {
		return 0;
	}
	// Pengananan jika offset melebihi ukuran blok
	if (offset >= BLOCK_SIZE) {
		// kalau nextBlock tidak ada, alokasikan 
		if (nextBlock[position] == END_BLOCK) {
			setNextBlock(position, allocateBlock());
		}
		return writeBlock(nextBlock[position], buffer, size, offset - BLOCK_SIZE);
	}
	
	// Tulis data sesungguhnya ke Data Pool
	poiFile.seekp(BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset);
	int size_now = size;
	if (offset + size_now > BLOCK_SIZE) {
		size_now = BLOCK_SIZE - offset;
	}
	poiFile.write(buffer, size_now);
	
	// Pengananan jika ukuran yang harus ditulis melebihi ukuran blok
	if (offset + size > BLOCK_SIZE) {
		// kalau nextBlock tidak ada, alokasikan 
		if (nextBlock[position] == END_BLOCK) {
			setNextBlock(position, allocateBlock());
		}
		return size_now + writeBlock(nextBlock[position], buffer + BLOCK_SIZE, offset + size - BLOCK_SIZE);
	}
	
	return size_now;
}

/**                   *
 * BAGIAN KELAS ENTRY *
 *                   **/
/** Konstruktor */
Entry::Entry() {
	// Buat Entry kosong
	position = 0;
	offset = 0;
	memset(data, 0, ENTRY_SIZE);
}

/** Konstruktor parameter */
Entry::Entry(ptr_block _position, unsigned char _offset) {
	// Set posisi dan offset
	position = _position;
	offset = _offset;
	
	// Baca Data Pool 
	filesystem.poiFile.seekg(BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset * ENTRY_SIZE);
	filesystem.poiFile.read(data, ENTRY_SIZE);
}

/** Dapatkan Entry berikutnya */
Entry Entry::nextEntry() {
	if (offset < 15) {
		// Penanganan jika offset kurang dari 15 byte
		return Entry(position, offset + 1);
	} else {
		// Penanganan jika offset lebih dari 15 byte
		return Entry(filesystem.nextBlock[position], 0);
	}
}

/** Dapatkan Entry dari path */
Entry Entry::getEntry(const char *path) {
	// Mendapatkan directori teratas
	unsigned int endstr = 1;
	while (path[endstr] != '/' && endstr < strlen(path)) {
		endstr++;
	}
	string topDirectory = string(path + 1, endstr - 1);
	
	// Mencari Entry dengan nama topDirectory 
	while (getName() != topDirectory && position != END_BLOCK) {
		*this = nextEntry();
	}
	
	if (isEmpty()) {
		// Penanganan Entry kosong	
		return Entry();
	}
	else {
	// Penanganan Entry berisi
		if (endstr == strlen(path)) {
			return *this;
		} else {
			// Cek apakah file/direktori 
			if (getAttr() & 0x8) {
				ptr_block index;
				memcpy((char*)&index, data + 0x1A, 2);
				Entry next(index, 0);
				return next.getEntry(path + endstr);
			} else {
				return Entry();
			}
		}
	}
}

/** Dapatkan Entry baru dari path */
Entry Entry::getNewEntry(const char *path) {
	// Mendapatkan direktori teratas 
	unsigned int endstr = 1;
	while (path[endstr] != '/' && endstr < strlen(path)) {
		endstr++;
	}
	string topDirectory = string(path + 1, endstr - 1);
	
	// Mencari Entry dengan nama topDirectory 
	Entry entry(position, offset);
	while (getName() != topDirectory && position != END_BLOCK) {
		*this = nextEntry();
	}
	
	if (isEmpty()) {
		// Penanganan Entry kosong, buat baru
		while (!entry.isEmpty()) {
			if (entry.nextEntry().position == END_BLOCK) {
				entry = Entry(filesystem.allocateBlock(), 0);
			} else {
				entry = entry.nextEntry();
			}
		}
		// Beri atribut pada Entry 
		entry.setName(topDirectory.c_str());
		entry.setAttr(0xF);
		entry.setIndex(filesystem.allocateBlock());
		entry.setSize(BLOCK_SIZE);
		entry.setTime(0);
		entry.setDate(0);
		entry.write();
		
		*this = entry;
	}

	if (endstr == strlen(path)) {
		return *this;
	} else {
		// Cek apakah file/direktori 
		if (getAttr() & 0x8) {
			ptr_block index;
			memcpy((char*)&index, data + 0x1A, 2);
			Entry next(index, 0);
			return next.getNewEntry(path + endstr);
		}
		else {
			return Entry();
		}
	}
}

/** Cari Entry yang kosong selanjutnya. */
Entry Entry::getNextEmptyEntry() {
	// Lewati Entry yang berisi
	Entry entry(*this);
	while (!entry.isEmpty()) {
		entry = entry.nextEntry();
	}

	// Penanganan blok penuh
	if (entry.position == END_BLOCK) {
		// Buat blok baru 
		ptr_block newPosition = filesystem.allocateBlock();
		ptr_block lastPos = position;
		while (filesystem.nextBlock[lastPos] != END_BLOCK) {
			lastPos = filesystem.nextBlock[lastPos];
		}
		filesystem.setNextBlock(lastPos, newPosition);
		entry.position = newPosition;
		entry.offset = 0;
	}
	
	return entry;
}

/** Kosongkan entry */
void Entry::makeEmpty() {
	// Menghapus byte pertama data 
	*(data) = 0;
	write();
}

/** Memeriksa apakah Entry kosong */
int Entry::isEmpty() {
	return *(data) == 0;
}

/** Getter-Setter atribut-atribut Entry */
string Entry::getName() {
	return string(data);
}

unsigned char Entry::getAttr() {
	return *(data + 0x15);
}

short Entry::getTime() {
	short result;
	memcpy((char*)&result, data + 0x16, 2);
	return result;
}

short Entry::getDate() {
	short result;
	memcpy((char*)&result, data + 0x18, 2);
	return result;
}

ptr_block Entry::getIndex() {
	ptr_block result;
	memcpy((char*)&result, data + 0x1A, 2);
	return result;
}

int Entry::getSize() {
	int result;
	memcpy((char*)&result, data + 0x1C, 4);
	return result;
}

void Entry::setName(const char* name) {
	strcpy(data, name);
}

void Entry::setAttr(const unsigned char attr) {
	data[0x15] = attr;
}

void Entry::setTime(const short time) {
	memcpy(data + 0x16, (char*)&time, 2);
}

void Entry::setDate(const short date) {
	memcpy(data + 0x18, (char*)&date, 2);
}

void Entry::setIndex(const ptr_block index) {
	memcpy(data + 0x1A, (char*)&index, 2);
}

void Entry::setSize(const int size) {
	memcpy(data + 0x1C, (char*)&size, 4);
}

/** Bagian Date Time */
time_t Entry::getDateTime() {
	unsigned int datetime;
	memcpy((char*)&datetime, data + 0x16, 4);
	
	time_t rawtime;
	time(&rawtime);
	struct tm *result = localtime(&rawtime);
	
	result->tm_sec = datetime & 0x1F;
	result->tm_min = (datetime >> 5u) & 0x3F;
	result->tm_hour = (datetime >> 11u) & 0x1F;
	result->tm_mday = (datetime >> 16u) & 0x1F;
	result->tm_mon = (datetime >> 21u) & 0xF;
	result->tm_year = ((datetime >> 25u) & 0x7F) + 10;
	
	return mktime(result);
}

void Entry::setCurrentDateTime() {
	time_t now_t;
	time(&now_t);
	struct tm *now = localtime(&now_t);
	
	int sec = now->tm_sec;
	int min = now->tm_min;
	int hour = now->tm_hour;
	int day = now->tm_mday;
	int mon = now->tm_mon;
	int year = now->tm_year;
	
	int _time = (sec >> 1) | (min << 5) | (hour << 11);
	int _date = (day) | (mon << 5) | ((year - 10) << 9);
	
	memcpy(data + 0x16, (char*)&_time, 2);
	memcpy(data + 0x18, (char*)&_date, 2);
}

/** Menuliskan entry ke filesystem */
void Entry::write() {
	if (position != END_BLOCK) {
		filesystem.poiFile.seekp(BLOCK_SIZE * DATA_POOL_OFFSET + position * BLOCK_SIZE + offset * ENTRY_SIZE);
		filesystem.poiFile.write(data, ENTRY_SIZE);
	}
}
