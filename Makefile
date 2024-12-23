# compiler
CC = gcc

# flags
CFLAGS = -Wall -g

all: hw2

hw2: main.o queue.o dispatcher.o utils.o common.o worker.o
	$(CC) $(CFLAGS) -o hw2 main.o queue.o dispatcher.o utils.o common.o worker.o

main.o: main.c queue.h dispatcher.h utils.h common.h worker.h
	$(CC) $(CFLAGS) -c main.c -o main.o

utils.o: utils.c utils.h common.h queue.h
	$(CC) $(CFLAGS) -c utils.c -o utils.o

worker.o: worker.c worker.h utils.h common.h queue.h
	$(CC) $(CFLAGS) -c worker.c -o worker.o

dispatcher.o: dispatcher.c dispatcher.h worker.h utils.h common.h queue.h
	$(CC) $(CFLAGS) -c dispatcher.c -o dispatcher.o

common.o: common.c common.h queue.h
	$(CC) $(CFLAGS) -c common.c -o common.o
	
queue.o: queue.c queue.h common.h
	$(CC) $(CFLAGS) -c queue.c -o queue.o

clean:
	rm -f hw2 main.o utils.o worker.o dispatcher.o common.o queue.o

