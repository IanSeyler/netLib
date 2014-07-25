/*
	bufferserver.c - Module file for Test Buffer Server demo using netLib
	v0.4 - 2007.08.30
	Coded by Ian Seyler (iseyler@gmail.com)
	http://bubbai.ath.cx/projects/netlib/
	Copyright (c) 2007 Ian Seyler. All Rights Reserved.
	Distributed under the terms of the MIT License.
*/


#include "netSocket.h"
#include "netBuffer.h"

#define MAXCONCURRENTCLIENTS 3

int main()
{
	int tint, tint3, step = 60, freeclient = -1;
	netSocket *clientsocket[MAXCONCURRENTCLIENTS];
	netSocket *tempsocket; // YOU MUST INITIALIZE ALL SOCKETS BEFORE USING THEM!!
	netSocket *listensocket;
	netBuffer *tempbuffer; // YOU MUST INITIALIZE ALL BUFFERS BEFORE USING THEM!!
	char *netBuff;
	char netBuffData[128];
	unsigned char rawdata[4] = {0xBA, 0xBE, 0x0D, 0xAD};
	char message[]="Sample test string.";
	netBuff = netBuffData;

	printf("Buffer Server demo for netLib v0.4\n");

	netInit(); // Start the netLib library

	for(tint=0; tint<MAXCONCURRENTCLIENTS; tint++)
	{
		clientsocket[tint] = netInitSocket();
		printf(" - Client %d", tint+1);
	}
	tempsocket = netInitSocket(); // initiate the socket
	listensocket = netInitSocket(); // initiate the socket
	tempbuffer = netInitBuffer(128); // create a 128 byte buffer

	netListen(listensocket, 8192); // Set the server to listen on port 8192

	// Main program loop
	for (;;)
	{
		tempbuffer->data[0] = step++; // increment the message

		// listen for new connections and accept them if there is room
		if (netIsDataPending(listensocket) == 1)
		{
			for(tint=0; tint<MAXCONCURRENTCLIENTS; tint++) // loop through all the client records and find a free one
			{
				if (clientsocket[tint]->inuse == 0)
				{
					freeclient = tint;
					break; // get out of the for loop... we have the freeclient token
				}
			}

			if (freeclient != -1) // There is a valid free connection.. use it!
			{
				netAccept(listensocket, clientsocket[freeclient]);
				printf(" --> Client is %d", tint+1);
				freeclient = -1; // clear the token
			}
			else // No more connection.. kick them off
			{
				printf("\nNo more free connections.");
				netAccept(listensocket, tempsocket); // accept and disconnect the socket
				// Do something here???
				netDisconnect(tempsocket);
			}
		} // we are done listening here

		// Update current clients
		for(tint=0; tint<MAXCONCURRENTCLIENTS; tint++)
		{
			if (clientsocket[tint]->inuse == 1)
			{
				netBufferClear(tempbuffer);
				netBufferAppendInt32(tempbuffer, 16909060);
				netBufferAppendString(tempbuffer, message);
				netBufferAppendInt32(tempbuffer, 2147483647);
				netBufferAppendInt16(tempbuffer, 32768);
				netBufferAppendInt32(tempbuffer, 87654321);
				netBufferAppendData(tempbuffer, rawdata, 4);
				netSendBuffer(clientsocket[tint], tempbuffer); // NetSendBuffer will do the cleaning if there is an error.
				printf("\nBUFF: ");
				for (tint3=0; tint3<tempbuffer->datalength; tint3++)
					printf("%02X", tempbuffer->dataptr[tint3]);
			}
		}
		// done updating the current clients

	}

	netDisconnect(listensocket);

	netStop(); // Stop the netLib library

	printf("\n");
	return 0;
}
