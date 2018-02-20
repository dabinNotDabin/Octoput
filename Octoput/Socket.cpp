#include "global.h"
#include "Socket.h"

using namespace std;

Socket::Socket()
{
	FD == -1;
	addressInitialized = false;
}



Socket::~Socket()
{
}


int Socket::getFD()
{
	return FD;
}

int Socket::getFamily()
{
	return fam;
}

bool Socket::initSocket(short family, short type, short protocol)
{
	int fd = socket(family, type, protocol);

	if (fd < 0)
	{
		cout << "Socket creation failed.. Socket id is -1.\n";
		return false;
	}

//	cout << "Socket initialized\n";

	FD = fd;
	fam = family;

	return true;
}



// Set the address of the socket to a specific port with general settings for other parameters.
// The family used will be the family that the socket is initialized with.
void Socket::associateAddress(int port)
{
	memset(&socketAddress, 0, sizeof(socketAddress));
	socketAddress.sin_family = fam;
	socketAddress.sin_port = htons(port);
	socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	addressInitialized = true;
}

// Set the address of the socket to match that of the address parameter.
// This can be problematic if the family in the argument doesn't match the family the
//		socket was initialized with.
void Socket::associateAddress(struct sockaddr_in address)
{
	memset(&socketAddress, 0, sizeof(socketAddress));
	socketAddress.sin_family = address.sin_family;
	socketAddress.sin_port = address.sin_port;
	socketAddress.sin_addr = address.sin_addr;

	addressInitialized = true;
}


void Socket::getAddress(struct sockaddr_in &address)
{
	if (addressInitialized)
	{
		memset(&address, 0, sizeof(address));
		address.sin_family = socketAddress.sin_family;
		address.sin_port = socketAddress.sin_port;
		address.sin_addr = socketAddress.sin_addr;
	}
}



bool Socket::addressIsInitialized()
{
	return addressInitialized;
}