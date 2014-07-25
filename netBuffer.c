/*
	netBuffer.c - Module file for netLib Network Communication Library (Buffers)
	v0.4 - 2007.08.30
	Coded by Ian Seyler (iseyler@gmail.com)
	http://bubbai.ath.cx/projects/netlib/
	Copyright (c) 2007 Ian Seyler. All Rights Reserved.
	Distributed under the terms of the MIT License.
*/


#include "netBuffer.h"


/*========================================================*
 * Function : netInitBuffer()
 * Purpose  : Initiate a new netBuffer.
 * Entry    : int size = The size in bytes of the buffer
 * Returns  : a pointer to a netBuffer, NULL on error
 *========================================================*/

netBuffer* netInitBuffer (int size)
{
	netBuffer *netBuff;

	#if nlVerbose >= 1
		fprintf(stderr, "\nnetLib: InitBuffer");
	#endif

	if (size > netMaxBufferSize)
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", Error! (Buffer over set size limit)");
		#endif
		return NULL;
	}

	netBuff = malloc(sizeof(netBuffer));
	if (NULL == netBuff)
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", Error! (Allocating Pointer Buffer)");
		#endif
		return NULL;
	}
	else
	{
		memset(netBuff, '\0', sizeof(netBuffer)); // zero the entire structure
		netBuff->data = malloc(size+2); // add two since datalength is at begining of buffer
		if (NULL == netBuff->data)
		{
			#if nlVerbose >= 1
				fprintf(stderr, ", Error! (Allocating Data Buffer)");
			#endif
			free(netBuff); // We couln't get enough memory for the data buffer so free what we have used so far.
			return NULL;		
		}
		else
		{
			netBuff->size = size; // set the size for buffer data
			memset(netBuff->data, '\0', size+2); // zero the data structure
			netBuff->lenptr = netBuff->data;
			netBuff->dataptr = netBuff->data + 2;
			return netBuff;
		}
	}
}


/*========================================================*
 * Function : netSendBuffer()
 * Purpose  : Send data from a netBuffer to a netSocket.
 * Entry    : netSocket *netSock = The socket to send to
 *            netBuffer *netBuff = The buffer to be sent out
 * Returns  : number of bytes that were sent, -1 on error
 *========================================================*/

int netSendBuffer (netSocket *netSock, netBuffer *netBuff)
{
	int byte_count = 0;
	int bytes_sent = 0; // keep track of how many bytes have been sent in this run
	int bytes_left = (netBuff->datalength+2) - netBuff->readAt;
	short temp = htons(netBuff->datalength);
	#if nlVerbose >= 2
		int verbosetint;
	#endif

	#if nlVerbose >= 1
		fprintf(stderr, "\nnetLib: Send Buffer");
	#endif

	#if nlVerbose >= 2
		fprintf(stderr, "\nSENDBUFF: ");
		for (verbosetint=0; verbosetint<netBuff->datalength; verbosetint++)
			fprintf(stderr, "%02X", netBuff->dataptr[verbosetint]);
	#endif

	memcpy(&netBuff->data[0], (char*)&temp, 2);

	while(bytes_sent < netBuff->datalength+2) // keep going until the buffer has been sent
	{
		#ifdef MSG_NOSIGNAL // Stop SIGPIPE errors in Linux
			byte_count = send(netSock->number, netBuff->data+bytes_sent, bytes_left, MSG_NOSIGNAL);
		#else
			byte_count = send(netSock->number, netBuff->data+bytes_sent, bytes_left, 0);
		#endif

		#if nlVerbose >= 1
			fprintf(stderr, ", transfered %d bytes", byte_count);
		#endif

		if (0 >= byte_count)
		{
			#if nlVerbose >= 1
				printf (", Connection closed");
				#if nlVerbose >= 2
					#ifdef MS_WINDOWS
						fprintf(stderr, ", ERRNO = %d", WSAGetLastError());
					#else
						fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
					#endif
				#endif
			#endif
			netDisconnect(netSock); // Since the connection has closed we can disconnect it
			netBuff->readAt = 0; // Transfer has failed. Reset at for next call of netSendBuffer
			netBuff->complete = 0;
			netSock->retval = -1;
			return -1;
		}
		else
		{
			bytes_sent += byte_count;
			netBuff->readAt += byte_count; // keep track of where we are in the buffer
			bytes_left -= byte_count;
			#if nlVerbose >= 1
				if (bytes_sent < netBuff->datalength+2)
					fprintf(stderr, ", waiting to send %d bytes", bytes_left);
			#endif
		}
	}

	if (netBuff->readAt == netBuff->datalength+2) // We sent everything we need.. reset at and set complete
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", complete!");
		#endif
		netBuff->readAt = 0;
		netBuff->complete = 1;
	}

	netSock->retval = bytes_sent;
	return bytes_sent;
}


