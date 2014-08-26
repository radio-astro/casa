#!/usr/bin/env python
import os
import re
import shutil
import string
import copy
import math
import time
from taskinit import *
from parallel.parallel_task_helper import ParallelTaskHelper
import simple_cluster
import partitionhelper as ph
import inspect
from numpy.f2py.auxfuncs import throw_error


# Decorator function to print the arguments of a function
def dump_args(func):
    "This decorator dumps out the arguments passed to a function before calling it"
    argnames = func.func_code.co_varnames[:func.func_code.co_argcount]
    fname = func.func_name
   
    def echo_func(*args,**kwargs):
        print fname, ":", ', '.join('%s=%r' % entry for entry in zip(argnames,args) + kwargs.items())
        return func(*args, **kwargs)
   
    return echo_func

"""
ParallelDataHelper is a class to process Multi-MS. It can process the MMS
as input and as an output. When the input is an MMS, this class will create
parallel jobs for each subMS and execute them using ParallelTaskHelper.
When the purpose is to create an output MMS, this class will work the heuristics
to partition the input MS based on the given separation axis and the transformations
performed by the client task.

There are two types of tasks that may use this class:
1) tasks that read an input MMS and create an output MMS with the same
   structure of the input (similar partition axis). Ex: split2

2) tasks that do 1) and can also create an output MMS from an input MS.
   These tasks have a parameter createmms with several sub-parameters.
   Ex: partition and mstransform.

In order to call ParallelDataHelper from a task, a few things are needed:
See examples in task_mstransform, task_partition.py, task_split2 or task_hanningsmooth2.py.

 *  It is assumed that the client task has data selection parameters, or at least
    spw and scan parameters.
         
 *  ParallelDataHelper will override the ParallelTaskHelper methods:
    isParallelMS(), initialize(), generateJobs() and postExecution().
       
 1) How to use ParallelDataHelper to process an input MMS (similar to split2)
    and create an output MMS with the same parallel structure of the input.
 
    from parallel.parallel_data_helper import ParallelDataHelper
    
    # Initiate the helper class    
    pdh = ParallelDataHelper('taskname', locals()) 
    
    # Validate input and output parameters
    pdh.setupIO()
    
    # To read and work with input MMS (the output will be an MMS too)
    if pdh.isParallelMS() and keepmms==True:
    
        # validate some parameters. In some cases the MMS needs to be
        # treated as a monolithic MS. In these cases, use mstransform instead
        pdh.validateInputParams()
                
        # Get a cluster
        pdh.setupCluster('taskname')
        
        # run the jobs in parallel
        try:
            pdh.go()
        except Exception, instance:
            casalog.post('%s'%instance,'ERROR')
            return False
            
        return True

 2) How to use ParallelDataHelper to create an output MMS (similar to partition)
    
    from parallel.parallel_data_helper import ParallelDataHelper
    
    # Initiate the helper class    
    pdh = ParallelDataHelper('taskname', locals()) 
    
    # Validate input and output parameters
    pdh.setupIO()
    
    if createmms==True:   
            
        # Get a cluster
        pdh.setupCluster('taskname')
        
        try:
            pdh.go()
        except Exception, instance:
            casalog.post('%s'%instance,'ERROR')
            return False
                    
        return True
"""


class ParallelDataHelper(ParallelTaskHelper):

    def __init__(self, thistask, args={}):
        
        self.__args = args
        self.__taskname = thistask
        
        self.__selectionScanList = None
        self.__ddistart = None
        self._msTool = None
        self._tbTool = None
        
        if not self.__args.has_key('spw'):
            self.__args['spw'] = ''
            
        self.__spwSelection = self.__args['spw']
        self.__spwList = None
        # __ddidict contains the names of the subMSs to consolidate.
        # The keys are the ddistart and the values the subMSs names
        self.__ddidict = {}
        
        # Start parameter for DDI in main table of each sub-MS.
        # It should be a counter of spw IDs starting at 0
        if self.__args.has_key('ddistart'):
              self.__ddistart = self.__args['ddistart']
                                
                                                                
    def setTaskName(self, thistask=''):        
        self.__taskname = thistask       
        
#    @dump_args
    def setupIO(self):
        """ Validate input and output parameters """
        
        if isinstance(self.__args['vis'], str):
            if not os.path.exists(self.__args['vis']):
                raise IOError, 'Visibility data set not found - please verify the name.'

        if isinstance(self.__args['outputvis'], str):
            # only one output MS
            if self.__args['outputvis'].isspace() or self.__args['outputvis'].__len__() == 0:
                raise IOError, 'Please specify outputvis.'
            
            elif os.path.exists(self.__args['outputvis']):
                raise IOError, "Output MS %s already exists - will not overwrite it."%self.__args['outputvis']
            
        flagversions = self.__args['outputvis']+".flagversions"
        if os.path.exists(flagversions):
            raise IOError, "The flagversions %s for the output MS already exist. Please delete it."%flagversions                                     
        
        return True 
        
