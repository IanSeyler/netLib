/*
	echoserver.c - Module file for Echo Server demo using netLib
	v0.4 - 2007.08.30
	Coded by Ian Seyler (iseyler@gmail.com)
	http://bubbai.ath.cx/projects/netlib/
	Copyright (c) 2007 Ian Seyler. All Rights Reserved.
	Distributed under the terms of the MIT License.
*/


#include "netSocket.h"

int main()
{
	int tint;
	netSocket *clientsocket, *listensocket;
	char tempbuffer[1];
	
	printf("Echo Server demo for netLib v0.4\n");

	netInit(); // Start the netLib library

	clientsocket = netInitSocket(); // Initiate the socket
	listensocket = netInitSocket();

	netListen(listensocket, 8192); // Set the server to listen on port 8192

	printf("\n\n");

	// Main program loop
	for (;;)
	{
		if (netAccept(listensocket, clientsocket) > 0) // A client has connected!
		{
			for (;;)
			{
				tint = netRecv(clientsocket, tempbuffer, 1); // Get a byte from the client
				if (tint > 0)
				{
					printf("\nGot %c", tempbuffer[0]); // Print the char we got
					netSend(clientsocket, tempbuffer, 1); // Send it back to the client
				}
				else if (tint == -1)
				{
					printf("\n");
					break;
				}
			}
		}
		else
		{
			printf("."); // Prints this while waiting for a client
		}
	}

	netDisconnect(listensocket);

	netStop(); // Stop the netLib library

	printf("\n");
	return 0;
}
