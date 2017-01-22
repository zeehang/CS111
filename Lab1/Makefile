CC = gcc

TARGET = simpsh
ORIGINAL = lab1
all:
	$(CC) -g $(ORIGINAL).c -o $(TARGET)

clean:
	$(RM) lab1.o lab1-MichaelZhang.tar.gz simpsh

dist:
	tar -czvf lab1-MichaelZhang.tar.gz --transform 's,^,lab1-MichaelZhang/,' lab1.c Makefile README testscript.sh

check: all
	chmod 744 testscript.sh
	./testscript.sh
