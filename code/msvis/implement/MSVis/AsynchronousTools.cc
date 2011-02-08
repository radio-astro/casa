/*
 * AsynchronousTools.cc
 *
 *  Created on: Nov 1, 2010
 *      Author: jjacobs
 */

#include <assert.h>
#include <cstdarg>
#include <errno.h>
#include <fstream>
#include <pthread.h>
#include <queue>
#include <semaphore.h>
#include <time.h>
#include <sys/time.h>

#include <casa/Exceptions/Error.h>

#include "AsynchronousTools.h"
#include "UtilJ.h"

using namespace std;
using namespace casa::utilj;

namespace casa {

namespace async {

class ConditionImpl {

    friend class Condition;

private:

    ConditionImpl () : condition_p (NULL) {}

    pthread_cond_t * condition_p;
};

class MutexImpl {

    friend class Mutex;

private:

    MutexImpl () : mutex_p (NULL) {}

    pthread_mutex_t * mutex_p;

};

class SemaphoreImpl {

    friend class Semaphore;

private:

    SemaphoreImpl () : semaphore_p (NULL) {}

    sem_t * semaphore_p; // [use]
};

struct timespec
convertMsDeltaToTimespec (Int milliseconds)
{
    // Get the time with a possible accuracy to microseconds and
    // then convert it into the timeout's data structure used by
    // pthreads

    struct timeval tVal;
    gettimeofday (& tVal, NULL);

    struct timespec t;
    t.tv_sec = tVal.tv_sec;
    t.tv_nsec = tVal.tv_usec * 1000;

    // Add the wait time in milliseconds to this structure
    // taking care to handle carry out and avoid overflow

    t.tv_sec += milliseconds / 1000;
        // extract seconds and add them in

    t.tv_nsec += (milliseconds % 1000) * 1000000L;
        // extract ms, convert to ns and add in

    t.tv_sec += t.tv_nsec / 1000000000L;
        // carry out of the ns field into seconds

    t.tv_nsec %= 1000000000L;
        // remove any seconds carried out of ns field

    return t;
}


Condition::Condition ()
{
    impl_p = new ConditionImpl ();
    impl_p->condition_p = new pthread_cond_t ();
    int code = pthread_cond_init (impl_p->condition_p, NULL);
    ThrowIfError (code, "Condition::init");
}

Condition::~Condition ()
{
    int code = pthread_cond_destroy (impl_p->condition_p);
    ThrowIfError (code, "Condition::destroy");

    delete impl_p->condition_p;

    delete impl_p;
}

void
Condition::broadcast ()
{
    int code = pthread_cond_broadcast (impl_p->condition_p);
    ThrowIfError (code, "Condition::broadcast");
}

void
Condition::signal ()
{
    int code = pthread_cond_signal (impl_p->condition_p);
    ThrowIfError (code, "Condition::signal");
}

void
Condition::wait (Mutex & mutex)
{
    int code = pthread_cond_wait (impl_p->condition_p, mutex.getRep());
    ThrowIfError (code, "Condition::wait");
}
/*
Bool
Condition::wait (Mutex & mutex, int milliseconds)
{
    Assert (milliseconds >= 0); // weird if it's negative

    struct timespec t = convertMsDeltaToTimespec (milliseconds);
    int code = pthread_cond_timedwait (impl_p->condition_p, mutex.getRep(), & t);

    bool gotWait = true;
    if (code == ETIMEDOUT){
        gotWait = false;
    }
    else{
        ThrowIfError (code, format ("Condition::wait (%d)", milliseconds));
    }

    return gotWait;
}
*/

Bool Logger::loggingStarted_p = False;
Mutex * Logger::nameMutex_p = NULL;
map <pthread_t, String> Logger::threadNames_p;

Logger::LoggerThread *
Logger::get()
{
    static LoggerThread * singleton = NULL;

    if (singleton == NULL){
        singleton = new LoggerThread ();
    }

    return singleton;
}

void
Logger::log (const char * format, ...)
{
    va_list vaList;
    va_start (vaList, format);

    char buffer[4096];

    // Create the text to be logged

    vsnprintf (buffer, sizeof (buffer), format, vaList);

    // Grab the timestamp and pid (for IDing threads)

    String threadNameText;

    {
        MutexLocker ml (* nameMutex_p);

        pthread_t tid = pthread_self();
        ThreadNames::iterator threadName = threadNames_p.find (tid);
        if (threadName != threadNames_p.end ()){
            threadNameText = String (" [") + (threadName->second) + "] : ";
        }
        else{
            threadNameText = utilj::format (" [0x%08x] : ", tid);
        }
    }

    String prefix = utilj::getTimestamp() + threadNameText;

    // Allocate a buffer to put into the queue

    char * outputText = new char [strlen(buffer) + prefix.size() + 1];

    // Fill in the timestamp, delimiter and the log text

    strcpy (outputText, prefix.c_str());
    strcat (outputText, buffer);

    va_end (vaList);

    // Lock the queue, push on the block of text and increment
    // the drain semaphore

    get() -> log (outputText); // ownership passes to the thread

}

void
Logger::registerName (const String & threadName)
{
    MutexLocker ml (* nameMutex_p);

    threadNames_p [pthread_self()] = threadName;
}

void
Logger::start (const char * filename)
{
    nameMutex_p = new Mutex ();

    get()->setLogFilename (filename == NULL ? "" : filename);

    get()->startThread();

    loggingStarted_p = True;
}

Logger::LoggerThread::LoggerThread ()
{}

Logger::LoggerThread::~LoggerThread ()
{
    terminate();

    this->drainSemaphore_p.post(); // wake up stalled thread so it can quit

    this->join();

    if (deleteStream_p)
    {
        dynamic_cast<ofstream *> (logStream_p)->close();
        delete logStream_p;
    }
}

void
Logger::LoggerThread::log (char * text)
{
    MutexLocker m (mutex_p);

    outputQueue_p.push (text);

    drainSemaphore_p.post ();
}

void *
Logger::LoggerThread::run ()
{
    // Determine where to write the logging info.  If nothing is specified or either "cerr" or
    // "stdout" are specified then use standard error.  If "cout" or "stdout" are specified then
    // use standard out.  Otherwise open the specified file and write to that.

    if (logFilename_p.empty () || logFilename_p == "cerr" || logFilename_p == "stderr"){
        logStream_p = & cerr;
        deleteStream_p = False;
    }
    else if (logFilename_p == "cout" || logFilename_p == "stdout"){
        logStream_p = & cout;
        deleteStream_p = False;
    }
    else{
        logStream_p = new ofstream (logFilename_p.c_str(), ios::out);
        deleteStream_p = True;
    }

    * logStream_p << utilj::getTimestamp() << ": Logging started" << endl;

    // Loop waiting on the drain semaphore.  This should be incremented once
    // every time users add a block of text to the queue.

    while (True){

        if (! terminationRequested ())
            drainSemaphore_p.wait (); // one count per item on queue

        char * text;

        {
            // Pop the front block of output off of the queue
            // Keep mutex locked while accessing queue.

            MutexLocker locker (mutex_p);

            if (terminationRequested() && outputQueue_p.empty()){
                break;
            }

            text = outputQueue_p.front();

            outputQueue_p.pop();
        }

        // Now output the text and then delete the storage

        * logStream_p << text;

        logStream_p->flush();

        delete text;

    }

    * logStream_p << "*** Logging terminated" << endl;

    return NULL;
}

void
Logger::LoggerThread::setLogFilename (const String & filename)
{
    logFilename_p = filename;
}


Mutex::Mutex ()
{
    impl_p = new MutexImpl ();

    pthread_mutex_t init = PTHREAD_MUTEX_INITIALIZER;

    impl_p->mutex_p = new pthread_mutex_t;
    * (impl_p->mutex_p) =(init);

    int code = pthread_mutex_init (impl_p->mutex_p, NULL);
    ThrowIfError (code, "Mutex::init failed");

}

Mutex::~Mutex ()
{
    pthread_mutex_destroy (impl_p->mutex_p);

    delete impl_p->mutex_p;

    delete impl_p;
}

pthread_mutex_t *
Mutex::getRep ()
{
    return impl_p->mutex_p;
}

void
Mutex::lock ()
{
    int code = pthread_mutex_lock (impl_p->mutex_p);
    ThrowIfError (code, "Mutex::lock failed");
}

/*
Bool
Mutex::lock (Int milliseconds)
{

    Assert (milliseconds >= 0); // weird if it's negative

    struct timespec t = convertMsDeltaToTimespec (milliseconds);
    int code = pthread_mutex_timedlock (impl_p->mutex_p, & t);

    bool gotLock = true;
    if (code == ETIMEDOUT){
        gotLock = false;
    }
    else{
        ThrowIfError (code, format ("Mutex::lock (%d)", milliseconds));
    }

    return gotLock;
}
*/

Bool
Mutex::trylock ()
{
    bool gotLock = true;
    int code = pthread_mutex_trylock (impl_p->mutex_p);

    if (code == EBUSY){
        gotLock = false;
    }
    else{
        ThrowIfError (code, "Mutex::trylock");
    }

    return gotLock;
}

void
Mutex::unlock ()
{
    int code = pthread_mutex_unlock (impl_p->mutex_p);
    ThrowIfError (code, "Mutex::unlock");
}


MutexLocker::MutexLocker (Mutex & mutex)
  : mutex_p (mutex)
{
    mutex_p.lock();
}

MutexLocker::~MutexLocker ()
{
    mutex_p.unlock();
}

Semaphore::Semaphore (int initialValue)
{
    Assert (initialValue >= 0);

    impl_p = new SemaphoreImpl ();
    impl_p->semaphore_p = new sem_t;
    int code = sem_init (impl_p->semaphore_p, 0, initialValue);
    ThrowIfError (code == 0 ? 0 : errno, "Semaphore::init");
}

Semaphore::~Semaphore ()
{
    int code = sem_destroy (impl_p->semaphore_p);
    ThrowIfError (code == 0 ? 0 : errno, "Semaphore::destroy");

    delete impl_p->semaphore_p;
    delete impl_p;
}

Int
Semaphore::getValue ()
{
    int value;
    int code = sem_getvalue (impl_p->semaphore_p, & value);
    ThrowIfError (code == 0 ? 0 : errno, "Semaphore::getValue");

    return value;
}

void
Semaphore::post ()
{
    int code = sem_post (impl_p->semaphore_p);
    ThrowIfError (code == 0 ? 0 : errno, "Semaphore::post");
}

Bool
Semaphore::trywait ()
{
    int code = sem_trywait (impl_p->semaphore_p);
    bool gotSemaphore = true;

    if (code != 0 && errno == EAGAIN){
        gotSemaphore = false;
    }
    else{
        ThrowIfError (code == 0 ? 0 : errno, "Semaphore::wait");
    }

    return gotSemaphore;
}

void
Semaphore::wait ()
{
    int errorCode = 0;
    int code;

    do {

        code = sem_wait (impl_p->semaphore_p);
        errorCode = errno;

    } while (code != 0 && errorCode == EINTR);

    ThrowIfError (code == 0 ? 0 : errorCode, "Semaphore::wait");
}

Bool
Semaphore::wait (int milliseconds)
{
    Assert (milliseconds >= 0); // it's weird if it's negative

    struct timespec t = convertMsDeltaToTimespec (milliseconds);
    int errorCode = 0;
    int code;

    do {

        code = sem_wait (impl_p->semaphore_p);
        errorCode = errno;

    } while (code != 0 && errorCode == EINTR);


    Bool gotSemaphore = True;

    if (code == 0){
        gotSemaphore = True;
    } else if (errno == ETIMEDOUT){
        gotSemaphore = False;
    } else {
        ThrowIfError (errno, format ("Mutex::lock (%d)", milliseconds));
    }

    return gotSemaphore;
}


Thread::Thread ()
{
    id_p = new pthread_t;
    started_p = false;
    terminationRequested_p = false;
}

Thread::~Thread ()
{
    // Make sure the thread knows it's time to quit

    terminate ();

    delete id_p;
}

pthread_t
Thread::getId () const
{
    return * id_p;
}

void *
Thread::join ()
{
    void * result;
    pthread_join (* id_p, & result);

    return result;
}

bool
Thread::isStarted () const
{
    return started_p;
}

void
Thread::startThread ()
{
    // Create the thread, passing a pointer to this object as its
    // single argument.  Subclass Thread to pass other information
    // into the thread function.

    int code = pthread_create (id_p, NULL, threadFunction, this);
    started_p = true;
    ThrowIfError (code, "Thread::create");
}

void
Thread::terminate ()
{
    terminationRequested_p = true;
}


bool
Thread::terminationRequested () const
{
    return terminationRequested_p;
}

void *
Thread::threadFunction (void * arg)
{
    Thread * thread = reinterpret_cast<Thread *> (arg);

    void * result = thread->run ();

    return result; // use thread variable to store any results
}



} // end namespace Async

} // end namespace CASA
