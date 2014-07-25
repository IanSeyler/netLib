/*
	netBuffer.h - Header file for netLib Network Communication Library (Buffers)
	v0.4 - 2007.08.30
	Coded by Ian Seyler (iseyler@gmail.com)
	http://bubbai.ath.cx/projects/netlib/
	Copyright (c) 2007 Ian Seyler. All Rights Reserved.
	Distributed under the terms of the MIT License.
*/


#include "netSocket.h"

/* Structure of a netBuffer */
struct netBufferStruct {
	int size; // allocated size in bytes of the buffer
	int readAt; // location in buffer where we are currently reading
	int writeAt; // location in buffer where we are currently writing
	char complete; // set to 1 when we have completed the buffer
	short datalength; // the length of the data in the buffer
	unsigned char *data; // pointer to the data
	unsigned char *lenptr; // pointer to datalength (same as *data)
	unsigned char *dataptr; // pointer to the start of the actual data (*data + 2)
};

typedef struct netBufferStruct netBuffer;

#define netMaxBufferSize 4096

netBuffer* netInitBuffer (int size); //initialize a new netBuffer of a specified size in bytes

int netSendBuffer (netSocket *netSock, netBuffer *netBuff); // send data to a netSocket (in buffer form)
int netRecvBuffer (netSocket *netSock, netBuffer *netBuff); // receive data from a netSocket (in buffer form)

int netBufferAppendInt8 (netBuffer *netBuff, char data); // append an 8-bit int to a netBuffer
int netBufferAppendInt16 (netBuffer *netBuff, short data); // append an 16-bit int to a netBuffer
int netBufferAppendInt32 (netBuffer *netBuff, int data); // append an 32-bit int to a netBuffer
int netBufferAppendString (netBuffer *netBuff, const char *data); // append a string to a netBuffer
int netBufferAppendData (netBuffer *netBuff, const char *data, int bytes); // append raw data to a netBuffer

char netBufferRemoveInt8 (netBuffer *netBuff); // remove an 8-bit int from a netBuffer
short netBufferRemoveInt16 (netBuffer *netBuff); // remove an 16-bit int from a netBuffer
int netBufferRemoveInt32 (netBuffer *netBuff); // remove an 32-bit int from a netBuffer
int netBufferRemoveString (char *data, netBuffer *netBuff); // remove a string from a netBuffer
int netBufferRemoveData (char *data, netBuffer *netBuff, int bytes); // append raw data to a netBuffer

int netBufferClear (netBuffer *netBuff); // clear a netBuffer