#    @dump_args
    def validateInputParams(self):
        """ This method should run before setting up the cluster to work all the
           heuristics associated with the input MMS and the several
           transformations that the task does.
           The parameters are validated based on the client task.
           This method must use the self.__args parameters from the local class.
           
           This method will determine if the task can process the MMS in parallel
           or not.
           
           It returns a dictionary of the following:
            retval{'status': True,  'axis':''}         --> can run in parallel        
            retval{'status': False, 'axis':'value'}    --> treat MMS as monolithic MS, set axis of output MMS
            retval{'status': False, 'axis':''}         --> treat MMS as monolithic MS, create output MS
               
           the new axis, which can be: scan,spw or auto.
           
        """                
        # Return dictionary
        retval = {}
        retval['status'] = True
        retval['axis'] = ''
        
        # Get the separationaxis of input MMS. 
        sepaxis = ph.axisType(self.__args['vis'])
        if sepaxis.isspace() or sepaxis.__len__() == 0:
            sepaxis = 'unknown'
        elif sepaxis == 'scan,spw':
            sepaxis = 'auto'
            
        #Get list of subMSs in MMS
        subMSList = ParallelTaskHelper.getReferencedMSs(self.__args['vis'])
        
        if self.__taskname == "mstransform":
            
            if (self.__args['combinespws'] == True or self.__args['nspw'] > 1) and \
                (self.__args['timeaverage'] == False):
                spwsel = self.__getSpwIds(self.__args['vis'], self.__args['spw'])                    
                # Get dictionary with spwids of all subMS in the MMS
                spwdict = ph.getScanSpwSummary(subMSList)                
                # For each subMS, check if it has the spw selection
                for subms in subMSList:
                    subms_spwids = ph.getSubMSSpwIds(subms, spwdict)
                    slist = map(str,subms_spwids)
                    print 'subms=%s, slist=%s'%(subms,slist)
                    # Check if the subms contains all the selected spws
                    if not self.__isSpwContained(spwsel, slist):
                        casalog.post('Cannot combine or separate spws in parallel because the subMSs do not contain all the selected spws',\
                              'WARN')
                        # Set the new separation axis for the output
                        retval['status'] = False
                        retval['axis'] = 'scan'
                        break
                    
            elif (self.__args['timeaverage'] == True and self.__args['timespan'] == 'scan') and \
                (self.__args['combinespws'] == False and self.__args['nspw'] == 1):
                # Get the value of timebin as a float
                timebin = self.__args['timebin']
                tsec = qa.quantity(timebin,'s')['value']
                scansel = self.__getScanIds(self.__args['vis'], self.__args['scan'])
                # For each subms, check if scans length is <=  timebin
                for subms in subMSList:
                    if not self.__isScanContained(subms, scansel, tsec):
                        casalog.post('Cannot process MMS in parallel when timespan=\'scan\' because the subMSs do not contain all the selected scans',\
                                     'WARN')
                        # Set the new separation axis for the output
                        retval['status'] = False
                        retval['axis'] = 'spw'
                        break
                
            # Two transformations are requested.
            elif (self.__args['combinespws'] == True or self.__args['nspw'] > 1) and \
                (self.__args['timeaverage'] == True and self.__args['timespan'] == 'scan'):
                # Check spws and scans in subMSs
                spwsel = self.__getSpwIds(self.__args['vis'], self.__args['spw'])
                spwdict = ph.getScanSpwSummary(subMSList) 
                scansel = self.__getScanIds(self.__args['vis'], self.__args['scan'])
                timebin = self.__args['timebin']
                tsec = qa.quantity(timebin,'s')['value']
                for subms in subMSList:
                    subms_spwids = ph.getSubMSSpwIds(subms, spwdict)
                    slist = map(str,subms_spwids)
                    if self.__isSpwContained(spwsel, slist):
                        if not self.__isScanContained(subms, scansel, tsec):
                            casalog.post('The subMSs of input MMS do not contain the necessary scans','WARN')
                            retval['status'] = False
                            retval['axis'] = ''
                            break                        
                    else:
                        casalog.post('The subMSs of input MMS do not contain the necessary spws','WARN')
                        retval['status'] = False
                        retval['axis'] = ''
                        break
                                                
                                    
        elif self.__taskname == "split2":                            
            if (sepaxis != 'spw' and self.__args['combine'] == 'scan'):
                scansel = self.__getScanIds(self.__args['vis'], self.__args['scan'])
                timebin = self.__args['timebin']
                tsec = qa.quantity(timebin,'s')['value']
                for subms in subMSList:
                    if not self.__isScanContained(subms, scansel, tsec):
                        casalog.post('Cannot process MMS in parallel when combine=\'scan\' because the subMSs do not contain all the selected scans',\
                                     'WARN')
                        casalog.post("Please set keepmms to False or use task mstransform in this case.",'ERROR')
                        retval['status'] = False
                        retval['axis'] = ''
                        break

        return retval

#    @dump_args
    def __getSpwIds(self, msfile, spwsel):
        """Get the spw IDs of the spw selection
        Keyword arguments
            msfile    -- MS or MMS name
            spwsel    -- spw selection
            
            It will remove the channels from the selection and return only the spw ids.
        """
        myspwsel = spwsel
        if myspwsel.isspace() or myspwsel.__len__() == 0:
            myspwsel = '*'
    
        spwlist = []
        msTool = mstool()
        try:
            seldict = msTool.msseltoindex(vis=msfile,spw=myspwsel)
        except:
            return spwlist
    
        spwids = list(set(seldict['spw']))
        spwlist = map(str,spwids)
    
        del msTool
        return spwlist
                    
