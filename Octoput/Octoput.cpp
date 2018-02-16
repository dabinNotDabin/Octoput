

#include "global.h"
#include "UDPClient.h"
#include "UDPServer.h"



struct OctoMonocto
{
	short numFullOctoblocks;
	short partialOctoblockSize;
	short partialOctolegSize;
	short leftoverDataSize;
};

void * worker(void * id)
{

	pthread_exit(0);
}

int main(int argc, char** argv)
{
	int port;


	int numFullOctoblocksNeeded;
	int octoblockSize = 8888;

	int partialOctoblockSize;
	int partialOctolegSize;
	int leftoverDataSize;

	std::string filename;

	std::string *octoblocks;

	std::ifstream in;
	std::string fileContents;

	
	unsigned int serverPort = 12345;
	unsigned int clientPort = 54321;



	if (argc > 1 && argc < 4)
	{
		// No port specified
		if (argc == 2)
		{
			port = 12345;
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



		// By here we have: 
		//	N Octoblocks of size 8888 in octoblocks[0] through octoblocks[numFullOctoblocksNeeded - 1]
		//	An Octoblock of size "partialOctoblockSize" in octoblocks[numFullOctoblocksNeeded]   ***if partialOctoblockSize > 0
		//	An Octoblock of size "leftoverDataSize" in octoblocks[numFullOctoblocksNeeded + 1]   ***if leftoverDataSize > 0

		OctoMonocto octoMonocto;
		octoMonocto =
		{
			(short)numFullOctoblocksNeeded,
			(short)partialOctoblockSize,
			(short)partialOctolegSize,
			(short)leftoverDataSize
		};



		UDPServer server(AF_INET, SOCK_DGRAM, IPPROTO_UDP, serverPort, clientPort);
		UDPClient client(AF_INET, SOCK_DGRAM, IPPROTO_UDP, clientPort, serverPort);


		

		string octoDesripto =
			"Number Of Full Octoblocks: " + to_string(octoMonocto.numFullOctoblocks) + "\r\n" +
			"Size Of Partial Octoblock: " + to_string(octoMonocto.partialOctoblockSize) + "\r\n" +
			"Size Of Partial Octolegs: " + to_string(octoMonocto.partialOctolegSize) + "\r\n" +
			"Size Of Leftover Data: " + to_string(octoMonocto.leftoverDataSize) + "\r\n";

		unsigned short checksum;

		string testDestIP = "192.168.0.30";
		clientPort = 10;
		checksum = server.computeChecksum(octoDesripto.c_str(), testDestIP.c_str(), clientPort);

		




		// Begin sending octolegs through to the client socket.
		//	Initiate by sending a file information packet that tells the receiver what to expect for data
		//		Send first before any threading takes place.
		//	Construct an octoleg header with octoleg size and sequence number tag, identifying it within it's octoblock
		//	Send octolegs, possibly using threads.
		//		Need 8 threads since only one octoblock can be sent at a time.
		//		At the beginning of each octoblock, a counter is set to 0.
		//			When it reaches 7, we know all octolegs have been sent.
		//			At 7, the counter is reset and threads must be reinitialized for new octoblock.
		//		Each octoleg is sent by a thread.
		//			A timer is set to 0 on send and the thread successfully exits when an ack is received.
		//			If the timer reaches some threshold, the octoleg is resent.
		//			Each thread is passed in an identifier indicating which octoleg is is responsible for.
		//			When an ACK is received, the thread can exit if the ACK's checksum is valid.
		//			Each thread increments the counter by 1 before exiting.

		//	Receive octolegs, possibly using threads.
		//		Need 8 threads since only one octoblock can be sent at a time.
		//		At the beginning of each octoblock, a counter is set to 0.
		//			When it reaches 7, we know all octolegs have been received.
		//			At 7, the counter is reset and threads must be reinitialized for new octoblock.
		//		Each octoleg is processed by a thread.
		//			Each thread is passed in an identifier indicating which octoleg is is responsible for.
		//			When an octoleg is successfully received, it's data can be stored in the buffer.
		//				position in the buffer will be determined by the octoleg #, octoblock #, and their size info.
		//			Each thread increments the counter by 1 before exiting.



		// The client side file buffer should be safe since each thread is accessing a separate portion of it's memory
		// On the client side, need an incoming octoblock buffer with 8 arrays, one for each octoleg.
		// The client receives data continuously and checks the first 8 bits for the octoleg flag
		//		It stores the data in the buffer associated with that ocotleg and sets a flag
		//		telling the thread that is processing that octoleg that data has arrived.
		//		The thread processes the segment, evaluating the checksum and sends ACK if valid.
		//			Condition variables would work well to signal that a piece of data has arrived.
		//
		// The server instantiates the global octoblock buffer that holds the data to be sent.
		//		The server threads each construct and send their own octoleg including header information
		//		and monitor for ACKS / deal with retransmissions.
		//		
		//		The ACKs are sent with checksums as well and these are evaluated by the threads
		//
		//		The server thus needs to maintain an ACK reception buffer where ACKs are placed according to the
		//		octoleg flag, with a flag that is raised letting the thread know to check the ACK.
		//			Condition variables would work well to wait on the incoming ACK


		//pthread_t *threads;
		//threads = new pthread_t[totalOctoblocksNeeded * 8];
		//long status;
		//long i;
		//for (i = 0; i < totalOctoblocksNeeded * 8; i++)
		//{
		//	status = pthread_create(&threads[i], NULL, worker, (void *)i);
		//	if (status != 0)
		//	{
		//		std::cout << "Creation of thread resulted in error.\n";
		//		exit(-1);
		//	}
		//}



		//for (i = 0; i < totalOctoblocksNeeded * 8; i++)
		//	pthread_join(threads[i], NULL);


		//delete[] threads;
		//delete[] octoblocks;
	}
	else
	{
		cout
			<< "Usage: \n"
			<< "\tOctoput <port#> <filename>\n"
			<< "\t\tOR\n"
			<< "\tOctoput <filename>\tport is default to 12345.\n";
	}

    return 0;
}

