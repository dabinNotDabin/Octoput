#pragma once



class Socket
{
public:
	Socket();
	~Socket();
	
	// <type> should be SOCK_DGRAM when using UDP or SOCK_STREAM for TCP
	// <protocol> should be IPPROTO_UDP when using UDP or 0 for TCP
	bool init(short type, short protocol);
	bool init(short sin_family, short type, short protocol);

	// Needs to be called before the socket is bound with a file descriptor (bindAddressWithSocket())	
	void setAddress(struct sockaddr_in address);
	void setAddress(int port);

	bool bindAddressWithSocket();
	int getID();

private:
	// ID assigned on initialization
	int ID;
	bool addressInitialized;

	// Socket address as maintained by OS
	struct sockaddr_in socketAddress;
};