#    @dump_args
    def __isSpwContained(self, spwlist, subms_spws):
        """ Return True if the subMS contains the spw selection or False otherwise. 
        Keyword arguments:
            spwlist    -- list of selected spwids in MMS, e.g. ['0','1']. Do not include channels
            subms_spws -- list of spwids in subMS
        """
        
        isSelected = False
                                
        # Check if the selected spws are in the subMS
        if set(spwlist) <= set(subms_spws):
            isSelected = True
                        
        return isSelected

    def __getScanIds(self, msfile, scansel):
        """ Get the scan IDs of the scan selection.
        Keyword arguments:
        msfile    -- MS or MMS name
        scansel   -- scan selection
        
        Returns a list of the scan IDs (list of strings) or [] in case of failure.
        """
        scanlist = []
        if scansel.isspace() or scansel.__len__() == 0:
            # Get all the scan ids
            mymsmd = msmdtool()
            mymsmd.open(msfile)
            scans = mymsmd.scannumbers()
            mymsmd.close()
            scanlist = map(str,scans)
        else:
            try:            
                myms.open(msfile)
                myms.msselect({'scan':scansel})
                scans = myms.msselectedindices()['scan']
                scanlist = map(str,scans)
                myms.close()
            except:
                myms.close()
                scanlist = []
            
        return scanlist
        
    def __isScanContained(self, subms, scanlist, tbin):
            """ Check if subMS contains all the selected scans
                and if the duration of the subMS scans is larger or 
                equal to the timebin.
                
            Keyword arguments:
            subms      -- subMS name
            scanlist   -- list with selected scans for the MMS
            tbin       -- timebin as a Float
            
            Returns True on success, False otherwise.
            """
            isContained = False                            
            
            mymsmd = msmdtool()
            mymsmd.open(subms)
            
            # Check if subms scans contain all selected scans
            hasScans = False
            s = mymsmd.scannumbers()
            subms_scans = map(str, s)
            if set(scanlist) <= set(subms_scans):
                hasScans = True
                
            if hasScans:
                t = mymsmd.timesforscans(s)
                mymsmd.close()
                t_range = t.max() - t.min()
            
                if t_range >= tbin:  
                    isContained = True
                
            return isContained
                
    def validateOutputParams(self):
        """ This method should run before setting up the cluster to work all the
           heuristics associated with the separationaxis and the several
           transformations that the task does.
           This method must use the local class self.__args parameters
        """
        
        # success
        retval = 1
        if not self.__args.has_key('separationaxis'):
            return retval
        
        else:
            sepaxis = self.__args['separationaxis']

        # Task mstransform
        if self.__taskname == "mstransform":
            if sepaxis != 'scan' and (self.__args['combinespws'] == True or self.__args['nspw'] > 1):
                casalog.post('Cannot partition MS per spw or auto when combinespws = True or nspw > 1', 'WARN')
                retval = 0
                        
            elif sepaxis != 'spw' and self.__args['timespan'] == 'scan':
                    casalog.post('Time averaging across scans may lead to wrong results when separation axis is not spw', 'WARN')
            
        return retval

    @staticmethod
    def isParallelMS(vis):
        """ This method will read the value of SubType in table.info
            of the Multi-MS or MS. 
            
        Keyword arguments:
            vis  -- name of MS or Multi-MS
            
            It returns True if SubType is CONCATENATED, False otherwise.
            This method overrides the one from ParallelTaskHelper.
        """
               
        msTool = mstool()
        if not msTool.open(vis):
            raise ValueError, "Unable to open MS %s," % vis
        rtnVal = msTool.ismultims() and \
                 isinstance(msTool.getreferencedtables(), list)

        msTool.close()
        return rtnVal

    def override__args(self,arg,value):
        """ Override a parameter value in ParallelDataHelper arguments
        
        Keyword arguments:
            arg    --  name of the parameter
            value  --  value of the parameter
            
            It is usually used for the outputvis or createmms parameters.
        """
        self.__args[arg] = value
        
#    @dump_args
    def setupCluster(self, thistask=''):
        """ Get a simple_cluster 
        
        Keyword argument:
            thistask  --  the task calling this class
            
            ParallelTaskHelper will populate its self._arg dictionary with
            a copy of the parameters of the calling task, which have been
            set in self.__args.
        """
        
        # It needs to use the updated list of parameters!!!
        ParallelTaskHelper.__init__(self, task_name=thistask, args=self.__args)            
    
#    @dump_args
    def setupParameters(self, **pars):
        """ Create a dictionary with non-empty parameters 
        
        Keyword argument:
            **pars  --  a dictionary with key:value pairs
            
            It will return a dictionary with only non-empty parameters.
        """
        
        seldict = {}
        for k,v in pars.items():
            if v != None and v != "":
                seldict[k] = v
       
        return seldict
    
#    @dump_args
    def validateModelCol(self):
        """ Add the realmodelcol parameter to the configuration
           only for some values of datacolumn. Specific for mstransform.
           This method must use the local class self.__args parameters.
        """
        
        ret = False
        
        dc = self.__args['datacolumn'].upper()
        if dc=='MODEL' or dc=='ALL' or dc=='DATA,MODEL,CORRECTED':
            ret = True

        return ret
    
#    @dump_args
    def initialize(self):
        """Initializes some parts of the cluster setup.
            Add the full path for the input and output MS.
            Creates the temporary directory to save each
            parallel subMS. The directory is called
            <outputvis>.data.
           This method overrides the one from ParallelTaskHelper.
        """
        
        casalog.origin("ParallelDataHelper")

        # self._arg is populated inside ParallelTaskHelper._init_()
        self._arg['vis'] = os.path.abspath(self._arg['vis'])
            
        if (self._arg['outputvis'] != ""):
            self._arg['outputvis'] = os.path.abspath(self._arg['outputvis'])        

        outputPath, self.outputBase = os.path.split(self._arg['outputvis'])
        try:
            self.outputBase = self.outputBase[:self.outputBase.rindex('.')]
        except ValueError:
            # outputBase must not have a trailing .
            pass

        if self.outputBase == '.' or self.outputBase == './':
            raise ValueError, 'Error dealing with outputvis'
        
        # The subMS are first saved inside a temporary directory
        self.dataDir = outputPath + '/' + self.outputBase+'.data'
        if os.path.exists(self.dataDir): 
            shutil.rmtree(self.dataDir)

        os.mkdir(self.dataDir)                    
         
#    @dump_args
    def generateJobs(self):
        """ This is the method which generates all of the actual jobs to be done.
            This method overrides the one in ParallelTaskHelper baseclass.
        """
        
        casalog.origin("ParallelDataHelper")
        casalog.post("Analyzing MS for partitioning")

        # Input MMS, processed in parallel; output is an MMS
        # For tasks such as split2, hanningsmooth2
        if ParallelDataHelper.isParallelMS(self._arg['vis']) and (not self._arg.has_key('monolithic_processing')):           
            self.__createNoSeparationCommand()
            
        # For mstransform when processing input MMS in parallel
        elif ParallelDataHelper.isParallelMS(self._arg['vis']) and  self._arg['monolithic_processing'] == False:
            self.__createNoSeparationCommand()
           
        # For tasks that create an output MMS. In these cases
        # input can be an MMS processed monolithically or an input MS
        elif self._arg['createmms']:
            self.__createPrimarySplitCommand()
                
        return True

