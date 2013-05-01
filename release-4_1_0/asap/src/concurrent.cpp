//
// C++ Interface: concurrent
//
// Description:
//
//
// Author: Kohji Nakamura <k.nakamura@nao.ac.jp>, (C) 2012
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <stdio.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include "concurrent.h"

//#define LOG(x) do {}while(false)
#define LOG(x) fprintf(stderr, "Error: %d\n", (x))

namespace concurrent {
/* ======================= Mutex ======================= */
Mutex::Mutex() throw(int)
{
	int result = pthread_mutex_init(&mutex, NULL);
	if (result != 0) {
		LOG(result);
		throw result;
	}
}

Mutex::~Mutex()
{
	int result = pthread_mutex_destroy(&mutex);
	if (result != 0) {
		LOG(result);
	}
}

void Mutex::lock() throw(int)
{
	int result = pthread_mutex_lock(&mutex);
	if (result != 0) {
		LOG(result);
		throw result;
	}
}

bool Mutex::try_lock() throw(int)
{
	int result = pthread_mutex_trylock(&mutex);
	if (result == 0) {
		return true;
	}
	if (result == EBUSY) {
		return false;
	}
	LOG(result);
	throw result;
}

void Mutex::unlock() throw(int)
{
	int result = pthread_mutex_unlock(&mutex);
	if (result != 0) {
		LOG(result);
		throw result;
	}
}

/* ======================= Semaphore ======================= */
Semaphore::Semaphore(unsigned initial) throw(int)
{
	//mutex = PTHREAD_MUTEX_INITIALIZER;
	//cond = PTHREAD_COND_INITIALIZER;
	sem = initial;

	int result = pthread_mutex_init(&mutex, NULL);
	if (result != 0) {
		LOG(result);
		throw result;
	}

	result = pthread_cond_init(&cond, NULL);
	if (result != 0) {
		pthread_mutex_destroy(&mutex);
		LOG(result);
		throw result;
	}
}

Semaphore::~Semaphore()
{
	int result = pthread_mutex_destroy(&mutex);
	result = pthread_cond_destroy(&cond);
}

void Semaphore::up(unsigned amount) throw(int)
{
	assert(0 < amount && amount <= UINT_MAX - sem);
	int result = pthread_mutex_lock(&mutex);
	if (result == 0) {
		sem += amount;
		result = pthread_cond_signal(&cond);
		int result2 = pthread_mutex_unlock(&mutex);
		if (result == 0 && result2 == 0) {
			return;
		}
		if (result != 0 && result2 != 0) {
			LOG(result);
			result = 0;
		}
		if (result == 0) {
			result = result2;
		}
	}
	LOG(result);
	throw result;
}

void Semaphore::down(unsigned amount) throw(int)
{
	assert(0 < amount);
	int result = pthread_mutex_lock(&mutex);
	if (result == 0) {
		while (sem < amount) {
			result = pthread_cond_wait(&cond, &mutex);
			if (result != 0) {
				LOG(result);
				break;
			}
		}
		if (sem >= amount) {
			sem -= amount;
		}
		int result2 = pthread_mutex_unlock(&mutex);
		if (result == 0 && result2 == 0) {
			return;
		}
		if (result != 0 && result2 != 0) {
			LOG(result);
			result = 0;
		}
		if (result == 0) {
			result = result2;
		}
	}
	LOG(result);
	throw result;
}

/* ======================= Broker ======================= */
Broker::Broker(bool (*producer)(void *context) throw(PCException),
			   void (*consumer)(void *context) throw(PCException))
{
	this->producer = producer;
	this->consumer = consumer;
}

Broker::~Broker()
{
}

void Broker::enableNested()
{
#ifdef _OPENMP
	omp_set_nested(1);
#endif
}

void Broker::disableNested()
{
#ifdef _OPENMP
	omp_set_nested(0);
#endif
}

void Broker::setNestedState(bool nested)
{
#ifdef _OPENMP
	omp_set_nested(static_cast<int>(nested));
#endif
}

bool Broker::getNestedState()
{
#ifdef _OPENMP
	return omp_get_nested() ? true : false;
#else
	return false;
#endif
}

void Broker::runProducerAsMasterThread(void *context, unsigned do_ahead)
	throw(PCException)
{
	_run(context, do_ahead, ProdAsMaster);
}

void Broker::runConsumerAsMasterThread(void *context, unsigned do_ahead)
	throw(PCException)
{
	_run(context, do_ahead, ConsAsMaster);
}

void Broker::run(void *context, unsigned do_ahead) throw(PCException)
{
	_run(context, do_ahead, Unspecified);
}

void Broker::_run(void *context, unsigned do_ahead, ThreadSpec threadSpec)
	throw(PCException)
{
	assert(do_ahead > 0);
#ifdef _OPENMP
	PCException *prodEx = NULL;
	PCException *consEx = NULL;
	unsigned queuedJobs = 0;
	int consumerTerminated = 0;
	Semaphore semaphore_for_consumer;
	Semaphore semaphore_for_producer(do_ahead);

	#pragma omp parallel num_threads(2) \
		shared(semaphore_for_consumer, semaphore_for_producer, \
			   consumerTerminated, queuedJobs)
	{
		//fprintf(stderr, "run: %p %d\n", context, omp_get_thread_num());
		bool runProd = true;
		if (threadSpec == Unspecified) {
			#pragma omp single
			{
				runProd = false;
			}
		} else {
			bool isMaster = false;
			#pragma omp master
			{
				isMaster = true;
			}
			if (threadSpec == ProdAsMaster) {
				runProd = isMaster;
			} else { // ConsAsMaster
				runProd = ! isMaster;
			}
		}

		if (runProd) { // producer
			for (;;) {
				semaphore_for_producer.down();
				int consumerDead = 0;
				#pragma omp atomic
				consumerDead += consumerTerminated;
				if (consumerDead) {
					break;
				}
				try {
					bool produced = producer(context);
					if (! produced) {
						break;
					}
				} catch (PCException &e) {
					prodEx = &e;
					break;
				}
				#pragma omp atomic
				queuedJobs++;
				semaphore_for_consumer.up();
			}
			// additional 'up' to give consumer a chance to terminate.
			semaphore_for_consumer.up();
		} else { // consumer
			for (;;) {
				semaphore_for_consumer.down();
				unsigned remainingJobs = 0U;
				#pragma omp atomic
				remainingJobs += queuedJobs;
				if (remainingJobs == 0U) {
					break;
				}
				#pragma omp atomic
				queuedJobs--;
				try {
					consumer(context);
				} catch (PCException &e) {
					consEx = &e;
					break;
				}
				semaphore_for_producer.up();
			}
			#pragma omp atomic
			consumerTerminated++;
			// additional 'up' to give producer a chance to terminate.
			semaphore_for_producer.up();
		}
	}
	if (prodEx) {
		prodEx->raise();
	} else if (consEx) {
		consEx->raise();
	}
#else
	runSequential(context);
#endif
}

void Broker::runSequential(void *context) throw(PCException)
{
	for (;;) {
		bool produced = producer(context);
		if (! produced) {
			break;
		}
		consumer(context);
	}
}

} // namespace
