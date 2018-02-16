#include "global.h"
#include "UDPServer.h"

using namespace std;

#include <sstream>


UDPServer::UDPServer()
{
}


UDPServer::~UDPServer()
{
	close(UDPSocket->getFD());

	if (UDPSocket != NULL)
		delete UDPSocket;
}


// Note that this is configured to use the local host IP and the given family for the client's address.
UDPServer::UDPServer(short family, short type, short protocol, unsigned int port, unsigned int clientPort)
{
	UDPSocket = new Socket();

	if (!UDPSocket->initSocket(family, type, protocol))
	{
		cout << "Failure initializing socket with <family>, <type>, and <protocol>.\n";
		UDPServer();
	}
	else
	{
		UDPSocket->associateAddress(port);
		bindSocket();

		clientAddress.sin_family = family;
		clientAddress.sin_port = htons(clientPort);
		inet_pton(AF_INET, "127.0.0.1", &clientAddress.sin_addr);
	}
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


std::string UDPServer::constructHeader(char octolegFlag, short packetSize, const char* data)
{
	string headerStr;
	int checksum;

	headerStr = octolegFlag + to_string(packetSize) + string(data);
//	checksum = computeChecksum(headerStr.c_str());

	return headerStr + to_string(checksum);
}


unsigned short UDPServer::computeChecksum(const char* data, const char* destinationIP, unsigned int clientPort)
{
	struct sockaddr_in serverAddress;
	char* sourceIP;	
	string pseudoHeaderIPs;
	string udpPacketData;
	string worker;
	unsigned short i;
	unsigned short j;
	unsigned short checksum;
	unsigned int sum;
	char* byte;

//	UDPSocket->getAddress(serverAddress);

	inet_pton(AF_INET, "192.168.0.31", &serverAddress.sin_addr);
	sourceIP = inet_ntoa(serverAddress.sin_addr);
	
	pseudoHeaderIPs = string(sourceIP + string(".") + string(destinationIP));
	cout << "SourceIP: " << pseudoHeaderIPs << endl;


	// Begin Source IP
	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
	i = atoi(worker.c_str());
//	cout << i << endl;
	pseudoHeaderIPs = pseudoHeaderIPs.substr(worker.length() + 1);
//	cout << "Pseudo: " << pseudoHeaderIPs << endl;
	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
//	cout << atoi(worker.c_str()) << endl;
	i = (i << 8) | atoi(worker.c_str());
	cout << "I: " << i << endl;
	pseudoHeaderIPs = pseudoHeaderIPs.substr(worker.length() + 1);
//	cout << "Pseudo: " << pseudoHeaderIPs << endl;


	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
	j = atoi(worker.c_str());
//	cout << j << endl;
	pseudoHeaderIPs = pseudoHeaderIPs.substr(worker.length() + 1);
//	cout << "Pseudo: " << pseudoHeaderIPs << endl;
	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
//	cout << atoi(worker.c_str()) << endl;
	j = (j << 8) | atoi(worker.c_str());
	cout << "J: " << j << endl;
	pseudoHeaderIPs = pseudoHeaderIPs.substr(worker.length() + 1);
//	cout << "Pseudo: " << pseudoHeaderIPs << endl;


	sum = i + j;
	cout << "SUM: " << sum << endl;


	// Begin Dest IP
	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
	i = atoi(worker.c_str());
//	cout << i << endl;
	pseudoHeaderIPs = pseudoHeaderIPs.substr(worker.length() + 1);
//	cout << "Pseudo: " << pseudoHeaderIPs << endl;
	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
//	cout << atoi(worker.c_str()) << endl;
	i = (i << 8) | atoi(worker.c_str());
	cout << "I: " << i << endl;
	pseudoHeaderIPs = pseudoHeaderIPs.substr(worker.length() + 1);
//	cout << "Pseudo: " << pseudoHeaderIPs << endl;

	sum = sum + i;

	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
	i = atoi(worker.c_str());
//	cout << i << endl;
	pseudoHeaderIPs = pseudoHeaderIPs.substr(worker.length() + 1);
//	cout << "Pseudo: " << pseudoHeaderIPs << endl;
	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
//	cout << atoi(worker.c_str()) << endl;
	i = (i << 8) | atoi(worker.c_str());
	cout << "I: " << i << endl;
//	cout << "Pseudo: " << pseudoHeaderIPs << endl;

	sum = sum + i;
	// End Dest IP

	cout << "SUM: " << sum << endl;


	// Begin Protocol
	i = 17;
	sum = sum + i;
	cout << "SUM: " << sum << endl;
	// End Protocol


	// Begin UDP Length
	udpPacketData = string(data);
	i = 8 + udpPacketData.length();
	sum = sum + i;
	cout << "SUM: " << sum << endl;
	// End UDP Length


	// Begin Src Port
	i = serverAddress.sin_port;
	i = 20;
	sum = sum + i;
	cout << "SUM: " << sum << endl;
	// End Src Port

	// Begin Dst Port
	i = clientPort;
	sum = sum + i;
	cout << "SUM: " << sum << endl;
	// End Dst Port

	// Begin UDP Length
	udpPacketData = string(data);
	i = 8 + udpPacketData.length();
	sum = sum + i;
	cout << "SUM: " << sum << endl;
	// End UDP Length

	cout << "UDP Packet: " << udpPacketData << endl;

	cout << "UDP Packet Len: " << udpPacketData.length() << endl;

	char c;
	for (int x = 0; x < udpPacketData.length(); x += 2)
	{
		c = data[x];
		i = (unsigned short)c;
//		cout << "I: " << i << endl;

		c = data[x+1];
//		cout << "I: " << (unsigned short)c << endl;
		i = (i << 8) | (unsigned short)c;
//		cout << "I: " << i << endl;

		sum = sum + i;
//		cout << "SUM: " << sum << endl;
	}

	
	checksum = oneComplementSum(sum);

	if (checksum != 65535)
		checksum = ~checksum;

	cout << "Checksum: " << checksum << endl;
}


unsigned short UDPServer::oneComplementSum(unsigned int k)
{
	unsigned int l = 65535;

	while ((k >> 16) > 0)
	{
		k = (k & l) + (k >> 16);
	}

	return k;
}