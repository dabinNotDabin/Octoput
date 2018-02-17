#include "global.h"
#include "UDPServer.h"



using namespace std;

#include <sstream>


UDPServer::UDPServer()
{
}


UDPServer::~UDPServer()
{
	if (UDPSocket != NULL)
	{
		close(UDPSocket->getFD());
		delete UDPSocket;
	}

	if (octoblocks != NULL)
		delete[] octoblocks;
}


int UDPServer::getSocketFD()
{
	return UDPSocket != NULL ? UDPSocket->getFD() : -1;
}


struct sockaddr_in UDPServer::getClientAddress()
{
	return clientAddress;
}



struct sockaddr_in UDPServer::getAddress()
{
	struct sockaddr_in addr;

	UDPSocket->getAddress(addr);

	return addr;
}



// Note that this is configured to use the local host IP and the given family for the client's address.
UDPServer::UDPServer(short family, short type, short protocol, unsigned int port, unsigned int clientPort)
{
	UDPSocket = new Socket();
	sockaddr_in address;

	if (!UDPSocket->initSocket(family, type, protocol))
	{
		cout << "Failure initializing socket with <family>, <type>, and <protocol>.\n";
		UDPServer();
	}
	else
	{
		memset(&address, 0, sizeof(address));
		address.sin_family = family;
		address.sin_port = htons(port);
		inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);

		UDPSocket->associateAddress(address);
//		UDPSocket->associateAddress(port);
		bindSocket();

		clientAddress.sin_family = family;
		clientAddress.sin_port = htons(clientPort);
		inet_pton(AF_INET, "127.0.0.1", &clientAddress.sin_addr);
		cout << "Client Port Set In Server Constructor Netw Order: " << clientAddress.sin_port << endl;
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



void UDPServer::commenceOctovation()
{	
	string filename = getFileRequest();
	string octoDescripto;
	string confirmation = "OK";
	sockaddr_in serverAddress;

	cout << "Filename good.\n";

	if (filename.compare("\0") == 0)
	{
		cout << "Receive filename unsuccessful..exiting.\n";
		exit(0);
	}
	else
	{
		cout << "Sending confirmation.\n";

		cout << "ClientPort Before Send: " << clientAddress.sin_port << endl;
		sendto
		(
			UDPSocket->getFD(),
			confirmation.c_str(),
			confirmation.length(),
			0,
			clientAddressPtr,
			clientAddressLen
		);
		cout << "ClientPort After Send: " << clientAddress.sin_port << endl;
	}

	in.open(filename);
	getline(in, fileContents, '\0');
	in.close();


	instantiateOctoMonocto();
	instantiateOctoblocks();

	UDPSocket->getAddress(serverAddress);

	
	octoDescripto =
		"Total Size Of File: " + to_string(fileContents.length()) + "\r\n" +
		"Number Of Full Octoblocks: " + to_string(octoMonocto.numFullOctoblocks) + "\r\n" +
		"Size Of Partial Octoblock: " + to_string(octoMonocto.partialOctoblockSize) + "\r\n" +
		"Size Of Partial Octolegs: " + to_string(octoMonocto.partialOctolegSize) + "\r\n" +
		"Size Of Leftover Data: " + to_string(octoMonocto.leftoverDataSize) + "\r\n";

	cout << "Server Port: " << ntohs(serverAddress.sin_port) << endl;

	// Not right because string versions of these quantities will be 1 byte per digit.
	// not sure on the use of ntohs

	unsigned short serverPort;
	unsigned short clientPort;
	unsigned short packetLen;
	unsigned short checksum;
	unsigned char firstHalf;
	unsigned char secondHalf;
	unsigned char* header = new unsigned char[9];
	unsigned char sendMssg[1200];

	// Server Port
	serverPort = serverAddress.sin_port;
	cout << "ServerPort: " << serverPort << endl;
	cout << "Server IP: " << inet_ntoa(serverAddress.sin_addr) << endl;
	firstHalf = serverPort >> 8;
	secondHalf = serverPort & 0xFF;


	header[0] = firstHalf;
	header[1] = secondHalf;


	// Client Port
	clientPort = clientAddress.sin_port;
	cout << "ClientPort: " << clientAddress.sin_port << endl;
	firstHalf = clientPort >> 8;
	secondHalf = clientPort & 0xFF;

	header[2] = firstHalf;
	header[3] = secondHalf;


	// Packet Len
	packetLen = octoDescripto.length() + 8;
	cout << "Packet Len: " << packetLen << endl;
	firstHalf = packetLen >> 8;
	secondHalf = packetLen & 0xFF;

	cout << "Packet Len First Half: " << (int)firstHalf << endl;
	cout << "Packet Len Secnd Half: " << (int)secondHalf << endl;

	header[4] = firstHalf;
	header[5] = secondHalf;

	cout << "Client IP: " << inet_ntoa(clientAddress.sin_addr) << endl;

	// Checksum
	checksum = computeChecksum
	(
		octoDescripto.c_str(),
		inet_ntoa(clientAddress.sin_addr),
		(unsigned int)(clientAddress.sin_port)
	);
	
	cout << "Checksum: " << checksum << endl;

	firstHalf = checksum >> 8;
	secondHalf = checksum & 0xFF;

	header[6] = firstHalf;
	header[7] = secondHalf;

	header[8] = '\0';


	for (int i = 0; i < 9; i++)
	{
		cout << "Index: " << i << " = " << (unsigned int)header[i] << endl;
	}


	string headerStr((const char*)header);

	memcpy(sendMssg, header, 8);
	memcpy(sendMssg + 8, octoDescripto.c_str(), octoDescripto.length());

	cout << "Octo Descripto being sent as:\n";
	for (int i = 0; i < packetLen; i++)
	{
		cout << "Index: " << i << " = " << (unsigned int)sendMssg[i] << endl;
	}


	sendto
	(
		UDPSocket->getFD(),
		sendMssg,
		packetLen,
		0,
		clientAddressPtr,
		clientAddressLen
	);

	
	return;
}



std::string UDPServer::getFileRequest()
{
	int nBytesRcvd;
	string filenameStr;
	char filename[276];
	cout << "Received.\n";

	cout << "ClientPort Before Rcv: " << clientAddress.sin_port << endl;
	nBytesRcvd = recvfrom
	(
		UDPSocket->getFD(),
		filename,
		276,
		0,
		clientAddressPtr,
		&clientAddressLen
	);
	cout << "ClientPort After Rcv: " << clientAddress.sin_port << endl;


	// Probably wanna check that nBytesReceived correlates with value in header
	if (nBytesRcvd == -1)
	{
		cout << "Failure.\n";
		return '\0';
	}

	filename[nBytesRcvd] = '\0';
	cout << "Filename Received: ";
	for (int i = 0; i < nBytesRcvd; i++)
		cout << filename[i];
	cout << " of length: " << nBytesRcvd << endl;


	filenameStr = string(filename);
	in.open(filenameStr);

	if (!in.good())
		cout << "File not good.\n";

	while ((filenameStr.compare("quit") != 0) && !in.good())
	{
		in.close();

		nBytesRcvd = recvfrom
		(
			UDPSocket->getFD(),
			filename,
			276,
			0,
			clientAddressPtr,
			&clientAddressLen
		);

		// Probably wanna check that nBytesReceived correlates with value in header
		if (nBytesRcvd == -1)
			return '\0';

		filename[nBytesRcvd] = '\0';
		cout << "Filename Received: ";
		for (int i = 0; i < nBytesRcvd; i++)
			cout << filename[i];
		cout << endl;

		in.open(filename);
	}

	in.close();

	filenameStr = string(filename);
	if (filenameStr.compare("quit") == 0)
		return '\0';
	else
		return filenameStr;
}



void UDPServer::instantiateOctoMonocto()
{
	numFullOctoblocksNeeded = fileContents.length() / octoblockSize;

	// If partialOctoblockSize is not 0, have to send partial octoblock.
	partialOctoblockSize = fileContents.length() % 8888;

	if (partialOctoblockSize != 0)
	{
		leftoverDataSize = partialOctoblockSize % 8;
		partialOctolegSize = (partialOctoblockSize - leftoverDataSize) / 8;
	}
	else
	{
		partialOctoblockSize = -1;
		partialOctolegSize = -1;
		leftoverDataSize = -1;
	}


	totalOctoblocksNeeded =
		numFullOctoblocksNeeded +
		((partialOctoblockSize > 0) ? 1 : 0) +
		((leftoverDataSize > 0) ? 1 : 0);

	octoMonocto =
	{
		(short)fileContents.length(),
		(short)numFullOctoblocksNeeded,
		(short)partialOctoblockSize,
		(short)partialOctolegSize,
		(short)leftoverDataSize
	};

}



void UDPServer::instantiateOctoblocks()
{
	octoblocks = new std::string[totalOctoblocksNeeded];

	// Divide file contents into octoblocks and store in array, padding leftover data if necessary.
	for (int i = 0; i < totalOctoblocksNeeded; i++)
	{
		if (i < numFullOctoblocksNeeded)
		{
			octoblocks[i] = fileContents.substr(i * octoblockSize, octoblockSize);
		}
		else if (i == totalOctoblocksNeeded - 2)
		{
			octoblocks[i] = fileContents.substr(numFullOctoblocksNeeded * octoblockSize, partialOctoblockSize);
		}
		else // (i == totalOctoblocksNeeded - 1)
		{
			octoblocks[i] = fileContents.substr(numFullOctoblocksNeeded * octoblockSize + partialOctoblockSize);
		}
	}
}




std::string UDPServer::constructHeader(char octolegFlag, short packetSize, const char* data)
{
	string headerStr;
	int checksum;

	headerStr = octolegFlag + to_string(packetSize) + string(data);
//	checksum = computeChecksum(headerStr.c_str());

	return headerStr + to_string(checksum);
}







unsigned short UDPServer::computeChecksum(const char* data, const char* clientIP, unsigned int clientPort)
{
	sockaddr_in serverAddress;
	string pseudoHeaderIPs;
	string udpPacketData;
	string worker;
	unsigned short i;
	unsigned short j;
	unsigned short checksum;
	unsigned int sum;
	char* byte;

	cout << "Client IP in Server Compute Checksum: " << string(clientIP) << endl;

	UDPSocket->getAddress(serverAddress);

	pseudoHeaderIPs = inet_ntoa(serverAddress.sin_addr) + string(".") + string(clientIP);
	cout << "Header IPs: " << pseudoHeaderIPs << endl;


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

	//	cout << "UDP Packet: " << udpPacketData << endl;

	//	cout << "UDP Packet Len: " << udpPacketData.length() << endl;

	char c;
	for (int x = 0; x < udpPacketData.length(); x += 2)
	{
		c = data[x];
		i = (unsigned short)c;
		//		cout << "I: " << i << endl;

		c = data[x + 1];
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

	return checksum;
}