/*========================================================*
 * Function : netRecvBuffer()
 * Purpose  : Receive data from a netSocket to a netBuffer.
 * Entry    : netSocket *netSock = The socket to receive from
 *            netBuffer *netBuff = The buffer where the incoming data will be stored
 * Returns  : number of bytes that were received, -1 on error
 *========================================================*/

int netRecvBuffer (netSocket *netSock, netBuffer *netBuff)
{
	int byte_count = 0;
	int size = 0;
	short length = 0;
	int bytes_recv = 0; // keep track of how many bytes have been received in this run
	int bytes_left = netBuff->datalength - netBuff->readAt;
	short temp = 0;
	#if nlVerbose >= 2
		int verbosetint;
	#endif

	#if nlVerbose >= 1
		fprintf(stderr, "\nnetLib: Receive Buffer");
	#endif

	#if nlVerbose >= 2
		fprintf(stderr, "\nRECVBUFF: ");
		for (verbosetint=0; verbosetint<netBuff->datalength; verbosetint++)
			fprintf(stderr, "%02X", netBuff->dataptr[verbosetint]);
	#endif

	if(1 < netIsDataPending(netSock))
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", nothing is pending");
		#endif
		netSock->retval = 0;
		return 0;
	}

	if (0 == netBuff->readAt)
	{
		netBuff->complete = 0;

		byte_count = recv(netSock->number, (void *)&temp, 2, 0);
		if (0 >= byte_count)
		{
			#if nlVerbose >= 1
				printf (", Connection closed");
				#if nlVerbose >= 2
					#ifdef MS_WINDOWS
						fprintf(stderr, ", ERRNO = %d", WSAGetLastError());
					#else
						fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
					#endif
				#endif
			#endif
			netDisconnect(netSock); // Since the connection has closed we can disconnect it
			netBuff->readAt = 0; // Transfer has failed. Reset at for next call of netRecvBuffer
			netBuff->complete = 0;
			netSock->retval = -1;
			return -1;
		}

		length = ntohs(temp);
		if (length > netBuff->size)
		{
			return -1; // MAJOR ERROR.. we got a packet larger then we can hold.
			printf("MAJOR ERROR!\n");
		}

		netBuff->datalength = length;
	}

	bytes_left = length - netBuff->readAt;

	while(0 != bytes_left) // keep going until the buffer is full
	{
		byte_count = recv(netSock->number, netBuff->dataptr+netBuff->readAt, bytes_left, 0);

		#if nlVerbose >= 1
			fprintf(stderr, ", transfered %d bytes", byte_count);
		#endif

		if (0 >= byte_count)
		{
			#if nlVerbose >= 1
				printf (", Connection closed");
				#if nlVerbose >= 2
					#ifdef MS_WINDOWS
						fprintf(stderr, ", ERRNO = %d", WSAGetLastError());
					#else
						fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
					#endif
				#endif
			#endif
			netDisconnect(netSock); // Since the connection has closed we can disconnect it
			netBuff->readAt = 0; // Transfer has failed. Reset at for next call of netRecvBuffer
			netBuff->complete = 0;
			netSock->retval = -1;
			return -1;
		}
		else
		{
			bytes_recv += byte_count;
			netBuff->readAt += byte_count; // keep track of where we are in the buffer
			bytes_left -= byte_count;
			#if nlVerbose >= 1
				if (bytes_recv < netBuff->datalength)
					fprintf(stderr, ", waiting to receive %d bytes", bytes_left);
			#endif

			if (1 > netIsDataPending(netSock))
			{
				if (netBuff->readAt == netBuff->datalength) // We got everything we need.. reset at and set complete
				{
					netBuff->readAt = 0;
					netBuff->complete = 1;
					netSock->retval = bytes_recv;
					#if nlVerbose >= 1
						fprintf(stderr, ", complete!");
					#endif
					return bytes_recv;
				}
				#if nlVerbose >= 1
					fprintf(stderr, ", nothing more is pending");
				#endif
				netSock->retval = bytes_recv;
				return bytes_recv;
			}
		}
	}

	if (netBuff->readAt == netBuff->datalength) // We got everything we need.. reset at and set complete
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", complete!");
		#endif
		netBuff->readAt = 0;
		netBuff->complete = 1;
	}

	netSock->retval = bytes_recv;
	return bytes_recv;
}


