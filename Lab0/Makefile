CC = gcc

TARGET = lab0

all:
	$(CC) -g $(TARGET).c -o $(TARGET)

clean:
	$(RM) lab0.o lab0-404606017.tar.gz lab0

check: all nonexist nonwrite catchtest normalcases segtest

nonexist:
	./lab0 --input=hellono > output.txt; test $$? -eq 1

nonwrite:
	touch nonwrite.txt;
	touch testinput.txt;
	chmod a-w nonwrite.txt;
	./lab0 --output=nonwrite.txt < testinput.txt; test $$? -eq 2;
	rm -f nonwrite.txt; rm -f testinput.txt;

catchtest:
	./lab0 --catch --segfault; test $$? -eq 3

normalcases:
	touch input.txt;
	./lab0 < input.txt > output.txt; test $$? -eq 0;
	./lab0 --input=input.txt > output.txt; test $$? -eq 0;
	./lab0 --output=output.txt < input.txt; test $$? -eq 0;
	./lab0 --input=input.txt --output=output.txt; test $$? -eq 0;
	rm -f output.txt; rm -f input.txt;

segtest:
	./lab0 --segfault; test $$? -eq 139;

dist:
	tar -czvf lab0-404606017.tar.gz lab0.c Makefile README gdb_backtrace.png gdb_breakpoint.png
