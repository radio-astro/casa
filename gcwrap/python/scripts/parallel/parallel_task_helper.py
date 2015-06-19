#!/usr/bin/env python
from taskinit import *
import os
import copy
import shutil
import simple_cluster
import partitionhelper as ph

# To handle thread-based Tier-2 parallelization
import thread 
import threading

# jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
import traceback

# jagonzal (Migration to MPI)
from mpi4casa.MPIEnvironment import MPIEnvironment
from mpi4casa.MPICommandClient import MPICommandClient

class ParallelTaskHelper:
    """
    This is the extension of the TaskHelper to allow for parallel
    operation.  For simple tasks all that should be required to make
    a task parallel is to use this rather than the TaskHelper method
    above
    """
    
    __bypass_parallel_processing = 0
    __async_mode = False
    __multithreading = False    
    
    def __init__(self, task_name, args = {}):
        self._arg = dict(args)
        self._arguser = {}
        self._taskName = task_name
        self._executionList = []
        self._jobQueue = None
        # Cache the initial inputs
        self.__originalParams = args
        # jagonzal: Add reference to cluster object
        self._cluster = None
        self._mpi_cluster = False
        self._command_request_id_list = None
        if (self.__bypass_parallel_processing == 0):
            # jagonzal (Migration to MPI)
            if MPIEnvironment.is_mpi_enabled and MPIEnvironment.is_mpi_client:
                self._mpi_cluster = True
                self._command_request_id_list = []
                self._cluster = MPICommandClient()
            else:
                self._cluster = simple_cluster.simple_cluster.getCluster()
        # jagonzal: To inhibit return values consolidation
        self._consolidateOutput = True
        # jagonzal (CAS-4287): Add a cluster-less mode to by-pass parallel processing for MMSs as requested 
        self._sequential_return_list = {}
        
    def override_arg(self,arg,value):
        self._arguser[arg] = value

    def initialize(self):
        """
        This is the setup portion.
        Currently it:
           * Finds the full path for the input vis.
           * Initialize the MPICommandClient
        """
        self._arg['vis'] = os.path.abspath(self._arg['vis'])
        
        # jagonzal (Migration to MPI)
        if self._mpi_cluster:
            self._cluster.start_services()
            
    def getNumberOfServers(self):
        """
        Return the number of engines (iPython cluster) or the number of servers (MPI cluster)
        """
        if (self.__bypass_parallel_processing == 0):
            if self._mpi_cluster:
                return len(MPIEnvironment.mpi_server_rank_list())
            else:
                return len(self._cluster._cluster.get_engines())
        else:
            return None

    def generateJobs(self):
        """
        This is the method which generates all of the actual jobs to be
        done.  The default is to asume the input vis is a reference ms and
        build one job for each referenced ms.
        """
        
        casalog.origin("ParallelTaskHelper")
        
        try:
            msTool = mstool()
            if not msTool.open(self._arg['vis']):
                raise ValueError, "Unable to open MS %s," % self._arg['vis']
            if not msTool.ismultims():
                raise ValueError, \
                      "MS is not a MultiMS, simple parallelization failed"

            subMs_idx = 0
            for subMS in msTool.getreferencedtables():
                localArgs = copy.deepcopy(self._arg)
                localArgs['vis'] = subMS
                
                for key in self._arguser:
                    localArgs[key] = self._arguser[key][subMs_idx]
                subMs_idx += 1
                
                if not self._mpi_cluster:
                    self._executionList.append(simple_cluster.JobData(self._taskName,localArgs))
                else:
                    self._executionList.append([self._taskName + '()',localArgs])
                
            msTool.close()
            return True
        except Exception, instance:
            casalog.post("Error handling MMS %s: %s" % (self._arg['vis'],instance),"WARN","generateJobs")
            msTool.close()
            return False


    def executeJobs(self):
        
        casalog.origin("ParallelTaskHelper")
        
        # jagonzal (CAS-4287): Add a cluster-less mode to by-pass parallel processing for MMSs as requested 
        if (self.__bypass_parallel_processing == 1):
            for job in self._executionList:
                parameters = job.getCommandArguments()
                try:
                    exec("from taskinit import *; from tasks import *; " + job.getCommandLine())
                    # jagonzal: Special case for partition
                    if (parameters.has_key('outputvis')):
                        self._sequential_return_list[parameters['outputvis']] = returnVar0
                    else:
                        self._sequential_return_list[parameters['vis']] = returnVar0
                except Exception, instance:
                    str_instance = str(instance)
                    if (string.find(str_instance,"NullSelection") == 0):
                        casalog.post("Error running task sequentially %s: %s" % (job.getCommandLine(),str_instance),"WARN","executeJobs")
                        traceback.print_tb(sys.exc_info()[2])
                    else:
                        casalog.post("Ignoring NullSelection error from %s" % (parameters['vis']),"INFO","executeJobs")
            self._executionList = []
        elif not self._mpi_cluster:
            self._jobQueue = simple_cluster.JobQueueManager(self._cluster)
            self._jobQueue.addJob(self._executionList)
            self._jobQueue.executeQueue()
        # jagonzal (Migration to MPI)
        else:
            for job in self._executionList:
                command_request_id = self._cluster.push_command_request(job[0],False,None,job[1])
                self._command_request_id_list.append(command_request_id[0])


    def postExecution(self):   
        
        casalog.origin("ParallelTaskHelper")
           
        ret_list = {}
        if (self.__bypass_parallel_processing==1):
            ret_list = self._sequential_return_list
            self._sequential_return_list = {}        
        elif (self._cluster != None):
            if self._mpi_cluster:
                # jagonzal (CAS-7631): Support for thread-based Tier-2 parallelization
                if ParallelTaskHelper.getMultithreadingMode():
                    event = self._cluster.get_command_response_event(self._command_request_id_list)
                    ParallelTaskWorker.releaseTaskLock()
                    event.wait()
                    ParallelTaskWorker.acquireTaskLock()
                # Get command response
                command_response_list =  self._cluster.get_command_response(self._command_request_id_list,True,True)
                # Format list in the form of vis dict
                ret_list = {}
                for command_response in command_response_list:
                    vis = command_response['parameters']['vis']
                    ret_list[vis] = command_response['ret']
            else:
                ret_list =  self._cluster.get_return_list()
        else:
            return None
        
        ret = ret_list
        if self._consolidateOutput: ret = ParallelTaskHelper.consolidateResults(ret_list,self._taskName)
        
        return ret
        
        
    @staticmethod
    def consolidateResults(ret_list,taskname):
        
        index = 0
        if isinstance(ret_list.values()[0],bool):
            retval = True
            for subMs in ret_list:
                if not ret_list[subMs]:
                    casalog.post("%s failed for sub-MS %s" % (taskname,subMs),"WARN","consolidateResults")
                    retval = False
                index += 1
            return retval
        elif any(isinstance(v,dict) for v in ret_list.itervalues()):
            ret_dict = {}
            for subMs in ret_list:
                dict_i = ret_list[subMs]
                if isinstance(dict_i,dict):
                    try:
                        ret_dict = ParallelTaskHelper.sum_dictionaries(dict_i,ret_dict)
                    except Exception, instance:
                        casalog.post("Error post processing MMS results %s: %s" % (subMs,instance),"WARN","consolidateResults")
            return ret_dict
        
        
    @staticmethod
    def sum_dictionaries(dict_list,ret_dict):
        for key in dict_list:
            item = dict_list[key]
            if isinstance(item,dict):
                if ret_dict.has_key(key):
                    ret_dict[key] = ParallelTaskHelper.sum_dictionaries(item,ret_dict[key])
                else:
                    ret_dict[key] = ParallelTaskHelper.sum_dictionaries(item,{})
            else:
                if ret_dict.has_key(key):
                    if not isinstance(ret_dict[key],str):
                        ret_dict[key] += item
                else:
                    ret_dict[key] = item
        return ret_dict   
    
    
    @staticmethod
    def getResult(command_request_id_list,taskname):
        
        # Access MPICommandClietn singleton instance
        client = MPICommandClient()
        
        # Get response list
        command_response_list =  client.get_command_response(command_request_id_list,True,True)
                
        # Format list in the form of vis dict
        ret_list = {}
        for command_response in command_response_list:
            vis = command_response['parameters']['vis']
            ret_list[vis] = command_response['ret']
            
        # Consolidate results and return
        ret = ParallelTaskHelper.consolidateResults(ret_list,taskname)
        
        return ret                    
            
            
    def go(self):
        
        casalog.origin("ParallelTaskHelper")
        
        self.initialize()
        if (self.generateJobs()):
            self.executeJobs()
            
            if ParallelTaskHelper.__async_mode:
                return list(self._command_request_id_list)
            else:
                try:
                    retVar = self.postExecution()
                except Exception, instance:
                    casalog.post("Error post processing MMS results %s: %s" % (self._arg['vis'],instance),"WARN","go")
                    traceback.print_tb(sys.exc_info()[2])
                    return False
        else:
            retVar = False
            
        # Restore casalog origin
        casalog.origin(self._taskName)
        
        return retVar

    @staticmethod
    def getReferencedMSs(vis):
        
        msTool = mstool()
        if not msTool.open(vis):
            raise ValueError, "Unable to open MS %s." % vis

        if not msTool.ismultims():
            raise ValueError, "MS %s is not a reference MS." % vis

        rtnValue = msTool.getreferencedtables()
        if not isinstance(rtnValue, list):
            rtnValue = [rtnValue]
      
        msTool.close()
        return rtnValue


    @staticmethod
    def restoreSubtableAgreement(vis, mastersubms='', subtables=[]):
        """
        Tidy up the MMS vis by replacing the subtables of all SubMSs
        by the subtables from the SubMS given by "mastersubms".
        If specified, only the subtables in the list "subtables"
        are replaced, otherwise all.
        If "mastersubms" is not given, the first SubMS of the MMS
        will be used as master.
        """

        msTool = mstool();
        msTool.open(vis)
        theSubMSs = msTool.getreferencedtables()
        msTool.close()

        tbTool = tbtool();
        
        if mastersubms=='':
            tbTool.open(vis)
            myKeyw = tbTool.getkeywords()
            tbTool.close()
            mastersubms=os.path.dirname(myKeyw['ANTENNA'].split(' ')[1]) #assume ANTENNA is present

        mastersubms = os.path.abspath(mastersubms)
            
        theSubTables = ph.getSubtables(mastersubms)

        if subtables==[]:
            subtables=theSubTables
        else:
            for s in subtables:
                if not (s in theSubTables):
                    raise ValueError, s+' is not a subtable of '+ mastersubms

        origpath = os.getcwd()      
        masterbase = os.path.basename(mastersubms)
        
        for r in theSubMSs:
            rbase = os.path.basename(r)
            if not rbase==masterbase:
                for s in subtables:
                    theSubTab = r+'/'+s
                    if os.path.islink(theSubTab): # don't copy over links
                        if(os.path.basename(os.path.dirname(os.path.realpath(theSubTab)))!=masterbase):
                            # the mastersubms has changed: make new link
                            os.chdir(r)
                            shutil.rmtree(s, ignore_errors=True)
                            os.symlink('../'+masterbase+'/'+s, s)
                            os.chdir(origpath)
                    else:    
                        shutil.rmtree(theSubTab, ignore_errors=True)
                        shutil.copytree(mastersubms+'/'+s, theSubTab)

        return True

    @staticmethod
    def bypassParallelProcessing(switch=1):
        """
        # jagonzal (CAS-4287): Add a cluster-less mode to by-pass parallel processing for MMSs as requested 
        switch=1 => Process each sub-Ms sequentially
        switch=2 => Process the MMS as a normal MS
        """        
        ParallelTaskHelper.__bypass_parallel_processing = switch
        
    @staticmethod
    def getBypassParallelProcessing():
        """
        # jagonzal (CAS-4287): Add a cluster-less mode to by-pass parallel processing for MMSs as requested 
        switch=1 => Process each sub-Ms sequentially
        switch=2 => Process the MMS as a normal MS
        """        
        return ParallelTaskHelper.__bypass_parallel_processing        
    
    @staticmethod
    def setAsyncMode(async=False):     
        ParallelTaskHelper.__async_mode = async
        
    @staticmethod
    def getAsyncMode():
        return ParallelTaskHelper.__async_mode    
    
    @staticmethod
    def setMultithreadingMode(multithreading=False):     
        ParallelTaskHelper.__multithreading = multithreading
        
    @staticmethod
    def getMultithreadingMode():
        return ParallelTaskHelper.__multithreading
    
    @staticmethod
    def isParallelMS(vis):
        """
        This method will let us know if we can do the simple form
        of parallelization by invoking on many referenced mss.
        """
        
        # jagonzal (CAS-4287): Add a cluster-less mode to by-pass parallel processing for MMSs as requested 
        if (ParallelTaskHelper.__bypass_parallel_processing == 2):
            return False
        
        msTool = mstool()
        if not msTool.open(vis):
            raise ValueError, "Unable to open MS %s," % vis
        rtnVal = msTool.ismultims() and \
                 isinstance(msTool.getreferencedtables(), list)

        msTool.close()
        return rtnVal
    
    @staticmethod
    def findAbsPath(input):
        if isinstance(input,str):
            return os.path.abspath(input)

        if isinstance(input, list):
            rtnValue = []
            for file_i in input:
                rtnValue.append(os.path.abspath(file_i))
            return rtnValue

        # Your on your own, don't know what to do
        return input

    @staticmethod
    def listToCasaString(inputList):
        """
        This Method will take a list of integers and try to express them as a 
        compact set using the CASA notation.
        """
        if inputList is None or len(inputList) == 0:
            return ''
        
        def selectionString(rangeStart, rangeEnd):
            if rangeStart == rangeEnd:
                return str(rangeStart)
            return "%d~%d" % (rangeStart, rangeEnd)
    
        inputList.sort()
        compactStrings = []
        rangeStart = inputList[0]
        lastValue = inputList[0]
        for val in inputList[1:]:
            if val > lastValue + 1:
                compactStrings.append(selectionString(rangeStart,lastValue))
                rangeStart = val
            lastValue = val
        compactStrings.append(selectionString(rangeStart,lastValue))

        return ','.join([a for a in compactStrings])
    

