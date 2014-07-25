/*
	netSocket.h - Header file for netLib Network Communication Library (Main)
	v0.4 - 2007.08.30
	Coded by Ian Seyler (iseyler@gmail.com)
	http://bubbai.ath.cx/projects/netlib/
	Copyright (c) 2007 Ian Seyler. All Rights Reserved.
	Distributed under the terms of the MIT License.
*/


#ifndef __NETSOCKET_H__
#define __NETSOCKET_H__


#include <stdio.h> // fprintf
#include <stdlib.h> // malloc
#include <string.h> // memset
#ifdef MS_WINDOWS
	#include <winsock.h>
	#include <winerror.h>
	typedef int socklen_t; // Windows does not define this.
#endif
#ifdef UNIX
	#include <unistd.h>
	#include <errno.h> // to look at error return values
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h> // for dns lookup
	#include <fcntl.h>
#endif
#ifdef BEOS
	#include <unistd.h>
	#include <errno.h> // to look at error return values
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h> // for dns lookup
	#include <fcntl.h>
	typedef int socklen_t; // BeOS does not define this.
	#define PF_INET AF_INET // Just using the value for AF_INET (Mac OS X does the same thing)
#endif

/* Structure of a netSocket */
struct netSocketStruct {
	int number; // the socket descriptor from the socket() call
	int inuse; // 1 for active, 0 for not active
	int retval; // all functions will update this variable with the function return value
	struct timeval timeout; // timeout value
	struct sockaddr_in addr; // connector's address information
};

typedef struct netSocketStruct netSocket;

#define netOK 0 // For use in comparisons. ie: if (netConnect(tempsock, "blah.com", 80) == netOK)
#define netEnable 1 // For use with netSetOption
#define netDisable 0 // For use with netSetOption
#define netDontRoute 0 // For use with netSetOption
#define netNagleAlgorithm 1 // For use with netSetOption
#define netDebug 2 // For use with netSetOption
#define netNonBlocking 3 // For use with netSetOption

int netInit (); // initiate the library
int netStop (); // stop the library

netSocket* netInitSocket (); // initialize a new netSocket

int netConnect (netSocket *netSock, const char *address, unsigned short port); // connect to a netSocket
int netDisconnect (netSocket *netSock); // disconnect from a netSocket

int netListen (netSocket *netSock, unsigned short port); // open a netSocket and listen on it
int netAccept (netSocket *listenSock, netSocket *netSock); // accept a connection from a listening netSocket

int netIsDataPending (netSocket *netSock); // check to see if a netSocket has data that is pending to be received

int netSend (netSocket *netSock, const char *data, int bytes); // send data to a netSocket
int netRecv (netSocket *netSock, char *data, int bytes); // receive data from a netSocket

int netSetTimeout (netSocket *netSock, float timeoutvalue); // set the timeout value on a netsocket
int netSetOption (netSocket *netSock, int option, int enable); // set an option on a netsocket


#endif
