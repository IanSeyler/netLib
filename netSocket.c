/*
	netSocket.c - Module file for netLib Network Communication Library (Main)
	v0.4 - 2007.08.30
	Coded by Ian Seyler (iseyler@gmail.com)
	http://bubbai.ath.cx/projects/netlib/
	Copyright (c) 2007 Ian Seyler. All Rights Reserved.
	Distributed under the terms of the MIT License.
*/


#include "netSocket.h"


/*
Used for setting the verbose mode of the library.
0 = Silent, 1 = Verbose, 2 = Verbose + Debug
*/
#ifndef nlVerbose
	#define nlVerbose 0
#endif

int nlEndian; // Global variable for the host endian


/*========================================================*
 * Function : netInit()
 * Purpose  : Initiate the netLib library.
 * Entry    : nothing
 * Returns  : 0 on success, -1 on failure
 *========================================================*/

int netInit ()
{
	int endiantest = 1;

	#ifdef MS_WINDOWS
	{
		fprintf(stderr, "\nnetLib: Start Windows networking");
		WSADATA wsaData; // If WSADATA doesn't work then try WSAData instead.
		if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) // Init the WinSock library.
		{
			fprintf(stderr, ", WSAStartup Error!\n");
			return -1;
		}
	}
	#endif
	#ifdef UNIX
		fprintf(stderr, "\nnetLib: Start Unix networking");
		#ifdef SIGPIPE
            signal(SIGPIPE, SIG_IGN); // This removes "Broken Pipe" errors in BSD
        #endif
	#endif
	#ifdef BEOS
		fprintf(stderr, "\nnetLib: Start BeOS networking");
	#endif
	fprintf(stderr, " (v0.4)"); // The version of netLib
	#if nlVerbose == 1
		fprintf(stderr, " - Verbose mode");
	#endif
	#if nlVerbose == 2
		fprintf(stderr, " - Verbose + Debug mode");
	#endif

	if(1 == *(char *)&endiantest)
		nlEndian = 0; // Little endian - Intel
	else
		nlEndian = 1; // Big endian - PPC

	if (sizeof(int) != 4 && sizeof(short) != 2 && sizeof(char) != 1)
		fprintf(stderr, " - Possible buffer errors!");

	return 0;
}


/*========================================================*
 * Function : netStop()
 * Purpose  : Stop the netLib library.
 * Entry    : nothing
 * Returns  : 0 on success, -1 on failure
 *========================================================*/

int netStop ()
{
	#if nlVerbose >= 1
		fprintf(stderr, "\nnetLib: Stop");
	#endif

	#ifdef MS_WINDOWS
	{
		WSACleanup(); // We are done using the WinSock library.
	}
	#endif
	return 0;
}


/*========================================================*
 * Function : netInitSocket()
 * Purpose  : Initiate a new netSocket.
 * Entry    : nothing
 * Returns  : a pointer to a netSocket, NULL on error
 *========================================================*/

netSocket* netInitSocket ()
{
	netSocket *netSock;

	#if nlVerbose >= 1
		fprintf(stderr, "\nnetLib: InitSocket");
	#endif

	netSock = malloc(sizeof(netSocket));
	if (NULL == netSock)
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", Error!");
		#endif
		return NULL;
	}
	else
	{
		memset(netSock, '\0', sizeof(netSocket)); // zero the entire structure
		netSetTimeout(netSock, 0.1);
		return netSock;
	}
}


/*========================================================*
 * Function : netConnect()
 * Purpose  : Connect a netSocket to the specified destination.
 * Entry    : netSocket *netSock = Socket to connect
 *            const char *address = Character string of the destination (eg: "bubbai.ath.cx" or "192.168.0.1")
 *            unsigned short port = The port on the destination to connect to
 * Returns  : 0 on success, -1 on failure
 *========================================================*/