/*========================================================*
 * Function : netBufferAppendInt8()
 * Purpose  : Pack an 8-bit integer into a netBuffer.
 * Entry    : netBuffer *netBuff = The buffer where the integer data will be stored
 *            char data = The integer to store
 * Returns  : 0 on success, -1 on error
 *========================================================*/

int netBufferAppendInt8 (netBuffer *netBuff, char data)
{
	if (netBuff->writeAt >= netBuff->size)
	{
		return -1;
	}

	netBuff->dataptr[netBuff->writeAt] = (char)data;
	netBuff->writeAt++;
	netBuff->datalength++;
	return 0;
}


/*========================================================*
 * Function : netBufferAppendInt16()
 * Purpose  : Pack a 16-bit integer into a netBuffer.
 * Entry    : netBuffer *netBuff = The buffer where the integer data will be stored
 *            short data = The integer to store
 * Returns  : 0 on success, -1 on error
 *========================================================*/

int netBufferAppendInt16 (netBuffer *netBuff, short data)
{
	short tshort = htons(data);
	char *ptr = (char*)&tshort;

	if (netBuff->writeAt >= netBuff->size)
	{
		return -1;
	}

	memcpy(&netBuff->dataptr[netBuff->writeAt], ptr, 2);
	netBuff->writeAt += 2;
	netBuff->datalength += 2;
	return 0;
}


/*========================================================*
 * Function : netBufferAppendInt32()
 * Purpose  : Pack a 32-bit integer into a netBuffer.
 * Entry    : netBuffer *netBuff = The buffer where the integer data will be stored
 *            int data = The integer to store
 * Returns  : 0 on success, -1 on error
 *========================================================*/

int netBufferAppendInt32 (netBuffer *netBuff, int data)
{
	int tint = htonl(data);
	char *ptr = (char*)&tint;

	if (netBuff->writeAt >= netBuff->size)
	{
		return -1;
	}

	memcpy(&netBuff->dataptr[netBuff->writeAt], ptr, 4);
	netBuff->writeAt += 4;
	netBuff->datalength += 4;
	return 0;
}


/*========================================================*
 * Function : netBufferAppendString()
 * Purpose  : Pack a string into a netBuffer.
 * Entry    : netBuffer *netBuff = The buffer where the string will be stored
 *            const char *data = The string to store
 * Returns  : 0 on success, -1 on error
 *========================================================*/

int netBufferAppendString (netBuffer *netBuff, const char *data)
{
	int len;
	len = strlen(data);

	if (len > (netBuff->size - netBuff->writeAt))
	{
		return -1;
	}
	strcpy((unsigned char*)netBuff->dataptr+netBuff->writeAt, (unsigned char*)data);
	netBuff->writeAt += len+1;
	netBuff->datalength += len+1;

	return 0;
}


/*========================================================*
 * Function : netBufferAppendData()
 * Purpose  : Pack raw data into a netBuffer.
 * Entry    : netBuffer *netBuff = The buffer where the data will be stored
 *            const char *data = The data to store
 *            int bytes = The number of bytes to store
 * Returns  : 0 on success, -1 on error
 *========================================================*/

