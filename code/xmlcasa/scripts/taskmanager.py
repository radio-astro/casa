from IPython.kernel import client
import subprocess
import signal
import atexit
import time
import os
import sys

class taskmanager(object):
    "manage task engines"
    __dir = { 'home': os.environ['HOME'] + "/.casa/tm" }
    __cert = { }
    __furl = { }
    __hub = { 'init atend': True, 'proc': None, 'engines': [], 'engine hosts': [ ],
              'mec': None, 'tc': None, 'tasks initialized': [], 'task path': [],
              'log root': None }

    def retrieve(self, reciept):
        return self.__hub['tc'].get_task_result(reciept)

    def abort(self, reciept):
        return self.__hub['tc'].abort(reciept)

    def execute(self, taskname, *args, **kwargs):

        if self.__hub['mec'] is None or self.__hub['tc'] is None :
            self.__establish_clients( )

        try:
            self.__hub['tasks initialized'].index(taskname)
            already_initialized = True
        except:
            already_initialized = False

        if not already_initialized :
            self.__hub['mec'].execute("from task_" + taskname + " import " + taskname,block=False)
            self.__hub['tasks initialized'].append(taskname)

        return self.__hub['tc'].run(client.StringTask("import os; os.chdir('" + os.getcwd() + "'); result = " + taskname + "(*args,**kwargs)",
                                                      push={'args': args, 'kwargs': kwargs }, pull=['result']))

    def __finalize(self):
        for e in self.__hub['engines'] :
            if e['proc'].pid is not None:
                os.kill(e['proc'].pid,signal.SIGKILL)
        if self.__hub['proc'] is not None and self.__hub['proc'].pid is not None:
            os.kill(self.__hub['proc'].pid,signal.SIGINT)
        if os.path.exists(self.__furl['engine']):
            os.unlink(self.__furl['engine'])
        if os.path.exists(self.__furl['mec']):
            os.unlink(self.__furl['mec'])
        if os.path.exists(self.__furl['tc']):
            os.unlink(self.__furl['tc'])
        if os.path.exists(self.__dir['log root'] + '/last'):
            self.__rmdir(self.__dir['log root'] + '/last')
        os.rename(self.__dir['session log root'], self.__dir['log root'] + '/last')
            

    def __start_engine(self,host='localhost'):

        if host != 'localhost':
            raise Exception, "remote hosts not currently supported...\nplease report as JIRA ticket so that we know someone is interested in remote tasks"

        self.__mkdir(self.__dir['session log root'])
        engine = { }
        engine['host'] = host
        engine['active'] = True
        engine['proc'] = subprocess.Popen( [ 'ipengine', '--furl-file=' + self.__furl['engine'],
                                             '--ipythondir=' + self.__dir['rc'],
                                             '--logfile=' + self.__dir['session log root'] + "/engine." ],
                                           stdout=subprocess.PIPE, stderr=subprocess.STDOUT )
        engine['log'] = self.__dir['session log root'] + '/engine.' + str(engine['proc'].pid) + ".log"
        self.__hub['engines'].append(engine)
        return len(self.__hub['engines']) - 1

    def __start_engines(self):
        for i in range(0,10):
            if os.path.exists(self.__furl['engine']):
                break
            time.sleep(1)
        for host in self.__hub['engine hosts']:
            self.__start_engine( host )

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

        self.__start_engines( )


    def __establish_clients(self):
        block = True
        for i in range(0,10):
            if os.path.exists(self.__furl['mec']):
                break
            time.sleep(1)
        self.__hub['mec'] = client.MultiEngineClient(self.__furl['mec'])
        self.__hub['tc'] =  client.TaskClient(self.__furl['tc'])

        x = self.__hub['mec'].execute('import sys')
        path = self.__hub['task path'][:]
        path.reverse( )
        for p in path:
            x = self.__hub['mec'].execute("sys.path.insert(0,'" + p + "')",block=block)

        x = self.__hub['mec'].execute('import signal',block=block)
        x = self.__hub['mec'].execute("original_sigint_handler = signal.signal(signal.SIGINT,signal.SIG_IGN)",block=block)

    def __init__(self,task_path=[''],engines=['localhost', 'localhost', 'localhost', 'localhost']):

        self.__dir['rc'] = self.__dir['home'] + '/rc'
        self.__dir['furl'] = self.__dir['home'] + '/furl'
        self.__dir['log root'] = self.__dir['home'] + '/log'

        self.__dir['session log root'] = self.__engine_log_root( )

        self.__hub['engine hosts'] = engines
        self.__hub['task path'] = task_path

        self.__furl['engine'] = self.__dir['furl'] + '/ipcontroller-engine.furl'
        self.__furl['mec'] = self.__dir['furl'] + '/ipcontroller-mec.furl'
        self.__furl['tc'] = self.__dir['furl'] + '/ipcontroller-tc.furl'

        self.__cert['client'] = self.__dir['furl'] + '/ipcontroller-client.pem'
        self.__cert['engine'] = self.__dir['furl'] + '/ipcontroller-engine.pem'

        if self.__hub['init atend'] :
            atexit.register(taskmanager.__finalize,self)
            self.__hub['init atend'] = False

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
                    os.remove( p + dirsep + f )
                elif os.path.isdir( p + dirsep + f ):
                    self.__rmdir( p + dirsep + f )
            os.rmdir(p)

if os.environ.has_key('__CASAPY_PYTHONDIR'):
    tm = taskmanager( task_path=[ '', os.environ['__CASAPY_PYTHONDIR'] ] )
else:
    tm = taskmanager( task_path=[ '', '/CASASUBST/task_directory/' ] )
