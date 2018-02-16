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


int UDPServer::getSocketFD()
{
	return UDPSocket != NULL ? UDPSocket->getFD() : -1;
}


struct sockaddr_in UDPServer::getClientAddress()
{
	return clientAddress;
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



void UDPServer::commenceOctovation()
{	
	string filename = getFileRequest();
	string octoDescripto;

	if (filename == "\0")
	{
		cout << "Receive filename unsuccessful..exiting.\n";
		exit(0);
	}


	in.open(filename);
	getline(in, fileContents, '\0');
	in.close();


	instantiateOctoMonocto();
	instantiateOctoblocks();



	octoDescripto =
		"Total Size Of File: " + to_string(fileContents.length()) + "\r\n" +
		"Number Of Full Octoblocks: " + to_string(octoMonocto.numFullOctoblocks) + "\r\n" +
		"Size Of Partial Octoblock: " + to_string(octoMonocto.partialOctoblockSize) + "\r\n" +
		"Size Of Partial Octolegs: " + to_string(octoMonocto.partialOctolegSize) + "\r\n" +
		"Size Of Leftover Data: " + to_string(octoMonocto.leftoverDataSize) + "\r\n";


	sendto
	(
		UDPSocket->getFD(),
		octoDescripto.c_str(),
		octoDescripto.length(),
		0,
		clientAddressPtr,
		clientAddressLen
	);





	return;
}



std::string UDPServer::getFileRequest()
{
	int nBytesRcvd;
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

	filename[nBytesRcvd + 1] = '\0';
	cout << "Filename Received: ";
	for (int i = 0; i < nBytesRcvd; i++)
		cout << filename[i];
	cout << endl;

	in.open(filename);


	while (!(strncmp(filename, "quit", 4) == 0) && !in.good())
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

		filename[nBytesRcvd + 1] = '\0';
		cout << "Filename Received: ";
		for (int i = 0; i < nBytesRcvd; i++)
			cout << filename[i];
		cout << endl;

		in.open(filename);
	}

	in.close();

	if (strncmp(filename, "quit", 4) == 0)
		return '\0';
	else
		return filename;
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
		(short)numFullOctoblocksNeeded,
		(short)partialOctoblockSize,
		(short)partialOctolegSize,
		(short)leftoverDataSize
	};

}



void UDPServer::instantiateOctoblocks()
{
	octoblocks = new std::string[totalOctoblocksNeeded];
	octoblocks = new std::string[10000000];

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


