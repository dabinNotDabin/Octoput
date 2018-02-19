#pragma once


#include <queue>
#include <vector>
#include "global.h"

using namespace std;

class TaskQueue
{
private:
	bool workFinished;
	unsigned short numThreads;
	unsigned short currentOctoblock;

	queue<uint8_t> octolegQueue;

	pthread_mutex_t queueMutex;
	pthread_cond_t queueEmpty;
	pthread_cond_t octoblocked;

public:

	TaskQueue();

	~TaskQueue();

	// Returns -1 if input read loop set workFinished to true and queue is empty or
	// if the queue is empty but the empty condition variable was signaled.
	// This handles the case where threads were waiting for the signal prior to
	// the workFinished variable being set.  
	uint8_t getTask(unsigned short requestedOctoblock);


	// Controls pushing regular tasks into the queue to be processed, employing
	// thread safe strategies to avoid race conditions.
	void putTask(uint8_t n);



	// Signals the condition variable when workFinished set to true to release
	// any threads that were waiting for a task to be added to the queue.
	// Doesn't matter which tasks threads were waiting on (getTask or getBigTask)
	void setWorkFinished(bool finished);


	void nextOctoblock();
};
