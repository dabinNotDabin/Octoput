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
	struct sockaddr_in getServerAddress();


	void commenceOctovation(UDPServer& s);

private:
	Socket* UDPSocket;
	struct sockaddr_in serverAddress;
	struct sockaddr* serverAddressPtr = (struct sockaddr*)&serverAddress;
	int serverAddressLen = sizeof(*serverAddressPtr);


	string askUserForFilename(UDPServer& s);
	bool validateMessage(const char* data, int dataLen);
	bool validateChecksum(const char* data);

	UDPServer server;
};