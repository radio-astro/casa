import os
import math
import time
import thread
import commands
import numpy as np
from taskinit import *
from tasksinfo import *
from parallel_go import *

# jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
import traceback

class simple_cluster:
    """The simple_cluster creates and maintains an ipcluster environment
    for controlling parallel execution of casa tasks (tools and scripts)
    """
    
    __default_mem_per_engine = 512
    __default_mem_fraction = 0.9
    __default_cpu_fraction = 0.9
    
    
    def __init__(self,monitoringFile='monitoring.log',verbose=False):
        
        self._project=""
        self._hosts=[]
        self._jobs={}
        self._rsrc={}
        self._job_title=1
        self._monitor_on=True
        self._monitor_running=False
        self._monitoringFile=monitoringFile
        self._verbose=verbose
        self._resource_on=True
        self._resource_running=False
        self._configdone=False
        self._cluster=cluster()
        self._JobQueueManager=None
        self._enginesRWoffsets={}
        self.__localCluster = False
        self.__running = False
        # jagonzal: This is basically the destructor (i.e. for graceful finalization)
        atexit.register(simple_cluster.stop_cluster,self)

    # jagonzal (CAS-4372): By-pass ssh redirection when deploying engines in localhost 
    def shell(self, hostname):
      """Creates the command line to execute the give command on the given host.
      If and only if the host is not localhost, ssh is used."""

      if self.uniqueIP(hostname) == self.uniqueIP("localhost"):
         return "eval 2>/dev/null "
      else:
         # f: Requests ssh to go to background just before command execution.
         # q: Quiet mode.  Causes all warning and diagnostic messages to be suppressed.
         # x: Disables X11 forwarding.
         return "ssh -fqx " + hostname
        
    def uniqueIP(self, hostname):
      """Returns a unique IP address of the given hostname,
      i.e. not 127.0.0.1 for localhost but localhost's global IP"""
      
      ip = socket.gethostbyname(hostname)
      
      if ip == "127.0.0.1":
         ip = socket.gethostbyname(socket.getfqdn())
         
      return ip
  
    ####################################################################
    # Static method that returns whatever the current definition of a
    # cluster is.  If none is defined the default cluster is created,
    # initialized and returned.
    ####################################################################
    @staticmethod
    def getCluster():
        # This method will check for a cluster existing at the global
        # scope.  If it does not exist then a default cluster will be
        # created.
        # The cluster object is returned.
        myf = sys._getframe(len(inspect.stack())-1).f_globals
        if not 'procCluster' in myf.keys():
            sc = simple_cluster()
            if (sc.init_cluster()):
                return myf['procCluster']
            else:
                return None
        else:
            sc = myf['procCluster']
            if not sc.isClusterRunning():
                if (sc.init_cluster()):
                    return myf['procCluster']
                else:
                    return None
            else:
                return myf['procCluster']
    
    @staticmethod 
    def setDefaults(default_mem_per_engine=512,default_mem_fraction=0.9,default_cpu_fraction=0.9):
        
        simple_cluster.__default_mem_per_engine = default_mem_per_engine
        simple_cluster.__default_mem_fraction = default_mem_fraction
        simple_cluster.__default_cpu_fraction = default_cpu_fraction
        
    def isClusterRunning(self):
        return self.__running
    
    ###########################################################################
    ###   cluster verifiction
    ###########################################################################
    def config_cluster(self, cfg, force=False):
        """Read the configuration file and validate cluster definitions.  

        Keyword arguments:
        cfg -- the name of cluster configuration file
        force -- whether or not to reconfigure if a configured cluster exists

        A configuration file is an ASCII text file. Each line defines a node
        (also called host) with one line per host to be used, and the following 
        format: 
        
        - <hostname>, <number of engines>, <work directory>
        - <hostname>, <number of engines>, <work directory>, <fraction of total RAM>
        - <hostname>, <number of engines>, <work directory>, <fraction of total RAM>, <RAM per engine>
        
        where the interpretation of the parameters is as follows: 
        
        - hostname: Hostname of the target node where the cluster is deployed 
        
          NOTE: The hostname has to be provided w/o quotes
        
        - number of engines: Supports in turns 3 different formats 
        
            * If provided as an integer >1: It is interpreted as 
              the actual user-specified maximum number of engines
              
            * If provided as an integer =0: It will deploy as maximum 
              engines as possible, according to the idle CPU capacity 
              available at the target node
              
            * If provided as a float between 0 and 1: It is interpreted 
              as the percentage of idle CPU capacity that the cluster 
              can use in total at the target node

        - work directory: Area in which the cluster will put intermediate 
          files such as log files, configuration files, and monitoring files
        
          NOTE1: This area has to be accessible from the controller (user) machine, 
                 and mounted in the same path of the filesystem 
                 
          NOTE2: The path name has to be provided w/o quotes
        
        - fraction of total RAM: Supports in turns 3 different formats:
        
            * If provided as an integer >1: It is interpreted as the actual 
              user-specified maximum amount of RAM to be used in total at 
              the target node
              
            * If provided as an integer =0: It will deploy as maximum engines 
              as possible, according to the free RAM available at target node
              
            * If provided as a float between 0 and 1: It is interpreted as 
              the percentage of free RAM that the cluster can use in total 
              at the target node
            
        - RAM per engine: Integer, which is interpreted as the required memory 
          per engine in MB (default is 512MB) 
          
        It is also possible to add comments, by using the # character at the 
        beginning of the line. Example:
        
        #####################################################
        
        # CASA cluster configuration file for expert user
        orion, 10, /home/jdoe/test/myclusterhome1
        m42, 4, /home/jdoe/test/myclusterhome2, 0.6, 1024
        antares, 0.6, /home/jdoe/test/myclusterhome3, 0, 2048
        
        #####################################################
        
        - At host ``orion'': It will deploy up to 10 engines, with working 
          directory /home/jdoe/test/myclusterhome1, and using as much free 
          RAM available as possible (up to 90% by default), taking into 
          account that each engine can use up to 512 MB (the default and minimum)
          
        - At host ``m42'': It will deploy up to 4 engines, with working directory 
          /home/jdoe/test/myclusterhome2, and using at the most 60% of the free RAM 
          available, taking into account that each engine can use up to 1024 MB.   
          
        - At host ``antares'': It will deploy as many engines as possible, with 
          working directory /home/jdoe/test/myclusterhome3, using up to 60% of the 
          idle CPU capacity / cores, and as much free RAM available as possible 
          (up to 90% by default), taking into account that each engine can use up 
          to 2048 MB.  
 
        Normally, one does not call this function directly. 
        The init_cluster function will trigger this function.

        """
        
        casalog.origin("simple_cluster")
        
        if (len(self._hosts)>0 or self._configdone) and not force:
            casalog.post("Cluster already configured","WARN","config_cluster")
            return
        self._hosts=[]
        self._jobs={}
        configfile=open(cfg,'r')
        s=configfile.readlines()
        for line in s:
            sLine=line.rstrip()
            if str.find(sLine, '#')==0:
                continue
            xyzd=str.split(sLine, ',')
            if len(xyzd)<3:
                casalog.post("Node config should be at least: 'hostname, numengine, workdir'","WARN","config_cluster")
                casalog.post("The following entry will be ignored: %s" % sLine,"WARN","config_cluster")
                continue
            # jagonzal (CAS-4276): (Max) number of engines is not an integer any more
            # try:
            #     a=int(xyzd[1])
            # except:
            #     print 'the numofengines should be a integer instead of:', xyzd[1]
            #     print 'this entry will be ignored'
            #     continue
            [a, b, c]=[str.strip(xyzd[0]), float(xyzd[1]), str.strip(xyzd[2])]
            if len(a)<1:
                casalog.post("Hostname can not be empty","WARN","config_cluster")
                casalog.post("The following entry will be ignored: %s" % sLine,"WARN","config_cluster")
                continue
            if len(c)<1:
                casalog.post("Workdir can not be empty","WARN","config_cluster")
                casalog.post("The following entry will be ignored: %s" % sLine,"WARN","config_cluster")
                continue
            
            ### jagonzal (CAS-4276): New cluster specification file ###
             
            # Retrieve available resources to cap number of engines deployed 
            hostname = str(xyzd[0])
            (ncores_available,memory_available,cpu_available) = self.check_host_resources(hostname)

            # Determine maximum number of engines that can be deployed at target node
            max_engines_user = b
            if (max_engines_user<=0):
                max_engines = int(round(self.__default_cpu_fraction*(cpu_available/100.0)*ncores_available))
                if (max_engines < 2):
                    casalog.post("CPU free capacity available %s of %s cores would not support cluster mode at node %s, starting only 1 engine" 
                                 %(str(cpu_available),str(ncores_available),hostname),"WARN","config_cluster")
                    max_engines = 1                
            elif (max_engines_user<=1):
                max_engines = int(round(max_engines_user*ncores_available))
            else:
                max_engines = int(max_engines_user)
            
            casalog.post("Will deploy up to %s engines at node %s" % (str(max_engines),hostname), "INFO","config_cluster")
            
            # Determine maximum memory that can be used at target node
            if len(xyzd)>=4:
                max_mem_user = float(xyzd[3])
                if (max_mem_user<=0):
                    max_mem = int(round(self.__default_mem_fraction*memory_available))
                elif (max_mem_user<=1):
                    max_mem = int(round(max_mem_user*memory_available))
                else:
                    max_mem = int(max_mem_user)
            else:
                max_mem = int(round(self.__default_mem_fraction*memory_available))
                    
                    
            casalog.post("Will use up to %sMB of memory at node %s" % (str(max_mem),hostname), "INFO","config_cluster")
            
            # User can provide an estimation of the amount of RAM memory necessary per engine        
            mem_per_engine = self.__default_mem_per_engine
            if len(xyzd)>=5:
                mem_per_engine_user = float(xyzd[4])
                if (mem_per_engine_user>self.__default_mem_per_engine):
                    mem_per_engine = mem_per_engine_user
            
            # Apply heuristics: If memory limits number of engines then we can increase the number of openMP threads
            nengines=int(round(float(max_mem)/float(mem_per_engine)))
            if (nengines < 2):
                casalog.post("Free memory available %sMB would not support cluster mode at node %s, starting only 1 engine"
                             % (str(max_mem),hostname), "WARN","config_cluster")
                nengines=1
            else:
                casalog.post("Required memory per engine %sMB allows to deploy up to %s engines at node %s " 
                             % (str(mem_per_engine),str(nengines),hostname), "INFO","config_cluster")
        
            nengines = int(nengines)
            if (nengines>max_engines): 
                casalog.post("Cap number of engines deployed at node %s from %s to %s in order to meet maximum number of engines constraint" 
                             % (hostname,str(nengines),str(max_engines)), "INFO","config_cluster")
                nengines = max_engines       
            
            omp_num_nthreads = 1
            while (nengines*omp_num_nthreads*2 <= max_engines):
                omp_num_nthreads *= 2
                
            if (omp_num_nthreads>1):
                casalog.post("Enabling openMP to %s threads per engine at node %s" % (str(omp_num_nthreads),hostname), "INFO","config_cluster")
                
            self._hosts.append([a, nengines, c, omp_num_nthreads])
    
        self._configdone=self.validate_hosts()

        if not self._configdone:
            casalog.post("Failed to configure the cluster", "WARN","config_cluster")

    def check_host_resources(self,hostname):
        """
        jagonzal (CAS-4276 - New cluster specification file): Retrieve resources available
        at target node in order to dynamically deploy the engines to fit the idle capacity
        """
        
        casalog.origin("simple_cluster")
        
        ncores = 0
        memory = 0.
        cmd_os = self.shell(hostname) + " 'uname -s'"
        os = commands.getoutput(cmd_os)
        if (os == "Linux"):
            cmd_ncores = self.shell(hostname) + " 'cat /proc/cpuinfo' "           
            res_ncores = commands.getoutput(cmd_ncores)
            str_ncores = res_ncores.count("processor")
            
            try:
                ncores = int(str_ncores)
            except:
                casalog.post("Problem converting number of cores into numerical format at node %s: %s" % (hostname,str_ncores),"WARN","check_host_resources")
                pass
            
            memory=0.0
            for memtype in ['MemFree',
                            'Buffers',
                            'Cached' # "Cached" has a the problem that there can also be "SwapCached"
                            ]:
                cmd_memory = self.shell(hostname) + " 'cat /proc/meminfo | grep -v SwapCached | grep "+memtype+"'"
                str_memory = commands.getoutput(cmd_memory)
                str_memory = string.replace(str_memory,memtype+':','')
                str_memory = string.replace(str_memory,"kB","")
                str_memory = string.replace(str_memory," ","")
            
                try:
                    memory += float(str_memory)/1024.
                except:
                    casalog.post("Problem converting memory into numerical format at node %s: %s" % (hostname,str_memory),"WARN","check_host_resources")
                    break
            
            cmd_cpu = self.shell(hostname) + " 'top -b -n 1 | grep Cpu' "
            str_cpu = commands.getoutput(cmd_cpu)
            list_cpu = string.split(str_cpu,',')
            str_cpu = "100"
            for item in list_cpu:
                if item.count("%id")>0:
                    str_cpu = string.replace(item,"%id","")
                    str_cpu = string.replace(str_cpu," ","")
            
            try:
                cpu = float(str_cpu)
            except:
                casalog.post("Problem converting available cpu into numerical format at node %s: %s" % (hostname,str_cpu),"WARN","check_host_resources")
            
        else: # Mac OSX
            cmd_ncores = self.shell(hostname) + " '/usr/sbin/sysctl -n hw.ncpu'"
            res_ncores = commands.getoutput(cmd_ncores)
            str_ncores = res_ncores
            
            try:
                ncores = int(str_ncores)
            except:
                casalog.post("Problem converting number of cores into numerical format at node %s: %s" % (hostname,str_ncores),"WARN","check_host_resources")
                pass            
            
            cmd_memory = self.shell(hostname) + " 'top -l 1 | grep PhysMem: | cut -d , -f5 ' "
            str_memory = commands.getoutput(cmd_memory)
            str_memory = string.replace(str_memory,"M free.","")
            str_memory = string.replace(str_memory," ","")

            try:
                memory = float(str_memory)
            except:
                casalog.post("Problem converting memory into numerical format at node %s: %s" % (hostname,str_memory),"WARN","check_host_resources")
                pass
            
            cmd_cpu = self.shell(hostname) + " 'top -l1 | grep usage' "
            str_cpu = commands.getoutput(cmd_cpu)
            list_cpu = string.split(str_cpu,',')
            str_cpu = "100"
            for item in list_cpu:
                if item.count("% idle")>0:
                    str_cpu = string.replace(item,"% idle","")
                    str_cpu = string.replace(str_cpu," ","")
            
            try:
                cpu = float(str_cpu)
            except:
                casalog.post("Problem converting available cpu into numerical format at node %s: %s" % (hostname,str_cpu),"WARN","check_host_resources")            
        
        ncores = int(ncores)
        memory = int(round(memory))
        casalog.post("Host %s, Number of cores %s, Memory available %sMB, CPU capacity available %s%%" % (hostname,str(ncores),str(memory),str(cpu)), "INFO","check_host_resources")
        
        return (ncores,memory,cpu)
            
    
    def validate_hosts(self):
        """Validate the cluster specification.

        This function is normally called internally by configure_cluster
        function. 

        """
        
        casalog.origin("simple_cluster")
        
        # jagonzal (CAS-4287): Add a cluster-less mode to by-pass parallel processing for MMSs as requested 
        if (len(self._hosts)==1):
            if (self._hosts[0][1] == 1):
                if (self.uniqueIP(self._hosts[0][0]) == self.uniqueIP("localhost")):
                    casalog.post("Only one engine can be deployed at localhost, disabling cluster mode","WARN","validate_hosts")
                    from parallel.parallel_task_helper import ParallelTaskHelper
                    ParallelTaskHelper.bypassParallelProcessing(1)
                    return False
        
        uhost=set()
        for i in range(len(self._hosts)):
            uhost.add(self._hosts[i][0])
        if len(uhost)==0:
            casalog.post("Configuration table is empty","WARN","validate_hosts")
            return False
        if len(uhost)<len(self._hosts):
            casalog.post("Configuration table contains repeated node name","WARN","validate_hosts")
            return False
        for i in range(len(self._hosts)):
            if type(self._hosts[i][1])!=int:
                casalog.post("Number of engines must be an integer","WARN","validate_hosts")
                return False
        for i in range(len(self._hosts)):
            if not os.path.exists(self._hosts[i][2]):
                casalog.post("Directory %s does not exist" % self._hosts[i][2],"WARN","validate_hosts")
                return False
        for i in range(len(self._hosts)):
            try:
                tfile=self._hosts[i][2]+'/nosuchfail'
                fid = open(tfile, 'w')
                fid.close()
                os.remove(tfile)
            except IOError:
                casalog.post("Failed write permission to directory %s" % self._hosts[i][2],"WARN","validate_hosts")
                return False
        return True
    
    
    ###########################################################################
    ###   project management
    ###########################################################################
    def create_project(self, proj=""):
        """Create a project. 

        Keyword arguments:
        proj -- the name of project (default: 'proj'+timestamp).  

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        This function is normally called internally by init_cluster function. 

        Example:
        CASA <33>: sl.list_projects
        host: casa-dev-07 ------------------------>>>>
        bProj  bsplit  csplit  my_project
        host: casa-dev-08 ------------------------>>>>
        bProj  bsplit  csplit  my_project
        host: casa-dev-10 ------------------------>>>>
        bProj  bsplit  csplit

        CASA <34>: sl.create_project('dflag')
        output directory:
        /home/casa-dev-07/hye/ptest/dflag
        /home/casa-dev-08/hye/ptest/dflag
        /home/casa-dev-10/hye/ptest/dflag

        CASA <36>: sl.list_projects
        host: casa-dev-07 ------------------------>>>>
        bProj  bsplit  csplit  dflag  my_project
        host: casa-dev-08 ------------------------>>>>
        bProj  bsplit  csplit  dflag  my_project
        host: casa-dev-10 ------------------------>>>>
        bProj  bsplit  csplit  dflag

        """
        
        casalog.origin("simple_cluster")
        
        if not self._configdone:
            return
        if type(proj)!=str:
            casalog.post("Project name must be string","WARN","create_project")
            return
    
        self._project=proj.strip()
        if self._project=="":
            tm=time.strftime("%Y%b%d-%Hh%Mm%Ss", time.localtime())
            self._project='proj'+tm
    
        for i in range(len(self._hosts)):
            if not os.path.exists(self._hosts[i][2]+'/'+proj.strip()):
                cmd='mkdir '+self._hosts[i][2]+'/'+self._project
                os.system(cmd)
    
    def do_project(self, proj):
        """Use a project previously created. 

        Keyword arguments:
        proj -- the name of project.

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        Example:
        CASA <38>: sl._project
        Out[38]: 'dflag'

        CASA <39>: sl.do_project('csplit')
        output directory:
        /home/casa-dev-07/hye/ptest/csplit
        /home/casa-dev-08/hye/ptest/csplit
        /home/casa-dev-10/hye/ptest/csplit

        CASA <40>: sl._project
        Out[40]: 'csplit'

        """
        
        casalog.origin("simple_cluster")
        
        if not self._configdone:
            return
        if type(proj)!=str or proj.strip()=="":
            casalog.post("Project name must be a nonempty string","WARN","do_project")
            return
    
        projexist=True
        for i in range(len(self._hosts)):
            if not os.path.exists(self._hosts[i][2]+'/'+proj.strip()):
                casalog.post("No project directory found on %s" % self._hosts[i][0],"WARN","do_project")
                projexist=False
        if projexist:
            self._project=proj.strip()
        casalog.post("Output directory:","INFO","do_project")
        for i in range(len(self._hosts)):
            casalog.post("Output directory: %s/%s" % (self._hosts[i][2],self._project),"INFO","do_project")
    
    def erase_project(self, proj):
        """Erase files and dirs of a project. 

        Keyword arguments:
        proj -- the name of project.

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        Example:
        CASA <30>: sl.list_projects
        host: casa-dev-07 ------------------------>>>>
        aNew  bProj  bsplit  csplit  my_project
        host: casa-dev-08 ------------------------>>>>
        aNew  bProj  bsplit  csplit  my_project
        host: casa-dev-10 ------------------------>>>>
        bProj  bsplit  csplit

        CASA <31>: sl.erase_project('aNew')

        CASA <32>: sl.list_projects
        host: casa-dev-07 ------------------------>>>>
        bProj  bsplit  csplit  my_project
        host: casa-dev-08 ------------------------>>>>
        bProj  bsplit  csplit  my_project
        host: casa-dev-10 ------------------------>>>>
        bProj  bsplit  csplit

        """
        
        casalog.origin("simple_cluster")
        
        if not self._configdone:
            return
        if type(proj)!=str or proj.strip()=="":
            casalog.post("Project name must be a nonempty string","WARN","erase_project")
            return
        for i in range(len(self._hosts)):
            cmd='rm -rf '+self._hosts[i][2]+'/'+proj
            os.system(cmd)
        if proj==self._project:
            self._project=""
        if self._project=="":
            pass
    
    def clear_project(self, proj):
        """Remove all previous results of the proj

        Keyword arguments:
        proj -- the name of project.

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        Example:
        CASA <27>: sl.list_project('my_project')
        host: casa-dev-07 ------------------------>>>>
        casapy-20101122165601-5.log  sim.alma.csv.mid-f0-s0-b640-e768.flux
        host: casa-dev-08 ------------------------>>>>
        casapy-20101122165601-6.log  sim.alma.csv.mid-f0-s0-b768-e896.flux
        host: casa-dev-10 ------------------------>>>>
        casapy-20101122165601-7.log  sim.alma.csv.mid-f0-s0-b320-e640.flux

        CASA <28>: sl.clear_project('my_project')

        CASA <29>: sl.list_project('my_project')
        host: casa-dev-07 ------------------------>>>>
        host: casa-dev-08 ------------------------>>>>
        host: casa-dev-10 ------------------------>>>>

        """
        
        casalog.origin("simple_cluster")
        
        # This can be a slow operation, it is better to do it parallel
        if not self._configdone:
            return
        if type(proj)!=str or proj.strip()=="":
            casalog.post("Project name must be a nonempty string","WARN","clear_project")
            return
        for i in range(len(self._hosts)):
            cmd='rm -rf '+self._hosts[i][2]+'/'+proj.strip()+'/*'
            os.system(cmd)
    
    def list_project(self, proj):
        """List previous results of the proj

        Keyword arguments:
        proj -- the name of project.

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        Example:
        CASA <19>: sl.list_project('bsplit')
        host: casa-dev-07 ------------------------>>>>
        test_regression_TDEM0003-f0-s11.ms   test_regression_TDEM0003-f0-s13.ms
        host: casa-dev-08 ------------------------>>>>
        test_regression_TDEM0003-f0-s10.ms  test_regression_TDEM0003-f3-s9.ms
        host: casa-dev-10 ------------------------>>>>
        test_regression_TDEM0003-f0-s0.ms   test_regression_TDEM0003-f4-s11.ms

        """
        
        casalog.origin("simple_cluster")
        
        if not self._configdone:
            return
        if type(proj)!=str or proj.strip()=="":
            casalog.post("Project name must be a nonempty string","WARN","list_project")
            return
        for i in range(len(self._hosts)):
            casalog.post("Host: %s ------------------------>>>>" % self._hosts[i][0] ,"INFO","list_project")
            cmd='ls '+self._hosts[i][2]+'/'+proj
            os.system(cmd)
    
    def erase_projects(self):
        """Erase all previous results of all projects

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        """
        if not self._configdone:
            return
        for i in range(len(self._hosts)):
            cmd='rm -rf '+self._hosts[i][2]+'/*'
            os.system(cmd)
        self._project=""
    
    def list_projects(self):
        """List all previous projects

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        Example:
        <CASA 16:>sl.list_projects
        host: casa-dev-07 ------------------------>>>>
        aNew  bProj  bsplit  csplit  my_project
        host: casa-dev-08 ------------------------>>>>
        aNew  bProj  bsplit  csplit  my_project
        host: casa-dev-10 ------------------------>>>>
        bProj  bsplit  csplit

        """
        
        casalog.origin("simple_cluster")
        
        if not self._configdone:
            return
        for i in range(len(self._hosts)):
            casalog.post("Host: %s ------------------------>>>>" % self._hosts[i][0] ,"INFO","list_projects")
            cmd='ls '+self._hosts[i][2]+'/' 
            os.system(cmd)
    
    def reset_project(self):
        """Erase previous result and reset the status current project.

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        Example:
        CASA <43>: sl.list_project('bProj')
        ....too many here...

        CASA <44>: sl.do_project('bProj')
        output directory:
        /home/casa-dev-07/hye/ptest/bProj
        /home/casa-dev-08/hye/ptest/bProj
        /home/casa-dev-10/hye/ptest/bProj

        CASA <45>: sl.list_project('bProj')
        host: casa-dev-07 ------------------------>>>>
        host: casa-dev-08 ------------------------>>>>
        host: casa-dev-10 ------------------------>>>>

        """
        if not self._configdone:
            return
        self.stop_monitor()
        self.clear_project(self._project)
        self._jobs={}
        self._job_title=1
        self._monitor_on=True
    
    def get_hosts(self):
        """List current cluster.

        CASA <48>: sl.get_hosts
        Out[48]:
        [['casa-dev-07', 4, '/home/casa-dev-07/hye/ptest'],
         ['casa-dev-08', 4, '/home/casa-dev-08/hye/ptest'],
         ['casa-dev-10', 4, '/home/casa-dev-10/hye/ptest']]

        """
        return self._hosts 
    
    ###########################################################################
    ###   cluster management
    ###########################################################################
    
    # jagonzal (CAS-4292): This method is deprecated, because I'd like to 
    # avoid brute-force methods like killall which just hide state errors
    def cold_start(self):
        """kill all engines on all hosts. Shutdown current cluster.
        
        This is used if a complete restart of the cluster is needed. One can
        rerun init_cluster after this. This also kills possible leftover
        engines from previous sessions.

        """
        if not self._configdone:
            return
        # jagonzal (CAS-4292): Stop the cluster via parallel_go before using brute-force killall
        self.stop_cluster()
        for i in range(len(self._hosts)):
            hostname = self._hosts[i][0]
            cmd=self.shell(hostname) + ' "killall -9 ipengine"'
            os.system(cmd)
            
    #   jagonzal (CAS-4292): Method for gracefull finalization (e.g.: when closing casa)
    def stop_cluster(self):
        """ Destructor method to shut down the cluster gracefully """
        # Stop cluster and thread services
        self.stop_monitor()
        # Now we can stop the cluster w/o problems
        # jagonzal (CAS-4292): Stop the cluster w/o using brute-force killall as in cold_start
        self._cluster.stop_cluster()            
        self.__running = False
    
    def stop_nodes(self):
        """Stop all engines on all hosts of current cluster.
        
        After running this, the cluster contains no engines.

        """
        if not self._configdone:
            return
        for i in self._cluster.get_nodes():
            self._cluster.stop_node(i)
    
    def start_cluster(self):
        """Start a cluster with current configuration.
        
        Normally, one does not need to run this function directly. The 
        init_cluster will call this internally.

        """
        
        casalog.origin("simple_cluster")
        
        if not self._configdone:
            return
        for i in range(len(self._hosts)):
            self._cluster.start_engine(self._hosts[i][0], self._hosts[i][1], 
                                   self._hosts[i][2]+'/'+self._project,self._hosts[i][3])
            if (self._hosts[i][3]>1):
                omp_num_threads = self._cluster.execute("print os.environ['OMP_NUM_THREADS']",i)[0]['stdout']
                if (omp_num_threads.count(str(self._hosts[i][3]))>0):
                    casalog.post("Open MP enabled at host %s,  OMP_NUM_THREADS=%s" % (self._hosts[i][0],str(self._hosts[i][3])), "INFO","start_cluster")
                else:
                    casalog.post("Problem enabling Open MP at host %s: %s" % (self._hosts[i][0],omp_num_threads),"WARN","start_cluster")
                    
        self.start_logger()
        self.start_monitor()
        # jagonzal (CAS-4324): Is better to update the resources in the
        # check_status method, after updating the status of the jobs
        # self.start_resource()
        self._rsrc = self.show_resource(True)
    
    def get_host(self, id):
        """Find out the name of the node that hosts this engine.

        Keyword arguments:
        id -- the engine id

        Example:
        CASA <50>: sl.get_host(8)
        Out[50]: 'casa-dev-10'

        """
        
        casalog.origin("simple_cluster")
        
        if not self._configdone:
            return
        ids=self._cluster.get_ids()
        if type(id)!=int:
            casalog.post("The argument must be an engine id (int)","WARN","get_host")
            return ''
        if ids.count(id)!=1:
            casalog.post("Engine %d does not exist" % id,"WARN","get_host")
            return ''
        e=self._cluster.get_engines()
        for i in range(len(e)):
            if e[i][0]==id:
                return e[i][1] 
    
    def get_engine_store(self, id):
        """Get the root path where an engine writes out result

        Keyword arguments:
        id -- the engine id

        Example:
        CASA <52>: sl.get_engine_store(8)
        Out[52]: '/home/casa-dev-10/hye/ptest/bProj/'

        """
        if not self._configdone:
            return
        hst=self.get_host(id)
        for i in range(len(self._hosts)):
            if self._hosts[i][0]==hst:
                pth=self._hosts[i][2]
                sl=''
                if not pth.endswith('/'):
                    sl='/' 
                return pth+sl+self._project+'/'
    
    ###########################################################################
    ###   log management
    ###########################################################################
    def start_logger(self):
        """Link all engine logs to the current directory. 

        After running this, the current directory contains links to each of
        the engine logs with file name 'engine-[id].log such that one can 
        conveniently browse engine logs with casa logviewer.

        """
        if not self._configdone:
            return
        lg=self._cluster.get_casalogs()
        os.system('rm -f engine-*.log')
        for i in lg:
            eng='engine'+i[str.rfind(i,'-'):]
            os.symlink(i, eng)
    
    ###########################################################################
    ###   resource management
    ###########################################################################   
    def start_resource(self): 
        """Start monitoring resource usage.

        Four critical resource usage indicators (for parallel execution), 
        namely, %cpu, %iowait, %mem and %memswap on all hosts are continuously
        checked. This infomation can be used to tune the parallel performance.

        Normally, one does not call this function directly. The init_cluster
        will call this function.

        """
        
        if not self._configdone:
            return
        self._resource_on=True 
        self._rsrc={}
        return thread.start_new_thread(self.update_resource, ())
    
    def update_resource(self):
        """Set up repeated resource checking.

        Four critical resource usage indicators (for parallel execution), 
        namely, %cpu, %iowait, %mem and %memswap on all hosts are continuously
        checked. This infomation can be used to tune the parallel performance.

        Normally, one does not call this function directly. The init_cluster
        will call this function.

        """
                    
        self._resource_running=True
        if not self._configdone:
            return
        while self._resource_on:
            if ((len(self._jobs.keys())>0) or (len(self._rsrc.keys())==0)):
                self.check_resource()
            time.sleep(5)
        self._resource_running=False
    
    def stop_resource(self): 
        """Stop monitoring resource usage.

        Four critical resource usage indicators (for parallel execution), 
        namely, %cpu, %iowait, %mem and %memswap on all hosts are continuously
        checked. This infomation can be used to tune the parallel performance.

        Normally, one does not call this function directly. The init_cluster
        will call this function.

        """
        if not self._configdone:
            return
        self._resource_on=False 
        while (self._resource_running):
            time.sleep(1)
        self._rsrc={}
    
    def show_resource(self, long=False):
        """
        jagonzal (CAS-4372): Old resource monitoring functions were causing crashes in NRAO cluster
        """
        if not self._configdone:
            return
        if long:
            return self.check_resource(False)
        else:
            self.check_resource(True)
    
    def check_resource(self, verbose_local=False):
        """
        jagonzal 29/05/12 (CAS-4137) - HPC project (CAS-4106)
        ========================================================================
        Advanced monitoring function that provides CPU,Memory,I/O and job queue
        stats per node and total per host. 

        There are 2 usage modes:
        - Called from the monitoring thread in regular time intervals 
          (i.e. within the check_status method), to only dump the stats 
          into a file or the terminal.
        - Called from the command line, to return a dictionary in addition 
          of dumping the stats into a file or into the terminal.

        In both cases logging can be controlled in the following way:
        - User can provide a file-name when creating the simple_cluster 
          object, trough the string parameter "monitoringFile", to specify 
          the location of the monitoring file, otherwise it defaults to 
          'monitoring.log' in the working directory. 
        - User can also switch verbose mode when creating the simple_cluster 
          object, trough the boolean parameter "verbose", to have the monitoring 
          info dumped into the terminal, otherwise it defaults to False and it 
          only dumps into the monitoring file. 
        - User can even use the stand alone method show_state specifying 
          verbosity only for that particular call via the "verbose_local" 
          parameter, and the method returns a dictionary with all the stats 
          per node and total per host. 

        Examples:
        - Stand-Alone usage from terminal to return a dictionary
          from simple_cluster import *
          sc = simple_cluster.getCluster()
          stats = sc.show_state(False)
        - Stand-Alone usage from terminal to print the stats into the terminal
          from simple_cluster import *
          sc = simple_cluster.getCluster()
          sc.show_state(True)
        - Service-Mode usage to specify a custom file for dumping the stats
          from simple_cluster import *
          sc = simple_cluster('mycustomfile.log')
          sc.init_cluster('cluster-config.txt','test-rhel')
        - Service-Mode usage to specify a custom file for dumping the stats
          and verbose mode to additionally dump the stats into the terminal
          from simple_cluster import *
          sc = simple_cluster('mycustomfile.log',True)
          sc.init_cluster('cluster-config.txt','test-rhel')
        """

        # Determine verbose mode
        verbose = self._verbose or verbose_local

        # Open monitoring file
        fid = open(self._monitoringFile + ".tmp", 'w')
		
        # Print header
        print >> fid, "%20s%10s%10s%10s%10s%10s%10s%10s%15s%15s%20s%30s" % ( "Host","Engine","Status","CPU[%]","Memory[%]","Time[s]",
                                                                             "Read[MB]","Write[MB]","Read[MB/s]","Write[MB/s]","Job","Sub-MS")
        if (verbose):
            print "%20s%10s%10s%10s%10s%10s%10s%10s%15s%15s%20s%30s" % ("Host","Engine","Status","CPU[%]","Memory[%]","Time[s]",
                                                                        "Read[MB]","Write[MB]","Read[MB/s]","Write[MB/s]","Job","Sub-MS")

        result = {}
        engines_list = self._cluster.get_engines()
        for engine in engines_list:
            hostname = str(engine[1])            
            # First of all get operating system
            cms_os = self.shell(hostname) + " 'uname -s'"
            os = commands.getoutput(cms_os)
            # Get read/write activity
            read_bytes = 0.0
            write_bytes = 0.0
            if (os == "Linux"):
                # Get read activity
                cmd_read_bytes = self.shell(hostname) + " 'cat /proc/" + str(engine[2]) + "/io | grep read_bytes'"
                read_bytes=commands.getoutput(cmd_read_bytes)
                read_bytes = read_bytes.split(":")
                try:
                    read_bytes = float(read_bytes[1])
                except:
                    if (verbose):
                        print "Problem converting read_bytes into float for engine " + str(engine[0]) + " running in host " + str(engine[1])
                        print "read_bytes: [" +  str(read_bytes) + "]"
                    read_bytes = 0
                # Get write activity
                cmd_write_bytes = self.shell(hostname) + " 'cat /proc/" + str(engine[2]) + "/io | grep write_bytes | head -1'"
                write_bytes=commands.getoutput(cmd_write_bytes)
                write_bytes = write_bytes.split(":")
                try:
                    write_bytes = float(write_bytes[1])
                except:
                    if (verbose):
                        print "Problem converting write_bytes into float for engine " + str(engine[0]) + " running in host " + str(engine[1])
                        print "write_bytes: [" +  str(write_bytes) + "]"
                    write_bytes = 0.0
            # Get resources usage (cpu, mem, elapsed time since start)
            cmd_resources = self.shell(hostname) + " 'ps -p " + str(engine[2]) + " -o %cpu,%mem,etime' | tail -1"
            resources=commands.getoutput(cmd_resources)
            resources = resources.split(" ")
            while resources.count('')>0:
                resources.remove('')
            # Convert CPU into number
            cpu = 0
            try:
                cpu = round(float(resources[0]))
            except:
                if (verbose):
                        print "Problem converting CPU into float for engine " + str(engine[0]) + " running in host " + str(engine[1])
                        print "CPU: [" +  resources[0] + "]"
            # Convert Memory into number
            memory = 0
            try:
                memory = round(float(resources[1]))
            except:
                if (verbose):
                        print "Problem converting memory into float for engine " + str(engine[0]) + " running in host " + str(engine[1])
                        print "Memory: [" +  resources[1] + "]"
            # Initialize engine RW offsets map
            if not self._enginesRWoffsets.has_key(engine[0]):
                self._enginesRWoffsets[engine[0]] = {}
            # Store engine RW offsets values
            self._enginesRWoffsets[engine[0]]['read_offset'] = read_bytes
            self._enginesRWoffsets[engine[0]]['write_offset'] = write_bytes
            # Initialize host map
            if not result.has_key(engine[1]):
                result[engine[1]] = {}
                result[engine[1]]["CPU"] = 0.0
                result[engine[1]]["Memory"] = 0.0
                result[engine[1]]["Read"] = 0.0
                result[engine[1]]["Write"] = 0.0
                result[engine[1]]["ReadRate"] = 0.0
                result[engine[1]]["WriteRate"] = 0.0
            # Initialize engine map
            if not result[engine[1]].has_key(engine[0]):
                result[engine[1]][engine[0]] = {}
            # Store default engine values
            result[engine[1]][engine[0]]["CPU"] = cpu
            result[engine[1]][engine[0]]["Memory"] = memory
            result[engine[1]][engine[0]]["Read"] = 0
            result[engine[1]][engine[0]]["Write"] = 0
            result[engine[1]][engine[0]]["ReadRate"] = 0
            result[engine[1]][engine[0]]["WriteRate"] = 0
            result[engine[1]][engine[0]]["Sub-MS"] = ""
            result[engine[1]][engine[0]]["Status"] = "Idle"
            result[engine[1]][engine[0]]["Time"] = 0
            result[engine[1]][engine[0]]["Job"] = ""
            # Retrieve job status information from job structure
            for job in self._jobs.keys():
                jobEngine = self._jobs[job]['engine']
                if (jobEngine == engine[0]):           
                    result[engine[1]][engine[0]]["Sub-MS"] = self._jobs[job]['subms']
                    result[engine[1]][engine[0]]["Status"] = self._jobs[job]['status']
                    result[engine[1]][engine[0]]["Time"] = round(self._jobs[job]['time'])
                    result[engine[1]][engine[0]]["Job"] = self._jobs[job]['short'].split('=').pop().replace(' ','')
                    result[engine[1]][engine[0]]["Read"] = float(read_bytes - self._jobs[job]['read_offset'])/(1024*1024)
                    result[engine[1]][engine[0]]["Write"] = float(write_bytes - self._jobs[job]['write_offset'])/(1024*1024)                      
                    # Compute data rates
                    if (result[engine[1]][engine[0]]["Time"] > 0):
                        result[engine[1]][engine[0]]["ReadRate"] = result[engine[1]][engine[0]]["Read"] / result[engine[1]][engine[0]]["Time"]
                        result[engine[1]][engine[0]]["WriteRate"] = result[engine[1]][engine[0]]["Write"] / result[engine[1]][engine[0]]["Time"]
            # Accumulate host values
            result[engine[1]]["CPU"] += result[engine[1]][engine[0]]["CPU"]
            result[engine[1]]["Memory"] += result[engine[1]][engine[0]]["Memory"]
            result[engine[1]]["Read"] += result[engine[1]][engine[0]]["Read"]
            result[engine[1]]["Write"] += result[engine[1]][engine[0]]["Write"]
            result[engine[1]]["ReadRate"] += result[engine[1]][engine[0]]["ReadRate"]
            result[engine[1]]["WriteRate"] += result[engine[1]][engine[0]]["WriteRate"]
            # Print nodes info
            print >> fid, "%20s%10d%10s%10d%10d%10d%10d%10d%15d%15d%20s%30s" % ( engine[1],engine[0],
                                                                                 result[engine[1]][engine[0]]["Status"],
                                                                                 result[engine[1]][engine[0]]["CPU"],
                                                                                 result[engine[1]][engine[0]]["Memory"],
                                                                                 result[engine[1]][engine[0]]["Time"],
                                                                                 result[engine[1]][engine[0]]["Read"],
                                                                                 result[engine[1]][engine[0]]["Write"],
                                                                                 result[engine[1]][engine[0]]["ReadRate"],
                                                                                 result[engine[1]][engine[0]]["WriteRate"],
                                                                                 result[engine[1]][engine[0]]["Job"],
                                                                                 result[engine[1]][engine[0]]["Sub-MS"])
            if (verbose):
                print "%20s%10d%10s%10d%10d%10d%10d%10d%15d%15d%20s%30s" % ( engine[1],engine[0],
                                                                             result[engine[1]][engine[0]]["Status"],
                                                                             result[engine[1]][engine[0]]["CPU"],
                                                                             result[engine[1]][engine[0]]["Memory"],
                                                                             result[engine[1]][engine[0]]["Time"],
                                                                             result[engine[1]][engine[0]]["Read"],
                                                                             result[engine[1]][engine[0]]["Write"],
                                                                             result[engine[1]][engine[0]]["ReadRate"],
                                                                             result[engine[1]][engine[0]]["WriteRate"],
                                                                             result[engine[1]][engine[0]]["Job"],
                                                                             result[engine[1]][engine[0]]["Sub-MS"])

        # Print separation between nodes and hosts info
        print >> fid, "%20s%10s%10s%10s%10s%10s%10s%10s%15s%15s%20s%30s" % ( "====================",
                                                                             "==========",
                                                                             "==========",
                                                                             "==========",
                                                                             "==========",
                                                                             "==========",
                                                                             "==========",
                                                                             "==========",
                                                                             "===============",
                                                                             "===============",
                                                                             "====================",
                                                                             "==============================")

        if (verbose):
            print "%20s%10s%10s%10s%10s%10s%10s%10s%15s%15s%20s%30s" % ( "====================",
                                                                         "==========",
                                                                         "==========",
                                                                         "==========",
                                                                         "==========",
                                                                         "==========",
                                                                         "==========",
                                                                         "==========",
                                                                         "===============",
                                                                         "===============",
                                                                         "====================",
                                                                         "==============================")

        # Print hosts info
        for host in result:
            print >> fid, "%20s%10s%10s%10d%10d%10s%10d%10d%15d%15d%20s%30s" % ( host,"Total","",
                                                                                 result[host]["CPU"],
                                                                                 result[host]["Memory"],
                                                                                 "",
                                                                                 result[host]["Read"],
                                                                                 result[host]["Write"],
                                                                                 result[host]["ReadRate"],
                                                                                 result[host]["WriteRate"],
                                                                                 "","")
        if (verbose):
            for host in result:
                print "%20s%10s%10s%10d%10d%10s%10d%10d%15d%15d%20s%30s" % ( host,"Total","",
                                                                             result[host]["CPU"],
                                                                             result[host]["Memory"],
                                                                             "",
                                                                             result[host]["Read"],
                                                                             result[host]["Write"],
                                                                             result[host]["ReadRate"],
                                                                             result[host]["WriteRate"],
                                                                             "","")

        # Print final separator
        print >> fid, "%20s%10s%10s%10s%10s%10s%10s%10s%15s%15s%20s%30s" % ( "====================",
                                                                             "==========",
                                                                             "==========",
                                                                             "==========",
                                                                             "==========",
                                                                             "==========",
                                                                             "==========",
                                                                             "==========",
                                                                             "===============",
                                                                             "===============",
                                                                             "====================",
                                                                             "==============================")
        if (verbose):
            print "%20s%10s%10s%10s%10s%10s%10s%10s%15s%15s%20s%30s" % ( "====================",
                                                                         "==========",
                                                                         "==========",
                                                                         "==========",
                                                                         "==========",
                                                                         "==========",
                                                                         "==========",
                                                                         "==========",
                                                                         "===============",
                                                                         "===============",
                                                                         "====================",
                                                                         "==============================")

        # Close monitoring file
        fid.close()

        # Rename monitoring file
        commands.getstatusoutput("mv " + self._monitoringFile + ".tmp" + " " + self._monitoringFile)

        # Update resource member
        self._rsrc = result
        
        return result

    def get_return_list(self):
		"""
		jagonzal (CAS-4376): Gather return variables from the different engines back to the main CASA controller instance
		"""

		return_list = {}
		jobQueue = self._JobQueueManager.getOutputJobs()
		for job in jobQueue:
			return_list[job.getCommandArguments()['vis']]=job.getReturnValues()

		return return_list


    ###########################################################################
    ###   execution status management
    ###########################################################################
    def check_job(self):
        """Check the execution status of current noblock jobs  on all engines.

        This function can be used to block the terminal until all submitted
        jobs finish. 

        Example:
        CASA <2>: from simple_cluster import simple_cluster
        CASA <3>: sl=simple_cluster()
        CASA <4>: sl.init_cluster("my_cluster", "csplit")
        CASA <5>: sl.simple_split('/lustre/casa-store/hye/10B-209a_5s.ms/', '')
        CASA <6>: sl.check_job()

        """

        if not self._configdone:
            return
        done=False
        while(not done):
            time.sleep(5)
            done=True
            for i in sl._jobs.keys():
                try:
                    if not self._cluster.check_job(i):
                        done=False
                except:
                    traceback.print_tb(sys.exc_info()[2])
                    raise
    
    def check_status(self, notify=False):
        """Check the execution status of submitted no-block jobs

        Keyword arguments:
        notify -- whether or not to display detailed resource usage info

        Normally, one does not call this function directly. The start_monitor
        will call this function internally.

        """
        
        casalog.origin("simple_cluster")

        self._monitor_running = True

        if not self._configdone:
            return
        while self._monitor_on:
            time.sleep(5)
            curr={}
            try:
                pend=self._cluster.queue_status()
                for i in xrange(len(pend)):
                    (a, b)=pend[i]
                    curr[a]=b['pending']
            except:
                # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
                # traceback.print_tb(sys.exc_info()[2])
                pass
            for job in self._jobs.keys():
                if type(job)==type(None):
                    self._jobs[job]['status']="unknown"
                else:
                    eng=self._jobs[job]['engine']
                    sht=self._jobs[job]['short']
                    try:
                        x=1
                        try:
                            x=job.get_result(block=False)
                        except client.CompositeError, exception:
                            if notify and self._jobs[job]['status']=="scheduled":
                                casalog.post("Error retrieving result of job %s from engine %s: %s, backtrace:" % (sht,str(eng),str(exception)),"SEVERE","check_status")
                                exception.print_tracebacks()
                            self._jobs[job]['status']="broken"
                        except:
                            casalog.post("Error retrieving result of job %s from engine %s, backtrace:" % (sht,str(eng)),"SEVERE","check_status")
                            traceback.print_tb(sys.exc_info()[2])

                        if x==None:
                            cmd=self._jobs[job]['command']
                            if curr.has_key(eng):
                                wk=eval(curr[eng].lstrip('execute(').rstrip(')'))
                                if wk==cmd:
                                    self._jobs[job]['status']="running"
                                    if self._jobs[job]['start']=='':
                                        if notify:
                                            casalog.post("Engine %d job %s started" %(eng,sht),"INFO","check_status")
                                        self._jobs[job]['start']=time.time()
                                    self._jobs[job]['time']=time.time()-self._jobs[job]['start'] 
                                else:
                                    pass
                        else:
                            if self._jobs[job]['status']=="running":
                                if notify:
                                    casalog.post("Engine %d job %s finished" %(eng,sht),"INFO","check_status")
                                self._jobs[job]['status']="done"
                            if self._jobs[job]['status']=="scheduled":
                                if isinstance(x, int):
                                    if notify:
                                        casalog.post("Engine %d job %s broken" %(eng,sht),"SEVERE","check_status")
                                    self._jobs[job]['status']="broken"
                                else:
                                    if notify:
                                        casalog.post("Engine %d job %s finished" %(eng,sht),"INFO","check_status")
                                    self._jobs[job]['status']="done"
                    except:
                        if notify and self._jobs[job]['status']=="running":
                            casalog.post("Engine %d job %s broken" %(eng,sht),"SEVERE","check_status")
                        self._jobs[job]['status']="broken"
                
            # jagonzal (CAS-4324): This method consumes lots of resources, and the user terminal
            # is not very responsive while it's running, so we execute it only when there are jobs
            # beign processed.        
            if (len(self._jobs.keys())>0):
                try:
                    self.check_resource()
                except:
                    pass

            gr=set()
            for val in self._jobs.values():
                gr.add(val['jobgroup'])
    	    for i in list(gr):
                eml=''
                jobname='None'
                if i.count(':')>0:
                    pos=i.find(':')
                    jobname=i[pos+1:].strip()
                    eml=i[:pos].strip()
                elif i.count('@')>0:
                    eml=i.strip()
                else:
                    jobname=i.strip()
                
                finish=True
                for val in self._jobs.values():
                    if val['jobgroup']==i and not (val['status']=='done' or 
                                                val['status']=='broken'):
                        finish=False
                if finish:
                    tm=time.ctime()
                    jobname=jobname[0:30]
                    msg='\n#### '+jobname+' '+ \
                          '#'*(68-len(jobname)-len(tm))+' '+tm+' ####'
                    msg+='\nengine    status  time(s)  command\n'
                    rmv=[]
                    for job in self._jobs.keys():
                        if self._jobs[job]['jobgroup']==i:
                            msg+="%6d%10s%9d%2s%s\n" % (self._jobs[job]['engine'], 
                                  self._jobs[job]['status'],
                                  int(self._jobs[job]['time']), 
                                  '  ',
                                  self._jobs[job]['command'])
                            rmv.append(self._jobs[job]['jobname'])
                    if i.strip()!='':
    
                        # Append to project result file
                        f=open(self._project+'.result', 'a')
                        f.write(msg)
                        f.write('\n')
                        f.close()
    
                        if eml!='' and eml.count(' ')==0:
                            #send email
                            #import smtplib
                            #from email.mime.text import MIMEText
                            #mal=MIMEText(msg)
                            #mal['Subject']='your parallel job finished'
                            #me=os.environ['USER']
                            #mal['From']=me
                            #mal['To']=i 
                            #s=smtplib.SMTP()
                            #s.connect()
                            #s.sendmail(me, [i], mal.as_string())
                            #s.close()
        
                            msgf='/tmp/emailmsg.txt'
                            f=open(msgf, 'w')
                            f.write(msg)
                            f.write('\n')
                            f.close()
                            cmd='/bin/mail -s "parallel job \''+jobname+\
                                '\' finished" '+eml+' < '+msgf
                            os.system(cmd)
                             
                        for j in rmv:
                            self.remove_record(j)

        self._monitor_running = False
                            
    
    def start_monitor(self): 
        """Start monitoring  execution status of submitted no-block jobs

        Normally, one does not call this function directly. The init_cluster
        will call this function.

        """
        if not self._configdone:
            return
        self._monitor_on=True 
        return thread.start_new_thread(self.check_status, (True,))
    
    def stop_monitor(self): 
        """Stop monitoring execution status of submitted no-block jobs

        Normally, one does not call this function directly. 

        """
        if not self._configdone:
            return
        self._monitor_on=False
        while (self._monitor_running):
            time.sleep(1)
    
    def show_queue(self):
        """Display job queue.

        Example:
        CASA <2>: from simple_cluster import simple_cluster
        CASA <3>: sl=simple_cluster()
        CASA <4>: sl.init_cluster("my_cluster", "csplit")
        CASA <5>: sl.simple_split('/lustre/casa-store/hye/10B-209a_5s.ms/', 
                                  'you@nrao.edu:3rd split')
        CASA <6>: sl.show_queue()

        """
        if not self._configdone:
            return
        return self._cluster.queue_status()
    
    def get_status(self, long=False):
        """Display job execution status.

        Keyword arguments:
        long -- whether or not to display detailed execution status info

        Example:
        CASA <2>: from simple_cluster import simple_cluster
        CASA <3>: sl=simple_cluster()
        CASA <4>: sl.init_cluster("my_cluster", "csplit")
        CASA <5>: sl.simple_split('/lustre/casa-store/hye/10B-209a_5s.ms/', 
                                  'you@nrao.edu:3rd split')
        CASA <6>: sl.get_status()
        engine    status  time(s)     start  command   title
             0      done       31  16:41:56    split      15
             2 scheduled        0              split      78
             7      done       41  16:42:38    split      16
             9   running       51  16:42:59    split      17
             1      done       36  16:41:56    split      18

        """

        if not self._configdone:
            return
        if long:
            return self._jobs
        else:
            if len(self._jobs.keys())==0:
                return self._jobs
            else:
                print 'engine    status  time(s)     start  command   title'
                for job in self._jobs.keys():
                    print "%6d%10s%9d%10s%9s%8d" % (self._jobs[job]['engine'], 
                           self._jobs[job]['status'],
                           int(self._jobs[job]['time']), 
                           '' if type(self._jobs[job]['start'])==str 
                              else
                                 time.strftime("%H:%M:%S", 
                                        time.localtime(self._jobs[job]['start'])),
                           self._jobs[job]['short'].strip()[:9],
                           self._jobs[job]['jobname'])

    def get_jobId(self, status):
        """Get a list of jobs of the given status

        Keyword arguments:
        status -- the job status or the job title

        Example:
        CASA <2>: from simple_cluster import simple_cluster
        CASA <3>: sl=simple_cluster()
        CASA <4>: sl.init_cluster("my_cluster", "csplit")
        CASA <5>: sl.simple_split('/lustre/casa-store/hye/10B-209a_5s.ms/', 
                                  'you@nrao.edu:3rd split')
        CASA <6>: sl.get_jobId('done')

        """

        if not self._configdone:
            return []
        if len(self._jobs.keys())==0:
            return [] 
        else:
            jobId=[] 
            for job in self._jobs.keys():
                if self._jobs[job]['status']==status:
                    jobId.append(self._jobs[job]['jobname'])
            return jobId

    def remove_record(self, jobname=None):
        """Remove job execution status of a job.

        Keyword arguments:
        jobname -- the jobname or status of job(s) to be removed from display

        if jobName is not specified or is None all jobs are removed.
        """

        if not self._configdone:
            return
        
        for job in self._jobs.keys():
            if jobname is None:
                del self._jobs[job]
            elif type(jobname)==int and self._jobs[job]['jobname']==jobname:
               del self._jobs[job]
            elif type(jobname)==str and self._jobs[job]['status']==jobname:
               del self._jobs[job]
    
    ###########################################################################
    ###   job distribution functions
    ###########################################################################
    def make_call(self, func, param):
        """Make a function call string with function name and parameters.

        Keyword arguments:
        func -- the name of the function
        param -- the dictionary of parameters and values

        Example:
        CASA <12>: param=dict()
        CASA <13>: param['vis']='NGC5921.ms'
        CASA <14>: param['spw']='4'
        CASA <15>: sl.make_call('flagdata', param)
          Out[15]: 'flagdata(vis="NGC5921.ms", spw=4)'

        """
        
        casalog.origin("simple_cluster")
        
        if type(func)!=str or type(param)!=dict:
            casalog.post("Func must be a str and param must be a dictionary","WARN","make_call")
            return None 
        cmd=func+'('
        for (k, v) in param.iteritems():
            cmd+=k+'='
            if type(v)==str:
                cmd+='"'+v+'"'
            elif type(v)==np.ndarray:
                cmd+=repr(v)
            else:
                cmd+=str(v)
            cmd+=', '
        cmd=cmd[0:-2]
        cmd+=')'
        return cmd
    
    def do_and_record(self, cmd, id, group='', subMS=''):
        """Submit a function call to an engine and record its execution status.

        Keyword arguments:
        cmd -- the function call string
        id -- the id of the engine to be assigned 
        group -- the group this cmd belongs to and the receipt of notification
                 group can be an email address or a label of the job or both
                 separated by a ':'. Once all the jobs that has the same label
                 finish, an email notification will be sent.

        Example:
        CASA <12>: param=dict()
        CASA <13>: param['vis']='NGC5921.ms'
        CASA <14>: param['spw']='4'
        CASA <15>: cmd=sl.make_call('flagdata', param)
        CASA <17>: sl.do_and_record(cmd, 7, 'you@nrao.edu:flag ngc5921')

        """
        if not self._configdone:
            return
        job=self._cluster.odo(cmd, id)
        self._jobs[job]={}
        self._jobs[job]['start']=''
        self._jobs[job]['time']=0
        if self._enginesRWoffsets.has_key(id):
            self._jobs[job]['read_offset']=self._enginesRWoffsets[id]['read_offset']
            self._jobs[job]['write_offset']=self._enginesRWoffsets[id]['write_offset']
        else:
            self._jobs[job]['read_offset']=0
            self._jobs[job]['write_offset']=0
        self._jobs[job]['command']=cmd
        if len(cmd)<9:
            self._jobs[job]['short']=cmd
        else:
            self._jobs[job]['short']=cmd[:str.find(cmd, '(')]
        self._jobs[job]['subms']=subMS
        self._jobs[job]['status']="scheduled"
        self._jobs[job]['engine']=id
        self._jobs[job]['jobname']=self._job_title
        self._jobs[job]['jobgroup']=group
        self._job_title+=1
        return self._job_title-1
    
    ###########################################################################
    ###   result processing functions
    ###########################################################################
    
    def list_result(self):
        """read the project.result file and write out all labels

        Example:
        CASA <33>: sl.list_result
        Out[33]:
        ['#### new split ####################### Mon Mar 14 14:48:08 2011 ####',
         '#### flag ngc5921 #################### Wed Mar 16 10:43:12 2011 ####']

        """
        if not self._configdone:
            return
        f=open(self._project+'.result', 'r')
        s=f.readlines()
        vec=[]
        for line in s:
            sLine=line.rstrip()
            if str.find(sLine, '#### ')==0:
                vec.append(sLine.strip())
            else:
                continue
        f.close()
        return vec
    
    def get_result(self, tm):
        """read the project.result file and write out result for a label

        Keyword arguments:
        tm -- the result label

        Example:
        CASA <33>: sl.list_result
        Out[33]:
        ['#### new split ####################### Mon Mar 14 14:48:08 2011 ####',
         '#### flag ngc5921 #################### Wed Mar 16 10:43:12 2011 ####']
        CASA <34>: sl.get_result('new split')
        Out[34]:
        ....ommit...

        """

        if not self._configdone:
            return
        f=open(self._project+'.result', 'r')
        s=f.readlines()
        reach=False
        vec=[]
        for line in s:
            sLine=line.strip()
            if str.find(sLine, '#### ')==0:
                if str.count(sLine, ' '+tm+' ')>0 and not reach:
                    reach=True
                else:
                    reach=False
            else:
                if reach and sLine!='' and not sLine.startswith('engine'):
                    vec.append(sLine)
        f.close()
        return vec

    def erase_result(self, tm):
        """read the project.result file and erase result for a label

        Keyword arguments:
        tm -- the result label

        Example:
        CASA <33>: sl.list_result
        Out[33]:
        ['#### new split ####################### Mon Mar 14 14:48:08 2011 ####',
         '#### flag ngc5921 #################### Wed Mar 16 10:43:12 2011 ####']
        CASA <34>: sl.erase_result('flag ngc5921')
        CASA <35>: sl.list_result
        Out[35]:
        ['#### new split ####################### Mon Mar 14 14:48:08 2011 ####']

        """
        if not self._configdone:
            return
        if type(tm)!=str or len(tm)==tm.count('#'):
            return 
        f=open(self._project+'.result', 'r')
        s=f.readlines()
        f.close()
        a=-1
        b=-1
        for line in xrange(len(s)):
            sLine=s[line].strip()
            if str.find(sLine, '#### ')==0:
                if str.count(sLine, ' '+tm+' ')>0 and a==-1:
                    a=line
                else:
                    b=line
        if a>-1 and b>a:
            f=open(self._project+'.result', 'w')
            f.writelines(s[:a])
            f.writelines(s[b:])
            f.close()
        return
    
    def get_output(self, result, item, **kwargs):
        """pick from result list the item that meets condistion in kwargs

        Keyword arguments:
        result -- the result label or the result from running get_result 
        item -- the result item to get
        kwargs -- the conditions to limit the result

        Example:
        CASA <33>: sl.list_result
          Out[33]:
        ['#### new split ####################### Mon Mar 14 14:48:08 2011 ####',
         '#### flag ngc5921 #################### Wed Mar 16 10:43:12 2011 ####']
        CASA <34>: sl.get_result('new split')
          Out[34]:
        ['10      done       30  split(vis="/lustre/casa-store/hye/10B-209a_5s.ms", outputvis="/home/casa-dev-10/hye/ptest/csplit/10B-209a_5s-f5-s10.ms", spw="10", datacolumn="DATA", field="J0738+1742")',
         '1      done       40  split(vis="/lustre/casa-store/hye/10B-209a_5s.ms", outputvis="/home/casa-dev-07/hye/ptest/csplit/10B-209a_5s-f3-s2.ms", spw="2", datacolumn="DATA", field="J0738+1742")',
         '2      done       75  split(vis="/lustre/casa-store/hye/10B-209a_5s.ms", outputvis="/home/casa-dev-07/hye/ptest/csplit/10B-209a_5s-f4-s10.ms", spw="10", datacolumn="DATA", field="2MJ0746")',
         ..... many other entries ...]
        CASA <35>: sl.get_output('new split', 'outputvis', field='3C84')
          Out[35]:
        ['/home/casa-dev-07/hye/ptest/csplit/10B-209a_5s-f1-s8.ms',
         '/home/casa-dev-10/hye/ptest/csplit/10B-209a_5s-f1-s14.ms',
         '/home/casa-dev-08/hye/ptest/csplit/10B-209a_5s-f0-s0.ms']

        """
        
        casalog.origin("simple_cluster")
        
        if not self._configdone:
            return
        if type(result)==str:
            result=self.get_result(result)
        if type(result)!=list:
            return []
        if len(result)==0:
            return []
        if type(item)!=str:
            casalog.post("The item name must be a string","WARN","get_output")
            return []
    
        vals=[]
        for key in kwargs:
            v=kwargs[key]
            if type(v)==str:
                vals.append(str(key)+'="'+v+'"')
            elif type(v)==np.ndarray:
                vals.append(str(key)+'='+repr(v))
            else:
                vals.append(str(key)+'='+str(v))
    
        vec=[]
        for i in result:
            pick=True
            for j in vals:
                if str.count(i, j)==0:
                    pick=False
            if pick:
                a=str.find(i, item)
                a=str.find(i, '=', a)
                b=str.find(i, ',', a+1)
                if a>=0 and b>=0:
                    vec.append(i[a+2:b-1])
        return vec

    def getVariables(self, varList, engine):
        """
        This method will return a list corresponding to all variables
        in the varList for the specified engine. This is a
        very thin wrapper around the pull method in the cluster.
        """
        if not isinstance(varList, list):
            varList = [varList]

        rtnVal = []
        for var in varList:
            pulled = self._cluster.pull(var, engine)
            rtnVal.append(pulled[engine])

        return rtnVal
        
    
    ###########################################################################
    ###   engine selection functions
    ###########################################################################

    def use_paths(self, dir_list=[]):
        """use engines that most close to the dirs (or ms)

        Keyword arguments:
        dir_list -- the result label

        Example:
        CASA <33>: sl.list_result
        Out[33]:
        ['#### new split ####################### Mon Mar 14 14:48:08 2011 ####',
        CASA <34>: sl.get_output('new split', 'outputvis', field='3C84')
          Out[34]:
        ['/home/casa-dev-07/hye/ptest/csplit/10B-209a_5s-f1-s8.ms',
         '/home/casa-dev-07/hye/ptest/csplit/10B-209a_5s-f0-s1.ms',
         '/home/casa-dev-07/hye/ptest/csplit/10B-209a_5s-f1-s9.ms']
        CASA <35>: sl.use_paths(
                     '/home/casa-dev-10/hye/ptest/csplit/10B-209a_5s-f1-s14.ms')
          Out[35]: [8]

        """
        
        casalog.origin("simple_cluster")
        
        if not self._configdone:
            return
        if len(dir_list)==0:
            casalog.post("dir_list can not be empty","WARN","use_paths")
            return []
        a=[]
        if type(dir_list)!=list:
            a.append(dir_list)
            dir_list=a
        if len(dir_list)>0:
            int_ok=True
            for i in dir_list:
                if type(i)!=str:
                    int_ok=False
            if not int_ok:
                casalog.post("path name in dir_list must be string","WARN","use_paths")
                return []
    
        a=[]
        hst=self.get_hosts()
        for i in dir_list:
            int_ok=False
            for j in range(len(hst)):
                if i.count(hst[j][2])>0:
                    a.append(hst[j][0])
                    int_ok=True
            if not int_ok:
                casalog.post("Could not find a host for %s" % str(i),"WARN","use_paths")
                return []
        
        e=dict()
        for k in xrange(len(hst)):
            h=hst[k][0]
            e[h]=[]
            for i in self._cluster.get_engines():
                if i[1]==h: 
                    e[h].append(i[0])
        val=e.values()
        key=e.keys()
        lenth=[]
        pos=[]
        for i in xrange(len(val)):
            lenth.append(len(val[i]))
            pos.append(0)
        vec=[]
        for k in a:
            for s in xrange(len(e)):
                if k==key[s]:
                    if pos[s]==lenth[s]:
                       pos[s]=0
                    vec.append(val[s][pos[s]])
                    pos[s]+=1
        return vec

    def use_hosts(self, host_list=[], engines_each=0):
        """use engines on the given nodes

        Keyword arguments:
        host_list -- the list of hosts
        engines_each -- number of engines to use on each host

        Example:
        CASA <45>: sl.get_hosts
          Out[45]:
                  [['casa-dev-07', 4, '/home/casa-dev-07/hye/ptest'],
                   ['casa-dev-08', 4, '/home/casa-dev-08/hye/ptest'],
                   ['casa-dev-10', 4, '/home/casa-dev-10/hye/ptest']]
        CASA <46>: sl.use_hosts(['casa-dev-07', 'casa-dev-10'], 2)
          Out[46]: [8, 9, 0, 1]

        """
        
        casalog.origin("simple_cluster")

        if not self._configdone:
            return
        if len(host_list)==0 and  engines_each==0:
            return self._cluster.get_ids()

        hst=self.get_hosts()
        a=[]
        if type(host_list)!=list:
            a.append(host_list)
            host_list=a
        if len(host_list)>0:
            int_ok=True
            for i in host_list:
                if type(i)!=str:
                    int_ok=False
            if not int_ok:
                casalog.post("host name in host_list must be string","WARN","use_hosts")
                return []
        else:
            host_list=[]
            for j in range(len(hst)):
                host_list.append(hst[j][0])

        for i in host_list:
            host_ok=False
            for j in range(len(hst)):
                if hst[j][0]==i:
                    host_ok=True
            if not host_ok:
                casalog.post("There is no host with name %s" % str(i),"WARN","use_hosts")
                return []

       
        e=dict()
        for k in host_list:
            e[k]=[]
            for i in self._cluster.get_engines():
                if i[1]==k: 
                    e[k].append(i[0])
        val=e.values()
        vec=[]
        if engines_each==0:
            engines_each=100
        for i in xrange(len(val)):
            j=0
            while j<min(engines_each, len(val[i])):
                vec.append(val[i][j])
                j+=1
        return vec

    def use_engines(self, use_id=[], spreadhost=1):
        """use engines on from a given list

        Keyword arguments:
        use_id -- the list of engine ids
        spreadhost -- whether to apply host first policy

        Example:
        CASA <52>: sl._cluster.get_ids()
          Out[52]: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
        CASA <54>: sl.use_engines([0, 1, 2, 9])
          Out[54]: [0, 1, 2, 9]
        CASA <55>: sl.use_engines()
          Out[55]: [4, 8, 0, 5, 9, 1, 6, 10, 2, 7, 11, 3]

        """
        
        casalog.origin("simple_cluster")
        
        if not self._configdone:
            return
        if len(use_id)>0:
            int_ok=True
            for i in use_id:
                if type(i)!=int:
                    int_ok=False
            if int_ok:
                return use_id
            else:
                casalog.post("Engine id in use_id must be integer","WARN","use_engines")
                return []
        elif spreadhost==0:
            return self._cluster.get_ids()
        else:
            e=dict()
            hst=self.get_hosts()
            for i in range(len(hst)):
                e[hst[i][0]]=[]
            for i in self._cluster.get_engines():
                e[i[1]].append(i[0])
            val=e.values()
            lenth=[]
            pos=[]
            for i in xrange(len(val)):
                lenth.append(len(val[i]))
                pos.append(0)
            vec=[]
            while len(vec)<len(self._cluster.get_ids()):
                for i in xrange(len(val)):
                   if pos[i]<lenth[i]:
                       vec.append(val[i][pos[i]])
                       pos[i]+=1
            return vec
    
    ###########################################################################
    ###   ms knowledge functions
    ###########################################################################
    def get_msname(self, vis):
        """get the ms name of given vis

        Keyword arguments:
        vis -- the path+name of visibility data

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <16>: sl.get_msname(vis)
          Out[18]: '10B-209a_5s'

        """

        vs=os.path.abspath(vis)
        msname=vs[str.rfind(vs,'/')+1:]
        if msname.endswith('.ms'):
            msname=msname[:str.rfind(msname, '.ms')]
        return msname
    
    def get_antenna_diam(self, vis):
        """get the diameter of antennas

        Keyword arguments:
        vis -- the path+name of visibility data

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <19>: sl.get_antenna_diam(vis)
          Out[19]: 25.0

        """

        tb.open(vis+'/ANTENNA')
        diams=tb.getcol('DISH_DIAMETER')
        diam=np.min(diams)
        if diam==0:
            diam=np.max(diams)
        tb.done()
        return diam
    
    def get_mean_reff(self, vis):
        """get the mean reference frequency

        Keyword arguments:
        vis -- the path+name of visibility data

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <20>: sl.get_mean_reff(vis)
          Out[20]: 6298222222.2222223

        """

        tb.open(vis+'/SPECTRAL_WINDOW')
        reff=tb.getcol('REF_FREQUENCY')
        tb.done()
        return reff.mean()
    
    def get_spw_reff(self, vis, spw=0):
        """get the reference frequency of spw

        Keyword arguments:
        vis -- the path+name of visibility data
        spw -- the spectral window id

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <21>: sl.get_spw_reff(vis, 8)
          Out[21]: 5056000000.0

        """
        
        casalog.origin("simple_cluster")

        tb.open(vis+'/SPECTRAL_WINDOW')
        if spw<0 or spw>=tb.nrows():
            casalog.post("Spectral window not available: %s" % str(spw),"WARN","get_spw_reff")
            return
        spw_reff=tb.getcell('REF_FREQUENCY', spw)
        tb.done()
        return spw_reff
    
    def get_spw_chan(self, vis, spw=0):
        """get the number of channels of spw

        Keyword arguments:
        vis -- the path+name of visibility data
        spw -- the spectral window id

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <24>: sl.get_spw_chan(vis, 8)
          Out[24]: 64

        """
        
        casalog.origin("simple_cluster")
        
        tb.open(vis+'/SPECTRAL_WINDOW')
        if spw<0 or spw>=tb.nrows():
            casalog.post("Spectral window not available: %s" % str(spw),"WARN","get_spw_chan")
            return
        spw_chan=tb.getcell('NUM_CHAN', spw)
        tb.done()
        return spw_chan
    
    def get_pol_corr(self, vis, pol=0):
        """get the number of coorelation of polarization 

        Keyword arguments:
        vis -- the path+name of visibility data
        pol -- the polarization id

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <31>: sl.get_pol_corr(vis, 0)
          Out[31]: 4

        """
        
        casalog.origin("simple_cluster")
        
        tb.open(vis+'/POLARIZATION')
        if pol<0 or pol>=tb.nrows():
            casalog.post("Polarization not available: %s" % str(pol),"WARN","get_pol_corr")
            return
        pol_corr=tb.getcell('NUM_CORR', pol)
        tb.done()
        return pol_corr
    
    def get_num_field(self, vis):
        """get the number of fields 

        Keyword arguments:
        vis -- the path+name of visibility data

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <32>: sl.get_num_field(vis)
          Out[32]: 6L

        """

        tb.open(vis+'/FIELD')
        num_field=tb.nrows()
        tb.done()
        return num_field
    
    def get_field_name(self, vis, id):
        """get the name of a field 

        Keyword arguments:
        vis -- the path+name of visibility data
        id -- the field id

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <35>: sl.get_field_name(vis, 5)
          Out[35]: 'J0738+1742'

        """
        
        casalog.origin("simple_cluster")
        
        tb.open(vis+'/FIELD')
        if id<0 or id>=tb.nrows():
            casalog.post("Field not available: %s" % str(id),"WARN","get_field_name") 
            return
        fn=tb.getcell('NAME', id)
        tb.done()
        return fn
    
    def get_num_spw(self, vis):
        """get the number of spectral windows 

        Keyword arguments:
        vis -- the path+name of visibility data

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <36>: sl.get_num_spw(vis)
          Out[36]: 18L

        """

        tb.open(vis+'/SPECTRAL_WINDOW')
        num_spw=tb.nrows()
        tb.done()
        return num_spw
    
    def get_num_desc(self, vis):
        """get number of data descriptions

        Keyword arguments:
        vis -- the path+name of visibility data

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <37>: sl.get_num_desc(vis)
          Out[37]: 18L

        """

        tb.open(vis+'/DATA_DESCRIPTION')
        num_desc=tb.nrows()
        tb.done()
        return num_desc
    
    def get_spw_id(self, vis, desc=0):
        """get spectral window id for desc

        Keyword arguments:
        vis -- the path+name of visibility data
        desc -- the data description id

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <38>: sl.get_spw_id(vis, 17)
          Out[38]: 17

        """
        
        casalog.origin("simple_cluster")

        tb.open(vis+'/DATA_DESCRIPTION')
        if desc<0 or desc>=tb.nrows():
            casalog.post("DDI not available: %s" % str(desc),"WARN","get_spw_id") 
            return
        spw_id=tb.getcell('SPECTRAL_WINDOW_ID', desc)
        tb.done()
        return spw_id
    
    def get_pol_id(self, vis, desc=0):
        """get polarization id for desc

        Keyword arguments:
        vis -- the path+name of visibility data
        desc -- the data description id

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <39>: sl.get_pol_id(vis, 17)
          Out[39]: 0

        """
        
        casalog.origin("simple_cluster")

        tb.open(vis+'/DATA_DESCRIPTION')
        if desc<0 or desc>=tb.nrows():
            casalog.post("DDI not available: %s" % str(desc),"WARN","get_pol_id") 
            return
        pol_id=tb.getcell('POLARIZATION_ID', desc)
        tb.done()
        return pol_id
    
    def get_field_desc(self, vis):
        """get source

        Keyword arguments:
        vis -- the path+name of visibility data

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <40>: sl.get_field_desc(vis)
          Out[40]:
        {(0, 0): {'cost': [0, 82],
                  'desc': 0,
                  'field': 0,
                  'nchan': 64,
                  'ncorr': 4,
                  'nrows': 16848,
                  'pol': 0,
                  'spw': 0},
         (0, 1): {'cost': [0, 82],
                  'desc': 1,
                  'field': 0,
                  'nchan': 64,
                  'ncorr': 4,
                  'nrows': 16848,
                  'pol': 0,
                  'spw': 1}
         ... ommit ...
        }

        """

        tb.open(vis)
        nrows=tb.nrows() 
        tb.done()
        k=nrows/1000
        t=nrows%1000
        if t>0:
           k+=1
        b={}
        tb.open(vis)
        for i in xrange(k):
            field=tb.getcol('FIELD_ID', i*1000, 1000)
            desc=tb.getcol('DATA_DESC_ID', i*1000, 1000)
            fd=zip(field, desc)
            newset=set(fd)
            for j in newset:
                if b.has_key(j):
                   b[j]['nrows']=b[j]['nrows']+fd.count(j)
                else:
                   a={}
                   a['field']=j[0]
                   a['desc']=j[1]
                   a['nrows']=fd.count(j)
                   b[j]=a
        tb.done()
        d=0
        for i in b.keys():
            b[i]['pol']=self.get_pol_id(vis, b[i]['desc'])
            b[i]['spw']=self.get_spw_id(vis, b[i]['desc'])
            b[i]['ncorr']=self.get_pol_corr(vis, b[i]['pol'])
            b[i]['nchan']=self.get_spw_chan(vis, b[i]['spw'])
            b[i]['cost']=b[i]['nchan']*b[i]['ncorr']*b[i]['nrows']
            d+=b[i]['cost']
        d/=len(b.keys()) 
        for i in b.keys():
            b[i]['cost']=[b[i]['cost']/d, len(b.keys())]
        return b
    
    
    ###########################################################################
    ###   setup -  
    ###########################################################################
    def init_cluster(self, clusterfile='', project=''):
        """Setup the cluster

        Keyword arguments:
        clusterfile -- the cluster definition file
        project -- the name of project (default: 'proj'+timestamp).  

        A configuration file is an ASCII text file. Each line defines a node
        (also called host) with one line per host to be used, and the following 
        format: 
        
        - <hostname>, <number of engines>, <work directory>
        - <hostname>, <number of engines>, <work directory>, <fraction of total RAM>
        - <hostname>, <number of engines>, <work directory>, <fraction of total RAM>, <RAM per engine>
        
        where the interpretation of the parameters is as follows: 
        
        - hostname: Hostname of the target node where the cluster is deployed 
        
          NOTE: The hostname has to be provided w/o quotes
        
        - number of engines: Supports in turns 3 different formats 
        
            * If provided as an integer >1: It is interpreted as 
              the actual user-specified maximum number of engines
              
            * If provided as an integer =0: It will deploy as maximum 
              engines as possible, according to the idle CPU capacity 
              available at the target node
              
            * If provided as a float between 0 and 1: It is interpreted 
              as the percentage of idle CPU capacity that the cluster 
              can use in total at the target node

        - work directory: Area in which the cluster will put intermediate 
          files such as log files, configuration files, and monitoring files
        
          NOTE1: This area has to be accessible from the controller (user) machine, 
                 and mounted in the same path of the filesystem 
                 
          NOTE2: The path name has to be provided w/o quotes
        
        - fraction of total RAM: Supports in turns 3 different formats:
        
            * If provided as an integer >1: It is interpreted as the actual 
              user-specified maximum amount of RAM to be used in total at 
              the target node
              
            * If provided as an integer =0: It will deploy as maximum engines 
              as possible, according to the free RAM available at target node
              
            * If provided as a float between 0 and 1: It is interpreted as 
              the percentage of free RAM that the cluster can use in total 
              at the target node
            
        - RAM per engine: Integer, which is interpreted as the required memory 
          per engine in MB (default is 512MB) 
          
        It is also possible to add comments, by using the # character at the 
        beginning of the line. Example:
        
        #####################################################
        
        # CASA cluster configuration file for expert user
        orion, 10, /home/jdoe/test/myclusterhome1
        m42, 4, /home/jdoe/test/myclusterhome2, 0.6, 1024
        antares, 0.6, /home/jdoe/test/myclusterhome3, 0, 2048
        
        #####################################################
        
        - At host ``orion'': It will deploy up to 10 engines, with working 
          directory /home/jdoe/test/myclusterhome1, and using as much free 
          RAM available as possible (up to 90% by default), taking into 
          account that each engine can use up to 512 MB (the default and minimum)
          
        - At host ``m42'': It will deploy up to 4 engines, with working directory 
          /home/jdoe/test/myclusterhome2, and using at the most 60% of the free RAM 
          available, taking into account that each engine can use up to 1024 MB.   
          
        - At host ``antares'': It will deploy as many engines as possible, with 
          working directory /home/jdoe/test/myclusterhome3, using up to 60% of the 
          idle CPU capacity / cores, and as much free RAM available as possible 
          (up to 90% by default), taking into account that each engine can use up 
          to 2048 MB.  

        Example:
        CASA <15>: from simple_cluster import *
        CASA <16>: sl=simple_cluster()
        CASA <17>: sl.init_cluster('cluster-config.txt', 'ProjectName')

        """
        
        casalog.origin("simple_cluster")

        if project==None or type(project)!=str or project.strip()=="":
            # Project name must be a non-empty string, otherwise set default
            project='cluster_project'
            casalog.post("No project specified using default project: " +\
                         "cluster_project", "WARN","init_cluster")
    
        if clusterfile==None or type(clusterfile)!=str or clusterfile.strip()=="":
            # Cluster file name must be a non-empty string, otherwise generate a default clusterfile
            # The default cluster should have:
            #    * One engine for each core on the current system
            #    * The working directory should be the cwd
            (sysname, nodename, release, version, machine)=os.uname()
            msg=nodename+', '+str(0)+', '+os.getcwd()
            # jagonzal (CAS-4293): Write default cluster config file in the 
            # current directory to avoid problems with writing permissions
            clusterfile='default_cluster'
            f=open(clusterfile, 'w')
            f.write(msg)
            f.close()
            self.__localCluster = True
    
        self.config_cluster(clusterfile, True)
        if not self._configdone:
            self.__running = False
            return False
        
        self.create_project(project)
        
        self.start_cluster()

        # Put the cluster object into the global namespace
        sys._getframe(len(inspect.stack())-1).f_globals['procCluster'] = self
        
        # Set running status
        self.__running = True
        
        return True
        
    
    ###########################################################################
    ###   example to distribute clean task over engines
    ###########################################################################
    def simple_clean(self, vs, nx, ny, mode='channel', email=''):
        """Make images with a simple cluster

        Keyword arguments:
        vs -- the visibility data
        nx, ny -- the size (pixels) of the image
        mode -- either 'channel' or 'continuum'
        email -- the email address to notify the completion

        Example:
        CASA <15>: from simple_cluster import *
        CASA <16>: sl=simple_cluster()
        CASA <17>: sl.init_cluster('my_cluster', 'aProj')
        CASA <18>: simple_clean(
            vis='/home/casa-dev-09/hye/ptest/sim.alma.csv.mid.ms', 
            nx=256, ny=256, mode='channel')

        """
        
        vis=os.path.abspath(vs)
        tb.clearlocks(vis)
    
        # Determine the cell size
        diam=self.get_antenna_diam(vis)
        freqmean=self.get_mean_reff(vis)
        casalog.post("vis: %s Diameter: %s FreqMean: %s" % (vis,str(diam),str(freqmean)),"INFO","simple_clean")
        fv=(3.0e8/freqmean/diam)*180*60*60/math.pi
        cell=[str(fv/nx)+'arcsec', str(fv/ny)+'arcsec']
    
        fdspw=self.get_field_desc(vis)
        ids=self._cluster.get_ids()
        msname=self.get_msname(vis)
    
        if len(fdspw)>len(ids):
            # More job chunks than engines, simply distribute by field and spw
            i=0
            for k in fdspw.values():
                id_i=ids[i]
                s={}
                s['vis']=vis
                fd=str(k['field'])
                spw=str(k['spw'])
                s['imagename']=self.get_engine_store(id_i)+msname+'-f'+fd+'-s'+spw
                s['field']=fd
                s['spw']=spw
                s['mode']=mode
                s['niter']=20000
                s['threshold']='0.001mJy'
                s['psfmode']='hogbom'
                s['imagermode']='csclean'
                s['imsize']=[nx, ny]
                s['cell']=cell
                s['calready']=False
                cmd=self.make_call('clean', s) 
                self.do_and_record(cmd, id_i, email)
                i+=1
                if i==len(ids):
                   i=0
        else:
            # Less job chanks than engines, need further devide by channel
            i=0
            for k in fdspw.values():
                spwchan=self.get_spw_chan(vis, k['spw'])
        
                nengs=len(ids)
                nchan=1
                try:
                    nchan=int(ceil(abs(float(spwchan))/nengs))
                except:
                    # jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
                    # traceback.print_tb(sys.exc_info()[2])
                    pass
        
                for j in xrange(len(ids)):
                    id_i=ids[i]
                    start=j*nchan
                    s={}
                    s['vis']=vis
                    fd=str(k['field'])
                    spw=str(k['spw'])
                    s['imagename']=(self.get_engine_store(id_i)+msname+'-f'+fd+
                             '-s'+spw+'-b'+str(start)+'-e'+str(start+nchan))
                    s['field']=fd
                    s['spw']=spw
                    s['mode']='channel'
                    s['niter']=20000
                    s['start']=start
                    s['nchan']=nchan
                    s['threshold']='0.001mJy'
                    s['psfmode']='hogbom'
                    s['imagermode']='csclean'
                    s['imsize']=[nx, ny]
                    s['cell']=cell
                    s['calready']=False
                    cmd=self.make_call('clean', s) 
                    self.do_and_record(cmd, id_i, email)
                    i+=1
                    if i==len(ids):
                       i=0
        
        self.get_status()
    
    def simple_split(self, vs, email):
        """split by source (field, spw) with parallel engines

        Keyword arguments:
        vs -- the visibility data
        email -- the email address to notify the completion

        Example:
        CASA <15>: from simple_cluster import *
        CASA <16>: sl=simple_cluster()
        CASA <17>: sl.init_cluster('my_cluster', 'aProj')
        CASA <18): vis='/home/casa-dev-09/hye/ptest/sim.alma.csv.mid.ms', 
        CASA <18>: simple_split(vis)

        """
        vis=os.path.abspath(vs)
        tb.clearlocks(vis)
       
        casalog.post("vis: %s" % vis,"INFO","simple_split")
        fdspw=self.get_field_desc(vis)
        ids=self.use_engines()
        msname=self.get_msname(vis)
    
        i=0
        for k in fdspw.values():
            id_i=ids[i]
            s={}
            s['vis']=vis
            fd=str(k['field'])
            spw=str(k['spw'])
            s['outputvis']=self.get_engine_store(id_i)+msname+'-f'+ \
                               fd+'-s'+spw+'.ms'
            if os.path.exists(s['outputvis']):
                os.system('rm -rf '+s['outputvis'])
            s['field']=self.get_field_name(vis, k['field']) 
            s['spw']=spw
            s['datacolumn']='DATA'
            cmd=self.make_call('split', s) 
            self.do_and_record(cmd, id_i, email)
            i+=1
            if i==len(ids):
               i=0
        self.get_status()
    
