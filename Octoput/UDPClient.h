#pragma once
#include <vector>
#include <map>

#include "Socket.h"
#include "UDPServer.h"

using namespace std;


// In this implementation, a client will connect to one host.
// The sloxy has the responsibility of managing the various representative clients.
class UDPClient
{
public:
	UDPClient();
	~UDPClient();

	UDPClient(short family, short type, short protocol, unsigned int port, unsigned int serverPort);

	int getSocketFD();
	bool bindSocket();
	struct sockaddr_in getServerAddress();
	struct sockaddr_in getAddress();


	void commenceOctovation();

private:
	Socket* UDPSocket;
	struct sockaddr_in serverAddress;
	struct sockaddr* serverAddressPtr = (struct sockaddr*)&serverAddress;
	int serverAddressLen = sizeof(*serverAddressPtr);

	OctoMonocto octoMonocto;


	string askUserForFilename();
	bool validateMessage(const char* data, int dataLen);
	bool validateChecksum(const char* data);
	bool parseOctoDescripto(const unsigned char* octoDescripto);


	unsigned short computeChecksum(const unsigned char* data, const char* serverIP, unsigned int serverPort);
};