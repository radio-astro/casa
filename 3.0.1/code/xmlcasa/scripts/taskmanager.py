from IPython.kernel import client
from IPython.kernel.multiengineclient import PendingResult
from mindpipes import mindpipes
import subprocess
import inspect
import signal
import string
import atexit
import time
import sets
import sys
import os
import re

import casac
casalog = casac.homefinder.find_home_by_name('logsinkHome').create()

def log_message( state, file, lines ) :
    casalog.origin(str(state['engine']['current task']))
    if state['out'] == "stderr" :
        for line in lines:
            casalog.post(line,"WARN")
    else :
        for line in lines:
            casalog.post(line,"INFO")

#def log_message( state, file, lines ) :
#    for line in lines:
#        print "[" + str(state['engine']['current task']) + "/" + str(state['out']) + "]: " + line

class taskmanager(object):
    "manage task engines"
    __dir = { 'home': os.environ['HOME'] + "/.casa/tm" }
    __cert = { }
    __furl = { }
    __hub = { 'init atend': False, 'proc': None, 'engines': [], 'engine hosts': { },
              'mec': None, 'tasks initialized': [], 'task path': [], 'initialized': False,
              'log root': None, 'execute count': 0, 'result map': { }, 'pipe minder': None }

    def retrieve(self, reciept):
        try:
            if isinstance( self.__hub['result map'][reciept]['result'], PendingResult ):
                result = self.__hub['result map'][reciept]['result'].get_result(block=False)
                if result is not None:
                    target = self.__hub['result map'][reciept]['engine']['index']
                    result_name = "result_%04d" % reciept
                    self.__hub['result map'][reciept]['result'] = { 'result': self.__hub['mec'].pull( result_name, targets=[target] )[0] }
                    self.__hub['result map'][reciept]['result output'] = result[0]
                    if self.__hub['result map'][reciept]['result output'].has_key('stdout') :
                        engine = self.__hub['result map'][reciept]['engine']
                        log_message({'out': 'stdout', 'engine': engine}, 1, self.__hub['result map'][reciept]['result output']['stdout'].splitlines() )
                    if self.__hub['result map'][reciept]['result output'].has_key('stderr') :
                        log_message({'out': 'stderr', 'engine': engine}, 2, self.__hub['result map'][reciept]['result output']['stderr'].splitlines() )
                    return self.__hub['result map'][reciept]['result']
                else:
                    return { 'result': 'pending' }
        except:
            pass

        if self.__hub['result map'].has_key(reciept):
            return self.__hub['result map'][reciept]['result']
        else:
            raise Exception, "invalid reciept: " + str(reciept)

    def execute(self, taskname, *args, **kwargs):

        if not self.__hub['initialized']:
            self.__initialize( )

        engine = self.__find_engine( )
        targets = [ engine['index'] ]

        if self.__hub['mec'] is None :
            self.__setup_mec( )

        if engine['setup'] is not True:
            self.__setup_engine(engine)

        try:
            engine['loaded tasks'].index(taskname)
            already_initialized = True
        except:
            already_initialized = False

        if not engine['loaded tasks'].has_key(taskname) :
            self.__hub['mec'].execute( "from task_" + taskname + " import " + taskname, block=False, targets=targets )
            engine['loaded tasks'][taskname] = True

        count = self.__hub['execute count']
        self.__hub['execute count'] = count + 1
        x = self.__hub['mec'].push( {'args': args, 'kwargs': kwargs}, targets=targets )
        result_name = "result_%04d" % count
        engine['current task'] = taskname
        self.__hub['result map'][count] = { 'engine': engine,
                                            'result': self.__hub['mec'].execute( "casalog.origin('taskmanager'); " + \
                                                                                 "casalog.post('##### async task launch:     " + taskname + " ########################'); " + \
                                                                                 "os.chdir( '" + os.getcwd( ) + "' ); " + \
                                                                                 result_name + " = " + taskname + "(*args,**kwargs); " + \
                                                                                 "os.chdir( _startup_dir_ ); " + \
                                                                                 "casalog.origin('taskmanager'); " + \
                                                                                 "casalog.post('##### async task completion: " + taskname + " ########################')", \
                                                                                     block=False,targets=targets) }
        ## not sure if any pause is necessary or not...
        time.sleep(0.25)
        return count

    def __finalize(self):

        if self.__hub['mec'] is not None :
            self.__hub['mec'].kill(controller=True)
            for e in self.__hub['engines'] :
                try:
                    if e['proc'].pid is not None:
                        os.kill(e['proc'].pid,signal.SIGKILL)
                except:
                    pass

        try:
            if self.__hub['proc'] is not None and self.__hub['proc'].pid is not None:
                os.kill(self.__hub['proc'].pid,signal.SIGINT)
        except:
            pass

        self.__clean_furls( )
        if os.path.exists(self.__dir['log root'] + '/last'):
            self.__rmdir(self.__dir['log root'] + '/last')
        try:
            os.rename(self.__dir['session log root'], self.__dir['log root'] + '/last')
        except:
            print "could not rename " + self.__dir['session log root'] + " to " + self.__dir['log root'] + '/last' + " ..."


    def __find_engine( self ) :
        for engine in self.__hub['engines'] :
            prop = self.__hub['mec'].get_properties(targets=[engine['index']],block=False)
            status = prop.client.queue_status()[engine['index']][1]['pending']
            if engine['active'] and ( status == 'None' or re.match('^get_properties',status) is not None ):
                return engine
        for i in range(0,5):
            self.__start_engine( )
            for j in range(0,5):
                for engine in self.__hub['engines'] :
                    prop = self.__hub['mec'].get_properties(targets=[engine['index']],block=False)
                    status = prop.client.queue_status()[engine['index']][1]['pending']
                    if engine['active'] and ( status == 'None' or re.match('^get_properties',status) is not None ):
                        return engine
                time.sleep(0.25)
        raise Exception, "no engines available"

    def __start_engine(self,host='localhost',num=1):

        if host != 'localhost':
            raise Exception, "remote hosts not currently supported...\nplease report as JIRA ticket so that we know someone is interested in remote tasks"
        if num <= 0 :
            raise Exception, "__start_engine called with num == 0"

        if not os.path.exists(self.__dir['session log root']):
            self.__mkdir(self.__dir['session log root'])

        for i in range(0,20):
            if os.path.exists(self.__furl['engine']):
                break
            time.sleep(0.5)

        for i in range(0,num) :
            engine = { }
            engine['host'] = host
            engine['active'] = True

            if self.__hub['mec'] is None :
                self.__setup_mec( )

            ids = self.__hub['mec'].get_ids( )
            if ids is None :
                ids = [ ]
            preset = sets.Set( ids )
            engine['stdout'] = os.pipe( )
            engine['stderr'] = os.pipe( )
            self.__hub['pipe minder'].watch(engine['stdout'][0],log_message,{'out': 'stdout', 'engine': engine })
            self.__hub['pipe minder'].watch(engine['stderr'][0],log_message,{'out': 'stderr', 'engine': engine })
            engine['proc'] = subprocess.Popen( [ 'ipengine', '--furl-file=' + self.__furl['engine'],
                                                 '--ipythondir=' + self.__dir['rc'],
                                                 '--logfile=' + self.__dir['session log root'] + "/engine." ],
                                               stdout=engine['stdout'][1], stderr=engine['stderr'][1])

            ids = self.__hub['mec'].get_ids( )
            if ids is None :
                ids = [ ]
            postset = sets.Set( ids )
            try_count = 0
            while len(postset - preset) == 0 :
                ids = self.__hub['mec'].get_ids( )
                if ids is None :
                    ids = [ ]
                postset = sets.Set( ids )
                if try_count > 20 :
                    raise Exception, "could not start ipengine"
                time.sleep(0.5)
                try_count += 1

            index = postset - preset;
            engine['index'] = index.pop( )
            engine['loaded tasks'] = { }
            engine['setup'] = False
            engine['log'] = self.__dir['session log root'] + '/engine.' + str(engine['proc'].pid) + ".log"
            engine['active'] = True
            self.__hub['engines'].append(engine)

        return len(self.__hub['engines']) - 1

    def __initialize(self):
        for host in self.__hub['engine hosts'].keys( ):
            if self.__hub['engine hosts'][host] > 0 :
                self.__start_engine(host,self.__hub['engine hosts'][host])
        self.__hub['initialized'] = True

    def __start_hub(self):
        self.__mkdir(self.__dir['session log root'])
        self.__hub['proc'] = subprocess.Popen( [ 'ipcontroller',
                                                 '--client-cert-file=' + self.__cert['client'],
                                                 '--engine-cert-file=' + self.__cert['engine'],
                                                 '--engine-furl-file=' + self.__furl['engine'],
                                                 '--multiengine-furl-file=' + self.__furl['mec'],
                                                 '--task-furl-file=' + self.__furl['tc'],
                                                 '--ipythondir=' + self.__dir['rc'],
                                                 '--logfile=' + self.__dir['session log root'] + "/controller." ],
                                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT )

        self.__hub['log'] = self.__dir['session log root'] + '/controller.' + str(self.__hub['proc'].pid) + ".log"

    def __setup_mec(self):
        for i in range(0,10):
            if os.path.exists(self.__furl['mec']):
                break
            time.sleep(0.5)
        time.sleep(0.25)
        self.__hub['mec'] = client.MultiEngineClient(self.__furl['mec'])

    def __setup_engine(self,engine) :
        if engine['setup'] is not True:

            ####
            #### needed to allow pushing of the global 'casa' state dictionary
            ####
            a=inspect.stack()
            stacklevel=0    
            for k in range(len(a)):
                if (string.find(a[k][1], 'ipython console') > 0):
                    stacklevel=k
            myf=sys._getframe(stacklevel).f_globals

            block = True
            targets = [ engine['index'] ]
            x = self.__hub['mec'].push( {'casa': myf['casa'] }, targets=targets )

            x = self.__hub['mec'].execute('import sys',targets=targets)
            path = self.__hub['task path'][:]
            path.reverse( )
            for p in path:
                x = self.__hub['mec'].execute("sys.path.insert(0,'" + p + "')",block=block,targets=targets)

            x = self.__hub['mec'].execute('import os',block=block,targets=targets)
            x = self.__hub['mec'].execute('_startup_dir_ = os.getcwd( )',block=block,targets=targets)
            x = self.__hub['mec'].execute('import signal',block=block,targets=targets)
            x = self.__hub['mec'].execute("original_sigint_handler = signal.signal(signal.SIGINT,signal.SIG_IGN)",block=block,targets=targets)
            x = self.__hub['mec'].execute('from taskinit import casalog',targets=targets)
            engine['setup'] = True

    def __init__(self,task_path=[''],engines={'localhost': 1}):

        if os.environ.has_key('__CASARCDIR__'):
            self.__dir['home'] = os.environ['__CASARCDIR__'] + "/tm"

        self.__dir['rc'] = self.__dir['home'] + '/rc'
        self.__dir['furl'] = self.__dir['home'] + '/furl'
        self.__dir['log root'] = self.__dir['home'] + '/log'

        self.__dir['session log root'] = self.__engine_log_root( )

        self.__hub['engine hosts'] = engines
        self.__hub['task path'] = task_path

        self.__furl['engine'] = self.__dir['furl'] + '/ipcontroller-engine.' + str(os.getpid()) + '.furl'
        self.__furl['mec'] = self.__dir['furl'] + '/ipcontroller-mec.' + str(os.getpid()) + '.furl'
        self.__furl['tc'] = self.__dir['furl'] + '/ipcontroller-tc.' + str(os.getpid()) + '.furl'

        self.__cert['client'] = self.__dir['furl'] + '/ipcontroller-client.' + str(os.getpid()) + '.pem'
        self.__cert['engine'] = self.__dir['furl'] + '/ipcontroller-engine.' + str(os.getpid()) + '.pem'

        if self.__hub['init atend'] :
            atexit.register(taskmanager.__finalize,self)
            self.__hub['init atend'] = False

        if self.__hub['pipe minder'] is None :
            self.__hub['pipe minder'] = mindpipes( )
            self.__hub['pipe minder'].start( )

        self.__mkdir(self.__dir['rc'])
        self.__mkdir(self.__dir['furl'])
        self.__mkdir(self.__dir['log root'])

        self.__clean_furls( )
        self.__start_hub( )


    def __clean_furls(self):
        if os.path.exists(self.__furl['engine']):
            os.unlink(self.__furl['engine'])
        if os.path.exists(self.__furl['mec']):
            os.unlink(self.__furl['mec'])
        if os.path.exists(self.__furl['tc']):
            os.unlink(self.__furl['tc'])

    def __engine_log_root(self):
        return self.__dir['log root'] + '/' + str(os.getpid())

    def __mkdir(self,p):
        dirsep = '/'
        pv = p.split(dirsep)
        path = ""
        for i in pv:
            if i:
                path = path + "/" + i
                self.__mkdir1(path)

    def __mkdir1(self,p):
        if os.path.exists(p):
            if not os.path.isdir(p):
                cnt = 1
                tmp = p + "."
                while os.path.exists(tmp + str(cnt)):
                    cnt += 1
                os.rename(p,tmp)
                os.mkdir(p)
        else:
            os.mkdir(p)

    def __rmdir(self,p):
        dirsep = '/'
        if os.path.isdir(p):
            contents = os.listdir(p)
            for f in contents:
                if os.path.isfile( p + dirsep + f ) or os.path.islink( p + dirsep + f ):
                    try:
                        os.remove( p + dirsep + f )
                    except:
                        print "could not remove:", p + dirsep + f
                elif os.path.isdir( p + dirsep + f ):
                    self.__rmdir( p + dirsep + f )
            try:
                os.rmdir(p)
            except:
                print "could not remove:", p
                print "renaming to:     ", p + ".nfs-" + str(os.getpid())
                try:
                    os.rename( p, p + ".nfs-" + str(os.getpid()) )
                except:
                    print "               ...renaming failed!!!"

if os.environ.has_key('__CASAPY_PYTHONDIR'):
    tm = taskmanager( task_path=[ '', os.environ['__CASAPY_PYTHONDIR'] ] )
else:
    tm = taskmanager( task_path=[ '', '/CASASUBST/task_directory/' ] )
