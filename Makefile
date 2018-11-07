all: client server

client: client.c comm.h comm.o util.o
	gcc -w client.c -g -o $@ comm.o util.o


server: server.c comm.h comm.o util.o
	gcc -w server.c -g -o $@ comm.o util.o

util.o: util.c util.h
	gcc -w -c util.c

comm.o: comm.c comm.h
	gcc -w -c comm.c

clean:
	rm -f *.o client server
