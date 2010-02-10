from IPython.kernel import client 
from subprocess import *
import os
import sys
import commands
import string
import atexit
import time
import types
import inspect
from math import *

a=inspect.stack()
stacklevel=0
for k in range(len(a)):
   if a[k][1] == "<string>" or (string.find(a[k][1], 'ipython console') > 0 or string.find(a[k][1],"casapy.py") > 0):
      stacklevel=k

myf=sys._getframe(stacklevel).f_globals

if myf.has_key('casa') :
   casa = myf['casa']
else:
   casa = { }

##scriptdir = '/home/casa-dev-08/dschieb/casapy-test-30.1.10182-001-64b/lib64/python2.5/'
class cluster(object):

   "control cluster engines for parallel tasks"
   
   __client=None
   __controller=None
   __timestamp=None
   __engines=[]
   __ipythondir=os.environ['IPYTHONDIR']
   __homepath=os.environ['HOME'] 
   __start_engine_file='start_engine.sh'
   __stop_node_file='stop_node.sh'
   __stop_engine_file='stop_engine.sh'
   __stop_controller_file='stop_controller.sh'
   __cluster_rc_file='clusterrc.sh'
   __prefix='/tmp/'+os.environ['USER']+'-'
   __init_now=True
   __new_engs=[]
   #__result={}

   def __init__(self):
      '''Initialize a Cluster.

      A Cluster enables parallel and distributed execution of CASA tasks and tools on a set of networked computers. A culster consists of one controller and one or more engines. Each engine is an independent Python instance that takes Python commands over a network connection. The controller provides an interface for working with a set of engines. A user uses casapy console to command the controller. A password-less ssh access to the computers that hosts engines is required for the communication between controller and engines.
      
      '''

      self.__client=None
      self.__controller=None
      self.__timestamp=None
      self.__engines=[]
      self.__ipythondir=os.environ['IPYTHONDIR']
      self.__homepath=os.environ['HOME'] 
      if (self.__ipythondir==None or self.__ipythondir==''):
         os.environ["IPYTHONDIR"]=os.environ['HOME']+'.casa/ipython'
      self.__ipythondir=os.environ['IPYTHONDIR']
      self.__init_now=True
      self.__new_engs=[]
      #print 'ipythondir', self.__ipythondir
      #print 'HOME:', self.homepath
      atexit.register(cluster.stop_cluster,self)

   def start_engine(self, node_name, num_engine, work_dir=None):
      '''Start engines on the given node.

      @param node_name The name of the computer to host the engines.
      @param num_engine The number of the engines to initialize for this run. 
      @param work_dir The working directory where outputs and logs from the engines will be stored. If work_dir is not supplied or does not exist, the user's home directory will be used. 
      Running this command multiple times on the same node is ok. The total number of the engines on the node increases for each run.
      Every engine has a unique integer id. The id is the key to send the instructions to the engine. The available engine ids can be obtained by calling get_ids() or get_engines().
 
      '''

      # start controller
      if not self.__start_controller():
         print 'controller is not started'
         return False

      # start the engine 
      #print 'controller', controller
      #print 'engines', self.engines
      out=open('/dev/null', 'w')
      err=open('/dev/null', 'w')
      dist=node_name+':'+self.__prefix+self.__cluster_rc_file
      #print 'dist=', dist
      p=Popen(['scp', self.__ipythondir+'/'+self.__cluster_rc_file, dist], stdout=out, stderr=err)
      dist=node_name+':'+self.__prefix+self.__start_engine_file
      #print 'dist=', dist
      p=Popen(['scp', self.__ipythondir+'/'+self.__start_engine_file, dist], stdout=out, stderr=err)
      out.close()
      err.close()
      sts = os.waitpid(p.pid, 0)
      for i in range(1, num_engine+1):
         args='bash '+self.__prefix+self.__start_engine_file
         #print 'args=', args
         q=Popen(['ssh', '-f', '-q', '-x', node_name, args])
         sts = os.waitpid(q.pid, 0)
         print "start engine %s on %s" % (i, node_name)
      self.__engines=self.__update_cluster_info(num_engine, work_dir)
      
   def start_cluster(self, cl_file):
      '''Start engines that listed in a file

      @param cl_file The name of the file that defines the engines.
      The cl_file is a text file. Each line contains 3 columns with node name, number of engines and work directory separated by space. A line started with # will be ignored. Example:
      #-----------------------------------------
      #node_name num_of_engines work_dir
      casa-dev-01 4 /home/casa-dev-01/hye/cluster
      #casa-dev-02 3 /home/casa-dev-02/hye/cluster
      subzero 1 /home/subzero/hye/test
      #olddog  2 /home/olddog/hye
      #-----------------------------------------

      start_cluster and start_engine can be used multiple times. 

      '''

      # start controller
      if not self.__start_controller():
         print 'controller is not started'
         return False
 
      # process the file
      #os.path.isfile(cl_file)
      try:
         clf=open(cl_file, 'r')
         lines = clf.readlines()
         #print lines
         #print len(lines)
         for line in lines:
            #print line
            if line.startswith('#'):
               continue
            words = string.split(line)
            if len(words) < 3:
               print "the node definition '%s' is invalid" % line 
               continue
            try:
               int(words[1])
            except:
               print "the '%s' must be an integer number:" % words[1] 
               continue
               
            # start all nodes
            self.__init_now=False
            self.start_engine(words[0], int(words[1]), words[2])

         clf.close()
      except IOError:
         print "cluster file '%s' doesn't exist" % cl_file

      if len(self.__new_engs)>0:
         self.__init_nodes(self.__new_engs)
         self.__engines=self.__client.pull(['id', 'host', 'pid', 'inited'])
      self.__new_engs=[]
      self.__init_now=True

   def __start_controller(self):
      '''(Internal) Start the controller.
    
      A user does not need to call this function directly. When a user runs either start_cluster or start_engine, it will check the existence of a valid controller. If the controller does not exist, this function will be called auto matically. All engines will connect to the valid controller. 

      '''
      # if there is already a controller, use it
      if (self.__controller!=None):
         #print "the controller %s is already running" % self.__controller
         return True

      from time import strftime
      timestamp=strftime("%Y%m%d%H%M%S")
      lfile=self.__ipythondir+'/log/casacontroller-'+timestamp+'-'
      cmd=commands.getoutput("which ipcontroller")
      self.__controller=Popen([cmd, '--logfile='+lfile]).pid
      #print 'cmd=', cmd, 'pid=', self.__controller
      if (self.__controller==None):
         return False


      self.__timestamp=timestamp
      print "controller %s started" % self.__controller
      self.__write_bashrc()
      self.__write_start_engine()
      self.__write_stop_node()
      self.__write_stop_controller()
      
      info=self.__ipythondir+'/log/casacontroller-'+str(self.__timestamp)+'-'+str(self.__controller)+'.log'
      #print 'info=', info
      for i in range(1, 15):
         #print os.path.exists(info)
         if os.path.exists(info):
            break
         time.sleep(1)
         #print "wait", i 

      self.__client=client.MultiEngineClient()
      #print "__client=", self.__client

      #self.__client.activate()

      return True

   def __write_start_engine(self):
      '''(Internal) Create script for starting engines.

      The created script will be stored in the user's $IPYTHONDIR. The start_cluster and start_engine will upload the script to the node and execute it in the proper shell.

      '''

      ef=open(self.__ipythondir+'/'+self.__start_engine_file, 'w')
      bash=commands.getoutput("which bash")
      ef.write('#!%s\n' % bash)
      #ef.write('. %s/.bashrc\n' % os.environ['HOME'])
      ef.write('. %s%s\n' % (self.__prefix, self.__cluster_rc_file))
      cmd=commands.getoutput("which ipengine")
      ef.write('export contrid=%s\n' % self.__controller)
      ef.write('export stamp=%s\n' % self.__timestamp)
      ef.write(cmd+' --logfile='+self.__ipythondir+'/log/casaengine-'+self.__timestamp+'-'+str(self.__controller)+'- &\n')
      ef.close()

   def __write_stop_node(self):
      '''(Internal) Create script for stoping a node.

      The created script will be stored in the user's $IPYTHONDIR. The stop_cluster and stop_engine will upload the script to the node and execute it in the proper shell.

      '''
      ef=open(self.__ipythondir+'/'+self.__stop_node_file, 'w')
      bash=commands.getoutput("which bash")
      ef.write('#!%s\n' % bash)
      ef.write("ps -fu `whoami` | grep ipengine | grep -v grep | awk '{print $2}' | xargs kill -TERM")
      ef.close()

   def __write_stop_controller(self):
      '''(Internal) Create script for stoping the controller.

      The created script will be stored in the user's $IPYTHONDIR. The stop_cluster will execute it in the proper shell.

      '''
      ef=open(self.__ipythondir+'/'+self.__stop_controller_file, 'w')
      bash=commands.getoutput("which bash")
      ef.write('#!%s\n' % bash)
      ef.write("ps -ef | grep `whoami` | grep ipcontroller | grep -v grep | awk '{print $2}' | xargs kill -TERM")
      ef.close()

   def __write_bashrc(self):
      '''(Internal) Create file containning bash startup instructions for the engine host.

      When the controller startup, the necessary environment information for running cluster is extracted from the user's current shell (that runs this casapy session) and written to a rc file. The created script will be stored in the user's $IPYTHONDIR. The start_cluster and start_engine will upload the rc file to the nodes and establish the engine environment.

      '''
      bashrc=open(self.__ipythondir+'/'+self.__cluster_rc_file, 'w')
      bash=commands.getoutput("which bash")
      bashrc.write("#!%s\n" % bash)
      #for param in os.environ.keys():
      #   bashrc.write("export %s=%s\n" % (param,os.environ[param]))
      envList=['PATH', 'LD_LIBRARY_PATH', 'IPYTHONDIR', 
               'CASAPATH', 'CASAARCH',
               'PYTHONHOME', '__CASAPY_PYTHONDIR',
               'PGPLOT_DEV', 'PGPLOT_DIR', 'PGPLOT_FONT'] 
      for param in envList:
         try:
            bashrc.write('export %s="%s"\n' % (param,os.environ[param]))
         except:
            #print 'environment variable', param, 'not found'
            pass

      bashrc.write("export HOSTNAME=`uname -n`")
      bashrc.close()

   def stop_engine(self, engine_id):
      '''Stop an engine.

      @param engine_id The id of the engine to be stopped.
      If an engine with the given id is in the current cluster, running this function will stop the engine and remove it from the engine list.
      
      '''
       
      node_name=''
      procid=None
      for i in self.__engines:
         if (i[0]==engine_id):
            node_name=i[1]
            procid=i[2]

      if (node_name=='' or procid is None):
         print 'could not find engine %d' % engine_id
         return

      ef=open(self.__ipythondir+'/'+self.__stop_engine_file, 'w')
      bash=commands.getoutput("which bash")
      ef.write('#!%s\n' % bash)
      ef.write("kill -9 %d" % procid)
      ef.close()

      out=open('/dev/null', 'w')
      err=open('/dev/null', 'w')
      dist=node_name+':'+self.__prefix+self.__stop_engine_file
      #print 'dist=', dist
      p=Popen(['scp', self.__ipythondir+'/'+self.__stop_engine_file, dist], stdout=out, stderr=err)
      out.close()
      err.close()
      sts = os.waitpid(p.pid, 0)
      args='bash '+self.__prefix+self.__stop_engine_file
      #print 'args=', args
      Popen(['ssh', '-f', '-q', '-x', node_name, args])
      print 'stop engine %d on %s\n' % (engine_id, node_name)
      self.__engines=self.__update_cluster_info(-1)


   def stop_node(self, node_name):
      '''Stop a node (a engine-host computer)

      @param node_node The node to be stopped.
      If a computer with the given name is in the current cluster, running this function will stop all the engines currently running on that node and remove the node and engines from the engine list. This function will not shutdown the computer.
     
      '''
      out=open('/dev/null', 'w')
      err=open('/dev/null', 'w')
      dist=node_name+':'+self.__prefix+self.__stop_node_file
      #print 'dist=', dist
      p=Popen(['scp', self.__ipythondir+'/'+self.__stop_node_file, dist], stdout=out, stderr=err)
      out.close()
      err.close()
      sts = os.waitpid(p.pid, 0)
      args='bash '+self.__prefix+self.__stop_node_file
      #print 'args=', args
      Popen(['ssh', '-f', '-q', '-x', node_name, args])
      print 'stop engines on %s\n' % node_name
      # what to do with client.kill() ?

      num_engine=0
      for i in self.__engines:
         if i[1]==node_name:
            num_engine=num_engine-1

      self.__engines=self.__update_cluster_info(num_engine)

   def __stop_controller(self):
      '''(Internal) Stop the controller.

      This is the last thing for quiting the cluster gracely. 

      '''
      # if it is already down
      if (self.__controller==None):
         #print "there is no running controller"
         return True

      import commands
      node_name=commands.getoutput("uname -n")
      #node_name=os.environ['HOSTNAME']
      out=open('/dev/null', 'w')
      err=open('/dev/null', 'w')
      dist=node_name+':'+self.__prefix+self.__stop_controller_file
      #print 'dist=', dist
      p=Popen(['scp', self.__ipythondir+'/'+self.__stop_controller_file, dist], stdout=out, stderr=err)
      out.close()
      err.close()
      sts = os.waitpid(p.pid, 0)
      args='bash '+self.__prefix+self.__stop_controller_file
      #print 'args=', args
      Popen(['ssh', '-f', '-q', '-x', node_name, args])

      #cmd="ps -ef | grep `whoami` | grep ipcontroller | grep -v grep | awk '{print $2}' | xargs kill -9"
      #print 'cmd=', cmd
      #Popen(cmd, shell=True)

      try:
         os.remove(self.__ipythondir+'/'+self.__cluster_rc_file)
         os.remove(self.__ipythondir+'/'+self.__start_engine_file)
         os.remove(self.__ipythondir+'/'+self.__stop_node_file)
         os.remove(self.__ipythondir+'/'+self.__stop_controller_file)
         os.remove(self.__ipythondir+'/'+self.__stop_engine_file)
      except:
         pass

      self.__controller=None
      print "controller stopped"
      return True

   def stop_cluster(self):
      '''Stop the cluster

      This function stops all the running engines and the controller.

      '''
      # shutdown all engines
      elist=[]
      for i in self.__engines:
         elist.append(i[1])
      fruit=set(elist)

      self.__engines=[]
      for i in fruit:
         try:
            self.stop_node(i)
         except:
            continue

      # shutdone controller
      self.__stop_controller()
      #self.__controller=None

      #self.__client=None


   def wash_logs(self):
      '''Clean up the cluster log files.

      A set of logs containing controller-engine information will be created every time a cluster is created. This function deletes all cluster log files that cumulated in the user's $IPYTHONDIR, if there is no active cluster running. (The files will be removed only before starting any engine of after stoping the whole cluster.

      '''
      # do this only if no controller running
      if (self.__controller!=None):
         #print "no wash because there is a running controller"
         return True

      os.system("rm -rf %s/log/casacontroller*" % self.__ipythondir)
      os.system("rm -rf %s/log/casaengine*" % self.__ipythondir)
      os.system("rm -rf %s/log/ipcontroller*" % self.__ipythondir)

   def __init_nodes(self, i):
     '''(Internal) Initialize engines

     @param i The list of the engine ids
     An engine is a Python interpreter. To make an engine capable of running CASA tasks and tools, we must setup the environment and import necessary modules. This function effectively make every engine a running CASA instance (except that it is a non-interactive CASA running in Python, in contrast the casapy that is an interactive CASA running in IPython).

     '''
     
     print 'initialize engines', i
     self.__client.push({'casa': casa })
     self.__client.execute('import os', i)
     self.__client.execute('if os.path.isdir(work_dir):os.chdir(work_dir)\nelse:work_dir=os.environ["HOME"]', i)
     phome=''
     try:
       phome=os.environ["PYTHONHOME"]
     except:
       pass
       #print 'environment variable PYTHONHOME not found'
     
     if phome=='':
        try:
           v=str.split(os.environ["CASAPATH"], ' ')
           phome=v[0]+'/'+v[1]
        except:
           pass
           #print 'environment variable CASAPATH not found'

     dhome=''
     try:
       dhome=os.environ["CASAARCH"]
     except:
       pass
       #print 'environment variable CASAARCH not found'

     if phome=='':
        print 'could not locate casa_in_py.py'
        return None

     # this for the release 
     #self.__client.push(dict(phome=phome), i)
     #self.__client.execute('import sys', i)
     #import platform
     #if (platform.architecture()[0]=='64bit'):
     #   self.__client.execute('scriptdir=phome+"/lib64/python2.5/"', i)
     #else:
     #   self.__client.execute('scriptdir=phome+"/lib/python2.5/"', i)
     #self.__client.execute('sys.path.insert(2, scriptdir)', i)

     #print phome, dhome
     if (dhome!=phome):
        phome=dhome

     sdir='/CASASUBST/python_library_directory/'+'/'
     ##sdir='/home/casa-dev-08/dschieb/casapy-test-30.1.10182-001-64b/lib64/python2.5/'
     self.__client.push(dict(phome=phome), i)
     self.__client.execute('import sys', i)
     #self.__client.execute('scriptdir=phome+"/python/2.5/"', i)
     self.__client.push(dict(sdir=sdir), i)
     self.__client.execute('scriptdir=sdir', i)

     #print self.__client.pull('scriptdir')

     self.__client.execute('sys.path.insert(2, scriptdir)', i)
     try:
        #self.__client.execute('from casa_in_py import *')
        self.__client.execute("execfile(scriptdir+'casa_in_py.py')", i)
        self.__client.execute('inited=True', i)
     except IOError, e:
        print "could not initialize the node\n", e

   def reset_cluster(self):
      '''Re-initialize the engines.

      This function reset the running environment for all the available engines.
      
      '''
      if self.__client is None:
         pass

      tobeinit=self.__client.pull('id')
      #print tobeinit

      if len(tobeinit)>0:
         self.__init_nodes(tobeinit)
         self.__engines=self.__client.pull(['id', 'host', 'pid', 'inited'])

   def __update_cluster_info(self, num_engine, work_dir=None):
      '''(Internal) Construct the list of engines.

      @param num_engine The number of new engines 
      @param work_dir The initial working directory 
      This function appends num_engine engines to the engine list and setup initial Python environment on them. Before further initialization, an engine can only run Python programs (it can not run CASA tasks or tools).

      '''

      if self.__client is None :
         print "controller client is not running" 
         return [] 

      #self.__client.activate()

      engs=len(self.__engines)+num_engine
      if engs<0:
         engs=0
      i=0
      idlist=self.__client.get_ids()
      while (len(idlist)!=engs and i<10):
         idlist=self.__client.get_ids()
         time.sleep(1)
         i=i+1
         #print idlist
         
      # here we only take care of the quick-init-abel
      # items. the init of real casa_in_py will be
      # done in parallel 
      tobeinit=[]
      for i in idlist:
         inited=False
         try:
            inited=self.__client.pull('inited', i)
         except:
            #not inited
            tobeinit.append(i)
            self.__client.execute('id=%d'%i, i)
            self.__client.execute('import os', i)
            self.__client.execute('import socket', i)
            self.__client.execute('host=socket.gethostname()', i)
            self.__client.execute('pid=os.getpid()', i)
            self.__client.execute('job=None', i)
            self.__client.execute('import signal', i)
            self.__client.execute('original_sigint_handler = signal.signal(signal.SIGINT,signal.SIG_IGN)', i)

            if work_dir!=None and os.path.isdir(work_dir):
               self.__client.push(dict(work_dir=work_dir), i)
            else:
               self.__client.execute('work_dir=os.environ["HOME"]', i)

            # these are environment variabls set for each 
            # node at startup. it may be better to set as
            # global in this module then pass to each 
            # engine when update_cluster_info

            self.__client.execute('contrid=os.environ["contrid"]', i)
            self.__client.execute('stamp=os.environ["stamp"]', i)
            self.__client.execute('inited=False', i)
            #print 'set attribute on engine', i

      self.__new_engs.extend(tobeinit)

      if self.__init_now:
         if len(self.__new_engs)>0:
            self.__init_nodes(self.__new_engs)

         self.__init_now=True
         self.__new_engs=[]

      if len(idlist)>0:
         #print self.__client.pull(['id', 'host', 'pid', 'contrid', 'stamp', 'inited', work_dir])
         return self.__client.pull(['id', 'host', 'pid', 'inited'])
      else:
         return []

   def get_casalogs(self):
      '''Get a list of the casa logs for all the current cluster engines.

      Each working engine is a CASA instance and saves its own log. This function retrun the list of logs with their full path. One can view the log contents with casalogviewer.

      '''
      #self.__client.pull(['work_dir', 'thelogfile'])
      self.__client.execute('tmp=work_dir+"/"+thelogfile')
      return self.__client.pull('tmp')

   def read_casalogs(self):
      '''Read the casa log files.

      The current implementation of this function is only a prototype. A multi-log viewer needs to be developed.

      '''
      import os
      import string
      files = string.join(self.get_casalogs(), ' ')
      #print files
      os.system("emacs "+files+ "&")

   def pad_task_id(self, b='', task_id=[]):
      # pad task id (not engine id!)
      '''Generate a dictionary of id-padded variables 

      @param b The base name to be padded
      @param task_id A list of integers to pad the base name 
      One way of distributing varaibles to a set of engnines is through python a dictionary. This is a convenience function for quick generating a dictionary of padded names. Example:
      x=c.pad_task_id('basename', [3, 5, 8])
      x
      {3: 'basename-3', 5: 'basename-5', 8: 'basename-8'}
     
      '''

      base={} 

      int_id=True
      for j in task_id:
         if type(j)!=types.IntType or j<0:
            print 'task id', j , 'must be a positive integer'
            int_id=False
            break
      if not int_id:
         return base

      if len(task_id)==0:
         task_id=list(xrange(0, len(self.__engines))) 
      for j in task_id:
         base[j]=b+'-'+str(j)
      return base
       
   def one_to_n(self, arg, task_id=[]):
      '''Genrate a dictionary of one variable for n keys

      @param arg The variable to be distributed
      @param task_id The list of integer ids
      One way of distributing varaibles to a set of engnines is through python a dictionary. This is a convenience function for quick generating a dictionary of same variable for n keys. Example:
      x=c.one_to_n('basename', [1, 2, 7])
      x
      {1: 'basename', 2: 'basename', 7: 'basename'}

      '''
      # assign 1 value to n targets
      base={} 
      int_id=True
      for j in task_id:
         if type(j)!=types.IntType or j<0:
            print 'task id', j , 'must be a positive integer'
            int_id=False
            break
      if not int_id:
         return base

      if len(task_id)==0:
         task_id=list(xrange(0, len(self.__engines))) 
      for j in task_id:
         base[j]=arg
      return base
       
   def n_to_n(self, args=[], task_id=[]):
      '''Generate a dictionary of n varables

      @param arags A list of n variables
      @param task_id A list of n integer ids
      One way of distributing varaibles to a set of engnines is through python a dictionary. This is a convenience function for quick generating a dictionary of a set of n variables for n keys. Example:
      x=c.n_to_n(['a', 'b', 'c'], [3, 6, 7])
      x
      {3: 'a', 6: 'b', 7: 'c'}

      '''
      # assign n value to n targets
      base={} 

      if len(args)==0:
         return base

      int_id=True
      for j in task_id:
         if type(j)!=types.IntType or j<0:
            print 'task id', j , 'must be a positive integer'
            int_id=False
            break
      if not int_id:
         return base

      if len(task_id)==0:
         task_id=list(xrange(0, len(self.__engines))) 

      i=-1
      for j in task_id:
         i=i+1
         if i==len(args):
            break
         base[j]=args[i]
      return base

   def split_channel(self, spw, nchan, task_id=[]):
      '''Generate a dictionary to distribute the spectral windows

      @param spw The spectral window
      @param nchan The number of channels to split
      @param task_id The list of integer ids
      One way of distributing a spectral windows to a set of engnines is through python a dictionary. This is a convenience function for quick generating a dictionary of spw expressions. Example:
      x=c.split_channel(1, 127, [2,3,4])
      x
      {0: '1:0~42', 1: '1:43~85', 2: '1:86~128'}

      '''
      base={}
      if len(task_id)==0:
         task_id=list(xrange(0, len(self.__engines))) 

      if len(task_id)==0:
         print "no engines available, no split done"
         return base

      if nchan<len(task_id):
         print "no enough channels to split"
         return base

      nx=1
      try:
         nx=int(ceil(abs(float(nchan))/len(task_id)))
      except:
         pass

      #print nx, nchan, len(task_id)
      i=-1
      for j in task_id:
         i=i+1
         if i==len(task_id):
            break
         st=i*nx
         se=st+nx-1
         base[i]=str(spw)+":"+str(st)+"~"+str(se)
      return base
       
   def pgc(self,*args,**kwargs):
      '''Parallel execution of commands and/or dictionary of commands


      '''

      tasks={}
      for j in self.__client.get_ids():
         tasks[j]=[]
      
      #print '-' * 10, 'args:'
      for i in args:
         #print i
         if type(i)==types.DictType:
            for j in i.keys():
               if type(j)!=types.IntType or j<0:
                  print 'task id', j , 'must be a positive integer'
                  #return None
                  pass
               else:
                  st=''
                  if type(i[j])==types.StringType:
                     #st="'"+i[j]+"'"
                     st=i[j]
                  else:
                     #ignore it? or let error bounce 
                     #st="'"+str(i[j])+"'"
                     pass
                  if st!='':
                     tasks[j].append(st)
         elif type(i)==types.StringType:
            #for all tasks
            for j in xrange(0, len(self.__engines)): 
               tasks[j].append(i)
         else:
            print 'command', i, 'must be a string or a dictionary'

      #may be better to use non-block mode and catch the result
     
      keys = kwargs.keys()
      job='NoName'
      block=True
      for kw in keys:
         #print kw
         if kw.lower()=='job' or kw.lower()=='jobname':
            job=kwargs[kw]
         if kw.lower()=='block':
            block=kwargs[kw]
 
      #print "tasks", tasks
      for i in tasks.keys():      
         cmd=string.join(tasks[i], '\n')
         self.__client.push(dict(job=cmd), i)
         #print 'cmd:', cmd, 'i:', i
         #self.__client.execute(cmd, block=False, i)

      return self.__client.execute('exec(job)',block=block,targets=tasks.keys())
      #self.__result[job]=self.__client.execute('exec(job)',block=block,targets=tasks.keys())
      #return self.__client.execute('exec(job)',block=False,targets=tasks.keys())

   def parallel_go_commands(self,*args,**kwargs):
      '''Parallel execution of commands and/or dictionary of commands


      '''
      self.pgc(*args,**kwargs)
 
   def pgk(self, **kwargs):
      '''Parallel execution to set keywords


      '''

      # set default action
      pgTask=None
      pgTimer=False
      pgAction='run'
      pgBlock=False
      pgJob="NoName"

      tasks={}
      for j in self.__client.get_ids():
         tasks[j]=[]
      
      #print '-' * 10, 'kargs:'
      keys = kwargs.keys()
      #keys.sort()

      for kw in keys:
         #print kw
         if kw.lower()=='action':
            pgAction=kwargs[kw]
         if kw.lower()=='timer':
            pgTimer=True
         if kw.lower()=='task' and type(kwargs[kw])==types.StringType:
            pgTask=kwargs[kw]
         if kw.lower()=='block' and kwargs[kw]==True:
            pgBlock=True
         if (kw.lower()=='job' or kw.lower()=='jobname') and type(kwargs[kw])==types.StringType :
            pgJob=kwargs[kw]

      if type(pgAction)!=types.StringType or (pgAction.lower()!='run' and pgAction.lower()!='check'):
         pgAction='write'
      #print 'pgAction', pgAction

      for kw in keys: 
         vals=kwargs[kw]
         print kw, ":", vals
         if type(vals)==types.DictType:
            for j in vals.keys():
               if type(j)!=types.IntType or j<0:
                  print 'task id', j , 'must be a positive integer'
                  return None
               else:
                  st=kw+'='
                  if type(vals[j])==types.StringType:
                     st=st+"'"+vals[j]+"'"
                  else:
                     st=st+str(vals[j])
                  tasks[j].append(st)
         else:
            if kw=='task':
               pgTask=vals
            elif kw=='action' or kw=='timer':
               pass
            else:
               for i in xrange(0, len(self.__engines)): 
                  st=kw+'='
                  if type(vals)==types.StringType:
                     st=st+"'"+vals+"'"
                  else:
                     st=st+str(vals)
                  tasks[i].append(st)

      if pgAction=='write':
         if (pgTask==None or pgTask==''):
            for i in tasks.keys():      
               print i, string.join(tasks[i], ',') 
         else:
            for i in tasks.keys():      
               print i, pgTask+"("+string.join(tasks[i], ',')+")" 
         return None
      
      if pgAction=='check':
         if (pgTask!=None and pgTask!=''):
            print 'use update_params or like to check params for each task'
            for i in tasks.keys():      
               cmd=pgTask+"("+string.join(tasks[i], ',')+")" 
         return None
               
      if pgAction=='run':
         #print 'run keyword'
         if (pgTask==None or pgTask==''):
            for i in tasks.keys():      
               cmd=string.join(tasks[i], '\n')
               #print 'cmd', cmd
               #self.__client.execute(cmd, i)
               self.__client.push(dict(job=cmd), i)

            #print 'cmd', cmd
            return self.__client.execute('exec(job)',block=pgBlock,targets=tasks.keys())
            #self.__result[job]=self.__client.execute('exec(job)',block=pgBlock,targets=tasks.keys())
           
         else:
            for i in tasks.keys():      
               cmd=pgTask+"("+string.join(tasks[i], ',')+")" 
               #self.__client.execute(cmd, i)
               self.__client.push(dict(job=cmd), i)

            #print 'cmd', cmd
            #self.__result[job]=self.__client.execute('exec(job)',block=pgBlock,targets=tasks.keys())
            return self.__client.execute('exec(job)',block=pgBlock,targets=tasks.keys())
         return
               
   def parallel_go_keywords(self, **kwargs):
      '''Parallel execution to set keywords


      '''
      self.pgk(**kwargs)

   def hello(self):
      '''Parallel execution to print 'hello' message from all engines


      '''
      print "Hello CASA Controller"
      return self.__client.execute('print "Hello CASA Node"')

   def __set_cwds(self, clusterdir):
      '''Set current working dir for all engines


      '''
      # this is not very useful because dirs are generally
      # different cross nodes 
      self.__client.execute('import os')
      self.__client.push(dict(clusterdir=clusterdir))
      self.__client.execute('os.chdir(clusterdir)')
      self.__client.execute("user=os.environ['USER']")
      self.__client.execute('print user')
      #self.__client.execute('import commands')
      #self.__client.execute('(exitstatus, outtext) = commands.getstatusoutput("uname -n")')
      #return self.__client.gather('outtext')
      self.__client.execute('import socket')
      self.__client.execute('host=socket.gethostname()')
      self.__client.execute('print host')
      self.__client.execute('print os.getcwd()')

   #def get_ids(cluster):
   #   return cluster._cluster__client.get_ids()
   def get_ids(self):
      '''get ids for all available engines


      '''
      return self.__client.get_ids()

   def get_nodes(self):
      '''get hostnames for all available engines


      '''
      from sets import Set 
      elist=[]
      for i in self.__engines:
         elist.append(i[1])
      return list(Set(elist))

   def get_engines(self):
      '''get current status of the engines


      '''
      return self.__engines

   def get_stdout(self,cmd):
      '''get the standard output from all engines for execting a comment


      '''
      return commands.getstatusoutput(cmd)      

   def pdo(self,job):
      '''parallel execution of a job


      '''
      return self.__client.execute(job)

   def odo(self,job,nodes):
      '''execute a job on a subset of engines


      '''
      return self.__client.execute(job,block=False,targets=nodes)
  
   def queue_status(self):
      '''query to queue status


      '''
      return self.__client.queue_status()

   def clear_queue(self):
      '''remove all jobs from the queue


      '''
      return self.__client.clear_queue()

   def get_properties(self):
      '''get the set properties from all engines


      '''
      return self.__client.get_properties()

   def keys(self):
      '''get all keys from all engines


      '''
      return self.__client.keys()

   def pull(self, key, targets='all'):
      '''get the value of a key
      @param key the var of interest
      @param targets, the engines of interest


      '''
      return self.__client.pull(key,targets)

   def get_result(self, i):
      '''get the result of previous execution


      '''
      return self.__client.get_result(i)

   def activate(self):
      '''set the cluster to parallel execution mode

  
      '''
      return self.__client.activate()

   def check_job(self, job, verbose=True):
      '''check the status of an asynch job

  
      '''
      try:
         x=job.get_result(block=False)
         if x==None:
            if verbose:
               print "job '%s' has not finished yet, result is pending" % job
            return False
         else:
            if verbose:
               print "job '%s' done" % job
            return True
      except:
         print "could not get the status of job '%s'" % job
         return False

   def howto(self):
      print '''A simple example for use the cluster
from parallel_go import *
c=cluster()
c.start_engine('casa-dev-08',2,'/home/casa-dev-08/hye/cluster')
#can use tb.clearlocks() to remove leftover
c.pgc('default("split")')
c.pgc('inp("split")')
c.pgk(mspath='/home/casa-dev-01/hye/test/')
c.pgk(msfile='ngc5921.ms')
c.pgc('vis=mspath+msfile')
c.pull('vis')
tb.clearlocks('/home/casa-dev-01/hye/test/5921.ms')
c.pgc('outputvis=work_dir+"/"+msfile+"-"+str(id)')
#alternatively 
#for i in c.get_ids():
#    p=c.pull('work_dir')[i]
#    f=c.pull('msfile')[i]
#    o=p+"/"+f+"--"+str(i)
#    c.pgk(outputvis={i: o})
c.pull('outputvis')
c.pgc('field="2"')
spw=c.split_channel(0, 64)
spw[0]='0:6~15'
spw[3]='0:48~55'
c.pgk(spw=spw)
c.pgc('inp("split")')
c.pgc('go("split")')
c.read_casalogs()
c.pgc('inp("clean")')
c.pgc('vis=outputvis')
c.pgk(imagetag='.clean')
c.pgc('imagename=vis+imagetag')
c.pgc('inp("clean")')
c.pgc('go("clean")')
c.pgc('import commands')
c.pgc('a=commands.getstatusoutput("ls ")')
'''

   def use_often(self):
      print '''Frequently used commands
from parallel_go import *
c=cluster()
c.hello()
c.get_ids()
c.get_nodes()
c.activate()
px print "cluster activated"
c.pdo 'print "parallel"'
c.odo('print "node 0 only"', 0)
c.odo('print "node 0 and 1"', [0,1])
c.odo 'print "node 1 only"', 1
c.queue_status()
c.clear_queue()
c.get_properties()
c.keys()
c.pull('mpi')
c.get_result(1)

#pg.activate()
#px 'from casa_in_py import *'
'''

'''
for i in 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16;
do ssh casa-dev-$i "ps -ef | grep hye" ; done
'''
'''
c.pgc('import time', {0: 'time.sleep(10); x=5; y="y is y"', 1: 'time.sleep(12);a=7;b="b is not y"'},block=False,job="wakeup")

c.check_job('wakeup')
c.pull('x',0)
c.pull('y',0)
c.pull('a',1)
c.pull('b',1)
c.odo('print x', 0)
c.odo('print x', 0).r

'''