#    @dump_args
    def __createNoSeparationCommand(self):
        """ Add commands to be executed by the engines when input is an MMS. 
            This method overrides the following parameter:
            self._arg['createmms']
        """

        submslist = ParallelTaskHelper.getReferencedMSs(self._arg['vis'])
        if len(submslist) == 0:
            raise ValueError, 'There are no subMSs in input vis'
                    
        tbTool = tbtool()

        listOutputMS = []

        subMs_idx = 0
        for subMS in submslist:

            # make sure the SORTED_TABLE keywords are disabled
            tbTool.open(subMS, nomodify=False)
            if 'SORTED_TABLE' in tbTool.keywordnames():
                tobeDeleted = tbTool.getkeyword('SORTED_TABLE').split(' ')[1]
                tbTool.removekeyword('SORTED_TABLE')
                os.system('rm -rf '+tobeDeleted)
            
            tbTool.close()    

            listOutputMS.append(self.dataDir+'/%s.%04d.ms' \
                                  % (self.outputBase, subMs_idx))
            subMs_idx += 1

        # Override the original parameters
        self.override_arg('outputvis',listOutputMS)
            
        self._consolidateOutput = False
        
        # Add to the list of jobs to execute
        subMs_idx = 0
        for subMS in submslist:
            localArgs = copy.copy(self._arg)
            localArgs['vis'] = subMS
            for key in self._arguser:
                localArgs[key] = self._arguser[key][subMs_idx]
                
            if self._arg.has_key('createmms'):
                self._arg['createmms'] = False
                localArgs['createmms'] = False
                
            subMs_idx += 1
            self._executionList.append(
                simple_cluster.JobData(self._taskName,localArgs))

    
#    @dump_args
    def __createPrimarySplitCommand(self):     
        """ This method overwrites the following parameter:
            self._arg['separationaxis'] when running the monolithic case
        """
                
        if self._arg['createmms']:
                        
            if self._arg['separationaxis'].lower() == 'scan':
                self.__createScanSeparationCommands()
            elif self._arg['separationaxis'].lower() == 'spw':
                self.__createSPWSeparationCommands()
            elif self._arg['separationaxis'].lower() == 'auto':
                self.__createDefaultSeparationCommands()
            else:
                # Use a default
                self.__createDefaultSeparationCommands()
#         else:
#             # TODO: REVIEW this later. 
#             # Single mms case
#             singleCmd = copy.copy(self._arg)
#             scanList = self.__selectionScanList
# #           if scanList is None:
# #               self._selectMS()
# #               scanList = self._getScanList()
#             if scanList is not None:
#                 singleCmd['scan'] = ParallelTaskHelper.\
#                                     listToCasaString(scanList)
#             self._executionList.append(
#                 simple_cluster.JobData(self._taskName, singleCmd))
            
#    @dump_args
    def __createScanSeparationCommands(self):
        """ This method is to generate a list of commands to partition
             the data based on scan.
        """
       
        scanList = self.__selectionScanList
        if scanList is None:
            self.__selectMS()
            scanList = self.__getScanList()

        # Make sure we have enough scans to create the needed number of
        # subMSs.  If not change the total expected.
        numSubMS = self._arg['numsubms']
        numSubMS = min(len(scanList),numSubMS)
        
        partitionedScans = self.__partition(scanList, numSubMS)        
        for output in xrange(numSubMS):
            mmsCmd = copy.copy(self._arg)
            mmsCmd['createmms'] = False
            mmsCmd['scan']= ParallelTaskHelper.\
                            listToCasaString(partitionedScans[output])                                                
            mmsCmd['outputvis'] = self.dataDir+'/%s.%04d.ms' \
                                  % (self.outputBase, output)
            self._executionList.append(
                simple_cluster.JobData(self._taskName, mmsCmd))

#    @dump_args
    def __createSPWSeparationCommands(self):
        """ This method is to generate a list of commands to partition
             the data based on spw.
        """

        # Get a unique list of selected spws        
        self.__selectMS()
        spwList = self.__getSPWUniqueList()
        numSubMS = self._arg['numsubms']
        numSubMS = min(len(spwList),numSubMS)

        # Get a dictionary of the spws parted for each subMS
        spwList = map(str,spwList)
        partitionedSPWs1 = self.__partition1(spwList,numSubMS)

        # Add the channel selections back to the spw expressions
        newspwsel = self.__createSPWExpression(partitionedSPWs1)
        
        # Validate the chanbin parameter
        validbin = False
        parname = self.getChanAvgParamName()
        if self.validateChanBin():
            if isinstance(self._arg[parname],list):
                freqbinlist = self.__partition1(self._arg[parname],numSubMS)
                validbin = True
                    
        # Calculate the ddistart for each engine. This will be used
        # to calculate the DD IDs of the output main table of the subMSs
        ddistartlist = self.__calculateDDIstart({}, partitionedSPWs1)
        if (len(ddistartlist) != len(partitionedSPWs1)):
            casalog.post('Error calculating the ddistart indices','SEVERE')
            raise
        
        for output in xrange(numSubMS):
            mmsCmd = copy.copy(self._arg)
            mmsCmd['createmms'] = False
            if self.__selectionScanList is not None:
                mmsCmd['scan'] = ParallelTaskHelper.\
                                 listToCasaString(self.__selectionScanList)            
            mmsCmd['spw'] = newspwsel[output]
            if validbin:
                mmsCmd[parname] = freqbinlist[output]
                
            self.__ddistart = ddistartlist[output]
            mmsCmd['ddistart'] = self.__ddistart
            mmsCmd['outputvis'] = self.dataDir+'/%s.%04d.ms' \
                                  % (self.outputBase, output)

            # Dictionary for the spw/ddi consolidation later
            self.__ddidict[self.__ddistart] = self.dataDir+'/%s.%04d.ms' \
                                  % (self.outputBase, output)

            self._executionList.append(
                simple_cluster.JobData(self._taskName, mmsCmd))
            
