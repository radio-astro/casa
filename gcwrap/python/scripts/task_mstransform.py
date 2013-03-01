
import os
import shutil
import string
import copy
import math
from taskinit import *
from parallel.parallel_task_helper import ParallelTaskHelper
import simple_cluster
import partitionhelper as ph


# Decorator function to print the arguments of a function
def dump_args(func):
    "This decorator dumps out the arguments passed to a function before calling it"
    argnames = func.func_code.co_varnames[:func.func_code.co_argcount]
    fname = func.func_name
   
    def echo_func(*args,**kwargs):
        print fname, ":", ', '.join('%s=%r' % entry for entry in zip(argnames,args) + kwargs.items())
        return func(*args, **kwargs)
   
    return echo_func

class MSTHelper(ParallelTaskHelper):
    def __init__(self, args={}):
        
        # Create a copy of the original local parameters
        self.__args = args
        self.__origpars = args.copy()
        self.__selpars = {}        
        self.__isMMS = False
        self._calScanList = None
        self._selectionScanList = None
        self._msTool = None
        self._tbTool = None
        
        # start parameter for DDI in main table of each sub-MS
        # It should be a counter of spw IDs starting at 0
        self.__ddistart = self.__origpars['ddistart']
        
#    @dump_args
    def setupIO(self):
        '''Validate input and output parameters'''
        
        for k,v in self.__origpars.items():
            if k == 'vis' and isinstance(v, str):
                # only one input MS
                if not os.path.exists(v):
                    raise Exception, 'Visibility data set not found - please verify the name.'
                
            elif k == 'outputvis' and isinstance(v, str):
                # only one output MS
                if v.isspace() or v.__len__() == 0:
                    casalog.post('Please specify outputvis.','SEVERE')
                    raise ValueError, 'Please specify outputvis.'
                elif os.path.exists(v):
                    casalog.post("Output MS %s already exists - will not overwrite it."%v,'SEVERE')
                    raise ValueError, "Output MS %s already exists - will not overwrite it."%v                    
        
        return True 

#    @dump_args
    def validateParams(self):
        '''This method should run before setting up the cluster to work all the
           heuristics associated with the separationaxis and the several
           transformations that the task does.'''
        
        # can continue in parallel
        retval = 2
        
        if (self.__args['separationaxis'] == 'spw'):
            if self.__args['combinespws'] == True:
                casalog.post('Cannot partition MS in spw axis when combinespws = True', 'WARN')
                retval = 0
        
            elif self.__args['nspw'] > 1:
                # It will separate spws. Do it in sequential
                # CANNOT process in sequential either because internally the
                # spws need to be combined first before the separation!!!!!!
#                casalog.post('Can only process the MS in sequential', 'WARN')
                casalog.post('Cannot partition MS in spw axis and separate spws (nspw > 1)', 'WARN')
                retval = 0
                
        elif (self.__args['separationaxis'] == 'scan'):
            if self.__args['timespan'] == 'scan':
                casalog.post('Cannot partition MS in scan when timebin span across scans.\n'\
                              'Will reset timespan to None', 'WARN')
                self.__args['timespan'] == ''
                retval = 2
                
        elif (self.__args['separationaxis'] == 'both'):
            if self.__args['combinespws'] == True:
                casalog.post('Cannot partition MS in spw,scan axes when combinespws = True', 'WARN')
                retval = 0

            elif self.__args['timespan'] == 'scan':
                casalog.post('Cannot partition MS in spw,scan axes when timebin span across scans.', 'WARN')
                retval = 0
                
            elif self.__args['nspw'] > 1:
                # It will separate spws. Do it in sequential
                # CANNOT process in sequential either because internally the
                # spws need to be combined first before the separation!!!!!!
#                casalog.post('Can only process the MS in sequential', 'WARN')
                casalog.post('Cannot partition MS in spw axis and separate spws (nspw > 1)', 'WARN')
                retval = 0

                
        return retval
        
#    @dump_args
    def setupCluster(self):
        '''Get a simple_cluster'''
        ParallelTaskHelper.__init__(self,'mstransform', self.__args)
            
    
