poifs-filesystem
===============

A custom filesystem built with FUSE (Filesystem in Userspace). For more information about FUSE, visit [http://fuse.sourceforge.net/](http://fuse.sourceforge.net/).

Usage
-----

	./poifs <mount folder> <filesystem.poifs> [--new]

If flag `--new` is used then a new empty filesystem will be created

List of implemented methods
---------------------------
- getattr
- readdir
- mkdir
- rmdir
- rename
- open
- unlink
- mknode
- truncate
- open
- write
- link
