
import os, re
import shutil
import string
import copy
import math
from taskinit import *
from parallel.parallel_task_helper import ParallelTaskHelper
import simple_cluster
import partitionhelper as ph
from update_spw import update_spwchan


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
        self.taskname = None
        self.__ddistart = None
        self._msTool = None
        self._tbTool = None
        
        if not self.__args.has_key('spw'):
            self.__args['spw'] = ''
            
        self.__spwSelection = self.__args['spw']
        self.__spwList = None
        # __ddidict contains the names of the subMSs to consolidate
        # the keys are the ddistart and the values the subMSs names
        self.__ddidict = {}
        
        # start parameter for DDI in main table of each sub-MS
        # It should be a counter of spw IDs starting at 0
        if self.__origpars.has_key('ddistart'):
              self.__ddistart = self.__origpars['ddistart']


    def setTaskName(self, thistask=''):
        '''Setup this run with the task that called self.setupCluster(taskname)'''
        self.taskname = thistask
        
#    @dump_args
    def setupIO(self):
        '''Validate input and output parameters'''
        
        for k,v in self.__args.items():
            if k == 'vis' and isinstance(v, str):
                # only one input MS
                if not os.path.exists(v):
                    raise Exception, 'Visibility data set not found - please verify the name.'
                
            elif k == 'outputvis' and isinstance(v, str):
                # only one output MS
                if v.isspace() or v.__len__() == 0:
                    raise ValueError, 'Please specify outputvis.'
                elif os.path.exists(v):
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
                self.__args['timespan'] = ''
                retval = 1
            # split2 parameter
            elif self.__args.has_key('combine') and self.__args['combine'] == 'scan':
                casalog.post('Cannot partition MS in scan when timebin span across scans.\n'\
                              'Will reset combine to None', 'WARN')
                self.__args['combine'] = ''
                retval = 1
                
        elif (self.__args['separationaxis'] == 'both'):
            if self.__args['combinespws'] == True:
                casalog.post('Cannot partition MS in spw,scan axes when combinespws = True', 'WARN')
                retval = 0

            elif self.__args['timespan'] == 'scan':
                casalog.post('Cannot partition MS in scan when timebin span across scans.\n'\
                              'Will reset timespan to None', 'WARN')
                self.__args['timespan'] = ''
                retval = 1
            # split2 parameter
            elif self.__args.has_key('combine') and self.__args['combine'] == 'scan':
                casalog.post('Cannot partition MS in scan when timebin span across scans.\n'\
                              'Will reset combine to None', 'WARN')
                self.__args['combine'] = ''
                retval = 1
                
            elif self.__args['nspw'] > 1:
                # CANNOT process in sequential either because internally the
                # spws need to be combined first before the separation!!!!!!
#                casalog.post('Can only process the MS in sequential', 'WARN')
                casalog.post('Cannot partition MS in spw axis and separate spws (nspw > 1)', 'WARN')
                retval = 0

        return retval
        
#    @dump_args
    def setupCluster(self, thistask=''):
        '''Get a simple_cluster'''
        if thistask == '':
            thistask = 'mstransform'
            
        # It needs to use the updated list of parameters!!!
#        ParallelTaskHelper.__init__(self, task_name=thistask, args=self.__origpars)
        ParallelTaskHelper.__init__(self, task_name=thistask, args=self.__args)
        self.setTaskName(thistask)            
    
#    @dump_args
    def setupParameters(self, **pars):
        '''Create a dictionary with non-empty parameters'''
        
        seldict = {}
        for k,v in pars.items():
            if v != None and v != "":
                self.__selpars[k] = v
        
        return self.__selpars
    
#    @dump_args
    def validateModelCol(self):
        '''Add the realmodelcol parameter to the configuration
           only for some values of datacolumn'''
        
        ret = False
        
        dc = self.__args['datacolumn'].upper()
        if dc=='MODEL' or dc=='ALL' or dc=='DATA,MODEL,CORRECTED':
            ret = True

        return ret
    
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

        if self.outputBase == '.' or self.outputBase == './':
            raise ValueError, 'Error dealing with outputvis'
        
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
    def generateJobs(self):
        '''This is the method which generates all of the actual jobs to be done.'''
        '''This method overrides the one in ParallelTaskHelper baseclass'''
        
        if self._arg['createmms']:
            if self._arg['outputvis'] != '':
                casalog.post("Analyzing MS for partitioning")
                self._createPrimarySplitCommand()
                                      
        return True
     
