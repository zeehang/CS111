CC = gcc

all:
	$(CC) -g -lmraa -lm lab4_part1.c -o lab4_1
	$(CC) -std=gnu99 -lmraa -lpthread -lm lab4_part2.c display.c -o lab4_2
	$(CC) -std=gnu99 -lmraa -lpthread -lm -lssl -lcrypto -I/usr/bin/ lab4_part3.c display.c -o lab4_3

tarball:
	