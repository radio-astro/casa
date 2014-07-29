#!/usr/bin/env python
from taskinit import *
import os
import copy
import shutil
import simple_cluster
import partitionhelper as ph

# jagonzal (CAS-4106): Properly report all the exceptions and errors in the cluster framework
import traceback

class ParallelTaskHelper:
    """
    This is the extension of the TaskHelper to allow for parallel
    operation.  For simple tasks all that should be required to make
    a task parallel is to use this rather than the TaskHelper method
    above
    """
    
    __bypass_parallel_processing = 0
    
    def __init__(self, task_name, args = {}):
        self._arg = args
        self._arguser = {}
        self._taskName = task_name
        self._executionList = []
        self._jobQueue = None
        # Cache the initial inputs
        self.__originalParams = args
        # jagonzal: Add reference to cluster object
        if (self.__bypass_parallel_processing == 0):
            self._cluster = simple_cluster.simple_cluster.getCluster()
        else:
            self._cluster = None
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
        """
        self._arg['vis'] = os.path.abspath(self._arg['vis'])

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
                localArgs = copy.copy(self._arg)
                localArgs['vis'] = subMS
                
                for key in self._arguser:
                    localArgs[key] = self._arguser[key][subMs_idx]
                subMs_idx += 1
                
                self._executionList.append(
                    simple_cluster.JobData(self._taskName,localArgs))
                
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
        else:
            self._jobQueue = simple_cluster.JobQueueManager(self._cluster)
            self._jobQueue.addJob(self._executionList)
            self._jobQueue.executeQueue()

    def postExecution(self):   
        
        casalog.origin("ParallelTaskHelper")
           
        ret_list = {}
        if (self.__bypass_parallel_processing==1):
            ret_list = self._sequential_return_list
            self._sequential_return_list = {}        
        elif (self._cluster != None):
            ret_list =  self._cluster.get_return_list()
        else:
            return None
        
        index = 0
        if isinstance(ret_list.values()[0],bool) and self._consolidateOutput:
            retval = True
            for subMs in ret_list:
                if not ret_list[subMs]:
                    casalog.post("%s failed for sub-MS %s" % (self._taskName,subMs),"WARN","postExecution")
                    retval = False
                index += 1
            return retval
        elif any(isinstance(v,dict) for v in ret_list.itervalues()) and self._consolidateOutput:
            ret_dict = {}
            for subMs in ret_list:
                dict_i = ret_list[subMs]
                # (CAS-4119): Neglectable NullSelection errors may cause flagdata to return None
                if isinstance(dict_i,dict):
                    try:
                        ret_dict = self.sum_dictionaries(dict_i,ret_dict)
                    except Exception, instance:
                        casalog.post("Error post processing MMS results %s: %s" % (subMs,instance),"WARN","postExecution")
            return ret_dict     
        elif (ret_list.values()[0]==None) and self._consolidateOutput:  
            return None      
        else:
            return ret_list
        
    # (CAS-4376): Consolidate list of return variables from the different engines into one single value 
    def sum_dictionaries(self,dict_list,ret_dict):
        for key in dict_list:
            item = dict_list[key]
            if isinstance(item,dict):
                if ret_dict.has_key(key):
                    ret_dict[key] = self.sum_dictionaries(item,ret_dict[key])
                else:
                    ret_dict[key] = self.sum_dictionaries(item,{})
            else:
                if ret_dict.has_key(key):
                    if not isinstance(ret_dict[key],str):
                        ret_dict[key] += item
                else:
                    ret_dict[key] = item
        return ret_dict   
            
    def go(self):
        
        casalog.origin("ParallelTaskHelper")
        
        self.initialize()
        if (self.generateJobs()):
            self.executeJobs()
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