#    @dump_args
    def setupParameters(self, **pars):
        '''Create a dictionary with non-empty parameters'''
        
        seldict = {}
        for k,v in pars.items():
            if v != None and v != "":
                self.__selpars[k] = v
        
        return self.__selpars

#    @dump_args
    def initialize(self):
        """Add the full path for the input and output MS.
           This method overrides the one from ParallelTaskHelper."""
        
                
        self._arg['vis'] = os.path.abspath(self._arg['vis'])
            
        if (self._arg['outputvis'] != ""):
            self._arg['outputvis'] = os.path.abspath(self._arg['outputvis'])        

        outputPath, self.outputBase = os.path.split(self._arg['outputvis'])
        try:
            self.outputBase = self.outputBase[:self.outputBase.rindex('.')]
        except ValueError:
            # outputBase must not have a trailing .
            pass

        self.dataDir = outputPath + '/' + self.outputBase+'.data'
#        if self._arg['createmms']:
        if os.path.exists(self.dataDir): 
            shutil.rmtree(self.dataDir)

        os.mkdir(self.dataDir)

        ## handle the POINTING and SYSCAL tables ##
        # Will not do anything here. They will be handled in
        # postExecution()
        self.ptab = self._arg['vis']+'/POINTING'
        self.stab = self._arg['vis']+'/SYSCAL'

        mytb = tbtool()

        # test their properties
        self.pointingisempty = True
        self.makepointinglinks = False
        self.pwriteaccess = True

        mytb.open(self.ptab)
        self.pointingisempty = (mytb.nrows()==0)
        mytb.close()
        self.makepointinglinks = not self.pointingisempty
        self.pwriteaccess = True
        
        self.syscalisempty = True
        self.makesyscallinks = False
        self.swriteaccess = True
        if(os.path.exists(self.stab)): # syscal is optional
            mytb.open(self.stab)
            self.syscalisempty = (mytb.nrows()==0)
            mytb.close()
            self.makesyscallinks = not self.syscalisempty

        # TODO: Review the pointing table later. I don't want to
        # modify the input MS!! If the task crashes in the middle,
        # the input MS will be in a corrupted state!!!
        
#        if not self.pointingisempty:
#            if os.access(os.path.dirname(self.ptab), os.W_OK) \
#                   and not os.path.islink(self.ptab):
#                # move to datadir
#                os.system('mv '+self.ptab+' '+self.dataDir)
#                # create empty copy in original place so partition does not need to deal with it
#                mytb.open(self.dataDir+'/POINTING')
#                tmpp = mytb.copy(newtablename=self.ptab, norows=True)
#                tmpp.close()
#                mytb.close()
#            else:
#                self.pwriteaccess = False
#                
#
#        if not self.syscalisempty:
#            if os.access(os.path.dirname(self.stab), os.W_OK) \
#                   and not os.path.islink(self.stab):
#                # move to datadir
#                os.system('mv '+self.stab+' '+self.dataDir)
#                # create empty copy in original place so partition does not need to deal with it
#                mytb.open(self.dataDir+'/SYSCAL')
#                tmpp = mytb.copy(newtablename=self.stab, norows=True)
#                tmpp.close()
#                mytb.close()
#            else:
#                self.swriteaccess = False
            
    
#    @dump_args
    def makeInputLists(self):
        '''Make data selection parameters lists if
        they are not, when a list of input MSs is requested'''
        
        # input is a list of MSs
        if (isinstance(self.__origpars['vis'], list)):
            lsize = self.__origpars['vis'].__len__()
            # Check data selection parameters
            for k,v in self.__selpars.items():
                if not isinstance(v, list):                    
                    # make it a list
                    listvalue = []
                    for i in range(lsize):
                        listvalue.append(v)
                        self.__selpars[k] = listvalue
                        
        return self.__selpars
     
