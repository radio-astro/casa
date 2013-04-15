import os
import shutil
import string
import copy
import math
from taskinit import *
from parallel.parallel_task_helper import ParallelTaskHelper
import partitionhelper as ph
import flaghelper as fh
import simple_cluster

class PartitionHelper(ParallelTaskHelper):
    def __init__(self, args = {}):
        ParallelTaskHelper.__init__(self,'oldpartition', args)
        self._calScanList = None
        self._selectionScanList = None
        self._msTool = None
        self._tbTool = None

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
                shutil.rmtree(self.dataDir)

            os.mkdir(self.dataDir)

            ## handle the POINTING and SYSCAL tables ##
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

            if not self.pointingisempty:
                if os.access(os.path.dirname(self.ptab), os.W_OK) \
                       and not os.path.islink(self.ptab):
                    # move to datadir
                    os.system('mv '+self.ptab+' '+self.dataDir)
                    # create empty copy in original place so partition does not need to deal with it
                    mytb.open(self.dataDir+'/POINTING')
                    tmpp = mytb.copy(newtablename=self.ptab, norows=True)
                    tmpp.close()
                    mytb.close()
                else:
                    self.pwriteaccess = False
                    

            if not self.syscalisempty:
                if os.access(os.path.dirname(self.stab), os.W_OK) \
                       and not os.path.islink(self.stab):
                    # move to datadir
                    os.system('mv '+self.stab+' '+self.dataDir)
                    # create empty copy in original place so partition does not need to deal with it
                    mytb.open(self.dataDir+'/SYSCAL')
                    tmpp = mytb.copy(newtablename=self.stab, norows=True)
                    tmpp.close()
                    mytb.close()
                else:
                    self.swriteaccess = False


    def generateJobs(self):
        '''
        This method overrides the method in the TaskHelper baseclass
        '''
        if self._arg['calmsselection'] in ['auto','manual']:
            casalog.post("Analyzing MS for Calibration MS Creation")
            self._createCalMSCommand()

        if self._arg['outputvis'] != '':
            casalog.post("Analyzing MS for partitioning")
            self._createPrimarySplitCommand()
            
        return True

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
            elif self._arg['separationaxis'].lower() == 'both':
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
            self._msTool = mstool()
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
        ddInfo = self._msTool.getspectralwindowinfo()
        spwList = [info['SpectralWindowId'] for info in ddInfo.values()]

        # Return a unique sorted list:
        sorted = list(set(spwList))
        sorted.sort()
        return sorted

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


    def postExecution(self):
        '''
        This overrides the post execution portion of the task helper
        in this case we probably need to generate the output reference
        ms.
        '''
        
        if self._arg['createmms']:
            casalog.post("Finalizing MMS structure")

            # restore POINTING and SYSCAL
            if self.pwriteaccess and not self.pointingisempty:
                print "restoring POINTING"
                os.system('rm -rf '+self.ptab) # remove empty copy
                os.system('mv '+self.dataDir+'/POINTING '+self.ptab)
            if self.swriteaccess and not self.syscalisempty:
                print "restoring SYSCAL"
                os.system('rm -rf '+self.stab) # remove empty copy
                os.system('mv '+self.dataDir+'/SYSCAL '+self.stab)
            
            # jagonzal (CAS-4287): Add a cluster-less mode to by-pass parallel processing for MMSs as requested 
            if (ParallelTaskHelper.getBypassParallelProcessing()==1):
                outputList = self._sequential_return_list
                self._sequential_return_list = {}
            else:
                outputList = self._jobQueue.getOutputJobs()
            # We created a data directory and many SubMSs,
            # now build the reference MS
            if self._arg['calmsselection'] in ['auto','manual']:
                # A Cal MS was created in the data directory, see if it was
                # successful, if so build a multi-MS
                if os.path.exists(self._arg['calmsname']):
                    raise ValueError, "Output MS already exists"
                self._msTool.createmultims(self._arg['calmsname'],
                         [self.dataDir +'/%s.cal.ms'%self.outputBase])
            
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

            mastersubms = subMSList[0]

            subtabs_to_omit = []

            # deal with POINTING table
            if not self.pointingisempty:
                shutil.rmtree(mastersubms+'/POINTING', ignore_errors=True)
                shutil.copytree(self.ptab, mastersubms+'/POINTING') # master subms gets a full copy of the original
            if self.makepointinglinks:
                for i in xrange(1,len(subMSList)):
                    theptab = subMSList[i]+'/POINTING'
                    shutil.rmtree(theptab, ignore_errors=True)
                    os.symlink('../'+os.path.basename(mastersubms)+'/POINTING', theptab)
                    # (link in target will be created my makeMMS)
                subtabs_to_omit.append('POINTING')

            # deal with SYSCAL table
            if not self.syscalisempty:
                shutil.rmtree(mastersubms+'/SYSCAL', ignore_errors=True)
                shutil.copytree(self.stab, mastersubms+'/SYSCAL') # master subms gets a full copy of the original
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


