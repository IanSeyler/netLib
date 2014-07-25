/*
	bufferclient.c - Module file for Test Buffer Client demo using netLib
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
	int tint, tint2, tint3;
	unsigned int tval;
	unsigned short tval2;
	int buffervalint;
	short buffervalshort;
	char buffervalstring[128];
	netSocket *tempsocket; // YOU MUST INITIALIZE ALL SOCKETS BEFORE USING THEM!! (netInitSocket)
	netBuffer *tempbuffer; // YOU MUST INITIALIZE ALL BUFFERS BEFORE USING THEM!! (netInitBuffer)

	printf("Buffer Client demo for netLib v0.4\n");

	if (argc < 2) // make sure we got a destination as an arguement
	{
		printf("Usage : bufferclient hostname (ex: testclient bubbai.ath.cx)\n");
		exit(1);
	}

	netInit(); // Start the netLib library

	tempsocket = netInitSocket(); // initiate the socket
	tempbuffer = netInitBuffer(128); // initiate a 128 byte buffer

	if (netConnect(tempsocket, argv[1], 8192) == netOK) // if the connection looks good, keep going
	{
		printf("\n");
		for (tint=0; tint<3; tint++) //loop of doing a netRecvBuffer 3 times
		{
			netBufferClear(tempbuffer);
			tint2 = netRecvBuffer(tempsocket, tempbuffer);

			if(tint2 > 0) // make sure we got some data
			{
				printf("\nBUFF: ");
				for (tint3=0; tint3<tempbuffer->datalength; tint3++)
					printf("%02X", tempbuffer->dataptr[tint3]);
				printf("\n");
				tval = netBufferRemoveInt32(tempbuffer);
				printf("int32: %d\n", tval);
				netBufferRemoveString(buffervalstring, tempbuffer);
				printf("string: %s\n", buffervalstring);
				tval = netBufferRemoveInt32(tempbuffer);
				printf("int32: %d\n", tval);
				tval2 = netBufferRemoveInt16(tempbuffer);
				printf("int16: %d\n", tval2);
				printf("int32: %d\n", netBufferRemoveInt32(tempbuffer));
			}
			else
			{
				printf("\nNo data?");
		//		break; // if the netRecvBuffer failed then break out to the netDisconnect
			}
		}
		netDisconnect(tempsocket); // Issue here. We disconnect twice if the server dies early as netRecvBuffer does cleanup on broken connections.
	}

	netStop(); // Stop the netLib library

	printf("\n");
	return 0;
}