#if __name__ == "__main__":
    #from simple_cluster import *
    #sl=simple_cluster()
    #sl.init_cluster('my_cluster', 'aProj')
    #vis='/home/casa-dev-09/hye/ptest/sim.alma.csv.mid.ms', 
    #simple_split(vis)

    #get_status()

    #init_cluster('my_cluster', 'my_project')
    #simple_clean(vis='/home/casa-dev-09/hye/ptest/sim.alma.csv.mid.ms', 
    #             nx=256, ny=256, mode='channel')
  
    #simple_clean(vis='/home/casa-dev-09/hye/para/sim.alma.csv.mid.ms', 
    #             nx=256, ny=256, mode='channel')
      
    #simple_clean(vis='/home/casa-dev-09/hye/pclean/sim100g_4chan15kRows.ms',
    #             nx=256, ny=256, mode='channel')
        


###########################################################################
###   job management classes
###########################################################################

class JobData:
    """
    This class incapsulates a single job.  The commandName is the name
    of the task to be executed.  The jobInfo is a dictionary of all
    parameters that need to be handled.
    """
    class CommandInfo:

        def __init__(self, commandName, commandInfo, returnVariable):
            self.commandName = commandName
            self.commandInfo = commandInfo
            self.returnVariable = returnVariable

        def getReturnVariable(self):
            return self.returnVariable
        
        def getCommandLine(self):
            firstArgument = True
            output = "%s = %s(" % (self.returnVariable, self.commandName)
            for (arg,value) in self.commandInfo.items():
                if firstArgument:
                    firstArgument = False
                else:
                    output += ', '
                if isinstance(value, str):
                    output += ("%s = '%s'" % (arg, value))
                else:
                    output += ("%s = " % arg) + str(value)
            output += ')'
            return output
    
    
    def __init__(self, commandName, commandInfo = {}):
        self._commandList = []
        self.status  = 'new'
        self.addCommand(commandName, commandInfo)
        self._returnValues = None
            

    def addCommand(self, commandName, commandInfo):
        """
        Add an additional command to this Job to be exectued after
        previous Jobs.
        """
        rtnVar = "returnVar%d" % len(self._commandList)
        self._commandList.append(JobData.CommandInfo(commandName,
                                                     commandInfo,
                                                     rtnVar))
    def getCommandLine(self):
        """
        This method will return the command line(s) to be executed on the
        remote engine.  It is usually only needed for debugging or for
        the JobQueueManager.
        """
        output = ''
        for idx in xrange(len(self._commandList)):
            if idx > 0:
                output += '; '
            output += self._commandList[idx].getCommandLine()
        return output

    def getCommandNames(self):
        """
        This method will return a list of command names that are associated
        with this job.
        """
        return [command.commandName for command in self._commandList]
    

    def getCommandArguments(self, commandName = None):
        """
        This method will return the command arguments associated with a
        particular job.
           * If commandName is not none the arguments for the command with
             that name are returned.
           * Otherwise a dictionary (with keys being the commandName and
             the value being the dictionary of arguments) is returned.
           * If there is only a single command the arguments for that
             command are returned as a dictionary.
        """
        returnValue = {}
        for command in self._commandList:
            if commandName is None or commandName == command.commandName:
                returnValue[command.commandName] = command.commandInfo
                                                   
        if len(returnValue) == 1:
            return returnValue.values()[0]
        return returnValue
    
    def getReturnVariableList(self):
        return [ci.returnVariable for ci in self._commandList]

    def setReturnValues(self, valueList):
        self._returnValues = valueList

    def getReturnValues(self):
        if self._returnValues is not None:
            if len(self._returnValues) == 1:
                return self._returnValues[0]
        return self._returnValues

