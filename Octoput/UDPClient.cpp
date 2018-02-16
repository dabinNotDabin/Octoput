#include "global.h"
#include "UDPClient.h"



UDPClient::UDPClient()
{

}

UDPClient::~UDPClient()
{
	close(UDPSocket->getFD());

	if (UDPSocket != NULL)
		delete UDPSocket;
}



// Note that this is configured to use the local host IP and the given family for the server's address.
UDPClient::UDPClient(short family, short type, short protocol, unsigned int port, unsigned int serverPort)
{
	UDPSocket = new Socket();

	if (!UDPSocket->initSocket(family, type, protocol))
	{
		cout << "Failure initializing socket with <family>, <type>, and <protocol>.\n";
		UDPClient();
	}
	else
	{
		UDPSocket->associateAddress(port);

		serverAddress.sin_family = family;
		serverAddress.sin_port = htons(serverPort);
		inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);
	}
	
}



