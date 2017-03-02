CC = gcc

all:
	$(CC) -g lab3a.c -o lab3a

tests:
	rm -rf directory.csv
	rm -rf indirect.csv
	./lab3a disk-image
	diff super.csv super_check.csv
	diff group.csv group_check.csv
	diff bitmap.csv bitmap_check.csv
	diff inode.csv inode_check.csv
	diff directory.csv directory_check.csv
	diff indirect.csv indirect_check.csv

tarball:
	tar -czvf lab3a404606017.tar.gz lab3a.c Makefile README