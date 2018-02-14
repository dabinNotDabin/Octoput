#include "Sloxy.h"
#include "Client.h"



UDPClient::UDPClient()
{

}

UDPClient::~UDPClient()
{

}


bool UDPClient::connectWithHost(struct sockaddr_in hostAddress)
{
	// Need an init that takes more info to set the socket up according to the host's protocol
	if (clientSocket.init(hostAddress.sin_family))
	{
		if (!clientSocket.connectToHost(hostAddress))
		{
			cout << "Connect to host failed.\n";
			return false;
		}
	}
	else
	{
		cout << "Instantiation of socket failed.\n";
	}

	return true;
}


int UDPClient::getWebHostSocketID()
{
	return clientSocket.getID();
}