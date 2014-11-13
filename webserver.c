/*
	webserver.c - Super simple webserver demo using netLib
	v0.1 - 2014.11.13
	Coded by Ian Seyler (iseyler@gmail.com)
	https://github.com/IanSeyler/netLib/
	Copyright (c) 2007 Ian Seyler. All Rights Reserved.
	Distributed under the terms of the MIT License.
*/


#include "netSocket.h"

const int port = 8192;
const int buffsize = 2000;

/* Default HTTP page with HTTP headers */
char webpage[] = 
"HTTP/1.0 200 OK\n"
"Server: BareMetal (http://www.returninfinity.com)\n"
"Content-type: text/html\n"
"\n"
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"<title>Hello world</title>\n"
"</head>\n"
"<body>\n"
"Hello World!\n"
"</body>\n"
"</html>\n";

int main()
{
	int tint;
	netSocket *clientsocket, *listensocket;
	char tempbuffer[buffsize];
	
	printf("Web Server demo for netLib v0.4\nListening on port %d\n", port);

	// Start the netLib library
	netInit();

	// Initiate the sockets
	clientsocket = netInitSocket();
	listensocket = netInitSocket();

	// Set the server to listen
	netListen(listensocket, port);

	printf("\n\n");

	// Main program loop
	for (;;)
	{
		// Check for an incoming connection
		if (netAccept(listensocket, clientsocket) > 0)
		{
			// A client has connected!
			printf("\nHello.");

			// Clear the buffer
			memset(tempbuffer, 0, buffsize);

			// Check for data to receive and print if there was something
			tint = netRecv(clientsocket, tempbuffer, buffsize);
			if (tint > 0)
			{
				printf("\nReceived %d bytes.", tint);
			//	printf("\n--------------------\n%s", tempbuffer);
			}

			// Send data to the client
			tint = netSend(clientsocket, webpage, strlen(webpage));
			printf("\nSent %d bytes.", tint);

			// Disconnect the client
			netDisconnect(clientsocket);
			printf("\nGoodbye.\n\n");
		}
		// Why isn't this done on its own...
		fflush(stdout);
	}

	netDisconnect(listensocket);

	// Stop the netLib library
	netStop();

	printf("\n");
	return 0;
}