def oldpartition(vis,
           outputvis,
           createmms,
           separationaxis,
           numsubms,
           datacolumn,
           calmsselection,
           calmsname,
           calfield,
           calscan,
           calintent,
           field,
           spw,
           antenna,
           timebin,
           combine,
           timerange,
           scan,
           scanintent,
           array,
           uvrange,
           observation
           ):
    """Create a multi visibility set from an existing visibility set:

    Keyword arguments:
    vis -- Name of input visibility file (MS)
            default: none; example: vis='ngc5921.ms'
    outputvis -- Name of output visibility file (MS)
                  default: none; example: outputvis='ngc5921_src.ms'
    createmms -- Boolean flag if we're creating Multi MS
                  default: True
        separationaxis -- what axis do we intend to split on.
                   default = 'scan'
                   Options: 'scan','spw','both'
        numsubms -- Number of sub-MSs to create.
                    default: 64
    datacolumn -- Which data column to split out
                  default='corrected'; example: datacolumn='data'
                  Options: 'data', 'corrected', 'model', 'all',
                  'float_data', 'lag_data', 'float_data,data', and
                  'lag_data,data'.
                  note: 'all' = whichever of the above that are present.
    calmsselection -- Method by which to create a separate Calibration ms
                   default = 'none'
                   options: 'none','auto','manual'
        calmsname -- Name of output calibration visibility file (MS)
                  default: none; example: outputvis='ngc5921_CAL.ms'
        calfield -- Field selection for calibration MS.
                  default: ''
        calscans -- Scan selection for calibration MS.
                  default: ''
        calintent -- Scan intent selection for calibration MS.
                  default: ''
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
        combine -- Data descriptors that time averaging can ignore:
                  scan, and/or state
                  Default '' (none)
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
    """
    #retval = True
    casalog.origin('partition')
    
    # Start by going through and checking all the parameters
    if not isinstance(vis, str) or not os.path.exists(vis):
        raise ValueError, \
              'Visibility data set (%s) not found - please verify the name' % \
              vis
    
    # SMC: The outputvis must be given
    # NOTE: added print statement because the exception msgs are
    # not being printed at this moment.
    if not outputvis or outputvis.isspace():
        print 'Please specify outputvis'
        raise ValueError, 'Please specify outputvis'

    outputvis = outputvis.rstrip('/')
    
    if outputvis != '' and os.path.exists(outputvis):
        print 'Output MS %s already exists - will not overwrite.'%outputvis
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
        
        # Create a backup of the flags that are in the MMS
        casalog.origin('partition')
        casalog.post('Create a backup of the flags that are in the MMS')
        fh.backupFlags(aflocal=None, msfile=outputvis, prename='partition')    

        return

    # Create the msTool
    try:
        msTool = mstool()
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
                         obs=str(observation))
        msTool.close()
    except Exception, instance:
        casalog.post("*** Error \'%s\' captured in partition" % (instance),'WARN')
        msTool.close()

    # Write history to output MS, not the input ms.
    try:
        param_names = oldpartition.func_code.co_varnames[:oldpartition.func_code.co_argcount]
        param_vals = [eval(p) for p in param_names]
        write_history(msTool, outputvis, 'oldpartition', param_names,
                      param_vals, casalog)
    except Exception, instance:
        casalog.post("*** Error \'%s\' updating HISTORY" % (instance),
                     'WARN')

    return True