int netConnect (netSocket *netSock, const char *address, unsigned short port)
{
	int tint = 0, socketargs = 0, setsockint = 1;
	struct hostent *h;

	#if nlVerbose >= 1
		fprintf(stderr, "\nnetLib: Connect");
		fprintf(stderr, " %s:%d", address, port);
	#endif

	memset(&netSock->addr, '\0', sizeof(netSock->addr));

	h = gethostbyname(address); // convert host name to IP and hold it in hostent structure
	if (NULL == h)
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", name lookup failed");
		#endif
		netSock->retval = -1;
		return -1; // DNS has failed.. bail out.
	}

	#if nlVerbose >= 1
		printf(" (Official name is: %s, IP address: %s)", h->h_name, inet_ntoa(*(struct in_addr*)h->h_addr));
	#endif

	netSock->addr.sin_family = AF_INET;
	netSock->addr.sin_port = htons(port); // setup the port we will be connecting to.
	netSock->addr.sin_addr.s_addr = *((unsigned long *) h->h_addr); // Translate the IP address string to a usable address.
	netSock->number = socket(PF_INET, SOCK_STREAM, socketargs);
	if (-1 == netSock->number)
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", socket ERROR");
		#endif
		netSock->retval = -1;
		return -1;
	}

	tint = connect(netSock->number, (struct sockaddr*)&netSock->addr, sizeof(netSock->addr)); // connect the socket

	if (0 != tint)
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", FAILED");
		#endif
		netDisconnect (netSock); // properly close the socket
		netSock->retval = -1;
		return -1;
	}
	else
	{
		#ifdef TCP_NODELAY // Disable the Nagle algorithm
		{
			#ifdef MS_WINDOWS
				BOOL optval = TRUE;
				int optlen = sizeof(BOOL);
				if (-1 == setsockopt(netSock->number, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, optlen))
				{
				#if nlVerbose >= 1
					fprintf(stderr, ", setsockopt (Nagle) Warning");
					#if nlVerbose >= 2
						fprintf(stderr, ", ERRNO = %d", WSAGetLastError());
					#endif
				#endif
				}
			#else
				if (-1 == setsockopt(netSock->number, IPPROTO_TCP, TCP_NODELAY, &setsockint, sizeof(int)))
				{
				#if nlVerbose >= 1
					fprintf(stderr, ", setsockopt (Nagle) Warning");
					#if nlVerbose >= 2
						fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
					#endif
				#endif
				}
			#endif
		}
		#endif
		#ifdef SO_NOSIGPIPE // Stops "Broken pipe" errors in Mac OS X and some Unix distributions. SIGPIPE will stop the program from running.
		{
			if (-1 == setsockopt(netSock->number, SOL_SOCKET, SO_NOSIGPIPE, &setsockint, sizeof(int)))
			{
				#if nlVerbose >= 1
					fprintf(stderr, ", setsockopt (Sigpipe) Error");
					#if nlVerbose >= 2
						fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
					#endif
				#endif
				netDisconnect(netSock);
				return -1;
			}
		}
		#endif
		netSock->inuse = 1; // mark this socket as being in use since it was successful
		netSock->retval = 0;

		return 0;
	}
}


/*========================================================*
 * Function : netDisconnect()
 * Purpose  : Disconnect a netSocket.
 * Entry    : netSocket *netSock = The socket to disconnect
 * Returns  : 0 on success, -1 on failure
 *========================================================*/

int netDisconnect (netSocket *netSock)
{
	#if nlVerbose >= 1
		fprintf(stderr, "\nnetLib: Disconnect");
	#endif

	shutdown(netSock->number, 2); // Stop reads and writes

	#ifdef UNIX // Unix uses close(), Windows and BeOS use closesocket().
	{
		close(netSock->number);
	}
	#else
	{	
		closesocket(netSock->number);
	}
	#endif

	memset(&netSock->addr, '\0', sizeof(netSock->addr));
	netSock->inuse = 0;
	netSock->number = 0;
	netSock->retval = 0;

	return 0;
}


/*========================================================*
 * Function : netListen()
 * Purpose  : Listen on a specified netSocket.
 * Entry    : netSocket *netSock = Socket to listen on
 *            unsigned short port = Port number to listen on
 * Returns  : 0 on success, -1 on failure
 *========================================================*/

