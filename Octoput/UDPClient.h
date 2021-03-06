#pragma once
#include "TaskQueue.h"
#include "Socket.h"

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
	bool bindSocket();
	struct sockaddr_in getServerAddress();
	struct sockaddr_in getAddress();


	void commenceOctovation();

private:
	Socket* UDPSocket;
	struct sockaddr_in serverAddress;
	struct sockaddr* serverAddressPtr = (struct sockaddr*)&serverAddress;
	int serverAddressLen = sizeof(*serverAddressPtr);

	OctoMonocto octoMonocto;

	unsigned char* incomingOctodata;
	bool octolegRcvd[N_OCTOLEGS_PER_OCTOBLOCK];

	static void* clientThread(void* id);
	int octoblockSize = 8888;
	int fullOctolegSize = 1111;


//	TaskQueue* taskQ;
	pthread_mutex_t socketMutex;
	pthread_mutex_t generalMutex;
	pthread_mutex_t octoblockMutex;
	pthread_cond_t octoblocked;
	unsigned short numOctoblocksReceived;
	unsigned short numOctolegsReceived;

	std::ofstream out;
	string filenameOut;

	string askUserForFilename();
	bool parseOctoDescripto(const unsigned char* octoDescripto);

	bool sendAck(char id);
	void attachHeader(char octolegFlag, unsigned short payloadSize, unsigned char* data);
	unsigned short computeChecksum(const unsigned char* data, const char* serverIP, unsigned int serverPort);
	unsigned char receiveMssg(unsigned char *buffer, unsigned short mssgLen);
};