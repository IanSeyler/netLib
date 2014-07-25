/*
	testclient.c - Module file for Test Client demo using netLib
	v0.4 - 2007.08.30
	Coded by Ian Seyler (iseyler@gmail.com)
	http://bubbai.ath.cx/projects/netlib/
	Copyright (c) 2007 Ian Seyler. All Rights Reserved.
	Distributed under the terms of the MIT License.
*/


#include "netSocket.h"
#include "netBuffer.h"

int main(int argc, char *argv[])
{
	int tint;
	netSocket *tempsocket; // YOU MUST INITIALIZE ALL SOCKETS BEFORE USING THEM!! (netInitSocket)
	netBuffer *tempbuffer; // YOU MUST INITIALIZE ALL BUFFERS BEFORE USING THEM!! (netInitBuffer)

	printf("Test Client demo for netLib v0.4\n");

	if (argc < 2) // make sure we got a destination as an arguement
	{
		printf("Usage : testclient hostname (ex: testclient bubbai.ath.cx)\n");
		exit(1);
	}

	netInit(); // Start the netLib library

	tempsocket = netInitSocket(); // initiate the socket
	tempbuffer = netInitBuffer(128); // initiate a 128 byte buffer

	if (netConnect(tempsocket, argv[1], 8192) == netOK) // if the connection looks good, keep going
	{
		for (tint=0; tint<5; tint++) //loop of doing a netRecvBuffer 5 times
		{
			if(netRecvBuffer(tempsocket, tempbuffer) > 0) // make sure we got some data
				printf("\nGot '%s'", tempbuffer->data);
			else
				break; // if the netRecvBuffer failed then break out to the netDisconnect
		}
		netDisconnect(tempsocket); // Issue here. We disconnect twice if the server dies early as netRecvBuffer does cleanup on broken connections.
	}

	netStop(); // Stop the netLib library

	printf("\n");
	return 0;
}
