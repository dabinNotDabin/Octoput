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
	struct sockaddr_in address;

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
	string headerStr;
	unsigned char sendMssg[1200];

//	struct sockaddr_in serverAddress;

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

//	UDPSocket->getAddress(serverAddress);

	
	octoDescripto =
		"Total Size Of File: " + to_string(fileContents.length()) + "\r\n" +
		"Number Of Full Octoblocks: " + to_string(octoMonocto.numFullOctoblocks) + "\r\n" +
		"Size Of Partial Octoblock: " + to_string(octoMonocto.partialOctoblockSize) + "\r\n" +
		"Size Of Partial Octolegs: " + to_string(octoMonocto.partialOctolegSize) + "\r\n" +
		"Size Of Leftover Data: " + to_string(octoMonocto.leftoverDataSize) + "\r\n";

//	cout << "Server Port: " << ntohs(serverAddress.sin_port) << endl;

	// Not right because string versions of these quantities will be 1 byte per digit.
	// not sure on the use of ntohs


//	headerStr = constructHeader('0', octoDescripto.length(), octoDescripto.c_str());

//	cout << "Header Str: " << headerStr << endl;

//	memcpy(sendMssg, headerStr.c_str(), HEADER_SIZE_BYTES);
	memcpy(sendMssg, octoDescripto.c_str(), octoDescripto.length());
	sendMssg[octoDescripto.length()] = '\0';
	attachHeader('\0', octoDescripto.length(), (char*)sendMssg);




//	cout << "Octo Descripto being sent as:\n";
//	for (int i = 0; i < packetLen; i++)
//	{
//		cout << "Index: " << i << " = " << (unsigned int)sendMssg[i] << endl;
//	}


	sendto
	(
		UDPSocket->getFD(),
		sendMssg,
		octoDescripto.length() + HEADER_SIZE_BYTES,
		0,
		clientAddressPtr,
		clientAddressLen
	);

/*
	pthread_mutex_t queueMutex;
	pthread_cond_t queueEmpty;
	pthread_mutex_init(&queueMutex, NULL);
	pthread_cond_init(&queueEmpty, NULL);
	pthread_mutex_destroy(&queueMutex);
	pthread_cond_destroy(&queueEmpty);


	pthread_mutex_lock(&queueMutex);
	numberQueue.push(n);
	pthread_cond_signal(&queueEmpty);
	pthread_mutex_unlock(&queueMutex);

	pthread_mutex_lock(&queueMutex);
	pthread_cond_wait(&queueEmpty, &queueMutex);
	pthread_mutex_unlock(&queueMutex);
*/

	//clock_t before;
	//double elapsed;

	//before = clock();

//	sleep(1);

	usleep(100000); // 100 000 usec = 100 msec


	int nBytesRcvd;
	unsigned short checksum;
	unsigned short rcvdChecksum;
	string ackStr;
	char ack[32];
	bool ackOK = false;

	nBytesRcvd = recvfrom
	(
		UDPSocket->getFD(),
		ack,
		32,
		0,
		clientAddressPtr,
		&clientAddressLen
	);
	
	while (!ackOK)
	{
		if (nBytesRcvd == -1)
		{
			cout << "No ACK received.\n";
		}
		else
		{
			rcvdChecksum = ((ack[6] << 8) | ack[7]);
			cout << "Received Checksum: " << rcvdChecksum << endl;
			
			ack[nBytesRcvd] = '\0';
			cout << "ACK Received: ";
			for (int i = 0; i < nBytesRcvd; i++)
				cout << ack[i];
			cout << " of length: " << nBytesRcvd << endl;

			checksum = computeChecksum((unsigned char*)ack, inet_ntoa(clientAddress.sin_addr), clientAddress.sin_port);
			cout << "Checksum Computed: " << checksum << endl;

			if (checksum == rcvdChecksum)
				ackOK = true;
			else
			{
				sendto
				(
					UDPSocket->getFD(),
					sendMssg,
					octoDescripto.length() + HEADER_SIZE_BYTES,
					0,
					clientAddressPtr,
					clientAddressLen
				);
			}
		}
	}
	// Start timer, when expired, try to receive ack.
	// While invalid or not received, resend octo descripto.
	
	//elapsed = clock() - before;

	//cout << "Time elapsed: " << elapsed / CLOCKS_PER_SEC << " seconds.\n";

	return;
}



