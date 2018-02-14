#pragma once
#include "Socket.h"



// It may be more elegant to maintain a request queue in the server that the sloxy can 
// query and relay to the representative client but since it only needs to service
// one client, I will instead have the Sloxy use the server to receive messages
// and process them as necessary before relaying requests to the representative client.
class UDPServer
{
public:
	UDPServer();
	~UDPServer();


	int initSocket(short family, short type, short protocol);

	bool bindAddressWithSocket(int socketFD, struct sockaddr_in address);

private:
	Socket *UDPSocket;


};


