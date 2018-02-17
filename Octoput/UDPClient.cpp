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
		delete UDPSocket;
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



void UDPClient::commenceOctovation()
{
	string filenameOK = askUserForFilename();
	int nBytesRcvd;
	char rcvMssg[1200];

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

	cout << "Octo Descripto Received: ";
	rcvMssg[nBytesRcvd] = '\0';
	for (int i = 0; i < nBytesRcvd; i++)
		cout << rcvMssg[i];
	cout << endl;



	// Build Octo Monocto.
	bool octoDescriptoOk = parseOctoDescripto(string(rcvMssg));

	cout << "Octo Descripto received " << (octoDescriptoOk ? "OK.\n" : "NOT OK.\n");

	if (octoDescriptoOk)
	{
		cout
			<< "Total Size Of File: " << octoMonocto.totalFileSize << endl
			<< "Number Of Full Octoblocks: " << octoMonocto.numFullOctoblocks << endl
			<< "Size Of Partial Octoblock: " << octoMonocto.partialOctoblockSize << endl
			<< "Size Of Partial Octolegs: " << octoMonocto.partialOctolegSize << endl
			<< "Size Of Leftover Data: " << octoMonocto.leftoverDataSize << endl;

		

	}

	return;
}




string UDPClient::askUserForFilename()
{
	int nBytesRcvd;
	string filename;
	string confirmationStr;
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

	if (nBytesRcvd == -1)
	{
		cout << "Failure receiving confirmation, try again or try another file.\n";
		return '\0';
	}

	confirmationStr = string(confirmation);
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

		// Probably wanna check that nBytesReceived correlates with value in header

		if (nBytesRcvd == -1)
			return '\0';
	}

	if (confirmationStr.compare("OK") != 0)
		return '\0';
	else
		return confirmationStr;
}



bool UDPClient::parseOctoDescripto(string octoDescripto)
{
	int posA;
	int posB;
	string worker;

	posA = octoDescripto.find("Total Size Of File: ", 0);

	if (posA != std::string::npos)
	{
		octoDescripto = octoDescripto.substr(posA + 20);

		posB = octoDescripto.find("\r\n");

		if (posB != std::string::npos)
			octoMonocto.totalFileSize = (short)atoi((octoDescripto.substr(0, posB)).c_str());
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


	
	cout << "OctoDescripto: " << octoDescripto << endl;
	posA = octoDescripto.find("Number Of Full Octoblocks: ", 0);

	if (posA != std::string::npos)
	{
		octoDescripto = octoDescripto.substr(posA + 27);

		posB = octoDescripto.find("\r\n");

		if (posB != std::string::npos)
			octoMonocto.numFullOctoblocks = (short)atoi((octoDescripto.substr(0, posB)).c_str());
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



	cout << "OctoDescripto: " << octoDescripto << endl;
	posA = octoDescripto.find("Size Of Partial Octoblock: ", 0);

	if (posA != std::string::npos)
	{
		octoDescripto = octoDescripto.substr(posA + 27);

		posB = octoDescripto.find("\r\n");

		if (posB != std::string::npos)
			octoMonocto.partialOctoblockSize = (short)atoi((octoDescripto.substr(0, posB)).c_str());
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



	cout << "OctoDescripto: " << octoDescripto << endl;
	posA = octoDescripto.find("Size Of Partial Octolegs: ", 0);

	if (posA != std::string::npos)
	{
		octoDescripto = octoDescripto.substr(posA + 26);

		posB = octoDescripto.find("\r\n");

		if (posB != std::string::npos)
			octoMonocto.partialOctolegSize = (short)atoi((octoDescripto.substr(0, posB)).c_str());
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



	cout << "OctoDescripto: " << octoDescripto << endl;
	posA = octoDescripto.find("Size Of Leftover Data: ", 0);

	if (posA != std::string::npos)
	{
		octoDescripto = octoDescripto.substr(posA + 23);

		posB = octoDescripto.find("\r\n");

		if (posB != std::string::npos)
			octoMonocto.leftoverDataSize = (short)atoi((octoDescripto.substr(0, posB)).c_str());
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



bool UDPClient::validateMessage(const char* data, int dataLen)
{

}



bool UDPClient::validateChecksum(const char* data)
{

}