#    @dump_args
    def __createDefaultSeparationCommands(self):
        """ This method is to generate a list of commands to partition
             the data based on both scan/spw axes.
        """
        
        casalog.post('Partition per scan/spw will ignore NULL combinations of these two parameters.')

        # Separates in scan and spw axes
        self.__selectMS()
            
        # Get the list of spectral windows as strings
        spwList = self.__getSPWUniqueList() 
        spwList = map(str,spwList)

        # Check if we can just divide on SPW or if we need to do SPW and scan
        numSubMS = self._arg['numsubms']
        numSpwPartitions = min(len(spwList),numSubMS)
        numScanPartitions = int(math.ceil(numSubMS/float(numSpwPartitions)))

        if numScanPartitions > 1:
            # Check that the scanlist is not null
            scanList = self.__selectionScanList
            if scanList is None:
                scanList = self.__getScanList()

            # Check that the number of scans is enough for the partitions
            if len(scanList) < numScanPartitions:
                numScanPartitions = len(scanList)
        else:
            scanList = None

        partitionedSpws  = self.__partition1(spwList,numSpwPartitions)
        partitionedScans = self.__partition(scanList,numScanPartitions)
        
        # The same list but as a dictionary
        str_partitionedScans = self.__partition1(scanList,numScanPartitions)
            
        # Validate the chanbin parameter
        validbin = False
        parname = self.getChanAvgParamName()
        if self.validateChanBin():
            if isinstance(self._arg[parname],list):
                freqbinlist = self.__partition1(self._arg[parname],numSpwPartitions)
                validbin = True

        # Add the channel selections back to the spw expressions
        newspwsel = self.__createSPWExpression(partitionedSpws)
        
        # Calculate the ddistart for the subMSs (for each engine)
        ddistartlist = self.__calculateDDIstart(str_partitionedScans, partitionedSpws)
                
        if (len(ddistartlist) != len(xrange(numSpwPartitions*numScanPartitions))):
            casalog.post('Error calculating ddistart for the engines', 'SEVERE')
            raise
        
        # Set the first DD ID for the sub-table consolidation
        ddi0 = ddistartlist[0]
        self.__ddistart = 0
        
        # index that composes the subms names (0000, 0001, etc.)
        sindex = 0
        for output in xrange(numSpwPartitions*numScanPartitions):
            
            # Avoid the NULL MS selections by verifying that the
            # combination scan-spw exist.
            scansellist = map(str, partitionedScans[output%numScanPartitions])
            selscan = ''
            for ss in scansellist:
                selscan = selscan + ',' + ss
            selscan = selscan.lstrip(',')
            if not self.__scanspwSelection(selscan,
                                 str(newspwsel[output/numScanPartitions])):
                continue

            # The first valid subMS must have DDI 0
            if sindex == 0:                 
                self.__ddidict[0] = self.dataDir+'/%s.%04d.ms'%\
                                     (self.outputBase, sindex)
            else:                                           
                self.__ddistart = ddistartlist[output]
                     
                if self.__ddistart != ddi0:
                    ddi0 = ddistartlist[output]          
                    # Dictionary for sub-table consolidation
                    self.__ddidict[self.__ddistart] = self.dataDir+'/%s.%04d.ms'% \
                                                    (self.outputBase, sindex)
            
            mmsCmd = copy.copy(self._arg)
            mmsCmd['createmms'] = False           
            mmsCmd['scan'] = ParallelTaskHelper.listToCasaString \
                             (partitionedScans[output%numScanPartitions])
            
            mmsCmd['spw'] = newspwsel[output/numScanPartitions]
            if validbin:
                mmsCmd[parname] = freqbinlist[output/numScanPartitions]
            mmsCmd['ddistart'] = self.__ddistart
            mmsCmd['outputvis'] = self.dataDir+'/%s.%04d.ms' \
                                  % (self.outputBase, sindex)
            self._executionList.append(
                simple_cluster.JobData(self._taskName, mmsCmd))
            
            sindex += 1 # index of subMS name

    def __scanspwSelection(self, scan, spw):
        """ Return True if the selection is True or False otherwise. """
        
        isSelected = False
        mysel = {}
        mysel['scan'] = scan
        mysel['spw'] = spw
        
        if self._msTool is None:
            # Open up the msTool
            self._msTool = mstool()
            self._msTool.open(self._arg['vis'])    
        else:
            self._msTool.reset()

        try:
            isSelected = self._msTool.msselect(mysel)
        except:
            isSelected = False
            casalog.post('Ignoring NULL combination of scan=%s and spw=%s'% \
                             (scan,spw),'DEBUG1')
        
        return isSelected
            
