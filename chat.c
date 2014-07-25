/*
	chat.c - Module file for Chat Client using netLib
	v0.2 - 2007.08.30
	Coded by Ian Seyler (iseyler@gmail.com)
	http://bubbai.ath.cx/
	Copyright (c) 2007 Ian Seyler. All Rights Reserved.
	Distributed under the terms of the MIT License.
*/


#include "netSocket.h"
#include "netBuffer.h"
#include "chat.h"

void printhelp();

int main(int argc, char *argv[])
{
	int tint, chatmode, returnval;
	char tempstring[100];
	char tempmessage[2048];
	char nickname[9]; // 8 chars plus a null
	char message[MessageSize];
	netSocket *serverSocket;
	netBuffer *chatBuffer;
	printf("Chat v0.2 (A demo using netLib)\n");
	printf("Designed and coded by Ian Seyler\n");
	
	if (argc <= 2)
	{
		printhelp();
		exit(1);
	}
	else if (argc > 3)
	{
		printf("\nToo many args");
		exit(1);
	}

	if (argv[2][1] == 'l' || argv[2][1] == 'L') // Listen
	{
		chatmode = 0;
	}
	else if (argv[2][1] == 's' || argv[2][1] == 'S') // Speak
	{
		chatmode = 1;
	}
	else
	{
		printf("\nInvalid Arguement\n");
		exit(1);
	}

	netInit();
	serverSocket = netInitSocket();
	chatBuffer = netInitBuffer(MessageSize);

	if (chatmode == 0) // Listen mode
	{
		printf("\n\nListen mode");
		printf("\n\nConnecting to chat server...");
		if (netConnect(serverSocket, argv[1], ChatPort) == netOK)
		{
			printf(" OK!\n\n");
			netSend(serverSocket, "0", 1); // tell the server we want listen mode
			while(1)
			{
				netBufferClear (chatBuffer);
				returnval = netRecvBuffer(serverSocket, chatBuffer);
				if (returnval > 0 && chatBuffer->complete == 1)
				{
					printf("%s\n", chatBuffer->dataptr);
//					for (tint=0; tint<chatBuffer->datalength+1; tint++)
//						printf("%d ", chatBuffer->data[tint]);
				}
				else if (returnval < 0)
				{
					printf("\nDisconnected from chat server.\n");
					netStop();
					exit(1); // error
				}
			}
		}
		else
		{
			printf("\nCould not connect to server.\n");
			netStop();
			exit(1);
		}
	}
	else if (chatmode == 1) // Speak mode
	{
		printf("\n\nSpeak mode");
		printf("\n\nEnter your nickname (Max 8 chars): ");
		fgets(tempstring, 80, stdin); // only read in 80 chars at a time
		fflush(stdin); // flush the rest of the keystrokes that are waiting in stdin
		tempstring[strlen(tempstring)-1] = '\0'; // get rid of the new line character
		strncpy(nickname, tempstring, 8);
		nickname[8] = '\0';
		printf("Your nickname is '%s'", nickname);

		printf("\n\nConnecting to chat server...");
		if (netConnect(serverSocket, argv[1], ChatPort) == netOK)
		{
			returnval = netSend(serverSocket, "1", 1); // tell the server we want speak mode
			returnval = netSend(serverSocket, nickname, 9);
			printf(" OK!\n\n");
		}
		else
		{
			printf("\nCould not connect to server.\n");
			netStop();
			exit(1);
		}
		
		while(1)
		{
			printf("Say: ");
			fgets(tempstring, 80, stdin); // only read in 80 chars at a time
			fflush(stdin); // flush the rest of the keystrokes that are waiting in stdin
			tempstring[strlen(tempstring)-1] = '\0'; // get rid of the new line character
			if (tempstring[0] == '/' && (tempstring[1] == 'q' || tempstring[1] == 'Q')) // exit the loop
				break;

			netBufferClear (chatBuffer);
			memset(tempmessage, '\0', sizeof(tempmessage));

			strncpy(tempmessage, nickname, strlen(nickname));
			memset(tempmessage+strlen(nickname), ' ', 8-strlen(nickname)); // pad the nick with spaces
			strncat(tempmessage, ": ", 2);
			strncat(tempmessage, tempstring, (MessageSize - 10)); // 128 - 10 for nickname
			
			netBufferAppendString (chatBuffer, tempmessage);
			
			if(netSendBuffer(serverSocket, chatBuffer) <= 0)
			{
				printf("\nDisconnected from chat server.\n");
				netStop();
				exit(1);
			}
		}
	}
	else // No mode was chosen
	{
		printf("\nProgram error. Mode not set.\n");
		netStop();
		exit(1);
	}

	printf("\nDisconnected from chat server.\n");

	netDisconnect(serverSocket);

	netStop();

	return 0;
}

void printhelp()
{
	printf("\nUsage: chat hostname [-listen or -speak]\n");
}
