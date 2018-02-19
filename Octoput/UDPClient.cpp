#include "global.h"
#include "UDPClient.h"
#include "UDPServer.h"




UDPClient::UDPClient()
{

}

UDPClient::~UDPClient()
{
	if (UDPSocket != NULL)
	{
		close(UDPSocket->getFD());
//		delete UDPSocket;
	}
}


int UDPClient::getSocketFD()
{
	return UDPSocket != NULL ? UDPSocket->getFD() : -1;
}


struct sockaddr_in UDPClient::getServerAddress()
{
	return serverAddress;
}


struct sockaddr_in UDPClient::getAddress()
{
	struct sockaddr_in addr;

	UDPSocket->getAddress(addr);

	return addr;
}


// Note that this is configured to use the local host IP and the given family for the server's address.
UDPClient::UDPClient(short family, short type, short protocol, unsigned int port, unsigned int serverPort)
{
	UDPSocket = new Socket();
	struct sockaddr_in address;

	if (!UDPSocket->initSocket(family, type, protocol))
	{
		cout << "Failure initializing socket with <family>, <type>, and <protocol>.\n";
		UDPClient();
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

		serverAddress.sin_family = family;
		serverAddress.sin_port = htons(serverPort);
		inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);
	}
}



bool UDPClient::bindSocket()
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





void UDPClient::commenceOctovation()
{
	string filenameOK = askUserForFilename();
	int nBytesRcvd;
	unsigned char rcvMssg[1200];
	unsigned short rcvdChecksum;
	unsigned short checksum;

	if (filenameOK.compare("\0") == 0)
	{
		cout << "File query unsuccessful..exiting.\n";
		return;
	}

	// Receive Octo Descripto.
	nBytesRcvd = recvfrom
	(
		UDPSocket->getFD(),
		rcvMssg,
		1200,
		0,
		serverAddressPtr,
		&serverAddressLen
	);

	if (nBytesRcvd == -1)
	{
		cout << "Failure receiving Octo Descripto.. exiting.\n";
		return;
	}

	cout << "Octo Descripto Received:\n";
	rcvMssg[nBytesRcvd] = '\0';
	for (int i = 0; i < nBytesRcvd; i++)
		cout << rcvMssg[i];
	cout << endl;



	rcvdChecksum = ((rcvMssg[6] << 8) | rcvMssg[7]);
	cout << "Received Checksum: " << rcvdChecksum << endl;

	checksum = computeChecksum(rcvMssg, inet_ntoa(serverAddress.sin_addr), serverAddress.sin_port);
	cout << "Checksum Computed: " << checksum << endl;

	if (checksum == rcvdChecksum)
		sendAck(0);

	//// While checksum not valid || parse NOT OK, receive message -- parse should be OK if checksum valid.

	//// Build Octo Monocto.
	//bool octoDescriptoOk = parseOctoDescripto(rcvMssg);

	//cout << "Octo Descripto parsed " << (octoDescriptoOk ? "OK.\n" : "NOT OK.\n");

	//if (octoDescriptoOk)
	//{
	//	cout
	//		<< "Total Size Of File: " << octoMonocto.totalFileSize << endl
	//		<< "Number Of Full Octoblocks: " << octoMonocto.numFullOctoblocks << endl
	//		<< "Size Of Partial Octoblock: " << octoMonocto.partialOctoblockSize << endl
	//		<< "Size Of Partial Octolegs: " << octoMonocto.partialOctolegSize << endl
	//		<< "Size Of Leftover Data: " << octoMonocto.leftoverDataSize << endl;
	//}



	return;
}




bool UDPClient::sendAck(unsigned char id)
{
	unsigned char ack[ACK_SIZE_BYTES + 1];

	ack[0] = id;
	ack[1] = '\0';
	attachHeader('\0', 1, ack);

	sendto
	(
		UDPSocket->getFD(),
		ack,
		ACK_SIZE_BYTES,
		0,
		(const sockaddr*)(&serverAddress),
		sizeof(serverAddress)
	);
}