#    @dump_args
    def generateJobs(self):
        '''This is the method which generates all of the actual jobs to be done.'''
        '''This method overrides the one in ParallelTaskHelper baseclass'''
        # How about when the input is a list of MMSs? What to do? Nothing!
        
        if self._arg['outputvis'] != '':
            casalog.post("Analyzing MS for partitioning")
            self._createPrimarySplitCommand()
        
        print self._msTool.name()
                 
        return True
     
    @dump_args
    def _createPrimarySplitCommand(self):         
                
        if self._arg['createmms']:
            if self._arg['separationaxis'].lower() == 'scan':
                self._createScanSeparationCommands()
            elif self._arg['separationaxis'].lower() == 'spw':
                self._createSPWSeparationCommands()
            elif self._arg['separationaxis'].lower() == 'both':
                self._createDefaultSeparationCommands()
        else:
            # Single mms case
            singleCmd = copy.copy(self._arg)
            if scanList is not None:
                singleCmd['scan'] = ParallelTaskHelper.\
                                    listToCasaString(scanList)
            self._executionList.append(
                simple_cluster.JobData(self._taskName, singleCmd))
            
#    @dump_args
    def _createScanSeparationCommands(self):
        scanList = self._selectionScanList
        if scanList is None:
            self._selectMS()
            scanList = self._getScanList()

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
    def _createSPWSeparationCommands(self):
        # This method is to generate a list of commands to partition
        # the data based on SPW.
        self._selectMS()
        spwList = self._getSPWList()
        numSubMS = self._arg['numsubms']
        numSubMS = min(len(spwList),numSubMS)

        partitionedSPWs = self.__partition(spwList,numSubMS)
                
        self.__ddistart = 0
        for output in xrange(numSubMS):
            mmsCmd = copy.copy(self._arg)
            mmsCmd['createmms'] = False
            if self._selectionScanList is not None:
                mmsCmd['scan'] = ParallelTaskHelper.\
                                 listToCasaString(self._selectionScanList)
            mmsCmd['spw'] = ParallelTaskHelper.\
                            listToCasaString(partitionedSPWs[output])
            mmsCmd['ddistart'] = self.__ddistart
            mmsCmd['outputvis'] = self.dataDir+'/%s.%04d.ms' \
                                  % (self.outputBase, output)
            self._executionList.append(
                simple_cluster.JobData(self._taskName, mmsCmd))
#            if self.__selectionFilter != None:
            self.__ddistart = self.__ddistart + partitionedSPWs[output].__len__()

        
        
#    @dump_args
    def _createDefaultSeparationCommands(self):
        # This method is similar to the SPW Separation mode above, except
        # that if there are not enough SPW to satisfy the numSubMS it uses
        #
        self._selectMS()
            
        # Get the list of spectral windows
        spwList = self._getSPWList() 

        # Check if we can just divide on SPW or if we need to do SPW and
        # scan
        numSubMS = self._arg['numsubms']
        numSpwPartitions = min(len(spwList),numSubMS)
        numScanPartitions = int(math.ceil(numSubMS/float(numSpwPartitions)))

        if numScanPartitions > 1:
            # Check that the scanlist is not null
            scanList = self._selectionScanList
            if scanList is None:
                scanList = self._getScanList()

            # Check that the number of scans is enough for the partitions
            if len(scanList) < numScanPartitions:
                numScanPartitions = len(scanList)
        else:
            scanList = None

        partitionedSpws  = self.__partition(spwList,numSpwPartitions)
        partitionedScans = self.__partition(scanList,numScanPartitions)

        self.__ddistart = 0
        for output in xrange(numSpwPartitions*numScanPartitions):
            mmsCmd = copy.copy(self._arg)
            mmsCmd['createmms'] = False
            
            mmsCmd['scan'] = ParallelTaskHelper.listToCasaString \
                             (partitionedScans[output%numScanPartitions])
            mmsCmd['spw'] = ParallelTaskHelper.listToCasaString\
                            (partitionedSpws[output/numScanPartitions])
            mmsCmd['ddistart'] = self.__ddistart
            mmsCmd['outputvis'] = self.dataDir+'/%s.%04d.ms' \
                                  % (self.outputBase, output)
            self._executionList.append(
                simple_cluster.JobData(self._taskName, mmsCmd))
            
            self.__ddistart = self.__ddistart + partitionedSpws[output/numScanPartitions].__len__()


