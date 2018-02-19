#pragma once
#include "Socket.h"
#include <time.h>


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

	int getSocketFD();
	struct sockaddr_in getClientAddress();
	struct sockaddr_in getAddress();

	// Initiate file transfer.
	void commenceOctovation();


private:
	Socket *UDPSocket;
	struct sockaddr_in clientAddress;
	struct sockaddr* clientAddressPtr = (struct sockaddr*)&clientAddress;
	int clientAddressLen = sizeof(*clientAddressPtr);

	OctoMonocto octoMonocto;


	std::ifstream in;
	std::string filename;
	std::string fileContents;

	int octoblockSize = 8888;
	int totalOctoblocksNeeded;
	int numFullOctoblocksNeeded;
	int partialOctoblockSize;
	int partialOctolegSize;
	int leftoverDataSize;

	std::string *octoblocks;




	std::string getFileRequest();
	void instantiateOctoMonocto();
	void instantiateOctoblocks();



	void attachHeader(unsigned char octolegFlag, unsigned short payloadSize, unsigned char* data);

	unsigned short computeChecksum(const unsigned char* data, const char* clientIP, unsigned int clientPort);
};