int netListen (netSocket *netSock, unsigned short port)
{
	int tint = 0, socketargs = 0;

	#ifndef MS_WINDOWS
		int setsockint = 1;
	#endif

	#if nlVerbose >= 1
		fprintf(stderr, "\nnetLib: Listen");
	#endif

	memset(&netSock->addr, '\0', sizeof(netSock->addr));

	netSock->addr.sin_family = AF_INET;
	netSock->addr.sin_port = htons(port); // The clients will connect to this port
	netSock->addr.sin_addr.s_addr = INADDR_ANY; // autoselect IP address
	netSock->number = socket(PF_INET, SOCK_STREAM, socketargs);
	if (-1 == netSock->number)
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", socket ERROR");
		#endif
		return -1;
	}

	#ifdef MS_WINDOWS
		BOOL optval = TRUE;
		int optlen = sizeof(BOOL);
		if (-1 == setsockopt(netSock->number, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, optlen))
		{
			#if nlVerbose >= 1
				fprintf(stderr, ", setsockopt (Reuse) Warning");
				#if nlVerbose >= 2
					fprintf(stderr, ", ERRNO = %d", WSAGetLastError());
				#endif
			#endif
		}
	#else
		if (-1 == setsockopt(netSock->number, SOL_SOCKET, SO_REUSEADDR, &setsockint, sizeof(int)))
		{
			#if nlVerbose >= 1
				fprintf(stderr, ", setsockopt (Reuse) Warning");
				#if nlVerbose >= 2
					fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
				#endif
			#endif
		}
	#endif

	tint = bind(netSock->number, (struct sockaddr*)&netSock->addr, sizeof(struct sockaddr));
	if (0 != tint)
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", Bind Failure");
			#if nlVerbose >= 2
				#ifdef MS_WINDOWS
					fprintf(stderr, ", ERRNO = %d", WSAGetLastError());
				#else
					fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
				#endif
			#endif
		#endif
		netDisconnect(netSock);
		netSock->retval = -1;
		return -1;
	}

	tint = listen(netSock->number, 10); // set up to be a server (listening) socket, 2nd argument is backlog
	if (0 != tint)
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", Listen Failure");
			#if nlVerbose >= 2
				#ifdef MS_WINDOWS
					fprintf(stderr, ", ERRNO = %d", WSAGetLastError());
				#else
					fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
				#endif
			#endif
		#endif
		netDisconnect(netSock);
		netSock->retval = -1;
		return -1;
	}

	netSock->retval = 0;
	return 0;
}


/*========================================================*
 * Function : netAccept()
 * Purpose  : Accept a connection from a listening port.
 * Entry    : netSocket *listenSock = The listening socket
 *            netSocket *netSock = The netSocket that has just connected
 * Returns  : 1 on an accepted connection, 0 on nothing to accept, -1 on error
 *========================================================*/

int netAccept (netSocket *listenSock, netSocket *netSock)
{
	socklen_t addrlen = sizeof(netSock->addr); // set to a value since windows needs it
	int setsockint = 1;

	#if nlVerbose >= 1
		fprintf(stderr, "\nnetLib: Accept");
	#endif

	if(1 > netIsDataPending(listenSock))
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", no pending connection");
		#endif
		netSock->retval = 0;
		return 0;
	}

	memset(&netSock->addr, '\0', sizeof(netSock->addr));

	netSock->number = accept(listenSock->number, (struct sockaddr*)&netSock->addr, &addrlen);

	if (-1 == netSock->number)
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", Error");
		#endif
		#if nlVerbose >= 2
			#ifdef MS_WINDOWS
				fprintf(stderr, ", ERRNO = %d", WSAGetLastError());
			#else
				fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
			#endif
		#endif
		netSock->retval = -1;
		return -1;
	}
	else
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", received connection from %s", inet_ntoa(netSock->addr.sin_addr));
		#endif
		#ifdef TCP_NODELAY // Disable the Nagle algorithm
		{
			#ifdef MS_WINDOWS
				BOOL optval = TRUE;
				int optlen = sizeof(BOOL);
				if (-1 == setsockopt(netSock->number, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, optlen))
				{
				#if nlVerbose >= 1
					fprintf(stderr, ", setsockopt (Nagle) Warning");
					#if nlVerbose >= 2
						fprintf(stderr, ", ERRNO = %d", WSAGetLastError());
					#endif
				#endif
				}
			#else
				if (-1 == setsockopt(netSock->number, IPPROTO_TCP, TCP_NODELAY, &setsockint, sizeof(int)))
				{
				#if nlVerbose >= 1
					fprintf(stderr, ", setsockopt (Nagle) Warning");
					#if nlVerbose >= 2
						fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
					#endif
				#endif
				}
			#endif
		}
		#endif
		#ifdef SO_NOSIGPIPE // Stops "Broken pipe" errors in Mac OS X and some Unix distributions. SIGPIPE will stop the program from running.
		{
			if (-1 == setsockopt(netSock->number, SOL_SOCKET, SO_NOSIGPIPE, &setsockint, sizeof(int)))
			{
				#if nlVerbose >= 1
					fprintf(stderr, ", setsockopt (Sigpipe) Error");
					#if nlVerbose >= 2
						fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
					#endif
				#endif
				netDisconnect(netSock); // Since the connection has closed we can disconnect it
				netSock->retval = -1;
				return -1;
			}
		}
		#endif

		netSock->inuse = 1; // mark the socket as being in use
		netSock->retval = 1;
		return 1;
	}
}


