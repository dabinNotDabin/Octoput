#include "Sloxy.h"
#include "Server.h"

using namespace std;




UDPServer::UDPServer()
{


}


UDPServer::~UDPServer()
{
}




int UDPServer::initSocket(short family, short type, short protocol)
{
	int FD = socket(family, type, protocol);

	if (FD < 0)
	{
		cout << "Socket creation failed.. Socket id is -1.\n";
		return -1;
	}

	cout << "Socket initialized\n";

	return FD;
}


// Call with.. 
//memset(&address, 0, sizeof(socketAddress));
//address.sin_family = AF_INET;
//address.sin_port = htons(port);
//address.sin_addr.s_addr = htonl(INADDR_ANY);
// For server that listens for any ip on port <port> using AF_INET
bool UDPServer::bindAddressWithSocket(int socketFD, struct sockaddr_in address)
{
	if (socketFD < 0)
	{
		cout << "Socket ID not valid, using private member variable as default.\n";

		if (UDPSocket != NULL)
		{
			cout << "Using default socketFD.\n";
			socketFD = UDPSocket->getID();
		}
		else
		{
			cout << "Default socket is null.\n";
			return false;
		}
	}

	int bindResult = bind(socketFD, (struct sockaddr *)&address, sizeof(address));
	if (bindResult < 0)
	{
		cout << "Socket binding failed..  Either the address is invalid or the socket is not free.\n\tAttempting to use the socketFD and address associated with default the Socket";
		if (UDPSocket != NULL)
		{
			socketFD == UDPSocket->getID();
			address = UDPSocket->getAddress();
			bindResult = bind(socketFD, (struct sockaddr *)&address, sizeof(address));

			if (bindResult < 0)
			{
				cout << "Failure binding default socket. Address may not be set properly.\n";
			}
			else
			{
				cout << "Success binding default socket.\n";
			}
		}
		else
		{
			cout << "Default Socket hasn't been initialized.\n";
			return false;
		}
	}
	cout << "Address bound to socket.\n";

	return true;
}
