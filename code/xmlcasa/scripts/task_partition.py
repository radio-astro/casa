import os
import string
import copy
import math
from taskinit import *
from parallel.parallel_task_helper import ParallelTaskHelper
import simple_cluster

class PartitionHelper(ParallelTaskHelper):
    def __init__(self, args = {}):
        ParallelTaskHelper.__init__(self,'partition', args)
        self._calScanList = None
        self._selectionScanList = None
        self._msTool = None

    def initialize(self):
        '''
        This method will prepare the system for working, in particular:
           * Open the input MS as a msTool
           * Create the Output Directory and the Data subdirectory
           * Populate the global data selection filter (if necessary)
        '''
        ParallelTaskHelper.initialize(self)

        for (arg,value) in self._arg.items():
            if value == None:
                self._arg[arg] = ''

                              
        # Since we are here we know that the outputvis should not be ''
        if self._arg['outputvis'] == '':
            raise ValueError, "An output vis was required."
        if os.path.exists(self._arg['outputvis']): 
                raise ValueError, \
                      "Vis directory for output (%s) already exists" %\
                      self._arg['outputvis']
            
        # Find the absolute path to the outputvis
        self._arg['outputvis'] = os.path.abspath(self._arg['outputvis'])
        outputPath, self.outputBase = os.path.split(self._arg['outputvis'])
        try:
            self.outputBase = self.outputBase[:self.outputBase.rindex('.')]
        except ValueError:
            # outputBase must not have a trailing .
            pass

        self.dataDir = outputPath + '/' + self.outputBase+'.data'
        if self._arg['createmms']:
            if os.path.exists(self.dataDir): 
                raise ValueError, \
                      "Data directory for output (%s) already exists" %\
                      self.dataDir

            os.mkdir(self.dataDir)

    def generateJobs(self):
        '''
        This method overrides the method in the TaskHelper baseclass
        '''
        if self._arg['calmsselection'] in ['auto','manual']:
            casalog.post("Analyzing MS for Calibration MS Creation")
            self._createCalMSCommand()

        if self._arg['outputvis'] != '':
            casalog.post("Analyzing MS for Partioning")
            self._createPrimarySplitCommand()

    def _createCalMSCommand(self):
        '''
        Create a command which will generate a MS with only those subMSs
        in the selected calibration
        '''
        self._selectMS(True)
        self._calScanList = self._getScanList()

        
        # Now create the command dict. for this
        calCmd = copy.copy(self._arg)
        calCmd['createmms'] = False
        calCmd['calmsselection'] = 'none'
        calCmd['scan'] = ParallelTaskHelper.listToCasaString(self._calScanList)
        if self._arg['createmms']:
            calCmd['outputvis'] = self.dataDir + '/%s.cal.ms' % self.outputBase
        else:
            calCmd['outputvis'] = self._arg['calmsname']
        self._executionList.append(
            simple_cluster.JobData(self._taskName, calCmd))

    def _createPrimarySplitCommand(self):            
        if self._arg['createmms']:
            if self._arg['separationaxis'].lower() == 'scan':
                self._createScanSeparationCommands()
            elif self._arg['separationaxis'].lower() == 'spw':
                self._createSPWSeparationCommands()
            elif self._arg['separationaxis'].lower() == 'default':
                self._createDefaultSeparationCommands()
        else:
            # Single mms case
            singleCmd = copy.copy(self._arg)
            singleCmd['calmsselection'] = 'none'
            if scanList is not None:
                singleCmd['scan'] = ParallelTaskHelper.\
                                    listToCasaString(scanList)
            self._executionList.append(
                simple_cluster.JobData(self._taskName, singleCmd))

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
            mmsCmd['calmsselection'] = 'none'            
            mmsCmd['scan']= ParallelTaskHelper.\
                            listToCasaString(partitionedScans[output])
            mmsCmd['outputvis'] = self.dataDir+'/%s.%04d.ms' \
                                  % (self.outputBase, output)
            self._executionList.append(
                simple_cluster.JobData(self._taskName, mmsCmd))
                
    def _createSPWSeparationCommands(self):
        # This method is to generate a list of commands to partition
        # the data based on SPW.
        self._selectMS()
        spwList = self._getSPWList()
        numSubMS = self._arg['numsubms']
        numSubMS = min(len(spwList),numSubMS)

        partitionedSPWs = self.__partition(spwList,numSubMS)
        for output in xrange(numSubMS):
            mmsCmd = copy.copy(self._arg)
            mmsCmd['createmms'] = False
            mmsCmd['calmsselection'] = 'none'
            if self._selectionScanList is not None:
                mmsCmd['scan'] = ParallelTaskHelper.\
                                 listToCasaString(self._selectionScanList)
            mmsCmd['spw'] = ParallelTaskHelper.\
                            listToCasaString(partitionedSPWs[output])
            mmsCmd['outputvis'] = self.dataDir+'/%s.%04d.ms' \
                                  % (self.outputBase, output)
            self._executionList.append(
                simple_cluster.JobData(self._taskName, mmsCmd))

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

        for output in xrange(numSpwPartitions*numScanPartitions):
            mmsCmd = copy.copy(self._arg)
            mmsCmd['createmms'] = False
            mmsCmd['calmsselection'] = 'none'

            
            mmsCmd['scan'] = ParallelTaskHelper.listToCasaString \
                             (partitionedScans[output%numScanPartitions])
            mmsCmd['spw'] = ParallelTaskHelper.listToCasaString\
                            (partitionedSpws[output/numScanPartitions])
            mmsCmd['outputvis'] = self.dataDir+'/%s.%04d.ms' \
                                  % (self.outputBase, output)
            self._executionList.append(
                simple_cluster.JobData(self._taskName, mmsCmd))

    def _selectMS(self, doCalibrationSelection = False):
        '''
        This method will open the MS and ensure whatever selection critera
        have been requested are honored.

        If doCalibrationSelection is true then the MS is selected to the
        calibration criteria.  If scanList is not None then it used as the
        scan selectior criteria.
        '''
        print "Start of Select MS"
        if self._msTool is None:
            # Open up the msTool
            self._msTool = mstool.create()
            self._msTool.open(self._arg['vis'])    
        else:
            self._msTool.reset()
        print "MS Tool Initialized"
            
        print "Getting Selection Filter"
        selectionFilter = self._getSelectionFilter()
        print "Selection Filter Complete"
        
        if not doCalibrationSelection and self._calScanList is not None:
            print "Augmenting Cal Selection"
            # We need to augment the selection to remove cal scans
            if self._selectionScanList is None:
                # Generate the selection scan list if needed
                if selectionFilter is not None:
                    self._msTool.msselect(selectionFilter)
                print "Getting Scan List"
                self._selectionScanList = self._getScanList()
                print "Scan List Complete"
                
                for scan in self._calScanList:
                    self._selectionScanList.remove(scan)

            # Augment the selection
            if selectionFilter is None:
                selectionFilter = {}
            selectionFilter['scan'] = ParallelTaskHelper.listToCasaString\
                                      (self._selectionScanList)

        print "Doing Primary Selection"
        if selectionFilter is not None:
            print selectionFilter
            self._msTool.msselect(selectionFilter)
        print "Primary Selection Complete"

        if doCalibrationSelection:
            print "Doing Calibration Selection"
            calFilter = self._getCalibrationFilter()
            self._msTool.msselect(calFilter)
            print "Calibration Selection Complete"


    def _getSPWList(self):
        '''
        This method returns the spectral window list from the current
        ms.  Be careful about having selection already done when you call this.
        '''
        if self._msTool is None:
            self._selectMS()
        
        # Now get the list of SPWs in the selected ms
        ddInfo = self._msTool.getspectralwindowinfo()['spwInfo']
        spwList = [info['SpectralWindowId'] for info in ddInfo.values()]

        # Return a unique sorted list:
        return list(set(spwList))

    def _getScanList(self):
        '''
        This method returns the scan list from the current ms.  Be careful
        about having selection already done when you call this.
        '''
        if self._msTool is None:
            self._selectMS()

        scanSummary = self._msTool.getscansummary()['summary']
        scanList = [int(scan) for scan in scanSummary]

        if len(scanList) == 0:
            raise ValueError, "No Scans present in the created MS."

        return scanList


    def postExecution(self):
        '''
        This overrides the post execution portion of the task helper
        in this case we probably need to generate the output reference
        ms.
        '''
        
        if self._arg['createmms']:
            outputList = self._jobQueue.getOutputJobs()
            # We created a data directory and many SubMSs build the reference
            # MSs
            if self._arg['calmsselection'] in ['auto','manual']:
                # A Cal MS was created in the data directory, see if it was
                # successful, if so build a reference MS
                if os.path.exists(self._arg['calmsname']):
                    raise ValueError, "Output MS already exists"
                self._msTool.createmultims(self._arg['calmsname'],
                         [self.dataDir +'/%s.cal.ms'%self.outputBase])
                
            subMSList = []
            for job in outputList:
                if job.status == 'done':
                    subMSList.append(job.getCommandArguments()['outputvis'])

            if len(subMSList) == 0:
                print "Error, no subMSs were created."
                return False

            # Really should double check here before creating.
            if os.path.exists(self._arg['outputvis']):
                raise ValueError, "Output MS already exists"
            self._msTool.createmultims(self._arg['outputvis'],subMSList)

            
        # Probably should check to see if anything failed
        # This should be done in the base class
        return True
        

    def _getSelectionFilter(self):
        # This method takes the list of specified selection criteria and
        # puts them into a dictionary.  There is a bit of name managling
        # necessary.
        # The pairs are: (msselection syntax, split task syntanc)
        selectionPairs = []
        selectionPairs.append(('field','field'))
        selectionPairs.append(('spw','spw'))
        selectionPairs.append(('baseline','antenna'))
        selectionPairs.append(('time','timerange'))
        selectionPairs.append(('scan','scan'))
        selectionPairs.append(('uvdist','uvrange'))
        selectionPairs.append(('scanintent','scanintent'))
        selectionPairs.append(('observation','observation'))
        return self.__generateFilter(selectionPairs)

    def _getCalibrationFilter(self):
        # Now get the calibrationSelectionFilter
        if self._arg['calmsselection'].lower() == 'auto':
            return {'scanintent':'CALIBRATE_*'}
        else:
            selectionPairs = []
            selectionPairs.append(('field','calfield'))
            selectionPairs.append(('scan','calscan'))
            selectionPairs.append(('scanintent','calintent'))
            return self.__generateFilter(selectionPairs)

    def __generateFilter(self, selectionPairs):
        filter = None
        for (selSyntax, argSyntax) in selectionPairs:
            if self._arg[argSyntax] != '':
                if filter is None:
                    filter = {}
                filter[selSyntax] = self._arg[argSyntax]
        return filter

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


