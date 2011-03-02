/*
 * AsynchronousTools.h
 *
 *  Created on: Nov 1, 2010
 *      Author: jjacobs
 */

#ifndef ASYNCHRONOUSTOOLS_H_
#define ASYNCHRONOUSTOOLS_H_

#include <casa/aips.h>
#include <casa/aipstype.h>
#include <casa/BasicSL/String.h>

#include <map>
#include <queue>

using std::map;
using std::queue;

//using namespace casa;

namespace casa {

namespace async {

class MutexImpl;

class Mutex {

    friend class Condition;

public:

    Mutex ();
    virtual ~Mutex ();

    void lock ();
    //Bool lock (Int milliseconds);
    void unlock ();
    Bool trylock ();

protected:

    pthread_mutex_t * getRep ();

private:

    MutexImpl * impl_p;

    Mutex (const Mutex & other); // illegal operation: do not define
    Mutex operator= (const Mutex & other); // illegal operation: do not define

};

class MutexLocker {

public:

    MutexLocker (Mutex & mutex);
    virtual ~MutexLocker ();

private:

    Mutex & mutex_p;

    MutexLocker (const MutexLocker & other); // illegal operation: do not define
    MutexLocker operator= (const MutexLocker & other); // illegal operation: do not define

};

class ConditionImpl;

class Condition {

public:

    Condition ();
    virtual ~Condition ();

    void broadcast ();
    void signal ();
    void wait (Mutex & mutex);
    // Bool wait (Mutex & mutex, int milliseconds);

private:

    ConditionImpl * impl_p;
};

class SemaphoreImpl;

class Semaphore {

public:

    Semaphore (int initialValue = 0);
    ~Semaphore ();

    Int getValue ();
    void post ();
    Bool trywait ();
    void wait ();
    Bool wait (int milliseconds);

private:

    SemaphoreImpl * impl_p;
    String name_p;

    Semaphore (const Semaphore & other); // illegal operation: do not define
    Semaphore operator= (const Semaphore & other); // illegal operation: do not define

};

class Thread {

public:

    typedef void * (* ThreadFunction) (void *);

    Thread ();
    virtual ~Thread ();

    pthread_t getId () const;
    void * join ();
    void startThread ();
    virtual void terminate ();
    bool terminationRequested () const;

protected:

    bool isStarted () const;
    virtual void * run () = 0;

    static void * threadFunction (void *);

private:

    pthread_t * id_p;
    bool started_p;
    bool terminationRequested_p;

};

class Logger : public Thread {

public:

    static void log (const char * format, ...);
    static void registerName (const String & threadName);
    static void start (const char * logFilename);

protected:

    class LoggerThread : public Thread {
    public:

        LoggerThread ();
        ~LoggerThread ();

        void log (char * text);
        void setLogFilename (const String & filename);

    protected:

        void * run ();

    private:

        Bool      deleteStream_p;
        Semaphore drainSemaphore_p;
        String    logFilename_p;
        ostream * logStream_p;
        Mutex mutex_p;
        queue<char *> outputQueue_p;
    };

    static LoggerThread * get();

//    void log (char * format, ...);
//    void setLogFilename (char * logFilename);
//    void terminate ();

private:

    typedef map <pthread_t, String>  ThreadNames;

    static bool loggingStarted_p;
    static Mutex * nameMutex_p;
    static ThreadNames threadNames_p;
};


}

}


#endif /* ASYNCHRONOUSTOOLS_H_ */
