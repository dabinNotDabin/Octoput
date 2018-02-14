#pragma once

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h> 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <errno.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sstream>
#include <map>

#include "Client.h"
#include "Server.h"


using namespace std;


// The sloxy must instantiate a client and a server.
// Have the server wait for and accept a connection
// Receive messages from the client through the server.
// Parse the messages (are they html, who are they for).
// Send head requests to the host (through the representative client)
//		to determine if the intended host accepts range requests.
// If so, reformat the request from the actual client to perform
//		the range requests until the entire message is received.
// Send that message back to the actual client.

class Sloxy
{
public:
	Sloxy();
	~Sloxy();

	void getHostInfoFromRequest(char *request, char *hostName, int &hostNameLen, int &port);

	void interceptActivity(int port);

private:
	// Map to associate host names to therepresentative client object that is connected to them.
	map<string, UDPClient> connectedHosts;

	// A map to associate client get request messages with a host's reply to it's equivalent head request.
	map<string, string> cachedRequests;

	// A server to intercept requests from the web browser.
	UDPServer server;
	UDPClient client;

	// A set of clients that act as representative for the actual client of the server.
	vector<UDPClient> representativeClients;
	
	// Uses an host name and port to build a host internet address.
	void buildHostInetAddr(string hostName, int port, struct sockaddr_in &hostInetAddress);

	// Receives a message from the socket identified by <socketID> and stores the result in
	// <message> and the length of the message in <msgLength>.
	void receiveMessage(int socketID, char message[], int &msgLength);
	void sendMessage(int socketID, char message[], int msgLength, int &bytesSent);

	bool isHtml(const char *httpRequest);
	void fromGetToHead(char *httpRequest, int &msgLength);
	void fromGetToRangeGet(char *httpRequest, int startPos, int numBytes, int &msgLength);
	int getContentLength(char *httpResponse);
	bool acceptsRanges(char *httpResponse);
	int getHttpResponseCode(char *httpResponse);
	string getFirstLine(char *sequence);
	string removeLineStartingWith(const char *sequence, string lineBeginning);
	bool hasLocationTag(const char *httpResponse);
	string getLocation(const char *httpResponse);
	string replaceUrl(const char *httpRequest, string newUrl);
};