#    @dump_args
    def _getDDIstart(self):
        '''Return the DDIstart internal parameter'''
        
        return self.__ddistart

#    @dump_args
    def _selectMS(self, doCalibrationSelection = False):
        '''
        This method will open the MS and ensure whatever selection criteria
        have been requested are honored.
        If scanList is not None then it used as the scan selection criteria.
        '''
        print "Start of Select MS"
        if self._msTool is None:
            # Open up the msTool
            self._msTool = mstool()
            self._msTool.open(self._arg['vis'])    
        else:
            self._msTool.reset()
            
        # It returns a dictionary if there was any selection otherwise None
        self.__selectionFilter = self._getSelectionFilter()
#        if not doCalibrationSelection and self._calScanList is not None:
#            # We need to augment the selection to remove cal scans
#            if self._selectionScanList is None:
#                # Generate the selection scan list if needed
#                if selectionFilter is not None:
#                    self._msTool.msselect(selectionFilter)
#                self._selectionScanList = self._getScanList()
#                
#                for scan in self._calScanList:
#                    self._selectionScanList.remove(scan)
#
#            # Augment the selection
#            if selectionFilter is None:
#                selectionFilter = {}
#            selectionFilter['scan'] = ParallelTaskHelper.listToCasaString\
#                                      (self._selectionScanList)

        if self.__selectionFilter is not None:
            print self.__selectionFilter
            self._msTool.msselect(self.__selectionFilter)

        # This is not necessary
#        if doCalibrationSelection:
#            calFilter = self._getCalibrationFilter()
#            self._msTool.msselect(calFilter)

#    @dump_args
    def _getScanList(self):
        '''
        This method returns the scan list from the current ms.  Be careful
        about having selection already done when you call this.
        '''
        if self._msTool is None:
            self._selectMS()

        scanSummary = self._msTool.getscansummary()
        scanList = [int(scan) for scan in scanSummary]

        if len(scanList) == 0:
            raise ValueError, "No Scans present in the created MS."

        return scanList

#    @dump_args
    def _getSPWList(self):
        '''
        This method returns the spectral window list from the current
        MS.  Be careful about having selection already done when you call this.
        '''
        if self._msTool is None:
            self._selectMS()
        
        # Now get the list of SPWs in the selected MS
        ddInfo = self._msTool.getspectralwindowinfo()
        spwList = [info['SpectralWindowId'] for info in ddInfo.values()]

        # Return a unique sorted list:
#        return list(set(spwList))

        # Return a unique sorted list:
        # Note: the above did not return a sorted list
        sorted = list(set(spwList))
        sorted.sort()
        return sorted

#    @dump_args
    def _getSelectionFilter(self):
        ''' This method takes the list of specified selection criteria and
            puts them into a dictionary.  There is a bit of name mangling necessary.
            The pairs are: (msselection syntax, split task syntanc)'''
        
        selectionPairs = []
        selectionPairs.append(('field','field'))
        selectionPairs.append(('spw','spw'))
        selectionPairs.append(('baseline','antenna'))
        selectionPairs.append(('time','timerange'))
        selectionPairs.append(('scan','scan'))
        selectionPairs.append(('uvdist','uvrange'))
        selectionPairs.append(('scanintent','intent'))
        selectionPairs.append(('observation','observation'))
        return self.__generateFilter(selectionPairs)

#    @dump_args
    def __generateFilter(self, selectionPairs):
        filter = None
        for (selSyntax, argSyntax) in selectionPairs:
            if self._arg[argSyntax] != '':
                if filter is None:
                    filter = {}
                filter[selSyntax] = self._arg[argSyntax]
        return filter

#    @dump_args
    def __partition(self, lst, n):
        '''
        This method will split the list lst into "n" almost equal parts
        if lst is none, then we assume an empty list
        '''
        if lst is None:
            lst = []
        
        division = len(lst)/float(n)
        
        return [ lst[int(round(division * i)):
                     int(round(division * (i+1)))] for i in xrange(int(n))]

#    @dump_args
    def postExecution(self):
        '''
        This overrides the post execution portion of the task helper
        in this case we probably need to generate the output reference
        ms.
        '''
        if self._arg['createmms']:
            casalog.post("Finalizing MMS structure")
            print self._msTool.name()

            # TODO: revise this later                        
            # restore POINTING and SYSCAL
