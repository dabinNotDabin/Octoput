#include "Sloxy.h"


Sloxy::Sloxy()
{

}

Sloxy::~Sloxy()
{

}


// Takes an HTTP request and deciphers the host's information.
// Sets hostName to the human readable host name and hostNameLen to the length of the host's name.
// Sets port = 80 if no port number specified, otherwise sets port equal to the specified port number.
void Sloxy::getHostInfoFromRequest(char *httpRequest, char *hostName, int &hostNameLen, int &port)
{
	string req(httpRequest);

	// Find the "Host:" tag
	size_t pos = req.find("Host:");
	size_t end;
	int len;

	// Ensures that the request contained the "Host:" tag
	if (pos != string::npos)
	{
		pos += 6;
		end = pos;

		// Find the end of the line.
		while (end < req.length() && req[end] != ':' && req[end] != '\r')
			end++;

		// Extract the host name from the request.
		if (end < req.length())
		{
			cout << "Host info (extracted from request):\n";

			len = end - pos;
			string hostStr = req.substr(pos, len);

			cout << "\tName: " << hostStr.c_str() << endl;

			memcpy(hostName, hostStr.c_str(), len + 1);

			hostNameLen = len;
		}
		else
			cout << "Error reading host name, end of request string reached before name extracted.\n";

		// If a port number was specified, extract it from the request.
		if (req[end] == ':')
		{
			end++;
			pos = end;

			while (req[end] != '\r')
				end++;

			len = end - pos;
			string portStr = req.substr(pos, len);

			port = atoi(portStr.c_str());
		}
		else
			port = 80;

		cout << "\tPort requested: " << port << endl;
	}
	else
		cout << "Host: tag not found in request header.\n";
}




// Uses an http request message to build a host internet address
void Sloxy::buildHostInetAddr(string hostName, int port, struct sockaddr_in &hostInetAddress)
{
	struct hostent *hostEnt = NULL;
	// if string arg is already a valid IP, no lookup is performed
	hostEnt = gethostbyname(hostName.c_str());

	if (hostEnt == NULL)
	{
		cout << "Host name not found.\n";
		return;
	}
	else
		cout << "Host name to IP lookup successful.\n";

	struct in_addr **addr_list;
	addr_list = (struct in_addr **) hostEnt->h_addr_list;

	memset((&hostInetAddress), 0, sizeof(hostInetAddress));
	hostInetAddress.sin_family = hostEnt->h_addrtype;
	hostInetAddress.sin_port = htons(port);
	hostInetAddress.sin_addr = *addr_list[0];

	cout << "Host IP: " << inet_ntoa(hostInetAddress.sin_addr) << endl;
	cout << "Host address type: " << hostInetAddress.sin_family << endl;
	cout << "Host port: " << ntohs(hostInetAddress.sin_port) << endl;
}




void Sloxy::sendMessage(int socketID, char message[], int msgLength, int &bytesSent)
{
	int sentCount = -1;
	int errsv = 0;

	sentCount = send(socketID, message, msgLength, 0);
	//sentCount = send(webServerSocketID, headRequest, receivedCount+1, 0);
	while (sentCount == -1 || sentCount < msgLength)
	{
		errsv = errno;
		cout << "Send message failed or incomplete.\n";
		// process errsv if necessary
	}

	bytesSent = sentCount;
}



void Sloxy::receiveMessage(int socketID, char message[], int &msgLength)
{
	int receivedCount = -1;
	int errsv = 0;
	char rcv_message[10000];

	receivedCount = recv(socketID, rcv_message, 10000, 0);
	if (receivedCount == 0)
	{
		cout << "No messages and peer has shutdown connection..\n";
		return;
	}
	else if (receivedCount == -1)
	{
		errsv = errno;
		cout << "Receive message failed..\n";
		// process errsv if necessary
		return;
	}
	else
	{
		memcpy(message, rcv_message, receivedCount + 1);
		msgLength = receivedCount;
	}
}





bool Sloxy::isHtml(const char *httpRequest)
{
	string req(httpRequest);

	size_t pos;
	size_t end;
	int len;

	// Isolate the URL.
	pos = req.find(' ') + 1;
	end = req.find(' ', pos);
	len = end - pos;
	string urlString = req.substr(pos, len);


	// Find the last period to isolate the url suffix.
	pos = urlString.find_last_of('.');

	// Find the html suffix.
	end = urlString.find("html", pos);

	// If not found, return false, otherwise, return true.
	if (end == string::npos)
		return false;

	return true;
}