int netBufferAppendData (netBuffer *netBuff, const char *data, int bytes)
{
	if (bytes > (netBuff->size - netBuff->writeAt))
	{
		return -1;
	}
	memcpy(&netBuff->dataptr[netBuff->writeAt], data, bytes);
	netBuff->writeAt += bytes;
	netBuff->datalength += bytes;

	return 0;
}


/*========================================================*
 * Function : netBufferRemoveInt8()
 * Purpose  : Unpack an 8-bit integer from a netBuffer.
 * Entry    : netBuffer *netBuff = The buffer where the integer is stored
 * Returns  : the 8-bit integer, -1 on error (not a reliable error number)
 *========================================================*/

char netBufferRemoveInt8 (netBuffer *netBuff)
{
	char tchar;

	if (netBuff->readAt >= netBuff->size)
	{
		return -1;
	}
	tchar = netBuff->dataptr[netBuff->readAt];
	netBuff->readAt++;

	return tchar;
}


/*========================================================*
 * Function : netBufferRemoveInt16()
 * Purpose  : Unpack a 16-bit integer from a netBuffer.
 * Entry    : netBuffer *netBuff = The buffer where the integer is stored
 * Returns  : the 16-bit integer, -1 on error (not a reliable error number)
 *========================================================*/

short netBufferRemoveInt16 (netBuffer *netBuff)
{
	short tshort;

	if (netBuff->readAt >= netBuff->size)
	{
		return -1;
	}

	memcpy(&tshort, &netBuff->dataptr[netBuff->readAt], 2);
	netBuff->readAt += 2;
	return ntohs(tshort);
}


/*========================================================*
 * Function : netBufferRemoveInt32()
 * Purpose  : Unpack a 32-bit integer from a netBuffer.
 * Entry    : netBuffer *netBuff = The buffer where the integer is stored
 * Returns  : the 32-bit integer, -1 on error (not a reliable error number)
 *========================================================*/

int netBufferRemoveInt32 (netBuffer *netBuff)
{
	int tint;

	if (netBuff->readAt >= netBuff->size)
	{
		return -1;
	}

	memcpy(&tint, &netBuff->dataptr[netBuff->readAt], 4);
	netBuff->readAt += 4;

	return ntohl(tint);
}


/*========================================================*
 * Function : netBufferRemoveString()
 * Purpose  : Unpack a string from a netBuffer.
 * Entry    : char *data = Where the string will be unpacked to
 *            netBuffer *netBuff = The buffer where the string is stored
 * Returns  : 0 on success, -1 on error
 *========================================================*/

int netBufferRemoveString (char *data, netBuffer *netBuff)
{
	int tint, strlen = 0;
	tint = netBuff->readAt;

	while (netBuff->data[tint] != 0x00)
	{
		tint++;
		strlen++;
	}

	strcpy((unsigned char*)data, (unsigned char*)netBuff->dataptr+netBuff->readAt);	
	netBuff->readAt += strlen-1;

	return 0;
}


/*========================================================*
 * Function : netBufferRemoveData()
 * Purpose  : Unpack raw data from a netBuffer.
 * Entry    : char *data = Where the data will be unpacked to
 *            netBuffer *netBuff = The buffer where the data is stored
 *            int bytes = The number of bytes to copy
 * Returns  : 0 on success, -1 on error
 *========================================================*/

int netBufferRemoveData (char *data, netBuffer *netBuff, int bytes)
{
	memcpy(data, &netBuff->dataptr[netBuff->writeAt], bytes);

	netBuff->readAt += bytes;

	return 0;
}


/*========================================================*
 * Function : netBufferClear()
 * Purpose  : Clear a netBuffer.
 * Entry    : netBuffer *netBuff = The buffer to be cleared
 * Returns  : 0 on success
 *========================================================*/

int netBufferClear(netBuffer *netBuff)
{
	netBuff->datalength = 0;
	netBuff->writeAt = 0;
	netBuff->readAt = 0;			
	memset(netBuff->data, '\0', netBuff->size+2); // zero the data structure

	return 0;
}
