#include "global.h"
#include "UDPClient.h"
#include "UDPServer.h"



UDPClient::UDPClient()
{

}

UDPClient::~UDPClient()
{
	close(UDPSocket->getFD());

	if (UDPSocket != NULL)
		delete UDPSocket;
}


int UDPClient::getSocketFD()
{
	return UDPSocket != NULL ? UDPSocket->getFD() : -1;
}

struct sockaddr_in UDPClient::getServerAddress()
{
	return serverAddress;
}


// Note that this is configured to use the local host IP and the given family for the server's address.
UDPClient::UDPClient(short family, short type, short protocol, unsigned int port, unsigned int serverPort)
{
	UDPSocket = new Socket();

	if (!UDPSocket->initSocket(family, type, protocol))
	{
		cout << "Failure initializing socket with <family>, <type>, and <protocol>.\n";
		UDPClient();
	}
	else
	{
		UDPSocket->associateAddress(port);

		serverAddress.sin_family = family;
		serverAddress.sin_port = htons(serverPort);
		inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr);
	}
	
}



void UDPClient::commenceOctovation(UDPServer& s)
{
	string filenameOK = askUserForFilename(s);

	
	if (filenameOK == "\0")
	{
		cout << "File query unsuccessful..exiting.\n";
		exit(0);
	}

	cout << "Confirmed.\n";
}




string UDPClient::askUserForFilename(UDPServer& s)
{
	int nBytesRcvd;
	string filename;
	char confirmation[23];

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


	// Server receive	
	char fname[276];
	cout << "Received.\n";

	nBytesRcvd = recvfrom
	(
		s.UDPSocket->getFD(),
		fname,
		276,
		0,
		s.clientAddressPtr,
		&(s.clientAddressLen)
	);


	// Probably wanna check that nBytesReceived correlates with value in header
	if (nBytesRcvd == -1)
	{
		cout << "Failure.\n";
		return '\0';
	}

	fname[nBytesRcvd + 1] = '\0';
	cout << "Filename Received: ";
	for (int i = 0; i < nBytesRcvd; i++)
		cout << fname[i];
	cout << endl;



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
	for (int i = 0; i < nBytesRcvd; i++)
		cout << confirmation[i];
	cout << endl;

	if (nBytesRcvd == -1)
	{
		cout << "Failure.\n";
		return '\0';
	}

	while (!(strncmp(confirmation, "OK", 4) == 0))
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

		for (int i = 0; i < nBytesRcvd; i++)
			cout << confirmation[i];
		cout << endl;

		// Probably wanna check that nBytesReceived correlates with value in header

		if (nBytesRcvd == -1)
			return '\0';
	}

	if (strncmp(confirmation, "OK", 4) == 0)
		return '\0';
	else
		return confirmation;
}



bool UDPClient::validateMessage(const char* data, int dataLen)
{

}



bool UDPClient::validateChecksum(const char* data)
{

}
