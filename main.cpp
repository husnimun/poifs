/**
 * File: main.cpp
 * Modification of main.cpp by Ahmad Zaky and Faiz Ilham
 */
/** Definisi library **/
#include <iostream>
#include "poi_fs.h"
#include "poi.h"
using namespace std;


/** Inisialisasi fungsi FUSE **/
struct fuse_operations poifs_op;

void init_fuse() {
	poifs_op.getattr	= poifs_getattr;
	poifs_op.readdir	= poifs_readdir;
	poifs_op.mkdir		= poifs_mkdir;
	poifs_op.open 		= poifs_open;
	poifs_op.rmdir 		= poifs_rmdir;
	poifs_op.rename		= poifs_rename;
	poifs_op.unlink		= poifs_unlink;
	poifs_op.mknod		= poifs_mknod;
	poifs_op.truncate	= poifs_truncate;
	poifs_op.write		= poifs_write;
	poifs_op.read		= poifs_read;
	poifs_op.link		= poifs_link;
	poifs_op.chmod		= poifs_chmod;
}


/** Variabel global filesystem *.poi **/
poi filesystem;


/* Fungsi Main */
int main(int argc, char** argv){
	// Invalid usage 
	if (argc < 3) {
		printf("Usage: ./mount-poi <mount folder> <filesystem.poifs> [--new]\n");
		return 0;
	}
	
	// Buat file system baru dengan parameter --new
	if (argc > 3 && string(argv[3]) == "--new") {
		filesystem.create(argv[2]);
	}
	
	filesystem.load(argv[2]);
	
	// Inisialisasi fuse
	int fuse_argc = 2; 
	char* fuse_argv[2] = {argv[0], argv[1]};
	
	// Jalankan fuse
	init_fuse();
	return fuse_main(fuse_argc, fuse_argv, &poifs_op, NULL);
}
