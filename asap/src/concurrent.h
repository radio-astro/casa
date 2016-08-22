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
#ifndef CONCURRENT_H
#define CONCURRENT_H

#include <assert.h>
#include <pthread.h>

namespace concurrent {
	class PCException {
	public:
		virtual void raise() /*throw(PCException)*/ { throw *this; }
		virtual ~PCException() {}
	};

	//typedef void (*produce_t)(void *context) throw(PCException);
	//typedef void (*consume_t)(void *context) throw(PCException);

	class Mutex {
		pthread_mutex_t mutex;
	public:
		Mutex() throw(int);
		virtual ~Mutex();
		void lock() throw(int);
		/**
		 * Returns true if this thread could lock.
		 * Returns false if already locked.
		 */
		bool try_lock() throw(int);
		void unlock() throw(int);
	private:
		Mutex(Mutex const &other);
		Mutex &operator =(Mutex const &other);
	};

	class Semaphore {
		pthread_mutex_t mutex;
		pthread_cond_t cond;
		unsigned sem;
	public:
		explicit Semaphore(unsigned initial = 0U) throw(int);
		void up(unsigned amount = 1U) throw(int);
		void down(unsigned amount = 1U) throw(int);
		virtual ~Semaphore();
	private:
		Semaphore(Semaphore const &other);
		Semaphore &operator =(Semaphore const &other);
	};

	class FIFOException {
	public:
		virtual void raise() { throw *this; }
		virtual ~FIFOException() {}
	};

	class EmptyException {
	public:
		virtual void raise() { throw *this; }
		virtual ~EmptyException() {}
	};

	class FullException {
	public:
		virtual void raise() { throw *this; }
		virtual ~FullException() {}
	};

	template <class T, size_t N>
	class FIFO {
		T elements[N + 1]; // +1 to make an implementation simple.
		Mutex mutex;
		size_t head;
		size_t tail;

		size_t wrap(size_t n) {
			return n % (N + 1);
		}

		void reset() {
			head = tail = 0;
		}

	public:
		FIFO() {
			assert(N > 0);
			reset();
		}
		virtual ~FIFO() {
			mutex.lock(); // wait until current lock is released.
			mutex.unlock();
		}

		virtual void lock() {
			mutex.lock();
		}

		/**
		 * Returns true if this thread could lock.
		 * Returns false if already locked.
		 */
		virtual bool try_lock() {
			return mutex.try_lock();
		}

		virtual void unlock() {
			mutex.unlock();
		}

		virtual void clear() {
			reset();
		}

		virtual void put(T const &value) throw(FullException) {
			size_t new_tail = wrap(tail + 1);
			if (head == new_tail) {
				throw FullException();
			}
			elements[tail] = value;
			tail = new_tail;
		}

		virtual T get() throw(EmptyException) {
			if (head == tail) {
				throw EmptyException();
			}
			T result = elements[head];
			head = wrap(head + 1);
			return result;
		}

		/**
		 * Returns capacity size.
		 */
		virtual size_t size() const {
			return N;
		}

		/**
		 * Returns number of elements in this FIFO.
		 */
		virtual size_t length() const {
			size_t result = tail - head;
			if (head > tail) {
				result = N + 1 - (head - tail);
			}
			return result;
		}
	private:
		FIFO(FIFO const &other);
		FIFO &operator =(FIFO const &other);
	};

	class Broker {
	protected:
		enum ThreadSpec {
			ProdAsMaster, ConsAsMaster, Unspecified
		};
		bool (*producer)(void *context) throw(PCException);
		void (*consumer)(void *context) throw(PCException);
		virtual void _run(void *context, unsigned do_ahead, ThreadSpec threadSpec) throw(PCException);
	public:
		Broker(bool (*producer)(void *context) throw(PCException),
			   void (*consumer)(void *context) throw(PCException));
		virtual ~Broker();
		static void enableNested();
		static void disableNested();
		static void setNestedState(bool nested);
		static bool getNestedState();

		virtual void run(void *context, unsigned do_ahead=1) throw(PCException);
		virtual void runProducerAsMasterThread(void *context, unsigned do_ahead=1) throw(PCException);
		virtual void runConsumerAsMasterThread(void *context, unsigned do_ahead=1) throw(PCException);
		virtual void runSequential(void *context) throw(PCException);
	};

#if 0
	template <class Context, class Product>
		class Producer {
	public:
		virtual Product produce(Context &ctx) throw(PCException) = 0;
		virtual ~Producer() {}
	};

	template <class Context, class Product>
		class Consumer {
	public:
		virtual void consume(Context &ctx, Product const&product) throw(PCException) = 0;
		virtual ~Consumer() {}
	};

class ProdCons {
 public:
	virtual ~ProdCons() {}

	virtual void runProducerAsMasterThread(void *context) throw(PCException) = 0;
	virtual void runConsumerAsMasterThread(void *context) throw(PCException) = 0;
	virtual void produce(void *context) throw(PCException) = 0;
	virtual void consume(void *context) throw(PCException) = 0;

	/**
	 * <src>produce()</src> should  call this method to
	 * let consumer to know it is ready.
	 */
	virtual void ready() = 0;

	/**
	 * Returns true if ready,
	 * otherwise, finished or there was an error, returns false.
	 *
	 * <src>consume()</src> should call this method
	 * to check if it is ready or not.
	 * If false is returned, <src>isError()</src> or/and
	 * <src>isFinished()</src> should be called to see the reason.
	 */
	virtual bool waitForReady() = 0;

	/**
	 * Returns true if <src>produce()</src> was finished,
	 * otherwise returns false.
	 */
	virtual bool isFinished() = 0;

	/**
	 * <src>produce()</src>/<src>consume()</src> should call this method
	 * to report error to <src>consume()</src>/<src>produce()</src>.
	 */
	virtual void reportError(void *errorInfo) = 0;

	/**
	 * Returns true and set errorInfo if <src>reportError()</src> is called,
	 * otherwise returns false and set errorInfo to NULL.
	 */
	virtual bool isError(void &*errorInfo) = 0;
};
#endif

}


#endif
