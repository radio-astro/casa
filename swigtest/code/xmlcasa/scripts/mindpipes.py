import threading
import select
import time
import base64
import uuid
import os
import posix
import fcntl

class mindpipes (threading.Thread) :
    __change = threading.Lock( )
    __queue_lock = threading.Lock( )
    __queue = { }

    class nub :
        __watcher = None
        __id = None
        def __init__(self,watcher,id):
            self.__watcher = watcher
            self.__id = str(id)
        def id ( self ) :
            return self.__id;
        def unwatch ( self ) :
            self.__watcher.remove(self.__id)

    def __init__(self):
        threading.Thread.__init__(self)
        self.__change.acquire( )
        self.setDaemon(True)

    def watch ( self, fd, callback, data=None ):
        fcntl.fcntl(fd, fcntl.F_SETFL, posix.O_NDELAY)
        id = base64.b64encode(uuid.uuid4().bytes,'..')[0:10]
        self.__queue_lock.acquire( )
        while self.__queue.has_key(id):
            id = base64.b64encode(uuid.uuid4().bytes,'..')[0:10]
        self.__queue[id] = { 'pipe': fd, 'callback': callback, 'buffer': '', 'data': data }
        self.__queue_lock.release( )
        while not self.__change.locked( ) :
            time.sleep(0.1)
        self.__change.release( )
        return self.nub(self,id)

    def remove ( self, nubbin ):
        self.__queue_lock.acquire( )
        if self.__queue.has_key(nubbin.id( )):
            x = self.queue.pop(nubbin.id( ))
            x.pop('buffer')
            self.__queue_lock.release( )
            while not self.__change.locked( ) :
                time.sleep(0.1)
            self.__change.release( )
            return x
        else:
            self.__queue_lock.release( )
            return None


    def run( self ) :
        while ( 1 ) :
            self.__queue_lock.acquire( )
            read_fds = [ ]
            for key in self.__queue.keys( ) :
                read_fds.append(self.__queue[key]['pipe'])
            self.__queue_lock.release( )
            if not self.__change.locked( ):
                self.__change.acquire( )
            while self.__change.locked( ) :
                if len(read_fds) > 0 :
                    self.__select(read_fds)
                else:
                    self.__change.acquire( )
                    break

    def __select( self, read_fds ) :
        timeout = 3
        (read, write, excep) = select.select(read_fds, [ ], [ ], timeout)
#       print "select ended: (" + str(len(read)) + "," + str(len(write)) + "," + str(len(excep)) + ")"
        for file in read:
            line = os.read(file,9216)
            lines = line.splitlines( )
            self.__queue_lock.acquire( )
            for key in self.__queue.keys( ) :
                if self.__queue[key]['pipe'] == file:
                    if self.__queue[key]['buffer'] !=  '':
                        lines[0] = self.__queue[key]['buffer'] + lines[0]
                        self.__queue[key]['buffer'] = ''
                    if line[len(line)-1] != '\n':
                        self.__queue[key]['buffer'] = lines.pop()
                    if len(lines) > 0:
                        self.__queue[key]['callback'](self.__queue[key]['data'],file,lines)
                    break
            self.__queue_lock.release( )