// data must be null terminated
void UDPClient::attachHeader(unsigned char octolegFlag, unsigned short payloadSize, unsigned char* data)
{
	struct sockaddr_in clientAddress;
	unsigned short serverPort;
	unsigned short clientPort;
	unsigned short packetLen;
	unsigned short checksum;
	unsigned char firstHalf;
	unsigned char secondHalf;
	unsigned char* header = new unsigned char[9];



	UDPSocket->getAddress(clientAddress);

	// Client Port
	clientPort = clientAddress.sin_port;
	cout << "ClientPort: " << clientAddress.sin_port << endl;
	firstHalf = clientPort >> 8;
	secondHalf = clientPort & 0xFF;

	header[0] = firstHalf;
	header[1] = secondHalf;


	// Server Port
	serverPort = serverAddress.sin_port;
	cout << "ServerPort: " << serverPort << endl;
	cout << "Server IP: " << inet_ntoa(serverAddress.sin_addr) << endl;
	firstHalf = serverPort >> 8;
	secondHalf = serverPort & 0xFF;

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
		inet_ntoa(serverAddress.sin_addr),
		(unsigned int)(serverAddress.sin_port)
	);

	cout << "Checksum in Client attachHeader(): " << checksum << endl;

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






string UDPClient::askUserForFilename()
{
	int nBytesRcvd;
	string filename;
	string confirmationStr;
	unsigned char confirmation[23];

	cout << "Enter the name of the file you would like to receive: ";
	cin >> filename;
	cout << endl;
	sendto
	(
		UDPSocket->getFD(),
		filename.c_str(),
		filename.length(),
		0,
		(const sockaddr*)(&serverAddress),
		sizeof(serverAddress)
	);

	cout << "Sent.\n";


	nBytesRcvd = recvfrom
	(
		UDPSocket->getFD(),
		confirmation,
		23,
		0,
		serverAddressPtr,
		&serverAddressLen
	);

	cout << "Confirmation Received: ";
	confirmation[nBytesRcvd] = '\0';
	for (int i = 0; i < nBytesRcvd; i++)
		cout << confirmation[i];
	cout << endl;

	// validate checksum.

	if (nBytesRcvd == -1)
	{
		cout << "Failure receiving confirmation, try again or try another file.\n";
		return '\0';
	}

	confirmationStr = string((char*)confirmation);
	while (confirmationStr.compare("OK") != 0)
	{
		cout << "Enter the name of the file you would like to receive: ";
		cin >> filename;
		cout << endl;
		sendto
		(
			UDPSocket->getFD(),
			filename.c_str(),
			filename.length(),
			0,
			(const sockaddr*)(&serverAddress),
			sizeof(serverAddress)
		);

		nBytesRcvd = recvfrom
		(
			UDPSocket->getFD(),
			confirmation,
			23,
			0,
			serverAddressPtr,
			&serverAddressLen
		);

		confirmation[nBytesRcvd] = '\0';
		for (int i = 0; i < nBytesRcvd; i++)
			cout << confirmation[i];
		cout << endl;

		
		// validate checksum.

		if (nBytesRcvd == -1)
			return '\0';

		confirmationStr = string((char*)confirmation);
	}

	if (confirmationStr.compare("OK") != 0)
		return '\0';
	else
		return confirmationStr;
}