/*========================================================*
 * Function : netIsDataPending()
 * Purpose  : Checks if a netSocket is waiting for action.
 * Entry    : netSocket *netSock = The socket to check
 * Returns  : 1 if data is pending, 0 if no data is pending
 *========================================================*/

int netIsDataPending (netSocket *netSock)
{
	fd_set selectlist;
	struct timeval tv;

	#if nlVerbose >= 2 // This message will show up a lot on debug level 2
		fprintf(stderr, "\nnetLib: IsDataPending");
	#endif

	FD_ZERO(&selectlist);
	FD_SET(netSock->number, &selectlist);

	tv.tv_sec = netSock->timeout.tv_sec; // Set the timeout delay from the netSocket
	tv.tv_usec = netSock->timeout.tv_usec;

	select(netSock->number+1, &selectlist, NULL, NULL, &tv);

	if (0 != FD_ISSET(netSock->number, &selectlist))
		netSock->retval = 1;
	else
		netSock->retval = 0;

	return netSock->retval;
}


/*========================================================*
 * Function : netSend()
 * Purpose  : Send a number of bytes to a netSocket.
 * Entry    : netSocket *netSock = Socket to send data to
 *            const char *data = Pointer to a character string, this is the data that will be transfered
 *            int bytes = the number of bytes to send starting at *data
 * Returns  : number of bytes that were sent, -1 on error
 *========================================================*/

int netSend (netSocket *netSock, const char *data, int bytes) // const void* data
{
	int byte_count = 0;

	#if nlVerbose >= 1
		fprintf(stderr, "\nnetLib: Send");
	#endif

	#ifdef MSG_NOSIGNAL // Stop SIGPIPE errors in Linux
		byte_count = send(netSock->number, data, bytes, MSG_NOSIGNAL);
	#else
		byte_count = send(netSock->number, data, bytes, 0);
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
		netSock->retval = -1;
		return -1;
	}

	netSock->retval = byte_count;
	return byte_count;
}


/*========================================================*
 * Function : netRecv()
 * Purpose  : Recveive a number of bytes from a netSocket.
 * Entry    : netSocket *netSock = Socket to receive data from
 *            char *data = Pointer to a character string, this is where the data will be stored
 *            int bytes = the number of bytes to receive
 * Returns  : number of bytes that were received, -1 on error
 *========================================================*/

int netRecv (netSocket *netSock, char *data, int bytes) // const void* data
{
	int byte_count = 0;

	#if nlVerbose >= 1
		fprintf(stderr, "\nnetLib: Receive");
	#endif

	if(1 > netIsDataPending(netSock))
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", nothing is pending");
		#endif
		netSock->retval = 0;
		return 0;
	}

	byte_count = recv(netSock->number, data, bytes, 0);

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
		netSock->retval = -1;
		return -1;
	}

	netSock->retval = byte_count;
	return byte_count;
}


/*========================================================*
 * Function : netSetTimeout()
 * Purpose  : Set the timeout on a netSocket.
 * Entry    : netSocket *netSock = The socket to set
 *            float timeoutvalue = The new timeout value
 * Returns  : 0 on success
 *========================================================*/

int netSetTimeout (netSocket *netSock, float timeoutvalue)
{
	#if nlVerbose >= 1
		fprintf(stderr, "\nnetLib: SetTimeout");
	#endif

	netSock->timeout.tv_sec = (int)timeoutvalue;
	netSock->timeout.tv_usec = (timeoutvalue - netSock->timeout.tv_sec) * 1000000;

	netSock->retval = 0;
	return 0;
}


/*========================================================*
 * Function : netSetOption()
 * Purpose  : Set an option on a netSocket.
 * Entry    : netSocket *netSock = The socket to set
 *            int option = The option to set
 *            int enable = 1 for enable, 0 for disable
 * Returns  : 0 on success, -1 on failure
 *========================================================*/

