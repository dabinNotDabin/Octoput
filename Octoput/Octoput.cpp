

#include "global.h"
#include "UDPClient.h"
#include "UDPServer.h"





void * worker(void * id)
{

	pthread_exit(0);
}

int main(int argc, char** argv)
{
	unsigned int serverPort = 12345;
	unsigned int clientPort = 54321;


	string serverClientChoice;

	ifstream in;
	string fileContents;


	//if (argc > 2 && argc < 4)
	//{
	//	// Change so client sends file request.
	//	//		Server port and Client port are taken as command line args.
	//	if (argc == 2)
	//	{
	//		
	//		filename = argv[1];
	//	}
	//	else
	//	{
	//		port = atoi(argv[1]);
	//		filename = argv[2];
	//	}


	//	in.open(filename);
	//	getline(in, fileContents, '\0');
	//	in.close();


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





unsigned short computeChecksum(const char* data, const char* destinationIP, unsigned int clientPort)
{
	struct sockaddr_in serverAddress;
	char* sourceIP;
	string pseudoHeaderIPs;
	string udpPacketData;
	string worker;
	unsigned short i;
	unsigned short j;
	unsigned short checksum;
	unsigned int sum;
	char* byte;

	//	UDPSocket->getAddress(serverAddress);

	inet_pton(AF_INET, "192.168.0.31", &serverAddress.sin_addr);
	sourceIP = inet_ntoa(serverAddress.sin_addr);

	pseudoHeaderIPs = string(sourceIP + string(".") + string(destinationIP));
	cout << "SourceIP: " << pseudoHeaderIPs << endl;


	// Begin Source IP
	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
	i = atoi(worker.c_str());
	//	cout << i << endl;
	pseudoHeaderIPs = pseudoHeaderIPs.substr(worker.length() + 1);
	//	cout << "Pseudo: " << pseudoHeaderIPs << endl;
	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
	//	cout << atoi(worker.c_str()) << endl;
	i = (i << 8) | atoi(worker.c_str());
	cout << "I: " << i << endl;
	pseudoHeaderIPs = pseudoHeaderIPs.substr(worker.length() + 1);
	//	cout << "Pseudo: " << pseudoHeaderIPs << endl;


	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
	j = atoi(worker.c_str());
	//	cout << j << endl;
	pseudoHeaderIPs = pseudoHeaderIPs.substr(worker.length() + 1);
	//	cout << "Pseudo: " << pseudoHeaderIPs << endl;
	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
	//	cout << atoi(worker.c_str()) << endl;
	j = (j << 8) | atoi(worker.c_str());
	cout << "J: " << j << endl;
	pseudoHeaderIPs = pseudoHeaderIPs.substr(worker.length() + 1);
	//	cout << "Pseudo: " << pseudoHeaderIPs << endl;


	sum = i + j;
	cout << "SUM: " << sum << endl;


	// Begin Dest IP
	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
	i = atoi(worker.c_str());
	//	cout << i << endl;
	pseudoHeaderIPs = pseudoHeaderIPs.substr(worker.length() + 1);
	//	cout << "Pseudo: " << pseudoHeaderIPs << endl;
	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
	//	cout << atoi(worker.c_str()) << endl;
	i = (i << 8) | atoi(worker.c_str());
	cout << "I: " << i << endl;
	pseudoHeaderIPs = pseudoHeaderIPs.substr(worker.length() + 1);
	//	cout << "Pseudo: " << pseudoHeaderIPs << endl;

	sum = sum + i;

	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
	i = atoi(worker.c_str());
	//	cout << i << endl;
	pseudoHeaderIPs = pseudoHeaderIPs.substr(worker.length() + 1);
	//	cout << "Pseudo: " << pseudoHeaderIPs << endl;
	worker = pseudoHeaderIPs.substr(0, pseudoHeaderIPs.find_first_of('.'));
	//	cout << atoi(worker.c_str()) << endl;
	i = (i << 8) | atoi(worker.c_str());
	cout << "I: " << i << endl;
	//	cout << "Pseudo: " << pseudoHeaderIPs << endl;

	sum = sum + i;
	// End Dest IP

	cout << "SUM: " << sum << endl;


	// Begin Protocol
	i = 17;
	sum = sum + i;
	cout << "SUM: " << sum << endl;
	// End Protocol


	// Begin UDP Length
	udpPacketData = string(data);
	i = 8 + udpPacketData.length();
	sum = sum + i;
	cout << "SUM: " << sum << endl;
	// End UDP Length


	// Begin Src Port
	i = serverAddress.sin_port;
	i = 20;
	sum = sum + i;
	cout << "SUM: " << sum << endl;
	// End Src Port

	// Begin Dst Port
	i = clientPort;
	sum = sum + i;
	cout << "SUM: " << sum << endl;
	// End Dst Port

	// Begin UDP Length
	udpPacketData = string(data);
	i = 8 + udpPacketData.length();
	sum = sum + i;
	cout << "SUM: " << sum << endl;
	// End UDP Length

	cout << "UDP Packet: " << udpPacketData << endl;

	cout << "UDP Packet Len: " << udpPacketData.length() << endl;

	char c;
	for (int x = 0; x < udpPacketData.length(); x += 2)
	{
		c = data[x];
		i = (unsigned short)c;
		//		cout << "I: " << i << endl;

		c = data[x + 1];
		//		cout << "I: " << (unsigned short)c << endl;
		i = (i << 8) | (unsigned short)c;
		//		cout << "I: " << i << endl;

		sum = sum + i;
		//		cout << "SUM: " << sum << endl;
	}


	checksum = oneComplementSum(sum);

	if (checksum != 65535)
		checksum = ~checksum;

	cout << "Checksum: " << checksum << endl;
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