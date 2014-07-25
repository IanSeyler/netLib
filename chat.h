/*
	chat.h - Global header file for Chat and Chatserver
	v0.2 - 2007.08.30
	Coded by Ian Seyler (iseyler@gmail.com)
	http://bubbai.ath.cx/
	Copyright (c) 2007 Ian Seyler. All Rights Reserved.
	Distributed under the terms of the MIT License.
*/


#include <stdio.h> // printf
#include <stdlib.h> // malloc
#include <string.h> // memset

#define ChatPort 8192
#define MessageSize 128
#define MaxConcurrentClients 10