bool UDPClient::parseOctoDescripto(const unsigned char* octoDescripto)
{
	int posA;
	int posB;
	unsigned short packetLen;
	unsigned short field;
	string worker;
	unsigned char* octoDescriptoNoHeader;
	string octoDescriptoStr;


	field = ((octoDescripto[0] << 8) | octoDescripto[1]);
	cout << "Src Port: " << field << endl;
	field = ((octoDescripto[2] << 8) | octoDescripto[3]);
	cout << "Dst Port: " << field << endl;
	packetLen = field = ((octoDescripto[4] << 8) | octoDescripto[5]);
	cout << "Pack Len: " << field << endl;
	field = ((octoDescripto[6] << 8) | octoDescripto[7]);
	cout << "Checksum: " << field << endl;


	octoDescriptoNoHeader = new unsigned char[packetLen - 7];
	memcpy(octoDescriptoNoHeader, octoDescripto + HEADER_SIZE_BYTES, packetLen - 7);

//	octoDescriptoNoHeader[packetLen - HEADER_SIZE_BYTES] = '\0';
	octoDescriptoStr = string((char*)octoDescriptoNoHeader);

//	delete[] octoDescriptoNoHeader;

	posA = octoDescriptoStr.find("Total Size Of File: ", 0);
	if (posA != std::string::npos)
	{
		octoDescriptoStr = octoDescriptoStr.substr(posA + 20);

		posB = octoDescriptoStr.find("\r\n");

		if (posB != std::string::npos)
			octoMonocto.totalFileSize = (short)atoi((octoDescriptoStr.substr(0, posB)).c_str());
		else
		{
			cout << "octoDescripto missing \"\r\n\" after total file size tag.\n";
			return false;
		}
	}
	else
	{
		cout << "OctoDescripto missing total size of file information.\n";
		return false;
	}


	
	cout << "OctoDescripto: " << octoDescriptoStr << endl;
	posA = octoDescriptoStr.find("Number Of Full Octoblocks: ", 0);

	if (posA != std::string::npos)
	{
		octoDescriptoStr = octoDescriptoStr.substr(posA + 27);

		posB = octoDescriptoStr.find("\r\n");

		if (posB != std::string::npos)
			octoMonocto.numFullOctoblocks = (short)atoi((octoDescriptoStr.substr(0, posB)).c_str());
		else
		{
			cout << "octoDescripto missing \"\r\n\" after num full octoblocks tag.\n";
			return false;
		}
	}
	else
	{
		cout << "OctoDescripto missing informatio on num full octoblocks required.\n";
		return false;
	}



	cout << "OctoDescripto: " << octoDescriptoStr << endl;
	posA = octoDescriptoStr.find("Size Of Partial Octoblock: ", 0);

	if (posA != std::string::npos)
	{
		octoDescriptoStr = octoDescriptoStr.substr(posA + 27);

		posB = octoDescriptoStr.find("\r\n");

		if (posB != std::string::npos)
			octoMonocto.partialOctoblockSize = (short)atoi((octoDescriptoStr.substr(0, posB)).c_str());
		else
		{
			cout << "octoDescripto missing \"\r\n\" after size of partial octoblock tag.\n";
			return false;
		}
	}
	else
	{
		cout << "OctoDescripto missing partial octoblock size information.\n";
		return false;
	}



	cout << "OctoDescripto: " << octoDescriptoStr << endl;
	posA = octoDescriptoStr.find("Size Of Partial Octolegs: ", 0);

	if (posA != std::string::npos)
	{
		octoDescriptoStr = octoDescriptoStr.substr(posA + 26);

		posB = octoDescriptoStr.find("\r\n");

		if (posB != std::string::npos)
			octoMonocto.partialOctolegSize = (short)atoi((octoDescriptoStr.substr(0, posB)).c_str());
		else
		{
			cout << "octoDescripto missing \"\r\n\" after partial octoleg size tag.\n";
			return false;
		}
	}
	else
	{
		cout << "OctoDescripto missing partial octoleg size information.\n";
		return false;
	}



	cout << "OctoDescripto: " << octoDescriptoStr << endl;
	posA = octoDescriptoStr.find("Size Of Leftover Data: ", 0);

	if (posA != std::string::npos)
	{
		octoDescriptoStr = octoDescriptoStr.substr(posA + 23);

		posB = octoDescriptoStr.find("\r\n");

		if (posB != std::string::npos)
			octoMonocto.leftoverDataSize = (short)atoi((octoDescriptoStr.substr(0, posB)).c_str());
		else
		{
			cout << "octoDescripto missing \"\r\n\" after leftover data size tag.\n";
			return false;
		}
	}
	else
	{
		cout << "OctoDescripto missing leftover data size information.\n";
		return false;
	}



	return true;
}





unsigned short UDPClient::computeChecksum(const unsigned char* data, const char* serverIP, unsigned int serverPort)
{
	sockaddr_in clientAddress;
	string pseudoHeaderIPs;
	string udpPacketData;
	string worker;
	unsigned short i;
	unsigned short j;
	unsigned short checksum;
	unsigned int sum;
	char* byte;

	cout << "Server IP in Client Compute Checksum: " << string(serverIP) << endl;

	UDPSocket->getAddress(clientAddress);

	pseudoHeaderIPs = inet_ntoa(clientAddress.sin_addr) + string(".") + string(serverIP);
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

//	cout << "Checksum: " << checksum << endl;

	return checksum;
}