#    @dump_args
    def __calculateDDIstart(self, partedscans, partedspws):
        """ Calculate the list of DDI values for each partition (each engine).
        
        Keyword arguments:
            partedscans    ---  dictionary of parted scans as returned from self.__partition1()
            partedspws     ---  dictionary of parted spws as returned from self.__partition1()
        
        It returns a list of ddistart values with the same size of the number of subMSs.
        """
            
        # Example of partedspws:
        # create 2 subMss with spw=0,1,2 and spw=3
        # partedSPWs = {0:['0','1','2'],1:['3']}
        #
        # create 3 subMSs with spw=0,1,2 spw=3 and spw=4,5
        # partedSPWs = {0:['0','1','2'],1:['3'],2:['4','5']}
                
        hasscans = True
        if len(partedscans) == 0:
            scans = ''
            hasscans = False

        ddistartList = []
               
        # scan+spw separation axis 
        if hasscans:
            count = 0
            for k,spws in partedspws.iteritems():
                for ks,scans in partedscans.iteritems():
                    if self._msTool is None:
                        self._msTool = mstool()
                        self._msTool.open(self._arg['vis'],nomodify=False)
                    else:
                        self._msTool.reset()
            
                    try:
                        # The dictionary with selected indices
                        seldict = self._msTool.msseltoindex(vis=self._arg['vis'],scan=scans,spw=spws)
                    except:
                        self._msTool.close()
                        continue
                                        
                    # Get the selected DD IDs
                    ddis = seldict['dd'].tolist()
                    ddsize = ddis.__len__()
                    if count == 0:
                        ddistart = 0
                        
                    # Create a ddistart list
                    ddistartList.append(ddistart)
                ddistart = ddistart + ddsize
                count = count + 1
           
        # spw separation axis 
        else:
            count = 0
            for k,spws in partedspws.iteritems():
                if self._msTool is None:
                    self._msTool = mstool()
                    self._msTool.open(self._arg['vis'],nomodify=False)
                else:
                    self._msTool.reset()
        
                try:
                    # The dictionary with selected indices
                    seldict = self._msTool.msseltoindex(vis=self._arg['vis'],scan=scans,spw=spws)
                except:
                    self._msTool.reset()
                    continue
                                    
                # Get the selected DD IDs
                ddis = seldict['dd'].tolist()
                ddsize = ddis.__len__()
                if count == 0:
                    ddistart = 0
                    
                # Create a ddistart list
                ddistartList.append(ddistart)
                ddistart = ddistart + ddsize
                count = count + 1
                                                
        return ddistartList
 
#    @dump_args
    def __selectMS(self):
        """ This method will open the MS and ensure whatever selection criteria
            have been requested are honored. If scanList is not None then it 
            is used as the scan selection criteria.
        """
        
        if self._msTool is None:
            self._msTool = mstool()
            self._msTool.open(self._arg['vis'])    
        else:
            self._msTool.reset()
            
        # It returns a dictionary if there was any selection otherwise None
        self.__selectionFilter = self.__getSelectionFilter()

        if self.__selectionFilter is not None:
            self._msTool.msselect(self.__selectionFilter)

#    @dump_args
    def __getScanList(self):
        """ This method returns the scan list from the current ms.  Be careful
            about having selection already done when you call this.
        """
        
        if self._msTool is None:
            self.__selectMS()

        scanSummary = self._msTool.getscansummary()
        scanList = [int(scan) for scan in scanSummary]

        if len(scanList) == 0:
            raise ValueError, "No Scans present in the created MS."

        scanList.sort()
        return scanList

#    @dump_args
    def __getSPWUniqueList(self):
        """ This method returns a unique list of spectral windows from the current
            MS.  Be careful about having selection already done when you call this.
        """
        
        if self._msTool is None:
            self.__selectMS()
        
        # Now get the list of SPWs in the selected MS
        ddInfo = self._msTool.getspectralwindowinfo()
#        spwList = [info['SpectralWindowId'] for info in ddInfo.values()]
        self.__spwList = [info['SpectralWindowId'] for info in ddInfo.values()]

        # Return a unique sorted list:
        sorted = list(set(self.__spwList))
#        sorted.sort()
        return sorted

#    @dump_args
    def __getSelectionFilter(self):
        """ This method takes the list of specified selection criteria and
            puts them into a dictionary.  There is a bit of name mangling necessary.
            The pairs are: (msselection syntax, mstransform task syntax).
        """
        
        selectionPairs = []
        selectionPairs.append(('field','field'))
        selectionPairs.append(('spw','spw'))
        selectionPairs.append(('polarization','correlation'))
        selectionPairs.append(('baseline','antenna'))
        selectionPairs.append(('time','timerange'))
        selectionPairs.append(('scan','scan'))
        selectionPairs.append(('uvdist','uvrange'))
        selectionPairs.append(('scanintent','intent'))
        selectionPairs.append(('observation','observation'))
        return self.__generateFilter(selectionPairs)

#    @dump_args
    def __generateFilter(self, selectionPairs):
        """It creates a dictionary of the non-empty selection parameters.
            
        Keyword argument:
        selectionPairs -- list with task parameter name = msselection parameter name
                          as returned from __getSelectionFilter()
                          
            It will look at the selection parameters in self_arg.
        """
        filter = None
        for (selSyntax, argSyntax) in selectionPairs:
            if self._arg.has_key(argSyntax) and self._arg[argSyntax] != '':
                if filter is None:
                    filter = {}
                filter[selSyntax] = self._arg[argSyntax]
                
        return filter

#    @dump_args
    def __partition(self, lst, n):
        """ This method will split the list lst into "n" almost equal parts
            if lst is none, then we assume an empty list.
        """
        
        if lst is None:
            lst = []
        
        division = len(lst)/float(n)
        
        return [ lst[int(round(division * i)):
                     int(round(division * (i+1)))] for i in xrange(int(n))]
    
#    @dump_args
    def __partition1(self, lst, n):
        """ This method will split the list lst into "n" almost equal parts.
            if lst is None, then we assume an empty list.
            
        Keyword arguments:
            lst --- spw list
            n   --- numsubms
            
        It returns a dictionary such as:
            given the selection spw='0,1:10~20,3,4,5'
            rdict = {0: ['0','1'], 1:['3','4','5']}
        """
        
        if lst is None:
            lst = []
        
        # Create a dictionary for the parted spws:
        rdict = {}
        division = len(lst)/float(n)
        for i in xrange(int(n)):
            part = lst[int(round(division * i)):int(round(division * (i+1)))]
            rdict[i] = part
    
        return rdict


