#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include "pthread.h"


#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h> 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <errno.h>

#include <netdb.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <map>



#define HEADER_SIZE_BYTES 8
#define ACK_SIZE_BYTES (HEADER_SIZE_BYTES + 1)

unsigned short oneComplementSum(unsigned int k);


struct OctoMonocto
{
	short totalFileSize;
	short numFullOctoblocks;
	short partialOctoblockSize;
	short partialOctolegSize;
	short leftoverDataSize;
};