def partition(vis,
           outputvis,
           createmms,
           separationaxis,
           numsubms,
           calmsselection,
           calmsname,
           calfield,
           calscan,
           calintent,
           datacolumn,
           field,
           spw,
           antenna,
           timebin,
           timerange,
           scan,
           scanintent,
           array,
           uvrange,
           observation,
           combine):
    """Create a visibility subset from an existing visibility set:

    Keyword arguments:
    vis -- Name of input visibility file (MS)
            default: none; example: vis='ngc5921.ms'
    outputvis -- Name of output visibility file (MS)
                  default: none; example: outputvis='ngc5921_src.ms'
    createmms -- Boolean flag if we're creating Multi MS
                  default: True
    separationaxis -- what axis do we intend to split on.
                   default = 'scan'
                   Options: 'scan'
    createcalms -- Boolean flag if we plan to create a separate Calibration ms
                   default = True
    calmsname -- Name of output calibration visibility file (MS)
                  default: none; example: outputvis='ngc5921_CAL.ms'
    calselection -- Method by which the calibration scans will be identified.
                  default: auto
    datacolumn -- Which data column to split out
                  default='corrected'; example: datacolumn='data'
                  Options: 'data', 'corrected', 'model', 'all',
                  'float_data', 'lag_data', 'float_data,data', and
                  'lag_data,data'.
                  note: 'all' = whichever of the above that are present.
    field -- Field name
              default: field = '' means  use all sources
              field = 1 # will get field_id=1 (if you give it an
                          integer, it will retrieve the source with that index)
              field = '1328+307' specifies source '1328+307'.
                 Minimum match can be used, egs  field = '13*' will
                 retrieve '1328+307' if it is unique or exists.
                 Source names with imbedded blanks cannot be included.
    spw -- Spectral window index identifier
            default=-1 (all); example: spw=1
    antenna -- antenna names
               default '' (all),
               antenna = '3 & 7' gives one baseline with antennaid = 3,7.
    timebin -- Interval width for time averaging.
               default: '0s' or '-1s' (no averaging)
               example: timebin='30s'
    timerange -- Time range
                 default='' means all times.  examples:
                 timerange = 'YYYY/MM/DD/hh:mm:ss~YYYY/MM/DD/hh:mm:ss'
                 timerange='< YYYY/MM/DD/HH:MM:SS.sss'
                 timerange='> YYYY/MM/DD/HH:MM:SS.sss'
                 timerange='< ddd/HH:MM:SS.sss'
                 timerange='> ddd/HH:MM:SS.sss'
    scan -- Scan numbers to select.
            default '' (all).
    scanintent -- Select based on the scan intent.
                  default '' (all)
    array -- (Sub)array IDs to select.     
             default '' (all).
    uvrange -- uv distance range to select.
               default '' (all).
    observation -- observation ID(s) to select.
                   default '' (all).
    combine -- Data descriptors that time averaging can ignore:
                  scan, and/or state
                  Default '' (none)
    """
    #retval = True
    casalog.origin('partition')
    
    # Start by going through and checking all the parameters
    if not isinstance(vis, str) or not os.path.exists(vis):
        raise ValueError, \
              'Visibility data set (%s) not found - please verify the name' % \
              vis

    if isinstance(outputvis, str):
        outputvis = outputvis.rstrip('/')

    if outputvis != '' and os.path.exists(outputvis):
        raise ValueError, "Output MS %s already exists - will not overwrite."%\
              outputvis
    if isinstance(antenna,list):
        antenna = ', '.join([str(ant) for ant in antenna])
    if isinstance(spw, list):
        spw = ','.join([str(s) for s in spw])
    elif isinstance(spw,int):
        spw = str(spw)

    # Accept digits without units ...assume seconds
    timebin = qa.convert(qa.quantity(timebin), 's')['value']
    timebin = str(timebin) + 's'
    if timebin == '0s':
        timebin= '-1s'

    if hasattr(combine, '_iter_'):
        combine = ', '.join(combine)

    if calmsselection in ['auto','manual']:
        if os.path.exists(calmsname):
            raise ValueError, ("Output calibration MS %s already exists "+\
                               "will not overwrite." ) % calmsname

    if createmms or not \
           ((calmsselection in ['auto','manual']) ^ (outputvis != '')):
        # This means we are creating more than a single MS do it in parallel
        ph = PartitionHelper(locals())
        ph.go()
        return

    # Create the msTool
    try:
        msTool = mstool.create()
        msTool.open(vis)
        msTool.partition(outputms=outputvis,
                         field=field,
                         spw=spw,
                         baseline=antenna,
                         subarray=array,
                         timebin=timebin,
                         time=timerange,
                         whichcol=datacolumn,
                         scan=scan,
                         uvrange=uvrange,
                         combine=combine,
                         intent=scanintent,
                         obs=observation)
    finally:
        msTool.close()

    # Write history to output MS, not the input ms.
    try:
        param_names = partition.func_code.co_varnames[:partition.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]
        write_history(msTool, outputvis, 'partition', param_names,
                      param_vals, casalog)
    except Exception, instance:
        casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                     'WARN')

    return True
