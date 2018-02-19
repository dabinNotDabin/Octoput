#include "global.h"
#include "TaskQueue.h"
#include <queue>
#include <vector>

using namespace std;


	TaskQueue::TaskQueue()
	{
		workFinished = false;
		pthread_mutex_init(&queueMutex, NULL);
		pthread_cond_init(&queueEmpty, NULL);
	}

	TaskQueue::~TaskQueue()
	{
		pthread_mutex_destroy(&queueMutex);
		pthread_cond_destroy(&queueEmpty);
	}


	// Returns -1 if input read loop set workFinished to true and queue is empty or
	// if the queue is empty but the empty condition variable was signaled.
	// This handles the case where threads were waiting for the signal prior to
	// the workFinished variable being set.  
	uint8_t TaskQueue::getTask()
	{
		uint8_t n = -1;

		pthread_mutex_lock(&queueMutex);

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

