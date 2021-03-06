#pragma once

// A socket has a file descriptor and can have an address associated with it.
// This class setters that allow you to initialize a socket and associate an address.
// It 

// Typical usage:
//	Instantiate a socket object.
//	Use initSocket(3) to initialize it.
//	Associate an address with it if necessary for binding and use getAddress(1) to bind.
//	Use getFD(0) to send/receive to/from socket.

class Socket
{
public:
	Socket();
	~Socket();
	
	// Uses the family, port and address from the argument.
	void associateAddress(struct sockaddr_in address);

	// Uses AF_INET for family and INADDR_ANY for the address
	void associateAddress(int port);

	// Overwrites the argument to contain the address associated with this socket.
	// If no address associated, it does nothing to the argument.
	void getAddress(struct sockaddr_in &address);

	// Returns the file descriptor assigned to this socket and -1 if not assigned.
	int getFD();

	// Initialize the socket, resulting in the OS assigning it a file descriptor.
	bool initSocket(short family, short type, short protocol);

	bool addressIsInitialized();

	int getFamily();

private:
	// ID assigned on initialization
	int FD;
	bool addressInitialized;

	int fam;

	// Socket address as maintained by OS
	struct sockaddr_in socketAddress;
};