class JobQueueManager:
    def __init__(self, cluster = None):
        self.__cluster=cluster
        if self.__cluster is None:
            self.__cluster = simple_cluster.getCluster()
        # jagonzal: Create a cross reference for the simple_cluster using this JobQueue
        self.__cluster._JobQueueManager = self
        self.__inputQueue = []
        self.__outputQueue = {}

    def addJob(self, jobData):
        """
        Add another JobData object to the queue of jobs to be executed.
        """
        # TODO Check the type of jobData
        if not isinstance(jobData,list):
            jobData = [jobData]
        for job in jobData:
            job.status='pending'
            self.__inputQueue.append(job)

    def clearJobs(self):
        """
        Remove all jobs from the queue, this is usually a good idea
        before reusing a JobQueueManager.
        """
        self.__inputQueue = []
        self.__outputQueue = {}
        
    def getOutputJobs(self, status = None):
        """
        This returns all jobs in the output queue which
        match the specified status.  If no status is specified
        the entire list of output jobs is returned.
        """
        if status is None:
            return self.__outputQueue.values()

        returnList = []
        for job in self.__outputQueue.values():
            if job.status == status:
                returnList.append(job)
        return returnList
        
    def getAllJobs(self):
        return self.__outputQueue.values()

    def executeQueue(self):
        """
        This method causes all jobs to be executed on the available engines
        It will block until the jobs are complete.
        """
        
        casalog.origin("simple_cluster")
        
        # Clear the queue of any existing results
        self.__cluster.remove_record()
        
        engineList = self.__cluster.use_engines()

        # jagonzal (CAS-): When there are 0 engines available an error must have happened
        if (len(engineList) < 1):
            casalog.post("There are 0 engines available, check the status of the cluster","WARN","executeQueue")
            return

        casalog.post("Executing %d jobs on %d engines" %
                     (len(self.__inputQueue), len(engineList)), "INFO","executeQueue")

        while len(self.__inputQueue) > 0:
            self._checkForCompletedJobs(engineList)

            for job in self.__inputQueue[:len(engineList)]:
                self.__inputQueue.remove(job)
                # This stores each job in the output queue indexed by it's JobID (name)
                self.__outputQueue[self.__cluster.do_and_record\
                                   (job.getCommandLine(), 
                                    engineList.pop(),
                                    subMS=job.getCommandArguments()['vis'].split('/').pop())
                                   ]=job

            # Wait for a bit then try again
            time.sleep(1)

        # Now we need to wait for the rest of the jobs to complete
        while self._checkForCompletedJobs(engineList):
            time.sleep(1)


    def _checkForCompletedJobs(self, engineList):
        """ This method will look at all jobs in the status, if they are
        marked as done it will:
           * update the outputQueue
           * add the engine back to the engine list
           * remove the report from the status
        """
        statusReport = self.__cluster.get_status(True).values()
        if len(statusReport) == 0:
            # Nothing running
            return False

        # jagonzal: This seems to be a hook between the cluster queue ang the JobManager queue
        for job in self.__cluster.get_status(True).values():
            # Update the status of the Job
            self.__outputQueue[job['jobname']].status = job['status']
            
            if job['status'] == 'done' or job['status'] == 'broken':
                if job['status'] == 'done':
                    # Get the return values is we're successful
                    self.__outputQueue[job['jobname']].setReturnValues \
                       (self.__cluster.getVariables\
                        (self.__outputQueue[job['jobname']].\
                         getReturnVariableList(),job['engine']))
                    
                engineList.append(job['engine'])
                self.__cluster.remove_record(job['jobname'])
        return True