class ParallelTaskWorker:
    
    # Initialize task lock
    __task_lock = threading.Lock()
    
    def __init__(self, cmd):
        
        self.__cmd = compile(cmd,"ParallelTaskWorker", "eval")
        self.__state = "initialized"
        self.__res = None        
        self.__thread = None
        self.__environment = self.getEnvironment()
        self.__formatted_traceback = None        
        self.__completion_event = threading.Event()  

    def getEnvironment(self):
        
        stack=inspect.stack()
        for stack_level in range(len(stack)):
            frame_globals=sys._getframe(stack_level).f_globals
            if frame_globals.has_key('update_params'):
                return dict(frame_globals)
            
        raise Exception("CASA top level environment not found")
        
    def start(self):
        
        # Initialize completion event
        self.__completion_event.clear()        
               
        # Spawn thread
        self.__thread = thread.start_new_thread(self.runCmd, ())
        
        # Mark state as running
        self.__state = "running"        

    def runCmd(self):
        
        # Acquire lock
        ParallelTaskWorker.acquireTaskLock()
        
        # Update environment with globals from calling context
        globals().update(self.__environment)
        
        # Run compiled command
        try:
            self.__res = eval(self.__cmd)
            # Mark state as successful
            self.__state = "successful"
            # Release task lock
            ParallelTaskWorker.releaseTaskLock()            
        except Exception, instance:
            # Mark state as failed
            self.__state = "failed"
            # Release task lock if necessary
            if ParallelTaskWorker.checkTaskLock():ParallelTaskWorker.releaseTaskLock()
            # Post error message
            self.__formatted_traceback = traceback.format_exc()
            casalog.post("Exception executing command '%s': %s" 
                         % (self.__cmd,self.__formatted_traceback),
                         "SEVERE","ParallelTaskWorker::runCmd")
        
        # Send completion event signal
        self.__completion_event.set()
        
    def getResult(self):
        
        if self.__state == "running":
            # Wait until completion event signal is received
            self.__completion_event.wait()
            
            
        if self.__state == "initialized":
            casalog.post("Worker not started",
                         "WARN","ParallelTaskWorker::getResult")
        elif self.__state == "successful":
            return self.__res            
        elif self.__state == "failed":
            casalog.post("Exception executing command '%s': %s" 
                         % (self.__cmd,self.__formatted_traceback),
                         "SEVERE","ParallelTaskWorker::runCmd")                        
    
    @staticmethod
    def acquireTaskLock():
        
        ParallelTaskWorker.__task_lock.acquire()
        
    @staticmethod
    def releaseTaskLock():
        
        ParallelTaskWorker.__task_lock.release()
        
    @staticmethod
    def checkTaskLock():
        
        return ParallelTaskWorker.__task_lock.locked()        
          
          
