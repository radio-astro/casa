from IPython.kernel import client 
from subprocess import *
import os
import sys
import commands
import string
import atexit
import time
import socket
import types
import inspect
import casadef
import numpy as np
from math import *
from get_user import get_user
# jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
import traceback
# jagonzal (CAS-4372): Introduce CASA logging system into cluster infrastructure
from casac import *
casalog = casac.logsink()
casalog.setglobal(True)

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

class cluster(object):

   "control cluster engines for parallel tasks"

   _instance = None
   
   __client=None
   __controller=None
   __timestamp=None
   __engines=[]
   __ipythondir=os.environ['PWD']+'/ipython'
   if(os.environ.has_key('IPYTHONDIR')):
      __ipythondir=os.environ['IPYTHONDIR']
   __homepath=os.environ['HOME'] 
   __start_controller_file='start_controller.sh'   
   __start_engine_file='start_engine.sh'
   __stop_node_file='stop_node.sh'
   __stop_engine_file='stop_engine.sh'
   __stop_controller_file='stop_controller.sh'
   __cluster_rc_file='clusterrc.sh'
   __user = get_user()
   __prefix = '/tmp/' + __user + '-'
   __init_now=True
   __new_engs=[]

   def __new__(cls, *args, **kwargs):
       if not cls._instance:
           cls._instance = super(cluster, cls).__new__(
                                cls, *args, **kwargs)
       return cls._instance

   def __call__(self):
       
       # If there is already a controller, use it
       return self

   def __init__(self):
      """Initialize a Cluster.

      A Cluster enables parallel and distributed execution of CASA tasks and tools on a set of networked computers. A culster consists of one controller and one or more engines. Each engine is an independent Python instance that takes Python commands over a network connection. The controller provides an interface for working with a set of engines. A user uses casapy console to command the controller. A password-less ssh access to the computers that hosts engines is required for the communication between controller and engines.

      """

      self.__client=None
      self.__controller=None
      self.__timestamp=None
      self.__engines=[]
      self.__ipythondir=os.environ['PWD']+'/ipython'
      if(os.environ.has_key('IPYTHONDIR')):
         self.__ipythondir=os.environ['IPYTHONDIR']
      else:
         os.environ['IPYTHONDIR']=self.__ipythondir
      self.__homepath=os.environ['HOME'] 
      if (self.__ipythondir==None or self.__ipythondir==''):
         os.environ["IPYTHONDIR"]=os.environ['HOME']+'.casa/ipython'
      self.__ipythondir=os.environ['IPYTHONDIR']
      self.__init_now=True
      self.__new_engs=[]
      atexit.register(cluster.stop_cluster,self)

   def _ip(self, host):
      """Returns a unique IP address of the given hostname,
      i.e. not 127.0.0.1 for localhost but localhost's global IP"""
      
      ip = socket.gethostbyname(host)
      
      if ip == "127.0.0.1":
         ip = socket.gethostbyname(socket.getfqdn())
         
      return ip

   def _cp(self, source, host, destination):
      """Creates the command to copy the source file to the destination file and destination host,
      using either scp or cp for the localhost. This is to avoid the requirement of password-less ssh
      in a single host environment."""

      if self._ip(host) == self._ip("localhost"):
         cmd = ['cp', source, destination]
      else:
         cmd = ['scp', source, host + ":" + destination]

      return cmd

   def _do(self, host, cmd):
      """Creates the command line to execute the give command on the given host.
      If and only if the host is not localhost, ssh is used."""

      if self._ip(host) == self._ip("localhost"):
         return cmd.split(" ")
      else:
         return ['ssh', '-f', '-q', '-x', host, cmd]

   def start_engine(self, node_name, num_engine, work_dir=None, omp_num_nthreads=1):
      """Start engines on the given node.
      @param node_name The name of the computer to host the engines.
      @param num_engine The number of the engines to initialize for this run. 
      @param work_dir The working directory where outputs and logs from the engines will be stored. If work_dir is not supplied or does not exist, the user's home directory will be used. 
      Running this command multiple times on the same node is ok. The total number of the engines on the node increases for each run.
      Every engine has a unique integer id. The id is the key to send the instructions to the engine. The available engine ids can be obtained by calling get_ids() or get_engines().
      """
      
      casalog.origin("parallel_go")
      
      # Start controller
      if not self.__start_controller():
         casalog.post("The controller is not started","WARN","start_engine")
         return False

      # Start the engine 
      out=open('/dev/null', 'w')
      err=open('/dev/null', 'w')
      cmd = self._cp(self.__ipythondir+'/'+self.__cluster_rc_file,
                node_name,
                self.__prefix+self.__cluster_rc_file)
      p=Popen(cmd, stdout=out, stderr=err)
      sts = os.waitpid(p.pid, 0)
      if sts[1] != 0:
          casalog.post("Command failed: %s" % (" ".join(cmd)),"WARN","start_engine")
      
      cmd = self._cp(self.__ipythondir+'/'+self.__start_engine_file,
                node_name,
                self.__prefix+self.__start_engine_file)

      p=Popen(cmd, stdout=out, stderr=err)
      sts = os.waitpid(p.pid, 0)
      if sts[1] != 0:
         casalog.post("Command failed: %s" % (" ".join(cmd)),"WARN","start_engine")
      for i in range(1, num_engine+1):
         args='bash '+self.__prefix+self.__start_engine_file
         cmd = self._do(node_name, args)
         q=Popen(cmd)
         sts = os.waitpid(q.pid, 0)
         if sts[1] != 0:
            casalog.post("Command failed: %s" % (" ".join(cmd)),"WARN","start_engine")
         casalog.post("start engine %s on %s" % (i, node_name),"INFO","start_engine")
      self.__engines=self.__update_cluster_info(num_engine, work_dir,omp_num_nthreads)
      
      out.close()
      err.close()

   # jagonzal (CAS-4292): This method crashes when initializing the nodes via __init_nodes,
   # so it is deprecated. Instead it is necessary to use directly the start_engine method 
   # which does not only start the engine, but also initializes it using scripts
   def start_cluster(self, cl_file):
      """Start engines that listed in a file

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

      """
      
      casalog.origin("parallel_go")

      # Start controller
      if not self.__start_controller():
         casalog.post("The controller is not started","WARN","start_cluster")
         return False
 
      # Process the file
      try:
         clf=open(cl_file, 'r')
         lines = clf.readlines()
         for line in lines:
            if line.startswith('#'):
               continue
            words = string.split(line)
            if len(words) < 3:
                casalog.post("The node definition is invalid: %s" % line,"WARN","start_cluster")
                continue
            try:
               int(words[1])
            except:
               # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
               # traceback.print_tb(sys.exc_info()[2])
               continue
               
            # Start all nodes
            self.__init_now=False
            casalog.post("start_engine(%s,%s,%s)" % (str(words[0]),str(words[1]),str(words[2])),"INFO","start_cluster")
            self.start_engine(words[0], int(words[1]), words[2])

         clf.close()
      except IOError:
         casalog.post("Cluster file '%s' doesn't exist" % cl_file,"SEVERE","start_cluster")

      if len(self.__new_engs)>0:
         self.__init_nodes(self.__new_engs)
         self.__engines=self.__client.pull(['id', 'host', 'pid', 'inited'])
      self.__new_engs=[]
      self.__init_now=True
  
   def __start_controller(self):
      """(Internal) Start the controller.
      
      A user does not need to call this function directly. When a user runs either start_cluster or start_engine, it will check the existence of a valid controller. If the controller does not exist, this function will be called auto matically. All engines will connect to the valid controller. 

      """
      
      casalog.origin("parallel_go")
       
      # If there is already a controller, use it
      if (self.__controller!=None):
         return True
     
      # First of all write bashrc file which is needed by other cluster files
      self.__write_bashrc()     

      # Generate time stamp and write start controller file
      from time import strftime
      timestamp=strftime("%Y%m%d%H%M%S")
      self.__write_start_controller(timestamp) 
      
      # Start controller in a detached terminal
      cmd = 'bash ' + self.__ipythondir + '/' + self.__start_controller_file
      self.__controller=Popen(cmd,shell=True).pid
      if (self.__controller==None):
         return False
      self.__timestamp=timestamp
      casalog.post("Controller %s started" % self.__controller  ,"INFO","start_controller")
      
      # Now write the rest of the cluster files
      self.__write_start_engine()
      self.__write_stop_controller()
      self.__write_stop_node()          
      
      # Wait for controller files to exist
      info=self.__ipythondir+'/log/casacontroller-'+str(self.__timestamp)+'-'+str(self.__controller)+'.log'
      meng=self.__ipythondir+'/security/casacontroller-mec-'+self.__timestamp+'.furl'

      for i in range(1, 15):
         if os.path.exists(info):
            break
         time.sleep(1)

      for i in range(1, 15):
         if os.path.exists(meng):
            break
         time.sleep(1)

      # Start-up client
      self.__client=client.MultiEngineClient(meng)

      return True

   def __write_start_engine(self):
      """(Internal) Create script for starting engines.

      The created script will be stored in the user's $IPYTHONDIR. The start_cluster and start_engine will upload the script to the node and execute it in the proper shell.

      """

      ef=open(self.__ipythondir+'/'+self.__start_engine_file, 'w')
      bash=commands.getoutput("which bash")
      ef.write('#!%s\n' % bash)
      ef.write('. %s%s\n' % (self.__prefix, self.__cluster_rc_file))
      cmd=commands.getoutput("which ipengine")
      ef.write('export contrid=%s\n' % self.__controller)
      ef.write('export stamp=%s\n' % self.__timestamp)
      ef.write(cmd+' --furl-file='+self.__ipythondir+'/security/casacontroller-engine-'+self.__timestamp+'.furl --logfile='+self.__ipythondir+'/log/casaengine-'+self.__timestamp+'-'+str(self.__controller)+'- 2>&1 | grep -v NullSelection &\n')
      ef.close()
      
   def __write_start_controller(self,timestamp):
      """

      """

      ef=open(self.__ipythondir+'/'+self.__start_controller_file, 'w')
      bash=commands.getoutput("which bash")
      ef.write('#!%s\n' % bash)
      ef.write('. %s/%s\n' % (self.__ipythondir, self.__cluster_rc_file))
      lfile=self.__ipythondir+'/log/casacontroller-'+timestamp+'-'
      ffile=self.__ipythondir+'/security/casacontroller-engine-'+timestamp+'.furl'
      efile=self.__ipythondir+'/security/casacontroller-mec-'+timestamp+'.furl'
      tfile=self.__ipythondir+'/security/casacontroller-tc-'+timestamp+'.furl'
      cmd = commands.getoutput("which ipcontroller")
      cmd += ' -xy '
      cmd += ' --engine-furl-file=' + ffile
      cmd += ' --multiengine-furl-file=' + efile
      cmd += ' --task-furl-file=' + tfile
      cmd += ' --logfile=' + lfile
      cmd += ' &\n'
      ef.write(cmd)
      ef.close()

   def __write_stop_node(self):
      """(Internal) Create script for stoping a node.

      The created script will be stored in the user's $IPYTHONDIR. The stop_cluster and stop_engine will upload the script to the node and execute it in the proper shell.

      """
      ef=open(self.__ipythondir+'/'+self.__stop_node_file, 'w')
      bash=commands.getoutput("which bash")
      ef.write('#!%s\n' % bash)
      # Stop all engines started by the current controller
      ef.write("ps -fu `whoami` | grep ipengine | grep -v grep | grep "+self.__timestamp+" | awk '{print $2}' | xargs kill -TERM>/dev/null")
      ef.close()

   def __write_stop_controller(self):
      """(Internal) Create script for stoping the controller.

      The created script will be stored in the user's $IPYTHONDIR. The stop_cluster will execute it in the proper shell.

      """
      ef=open(self.__ipythondir+'/'+self.__stop_controller_file, 'w')
      bash=commands.getoutput("which bash")
      ef.write('#!%s\n' % bash)
      ef.write("ps -ef | grep `whoami` | grep ipcontroller | grep -v grep | awk '{print $2}' | xargs kill -TERM >/dev/null")
      ef.close()

   def __write_bashrc(self):
      """(Internal) Create file containning bash startup instructions for the engine host.

      When the controller startup, the necessary environment information for running cluster is extracted from the user's current shell (that runs this casapy session) and written to a rc file. The created script will be stored in the user's $IPYTHONDIR. The start_cluster and start_engine will upload the rc file to the nodes and establish the engine environment.

      """
      bashrc=open(self.__ipythondir+'/'+self.__cluster_rc_file, 'w')
      bash=commands.getoutput("which bash")
      bashrc.write("#!%s\n" % bash)
      envList=['PATH', 'LD_LIBRARY_PATH', 'IPYTHONDIR', 
               'CASAPATH', 'CASAARCH',
               'PYTHONHOME', '__CASAPY_PYTHONDIR',
               'PGPLOT_DEV', 'PGPLOT_DIR', 'PGPLOT_FONT'] 
      for param in envList:
         try:
            bashrc.write('export %s="%s"\n' % (param,os.environ[param]))
         except:
            # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
            # traceback.print_tb(sys.exc_info()[2])
            pass

      bashrc.write("export HOSTNAME=`uname -n`")
      bashrc.close()

   def stop_engine(self, engine_id):
      """Stop an engine.

      @param engine_id The id of the engine to be stopped.
      If an engine with the given id is in the current cluster, running this function will stop the engine and remove it from the engine list.
      
      """
      
      casalog.origin("parallel_go")
       
      if type(engine_id).__name__ != 'int':
          casalog.post("engine id must be an integer","WARN","stop_engine")
          return None 

      node_name=''
      procid=None
      for i in self.__engines:
         if (i[0]==engine_id):
            node_name=i[1]
            procid=i[2]

      if (node_name=='' or procid is None):
          casalog.post("Could not find engine %d" % engine_id,"WARN","stop_engine")
          return

      ef=open(self.__ipythondir+'/'+self.__stop_engine_file, 'w')
      bash=commands.getoutput("which bash")
      ef.write('#!%s\n' % bash)
      ef.write("kill -9 %d" % procid)
      ef.close()

      out=open('/dev/null', 'w')
      err=open('/dev/null', 'w')
      cmd = self._cp(self.__ipythondir+'/'+self.__stop_engine_file,
                node_name,
                self.__prefix+self.__stop_engine_file)

      p=Popen(cmd, stdout=out, stderr=err)
      out.close()
      err.close()
      sts = os.waitpid(p.pid, 0)
      args='bash '+self.__prefix+self.__stop_engine_file
      Popen(self._do(node_name, args))
      casalog.post("stop engine %d on %s" % (engine_id, node_name),"INFO","stop_engine")
      self.__engines=self.__update_cluster_info(-1)


   def stop_node(self, node_name):
      """Stop a node (a engine-host computer)

      @param node_node The node to be stopped.
      If a computer with the given name is in the current cluster, running this function will stop all the engines currently running on that node and remove the node and engines from the engine list. This function will not shutdown the computer.
     
      """
      
      casalog.origin("parallel_go")
      
      if type(node_name).__name__ != 'str':
         casalog.post("node_name must be a string","WARN","stop_node")
         return None

      if self.get_nodes().count(node_name) == 0:
          casalog.post("There is no host with name %s" % node_name,"WARN","stop_node")
          return None

      out=open('/dev/null', 'w')
      err=open('/dev/null', 'w')
      cmd = self._cp(self.__ipythondir+'/'+self.__stop_node_file,
                node_name,
                self.__prefix+self.__stop_node_file)
      p=Popen(cmd, stdout=out, stderr=err)
      out.close()
      err.close()
      sts = os.waitpid(p.pid, 0)
      args='bash '+self.__prefix+self.__stop_node_file
      Popen(self._do(node_name, args))
      casalog.post("stop engines on %s" % node_name,"INFO","stop_node")

      num_engine=0
      for i in self.__engines:
         if i[1]==node_name:
            num_engine=num_engine-1

      self.__engines=self.__update_cluster_info(num_engine)

   def __stop_controller(self):
      """(Internal) Stop the controller.

      This is the last thing for quiting the cluster gracely. 

      """
      
      casalog.origin("parallel_go")
      
      # If it is already down
      if (self.__controller==None):
         return True

      import commands
      node_name=commands.getoutput("uname -n")
      out=open('/dev/null', 'w')
      err=open('/dev/null', 'w')
      cmd = self._cp(self.__ipythondir+'/'+self.__stop_controller_file,
                node_name,
                self.__prefix+self.__stop_controller_file)
      p=Popen(cmd, stdout=out, stderr=err)
      out.close()
      err.close()
      sts = os.waitpid(p.pid, 0)
      args='bash '+self.__prefix+self.__stop_controller_file
      Popen(self._do(node_name, args))

      try:
         os.remove(self.__ipythondir+'/'+self.__cluster_rc_file)
         os.remove(self.__ipythondir+'/'+self.__start_engine_file)
         os.remove(self.__ipythondir+'/'+self.__stop_node_file)
         os.remove(self.__ipythondir+'/'+self.__stop_controller_file)
         os.remove(self.__ipythondir+'/'+self.__stop_engine_file)
      except:
         # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
         # traceback.print_tb(sys.exc_info()[2])
         pass

      try:
         self.__controller=None
      except:
         # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
         # traceback.print_tb(sys.exc_info()[2])
         pass

      casalog.post("Controller stopped","INFO","stop_controller")
      return True

   def stop_cluster(self):
      """Stop the cluster

      This function stops all the running engines and the controller.

      """

      # jagonzal (CAS-4292): We have to check the controller instance directly because the method
      # start_cluster does not work properly (crashes when initializing the nodes via __init_nodes).
      # Actually start_cluster is deprecated, and it is necessary to use directly the start_engine 
      # method which does not only start the engine, but also initializes it using scripts
      if ((self.__controller==None) or (self.__client==None)):
         return
      # jagonzal (CAS-CHANGE): Do not use brute-force kill to schut down cluster
      else:
         # Kill the engines and controller using kernel.multiengineclient interface
         try:
             self.__client.kill(True,self.__engines,False)
             del self.__client
         except:
             traceback.print_exception((sys.exc_info()[0]), (sys.exc_info()[1]), (sys.exc_info()[2]))
         # Reset state before doing anything else, otherwise we may try to use one method from the client object
         self.__client=None
         self.__controller=None             
         # Update cluster info
         self.__engines=[]
         # Remove initialization/shut-down scripts
         try:
             os.remove(self.__ipythondir+'/'+self.__start_controller_file)             
             os.remove(self.__ipythondir+'/'+self.__cluster_rc_file)
             os.remove(self.__ipythondir+'/'+self.__start_engine_file)
             os.remove(self.__ipythondir+'/'+self.__stop_node_file)
             os.remove(self.__ipythondir+'/'+self.__stop_controller_file)
             
             os.remove(self.__prefix+self.__cluster_rc_file)
             os.remove(self.__prefix+self.__start_engine_file)            
         except:
             traceback.print_exception((sys.exc_info()[0]), (sys.exc_info()[1]), (sys.exc_info()[2]))             
         # jagonzal (CAS-4370): Remove all the ipcontroller/ipengine files because
         # otherwise it might confuse future cluster/MultiEngineClient instances
         self.wash_logs()
         return

      ### jagonzal (CAS-4292): Code below is deprecated ###
      
      # shutdown all engines
      elist=[]
      for i in self.__engines:
         elist.append(i[1])
      fruit=set(elist)

      for i in fruit:
         try:
            self.stop_node(i)
         except:
            # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
            traceback.print_exception((sys.exc_info()[0]), (sys.exc_info()[1]), (sys.exc_info()[2]))
            continue

      # shutdone controller
      try:
         self.__stop_controller()
      except:
         # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
         traceback.print_exception((sys.exc_info()[0]), (sys.exc_info()[1]), (sys.exc_info()[2]))
         pass

      try:
         # jagonzal (CAS-4106): We have to shut down the client, not activate it
         # besides, the activate method only enables parallel magic commands
         self.__client=None
      except:
         # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
         traceback.print_exception((sys.exc_info()[0]), (sys.exc_info()[1]), (sys.exc_info()[2]))
         pass

   def wash_logs(self):
      """Clean up the cluster log files.

      A set of logs containing controller-engine information will be created every time a cluster is created. This function deletes all cluster log files that cumulated in the user's $IPYTHONDIR, if there is no active cluster running. (The files will be removed only before starting any engine of after stoping the whole cluster.

      """
      # do this only if no controller running
      if (self.__controller!=None):
         return True

      # jagonzal (CAS-4370): Remove all the ipcontroller/ipengine files because
      # otherwise it might confuse future cluster/MultiEngineClient instances
      os.system("rm -rf %s/log/*" % self.__ipythondir)
      os.system("rm -rf %s/security/*" % self.__ipythondir)

   def __init_nodes(self, i):
     """(Internal) Initialize engines

     @param i The list of the engine ids
     An engine is a Python interpreter. To make an engine capable of running CASA tasks and tools, we must setup the environment and import necessary modules. This function effectively make every engine a running CASA instance (except that it is a non-interactive CASA running in Python, in contrast the casapy that is an interactive CASA running in IPython).

     """
     
     casalog.origin("parallel_go")
     casalog.post("Initialize engines %s" %str(i),"INFO","init_nodes")
     
     self.__client.push({'casa': casa })
     self.__client.execute('import os', i)
     self.__client.execute('if os.path.isdir(work_dir):os.chdir(work_dir)\nelse:work_dir=os.environ["HOME"]', i)
     phome=''
     try:
       phome=os.environ["PYTHONHOME"]
     except:
       # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
       # traceback.print_tb(sys.exc_info()[2])
       pass
     
     if phome=='':
        try:
           v=str.split(os.environ["CASAPATH"], ' ')
           phome=v[0]+'/'+v[1]
        except:
           # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
           # traceback.print_tb(sys.exc_info()[2])
           pass

     dhome=''
     try:
       dhome=os.environ["CASAARCH"]
     except:
       # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
       # traceback.print_tb(sys.exc_info()[2])
       pass

     if phome=='':
         casalog.post("could not locate casa_in_py.py","SEVERE","init_nodes")
         return None

     if (dhome!=phome):
        phome=dhome

     sdir = casadef.python_library_directory + '/'
     self.__client.push(dict(phome=phome), i)
     self.__client.execute('import sys', i)
     self.__client.push(dict(sdir=sdir), i)
     self.__client.execute('scriptdir=sdir', i)

     self.__client.execute('sys.path.insert(2, scriptdir)', i)
     try:
        self.__client.execute("execfile(scriptdir+'casa_in_py.py')", i)
        self.__client.execute('inited=True', i)
     except client.CompositeError, exception:
         casalog.post("Error initializing engine %s: %s" % (str(i), str(exception)),"SEVERE","init_nodes")
         exception.print_tracebacks()
     except:
         casalog.post("Error initializing engine %s" % str(i),"SEVERE","init_nodes")
         traceback.print_tb(sys.exc_info()[2])     
    

   def reset_cluster(self):
      """Re-initialize the engines.

      This function reset the running environment for all the available engines.
      
      """
      
      casalog.origin("parallel_go")
      
      if self.__client is None:
         casalog.post("Multiengineclient is not initialized","WARN","reset_cluster")
         return None

      try:
         tobeinit=self.__client.pull('id')
      except:
         # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
         # traceback.print_tb(sys.exc_info()[2])
         return None
         

      if len(tobeinit)>0:
         self.__init_nodes(tobeinit)
         self.__engines=self.__client.pull(['id', 'host', 'pid', 'inited'])

   def __update_cluster_info(self, num_engine, work_dir=None,omp_num_nthreads=1):
      """(Internal) Construct the list of engines.

      @param num_engine The number of new engines 
      @param work_dir The initial working directory 
      This function appends num_engine engines to the engine list and setup initial Python environment on them. Before further initialization, an engine can only run Python programs (it can not run CASA tasks or tools).

      """
      
      casalog.origin("parallel_go")

      if self.__client is None :
         casalog.post("Controller is not initialized","WARN","update_cluster_info")
         return [] 

      engs=len(self.__engines)+num_engine
      if engs<0:
         engs=0
      i=0
      idlist=self.__client.get_ids()
      while (len(idlist)!=engs and i<10):
         idlist=self.__client.get_ids()
         time.sleep(1)
         i=i+1
         
      # Here we only take care of the quick-init-abel items
      # The init of real casa_in_py will be done in parallel 
      tobeinit=[]
      for i in idlist:
         inited=False
         try:
            inited=self.__client.pull('inited', i)
         except:

            # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
            # traceback.print_tb(sys.exc_info()[2])

            tobeinit.append(i)
            self.__client.execute('id=%d'%i, i)
            self.__client.execute('import os', i)
            self.__client.execute('import socket', i)
            self.__client.execute('host=socket.gethostname()', i)
            self.__client.execute('pid=os.getpid()', i)
            self.__client.execute('job=None', i)
            self.__client.execute('import signal', i)
            self.__client.execute('original_sigint_handler = signal.signal(signal.SIGINT,signal.SIG_IGN)', i)
            # jagonzal (CAS-4276): New cluster specification file allows to automatically set the number of open MP threads
            self.__client.execute("os.environ['OMP_NUM_THREADS']='"+str(omp_num_nthreads)+"'", i)

            if work_dir!=None and os.path.isdir(work_dir):
               self.__client.push(dict(work_dir=work_dir), i)
            else:
               self.__client.execute('work_dir=os.environ["HOME"]', i)

            # These are environment variabls set for each node at startup. 
            # It may be better to set as global in this module then pass to each engine when update_cluster_info

            self.__client.execute('contrid=os.environ["contrid"]', i)
            self.__client.execute('stamp=os.environ["stamp"]', i)
            self.__client.execute('inited=False', i)

      self.__new_engs.extend(tobeinit)

      if self.__init_now:
         if len(self.__new_engs)>0:
            self.__init_nodes(self.__new_engs)

         self.__init_now=True
         self.__new_engs=[]

      if len(idlist)>0:
         return self.__client.pull(['id', 'host', 'pid', 'inited'])
      else:
         return []

   def get_casalogs(self):
      """Get a list of the casa logs for all the current cluster engines.

      Each working engine is a CASA instance and saves its own log. This function retrun the list of logs with their full path. One can view the log contents with casalogviewer.

      """
      try:
         self.__client.execute('tmp=work_dir+"/"+thelogfile')
         return self.__client.pull('tmp')
      except:
         # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
         # traceback.print_tb(sys.exc_info()[2])
         return None

   def read_casalogs(self):
      """Read the casa log files.

      The current implementation of this function is only a prototype. A multi-log viewer needs to be developed.

      """
      
      casalog.origin("parallel_go")
      
      import os
      import string
      _logs = self.get_casalogs()
      if _logs != None:
         files = string.join(_logs, ' ')
         os.system("emacs "+files+ "&")
      else:
         casalog.post("Cannot read casalogs","WARN","read_casalogs")

   def pad_task_id(self, b='', task_id=[]):
      """Generate a dictionary of id-padded variables 

      @param b The base name to be padded
      @param task_id A list of integers to pad the base name 
      One way of distributing varaibles to a set of engnines is through python a dictionary. This is a convenience function for quick generating a dictionary of padded names. Example:
      x=c.pad_task_id('basename', [3, 5, 8])
      x
      {3: 'basename-3', 5: 'basename-5', 8: 'basename-8'}
      x=c.pad_task_id([1,3],[0,1,2,3])
      x
      {0: '1-0', 1: '3-1', 2: '3-2', 3: '3-3'}
      x=c.pad_task_id(['a', 'b','c','d','e'],[0,1,2,3])
      x
      {0: 'a-0', 1: 'b-1', 2: 'c-2', 3: 'd-3'}
      y=c.pad_task_id(x)
      y
      {0: 'a-0-0', 1: 'b-1-1', 2: 'c-2-2', 3: 'd-3-3'}
     
      """
      
      casalog.origin("parallel_go")

      base={} 

      int_id=True
      for j in task_id:
         if type(j)!=types.IntType or j<0:
            casalog.post("Task id %s must be a positive integer" % str(j),"WARN","pad_task_id")
            int_id=False
            break
      if not int_id:
         return base

      if type(b)==list:
         for j in range(len(b)):
            if type(b[j])!=str:
               b[j]=str(b[j])

      if len(task_id)==0:
         task_id=list(xrange(0, len(self.__engines))) 
      if type(b)==str:
         for j in task_id:
            base[j]=b+'-'+str(j)
      if type(b)==list:
         k=len(b)
         m=len(task_id)
         if m<=k:
            for j in range(m):
               base[task_id[j]]=b[j]+'-'+str(task_id[j])
         else:
            for j in range(k):
               base[task_id[j]]=b[j]+'-'+str(task_id[j])
            for j in range(k,m):
               base[task_id[j]]=b[k-1]+'-'+str(task_id[j])

      if type(b)==dict:
          for i in b.keys():
             base[i]=b[i]+'-'+str(i)

      return base
       
   def one_to_n(self, arg, task_id=[]):
      """Genrate a dictionary of one variable for n keys

      @param arg The variable to be distributed
      @param task_id The list of integer ids
      One way of distributing varaibles to a set of engnines is through python a dictionary. This is a convenience function for quick generating a dictionary of same variable for n keys. Example:
      x=c.one_to_n('basename', [1, 2, 7])
      x
      {1: 'basename', 2: 'basename', 7: 'basename'}

      """
      
      casalog.origin("parallel_go")
      
      # assign 1 value to n targets
      base={} 
      int_id=True
      for j in task_id:
         if type(j)!=types.IntType or j<0:
            casalog.post("Task id %s must be a positive integer" % str(j),"WARN","one_to_n")
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
      """Generate a dictionary of n varables

      @param arags A list of n variables
      @param task_id A list of n integer ids
      One way of distributing varaibles to a set of engnines is through python a dictionary. This is a convenience function for quick generating a dictionary of a set of n variables for n keys. Example:
      x=c.n_to_n(['a', 'b', 'c'], [3, 6, 7])
      x
      {3: 'a', 6: 'b', 7: 'c'}

      """
      
      casalog.origin("parallel_go")
      
      # Assign n value to n targets
      base={} 

      if len(args)==0:
         return base

      int_id=True
      for j in task_id:
         if type(j)!=types.IntType or j<0:
            casalog.post("Task id %s must be a positive integer" % str(j),"WARN","n_to_n")
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

   def split_int(self, start, end, task_id=[]):
      """Generate a dictionary to distribute the spectral windows

      @param start The start integer value
      @param end The end integer value
      @param task_id The list of integer ids
      This is a convenience function for quick generating a dictionary of integer start points. Example:
      x=c.split_int(9, 127, [2,3,4])
      x
      {2: 9, 3: 49, 4: 89 }

      """
      
      casalog.origin("parallel_go")
      
      base={}
      if len(task_id)==0:
         task_id=list(xrange(0, len(self.__engines))) 

      if len(task_id)==0:
         casalog.post("There are no engines available","WARN","split_int")
         return base

      if type(start)!=int or type(end)!=int:
         casalog.post("start and end point must be integer","WARN","split_int")
         return base

      if start<0:
         casalog.post("start point must be greater than 0","WARN","split_int")
         return base

      if start>=end:
         casalog.post("end point must be greate than start point","WARN","split_int")
         return base

      nx=1
      try:
         nx=int(ceil(abs(float(end - start))/len(task_id)))
      except:
         # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
         # traceback.print_tb(sys.exc_info()[2])
         pass

      i=-1
      for j in task_id:
         i=i+1
         if i>=len(task_id):
            break
         st=i*nx
         base[j]=st+start
      return base
       
   def split_channel(self, spw, nchan, task_id=[]):
      """Generate a dictionary to distribute the spectral windows

      @param spw The spectral window
      @param nchan The number of channels to split
      @param task_id The list of integer ids
      One way of distributing a spectral windows to a set of engnines is through python a dictionary. This is a convenience function for quick generating a dictionary of spw expressions. Example:
      x=c.split_channel(1, 127, [2,3,4])
      x
      {0: '1:0~42', 1: '1:43~85', 2: '1:86~128'}

      """
      
      casalog.origin("parallel_go")
      
      base={}
      if len(task_id)==0:
         task_id=list(xrange(0, len(self.__engines))) 

      if len(task_id)==0:
         casalog.post("There are no engines available","WARN","split_channel")
         return base

      if nchan<len(task_id):
         casalog.post("There are no enough channels to split","WARN","split_channel")
         return base

      nx=1
      try:
         nx=int(ceil(abs(float(nchan))/len(task_id)))
      except:
         # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
         # traceback.print_tb(sys.exc_info()[2])
         pass

      i=-1
      for j in task_id:
         i=i+1
         if i==len(task_id):
            break
         st=i*nx
         se=st+nx-1
         base[j]=str(spw)+":"+str(st)+"~"+str(se)
      return base
       
   def pgc(self,*args,**kwargs):
      """Parallel execution of commands and/or dictionary 
         of commands

      @param *args any number of commands or dictionary of
             commands (where the key of the dictionary is the
             engine id)
      @param **kwargs available options are
             job=<str> or jobname=<str>
             block=<True/False>

      Example:
      c.pgc({0:'ya=3',1:'ya="b"'})
      c.pull('ya')
      {0: 3, 1: 'b'}

      c.pgc('xa=-1')
      c.pull('xa')
      {0: -1, 1: -1, 2: -1, 3: -1}
      c.pull('job')
      Out[23]: {0:'xa=-1', 1:'xa=-1', 2:'xa=-1', 3:'xa=-1'}

      """
      
      casalog.origin("parallel_go")

      tasks={}
      for j in self.__client.get_ids():
         tasks[j]=[]
      
      for i in args:
         if type(i)==types.DictType:
            for j in i.keys():
               if type(j)!=types.IntType or j<0:
                  casalog.post("task id %s must be a positive integer" % str(j),"WARN","pgc")
                  pass
               else:
                  st=''
                  if type(i[j])==types.StringType:
                     st=i[j]
                  else:
                     pass
                  if st!='':
                     tasks[j].append(st)
         elif type(i)==types.StringType:
            # For all tasks
            for j in xrange(0, len(self.__engines)): 
               tasks[j].append(i)
         else:
            casalog.post("command %s must be a string or a dictionary" % str(i),"WARN","pgc")

      # May be better to use non-block mode and catch the result

      # How to give name, say 'cmd_name', to a set of commands a name such
      # that cluster.pull('cmd_name') returns the script is excuteded?
     
      keys = kwargs.keys()
      job='NoName'
      block=True
      for kw in keys:
         if kw.lower()=='job' or kw.lower()=='jobname':
            job=kwargs[kw]
         if kw.lower()=='block':
            block=kwargs[kw]
 
      for i in tasks.keys():      
         cmd=string.join(tasks[i], '\n')
         self.__client.push(dict(job=cmd), i)

      return self.__client.execute('exec(job)',
              block=block,targets=tasks.keys())

   def parallel_go_commands(self,*args,**kwargs):
      """Parallel execution of commands and/or dictionary 
         of commands


      """
      self.pgc(*args,**kwargs)
 
   def pgk(self, **kwargs):
      """Parallel execution to set keywords

      @param **kwargs keyword args 

      Example:
      x=np.zeros((3,3))
      c.pgk(c={1:x},d=6,t='b',s={0:'y'})
      c.pull('c')
      {1: array([[ 0.,  0.,  0.],
                 [ 0.,  0.,  0.],
                 [ 0.,  0.,  0.]])}
      c.pull('d')
      {0: 6, 1: 6}
      c.pull('s')
      {0: 'y'}
      c.pull('t')
      {0: 'b', 1: 'b'}
      """
      
      casalog.origin("parallel_go")

      tasks={}
      for j in self.__client.get_ids():
         tasks[j]=dict()
      
      keys = kwargs.keys()

      for kw in keys: 
         vals=kwargs[kw]
         if type(vals)==types.DictType:
            for j in vals.keys():
               if type(j)!=types.IntType or j<0:
                  casalog.post("task id %s must be a positive integer" % str(j),"WARN","pgk")
                  pass
               else:
                  tasks[j][kw]=vals[j]
         else:
            for j in tasks.keys():
               tasks[j][kw]=vals

      for i in tasks.keys():      
         self.__client.push(tasks[i], i, True)
         
      return tasks

   def make_command(self, func, **kwargs):
      """Make command strings to be distributed to engines

      @func function name 
      @kwargs **kwargs available 

      Example:
      x=np.ones((3,3))
      c.make_command(func=None,c={1:x},d=6,t='b',s={0:'y'})
      {0: 's="y"; t="b"; d=6',
       1: 'c=array([[ 1., 1., 1.],\n[ 1., 1., 1.],\n[ 1., 1., 1.]]); t="b"; d=6'}
      c.make_command(func='g',c={1:x},d=6,t='b',s={0:'y'})
      {0: 'g(s="y", t="b", d=6)',
       1: 'g(c=array([[1., 1., 1.],\n[1., 1., 1.],\n[1., 1., 1.]]), t="b", d=6)'}
      """
      
      casalog.origin("parallel_go")

      tasks=self.pgk(**kwargs)

      if func!=None and type(func)!=str:
        casalog.post("func must be a str","WARN","make_command")
        return None

      if len(tasks)==0:
        casalog.post("Parameters not specified","WARN","make_command")
        return None

      if func==None or len(str.strip(func))==0:
         func=''

      func=str.strip(func)

      cmds=dict()
      for i in tasks.keys(): 
        cmd=''
        for (k, v) in tasks[i].iteritems():
          cmd+=k+'='
          if type(v)==str:
              cmd+='"'+v+'"'
          elif type(v)==np.ndarray:
              cmd+=repr(v)
          else:
              cmd+=str(v)
          if func=='':
            cmd+='; '
          else:
            cmd+=', '
        cmd=cmd[0:-2]
        if func!='':
           cmd=func+'('+cmd+')'
        cmds[i]=cmd

      return cmds
  
               
   def parallel_go_keywords(self, **kwargs):
      """Parallel execution to set keywords


      """
      self.pgk(**kwargs)

   def hello(self):
      """Parallel execution to print 'hello' message from all engines


      """
      
      casalog.origin("parallel_go")
      
      casalog.post("Hello CASA Controller","INFO","hello")
      
      if self.get_engines() != []:
         return self.__client.execute('casalog.origin("parallel_go");casalog.post("Hello CASA Controller","INFO","hello")')
      else:
         return None

   def __set_cwds(self, clusterdir):
      """Set current working dir for all engines


      """
      # This is not very useful because dirs are generally different cross nodes 
      self.__client.execute('import os')
      self.__client.push(dict(clusterdir=clusterdir))
      self.__client.execute('os.chdir(clusterdir)')
      self.__client.execute("user=self.__user")
      self.__client.execute('print user')
      self.__client.execute('import socket')
      self.__client.execute('host=socket.gethostname()')
      self.__client.execute('print host')
      self.__client.execute('print os.getcwd()')

   def get_ids(self):
      """get ids for all available engines


      """
      try:
         return self.__client.get_ids()
      except:
         # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
         # traceback.print_tb(sys.exc_info()[2])
         return []

   def get_nodes(self):
      """get hostnames for all available engines


      """
      from sets import Set 
      elist=[]
      for i in self.__engines:
         elist.append(i[1])
      return list(Set(elist))

   def get_engines(self):
      """get current status of the engines


      """
      return self.__engines

   def get_stdout(self,cmd):
      """get the standard output from all engines for execting a comment


      """
      return commands.getstatusoutput(cmd)      

   def pdo(self,job):
      """parallel execution of a job


      """
      return self.__client.execute(job)

   def odo(self,job,nodes):
      """execute a job on a subset of engines


      """
      return self.__client.execute(job,block=False,targets=nodes)
  
   def execute(self,job,nodes):
      """execute a job on a subset of engines in blocking mode


      """
      return self.__client.execute(job,block=True,targets=nodes)  
  
   def queue_status(self):
      """query to queue status


      """
      return self.__client.queue_status()

   def clear_queue(self):
      """remove all jobs from the queue


      """
      return self.__client.clear_queue()

   def get_timer(self, timer=''):
      """get the eleapsed time for a timer

      """
      
      casalog.origin("parallel_go")

      base={}
      prop=self.__client.get_properties() 
      for i in self.get_ids():
          try:
             ky=prop[i]['timertype']
             if ky=='proc':
                end=time.clock()
             else:
                end=time.time()
             base[i]='%.2f sec' % (end-prop[i][timer])
          except:
             pass
             # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
             # traceback.print_tb(sys.exc_info()[2])
         
      casalog.post("Timer: %s" % str(base),"INFO","get_timer")
      return

   def set_timer(self,timer='timer',type='proc',
                 targets=None,block=None):
      """set a timer 


      """
      
      if self.__client==None:
          return

      properties={}
      if type=='proc':
          properties[timer]=time.clock()
      else:
          properties[timer]=time.time()

      properties['timertype']=type

      self.__client.set_properties(properties,
               targets, block)

   def del_timer(self, timer=['']):
      """delete a timer

      """
      
      casalog.origin("parallel_go")

      for i in self.get_ids():
          self.__client.del_properties(timer, i)
          casalog.post("Delete timer %s %s" % (str(timer),str(i)),"INFO","del_timer")

      return

   def get_properties(self):
      """get the set properties from all engines


      """
      return self.__client.get_properties()

   def set_properties(self, properties, 
                      targets=None, block=None):
      """set properties for target engines

      @param properties a dictionary its keys are 


      """
      self.__client.set_properties(
                properties, targets, block)


   def keys(self):
      """get all keys from all engines


      """
      return self.__client.keys()

   def push(self, **kwargs):
      """set values to the engines
      @param kekword value to distribute
      @param targets, the engines of interest
      By default, this function set the keyword values to all engines.
      To set values on a subset of engines, use kekword parameter targets,
      whick takes integer or array of integer of engine ids.
      You can also use function pgk to set values onto the engines. 
      Example:
      c.push(a=[1,3,7.1])
      c.pull('a')
      {0: [1, 3, 7.0999999999999996], 1: [1, 3, 7.0999999999999996]}
      c.push(b=[1.2,3.7], targets=1)
      c.pull('b',[1])
      {1: [1.2, 3.7000000000000002]}
      c.pull('b')
      {1: [1.2, 3.7000000000000002]}

      """
      
      casalog.origin("parallel_go")

      keys = kwargs.keys()
      #keys.sort()
      if len(keys)==0:
          return False

      tgt=[]
      targets=None
      for kw in keys:
         if kw.lower()=='targets':
            targets=kwargs[kw]
            break

      if targets=='all' or targets==None or \
         type(targets)==list and len(targets)==0:
          tgt=list(xrange(0, len(self.__engines))) 
      elif type(targets)==list:
          for j in targets:
              if type(j)==types.IntType and j>=0:
                  tgt.append(j)
      elif type(targets)==int and targets>=0:
          tgt.append(targets)
           
      if len(tgt)==0:
           casalog.post("There are no target engines","WARN","push")
           return False

      ok=True
      for i in tgt: 
          try:
              self.__client.push(dict(kwargs),i)
          except:
              pass
              # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
              # traceback.print_tb(sys.exc_info()[2])

      return ok

   def pull(self, key, targets='all'):
      """get the value of a key
      @param key the var of interest
      @param targets, the engines of interest
      Example:
      c.pgc({0:'ya=3',1:'ya="b"'})
      c.pull('ya')
      {0: 3, 1: 'b'}
      c.pull('ya',[1])
      {1: 'b'}
      c.pull('ya',1)
      {1: 'b'}

      """
      
      casalog.origin("parallel_go")
      
      base={} 
      tgt=[]
      if targets=='all' or \
         type(targets)==list and len(targets)==0:
          tgt=list(xrange(0, len(self.__engines))) 
      elif type(targets)==list:
          for j in targets:
              if type(j)==types.IntType and j>=0:
                  tgt.append(j)
      elif type(targets)==int and targets>=0:
          tgt.append(targets)
           
      if len(tgt)==0:
           casalog.post("There are no target engines","WARN","push")
           return base

      for i in tgt: 
          rslt=None
          try:
              rslt=self.__client.pull(key,i)
          except:
              # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
              # traceback.print_tb(sys.exc_info()[2])
              pass 
          if rslt!=None:
              base[i]=rslt[0]

      return base

   def get_result(self, i):
      """get the result of previous execution


      """
      
      casalog.origin("parallel_go")
      
      # jagonzal (CAS-4375): We have to capture the engine's exceptions at this level
      try:
           res = self.__client.get_result()[i]
      except client.CompositeError, exception:
           casalog.post("Error retrieving result from engine %s: %s" % (str(i),str(exception)),"SEVERE","get_result")
           exception.print_tracebacks()
           res = None
      except:
           # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
           casalog.post("Error retrieving result from engine %s" % (str(i)),"SEVERE","get_result")
           traceback.print_tb(sys.exc_info()[2])

      return res

   def activate(self):
      """set the cluster to parallel execution mode

  
      """
      return self.__client.activate()


   def parallel_go_task(self,taskname=None,outfile='',
                   target=[],ipython_globals=None):
       """ Make parallel tasks using current input values 


       """
       self.pgt(taskname,outfile,target,ipython_globals)

   def pgt(self, taskname=None,outfile='',
                    target=[],ipython_globals=None):
       """ Make parallel tasks using current input values 
   
       taskname -- Name of task
           default: None = current active task; 
           example: taskname='bandpass'
           <Options: type tasklist() for the complete list>
       outfile -- Output file for the task inputs
           default: '' = taskname.parallel;
           example: outfile=taskname.orion
       target -- List of integer parallel engine ids
           default: [] = all current active engines;
           example: target=[0,2,4]
   
       """
       
       casalog.origin("parallel_go")
       
       base={} 
       for j in target:
           if type(j)!=types.IntType or j<0:
               casalog.post("engine id %s must be a positive integer" % str(j),"WARN","pgt")
               return base

       if len(target)==0:
           target=list(xrange(0, len(self.__engines))) 
       if len(target)==0:
           casalog.post("There are no target engines","WARN","pgt")
           return base

       try:
           if ipython_globals == None:
               t=len(inspect.stack())-1 
               myf=sys._getframe(t).f_globals
           else:
               myf=ipython_globals
   
           if taskname==None or taskname=='' or \
              type(taskname)!=str:
               taskname=myf['taskname']
   
           if outfile=='' or outfile==None or \
              type(outfile)!=str:
               outfile=taskname+'.parallel'
   
           tname=myf[taskname]
           if not myf.has_key(taskname) and \
              str(type(tname))!="<type 'instance'>" and \
              not hasattr(tname,"defaults"):
               raise TypeError("task %s is not defined " %
                               taskname)
           else:
               myf['taskname']=taskname
               myf['update_params'](func=myf['taskname'],
                     printtext=False, ipython_globals=myf)
           
           for j in target:
               script=taskname+'('
               for k in myf[taskname].parameters:
                   par=myf[taskname].parameters[k]
                   if type(par)==dict:  
                       val=par
                       for v in par.keys():
                           if type(v)==types.IntType and j==v:
                               val=par[v]
                               break
                           elif type(v)==str:
                               a=-1
                               try:
                                   a=int(v)
                               except:
                                   # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
                                   # traceback.print_tb(sys.exc_info()[2])
                                   pass
                               if a!=-1 and a==j:
                                   val=par[v]
                                   break
                       if type(val)==str:
                           script=script+k+"='"+val+"',"
                       else:
                           script=script+k+"="+str(val)+","
                   elif type(par)==str:
                       script=script+k+"='"+par+"',"
                   else:
                       script=script+k+"="+str(par)+","
               script=script.rstrip(',')
               script=script+')'
               base[j]=script
           return base
       except TypeError, e:
           casalog.post("TypeError: %s" % str(e),"SEVERE","pgt")


   def check_job(self, job, verbose=True):
      """check the status of an asynch job

  
      """
      
      casalog.origin("parallel_go")
      
      if type(job)==type(None):
          print "job None has no status"
          return True
      try:
         x=job.get_result(block=False)
         if x==None:
            if verbose:
               casalog.post("job '%s' has not finished yet, result is pending" % job,"INFO","check_job")
            return False
         else:
            if verbose:
               casalog.post("job '%s' done" % job,"INFO","check_job")
            return True         
      except client.CompositeError, exception:
         casalog.post("Error retrieving result of job from engine: %s, backtrace:" % (str(exception)),"SEVERE","check_job")
         exception.print_tracebacks()
         raise
      except:
         casalog.post("Error retrieving result of job from engine, backtrace:","SEVERE","check_job")
         traceback.print_tb(sys.exc_info()[2])
         raise

   def howto(self):
      print """A simple example for use the cluster
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
"""

   def use_often(self):
      print """Frequently used commands
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
"""

   def example(self):
      print """example: run clean on 4 engines

from parallel_go import *
c=cluster()
c.start_engine('casa-dev-08', 4,
               '/home/casa-dev-08/hye/cluster')
default clean
mspath='/home/casa-dev-09/hye/test/ngc6503/ngc6503_output/'
msname='ngc6503.ms.contsub'
vis=mspath+msname
wpath=[]
for i in c.pull('work_dir'):
   wpath.append(i+'/'+msname+'.clean')

imagename=c.pad_task_id(wpath)
mode='channel'
start=c.split_int(9, 127)
nchan=40
width=1
calready=False
gain=0.1
msize=[370,250]
psfmode='clark'
cell=[4.,4.]
niter=10000
threshold=0.0003
taskghting='briggs'
#rmode = 'norm'
robust=0.5
mask = ''
s=c.pgt()
job=[]
for i in c.get_ids():
    job.append(c.odo(s[i], i))

c.check_job(job[0])

c.get_result(0)

"""


cluster=cluster()


"""
for i in 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16;
do ssh casa-dev-$i "ps -ef | grep hye" ; done
"""
"""
c.pgc('import time', {0: 'time.sleep(10); x=5; y="y is y"', 1: 'time.sleep(12);a=7;b="b is not y"'},block=False,job="wakeup")

c.pull('x',0)
c.pull('y',0)
c.pull('a',1)
c.pull('b',1)
c.odo('print x', 0)
c.odo('print x', 0).r

"""