int Sloxy::getHttpResponseCode(char *httpResponse)
{	
	size_t posA = -1;
	size_t posB = -1;
	string response(httpResponse);

	posA = response.find_first_of(' ');

	if (posA == string::npos)
	{
		return -1;
	}
	else
	{
		posB = response.find_first_of(' ', posA + 1);
		if (posB == string::npos)
		{
			return -1;
		}
		else
		{
			posA++;
			return atoi((response.substr(posA, posB - posA)).c_str());
		}
	}
}



// Changes a GET request to a HEAD request.
// More specifically, replaces the first 3 characters in httpRequest with "HEAD"
void Sloxy::fromGetToHead(char *httpRequest, int &msgLength)
{
	string req(httpRequest);
	string headRequest("HEAD" + req.substr(3));

	memcpy(httpRequest, headRequest.c_str(), headRequest.length() + 1);

	msgLength = headRequest.length();
}



// Changes a GET request to a HEAD request.
// More specifically, replaces the first 3 characters in httpRequest with "HEAD"
void Sloxy::fromGetToRangeGet(char *httpRequest, int beginByte, int numBytes, int &msgLength)
{
	char buffer[20];

	string req(httpRequest);
	string begin = to_string(beginByte);
	string end = to_string(beginByte + numBytes - 1);

	req = req.substr(0, req.length() - 2);
	string rangeRequest(req + "Range: bytes=" + begin + "-" + end + "\r\n\r\n");

	memcpy(httpRequest, rangeRequest.c_str(), rangeRequest.length() + 1);

	msgLength = rangeRequest.length();
}




int Sloxy::getContentLength(char * httpResponse)
{
	string req(httpResponse);

	size_t pos = req.find("Content-Length:") + 16;
	size_t end = pos;

	int len;
	int numBytes = -1;

	while (req[end] != '\r')
		end++;

	len = end - pos;
	numBytes = atoi((req.substr(pos, len)).c_str());


	return numBytes;
}


bool Sloxy::acceptsRanges(char *httpResponse)
{
	string req(httpResponse);

	size_t pos = req.find("Accept-Ranges:");
	size_t end;
	int len;

	if (pos == string::npos)
	{
		cout << "\"Accept-Ranges:\" tag not found.\n";
		return false;
	}

	pos += 15;
	end = pos;

	while (req[end] != '\r')
		end++;

	len = end - pos;

	string rangeUnits = req.substr(pos, len);

	if (rangeUnits.compare("bytes") == 0)
		return true;

	return false;
}



string Sloxy::getFirstLine(char *sequence)
{
	int pos = -1;
	string str(sequence);

	pos = str.find("\r\n");

	if (pos == string::npos)
		return "fuck you";
	
	return str.substr(0, pos);
}


string Sloxy::removeLineStartingWith(const char *sequence, string lineBeginning)
{
	int posA = -1;
	int posB = -1;
	string str(sequence);

	posA = str.find(lineBeginning);

	if (posA == string::npos)
	{
		return str;
	}

	cout << "\nRemoving line with tag: " << lineBeginning << endl;

	posB = str.find("\r\n", posA);

	if (posB == string::npos)
	{
		return str;
	}

	cout << "\nRemoved line with tag: " << lineBeginning << endl;

	return str.substr(0, posA) + str.substr(posB + 2);
}




bool Sloxy::hasLocationTag(const char *httpResponse)
{
	int pos = -1;
	string response(httpResponse);

	pos = response.find("Location:");

	if (pos != string::npos)
		return true;
	else
		return false;
}




string Sloxy::getLocation(const char *httpResponse)
{
	int posA = -1;
	int posB = -1;
	string response(httpResponse);

	posA = response.find("Location:");
		
	if (posA == string::npos)
		return "fuck you - no location tag.\n";
	else
	{
		posA+= 10;
		posB = response.find("\r\n", posA);

		if (posB = string::npos)
			return "fuck you - reached end of response before end of location url.\n";
		else
			return response.substr(posA, posB - posA);
	} 
}