int netSetOption (netSocket *netSock, int option, int enable)
{
	int setsockint;
	
	#if nlVerbose >= 1
		fprintf(stderr, "\nnetLib: SetOption");
	#endif

	#ifdef MS_WINDOWS
		BOOL optval;
		int optlen = sizeof(BOOL);
		if (1 == enable)
		{
			optval = FALSE;
		}
		else
		{
			optval = TRUE;
		}
	#else
		if (1 == enable)
		{
			setsockint = 0;
		}
		else
		{
			setsockint = 1;
		}
	#endif

	if (option == netDontRoute)
	{
		#ifdef SO_DONTROUTE
			#if nlVerbose >= 1
				fprintf(stderr, ", netDontRoute");
			#endif
			#ifdef MS_WINDOWS
				if (-1 == setsockopt(netSock->number, SOL_SOCKET, SO_DONTROUTE, (char*)&optval, optlen))
				{
					#if nlVerbose >= 1
						fprintf(stderr, ", setsockopt (Reuse) Warning");
						#if nlVerbose >= 2
							fprintf(stderr, ", ERRNO = %d", WSAGetLastError());
						#endif
					#endif
				}
			#else
				if (-1 == setsockopt(netSock->number, SOL_SOCKET, SO_DONTROUTE, &setsockint, sizeof(int)))
				{
					#if nlVerbose >= 1
						fprintf(stderr, ", setsockopt (Reuse) Warning");
						#if nlVerbose >= 2
							fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
						#endif
					#endif
				}
			#endif
		#endif
	}
	else if (option == netNagleAlgorithm)
	{
		#ifdef TCP_NODELAY
			#if nlVerbose >= 1
				fprintf(stderr, ", netNagleAlgorithm");
			#endif
			#ifdef MS_WINDOWS
				if (-1 == setsockopt(netSock->number, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, optlen))
				{
				#if nlVerbose >= 1
					fprintf(stderr, ", setsockopt (Nagle) Warning");
					#if nlVerbose >= 2
						fprintf(stderr, ", ERRNO = %d", WSAGetLastError());
					#endif
				#endif
				}
			#else
				if (-1 == setsockopt(netSock->number, IPPROTO_TCP, TCP_NODELAY, &setsockint, sizeof(int)))
				{
				#if nlVerbose >= 1
					fprintf(stderr, ", setsockopt (Nagle) Warning");
					#if nlVerbose >= 2
						fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
					#endif
				#endif
				}
			#endif
		#endif
	}
	else if (option == netDebug)
	{
		#ifdef SO_DEBUG
			#if nlVerbose >= 1
				fprintf(stderr, ", netDebug");
			#endif
			#ifdef MS_WINDOWS
				if (-1 == setsockopt(netSock->number, SOL_SOCKET, SO_DEBUG, (char*)&optval, optlen))
				{
				#if nlVerbose >= 1
					fprintf(stderr, ", setsockopt (Debug) Warning");
					#if nlVerbose >= 2
						fprintf(stderr, ", ERRNO = %d", WSAGetLastError());
					#endif
				#endif
				}
			#else
				if (-1 == setsockopt(netSock->number, SOL_SOCKET, SO_DEBUG, &setsockint, sizeof(int)))
				{
				#if nlVerbose >= 1
					fprintf(stderr, ", setsockopt (Debug) Warning");
					#if nlVerbose >= 2
						fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
					#endif
				#endif
				}
			#endif
		#endif
	}
	else if (option == netNonBlocking)
	{
		#if nlVerbose >= 1
			fprintf(stderr, ", netNonBlocking");
		#endif
		#ifdef MS_WINDOWS
			u_long val;
			if (1 == enable)
				val = 1;
			else
				val = 0;
			if (-1 == ioctlsocket(netSock->number, FIONBIO, &val))
			{
			#if nlVerbose >= 1
				fprintf(stderr, ", ioctlsocket (NonBlocking) Warning");
				#if nlVerbose >= 2
					fprintf(stderr, ", ERRNO = %d", WSAGetLastError());
				#endif
			#endif
			}
		#else
			#ifdef O_NONBLOCK
				if (-1 == fcntl(netSock->number, F_SETFL, O_NONBLOCK))
				{
				#if nlVerbose >= 1
					fprintf(stderr, ", fcntl (NonBlocking) Warning");
					#if nlVerbose >= 2
						fprintf(stderr, ", ERRNO = %d (%s)", errno, strerror(errno));
					#endif
				#endif
				}
			#endif
		#endif
	}	

	return 0;
}
