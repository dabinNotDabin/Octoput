

#include "global.h"
#include "UDPClient.h"
#include "UDPServer.h"




int main(int argc, char** argv)
{
	unsigned int serverPort = 12345;
	unsigned int clientPort = 54321;


	string serverClientChoice;

	ifstream in;
	string fileContents;

	if (argc == 2)
	{

		serverClientChoice = string(argv[1]);


		while
			(
				(serverClientChoice.compare("server") != 0) &&
				(serverClientChoice.compare("client") != 0) &&
				(serverClientChoice.compare("quit") != 0) 
			)
		{
			cout
				<< "Command \"" << serverClientChoice << "\" is not valid.\n"
				<< "Enter \"server\" to run the server side or \"client\" to run the client side.\n"
				<< "Enter \"quit\" to exit.\n";

			cin >> serverClientChoice;
		}


		if (serverClientChoice.compare("server") == 0)
		{
			// If successful, the server owns a bound socket and is ready to send and receive
			UDPServer server(AF_INET, SOCK_DGRAM, IPPROTO_UDP, serverPort, clientPort);
			server.commenceOctovation();

			cout << "Exiting Server Program.\n";
		}
		else if (serverClientChoice.compare("client") == 0)
		{
			// If successful, the client owns a socket and is ready to send and receive
			UDPClient client(AF_INET, SOCK_DGRAM, IPPROTO_UDP, clientPort, serverPort);
			client.commenceOctovation();

			cout << "Exiting Client Program.\n";
		}
		else
		{
			return 0;
		}

		


		//unsigned short checksum;

		//string testDestIP = "192.168.0.30";
		//clientPort = 10;
		//checksum = computeChecksum("Hi", testDestIP.c_str(), clientPort);


		

		//char mssg[1131]; //max octoleg size 1111 + 20 for udp header and pseudo header
		//int mssgLen;

		//mssgLen = octoDesripto.length()
		//memcpy(mssg, octoDesripto.c_str(), mssgLen);


		// UDP header needs src and dst port, octoleg flag, checksum 
		//	-- length is not needed since it is conveyed with octo descripto


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
			<< "\tOctoput <\"server\">\n"
			<< "\t\tOR\n"
			<< "\tOctoput <\"client\">\n";
	}

    return 0;
}





unsigned short oneComplementSum(unsigned int k)
{
	unsigned int l = 65535;

	while ((k >> 16) > 0)
	{
		k = (k & l) + (k >> 16);
	}

	return k;
}