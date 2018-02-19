#pragma once
#include "TaskQueue.h"
#include "Socket.h"
#include "time.h"

class UDPServer
{
public:
	UDPServer();
	~UDPServer();
	
	UDPServer(short family, short type, short protocol, unsigned int port, unsigned int clientPort);

	// Associates <address> with <socketFD> and binds it.
	bool bindSocket(int socketFD, struct sockaddr_in address);

	//	Binds the default UDPSocket using the address associated with it.
	//	The UDPSocket must be instantiated first so it is assigned a FD (see "Socket.h").
	//	It uses the address associated with the UDPSocket.
	bool bindSocket();

	int getSocketFD();
	struct sockaddr_in getClientAddress();
	struct sockaddr_in getAddress();

	// Initiate file transfer.
	void commenceOctovation();


private:
	Socket *UDPSocket;
	struct sockaddr_in clientAddress;
	struct sockaddr* clientAddressPtr = (struct sockaddr*)&clientAddress;
	int clientAddressLen = sizeof(*clientAddressPtr);

	OctoMonocto octoMonocto;
	pthread_mutex_t generalMutex;


	std::ifstream in;
	std::string filename;
	std::string fileContents;

	int octoblockSize = 8888;
	int fullOctolegSize = 1111;


	unsigned char* octoblockData;
	bool ackRcvd[8];

	TaskQueue* taskQ;
	pthread_mutex_t socketMutex;
	static void* serverThread(void* id);

	unsigned short numOctoblocksTransferred;
	unsigned short numOctolegsTransferred;



	std::string getFileRequest();
	void instantiateOctoMonocto();



	void attachHeader(char octolegFlag, unsigned short payloadSize, unsigned char* data);
	unsigned short computeChecksum(const unsigned char* data, const char* clientIP, unsigned int clientPort);

	void sendMssgRequireAck(const unsigned char* mssg, int mssgLen, int timeoutMsec, char octolegID);
};