#    @dump_args
    def __chanSelection(self, spwsel):
        """ Create a dictionary of channel selections.
        
        Keyword arguments:
            spwsel --- a string with spw selection
            
        It returns a dictionary such as:
            spwsel = "'0,1:10~20"
            seldict = {0: {'channels': '', 'spw': '0'}, 
                       1: {'channels': '10~20', 'spw': '1'}}
        """
        
        # Split to get each spw in a list
        if spwsel.__contains__(','):
            spwlist = spwsel.split(',')        
        else:
            spwlist = spwsel.split(';')
                        
        spwid=[]
        chanlist=[]
        # Split to create two lists, one with channels, the other with spwIDs
        for isel in spwlist:
            # Get tail, colon and head
            (s, c, ch) = isel.rpartition(":")
            # Remove any blanks
            s = s.strip(' ')
            c = c.strip(' ')
            ch = ch.strip(' ')
            # If no tail, there was no colon to split. In this case, add the spwID
            if s == "":
                spwid.append(ch)
                chanlist.append('')
            else:
                spwid.append(s)
                chanlist.append(ch)
                
        # Create a dictionary
        seldict = {}
        for ns in xrange(len(spwid)):
            sel = {}
            sel['spw'] = spwid[ns]
            sel['channels'] = chanlist[ns]
            seldict[ns] = sel


        return seldict

#    @dump_args 
    def __createSPWExpression(self, partdict):
        """ Creates the final spw expression that will be sent to the engines.
            This adds back the channel selections to their spw counterparts.
            
        Keyword arguments:
            partdict --- dictionary from __partition1, such as:
                
            Ex: partdict = {0: ['0','1'], 1:['3','4','5']}
                when selection is spw = '0,1:10~20,3,4,5'
                and effective number of subMSs is 2.
        """           
        
        # Create a dictionary of the spw/channel selections
        # Ex: seldict = {0: {'channels': '', 'spw': '0'}, 
        #                1: {'channels': '10~20', 'spw': '1'}}
        seldict = self.__chanSelection(self.__spwSelection)
                
        newdict = copy.copy(partdict)
        
        # Match the spwId of partdict with those from seldict
        # For the matches that contain channel selection in seldict,
        # Add them to the spwID string in partdict
        for keys,vals in seldict.items():
            for k,v in partdict.items():
                for i in range(len(v)):
#                    if v[i] == seldict[keys]['spw'] and seldict[keys]['channels'] != '':
#                    if v[i] == vals['spw'] and vals['channels'] != '':
                    # matches, now edit pardict
                    if v[i] == vals['spw']:
#                        print v[i], seldict[keys]['spw'], seldict[keys]['channels']
                        if vals['channels'] != '':
                            spwexpr = vals['spw'] + ':' + vals['channels']
                        else:
#                        spwexpr = seldict[keys]['spw'] + ':' + seldict[keys]['channels']
                            spwexpr = vals['spw']
                        newdict[k][i] = spwexpr
        
        # We now have a new dictionary of the form:
        # newdict = {0: ['0', '1:10~20'], 1: ['3', '4','5']}
        # We want it to be:
        # newdict = {0: "0,1:10~20",1: "3, 4,5"}
        
        # Add a comma separator for each expression making
        # a single string for each key
        for k,v in newdict.items():
            spwstr = ""
            for s in range(len(v)):
                spwstr = spwstr + v[s] + ','
            newdict[k] = spwstr.rstrip(',')
                
        casalog.post('Dictionary of spw expressions is: ','DEBUG')
        casalog.post ('%s'%newdict,'DEBUG')
                
        return newdict

    def getChanAvgParamName(self):
        """ Get the channel average bin parameter name.
           It will return a string with the parameter name, based on
           the calling task.
        """

        casalog.origin("ParallelDataHelper")

        if self.__taskname == None:
            return None
        
        if self.__taskname == 'mstransform':
            return 'chanbin'
        elif self.__taskname == 'split' or self.__taskname == 'split2':
            return 'width'
            
        return None

#    @dump_args 
    def validateChanBin(self):
        """ Check if channel average bin parameter has the same
           size of the spw selection.
                                  
           Returns True if parameter is valid or False otherwise.
           This method must use the local class self.__args parameters.
           TBD: make it a static method
        """

        casalog.origin("ParallelDataHelper")
        
        retval = True
        
        # Get the parameter name, which depends on the task calling this class
        parname = self.getChanAvgParamName()
        casalog.post('Channel average parameter is called %s'%parname,'DEBUG1')
        if parname == None:
            retval = False
            
        elif self.__args.has_key(parname):
            fblist = self.__args[parname]
            if isinstance(fblist,list):   
                             
                if fblist.__len__() > 1:
                    if self.__spwList == None:           
                        msTool = mstool()
                        msTool.open(self.__args['vis'])
                        spwsel = self.__args['spw'] 
                        msTool.msselect({'spw':spwsel})
                        ddInfo = msTool.getspectralwindowinfo()
                        self.__spwList = [info['SpectralWindowId'] for info in ddInfo.values()]
                        msTool.close()
                        
                    if self.__spwList.__len__() != fblist.__len__():
                        retval = False
                        raise ValueError, 'Number of %s is different from the number of spw' %parname                
                 

        return retval
    