#            if self.pwriteaccess and not self.pointingisempty:
#                print "restoring POINTING"
#                os.system('rm -rf '+self.ptab) # remove empty copy
#                os.system('mv '+self.dataDir+'/POINTING '+self.ptab)
#            if self.swriteaccess and not self.syscalisempty:
#                print "restoring SYSCAL"
#                os.system('rm -rf '+self.stab) # remove empty copy
#                os.system('mv '+self.dataDir+'/SYSCAL '+self.stab)
            
            # jagonzal (CAS-4287): Add a cluster-less mode to by-pass parallel 
            # processing for MMSs as requested 
            if (ParallelTaskHelper.getBypassParallelProcessing()==1):
                outputList = self._sequential_return_list
                self._sequential_return_list = {}
            else:
                outputList = self._jobQueue.getOutputJobs()
            # We created a data directory and many SubMSs,
            # now build the reference MS
            
            subMSList = []
            if (ParallelTaskHelper.getBypassParallelProcessing()==1):
                for subMS in outputList:
                    subMSList.append(subMS)
            else:
                for job in outputList:
                    if job.status == 'done':
                        subMSList.append(job.getCommandArguments()['outputvis'])
                        
            subMSList.sort()
            print subMSList

            if len(subMSList) == 0:
                casalog.post("Error: no subMSs were created.", 'WARN')
                return False

            mastersubms = subMSList[0]
#            self._msTool.close()
#            self._msTool.open(mastersubms)
#            scansumm = self._msTool.getscansummary()
#            print scansumm
#            self._msTool.close()
            subtabs_to_omit = []

            # deal with POINTING table
            if not self.pointingisempty:
                print '******Dealing with POINTING table'
                shutil.rmtree(mastersubms+'/POINTING', ignore_errors=True)
                # master subms gets a full copy of the original
                shutil.copytree(self.ptab, mastersubms+'/POINTING') 
            if self.makepointinglinks:
                for i in xrange(1,len(subMSList)):
                    theptab = subMSList[i]+'/POINTING'
                    shutil.rmtree(theptab, ignore_errors=True)
                    os.symlink('../'+os.path.basename(mastersubms)+'/POINTING', theptab)
                    # (link in target will be created by makeMMS)
                subtabs_to_omit.append('POINTING')

#            # deal with SYSCAL table
            if not self.syscalisempty:
                print '******Dealing with SYSCAL table'
                shutil.rmtree(mastersubms+'/SYSCAL', ignore_errors=True)
                # master subms gets a full copy of the original
                shutil.copytree(self.stab, mastersubms+'/SYSCAL') 
            if self.makesyscallinks:
                for i in xrange(1,len(subMSList)):
                    thestab = subMSList[i]+'/SYSCAL'
                    shutil.rmtree(thestab, ignore_errors=True)
                    os.symlink('../'+os.path.basename(mastersubms)+'/SYSCAL', thestab)
                    # (link in target will be created my makeMMS)
                subtabs_to_omit.append('SYSCAL')
                
            ph.makeMMS(self._arg['outputvis'], subMSList,
                       True, # copy subtables
                       subtabs_to_omit # omitting these
                       )

            thesubmscontainingdir = os.path.dirname(subMSList[0].rstrip('/'))
            
            os.rmdir(thesubmscontainingdir)

        return True
        
#    @dump_args 
    def freqAvg(self, **pars):
        ''' Get the sub-parameters for freqaverage=True
            which are freqbin and useweights'''
        
        for k,v in pars.items():
            if k == "freqbin":
#                if isinstance(v, list):
#                    self.
                fb = v 
            elif k == "useweights":
                wght = v
        
        # Validate the parameters

#    @dump_args
    def defaultRegridParams(self):
        '''Reset the default values of the regridms transformation parameters based on the mode'''
        
        if self.__args['mode'] == 'channel' or self.__args['mode'] == 'channel_b':
            self.__args['start'] = str(self.__args['start'])
            self.__args['width'] = str(self.__args['width'])
            
        elif self.__args['mode'] == 'velocity':
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
           
