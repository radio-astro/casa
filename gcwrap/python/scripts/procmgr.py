from enum import Enum
from time import sleep, localtime, strftime
from subprocess import Popen, PIPE
from threading import Thread
import sys
import os

from casa_shutdown import add_shutdown_hook

class output_option(Enum):
    PIPE = 1
    STDOUT = 2
    DISCARD = 3


##
##  dgtool's pylot class was a useful reference for creating this class:
##  http://dgtool.treitos.com/2012/05/python-subprocess-manager-pylot.html
##
class procmgr(Thread):

    from procmgr import output_option

    class proc(Thread):
        def __init__(self, tag, cmd, output=output_option.DISCARD):
            """ tag: identification for this process
            cmd: list of strings representing the command followed by the arguments
            output: disposition of output STDOUT, PIPE, DISCARD (default is to discard output)"""
            Thread.__init__(self)
            self.tag = tag
            self.pid = None
            self.stderr = None
            self.stdout = None
            self.stdin  = None
            self.__cmd = cmd
            self.__output_option = output
            self.__running = False
            self.__proc = None
            self.__watchdog = None

        def stop(self):
            """ stop( ) -> None
            stops the process if it is running"""
            if self.__running:
                self.__running = False
                if self.__proc and self.__proc.poll( ) == None:
                    #print "%s => proc %s is being stopped" % (strftime("%y-%m-%d %H:%M:%S", localtime()), self.tag)
                    try:
                        self.__proc.terminate()
                        sleep(0.5)
                        if self.__proc.poll( ) is None:
                            print "%s => proc %s is being killed" % (strftime("%y-%m-%d %H:%M:%S", localtime()), self.tag)
                            self.__proc.kill()
                        if self.__watchdog.poll( ) is None:
                            self.__watchdog.kill()
                    except OSError:
                        pass

        def running(self):
            """ running( ) -> Bool
             check to see if the process is still running"""
            return self.__running

        def run(self):
            """ run( ) -> None
            start the process"""
            self.stop( )
            #print "%s => proc %s is being started" % (strftime("%y-%m-%d %H:%M:%S", localtime()), self.tag)
            if self.__output_option is output_option.PIPE:
                out = PIPE
            elif self.__output_option is output_option.STDOUT:
                out = os.fdopen(sys.stdout.fileno(), 'a', 0)
            else:
                out = file(os.devnull,'a')
            self.__proc = Popen( self.__cmd, stderr=out , stdout=out, stdin=PIPE )
            self.__watchdog = Popen( [ '/bin/bash','-c', 
                                       'while kill -0 %d > /dev/null 2>&1; do sleep 1; kill -0 %d > /dev/null 2>&1 || kill -9 %d > /dev/null 2>&1; done' % \
                                     (self.__proc.pid, os.getpid( ), self.__proc.pid) ] )

            self.stdin = self.__proc.stdin
            self.pid = self.__proc.pid
      
            if self.__output_option is output_option.PIPE:
                self.stdout = self.__proc.stdout
                self.stderr = self.__proc.stderr

            self.__running = True
            ## at this point, processes could automatically be restarted
            ## (in a loop) if __running is not false when the process
            ## expires... (as pylot does)
            self.__proc.wait( )


    # --- --- --- procmgr --- --- --- --- --- --- --- --- --- --- --- ---
    def __init__(self):
        Thread.__init__(self)
        self.__procs = { }
        self.__running = True
        add_shutdown_hook(self.shutdown)

    def running(self, tag):
        """tag: process identifier to check
        return True if a process with the given tag exists and is running"""
        if self.__running == False: return False
        if self.__procs.has_key( tag ):
            return self.__procs[tag].running( )
        return False

    def create(self, tag, cmd, with_output=False):
        """create(...) -> proc
        tag: identifier for the new process
        cmd: list of strings representing the command followed by the arguments
        with_output: should output be collected (default is to discard output)"""
        if self.__running == False: return None
        if not isinstance(cmd, (list,str)):
            print "'cmd' is not a list of strings"
            return None
        if self.running(tag):
            print "process with %s tag already exists" % tag
            return None
        self.__procs[tag] = self.proc(tag,cmd,with_output)
        self.__procs[tag].start( )
        sleep(0.1)
        return self.__procs[tag]

    def fetch(self,tag):
        """fetch the process associated with 'tag'"""
        if self.__running == False: return None
        if self.__procs.has_key( tag ):
            return self.__procs[tag]
        return None        

    def shutdown(self):
        """stops all managed processes"""
        if self.__running:
            for p in self.__procs:
                self.__procs[p].stop( )
        self.__running = False

    def __delitem__(self, key):
        print "you cannot stop processes this way"
        return None
    def __getitem__(self, tag):
        """fetch the process associated with 'tag'"""
        return self.fetch(tag)
    def __setitem__(self, key, value):
        print "you cannot create processes this way"
        return None

    def run(self):
        while self.__running:
            sleep(10)

