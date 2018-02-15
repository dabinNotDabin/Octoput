#include "global.h"
#include "UDPServer.h"

using namespace std;




UDPServer::UDPServer()
{
}


UDPServer::~UDPServer()
{
//	close(UDPSocket->getFD());

//	if (UDPSocket != NULL)
//		delete UDPSocket;
}






// Call with.. 
//memset(&address, 0, sizeof(socketAddress));
//address.sin_family = AF_INET;
//address.sin_port = htons(port);
//address.sin_addr.s_addr = htonl(INADDR_ANY);
// For server that listens for any ip on port <port> using AF_INET
bool UDPServer::bindSocket(int socketFD, struct sockaddr_in address)
{
	if (socketFD < 0)
	{
		cout << "Socket FD <socketFD> is not valid, cannot bind.\n";
		return false;
	}

	int bindResult = bind(socketFD, (struct sockaddr *)&address, sizeof(address));
	if (bindResult < 0)
	{
		cout << "Socket binding failed..  Either the address is invalid or the socket is not free.\n";
		return false;
	}
	else
	{
		cout << "Success binding socket.\n";
	}


	return true;
}




bool UDPServer::bindSocket()
{
	if (UDPSocket != NULL)
	{
		cout << "Using default socketFD and address to bind.\n";

		int bindResult;
		sockaddr_in address;

		if (!UDPSocket->addressIsInitialized())
		{
			cout << "No address has been associated with the default socket, cannot bind.\n";
			return false;
		}

		UDPSocket->getAddress(address);
		bindResult = bind(UDPSocket->getFD(), (struct sockaddr *)&address, sizeof(address));

		if (bindResult < 0)
		{
			cout << "Socket binding failed..  Either the address is invalid or the socket is not free.\n";
			return false;
		}
		else
		{
			cout << "Success binding socket.\n";
		}
	}
	else
	{
		cout << "Default socket is null, instantiate it first.\n";
		return false;
	}

	return true;
}


string UDPServer::constructHeader(char octolegFlag, short packetSize, const char* data)
{
	string headerStr;
	int checksum;

	headerStr = octolegFlag + to_string(packetSize) + string(data);
	checksum = computeChecksum(headerStr.c_str());

	return headerStr + to_string(checksum);
}


int UDPServer::computeChecksum(const char* data)
{

}


short UDPServer::oneComplementSum(short i, short j)
{
	short a = i & j;
	short x;
	short l;
	short wrap;
	unsigned short k = 65536;

	while (a != 0)
	{
		x = i ^ j;

		wrap = a & k;

		l = a << 1 + (k == 0 ? 0 : 1);

		i = x;
		j = l;
	}

	return i ^ j;
}