#@dump_args
def mstransform(
             vis, 
             outputvis,           # output
             createmms,           # MMS --> partition
             separationaxis, 
             numsubms,
             ddistart,           # THIS PARAMETER WILL BE HIDDEN LATER
             tileshape,          # tiling
             field,
             spw, 
             scan, 
             antenna, 
             correlation,
             timerange, 
             intent,
             array,
             uvrange,
             observation,
             feed,
             datacolumn,
             realmodelcol,
             combinespws,        # spw combination --> cvel
             freqaverage,        # frequency averaging --> split
             freqbin,
             useweights,
             hanning,            # Hanning --> cvel
             regridms,           # regridding to new frame --> cvel
             mode, 
             nchan, 
             start, 
             width, 
             nspw,               # spw separation
             interpolation,
             phasecenter,
             restfreq, 
             outframe, 
             veltype,
             timeaverage,        # time averaging --> split
             timebin,
             timespan,
             quantize_c,
             minbaselines              
             ):

    ''' This task can replace applycal, concat, cvel, partition and split '''
    
    casalog.origin('mstransform')
       
    # Initialize the helper class
    mth = MSTHelper(locals())
    
    # Validate input and output parameters
    mth.setupIO()
    

    # Process in parallel
    # LATER:
    createmms = False
    if createmms:
        
        # Validate the combination of some parameters
        # pval = 0 -> abort
        # pval = 1 -> run in sequential
        # pval = 2 -> run in parallel
        pval = mth.validateParams()
        if pval == 0:
            raise Exception, 'Cannot partition using separationaxis=%s with some of the requested transformations.'\
                            %separationaxis
                            
        # Process in parallel
        if pval == 2:
            casalog.post('Will process the input list in parallel')
            # Setup a dictionary of the selection parameters
            mth.setupParameters(field=field, spw=spw, array=array, scan=scan, correlation=correlation,
                                antenna=antenna, uvrange=uvrange, timerange=timerange, 
                                intent=intent, observation=str(observation),feed=feed)
            

            # Get a cluster
            mth.bypassParallelProcessing(1)
            mth.setupCluster()
            
            # Do the processing. 
            retval = mth.go()
            
            return retval

        
    # Create a local copy of the MSTransform tool
    mtlocal = casac.mstransformer()
        
    try:
                    
        # Gather all the parameters in a dictionary.
        
        config = {}
        config = mth.setupParameters(inputms=vis, outputms=outputvis, field=field, 
                    spw=spw, array=array, scan=scan, antenna=antenna, correlation=correlation,
                    uvrange=uvrange,time=timerange, intent=intent, observation=str(observation),
                    feed=feed)
        
        # Write the DDI start for each sub-MS, when creating an MMS
#        config['ddistart'] = mth._getDDIstart()
#        print 'Will add ddistart to config'
#        print config['ddistart']
        config['datacolumn'] = datacolumn
        config['realmodelcol'] = realmodelcol

        if combinespws:
            casalog.post('Combine spws %s into new output spw'%spw)
            config['combinespws'] = True
        if freqaverage:
            casalog.post('Parse frequency averaging parameters')
            config['freqaverage'] = True
            # freqbin can be an int or a list of int that will apply one to each spw
            config['freqbin'] = freqbin
            config['useweights'] = useweights
        if hanning:
            casalog.post('Apply Hanning smoothing')
            config['hanning'] = True
        if regridms:
            casalog.post('Parse regridding parameters')
            
            config['regridms'] = True
            # Reset the defaults depending on the mode
            # Only add non-empty string parameters to config dictionary
            start, width = mth.defaultRegridParams()
            config['mode'] = mode
            config['nchan'] = nchan
#            if start != '':
            config['start'] = start
#            if width != '':
            config['width'] = width
            if nspw > 1:
                casalog.post('Separate MS into %s spws'%nspw)

            config['nspw'] = nspw
            config['interpolation'] = interpolation
#            if restfreq != '':
            config['restfreq'] = restfreq
