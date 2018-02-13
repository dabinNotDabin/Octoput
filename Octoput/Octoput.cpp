// Octoput.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <fstream>


int main(int argc, char** argv)
{
	int port;


	int numFullOctoblocksNeeded;
	int octoblockSize = 8888;

	int partialOctoblockSize;
	int partialOctolegSize;
	int leftoverDataSize;

	string filename;

	string *octoblocks;

	ifstream in;
	string fileContents;

	if (argc > 1 && argc < 4)
	{
		// No port specified
		if (argc == 2)
		{
			port = 80;
			filename = argv[1];
		}
		else
		{
			port = atoi(argv[1]);
			filename = argv[2];
		}


		in.open(filename);
		getline(in, fileContents, '\0');
		in.close();


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


		int totalOctoblocksNeeded =
			numFullOctoblocksNeeded +
			((partialOctoblockSize > 0) ? 1 : 0) +
			((leftoverDataSize > 0) ? 1 : 0);

		octoblocks = new string[totalOctoblocksNeeded];


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



		// By here we have: 
		//	N Octoblocks of size 8888 in octoblocks[0] through octoblocks[numFullOctoblocksNeeded - 1]
		//	An Octoblock of size "partialOctoblockSize" in octoblocks[numFullOctoblocksNeeded]   ***if partialOctoblockSize > 0
		//	An Octoblock of size "leftoverDataSize" in octoblocks[numFullOctoblocksNeeded + 1]   ***if leftoverDataSize > 0

		// Instantiate a server socket.
		// Instantiate a client socket.
		// Begin sending octolegs through to the client socket.
		//	Initiate by sending a file information packet that tells the receiver what to expect for data
		//	Construct an octoleg header with octoleg size and sequence number tag, identifying it within it's octoblock
		//	Send octolegs, possibly using threads.
		//		Each octoleg is sent by a thread.
		//			A timer is set to 0 on send and the thread successfully exits when an ack is received.
		//			If the timer reaches some threshold, the octoleg is resent.


		pthread_t *threadArray;
		threadArray = new pthread_t[totalOctoblocksNeeded * 8];
		long status;
		long i;
		for (i = 0; i < totalOctoblocksNeeded * 8; i++)
		{
			status = pthread_create(&threadArray[i], NULL, isPrime, (void *)i);
			if (status != 0)
			{
				std::cout << "Creation of thread resulted in error.\n";
				exit(-1);
			}
		}

	}


	delete[] octoblocks;
	delete[] threadArray;

    return 0;
}

