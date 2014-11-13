all:
	gcc webserver.c netSocket.c -o webserver -DUNIX
clean:
	rm -f webserver
