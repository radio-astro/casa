import os
import re
import copy
import time
import signal
import atexit
import random
import operator
import threading
from subprocess import *
import ipython1.kernel.api as kernel

class taskmanager(object):
    "manage task engines"
    hub = { 'proc': None, 'output': None, 'ports': [ None, None, None],
            'used ports': [], 'verified': False, 'startup trys': 0,
            'timer': None, 'engines': [], 'tasks': { }, 'clear results': True,
            'remote controller': None, 'results': [], 'task path': [] }
    rcdir = os.environ['HOME'] + "/.casapy/tm"

    def status(self, reciept):
        self.__sweep_engines( )
        if self.hub['results'][reciept] is not None:
            if self.hub['results'][reciept]['status'] is not None:
                return self.hub['results'][reciept]['status']
            elif self.hub['results'][reciept]['engine'] is not None:
                if self.hub['results'][reciept]['result']['state'] == 'crashed':
                    return self.hub['results'][reciept]['result']
                elif self.hub['results'][reciept]['result']['state'] != 'done':
                    index = self.hub['results'][reciept]['engine']
                    return self.hub['remote controller'].status(self.hub['engines'][index]['id'])
                else:
                    return None
            else:
                return None
        else:
            return None

    def retrieve(self, reciept):
        self.__sweep_engines( )
        if self.hub['results'][reciept] is not None:
            result = self.hub['results'][reciept]['result']
            if result['state'] == 'done' and self.hub['clear results']:
                self.hub['results'][reciept] = None
            return result
        else:
            return None

    def abort(self, reciept):
        self.__sweep_engines( )
        if self.hub['results'][reciept] is not None and self.hub['results'][reciept]['status'] != 'done':
            if self.hub['results'][reciept]['engine'] is not None:
                self.__remove_engine(self.hub['results'][reciept]['engine'])
        self.hub['results'][reciept] = None

    def execute(self, taskname, *args, **kwargs):
        self.__sweep_engines( )
        index = self.__get_engine(taskname)
        id = self.hub['engines'][index]['id']
        self.hub['remote controller'].push(id,args=args)
        self.hub['remote controller'].push(id,kwargs=kwargs)
        self.hub['remote controller'].execute(id,"result = " + taskname + "(*args,**kwargs)",block=False)
        return self.hub['engines'][index]['result']

    def finalize(self):
        if os.path.exists(self.__hub_rc_path(False)):
            os.remove(self.__hub_rc_path(False))
        if self.hub['timer'] is not None:
            self.__timer_cancel( )
        for i in range(0,len(self.hub['engines'])):
            self.__remove_engine(i)
        if self.__initialized( ):
            os.kill(self.hub['proc'].pid,signal.SIGKILL)
        if self.hub['output'] is not None and not self.hub['output'].closed:
            self.hub['output'].close()
        if os.path.exists(self.__hub_log_path( )):
            hub_log_backup = self.rcdir + "/hub.last"
            if os.path.exists(hub_log_backup):
                if os.path.isdir(hub_log_backup):
                    self.__rmdir(hub_log_backup)
                else:
                    os.remove(hub_log_backup)
            os.rename( self.__hub_log_path( ), hub_log_backup )
        if os.path.exists(self.__engine_log_root( )):
            engine_log_backup = self.rcdir + "/engine.last"
            if os.path.exists(engine_log_backup):
                if os.path.isdir(engine_log_backup):
                    self.__rmdir(engine_log_backup)
                else:
                    os.remove(engine_log_backup)
            os.rename( self.__engine_log_root( ), engine_log_backup )
                
    def __init__(self,task_path=[''],clear_results=True):
        if not self.__initialized( ) and self.hub['timer'] is None:
            atexit.register(taskmanager.finalize,self)
            self.hub['task path'] = task_path[:]
            self.hub['clear results'] = clear_results
            self.__mkdir(self.rcdir)
            self.__start_hub( )
            time.sleep(1)
            self.__timer_verify( )

    def __get_engine(self,taskname):
        host = 'localhost'
        if not self.hub['tasks'].has_key(host):
            self.hub['tasks'][host] = { }
        if not self.hub['tasks'][host].has_key(taskname):
            self.hub['tasks'][host][taskname] = [ ]
        found_task = None
        for index in self.hub['tasks'][host][taskname]:
            if self.hub['engines'][index]['state'] == 'idle':
                self.hub['results'].append({'result': {'state': 'pending', 'result': None}, 'status': None, 'engine': index})
                self.hub['engines'][index]['result'] = len(self.hub['results'])-1
                self.hub['engines'][index]['state'] = 'active'
                return index
        index = self.__start_engine( )
        id = self.hub['engines'][index]['id']
        self.hub['results'].append({'result': {'state': 'pending', 'result': None}, 'status': None, 'engine': index})
        self.hub['engines'][index]['result'] = len(self.hub['results'])-1
        self.hub['engines'][index]['state'] = 'active'
        self.hub['engines'][index]['task'] = taskname
        self.hub['tasks'][host][taskname].append(index)
        self.hub['remote controller'].execute(id,"import sys",block=False)
        path = self.hub['task path'][:]
        path.reverse( )
        for p in path:
            self.hub['remote controller'].execute(id,"sys.path.insert(0,'" + p + "')",block=False)
        ###
        ### for now, ignore SIGINT so it does not upturn our ipengine
        ###
        self.hub['remote controller'].execute(id,"import signal",block=False)
        self.hub['remote controller'].execute(id,"original_sigint_handler = signal.signal(signal.SIGINT,signal.SIG_IGN)",block=False)
        ###
        self.hub['remote controller'].execute(id,"from task_" + taskname + " import " + taskname,block=False)
        return index

    def __start_engine(self):
        host = 'localhost'
        host_ip = '127.0.0.1'
        engine_port_index = 0
        controller_port_index = 1

        if self.hub['remote controller'] is None:
            while not self.hub['verified']:
                time.sleep(1)
                self.__verify( )
            self.hub['remote controller'] = kernel.RemoteController((host_ip,self.hub['ports'][controller_port_index]))
            self.hub['remote controller'].connect( )

        original_ids = self.hub['remote controller'].getIDs()

        engine = { }
        engine['host'] = host
        engine['output'] = open(self.__engine_log_path( ),'w')
        engine['proc'] = Popen('ipengine --controller-port=' + str(self.hub['ports'][engine_port_index]),
                               stdout=engine['output'],stderr=engine['output'],shell=True)
        new_ids = self.hub['remote controller'].getIDs()
        while len(new_ids) == len(original_ids):
            time.sleep(1)
            new_ids = self.hub['remote controller'].getIDs()

        engine['id'] = set(new_ids).difference(original_ids).pop( )
        engine['state'] = 'idle'
        engine['result'] = None
        
        self.hub['engines'].append(engine)
        return len(self.hub['engines']) - 1

    def __remove_engine(self, index):
        if self.hub['engines'][index] is not None:
            hostname = self.hub['engines'][index]['host']
            taskname = self.hub['engines'][index]['task']
            self.hub['tasks'][hostname][taskname].remove(index)

            if self.hub['engines'][index]['proc'] is not None and \
                    self.hub['engines'][index]['proc'].poll() is None:
                os.kill(self.hub['engines'][index]['proc'].pid,signal.SIGKILL)
            if self.hub['engines'][index]['output'] is not None and \
                    not self.hub['engines'][index]['output'].closed:
                self.hub['engines'][index]['output'].close()

        self.hub['engines'][index] = None

    def __sweep_engines(self):
        for i in range(0,len(self.hub['engines'])):
            if self.hub['engines'][i] is not None and \
                    self.hub['engines'][i]['proc'] is not None and \
                    self.hub['engines'][i]['state'] == 'active':
                if self.hub['engines'][i]['proc'].poll() is not None:
                    ### engine has crashed
                    slot = self.hub['engines'][i]['result']
                    self.hub['results'][slot]['result']['result'] = None
                    self.hub['results'][slot]['result']['state'] = 'crashed'
                    self.hub['engines'][i]['state'] = 'crashed'
                    if not self.hub['engines'][i]['output'].closed:
                        self.hub['engines'][i]['output'].close( )
                    self.hub['engines'][i]['output'] = None
                    self.hub['engines'][i]['proc'] = None
                else:
                    id = self.hub['engines'][i]['id']
                    state = self.hub['remote controller'].status(id)
                    if state['pending'] == 'None':
                        ### task has completed
                        slot = self.hub['engines'][i]['result']
                        if slot is not None:
                            self.hub['results'][slot]['result']['result'] = self.hub['remote controller'].pull(id,'result')
                            self.hub['results'][slot]['result']['state'] = 'done'
                            self.hub['results'][slot]['status'] = copy.deepcopy(self.hub['remote controller'].status(id))
                            self.hub['results'][slot]['engine'] = None
                        self.hub['engines'][i]['result'] = None
                        self.hub['engines'][i]['state'] = 'idle'

    def __initialized(self):
        if self.hub['proc'] is not None and self.hub['proc'].poll() is None:
            return True
        else:
            return False

    def __timer_cancel(self):
        if self.hub['timer'] is not None:
            self.hub['timer'].cancel( )
            self.hub['timer'] = None

    def __timer_verify(self):
        if not self.hub['verified']:
            if not self.__verify( ):
                self.hub['timer'] = threading.Timer(1,taskmanager.__timer_verify,[self])
                self.hub['timer'].start()
        
    def __verify(self):
        self.__timer_cancel( )

        ## check to see if startup was ok, if no exit status
        if self.hub['proc'].poll() is None:
            server_count = 0
            port_re = re.compile(r' starting on (\d+)$')
            log = open(self.__hub_log_path( ),'r')
            for l in log:
                port = re.search(port_re,l)
                if port and operator.contains(self.hub['ports'],int(port.group(1))):
                    server_count += 1
            log.close( )
            if server_count == len(self.hub['ports']):
                self.hub['verified'] = True
                return True
            
        ## if ipcontroller exited, try to find the port where conflict occurs
        if self.hub['proc'].poll() is not None:
            if self.hub['startup trys'] > 20:
                raise RuntimeError, "couldn't find a port"
            self.hub['startup trys'] += 1
            port_re = re.compile(r'Couldn.?\'t listen on any:(\d+):')
            identified_port = False
            err = open(self.__hub_log_path( ),'r')
            for l in err:
                port = re.search(port_re,l)
                if port:
                    self.__update_port(port.group(1))
                    identified_port = True
                    break
            err.close( )
            if not identified_port:
                raise RuntimeError, "couldn't find the port"
            self.__start_hub( )

        self.hub['verified'] = False
        return False

    def __update_port(self,port):
        for i in range(0,len(self.hub['ports'])):
            if self.hub['ports'][i] == int(port):
                self.hub['ports'][i] = self.__new_port( )
                break

    def __hub_log_path(self):
        return self.rcdir + '/hub.' + str(os.getpid()) + '.log'

    def __hub_rc_path(self,create=True):
        rcfile = self.rcdir + '/hubrc.py'
        if create and not os.path.exists(rcfile):
            rcout = open(rcfile,'w')
            rcout.write('import signal\n')
            rcout.write('print "ignore sigint..."\n')
            rcout.write('original_sigint_handler = signal.signal(signal.SIGINT,signal.SIG_IGN)\n')
            rcout.close( )
        return rcfile

    def __engine_log_root(self):
        return self.rcdir + '/engine.' + str(os.getpid())

    def __engine_log_path(self):
        engine_path = self.__engine_log_root( )
        self.__mkdir(engine_path)
        return engine_path + '/' + str(len(self.hub['engines'])+1) + '.log'

    def __new_port(self):
        port_range=[49152,65535]
        result = random.randint(*port_range)
        while operator.contains(self.hub['used ports'],result):
            result = random.randint(*port_range)
        self.hub['used ports'].append(result)
        return result

    def __start_hub(self):
        if self.hub['ports'][0] is None:
            #hub_initial_port = 13225    # one set of unassigned ports: 13225-13719
            #for i in range(0,len(self.hub['ports'])):
            #    self.hub['ports'][i] = hub_initial_port + i
            for i in range(0,len(self.hub['ports'])):
                self.hub['ports'][i] = self.__new_port( )
        if self.hub['output'] is not None and not self.hub['output'].closed:
            self.hub['output'].close()
        self.hub['output'] = open(self.__hub_log_path( ),'w')
        self.hub['proc'] = Popen('ipcontroller --engine-port=' + str(self.hub['ports'][0]) +
                         ' --vanilla-client-port=' + str(self.hub['ports'][1]) +
                         ' --factory-client-port=' + str(self.hub['ports'][2]) +
                         ' --rcfile=' + self.__hub_rc_path( ),
                         stdout=self.hub['output'],stderr=self.hub['output'],shell=True)

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
