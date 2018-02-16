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
	
	UDPServer(short family, short type, short protocol, unsigned int port, unsigned int clientPort);

	// Associates <address> with <socketFD> and binds it.
	bool bindSocket(int socketFD, struct sockaddr_in address);

	//	Binds the default UDPSocket using the address associated with it.
	//	The UDPSocket must be instantiated first so it is assigned a FD (see "Socket.h").
	//	It uses the address associated with the UDPSocket.
	bool bindSocket();


	unsigned short computeChecksum(const char* data, const char* destinationIP, unsigned int clientPort);

private:
	Socket *UDPSocket;
	sockaddr_in clientAddress;


	std::string constructHeader(char octolegFlag, short packetSize, const char* data);
	unsigned short oneComplementSum(unsigned int k);

};


