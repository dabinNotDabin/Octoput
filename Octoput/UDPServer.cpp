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

	if (octoblockData != NULL)
		delete[] octoblockData;

	if (taskQ != NULL)
		delete taskQ;
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
	taskQ = new TaskQueue();

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
		bindSocket();

		clientAddress.sin_family = family;
		clientAddress.sin_port = htons(clientPort);
		inet_pton(AF_INET, "127.0.0.1", &clientAddress.sin_addr);
		cout << "Client Port Set In Server Constructor Netw Order: " << clientAddress.sin_port << endl;
	}

	currentOctoblock = 0;
	for (uint8_t i = 0; i < 8; i++)
		taskQ->putTask(i);
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





void* UDPServer::serverThread(void* id)
{
//	cout << "Thread id: " << (long)id << endl;

	unsigned short octolegSize;
	unsigned char* octoleg;
	unsigned short startPos;
	uint8_t octolegID;

	UDPServer* server = ((UDPServer*)id);
	octolegID = server->taskQ->getTask();


	if (server->currentOctoblock < server->octoMonocto.numFullOctoblocks)
	{
		cout << "Working on full octoblocks.\n";
		octoleg = new unsigned char[server->octoblockSize + 1];

		startPos = (server->currentOctoblock * 8888) + (octolegID * server->fullOctolegSize);
		octolegSize = server->fullOctolegSize;
		memcpy(octoleg, (char*)(server->octoblockData[startPos]), octolegSize);
	}
	else if (server->currentOctoblock == server->octoMonocto.numFullOctoblocks)
	{
		cout << "Working on partial octoblock.\n";
		octoleg = new unsigned char[server->octoMonocto.partialOctolegSize + 1];

		startPos = 
			(server->octoMonocto.numFullOctoblocks * 8888) +
			(octolegID * server->octoMonocto.partialOctolegSize);
		octolegSize = server->octoMonocto.partialOctolegSize;
		memcpy(octoleg, (char*)(server->octoblockData[startPos]), octolegSize);
	}
	else
	{
		cout << "Working on leftover data.\n";
		octoleg = new unsigned char[2];

		startPos =
			(server->octoMonocto.numFullOctoblocks * 8888)  +
			(server->octoMonocto.partialOctolegSize * 8)	+
			(octolegID * 1);
		octolegSize = 1;
		memcpy(octoleg, (char*)(server->octoblockData[startPos]), octolegSize);
	}

	
	server->sendMssgRequireAck(octoleg, octolegSize, 100000);


	cout << "Ack Received.\n";

	

	pthread_exit(0);
}