std::string UDPServer::getFileRequest()
{
	int nBytesRcvd;
	string filenameStr;
	char filename[276];
	cout << "Received.\n";

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
		in.clear();
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



// data must be null terminated
void UDPServer::attachHeader(unsigned char octolegFlag, unsigned short payloadSize, char* data)
{
	struct sockaddr_in serverAddress;
	unsigned short serverPort;
	unsigned short clientPort;
	unsigned short packetLen;
	unsigned short checksum;
	unsigned char firstHalf;
	unsigned char secondHalf;
	unsigned char* header = new unsigned char[9];



	UDPSocket->getAddress(serverAddress);

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
	//	if (octolegFlag == '\0')
	packetLen = payloadSize + HEADER_SIZE_BYTES;
	//	else
	//		packetLen = (unsigned short)octolegFlag;

	cout << "Packet Len: " << packetLen << endl;
	firstHalf = packetLen >> 8;
	secondHalf = packetLen & 0xFF;

	cout << "Packet Len First Half: " << (int)firstHalf << endl;
	cout << "Packet Len Secnd Half: " << (int)secondHalf << endl;
	header[4] = firstHalf;
	header[5] = secondHalf;


	//	cout << "Client IP: " << inet_ntoa(clientAddress.sin_addr) << endl;


	string dataStr(data);

	header[6] = header[7] = 0;
	memcpy(data, header, HEADER_SIZE_BYTES);
	memcpy(data + HEADER_SIZE_BYTES, dataStr.c_str(), dataStr.length());


	// Checksum
	checksum = computeChecksum
	(
		(unsigned char*)data,
		inet_ntoa(clientAddress.sin_addr),
		(unsigned int)(clientAddress.sin_port)
	);

	cout << "Checksum in constructHeader(): " << checksum << endl;

	firstHalf = checksum >> 8;
	secondHalf = checksum & 0xFF;

	header[6] = firstHalf;
	header[7] = secondHalf;

	memcpy(data + 6, header + 6, 2);

	header[8] = '\0';


	for (int i = 0; i < 9; i++)
	{
		cout << "Index: " << i << " = " << (unsigned int)header[i] << endl;
	}


	delete[] header;
}





// data must be null terminated.

unsigned short UDPServer::computeChecksum(const unsigned char* data, const char* clientIP, unsigned int clientPort)
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
	cout << "SUM After Src IP: " << sum << endl;


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

	cout << "SUM After Dst IP: " << sum << endl;


	// Begin Protocol
	i = 17;
	sum = sum + i;
	cout << "SUM After Protocol: " << sum << endl;
	// End Protocol


	// Begin UDP Length (Pseudo header length field)
	//	udpPacketData = string(data + HEADER_SIZE_BYTES);
	//	cout << "Packet Len First Half: " << (unsigned int)data[4] << endl;
	//	cout << "Packet Len Secnd Half: " << (unsigned int)data[5] << endl;

	i = ((data[4] << 8) | data[5]);//HEADER_SIZE_BYTES + udpPacketData.length();
	sum = sum + i;
	cout << "SUM After Length: " << sum << endl;
	// End UDP Length


	// Begin Src Port
	i = ((data[0] << 8) | data[1]);
	i = 20;
	sum = sum + i;
	cout << "SUM After Src Port: " << sum << endl;
	// End Src Port

	// Begin Dst Port
	i = ((data[2] << 8) | data[3]);
	sum = sum + i;
	cout << "SUM After Dst Port: " << sum << endl;
	// End Dst Port

	// Begin UDP Length (UDP header length field)
	//	udpPacketData = string(data + HEADER_SIZE_BYTES);

	//	cout << "Data in Client compute checksum:\n" << udpPacketData << endl;

	i = ((data[4] << 8) | data[5]);// HEADER_SIZE_BYTES + udpPacketData.length();
	sum = sum + i;
	cout << "SUM After Length: " << sum << endl;
	// End UDP Length

	//	cout << "UDP Packet: " << udpPacketData << endl;

	//	cout << "UDP Packet Len: " << udpPacketData.length() << endl;

	unsigned short packetLen = ((data[4] << 8) | data[5]);
	char c;
	int x;
	for (x = HEADER_SIZE_BYTES; x < (packetLen - 1); x += 2)
	{
		c = data[x];
		i = (unsigned short)c;

		c = data[x + 1];
		i = (i << 8) | (unsigned short)c;

		sum = sum + i;
	}


	if (x == (packetLen - 1))
	{
		i = (unsigned short)data[x];

		i = (i << 8);
		sum = sum + i;
	}



	checksum = oneComplementSum(sum);

	if (checksum != 65535)
		checksum = ~checksum;

	cout << "Checksum: " << checksum << endl;

	return checksum;
}
