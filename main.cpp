#include <iostream>
#include "poifs.hpp"
#include "poi.hpp"

using namespace std;

struct fuse_operations poifs_op;

poi filesystem;

void init_fuse() {
	poifs_op.getattr	= poifs_getattr;
	poifs_op.readdir	= poifs_readdir;
	poifs_op.mkdir	= poifs_mkdir;
	poifs_op.open 	= poifs_open;
	poifs_op.rmdir 	= poifs_rmdir;
	poifs_op.rename	= poifs_rename;
	poifs_op.unlink	= poifs_unlink;
	poifs_op.mknod	= poifs_mknod;
	poifs_op.truncate= poifs_truncate;
	poifs_op.write	= poifs_write;
	poifs_op.read	= poifs_read;
	poifs_op.link	= poifs_link;
}

int main(int argc, char** argv){
	if (argc < 3) {
		printf("Usage: ./poifs <mount folder> <filesystem.poifs> [--new]\n");
		return 0;
	}
	
	// jika terdapat argumen --new, buat baru
	if (argc > 3 && string(argv[3]) == "--new") {
		filesystem.create(argv[2]);
	}
	
	filesystem.load(argv[2]);
	
	// inisialisasi fuse
	int fuse_argc = 2; 
	char* fuse_argv[2] = {argv[0], argv[1]};
	
	init_fuse();
	
	// serahkan ke fuse
	return fuse_main(fuse_argc, fuse_argv, &poifs_op, NULL);
}
