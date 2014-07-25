/*
	chatserver.c - Module file for Chatserver using netLib
	v0.2 - 2007.08.30
	Coded by Ian Seyler (iseyler@gmail.com)
	http://bubbai.ath.cx/
	Copyright (c) 2007 Ian Seyler. All Rights Reserved.
	Distributed under the terms of the MIT License.
*/


#include "netSocket.h"
#include "netBuffer.h"
#include "chat.h"

int main()
{
	int tint, tint2, freeclient = -1, returnval, loopcounter = 0, tval;
	netSocket *clientsocket[MaxConcurrentClients];
	netBuffer *clientbuffer[MaxConcurrentClients];
	int clientsockettype[MaxConcurrentClients];
	char clientnick[MaxConcurrentClients][9];
	netSocket *tempsocket; // YOU MUST INITIALIZE ALL SOCKETS BEFORE USING THEM!!
	netSocket *listensocket;
	netBuffer *tempbuffer; // YOU MUST INITIALIZE ALL BUFFERS BEFORE USING THEM!!
	char *netBuff;
	char netBuffData[MessageSize];
	char tempmessage[2048];
	netBuff = netBuffData;

	printf("Chat Server v0.2 (A demo using netLib)\n");
	printf("Designed and coded by Ian Seyler\n");

	netInit(); // Start the netLib library

	for(tint=0; tint<MaxConcurrentClients; tint++)
	{
		clientsocket[tint] = netInitSocket();
		clientbuffer[tint] = netInitBuffer(MessageSize);
		clientsocket[tint]->inuse = 0;
		clientsockettype[tint] = -1;
	}
	tempsocket = netInitSocket(); // initiate the socket
	listensocket = netInitSocket(); // initiate the socket
	tempbuffer = netInitBuffer(MessageSize); // create a buffer

	netListen(listensocket, ChatPort); // Set the server to listen

	printf("\n");

	// Main program loop
	for (;;)
	{
		// Listen for new connections and accept them if there is room
		if (netIsDataPending(listensocket) == 1)
		{
			printf("\nGot a connection.");
			for(tint=0; tint<MaxConcurrentClients; tint++) // loop through all the client records and find a free one
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
				printf(" Client is %d", tint+1);
				netRecv(clientsocket[freeclient], netBuff, 1);
				if (netBuff[0] == '0')
				{
					printf(" --> Listen mode");
					clientsockettype[freeclient] = 0;
				}
				else if (netBuff[0] == '1')
				{
					printf(" --> Speak mode");
					memset(netBuff, '\0', sizeof(netBuff));
					for (tint=0; tint<3; tint++) // loop here three times in case of a slow client connection
					{
						returnval = netRecv(clientsocket[freeclient], clientnick[freeclient], 9); // get the name
						if (returnval == 9)
							tint = 1000000; // one meeeellion
					}
					if (returnval != 9) // the client is corrupt
					{
						printf(" --> Corrupt client");
						netDisconnect(clientsocket[freeclient]);
					}
					else // we have a good client
					{
						clientsockettype[freeclient] = 1;
						printf(" : %s.", clientnick[freeclient]);
						memset(tempmessage, '\0', sizeof(tempmessage));
						strcpy(tempmessage, "CHANSERV: ");
						strcat(tempmessage, clientnick[freeclient]);
						strcat(tempmessage, " has joined the chat.");
						netBufferClear (tempbuffer);
						netBufferAppendString (tempbuffer, tempmessage);
						
						printf(" Sending global message.");
						for(tint=0; tint<MaxConcurrentClients; tint++)
						{
							if (clientsocket[tint]->inuse == 1 && clientsockettype[tint] == 0)
								if (netSendBuffer(clientsocket[tint], tempbuffer) <= 0)
									clientsockettype[tint] = -1;
						}
					}
				}
				else // The other end is not a chat client.
				{
					printf(" --> Corrupt client");
					netDisconnect(clientsocket[freeclient]);
				}
				freeclient = -1; // clear the token
			}
			else // No more connection.. kick them off
			{
				printf(" No more free connections.");
				netAccept(listensocket, tempsocket); // accept and disconnect the socket
				netSend(tempsocket, "Server message: No free connections. Please try again later.\0", MessageSize); // send an error message
				netDisconnect(tempsocket);
			}
		}
		// we are done listening here

		// Update current clients
		for(tint=0; tint<MaxConcurrentClients; tint++)
		{
			if (clientbuffer[tint]->complete == 1)
			{
				printf(" Sending message to clients.");
				for(tint2=0; tint2<MaxConcurrentClients; tint2++)
				{
					if (clientsocket[tint2]->inuse == 1 && clientsockettype[tint2] == 0)
					{
						if (netSendBuffer(clientsocket[tint2], clientbuffer[tint]) <= 0)
							clientsockettype[tint2] = -1;
					}
				}
				clientbuffer[tint]->complete = 0;
			}
		}
		// done updating the current clients

		netBufferClear (tempbuffer);

		// Check the speaking clients
		for(tint=0; tint<MaxConcurrentClients; tint++)
		{
			if (clientsocket[tint]->inuse == 1 && clientsockettype[tint] == 1)
			{
				returnval = netRecvBuffer(clientsocket[tint], clientbuffer[tint]);
				if (returnval < 0) // the client has disconnected
				{
					memset(tempmessage, '\0', sizeof(tempmessage));
					strcpy(tempmessage, "CHANSERV: ");
					strcat(tempmessage, clientnick[tint]);
					strcat(tempmessage, " has left the chat.");
					netBufferClear (clientbuffer[tint]);
					netBufferAppendString (clientbuffer[tint], tempmessage);
					clientbuffer[tint]->complete = 1;
					printf("\nMessage received from client %d.", tint+1);
				}
				else if (returnval > 0) // the client is sending a message
				{
					if (clientbuffer[tint]->complete == 1) // we have a whole message
						printf("\nMessage received from client %d.", tint+1);
				}
			}
		}
		// done updating the current clients
	}

	netDisconnect(listensocket);

	netStop(); // Stop the netLib library

	printf("\n");
	return 0;
}