#            if outframe != '':
            config['outframe'] = outframe
            config['veltype'] = veltype
        if timeaverage:
            casalog.post('Time averaging is not yet implemented', 'WARN')
#            config['timebin'] = timebin
#            config['timespan'] = timespan
#            config['quantize_c'] = quantize_c
#            config['minbaselines'] = minbaselines
        
        # Configure the tool and all the parameters
        
        casalog.post('%s'%config, 'DEBUG')
        mtlocal.config(config)
        
        # Open the MS, select the data and configure the output
        mtlocal.open()
        
        # Run the tool
        casalog.post('Apply the transformations')
        mtlocal.run()        
            
        mtlocal.done()
                    
    except Exception, instance:
        mtlocal.done()
        raise Exception, instance
    
    # Write history 
#    try:
#        mslocal = mstool()
#        mslocal.open(vis, nomodify=False)
#        mslocal.writehistory(message='taskname = mstransform', origin='mstransform')
#        param_names = mstransform.func_code.co_varnames[:mstransform.func_code.co_argcount]
#        param_vals = [eval(p) for p in param_names]
#        rval = None
#        rval &= write_history(mslocal, vis, 'mstransform', param_names,
#                               param_vals, casalog)
#       
#        mslocal.close()
#       
#    except Exception, instance:
#        mslocal.close()
#        casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
#                        'WARN')

    return True

# TODO:
# 1) allow freqbin to be a list to apply to each spw selection. DONE
# 2) Modify the separatespws transformation. DONE
# 3) Check realmodelcol and tileshape parameters
# 4) Parallelism
# 5) Check if partition can run on MMS. YES, DONE
# 6) Review the pointing table move!
# 7) Partition does not re-index the spw table, while split does it.
#    This needs to be solved. Maybe the tool needs to get a dictionary
#    that contains the informations: createmms=True, and a list of the
#    outputvis names (sub-Mss) associated with the spwIDs that will go
#    to each sub-MS.
#   
# HEURISTICS for the parallelization
#
# WE only process in parallel when:
#    1) there are multiple input MSs and createmms = True
#
# If there are multiple input MSs and createmms=False,
# loop through the input MS list and call the tool
# multiple times. We need to have an internal parameter
# to tell the DataHandler that there is more than one
# MS to be concatenated as the output.

# Parallel heuristics:
# 1) Do the data selection based on the partition parameters;
#    separationaxis and numsubms! DONE!

# Some constraints will have to be applied depending on the combination given 
# for separationaxis and the requested transformations. See the table below, 
# which shows with which transformation the separation axis can run.
# Catch these cases inside MSTHelper.

#separation   combinespws  separatespws  regridms  freqaverage  timespan  hanning
#axis
#---------------------------------------------------------------------------------------
#spw            NO           YES*         YES        YES          YES       YES
#scan           YES          YES          YES        YES          YES**     YES
#both           NO           YES*         YES        YES          NO        YES

# 2) Differences between partition and mstransform/split bahaviors:
#  Partition: when spw selection is 4,5,6,7 and numsubms=4,
#            each subms will have a spw that is not re-indexed; meaning
#            DDI in main table are 4,5,6,7 and spw table has all spws 0~15.

#  Split: it re-index all DDIs to 0,1,2,3 and splits the spw subtable to 0~3 rows.

#  in mstransform we want:
#  DDIs should be re-index when there is an spw selection, but consolidated
#  in the final MMS structure such that each subMS will have a different DDI (0,1,2,3)
#  and the spw subtable should have only the spws selected, not all of the original ones.
#  This should be done in the _createSPWSeparationCommand method. Add a new internal
#  parameter called DDIstart with the start DDI for each subMS.
#
# 2) For this, use the methods from PartitionHelper to create the
#    jobs. 
# 3) inside generateJobs(): loop through each MS in vis list.
# 4) call self.initialize to setup the outputvis name and handle
#    POINTING, SYSCAL. The creation of the dataDir should be done outside,
#    perhaps in generateJobs().
# 5) Call self._createPrimarySplitCommand() for each MS in the loop
# 6) This should add one job per subMS to be handled by the tool.



    
    
    
    
    
    
    
    
    
    
    
    
    
    
