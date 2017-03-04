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
	tar -zcvf lab3a404606017.tar.gz lab3a.c Makefile README directory_check.csv super_check.csv group_check.csv bitmap_check.csv inode_check.csv indirect_check.csv