#    @dump_args
    def defaultRegridParams(self):
        """ Reset the default values of the regridms transformation parameters based on the mode.
            Specific for mstransform task.
            This method must use the local class self.__args parameters.
            TBD: make it a static method
        """

        casalog.origin("ParallelDataHelper")
        
        if self.__args['mode'] == 'channel' or self.__args['mode'] == 'channel_b':
            self.__args['start'] = str(self.__args['start'])
            self.__args['width'] = str(self.__args['width'])
            
        elif self.__args['mode'] == 'velocity':
            restfreq = self.__args['restfreq']
            if restfreq == "" or restfreq.isspace():
                raise ValueError, "Parameter restfreq must be set when mode='velocity'"
            
            if self.__args['start'] == 0:
                self.__args['start'] = ''
                
            if self.__args['width'] == 1:
                self.__args['width'] = ''
                

            # Check if the parameter has valid velocity units
            if not self.__args['start'] == '':
                if (qa.quantity(self.__args['start'])['unit'].find('m/s') < 0):
                    raise TypeError, 'Parameter start does not have valid velocity units'
            
            if not self.__args['width'] == '':
                if (qa.quantity(self.__args['width'])['unit'].find('m/s') < 0):
                    raise TypeError, 'Parameter width does not have valid velocity units'
                                            
        elif self.__args['mode'] == 'frequency':
            if self.__args['start'] == 0:
                self.__args['start'] = ''
            if self.__args['width'] == 1:
                self.__args['width'] = ''
    
            # Check if the parameter has valid frequency units
            if not self.__args['start'] == '':
                if (qa.quantity(self.__args['start'])['unit'].find('Hz') < 0):
                    raise TypeError, 'Parameter start does not have valid frequency units'
    
            if not self.__args['width'] == '':
                if (qa.quantity(self.__args['width'])['unit'].find('Hz') < 0):
                    raise TypeError, 'Parameter width does not have valid frequency units'        
        
        start = self.__args['start']
        width = self.__args['width']
        
        return start, width

#    @dump_args
    def postExecution(self):
        """ This method overrides the postExecution method of ParallelTaskHelper,
            in which case we probably need to generate the output reference MS.
        """

        casalog.origin("ParallelDataHelper")        
        casalog.post("Finalizing MMS structure")
                
        if self._msTool:
            self._msTool.close()
                
        # We created a data directory and many SubMSs,
        # now build the reference MS. The outputList is a
        # dictionary of the form:
        # {'path/outputvis.data/SUBMSS/outputvis.0000.ms':True,
        #  'path/outuputvis.data/SUBMSS/outputvis.0001.ms':False}
      
        if (ParallelTaskHelper.getBypassParallelProcessing()==1):
            # This is the list of output SubMSs
            outputList = self._sequential_return_list
            self._sequential_return_list = {}
        else:                                                    
            outputList =  self._cluster.get_output_return_list()
                                                                     
                     
        # List of failed MSs. TBD
        nFailures = []
        
        subMSList = []

        nFailures = [v for v in outputList.values() if v == False]
    
        for subMS in outputList:
            # Only use the successfull output MSs
            if outputList[subMS]:
                subMSList.append(subMS)
                           
        subMSList.sort()

        if len(subMSList) == 0:
            casalog.post("Error: no subMSs were successfully created.", 'WARN')
            return False
        
        # TBD: the list of subMSs to be merged should reflect the
        # failures from above!!
        
        
        # When separationaxis='scan' there is no need to give ddistart. 
        # The tool looks at the whole spw selection and
        # creates the indices from it. After the indices are worked out, 
        # it applies MS selection. We do not need to consolidate either.
                                       
        # If axis is spw or auto, give a list of the subMSs
        # that need to be consolidated. This list is pre-organized
        # inside the separation functions above.
        
        # Only when input is MS or MS-like and createmms=True
        # Only partition and mstransform have the createmms parameter
        if self._arg.has_key('createmms') and self._arg['createmms'] == True:
            if (self._arg['separationaxis'] == 'spw' or 
                self._arg['separationaxis'] == 'auto'):   
                
                casalog.post('Consolidate the sub-tables')
             
                toUpdateList = self.__ddidict.values()
                                
                toUpdateList.sort()
                casalog.post('List to consolidate %s'%toUpdateList,'DEBUG')
                                
                # Consolidate the spw sub-tables to take channel selection
                # or averages into account.
                mtlocal1 = mttool()
                try:                        
                    mtlocal1.mergespwtables(toUpdateList)
                    mtlocal1.done()
                except Exception, instance:
                    mtlocal1.done()
                    casalog.post('Cannot consolidate spw sub-tables in MMS','SEVERE')
                    raise

        if len(nFailures) > 0:
            casalog.post('%s subMSs failed to be created'%nFailures)
            # need to rename/re-index the subMSs
            newList = copy.deepcopy(subMSList)
            idx = 0
            for subms in newList:
                suffix = re.findall(r".\d{4}.ms",subms)
#                newms = subms.rpartition(suffix[-1])[0]   
                newms = subms[:-len(suffix[-1])]
                newms = newms+'.%04d.ms'%idx
                os.rename(subms,newms)
                newList[idx] = newms
                idx += 1
            
            if len(subMSList) == len(newList):
                subMSList = newList
              
        # Get the first subMS to be the reference when
        # copying the sub-tables to the other subMSs  
        mastersubms = subMSList[0]
            
        subtabs_to_omit = ['POINTING','SYSCAL']
        
        # Parallel axis to write to table.info of MMS
        # By default take the one from the input MMS
        parallel_axis = ph.axisType(self.__args['vis'])
        if self._arg.has_key('createmms') and self._arg['createmms'] == True:
            parallel_axis = self._arg['separationaxis']

        if parallel_axis == 'auto' or parallel_axis == 'both':
            parallel_axis = 'scan,spw'
            
        # Copy sub-tables from first subMS to the others. The tables in
        # subtabs_to_omit are linked instead of copied.
        ph.makeMMS(self._arg['outputvis'], subMSList,
                   True, # copy subtables
                   subtabs_to_omit, # omitting these
                   parallel_axis
                  )
    
        thesubmscontainingdir = os.path.dirname(subMSList[0].rstrip('/'))
            
        os.rmdir(thesubmscontainingdir)
            
        return True
    
    
    
    
    
    
    
    
    
    
    
