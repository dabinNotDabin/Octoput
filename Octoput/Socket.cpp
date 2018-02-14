#include "Sloxy.h"
#include "Socket.h"

using namespace std;

Socket::Socket()
{
	ID == -1;
	addressInitialized = false;
}

Socket::~Socket()
{
}


int Socket::getID()
{
	return ID;
}

bool Socket::init(short type, short protocol)
{
	ID = socket(AF_INET, type, protocol);

	if (ID < 0)
	{
		cout << "Socket creation failed.. Socket id is -1.\n";
		return false;
	}

	cout << "Socket initialized\n";

	return true;
}

bool Socket::init(short sin_family, short type, short protocol)
{
	ID = socket(sin_family, type, protocol);

	if (ID < 0)
	{
		cout << "Socket creation failed.. Socket id is -1.\n";
		return false;
	}

	cout << "Socket initialized\n";

	return true;
}


// Set the address of the socket to a specific port with general settings for other parameters.
void Socket::setAddress(int port)
{
	memset(&socketAddress, 0, sizeof(socketAddress));
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(port);
	socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	addressInitialized = true;
}

// Set the address of the socket to match that of the address parameter.
void Socket::setAddress(struct sockaddr_in address)
{
	memset(&socketAddress, 0, sizeof(socketAddress));
	socketAddress.sin_family = address.sin_family;
	socketAddress.sin_port = address.sin_port;
	socketAddress.sin_addr = address.sin_addr;

	addressInitialized = true;
}



// Binds the address associated with this Socket to the physical socket for identification purposes.
// Clients can attempt to connect to this socket using the address that is bound to it.
// Sockets don't necessarily need an address bound to them, for instance, client sockets.
// It uses the address that can be set by a call to "setAddress"
bool Socket::bindAddressWithSocket()
{
	if (ID < 0 || !addressInitialized)
	{
		if (ID < 0)
			cout << "Socket ID not valid.\n";
		else
			cout << "Socket address must be initialzed with a call to \"setAddress\" before binding.\n";

		return false;
	}


	// Bind the socket to the port
	int bindResult = bind(ID, (struct sockaddr *)&socketAddress, sizeof(socketAddress));
	if (bindResult  < 0)
	{
		cout << "Socket binding failed.\n";
		return false;
	}
	
	cout << "Address bound to socket.\n";

	return true;
}



