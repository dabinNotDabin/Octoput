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
#include "fcntl.h"

#define N_OCTOLEGS_PER_OCTOBLOCK 8
#define N_THREADS 8
#define HEADER_SIZE_BYTES 8
#define ACK_SIZE_BYTES (HEADER_SIZE_BYTES + 1)

unsigned short oneComplementSum(unsigned int k);


typedef void* (*THREADFUNCPTR)(void*);


struct OctoMonocto
{
	short totalFileSize;
	short numFullOctoblocks;
	short partialOctoblockSize;
	short partialOctolegSize;
	short leftoverDataSize;
};