void UDPServer::commenceOctovation()
{	
	string filename = getFileRequest();
	string octoDescripto;
	string confirmation = "OK";

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


	in.open(filename, ios::in | ios::binary);
	fileContents = string((istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

	instantiateOctoMonocto();

	octoblockData = new unsigned char [octoMonocto.totalFileSize + (8 - (octoMonocto.totalFileSize % 8))];
	octoblockData = (unsigned char*)fileContents.data();

	for (int i = octoMonocto.totalFileSize; i % 8 != 0; i++)
		octoblockData[i] = '\0';

	cout << "FILE: \n" << fileContents << endl;

	in.close();

	octoDescripto =
		"Total Size Of File: " + to_string(fileContents.length()) + "\r\n" +
		"Number Of Full Octoblocks: " + to_string(octoMonocto.numFullOctoblocks) + "\r\n" +
		"Size Of Partial Octoblock: " + to_string(octoMonocto.partialOctoblockSize) + "\r\n" +
		"Size Of Partial Octolegs: " + to_string(octoMonocto.partialOctolegSize) + "\r\n" +
		"Size Of Leftover Data: " + to_string(octoMonocto.leftoverDataSize) + "\r\n";


	memcpy(sendMssg, octoDescripto.c_str(), octoDescripto.length());

	sendMssg[octoDescripto.length()] = '\0';
	attachHeader('\0', octoDescripto.length(), sendMssg);

	sendMssgRequireAck(sendMssg, octoDescripto.length() + HEADER_SIZE_BYTES, 100000);
	


	pthread_t* threads;
	threads = new pthread_t[8];
	long status;
	long i;
	for (i = 0; i < 8; i++)
	{
		status = pthread_create(&threads[i], NULL, &(this->serverThread), (void*)this);
//		status = pthread_create(&threads[i], NULL, (THREADFUNCPTR)&UDPServer::serverThread, (void*)this);
		if (status != 0)
		{
			std::cout << "Creation of thread resulted in error.\n";
			exit(-1);
		}
	}



	for (i = 0; i < 8; i++)
		pthread_join(threads[i], NULL);


	delete[] threads;



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

	//elapsed = clock() - before;

	//cout << "Time elapsed: " << elapsed / CLOCKS_PER_SEC << " seconds.\n";

	return;
}





void UDPServer::sendMssgRequireAck(const unsigned char* mssg, int mssgLen, int timeoutMsec)
{
	string mssgStr;
	string ackStr;
	int nBytesRcvd;
	unsigned short checksum;
	unsigned short rcvdChecksum;
	unsigned char ack[ACK_SIZE_BYTES + 1];
	bool ackOK = false;

	sendto
	(
		UDPSocket->getFD(),
		mssg,
		mssgLen,
		0,
		clientAddressPtr,
		clientAddressLen
	);

	usleep(timeoutMsec);

	memset(ack, 0, ACK_SIZE_BYTES);
	nBytesRcvd = recvfrom
	(
		UDPSocket->getFD(),
		ack,
		ACK_SIZE_BYTES,
		0,
		clientAddressPtr,
		&clientAddressLen
	);

	while (!ackOK)
	{
		rcvdChecksum = ((ack[6] << 8) | ack[7]);
		cout << "Received ACK Checksum: " << rcvdChecksum << endl;

		ack[nBytesRcvd] = '\0';
		cout << "ACK Received: ";
		for (int i = 0; i < nBytesRcvd; i++)
			cout << ack[i];
		cout << " of length: " << nBytesRcvd << endl;

		checksum = computeChecksum((unsigned char*)ack, inet_ntoa(clientAddress.sin_addr), clientAddress.sin_port);
		cout << "ACK Checksum Computed: " << checksum << endl;

		if (checksum == rcvdChecksum)
		{
			cout << "Ack OK.\n";
			ackOK = true;
		}
		else
		{
			sendto
			(
				UDPSocket->getFD(),
				mssg,
				mssgLen,
				0,
				clientAddressPtr,
				clientAddressLen
			);

			usleep(timeoutMsec);

			memset(ack, 0, ACK_SIZE_BYTES);
			nBytesRcvd = recvfrom
			(
				UDPSocket->getFD(),
				ack,
				ACK_SIZE_BYTES,
				0,
				clientAddressPtr,
				&clientAddressLen
			);

		}
	}

	return;
}






std::string UDPServer::getFileRequest()
{
	int nBytesRcvd;
	string filenameStr;
	unsigned char filename[276];
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


	filenameStr = string((char*)filename);
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

		filenameStr = string((char*)filename);
		in.open((char*)filename);
	}

	in.close();

//	filenameStr = string((char*)filename);
	if (filenameStr.compare("quit") == 0)
		return '\0';
	else
		return filenameStr;
}



void UDPServer::instantiateOctoMonocto()
{
	int totalOctoblocksNeeded;
	int numFullOctoblocksNeeded;
	int partialOctoblockSize;
	int partialOctolegSize;
	int leftoverDataSize;

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



// data must be null terminated
void UDPServer::attachHeader(unsigned char octolegFlag, unsigned short payloadSize, unsigned char* data)
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
	//	if (octolegFlag == NULL)
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


	string dataStr((char*)data);

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

	cout << "Checksum in Server attachHeader(): " << checksum << endl;

	firstHalf = checksum >> 8;
	secondHalf = checksum & 0xFF;

	header[6] = firstHalf;
	header[7] = secondHalf;

	cout << "Checksum First Half: " << (int)header[6] << endl;
	cout << "Checksum Secnd Half: " << (int)header[7] << endl;


	memcpy(data + 6, header + 6, 2);


	cout << "Checksum First Half: " << (int)data[6] << endl;
	cout << "Checksum Secnd Half: " << (int)data[7] << endl;



	header[8] = '\0';


	for (int i = 0; i < 9; i++)
	{
		cout << "Index: " << i << " = " << (unsigned int)header[i] << endl;
	}


//	delete[] header;
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
