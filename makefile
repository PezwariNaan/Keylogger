all: server client

server: server.c
	gcc -g -Wall server.c -o server

client: client.c
	i686-w64-mingw32-gcc client.c -g -Wall -o client.exe -lwsock32 

clean: 
	rm server
	rm client.exe