#    @dump_args
    def _createPrimarySplitCommand(self):         
                
        if self._arg['createmms']:
            if self._arg['separationaxis'].lower() == 'scan':
                self._createScanSeparationCommands()
            elif self._arg['separationaxis'].lower() == 'spw':
                self._createSPWSeparationCommands()
            elif self._arg['separationaxis'].lower() == 'both':
                self._createDefaultSeparationCommands()
        else:
            # TODO: REVIEW this later. ScanList does not exist!
            # Single mms case
            singleCmd = copy.copy(self._arg)
            scanList = self._selectionScanList
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

        # Get a unique list of selected spws        
        self._selectMS()
        spwList = self._getSPWUniqueList()
        numSubMS = self._arg['numsubms']
        numSubMS = min(len(spwList),numSubMS)

        # Get a dictionary of the spws parted for each subMS
        spwList = map(str,spwList)
        partitionedSPWs1 = self.__partition1(spwList,numSubMS)

        # Add the channel selections back to the spw expressions
        newspwsel = self.createSPWExpression(partitionedSPWs1)
        
        # Validate the chanbin parameter
        validbin = False
        if self.__origpars.has_key('chanaverage'):
            if self.__origpars['chanaverage'] and self.validateChanBin():
                # Partition chanbin in the same way
                freqbinlist = self.__partition1(self.__origpars['chanbin'],numSubMS)
                validbin = True
        
        # Calculate the ddistart for each engine. This will be used
        # to calculate the DD IDs of the output main table of the subMSs
        ddistartlist = self._calculateDDIstart({}, partitionedSPWs1)
        if (len(ddistartlist) != len(partitionedSPWs1)):
            casalog.post('Error calculating the ddistart indices','SEVERE')
            raise
        
        for output in xrange(numSubMS):
            mmsCmd = copy.copy(self._arg)
            mmsCmd['createmms'] = False
            if self._selectionScanList is not None:
                mmsCmd['scan'] = ParallelTaskHelper.\
                                 listToCasaString(self._selectionScanList)            
            mmsCmd['spw'] = newspwsel[output]
            if validbin:
                mmsCmd['chanbin'] = freqbinlist[output]
                
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
    def _createDefaultSeparationCommands(self):
        
        casalog.post('Partition per scan/spw will ignore NULL combinations of these two parameters.')

        # Separates in scan and spw axes
        self._selectMS()
            
        # Get the list of spectral windows as strings
        spwList = self._getSPWUniqueList() 
        spwList = map(str,spwList)

        # Check if we can just divide on SPW or if we need to do SPW and scan
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

        partitionedSpws  = self.__partition1(spwList,numSpwPartitions)
        partitionedScans = self.__partition(scanList,numScanPartitions)
        
        # The same list but as a dictionary
        str_partitionedScans = self.__partition1(scanList,numScanPartitions)

        # Validate the chanbin parameter when it is a list
        validbin = False
        if self.__origpars.has_key('chanaverage'):
            if self.__origpars['chanaverage'] and self.validateChanBin():
                # Partition chanbin in the same way
                freqbinlist = self.__partition1(self.__origpars['chanbin'],numSpwPartitions)
                validbin = True

        # Add the channel selections back to the spw expressions
        newspwsel = self.createSPWExpression(partitionedSpws)
        
        # Calculate the ddistart for the subMSs (for each engine)
        ddistartlist = self._calculateDDIstart(str_partitionedScans, partitionedSpws)
                
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
            if not self._scanspwSelection(selscan,
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
                mmsCmd['chanbin'] = freqbinlist[output/numScanPartitions]
            mmsCmd['ddistart'] = self.__ddistart
            mmsCmd['outputvis'] = self.dataDir+'/%s.%04d.ms' \
                                  % (self.outputBase, sindex)
            self._executionList.append(
                simple_cluster.JobData(self._taskName, mmsCmd))
            
            sindex += 1 # index of subMS name

    def _scanspwSelection(self, scan, spw):
        '''Return True if the selection is True or False otherwise'''
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
    def _calculateDDIstart(self, partedscans, partedspws):
        '''Return list with DDI values for each partition (each engine).
           It will return a list of ddistart values with the same size of 
           the number of subMSs.
        '''
            
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
    def _selectMS(self, doCalibrationSelection = False):
        '''
        This method will open the MS and ensure whatever selection criteria
        have been requested are honored.
        If scanList is not None then it used as the scan selection criteria.
        '''
        if self._msTool is None:
            # Open up the msTool
            self._msTool = mstool()
            self._msTool.open(self._arg['vis'])    
        else:
            self._msTool.reset()
            
        # It returns a dictionary if there was any selection otherwise None
        self.__selectionFilter = self._getSelectionFilter()

        if self.__selectionFilter is not None:
            self._msTool.msselect(self.__selectionFilter)


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

        scanList.sort()
        return scanList

#    @dump_args
    def _getSPWUniqueList(self):
        '''
        This method returns a unique list of spectral windows from the current
        MS.  Be careful about having selection already done when you call this.
        '''
        if self._msTool is None:
            self._selectMS()
        
        # Now get the list of SPWs in the selected MS
        ddInfo = self._msTool.getspectralwindowinfo()
#        spwList = [info['SpectralWindowId'] for info in ddInfo.values()]
        self.__spwList = [info['SpectralWindowId'] for info in ddInfo.values()]

        # Return a unique sorted list:
        sorted = list(set(self.__spwList))
#        sorted.sort()
        return sorted

#    @dump_args
    def _getSelectionFilter(self):
        ''' This method takes the list of specified selection criteria and
            puts them into a dictionary.  There is a bit of name mangling necessary.
            The pairs are: (msselection syntax, split task syntanc)'''
        
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
    def __partition1(self, lst, n):
        '''This method will split the list lst into "n" almost equal parts
            if lst is none, then we assume an empty list.
            lst --> spw list
            n   --> numsubms
            Returns a dictionary such as:
            given the selection spw='0,1:10~20,3,4,5'
            rdict = {0: ['0','1'], 1:['3','4','5']}
        '''
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
        ''' Create a dictionary of channel selections.
            spwsel --> a string with spw selection
            Return a dictionary such as:
            spwsel = "'0,1:10~20"
            seldict = {0: {'channels': '', 'spw': '0'}, 
                       1: {'channels': '10~20', 'spw': '1'}}'''
        
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
    def createSPWExpression(self, partdict):
        ''' Creates the final spw expression that will be sent to the engines.
            This adds back the channel selections to their spw counterparts.
           partdict --> dictionary from __partition1, such as:
                        Ex: partdict = {0: ['0','1'], 1:['3','4','5']}
                            when selection is spw = '0,1:10~20,3,4,5'
                            and effective number of subMSs is 2.'''            
        
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
        
#    @dump_args 
    def validateChanBin(self):
        '''When chanbin is a list, it should have the
           same size as the number of spws in selection.'''
        
        retval = True
        if self.__origpars.has_key('chanaverage'):
            fblist = self.__origpars['chanbin']
            if isinstance(fblist,list) and fblist.__len__() > 1: 
                if self.__spwList == None:           
                    msTool = mstool()
                    msTool.open(self.__origpars['vis'])
                    spwsel = self.__origpars['spw'] 
                    msTool.msselect({'spw':spwsel})
                    ddInfo = msTool.getspectralwindowinfo()
                    self.__spwList = [info['SpectralWindowId'] for info in ddInfo.values()]
                    msTool.close()
                        
                if self.__spwList.__len__() != fblist.__len__():
                    retval = False
    #                casalog.post('Number of chanbin is different of number of spw','ERROR')
                    raise ValueError, 'Number of chanbin is different of number of spw'                
                    
            else:
                retval = False

        return retval
    
#    @dump_args
    def defaultRegridParams(self):
        '''Reset the default values of the regridms transformation parameters based on the mode'''
        
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
        '''
        This method overrides the postExecution method of ParallelTaskHelper
        in this case we probably need to generate the output reference
        ms.
        '''
        if self._arg['createmms']:
            casalog.post("Finalizing MMS structure")
            
            if self._msTool:
                self._msTool.close()

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

            if len(subMSList) == 0:
                casalog.post("Error: no subMSs were created.", 'WARN')
                return False
            
            # When separationaxis='scan' there is no need to give ddistart. 
            # The tool looks at the whole spw selection and
            # creates the indices from it. After the indices are worked out, 
            # it applys MS selection. We do not need to consolidate either.
                                           
            # If axis is spw or both, give a list of the subMSs
            # that need to be consolidated. This list is pre-organized
            # inside the separation functions above.
            if (self.__origpars['separationaxis'] == 'spw' or 
                self.__origpars['separationaxis'] == 'both'):                
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
                  
            # Get the first subMS to be as a reference when
            # copying the sub-tables to the other subMSs  
            mastersubms = subMSList[0]
            subtabs_to_omit = []

            # deal with POINTING table
            if not self.pointingisempty:
#                print '******Dealing with POINTING table'
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

            # deal with SYSCAL table
            if not self.syscalisempty:
#                print '******Dealing with SYSCAL table'
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
                
            # Copy sub-tables from first subMS to the others.
            ph.makeMMS(self._arg['outputvis'], subMSList,
                       True, # copy subtables
                       subtabs_to_omit # omitting these
                       )

            thesubmscontainingdir = os.path.dirname(subMSList[0].rstrip('/'))
            
            os.rmdir(thesubmscontainingdir)

        return True

          
#@dump_args
def mstransform(
             vis, 
             outputvis,           # output
             createmms,           # MMS --> partition
             separationaxis, 
             numsubms,
             parallel,           # create MMS in parallel
             ddistart,           # internal parameter (hidden in XML)
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
             usewtspectrum,
             combinespws,        # spw combination --> cvel
             chanaverage,        # channel averaging --> split
             chanbin,
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
             maxuvwdistance
             ):

    ''' This task can replace split, cvel, partition and hanningsmooth '''
    
    casalog.origin('mstransform')
       
    # Initialize the helper class
    mth = MSTHelper(locals())
    
    # Validate input and output parameters
    try:
        mth.setupIO()
    except Exception, instance:
        casalog.post('%s'%instance,'ERROR')
        return False
    

    # Create an output multi-MS
    if createmms:
        
        # Validate the combination of some parameters
        # pval = 0 -> abort
        # pval = 2 -> run in parallel
        pval = mth.validateParams()
        if pval == 0:
            raise Exception, 'Cannot partition using separationaxis=%s with some of the requested transformations.'\
                            %separationaxis
                            
        # pval == 2, can process in parallel
        # Setup a dictionary of the selection parameters
        mth.setupParameters(field=field, spw=spw, array=array, scan=scan, correlation=correlation,
                            antenna=antenna, uvrange=uvrange, timerange=timerange, 
                            intent=intent, observation=str(observation),feed=feed)
        
        # The user decides to run in parallel or sequential
        if not parallel:
            casalog.post('Will process the MS in sequential')
            mth.bypassParallelProcessing(1)
        else:
            mth.bypassParallelProcessing(0)
            casalog.post('Will process the MS in parallel')

        # Get a cluster
        mth.setupCluster(thistask='mstransform')
        
        # Do the processing.
        try: 
            mth.go()
        except Exception, instance:
            casalog.post('%s'%instance,'ERROR')
            return False
        
        return True 

        
    # Create a local copy of the MSTransform tool
    mtlocal = casac.mstransformer()
    mslocal = mstool()
        
    try:
                    
        # Gather all the parameters in a dictionary.
        
        config = {}
        config = mth.setupParameters(inputms=vis, outputms=outputvis, field=field, 
                    spw=spw, array=array, scan=scan, antenna=antenna, correlation=correlation,
                    uvrange=uvrange,timerange=timerange, intent=intent, observation=str(observation),
                    feed=feed)
        
        # ddistart will be used in the tool when re-indexing the spw table
        config['ddistart'] = ddistart
        
        config['datacolumn'] = datacolumn
        if mth.validateModelCol():        
            # Make real a virtual MODEL column in the output MS
            config['realmodelcol'] = realmodelcol
        
        config['usewtspectrum'] = usewtspectrum
        
        # Add the tile shape parameter
        if tileshape.__len__() == 1:
            # The only allowed values are 0 or 1
            if tileshape[0] != 0 and tileshape[0] != 1:
                raise ValueError, 'When tileshape has one element, it should be either 0 or 1.'
                
        elif tileshape.__len__() != 3:
            # The 3 elements are: correlations, channels, rows
            raise ValueError, 'Parameter tileshape must have 1 or 3 elements.'
            
        config['tileshape'] = tileshape                

        if combinespws:
            casalog.post('Combine spws %s into new output spw'%spw)
            config['combinespws'] = True
            
        if chanaverage:
            casalog.post('Parse channel averaging parameters')
            config['chanaverage'] = True
            # chanbin can be an int or a list of int that will apply one to each spw
            mth.validateChanBin()
            config['chanbin'] = chanbin
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
            if start != '':
                config['start'] = start
            if width != '':
                config['width'] = width
            if nspw > 1:
                casalog.post('Separate MS into %s spws'%nspw)
            config['nspw'] = nspw
            config['interpolation'] = interpolation
            if restfreq != '':
                config['restfreq'] = restfreq
            if outframe != '':
                config['outframe'] = outframe
            if phasecenter != '':
                config['phasecenter'] = phasecenter
            config['veltype'] = veltype
            
        if timeaverage:
            casalog.post('Parse time averaging parameters')
            config['timeaverage'] = True
            config['timebin'] = timebin
            config['timespan'] = timespan
            config['maxuvwdistance'] = maxuvwdistance
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
        casalog.post('%s'%instance,'ERROR')
        return False


    # Update the FLAG_CMD sub-table to reflect any spw/channels selection
    if ((spw != '') and (spw != '*')) or chanaverage == True:
        isopen = False
        mytb = tbtool()
        try:
            mytb.open(outputvis + '/FLAG_CMD', nomodify=False)
            isopen = True
            nflgcmds = mytb.nrows()
            
            if nflgcmds > 0:
                mademod = False
                cmds = mytb.getcol('COMMAND')
                widths = {}
                #print "width =", width
                if hasattr(chanbin, 'has_key'):
                    widths = chanbin
                else:
                    if hasattr(chanbin, '__iter__') and len(chanbin) > 1:
                        for i in xrange(len(chanbin)):
                            widths[i] = chanbin[i]
                    elif chanbin != 1:
#                        print 'using ms.msseltoindex + a scalar width'
                        numspw = len(mslocal.msseltoindex(vis=vis,
                                                     spw='*')['spw'])
                        if hasattr(chanbin, '__iter__'):
                            w = chanbin[0]
                        else:
                            w = chanbin
                        for i in xrange(numspw):
                            widths[i] = w
#                print 'widths =', widths 
                for rownum in xrange(nflgcmds):
                    # Matches a bare number or a string quoted any way.
                    spwmatch = re.search(r'spw\s*=\s*(\S+)', cmds[rownum])
                    if spwmatch:
                        sch1 = spwmatch.groups()[0]
                        sch1 = re.sub(r"[\'\"]", '', sch1)  # Dequote
                        # Provide a default in case the split selection excludes
                        # cmds[rownum].  update_spwchan() will throw an exception
                        # in that case.
                        cmd = ''
                        try:
                            #print 'sch1 =', sch1
                            sch2 = update_spwchan(vis, spw, sch1, truncate=True,
                                                  widths=widths)
                            #print 'sch2 =', sch2
                            ##print 'spwmatch.group() =', spwmatch.group()
                            if sch2:
                                repl = ''
                                if sch2 != '*':
                                    repl = "spw='" + sch2 + "'"
                                cmd = cmds[rownum].replace(spwmatch.group(), repl)
                        #except: # cmd[rownum] no longer applies.
                        except Exception, e:
                            casalog.post(
                                "Error %s updating row %d of FLAG_CMD" % (e,
                                                                          rownum),
                                         'WARN')
                            casalog.post('sch1 = ' + sch1, 'DEBUG1')
                            casalog.post('cmd = ' + cmd, 'DEBUG1')
                        if cmd != cmds[rownum]:
                            mademod = True
                            cmds[rownum] = cmd
                if mademod:
                    casalog.post('Updating FLAG_CMD', 'INFO')
                    mytb.putcol('COMMAND', cmds)

            mytb.close()
            
        except Exception, instance:
            if isopen:
                mytb.close()
            mslocal = None
            mytb = None
            casalog.post("*** Error \'%s\' updating FLAG_CMD" % (instance),
                         'SEVERE')
            return False

    mytb = None

    # Write history to output MS, not the input ms.
    try:
        param_names = mstransform.func_code.co_varnames[:mstransform.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]
        write_history(mslocal, outputvis, 'mstransform', param_names,
                      param_vals, casalog)
    except Exception, instance:
        casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                     'WARN')
        return False

    mslocal = None
    
    return True
    
 
    
