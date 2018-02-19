#include "global.h"
#include "TaskQueue.h"
#include <queue>
#include <vector>

using namespace std;


	TaskQueue::TaskQueue()
	{
		workFinished = false;
		currentOctoblock = 0;
		pthread_mutex_init(&queueMutex, NULL);
		pthread_cond_init(&queueEmpty, NULL);
		pthread_cond_init(&octoblocked, NULL);
	}

	TaskQueue::~TaskQueue()
	{
		pthread_mutex_destroy(&queueMutex);
		pthread_cond_destroy(&queueEmpty);
		pthread_cond_destroy(&octoblocked);
	}

	
	// Returns 8 if input read loop set workFinished to true and queue is empty or
	// if the queue is empty but the empty condition variable was signaled.
	// This handles the case where threads were waiting for the signal prior to
	// the workFinished variable being set.  
	uint8_t TaskQueue::getTask(unsigned short requestedOctoblock)
	{
		uint8_t n = 8;

		// Since waiting unlocks the mutex, all threads can enter this waiting state.
		// Next octoblock will unblock all waiting threads and put them in a waiting state
		// to acquire the mutex when it becomes free in some order determined by the scheduler.
		pthread_mutex_lock(&queueMutex);
		while (requestedOctoblock > currentOctoblock)
		{
			pthread_cond_wait(&octoblocked, &queueMutex);
		}

		if (workFinished && octolegQueue.empty())
			n = -1;
		else
		{
			while (octolegQueue.empty())
			{
				if (workFinished)
					break;

				pthread_cond_wait(&queueEmpty, &queueMutex);
			}

			if (!octolegQueue.empty())
			{
				n = octolegQueue.front();
				octolegQueue.pop();
			}
		}
		pthread_mutex_unlock(&queueMutex);

		return n;
	}


	// Controls pushing regular tasks into the queue to be processed, employing
	// thread safe strategies to avoid race conditions.
	void TaskQueue::putTask(uint8_t n)
	{
		pthread_mutex_lock(&queueMutex);
		octolegQueue.push(n);
		pthread_cond_signal(&queueEmpty);
		pthread_mutex_unlock(&queueMutex);
	}



	// Signals the condition variable when workFinished set to true to release
	// any threads that were waiting for a task to be added to the queue.
	// Doesn't matter which tasks threads were waiting on (getTask or getBigTask)
	void TaskQueue::setWorkFinished(bool finished)
	{
		pthread_mutex_lock(&queueMutex);
		workFinished = finished;
		pthread_cond_broadcast(&queueEmpty);
		pthread_mutex_unlock(&queueMutex);
	}



	void TaskQueue::nextOctoblock()
	{
		pthread_mutex_lock(&queueMutex);
		currentOctoblock++;
		pthread_cond_broadcast(&octoblocked);
		pthread_mutex_unlock(&queueMutex);
	}

