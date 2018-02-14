#pragma once
#include <vector>
#include <map>

#include "Socket.h"

using namespace std;


// In this implementation, a client will connect to one host.
// The sloxy has the responsibility of managing the various representative clients.
class UDPClient
{
public:
	UDPClient();
	~UDPClient();


private:
	Socket UDPSocket;
};