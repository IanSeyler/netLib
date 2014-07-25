netLib Network Communication Library
v0.4 - 2007.08.30
Coded by Ian Seyler (iseyler@gmail.com)
http://bubbai.ath.cx/projects/netlib/
Copyright (c) 2007 Ian Seyler. All Rights Reserved.
Distributed under the terms of the MIT License.



Index
------
1) What is netLib?
2) License
3) Files included in the distribution
4) How to use netLib
5) Compile Example
6) References
7) Final Notes



1) What is netLib?
-------------------
The netLib Network Communication Library is a free, open-source C library for network communication over TCP/IP. It currently compiles on Windows, Unix (Linux, *BSD, Mac OS X) and BeOS. The netLib Network Communication Library was designed for simplicity, portability, and ease of use.


2) License
-----------
This software is distributed under the MIT License. Please see license.txt for more information.


3) Files included in the distribution
--------------------------------------
chat.c - A chat client.
chat.h - Global header for chat and chatserver.
chatserver.c - A multiple client chat server.
echoserver.c - A simple server that you can connect to using a telnet client. It will echo back what you type.
netSocket.c - The module file for the netLib library main functions.
netSocket.h - The header file for the netLib library main functions.
netBuffer.c - The module file for the netLib library buffer functions.
netBuffer.h - The header file for the netLib library buffer fucntions.
README.txt - This text file.
LICENSE.txt - The license for netLib.
testclient.c - A test client.
testserver.c - A test server.
docs/changelog.txt - A running log detailing the changes between the release versions.
docs/manual.html - The current manual.
docs/style.css - Style sheet for the manual.


4) How to use netLib
---------------------
Consult the manual in the docs folder or check for the latest version online at http://bubbai.ath.cx/projects/netlib/manual.html
The included testclient, testserver, bufferclient, bufferserver, and chat programs can be used as guides for your own programs.
All included examples except for echoserver require netBuffer as well.
The next section contains example on how to include netLib with your program (replace yournetapp.c with testserver.c or testclient.c).


5) Compile Example
-------------------
This was verified to work with different versions of the GCC compiler.
Tested OS's were Windows (2000, XP and Vista), Mac OS X (10.3 and 10.4), BeOS 5, and Linux (Fedora 7, Ubuntu 7.04, and Debian 4.0).

Windows:	gcc yournetapp.c netSocket.c netBuffer.c -o yournetapp -DMS_WINDOWS -L../lib/win32 -lwsock32
Unix/Mac:	gcc yournetapp.c netSocket.c netBuffer.c -o yournetapp -DUNIX
BeOS:		gcc yournetapp.c netSocket.c netBuffer.c -o yournetapp -DBEOS
BeOS BONE:	gcc yournetapp.c netSocket.c netBuffer.c -o yournetapp -DBEOS -lsocket

BeOS note: netLib has not been testing with Haiku.

To compile the buffer client on Windows you would use the command line:
gcc bufferclient.c netSocket.c netBuffer.c -o bufferclient -DMS_WINDOWS -L../lib/win32 -lwsock32

To compile the chat server on Unix you would use the command line:
gcc chatserver.c netSocket.c netBuffer.c -o chatserver -DUNIX

To compile the echo server on BeOS you would use the command line:
gcc echoserver.c netSocket.c -o echoserver -DUNIX


6) References
--------------
The following guides, articles, and examples were used while writting netLib.

Beej's Guide to Network Programming
http://beej.us/guide/bgnet/

BeOS (Be Operating System)
http://en.wikipedia.org/wiki/BeOS_R5

BSD Sockets: A Quick And Dirty Primer
http://www.mathematik.uni-ulm.de/help/sockets.html

Winsock Programmer's FAQ: Articles: BSD Sockets Compatibility
http://tangentsoft.net/wskfaq/articles/bsd-compatibility.html

Winsock Programmer's FAQ
http://tangentsoft.net/wskfaq/

Microsoft WinSock Reference
http://msdn2.microsoft.com/en-us/library/ms741416.aspx

socket(7) - Linux man page
http://www.die.net/doc/linux/man/man7/socket.7.html

7) Final Notes
---------------
If you have any questions, comments, concerns, bug reports, patches, suggestions or feature requests please send me an email.