string Sloxy::replaceUrl(const char *httpRequest, string newUrl)
{
	int posA = -1;
	int posB = -1;
	string request(httpRequest);
	string newRequest;

	posA = request.find_first_of(' ');

	if (posA == string::npos)
	{
		return "fuck you - no spaces whatsoever in httpRequest.\n";
	}
	else
	{
		posA++;
		posB = request.find_first_of(' ');

		if (posB == string::npos)
			return "fuck you - only one space found in entire httpRequest.\n";
		else
		{
			newRequest = request.substr(0, posA) + newUrl + request.substr(posB);
//			newLength = newRequest.length();
			return newRequest;
		}
	}
}



void Sloxy::interceptActivity(int port)
{
	int maxConnectQueue = 5;

	struct sockaddr_in hostInternetAddress;

	string messageBuilder;
	string partialMessage;
	string header;
	string str;

	char fromClientBuffer[10000];
	char toClientBuffer[10000];

	char fromHostBuffer[10000];
	char toHostBuffer[10000];

	char headRequest[10000];
	char rangeRequest[10000];

	int bytesRcvdFromClient = -1;
	int bytesSentToClient = -1;
	int bytesRcvdFromHost = -1;
	int bytesSentToHost = -1;
	int headerLength = -1;
	int errsv = 0;
	int contentLength = -1;
	int bytesReceived;

	int totalMsgLength;
	int totalReceived;

	int rangeSize = 100;
	int bytesToGet = -1;
	int msgLen = -1;

	int httpResponseCode = -1;


	// ========== INIT SERVER, ACCEPT A CONNECTION, RECEIVE A REQUEST ==========
	cout << "Initializing Server.\n";
	server.listenForClients(port, maxConnectQueue);
	server.acceptClientConnection();
	close(server.getListenerSocketID());
	cout << endl;

	

	receiveMessage(server.getWebClientSocketID(), fromClientBuffer, bytesRcvdFromClient);

	cout << "Server received a message of length: " << bytesRcvdFromClient << endl;
	cout << "---------- START MESSAGE ----------\n";
	for (int i = 0; i < bytesRcvdFromClient; i++)
		cout << fromClientBuffer[i];
	cout << "----------- END MESSAGE -----------\n";

	// ========== GET WEB SERVER ADDRESS FROM REQUEST ==========
	char * hostName = new char[200];
	int hostNameLen = 0;
	int webServerPort;

	cout << "\nGathering host information from request by client.\n";

	getHostInfoFromRequest(fromClientBuffer, hostName, hostNameLen, webServerPort);
	string hostStr(hostName);

	// If host has not been connected to by an existing representative client
	if (connectedHosts.find(hostStr) == connectedHosts.end() || true)
	{
		buildHostInetAddr(hostStr, webServerPort, hostInternetAddress);

		cout << "\nConnecting to host.\n";

		client.connectWithHost(hostInternetAddress);
	}
	else
	{
		// a client has already connected to this host.
		// do multithreading here.
	}

	// Remove conditions from GET requests
	str = removeLineStartingWith(fromClientBuffer, "If-Modified-Since:");
	str = removeLineStartingWith(str.c_str(), "If-None-Match:");

	memcpy(headRequest, str.c_str(), str.length() + 1);
	memcpy(toHostBuffer, str.c_str(), str.length() + 1);

	// Send head request to web host.
	fromGetToHead(headRequest, msgLen);
	sendMessage(client.getWebHostSocketID(), headRequest, msgLen, bytesSentToClient);
	cout << "Head request of length " << bytesSentToClient << " sent to web server.\n";

	// Receive reply to head request.
	memset(fromHostBuffer, '\0', 10000);
	receiveMessage(client.getWebHostSocketID(), fromHostBuffer, bytesRcvdFromHost);
	cout << "The header that the web server replied with had length: " << bytesRcvdFromHost << endl;

	contentLength = getContentLength(fromHostBuffer);
	totalMsgLength = bytesRcvdFromHost + contentLength;
	cout << "The header size plus the content length should be: " << totalMsgLength << endl;

	headerLength = bytesRcvdFromHost;
	header.assign(fromHostBuffer, headerLength);
	cout << "\n\nHeader:\n\n" << header;

	// Process response to head request and determine course of action.
	httpResponseCode = getHttpResponseCode(fromHostBuffer);
	cout << "HTTP response: " << httpResponseCode << endl;

	if (!isHtml(str.c_str()) || httpResponseCode == 404 || !acceptsRanges(fromHostBuffer))
	{
		cout << "Either file was not html, file was not found, or host doesn't accept range requests.\n";

		close(client.getWebHostSocketID());
		client.connectWithHost(hostInternetAddress);

		memcpy(toHostBuffer, str.c_str(), str.length() + 1);

		sendMessage(client.getWebHostSocketID(), toHostBuffer, str.length() + 1, bytesSentToHost);
		receiveMessage(client.getWebHostSocketID(), fromHostBuffer, bytesRcvdFromHost);

		string s(fromHostBuffer);
		cout << "TEST: \n" << s << endl;

		sendMessage(server.getWebClientSocketID(), fromHostBuffer, bytesRcvdFromHost, bytesSentToClient);
	}
	// For 301/302, probably want to resend head request with new url until a 200 is returned.
	else if (httpResponseCode == 301)
	{
		string getRedir = "GET /index.php HTTP/1.1\r\nHost: www.example.org\r\n\r\n";
		string redirResp = "HTTP/1.1 301 Moved Permanently\r\nLocation: http://www.example.org/index.asp\r\n\r\n";

		string newLocation = getLocation(redirResp.c_str());
		cout << "New location url: " << newLocation << endl;
		str = replaceUrl(getRedir.c_str(), newLocation);
		cout << "New Get: \n" << str << endl;

		//string newLocation = getLocation(fromHostBuffer);
		//cout << "New location url: " << newLocation << endl;

		//str = replaceUrl(str.c_str(), newLocation);
		//memcpy(toHostBuffer, str.c_str(), str.length() + 1);
	}
	else if (httpResponseCode == 302)
	{
		string newLocation = getLocation(fromHostBuffer);
		cout << "New location url: " << newLocation << endl;

		str = replaceUrl(str.c_str(), newLocation);
		memcpy(toHostBuffer, str.c_str(), str.length() + 1);
	}
	else
	{
		messageBuilder.clear();
		cout << "Web host accepts range requests.\n";
		totalReceived = 0;
		bytesToGet = rangeSize;

		while (totalReceived < contentLength)
		{
			if ((contentLength - totalReceived) < rangeSize)
				bytesToGet = contentLength - totalReceived;

			memcpy(rangeRequest, toHostBuffer, bytesRcvdFromClient + 1);
			fromGetToRangeGet(rangeRequest, totalReceived, bytesToGet, msgLen);

			//cout << "\nSending message to web host of length: " << msgLen << endl;
			//cout << "---------- START MESSAGE ----------\n";
			//for (int i = 0; i < msgLen; i++)
			//	cout << rangeRequest[i];
			//cout << "----------- END MESSAGE -----------\n";

			close(client.getWebHostSocketID());
			client.connectWithHost(hostInternetAddress);
			sendMessage(client.getWebHostSocketID(), rangeRequest, msgLen, bytesSentToHost);

			if (bytesSentToHost != msgLen)
				cout << "Full message not sent to web server.\n";

			memset(fromHostBuffer, '\0', 10000);
			receiveMessage(client.getWebHostSocketID(), fromHostBuffer, bytesRcvdFromHost);

			//cout << "\nReceived message from web host of length: " << bytesRcvdFromHost << endl;
			//cout << "---------- START MESSAGE ----------\n";
			//for (int i = 0; i < bytesRcvdFromHost; i++)
			//	cout << fromHostBuffer[i];
			//cout << "----------- END MESSAGE -----------\n";

			cout << "\nBytes to Get: " << bytesToGet << endl;

			partialMessage = fromHostBuffer;
			partialMessage = partialMessage.substr(partialMessage.length() - bytesToGet - 1, bytesToGet);
			messageBuilder += partialMessage;

			cout << "\n\n\n\nPartial Message:\n\n" << partialMessage << endl << endl;

			bytesReceived = getContentLength(fromHostBuffer);
			totalReceived += bytesReceived;

			cout << "\nTotal received: " << totalReceived << endl;
			cout << "Message so far: \n" << messageBuilder << endl;
			
			usleep(100000);
		}

		cout << "Full message received.\n";

		messageBuilder = header + messageBuilder;

		memcpy(toClientBuffer, messageBuilder.c_str(), totalMsgLength);
		sendMessage(server.getWebClientSocketID(), toClientBuffer, totalMsgLength, bytesSentToClient);
	}

	close(server.getWebClientSocketID());
	close(client.getWebHostSocketID());

	return;
}
