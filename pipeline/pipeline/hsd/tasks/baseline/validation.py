from __future__ import absolute_import, print_function

import math
import sys
import time
from math import sqrt

import numpy
import numpy.linalg as LA
import scipy.cluster.hierarchy as HIERARCHY
import scipy.cluster.vq as VQ

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.domain.datatable import DataTableImpl as DataTable
from . import rules
from .. import common
from ..common import utils

_LOG = infrastructure.get_logger(__name__)
LOG = utils.OnDemandStringParseLogger(_LOG)


def ValidationFactory(pattern):
    if pattern == 'RASTER':
        return ValidateLineRaster
    elif pattern == 'SINGLE-POINT' or pattern == 'MULTI-POINT':
        return ValidateLineSinglePointing
    else:
        raise ValueError, 'Invalid observing pattern'


class ValidateLineInputs(common.SingleDishInputs):
    def __init__(self, context, group_id, member_list, iteration, grid_ra, grid_dec,
                 window=None, edge=None, nsigma=None, xorder=None, yorder=None, 
                 broad_component=None, clusteringalgorithm=None):
        self._init_properties(vars())
        
    @property
    def window(self):
        return [] if self._window is None else self._window
    
    @window.setter
    def window(self, value):
        self._window = value
        
    @property
    def edge(self):
        return (0,0) if self._edge is None else self._edge
    
    @edge.setter
    def edge(self, value):
        self._edge = value

    @property
    def nsigma(self):
        return 3.0 if self._nsigma is None else self._nsigma
    
    @nsigma.setter
    def nsigma(self, value):
        self._nsigma = value
        
    @property
    def xorder(self):
        return -1 if self._xorder is None else self._xorder
    
    @xorder.setter
    def xorder(self, value):
        self._xorder = value

    @property
    def yorder(self):
        return -1 if self._yorder is None else self._yorder
    
    @yorder.setter
    def yorder(self, value):
        self._yorder = value
        
    @property
    def broad_component(self):
        return False if self._broad_component is None else self._broad_component
    
    @broad_component.setter
    def broad_component(self, value):
        self._broad_component = value
        
    @property
    def clusteringalgorithm(self):
        if hasattr(self, '_clusteringalgorithm') and self._clusteringalgorithm is not None:
            return self._clusteringalgorithm
        else:
            return rules.ClusterRule['ClusterAlgorithm']
        
    @clusteringalgorithm.setter
    def clusteringalgorithm(self, value):
        self._clusteringalgorithm = value

    @property
    def group_desc(self):
        return self.context.observing_run.ms_reduction_group[self.group_id]

    @property
    def reference_member(self):
        return self.group_desc[self.member_list[0]]


class ValidateLineResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(ValidateLineResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(ValidateLineResults, self).merge_with_context(context)
        # exporting datatable should be done within the parent task
#         datatable = self.outcome.pop('datatable')
#         datatable.exportdata(minimal=True)
        
    @property
    def datatable(self):
        return self._get_outcome('datatable')

    def _outcome_name(self):
        return ''


class ValidateLineSinglePointing(basetask.StandardTaskTemplate):
    Inputs = ValidateLineInputs

    def prepare(self, datatable=None, index_list=None, grid_table=None, detect_signal=None):
        """
        ValidateLine class for single-pointing or multi-pointing (collection of 
        fields with single-pointing). Accept all detected lines without 
        clustering analysis.

         detect_signal = {ID1: [RA, DEC, [[LineStartChannel1, LineEndChannel1],
                                         [LineStartChannel2, LineEndChannel2],
                                         [LineStartChannelN, LineEndChannelN]]],
                         IDn: [RA, DEC, [[LineStartChannel1, LineEndChannel1],
                                         [LineStartChannelN, LineEndChannelN]]]}

        lines: output parameter
           [LineCenter, LineWidth, Validity]  OK: Validity = True; NG: Validity = False
        """
        window = self.inputs.window
        LOG.debug('window={}', window)
        
        if datatable is None:
            LOG.debug('#PNP# instantiate local datatable')
            datatable = DataTable(self.inputs.context.observing_run.ms_datatable_name)
            datatable_out = datatable
        else:
            LOG.debug('datatable is propagated from parent task')
            datatable_out = None

        # for Pre-Defined Spectrum Window
        if len(window) != 0:
            LOG.info('Skip clustering analysis since predefined line window is set.')
            lines = _to_validated_lines(self.inputs.detect_signal)
            signal = self.inputs.detect_signal.values()[0]
            for row in self.inputs.index_list:
                datatable.putcell('MASKLIST',row,signal[2])
                datatable.putcell('NOCHANGE',row,False)
            outcome = {'lines': lines,
                       'channelmap_range': lines,
                       'cluster_info': {},
                       'datatable': datatable_out}
            result = ValidateLineResults(task=self.__class__,
                                         success=True,
                                         outcome=outcome)
                
            result.task = self.__class__
                
            return result

        # Dictionary for final output
        lines = []

        LOG.info('Accept all detected lines without clustering analysis.')

        iteration = self.inputs.iteration
        
        assert index_list is not None
        assert detect_signal is not None

        # First cycle
        #if len(grid_table) == 0:
        if iteration == 0:
            for row in index_list:
                mask_list = datatable.getcell('MASKLIST',row)
                no_change = datatable.getcell('NOCHANGE',row)
                #LOG.debug('DataTable = %s, detect_signal = %s, OldFlag = %s' % (mask_list, detect_signal[row][2], no_change))
                datatable.putcell('MASKLIST',row,detect_signal[row][2])
                datatable.putcell('NOCHANGE',row,False)

        # Iteration case
        else:
            for row in index_list:
                mask_list = datatable.getcell('MASKLIST',row)
                no_change = datatable.getcell('NOCHANGE',row)
                #LOG.debug('DataTable = %s, detect_signal = %s, OldFlag = %s' % (mask_list, detect_signal[0][2], no_change))
                if mask_list == detect_signal[0][2]:
                    #if type(no_change) != int:
                    if no_change < 0:
                        # 2013/05/17 TN
                        # Put iteration itself instead to subtract 1 since
                        # iteration counter is incremented *after* the
                        # baseline subtraction in refactorred code.
                        #datatable.putcell('NOCHANGE',row,iteration - 1)
                        datatable.putcell('NOCHANGE',row,iteration)
                else:
                    datatable.putcell('MASKLIST',row,detect_signal[0][2])
                    datatable.putcell('NOCHANGE',row,False)
        outcome = {'lines': lines,
                   'channelmap_range': lines,
                   'cluster_info': {},
                   'datatable': datatable_out}
        result = ValidateLineResults(task=self.__class__,
                                     success=True,
                                     outcome=outcome)
                
        result.task = self.__class__
                
        return result
    
    def analyse(self, result):
        return result


class ValidateLineRaster(basetask.StandardTaskTemplate):
    Inputs = ValidateLineInputs

    CLUSTER_WHITEN = 1.0
    #CLUSTER_WHITEN = 4.0 # sensitivity of line width compared to center position -> 1/4

    # as of 2017/7/4 Valid=0.5, Marginal=0.35, Questionable=0.2
    # should be Valid=0.7, Marginal=0.5, Questionable=0.3
    Valid = rules.ClusterRule['ThresholdValid']
    Marginal = rules.ClusterRule['ThresholdMarginal']
    Questionable = rules.ClusterRule['ThresholdQuestionable']
    MinFWHM = rules.LineFinderRule['MinFWHM']
    #MaxFWHM = rules.LineFinderRule['MaxFWHM']
    #Clustering_Algorithm = rules.ClusterRule['ClusterAlgorithm']
    DebugOutName = '%05d' % (int(time.time())%100000)
    DebugOutVer = [0, 0]
    
    @property
    def MaxFWHM(self):
        num_edge = sum(self.inputs.edge)
        spw = self.inputs.reference_member.spw
        nchan = spw.num_channels
        return int(max(0, nchan - num_edge) / 3)

    def prepare(self, datatable=None, index_list=None, grid_table=None, detect_signal=None):
        """
        2D fit line characteristics calculated in Process3
        Sigma clipping iterations will be applied if nsigma is positive
        order < 0 : automatic determination of fitting order (max = 5)

         detect_signal = {ID1: [RA, DEC, [[LineStartChannel1, LineEndChannel1, Binning],
                                         [LineStartChannel2, LineEndChannel2, Binning],
                                         [LineStartChannelN, LineEndChannelN, Binning]]],
                         IDn: [RA, DEC, [[LineStartChannel1, LineEndChannel1, Binning],
                                         [LineStartChannelN, LineEndChannelN, Binning]]]}

        lines: output parameter
           [LineCenter, LineWidth, Validity]  OK: Validity = True; NG: Validity = False
        """
        window = self.inputs.window

        if datatable is None:
            LOG.debug('#PNP# instantiate local datatable')
            datatable = DataTable(self.inputs.context.observing_run.ms_datatable_name)
            datatable_out = datatable
        else:
            LOG.debug('datatable is propagated from parent task')
            datatable_out = None

        # for Pre-Defined Spectrum Window
        if len(window) != 0:
            LOG.info('Skip clustering analysis since predefined line window is set.')
            lines = _to_validated_lines(self.inputs.detect_signal)
            signal = self.inputs.detect_signal.values()[0]
            for row in self.inputs.index_list:
                datatable.putcell('MASKLIST',row,signal[2])
                datatable.putcell('NOCHANGE',row,False)
            outcome = {'lines': lines,
                       'channelmap_range': lines,
                       'cluster_info': {},
                       'datatable': datatable_out}
            result = ValidateLineResults(task=self.__class__,
                                         success=True,
                                         outcome=outcome)
                
            return result

        iteration = self.inputs.iteration
        
        assert grid_table is not None
        assert index_list is not None
        assert detect_signal is not None

        grid_ra = self.inputs.grid_ra
        grid_dec = self.inputs.grid_dec
        broad_component = self.inputs.broad_component
        xorder = self.inputs.xorder
        yorder = self.inputs.yorder
        self.nchan = datatable.getcell('NCHAN', index_list[0])
        self.nsigma = self.inputs.nsigma

        ProcStartTime = time.time()
        LOG.info('2D fit the line characteristics...')

        #tSFLAG = datatable.getcol('FLAG_SUMMARY')
        Totallines = 0
        RMS0 = 0.0
        lines = []
        self.cluster_info = {}
        ROWS = range(len(grid_table))

        # RASTER CASE
        # Read data from Table to generate ID -> RA, DEC conversion table
        Region = []
        dummy = []
        flag = 1
        Npos = 0

        # 2017/7/4 clean-up detect_signal
        LOG.trace('Before: Detect_Signal = {}', detect_signal)
        detect_signal = self.clean_detect_signal(detect_signal)
        LOG.trace('After: Detect_Signal = {}', detect_signal)

        for row in ROWS:
            # detect_signal[row][2]: [[LineStartChannelN, LineEndChannelN, Binning],[],,,[]]
            if len(detect_signal[row][2]) != 0 and detect_signal[row][2][0][0] != -1:
                Npos += 1
                for line in detect_signal[row][2]:
                    # Check statistics flag. tSFLAG[row]==1 => Valid Spectra 2008/1/17
                    # Bug fix 2008/5/29
                    #if (line[0] != line[1]) and ((len(grid_table) == 0 and tSFLAG[row] == 1) or len(grid_table) != 0):
                    # refering tSFLAG is not correct
                    if line[0] != line[1]: #and tSFLAG[row] == 1:
                        #2014/11/28 add Binning info into Region
                        Region.append([row, line[0], line[1], detect_signal[row][0], detect_signal[row][1], flag, line[2]])
                        ### 2011/05/17 make cluster insensitive to the line width: Whiten
                        dummy.append([float(line[1] - line[0]) / self.CLUSTER_WHITEN, 0.5 * float(line[0] + line[1])])
        Region2 = numpy.array(dummy) # [FullWidth, Center]
        ### 2015/04/22 save Region to file for test
        if infrastructure.logging.logging_level == infrastructure.logging.LOGGING_LEVELS['trace'] or \
           infrastructure.logging.logging_level == infrastructure.logging.LOGGING_LEVELS['debug']:
            self.DebugOutVer[0] += 1
            with open('ClstRegion.%s.%02d.txt' % (self.DebugOutName, self.DebugOutVer[0]), 'w') as fp:
                for i in range(len(Region)):
                    fp.writelines('%d %f %f %f %f %d %d\n' % (Region[i][0],Region[i][1],Region[i][2],Region[i][3],Region[i][4],Region[i][5],Region[i][6]))

        del dummy
        LOG.debug('Npos = {}', Npos)
        # 2010/6/9 for non-detection
        if Npos == 0 or len(Region2) == 0: 
            outcome = {'lines': [],
                       'channelmap_range': [],
                       'cluster_info': {},
                       'datatable': datatable_out}
            result = ValidateLineResults(task=self.__class__,
                                         success=True,
                                         outcome=outcome)
                
            result.task = self.__class__
                
            return result

        # 2008/9/20 Dec Effect was corrected
        PosList = numpy.array([numpy.take(datatable.getcol('RA'),index_list),
                               numpy.take(datatable.getcol('DEC'),index_list)])
        DecCorrection = 1.0 / math.cos(PosList[1][0] / 180.0 * 3.141592653)
        grid_ra *= DecCorrection
        # Calculate Parameters for Gridding
        wra = PosList[0].max() - PosList[0].min()
        wdec = PosList[1].max() - PosList[1].min()
        cra = PosList[0].min() + wra/2.0
        cdec = PosList[1].min() + wdec/2.0
        # 2010/6/11 +1.0 -> +1.01: if wra is n x grid_ra (n is a integer), int(wra/grid_ra) is not n in some cases because of the lack of accuracy.
        nra = 2 * (int((wra/2.0 - grid_ra/2.0)/grid_ra) + 1) + 1
        ndec = 2 * (int((wdec/2.0 - grid_dec/2.0)/grid_dec) + 1) + 1
        x0 = cra - grid_ra/2.0 - grid_ra*(nra-1)/2.0
        y0 = cdec - grid_dec/2.0 - grid_dec*(ndec-1)/2.0
        LOG.debug('Grid = {} x {}\n', nra, ndec)
        self.cluster_info['grid'] = {}
        self.cluster_info['grid']['ra_min'] = x0
        self.cluster_info['grid']['dec_min'] = y0
        self.cluster_info['grid']['grid_ra'] = grid_ra
        self.cluster_info['grid']['grid_dec'] = grid_dec

        # Create Space for storing the list of spectrum (ID) in the Grid
        # 2013/03/27 TN
        # Grid2SpectrumID stores index of index_list instead of row numbers 
        # that are held by index_list.
        Grid2SpectrumID = [[[] for y in xrange(ndec)] for x in xrange(nra)]
        for i in range(len(index_list)):
            Grid2SpectrumID[int((PosList[0][i] - x0)/grid_ra)][int((PosList[1][i] - y0)/grid_dec)].append(i)

        ProcEndTime = time.time()
        LOG.info('Clustering: Initialization End: Elapsed time = {} sec', ProcEndTime - ProcStartTime)


        ######## Clustering: K-mean Stage ########
        # K-mean Clustering Analysis with LineWidth and LineCenter
        # Max number of protect regions are SDC.SDParam['Cluster']['MaxCluster'] (Max lines)
        ProcStartTime = time.time()
        LOG.info('Clustering Analysis Start')

        # Bestlines: [[center, width, T/F],[],,,[]]
        clustering_algorithm = self.inputs.clusteringalgorithm
        # 2017/7/25 fix to Hierarchical Clustering for test
        clustering_algorithm = 'hierarchy'
        LOG.debug('clustering algorithm is \'{}\'', clustering_algorithm)
        if clustering_algorithm == 'kmean':
            (Ncluster, Bestlines, BestCategory, Region) = self.clustering_kmean(Region, Region2)
        else:
            (Ncluster, Bestlines, BestCategory, Region) = self.clustering_hierarchy(Region, Region2, nThreshold=rules.ClusterRule['ThresholdHierarchy'], method='single')

        ProcEndTime = time.time()
        LOG.info('Clustering Analysis End: Elapsed time = {} sec', (ProcEndTime - ProcStartTime))
        # 2017/8/15 for non-detection after cleaninig
        if Ncluster == 0: 
            outcome = {'lines': [],
                       'channelmap_range': [],
                       'cluster_info': {},
                       'datatable': datatable_out}
            result = ValidateLineResults(task=self.__class__,
                                         success=True,
                                         outcome=outcome)
            result.task = self.__class__
            return result

        # Sort lines and Category by LineCenter: lines[][0]
        LineIndex = numpy.argsort([line[0] for line in Bestlines[:Ncluster]])
        lines = [Bestlines[i] for i in LineIndex]
        print('Ncluster, lines: {} {}'.format(Ncluster, lines))
        print('LineIndex: {}'.format(LineIndex))

        ### 2011/05/17 anti-scaling of the line width
        Region2[:,0] = Region2[:,0] * self.CLUSTER_WHITEN
        for Nc in range(Ncluster):
            lines[Nc][1] *= self.CLUSTER_WHITEN

        LineIndex2 = numpy.argsort(LineIndex)
        print('LineIndex2: {}'.format(LineIndex2))
        print('BestCategory: {}'.format(BestCategory))

        #for i in range(len(BestCategory)): category[i] = LineIndex2[BestCategory[i]]
        category = [LineIndex2[bc] for bc in BestCategory]


        ######## Clustering: Detection Stage ########
        ProcStartTime = time.time()
        LOG.info('Clustering: Detection Stage Start')

        (GridCluster, GridMember) = self.detection_stage(Ncluster, nra, ndec, x0, y0, grid_ra, grid_dec, category, Region, detect_signal)

        ProcEndTime = time.time()
        LOG.info('Clustering: Detection Stage End: Elapsed time = {} sec', (ProcEndTime - ProcStartTime))

        ######## Clustering: Validation Stage ########
        ProcStartTime = time.time()
        LOG.info('Clustering: Validation Stage Start')

        (GridCluster, GridMember, lines) = self.validation_stage(GridCluster, GridMember, lines)
        
        ProcEndTime = time.time()
        LOG.info('Clustering: Validation Stage End: Elapsed time = {} sec', (ProcEndTime - ProcStartTime))

        ######## Clustering: Smoothing Stage ########
        # Rating:  [0.0, 0.4, 0.5, 0.4, 0.0]
        #          [0.4, 0.7, 1.0, 0.7, 0.4]
        #          [0.5, 1.0, 6.0, 1.0, 0.5]
        #          [0.4, 0.7, 1.0, 0.7, 0.4]
        #          [0.0, 0.4, 0.5, 0.4, 0.0]
        # Rating = 1.0 / (Dx**2 + Dy**2)**(0.5) : if (Dx, Dy) == (0, 0) rating = 6.0

        ProcStartTime = time.time()
        LOG.info('Clustering: Smoothing Stage Start')

        (GridCluster, lines) = self.smoothing_stage(GridCluster, lines)

        ProcEndTime = time.time()
        LOG.info('Clustering: Smoothing Stage End: Elapsed time = {} sec', (ProcEndTime - ProcStartTime))
        
        ######## Clustering: Final Stage ########
        ProcStartTime = time.time()
        LOG.info('Clustering: Final Stage Start')

        (RealSignal, lines, channelmap_range) = self.final_stage(GridCluster, GridMember, Region, Region2, lines, category, grid_ra, grid_dec, broad_component, xorder, yorder, x0, y0, Grid2SpectrumID, index_list, PosList)

        ProcEndTime = time.time()
        LOG.info('Clustering: Final Stage End: Elapsed time = {} sec', (ProcEndTime - ProcStartTime))

        # Merge masks if possible
        ProcStartTime = time.time()
        LOG.info('Clustering: Merging Start')
        # RealSignal should have all row's as its key
        tmp_index = 0
        for row in index_list:
            if RealSignal.has_key(row):
                signal = self.__merge_lines(RealSignal[row][2], self.nchan)
            else:
                signal = [[-1,-1]]
                #RealSignal[row] = [PosList[0][tmp_index], PosList[1][tmp_index], signal]
            tmp_index += 1

            # In the following code, access to MASKLIST and NOCHANGE columns 
            # is direct to underlying table object instead of access via 
            # datatable object's method since direct access is faster. 
            # Note that MASKLIST [[-1,-1]] represents that no masks are 
            # available, while NOCHANGE -1 indicates NOCHANGE is False.
            #tMASKLIST = datatable.getcell('MASKLIST',row)
            #tNOCHANGE = datatable.getcell('NOCHANGE',row)
            tMASKLIST = datatable.getcell('MASKLIST',row)
            if len(tMASKLIST) == 0 or tMASKLIST[0][0] < 0:
                tMASKLIST = []
            else:
                tMASKLIST=tMASKLIST.tolist()#list(tMASKLIST)
            tNOCHANGE = datatable.getcell('NOCHANGE',row)
            #LOG.debug('DataTable = %s, RealSignal = %s' % (tMASKLIST, signal))
            if tMASKLIST == signal:
                #LOG.debug('No update on row %s: iter is %s'%(row,iteration))
                #if type(tNOCHANGE) != int:
                if tNOCHANGE < 0:
                    # 2013/05/17 TN
                    # Put iteration itself instead to subtract 1 since iteration
                    # counter is incremented *after* baseline subtraction
                    # in refactorred code.
                    #datatable.putcell('NOCHANGE',row,iteration - 1)
                    #datatable.putcell('NOCHANGE', row, iteration)
                    datatable.putcell('NOCHANGE', row, iteration)
            else:
                #datatable.putcell('NOCHANGE',row,False)
                #datatable.putcell('MASKLIST',row,numpy.array(RealSignal[row][2]))
                #LOG.debug('Updating row %s: signal=%s (type=%s, %s)'%(row,list(signal),type(signal),type(signal[0])))
                #datatable.putcell('MASKLIST',row,numpy.array(signal))
                #datatable.putcell('MASKLIST',row,signal)
                datatable.putcell('MASKLIST',row,signal)
                #datatable.putcell('NOCHANGE',row,-1)
                datatable.putcell('NOCHANGE',row,-1)
        del GridCluster, RealSignal
        ProcEndTime = time.time()
        LOG.info('Clustering: Merging End: Elapsed time = {} sec', (ProcEndTime - ProcStartTime))
        
        outcome = {'lines': lines,
                   'channelmap_range': channelmap_range,
                   'cluster_info': self.cluster_info,
                   'datatable': datatable_out}
        result = ValidateLineResults(task=self.__class__,
                                     success=True,
                                     outcome=outcome)
                
        result.task = self.__class__
                
        return result
    
    def analyse(self, result):
        return result

    def clean_detect_signal(self, DS):
        """
        Spectra in each grid positions are splitted into 3 groups along time series.
        Group of spectra is then combined to 1 spectrum. So, one grid position has
        3 combined spectra.
        Suppose that the real signal is correlated but the error is not, we can
        clean false signals (not correlated) in advance of the validation stage.
        detect_signal = {ID1: [RA, DEC, [[LineStartChannel1, LineEndChannel1, Binning],
                                         [LineStartChannel2, LineEndChannel2, Binning],
                                         [LineStartChannelN, LineEndChannelN, Binning]]],
                         IDn: [RA, DEC, [[LineStartChannel1, LineEndChannel1, Binning],
                                         [LineStartChannelN, LineEndChannelN, Binning]]]}
        """
        # grouping by position
        Gthreshold = 1.0 / 3600.
        DSkey = DS.keys()
        PosGroup = []
        # PosGroup: [[ID,ID,ID],[ID,ID,ID],...,[ID,ID,ID]]
        for ID in DS.keys():
            if ID in DSkey:
                del DSkey[DSkey.index(ID)]
                DStmp = DSkey[:]
                PosGroup.append([ID])
                for nID in DStmp:
                    if abs(DS[ID][0] - DS[nID][0]) < Gthreshold and abs(DS[ID][1] - DS[nID][1]) < Gthreshold:
                        del DSkey[DSkey.index(nID)]
                        PosGroup[-1].append(nID)
        LOG.debug('clean_detect_signal: PosGroup = {}', PosGroup)
        for PList in PosGroup:
            if len(PList) > 2:
                data = {}
                for i in PList:
                    data[i] = DS[i][2]
                # threshold 0.7: identical line is detected in all 3 data: strict checking
                # threshold 0.6: identical line is detected in 2 data out of 3
                ret = self.clean_detect_line(data.copy(), threshold=0.7)
                for i in PList:
                    DS[i][2] = ret[i]

        return DS

    def clean_detect_line(self, data, threshold=0.6):
        """
        Select only line candidates with good possibility by checking all spectra taken at the same position
        data: {ID1: [[LineStart, LineEnd, Binning],,,],
               ID2: [[LineStart, LineEnd, Binning],,,],
               IDn: [[LineStart, LineEnd, Binning],,,]}
        """
        ret = {}
        NSP = float(len(data))
        for ID in data.keys():
            ret[ID] = []
        for ID in data.keys():
            for i in range(len(data[ID])):
                if data[ID][i][0] != -1:
                    ref = data[ID][i][:]
                    tmp = []
                    # if binning > 1: expected number of lines in the grid doubles, i.e., two different offsets
                    for nID in data.keys():
                        Lines = [0.0, 0.0, 0.0]
                        for j in range(len(data[nID])):
                            # check binning is the same, and overlap
                            if data[nID][j][2] == ref[2] and self.CheckLineIdentity(ref, data[nID][j], overlap=0.7):
                                Lines[0] += data[nID][j][0]
                                Lines[1] += data[nID][j][1]
                                Lines[2] += 1.0
                                # clear identical line
                                data[nID][j] = [-1, -1, 1]
                        if Lines[2] > 0: tmp.append([nID, [Lines[0]/Lines[2], Lines[1]/Lines[2], ref[2]]])
                    if len(tmp) / NSP >= threshold:
                        for nID, ref in tmp:
                            ret[nID].append(ref)
        for ID in data.keys():
            if len(ret[ID]) == 0: ret[ID].append([-1, -1, 1])

        return ret

    def CheckLineIdentity(self, old, new, overlap=0.7):
        """
        True if the overlap of two lines is greater than the threshold
        1L          1R          1L         1R          1L        1R       1L       1R
         [          ]           [          ]            [         ]       [         ]
         xxxxxxxxxxxx           xxxxxxxxxxxx            xxxxxxxxxxx       xxxxxxxxxxx
        oooooooooooooo           oooooooooooo          ooooooooooo         ooooooooo
        [            ]           [          ]          [         ]         [       ]
        2L          2R          2L         2R          2L        2R       2L       2R

        True if Num(x) / Num(o) >= overlap

        old: [left, right, binning]
        new: [left, right, binning]
        """
        if(old[0] <= new[0] < old[1] or \
           old[0] < new[1] <= old[1] or \
           new[0] <= old[0] < new[1] or \
           new[0] < old[1] <= new[1]) and \
           (min(old[1], new[1]) - max(old[0], new[0]) + 1) / float(max((old[1] - old[0] + 1), (new[1] - new[0] + 1))) >= overlap:
            #print 'Identical', old, new
            return True
        else: return False

    def clustering_kmean(self, Region, Region2):
        # Region = [[row, chan0, chan1, RA, DEC, flag, Binning],[],[],,,[]]
        # Region2 = [[Width, Center],[],[],,,[]]
        MedianWidth = numpy.median(Region2[:,0])
        LOG.trace('MedianWidth = {}', MedianWidth)

        MaxCluster = int(rules.ClusterRule['MaxCluster'])
        LOG.info('Maximum number of clusters (MaxCluster) = {}', MaxCluster)

        # Determin the optimum number of clusters
        BestScore = -1.0
        # 2010/6/15 Plot the score along the number of the clusters
        ListNcluster = []
        ListScore = []
        ListBestScore = []
        converged = False
        #
        elapsed = 0.0
        self.DebugOutVer[1] += 1
        for Ncluster in xrange(1, MaxCluster + 1):
            index0=len(ListScore)
            # Fix the random seed 2008/5/23
            numpy.random.seed((1234,567))
            # Try multiple times to supress random selection effect 2007/09/04
            for Multi in xrange(min(Ncluster+1, 10)):
                codebook, diff = VQ.kmeans(Region2, Ncluster, iter=50)
                # codebook = [[clstCentX, clstCentY],[clstCentX,clstCentY],,[]] len=Ncluster
                # diff <= distortion
                NclusterNew = 0
                LOG.trace('codebook={}', codebook)
                # Do iteration until no merging of clusters to be found
                while(NclusterNew != len(codebook)):
                    category, distance = VQ.vq(Region2, codebook)
                    # category = [c0, c0, c1, c2, c0,,,] c0,c1,c2,,, are clusters which each element belongs to
                    # category starts with 0
                    # distance = [d1, d2, d3,,,,,] distance from belonging cluster center
                    LOG.trace('Cluster Category&Distance {}, distance = {}', category, distance)

                    # remove empty line in codebook
                    codebook = codebook.take([x for x in xrange(0,len(codebook)) 
                                            if any(category==x)], axis=0)
                    NclusterNew = len(codebook)

                    # Clear Flag
                    for i in xrange(len(Region)): Region[i][5] = 1

                    Outlier = 0.0
                    lines = []
                    for Nc in xrange(NclusterNew):
                        ### 2011/05/17 Strict the threshold, clean-up each cluster by nsigma clipping/flagging
                        ValueList = distance[(category == Nc).nonzero()[0]]
                        Stddev = ValueList.std()
                        Threshold = ValueList.mean() + Stddev * self.nsigma
                        del ValueList
                        LOG.trace('Cluster Clipping Threshold = {}, Stddev = {}', Threshold, Stddev)
                        for i in ((distance * (category == Nc)) > Threshold).nonzero()[0]:
                            Region[i][5] = 0 # set flag to 0
                            Outlier += 1.0
                        # Calculate Cluster Characteristics
                        MaxDistance = max(distance * ((distance < Threshold) * (category == Nc)))
                        indices = [x for x in xrange(len(category)) 
                                if category[x] == Nc and Region[x][5] != 0]
                        properties = Region2.take(indices, axis=0)
                        median_props = numpy.median(properties, axis=0)
                        lines.append([median_props[1], median_props[0], True, MaxDistance])
                    MemberRate = (len(Region) - Outlier)/float(len(Region))
                    MeanDistance = (distance * numpy.transpose(numpy.array(Region))[5]).mean()
                    LOG.trace('lines = {}, MemberRate = {}', lines, MemberRate)

                    # 2010/6/15 Plot the score along the number of the clusters
                    ListNcluster.append(Ncluster)
                    Score = self.clustering_kmean_score(MeanDistance, MedianWidth, NclusterNew, MemberRate)
                    ListScore.append(Score)
                    LOG.debug('NclusterNew = {}, Score = {}', NclusterNew, Score)

                    ### 2017/07/06 save Score to file for test
                    if infrastructure.logging.logging_level == infrastructure.logging.LOGGING_LEVELS['trace'] or \
                       infrastructure.logging.logging_level == infrastructure.logging.LOGGING_LEVELS['debug']:
                        with open('ClstProp.%s.%02d.txt' % (self.DebugOutName, self.DebugOutVer[1]), "wa") as fp:
                            fp.writelines('%d,%f,%f,%f,%f,%s\n' % (NclusterNew, Score, MeanDistance, MedianWidth, MemberRate, lines))

                    if BestScore < 0 or Score < BestScore:
                        BestNcluster = NclusterNew
                        BestScore = Score
                        BestCategory = category.copy()
                        BestCodebook = codebook.copy()
                        BestRegion = Region[:]
                        Bestlines = lines[:]

            ListBestScore.append(min(ListScore[index0:]))
            LOG.debug('Ncluster = {}, BestScore = {}', NclusterNew, ListBestScore[-1])
            # iteration end if Score(N) < Score(N+1),Score(N+2),Score(N+3)
            #if len(ListBestScore) > 3 and \
            #   ListBestScore[-4] <= ListBestScore[-3] and \
            #   ListBestScore[-4] <= ListBestScore[-2] and \
            #   ListBestScore[-4] <= ListBestScore[-1]:
            if len(ListBestScore) >= 10:
                LOG.info('Determined the Number of Clusters to be {}', BestNcluster)
                converged = True
                break

        if converged is False:
            LOG.warn('Clustering analysis not converged. Number of clusters may be greater than upper limit (MaxCluster={})', MaxCluster)

        self.cluster_info['cluster_score'] = [ListNcluster, ListScore]
        self.cluster_info['detected_lines'] = Region2
        self.cluster_info['cluster_property'] = Bestlines # [[Center, Width/WHITEN, T/F, ClusterRadius],[],,,[]]
        self.cluster_info['cluster_scale'] = self.CLUSTER_WHITEN
        #SDP.ShowClusterScore(ListNcluster, ListScore, ShowPlot, FigFileDir, FigFileRoot)
        #SDP.ShowClusterInchannelSpace(Region2, Bestlines, self.CLUSTER_WHITEN, ShowPlot, FigFileDir, FigFileRoot)
        LOG.info('Final: Ncluster = {}, Score = {}, lines = {}', BestNcluster, BestScore, Bestlines)
        LOG.debug('Category = {}, CodeBook = {}', category, BestCodebook)

        return (BestNcluster, Bestlines, BestCategory, BestRegion)

    def clustering_hierarchy(self, Region, Region2, nThreshold=3.0, method='single'):
    #def calc_clustering(self, nThreshold, method='ward'):
        """
        Hierarchical Clustering
        method = 'ward'    : Ward's linkage method
                 'single'  : nearest point linkage method
                 'complete': farthest point linkage method
                 'average' : average linkage method
                 'centroid': centroid/UPGMC linkage method
                 'median'  : median/WPGMC linkage method
        1st threshold is set to nThreshold x stddev(distance matrix)
        in:
            self.Data -> Region2
            self.Ndata
        out:
            self.Threshold
            self.Nthreshold
            self.Category
            self.Ncluster
        """
        Data = self.set_data(Region2, ordering=[0,1]) # Data: numpy[[width, center],[w,c],,,]
        Repeat = 3 # Number of artificial detection points to normalize the cluster distance 
        # Calculate LinkMatrix from given data set
        if method.lower() == 'single': # nearest point linkage method
            H_Clustering = HIERARCHY.single
        elif method.lower() == 'complete': # farthest point linkage method
            H_Clustering = HIERARCHY.complete
        elif method.lower() == 'average': # average linkage method
            H_Clustering = HIERARCHY.average
        elif method.lower() == 'centroid': # centroid/UPGMC linkage method
            H_Clustering = HIERARCHY.centroid
        elif method.lower() == 'median': # median/WPGMC linkage method
            H_Clustering = HIERARCHY.median
        else: # Ward's linkage method: default
            H_Clustering = HIERARCHY.ward
        # temporaly add artificial detection points to normalize the cluster distance
        tmp = numpy.zeros((Data.shape[0]+Repeat*2, Data.shape[1]), numpy.float)
        tmp[Repeat*2:] = Data.copy()
        for i in range(Repeat):
            tmp[i] = [self.nchan/2, 0]
            tmp[Repeat+i] = [self.nchan/2, self.nchan-1]
        LOG.debug('tmp[:10] = {}', tmp[:10])
        tmpLinkMatrix = H_Clustering(tmp)
        MedianDistance = numpy.median(tmpLinkMatrix.T[2])
        MeanDistance = tmpLinkMatrix.T[2].mean()
        Stddev = tmpLinkMatrix.T[2].std()
        del tmp, tmpLinkMatrix
        LOG.debug('MedianDistance = {}, MeanDistance = {}, Stddev = {}', MedianDistance, MeanDistance, Stddev)
        LOG.debug('Ndata = {}', Data.shape[0])

        # Divide data set into several clusters
        # LinkMatrix[n][2]: distance between two data/clusters
        # 1st classification
        LinkMatrix = H_Clustering(Data)
        #MedianDistance = numpy.median(LinkMatrix.T[2])
        #Stddev = LinkMatrix.T[2].std()
        Nthreshold = nThreshold
        #Threshold = MedianDistance + Nthreshold * Stddev
        Threshold = MeanDistance + Nthreshold * Stddev
        Category = HIERARCHY.fcluster(LinkMatrix, Threshold, criterion='distance')
        Ncluster = Category.max()
        LOG.debug('nThreshold = {}, method = {}', nThreshold, method)
        LOG.debug('Init Threshold = {}, Init Ncluster = {}', Threshold, Ncluster)
        print('Init Threshold: {}'.format(Threshold), end=' ')
        print('\tInit Ncluster: {}'.format(Ncluster))

        IDX = numpy.array([x for x in xrange(len(Data))])
        for k in range(Ncluster):
            C = Category.max()
            NewData = Data[Category==(k+1)] # Category starts with 1 (not 0)
            if(len(NewData) < 2):
                print('skip(%d): %d' % (k, len(NewData)))
                continue # LinkMatrix = ()
            NewIDX = IDX[Category==(k+1)]
            LinkMatrix = H_Clustering(NewData) # selected linkage method
            #print LinkMatrix
            MedianDistance = numpy.median(LinkMatrix.T[2])
            MeanDistance = LinkMatrix.T[2].mean()
            #print 'MedianD', MedianDistance
            Stddev = LinkMatrix.T[2].std()
            LOG.debug('MedianDistance = {}, MeanDistance = {}, Stddev = {}', MedianDistance, MeanDistance, Stddev)
            #NewThreshold = MedianDistance + Nthreshold ** 2. * Stddev
            #NewThreshold = MedianDistance + Nthreshold ** 1.5 * Stddev
            #NewThreshold = MeanDistance + Nthreshold ** 1.5 * Stddev
            #NewThreshold = MeanDistance + Nthreshold * 2.0 * Stddev
            NewThreshold = MeanDistance + Nthreshold * 1.5 * Stddev
            #NewThreshold = MedianDistance + Nthreshold ** 1.3 * Stddev
            #NewThreshold = MedianDistance + Nthreshold * Stddev
            LOG.debug('Threshold({}): {}', k, NewThreshold)
            print('Threshold(%d): %.1f' % (k, NewThreshold), end=' ')
            NewCategory = HIERARCHY.fcluster(LinkMatrix, NewThreshold, criterion='distance')
            NewNcluster = NewCategory.max()
            LOG.debug('NewCluster({}): {}', k, NewNcluster)
            print('\tNewNcluster(%d): %d' % (k, NewNcluster), end=' ')
            print('\t# of Members(%d): %d: ' % (k, ((Category == k+1)*1).sum()), end=' ')
            for kk in range(NewNcluster):
                print(((NewCategory == kk+1)*1).sum(), end=' ')
            print('')
            if NewNcluster > 1:
                for i in range(len(NewData)):
                    if NewCategory[i] > 1:
                        Category[NewIDX[i]] = C + NewCategory[i] - 1
        Ncluster = Category.max() # update Ncluster

        (Region, Range, Stdev, Category) = self.clean_cluster(Data, Category, Region, Nthreshold, 2) # nThreshold, NumParam
        # 2017/7/25 ReNumbering is done in clean_cluster
        #for i in range(len(Category)):
        #    #if Category[i] > Ncluster: Region[i][5] = 0 # flag out cleaned data
        #    Category[i] -= 1 # Category starts with 1 -> starts with 0 (to align kmean)
        Bestlines = []
        Ncluster = len(Range)
        for j in range(Ncluster):
            Bestlines.append([Range[j][1], Range[j][0], True, Range[j][4]])
        LOG.info('Final: Ncluster = {}, lines = {}', Ncluster, Bestlines)

        self.cluster_info['cluster_score'] = [[1,2,3,4,5], [1,2,3,4,5]]
        self.cluster_info['detected_lines'] = Region2
        self.cluster_info['cluster_property'] = Bestlines # [[Center, Width, T/F, ClusterRadius],[],[],,,[]]
        self.cluster_info['cluster_scale'] = self.CLUSTER_WHITEN

        return (Ncluster, Bestlines, Category, Region)

    def set_data(self, Observation, ordering='none'):
        """
        Observation: numpy.array([[val1, val2, val3,..,valN],
                                  [val1, val2, val3,..,valN],
                                   ........................
                                  [val1, val2, val3,..,valN]], numpy.float)
        where N is a max dimensions of parameter space
            ordering: 'none' or list of ordering of columns
              e.g., ordering=[2,3,1,0] => [col3,col2,col0,col1]

        self.Data: Observation data
        self.NumParam: Number of dimensions to be used for Clustering Analysis
        self.Factor: Set default Whitening factor (to be 1.0)
        """
        if ordering != 'none':
            NumParam = len(ordering)
            OrderList = ordering
        else:
            NumParam = len(Observation[0])
            OrderList = range(NumParam)
        if type(Observation) == list:
            Obs = numpy.array(Observation, numpy.float)
        else: Obs = Observation.copy()
        if len(Obs.shape) == 2:
            Data = numpy.zeros((Obs.shape[0], NumParam), numpy.float)
            for i in range(Obs.shape[0]):
                for j in range(NumParam):
                    Data[i][j] = Obs[i][OrderList[j]]
            Factor = numpy.ones(NumParam, numpy.float)
            Ndata = len(Data)
        else:
            print("Data should be 2-dimensional...exit...")
            sys.exit(0)
        del Obs, OrderList
        return (Data)

    def clean_cluster(self, Data, Category, Region, Nthreshold, NumParam):
        """
        Clean-up cluster by eliminating outliers
         Radius = StandardDeviation * nThreshold (circle/sphere)
        in:
            Data
            Category
            Nthreshold
            Ncluster
        out:
            Region: flag information is added
            Range: Range[Ncluster][5]: [ClusterCenterX, ClusterCenterY, 0, 0, Threshold]
            Stdev: Stdev[Ncluster][5]: [ClusterStddevX, ClusterStddevY, 0, 0, 0]
            Category: renumbered category
        """
        IDX = numpy.array([x for x in xrange(len(Data))])
        Ncluster = Category.max()
        C = Ncluster + 1
        ValidClusterID = []
        ValidRange = []
        ValidStdev = []
        ReNumber = {}
        Range = numpy.zeros((C, 5), numpy.float)
        Stdev = numpy.zeros((C, 5), numpy.float)
        for k in range(Ncluster):
            NewData = Data[Category == k+1].T
            NewIDX = IDX[Category == k+1]
            for i in range(NumParam):
                Range[k][i] = NewData[i].mean()
                Stdev[k][i] = NewData[i].std()
            if(NumParam == 4):
                Tmp = ((NewData - numpy.array([[Range[k][0]],[Range[k][1]],[Range[k][2]],[Range[k][3]]]))**2).sum(axis=0)**0.5
            elif(NumParam == 3):
                Tmp = ((NewData - numpy.array([[Range[k][0]],[Range[k][1]],[Range[k][2]]]))**2).sum(axis=0)**0.5
            else: # NumParam == 2
                Tmp = ((NewData - numpy.array([[Range[k][0]],[Range[k][1]]]))**2).sum(axis=0)**0.5
            Threshold = numpy.median(Tmp) + Tmp.std() * Nthreshold
            #Threshold = Tmp.mean() + Tmp.std() * Nthreshold
            Range[k][4] = Threshold
            LOG.trace('Threshold({}) = {}', k, Threshold)
            Out = NewIDX[Tmp > Threshold]
            #if (len(NewIDX) - len(Out)) < 6: # max 3 detections for each binning: detected in two binning pattern
            if (len(NewIDX) - len(Out)) < 3: # max 3 detections for each binning: detected in at least one binning pattern: sensitive to very narrow lines
                LOG.trace('Non Cluster: {}', len(NewIDX))
                for i in NewIDX:
                    #self.Category[i] = C
                    Region[i][5] = 0
                ReNumber[k+1] = 0
                continue
            LOG.trace('Out Of Cluster ({}): {}', k, len(Out))
            if len(Out > 0):
                for i in Out:
                    #Category[i] = C
                    Region[i][5] = 0
            ReNumber[k+1] = len(ValidClusterID)
            ValidClusterID.append(k)
        for k in ValidClusterID:
            ValidRange.append(Range[k])
            ValidStdev.append(Stdev[k])
        LOG.debug('ReNumbering Table: {}', ReNumber)
        for j in range(len(Category)): Category[j] = ReNumber[Category[j]]
        #return (Region, Range, Stdev)
        return (Region, numpy.array(ValidRange), numpy.array(ValidStdev), Category)
    
    def clustering_kmean_score(self, MeanDistance, MedianWidth, Ncluster, MemberRate):
        # Rating
        ### 2011/05/12 modified for (distance==0)
        ### 2014/11/28 further modified for (distance==0)
        ### 2017/07/05 modified to be sensitive to MemberRate
        # (distance * numpy.transpose(Region[5])).mean(): average distance from each cluster center
        #return(math.sqrt(((distance * numpy.transpose(numpy.array(Region))[5]).mean())**2.0 + (MedianWidth/2.0)**2.0) * (Ncluster+ 1.0/Ncluster) * (((1.0 - MemberRate)**0.5 + 1.0)**2.0))
        return(math.sqrt(MeanDistance**2.0 + (MedianWidth/2.0)**2.0) * (Ncluster+ 1.0/Ncluster) * ((1.0 - MemberRate) * 100.0 + 1.0))

    def detection_stage(self, Ncluster, nra, ndec, ra0, dec0, grid_ra, grid_dec, category, Region, detect_signal):
        """
        Region = [[row, chan0, chan1, RA, DEC, flag, Binning],[],[],,,[]]
        detect_signal = {ID1: [RA, DEC, [[LineStartChannel1, LineEndChannel1, Binning],
                                         [LineStartChannel2, LineEndChannel2, Binning],
                                         [LineStartChannelN, LineEndChannelN, Binning]]],
                         IDn: [RA, DEC, [[LineStartChannel1, LineEndChannel1, Binning],
                                         [LineStartChannelN, LineEndChannelN, Binning]]]}
        """
        # Create Grid Parameter Space (Ncluster * nra * ndec)
        MinChanBinSp = 50.0
        BinningVariation = 1 + int(math.ceil(math.log(self.nchan/MinChanBinSp)/math.log(4)))
        GridClusterWithBinning = numpy.zeros((Ncluster, BinningVariation, nra, ndec), dtype=numpy.float32)
        #GridCluster = numpy.zeros((Ncluster, nra, ndec), dtype=numpy.float32)
        GridMember = numpy.zeros((nra, ndec))

        # Set the number of spectra belong to each gridding positions
        for row in xrange(len(detect_signal)):
            GridMember[int((detect_signal[row][0] - ra0)/grid_ra)][int((detect_signal[row][1] - dec0)/grid_dec)] += 1

        for i in xrange(len(category)):
            if Region[i][5] == 1: # valid spectrum
                # binning = 4**n
                n = int(math.log(Region[i][6])/math.log(4.) + 0.1)
                # if binning larger than 1, detection is done twice: m=>0.5
                if n == 0: m = 1.0 # case binning=1
                else: m = 0.5
                try:
                    #2014/11/28 Counting is done for each binning separately
                    GridClusterWithBinning[category[i]][n][int((Region[i][3] - ra0)/grid_ra)][int((Region[i][4] - dec0)/grid_dec)] += m
                    #GridCluster[category[i]][int((Region[i][3] - ra0)/grid_ra)][int((Region[i][4] - dec0)/grid_dec)] += 1.0
                except IndexError:
                    pass
        GridCluster = GridClusterWithBinning.max(axis=1)
        #2014/11/28 select the largest value among different Binning
        #2017/7/4 normalize by the number of spectra in the grid
        #x1, x2, x3 = GridCluster.shape
        #for i in range(x1):
        #    GridCluster[i] /= GridMember
        #    for j in range(x2):
        #        for k in range(x3):
        #            if GridCluster[i][j][k] > 1.0: GridCluster[i][j][k] = 1.0
        #for i in xrange(Ncluster):
        #    for j in xrange(nra):
        #        for k in xrange(ndec):
        #            m = 0
        #            for l in xrange(BinningVariation):
        #                if GridClusterWithBinning[i][l][j][k] > m: m = GridClusterWithBinning[i][l][j][k]
        #            GridCluster[i][j][k] = m

        LOG.trace('GridClusterWithBinning = {}', GridClusterWithBinning)
        LOG.trace('GridCluster = {}', GridCluster)
        LOG.trace('GridMember = {}', GridMember)
        del GridClusterWithBinning
        # 2013/05/29 TN
        # cluster_flag is data for plotting clustering analysis results.
        # It stores GridCluster quantized by given thresholds.
        # it is defined as integer array and one digit is assigned to
        # one clustering stage in each integer value:
        #
        #     1st digit: detection
        #     2nd digit: validation
        #     3rd digit: smoothing
        #     4th digit: final
        #
        # If GridCluster value exceeds any threshold, corresponding
        # digit is incremented. For example, flag 3210 stands for,
        # 
        #     value didn't exceed any thresholds in detection, and 
        #     exceeded one (out of three) threshold in validation, and
        #     exceeded two (out of three) thresholds in smoothing, and
        #     exceeded three (out of four) thresholds in final.
        #
        self.cluster_info['cluster_flag'] = numpy.zeros(GridCluster.shape, dtype=numpy.uint16)
        threshold = [1.5, 0.5]
        self.__update_cluster_flag('detection', GridCluster, threshold, 1)
        
        return (GridCluster, GridMember)

    def validation_stage(self, GridCluster, GridMember, lines):
        # Validated if number of spectrum which contains feature belongs to the cluster is greater or equal to
        # the half number of spectrum in the Grid
        # Normally, 3 spectra are created for each grid positions,
        # therefore, gridmember[ra][dec] = 3 for most of the cases.
        # Normalized validity can be
        # 1/3 (0.2<V) -> only one detection -> Qestionable
        # 2/3 (0.5<V)-> two detections -> Marginal
        # 3/3 (0.7<V)-> detected for all spectra -> Valid
        # ThresholdValid should be 0.5 -> 0.7 in the future

        (Ncluster,nra,ndec) = GridCluster.shape
        MinChanBinSp = 50.0
        BinningVariation = 1 + int(math.ceil(math.log(self.nchan/MinChanBinSp)/math.log(4)))

        for Nc in range(Ncluster):
            LOG.trace('GridCluster[Nc]: {}', GridCluster[Nc])
            LOG.trace('Gridmember: {}', GridMember)
            for x in range(nra):
                for y in range(ndec):
                    if GridMember[x][y] == 0: GridCluster[Nc][x][y] = 0.0
                    elif GridMember[x][y] == 1 and GridCluster[Nc][x][y] > 0.9:
                        GridCluster[Nc][x][y] = 1.0
                    ### 2014/11/28 Binning valiation is taken into account in the previous stage
                    # normarize validity
                    #else: GridCluster[Nc][x][y] /= float(GridMember[x][y])
                    else: GridCluster[Nc][x][y] = min(GridCluster[Nc][x][y] / float(GridMember[x][y]), 1.0)

            if ((GridCluster[Nc] > self.Questionable)*1).sum() == 0: lines[Nc][2] = False

        threshold = [self.Valid, self.Marginal, self.Questionable]
        self.__update_cluster_flag('validation', GridCluster, threshold, 10)
        LOG.trace('GridCluster {}', GridCluster)
        LOG.trace('GridMember {}', GridMember)
        self.GridClusterValidation = GridCluster.copy()
        
        return (GridCluster, GridMember, lines)

    def smoothing_stage(self, GridCluster, lines):
        # Rating:  [0.0, 0.4, 0.5, 0.4, 0.0]
        #          [0.4, 0.7, 1.0, 0.7, 0.4]
        #          [0.5, 1.0, 6.0, 1.0, 0.5]
        #          [0.4, 0.7, 1.0, 0.7, 0.4]
        #          [0.0, 0.4, 0.5, 0.4, 0.0]
        # Rating = 1.0 / (Dx**2 + Dy**2)**(0.5) : if (Dx, Dy) == (0, 0) rating = 6.0
        # NewRating[0.0, 0.2, 0.3, 0.2, 0.0]
        #          [0.2, 0.5, 1.0, 0.5, 0.2]
        #          [0.3, 1.0, 6.0, 1.0, 0.3]
        #          [0.2, 0.5, 1.0, 0.5, 0.2]
        #          [0.0, 0.2, 0.3, 0.2, 0.0]
        # NewRating = 1.0 / (Dx**2 + Dy**2) : if (Dx, Dy) == (0, 0) rating = 6.0
        (Ncluster,nra,ndec) = GridCluster.shape
        GridScore = numpy.zeros((2, nra, ndec), dtype=numpy.float32)
        LOG.trace('GridCluster = {}', GridCluster)
        LOG.trace('lines = {}', lines)
        for Nc in xrange(Ncluster):
            if lines[Nc][2] != False:
                GridScore[:] = 0.0
                for x in xrange(nra):
                    range_x = range(-min(2,x),0) + range(1,min(3,nra-x))
                    for y in xrange(ndec):
                        range_y = range(-min(2,y),0) + range(1,min(3,ndec-y))
                        # TN refactoring 
                        # split smoothing loop
                        # dx = 0 and dy = 0
                        GridScore[0][x][y] += 6.0 * GridCluster[Nc][x][y]
                        GridScore[1][x][y] += 6.0
                        # dx = 0
                        for dy in range_y:
                            ny = y + dy
                            #Rating = 1.0 / abs(dy)
                            Rating = 1.0 / abs(dy*dy)
                            GridScore[0][x][y] += Rating * GridCluster[Nc][x][ny]
                            GridScore[1][x][y] += Rating
                        # dy = 0
                        for dx in range_x:
                            nx = x + dx
                            #Rating = 1.0 / abs(dx)
                            Rating = 1.0 / abs(dx*dx)
                            GridScore[0][x][y] += Rating * GridCluster[Nc][nx][y]
                            GridScore[1][x][y] += Rating
                        # dx != 0 and dy != 0
                        for dx in range_x:
                            for dy in range_y:
                                if (abs(dx) + abs(dy)) <= 3:
                                    (nx, ny) = (x + dx, y + dy)
                                    #Rating = 1.0 / sqrt(dx*dx + dy*dy)
                                    Rating = 1.0 / (dx*dx + dy*dy)
                                    GridScore[0][x][y] += Rating * GridCluster[Nc][nx][ny]
                                    GridScore[1][x][y] += Rating
                        #for dx in [-2, -1, 0, 1, 2]:
                        #    for dy in [-2, -1, 0, 1, 2]:
                        #       if (abs(dx) + abs(dy)) <= 3:
                        #           (nx, ny) = (x + dx, y + dy)
                        #           if 0 <= nx < nra and 0 <= ny < ndec:
                        #               if dx == 0 and dy == 0: Rating = 6.0
                        #               else: Rating = 1.0 / sqrt(dx*dx + dy*dy)
                        #               GridScore[0][x][y] += Rating * GridCluster[Nc][nx][ny]
                        #               GridScore[1][x][y] += Rating
                LOG.trace('Score :  GridScore[{}][0] = {}', Nc, GridScore[0])
                LOG.trace('Rating:  GridScore[{}][1] = {}', Nc, GridScore[1])
                #GridCluster[Nc] = GridScore[0] / GridScore[1]
                GridCluster[Nc] = GridScore[0] / GridScore[1] * 2.0 # for single valid detection
            if ((GridCluster[Nc] > self.Questionable)*1).sum() < 0.1: lines[Nc][2] = False

        threshold = [self.Valid, self.Marginal, self.Questionable]
        self.__update_cluster_flag('smoothing', GridCluster, threshold, 100)
        LOG.trace('threshold = {}', threshold)
        LOG.trace('GridCluster = {}', GridCluster)
        
        return (GridCluster, lines)

    def final_stage(self, GridCluster, GridMember, Region, Region2, lines, category, grid_ra, grid_dec, broad_component, xorder, yorder, x0, y0, Grid2SpectrumID, index_list, PosList):
                
        (Ncluster, nra, ndec) = GridCluster.shape
        xorder0 = xorder
        yorder0 = yorder

        LOG.trace('GridCluster = {}', GridCluster)
        LOG.trace('GridMember = {}', GridMember)
        LOG.trace('lines = {}', lines)
        LOG.info('Ncluster={}', Ncluster)

        # Dictionary for final output
        RealSignal = {}

        HalfGrid = 0.5 * sqrt(grid_ra*grid_ra + grid_dec*grid_dec)
        
        MinFWHM = self.MinFWHM
        MaxFWHM = self.MaxFWHM
        
        # for Channel Map velocity range determination 2014/1/12
        channelmap_range = []
        for i in range(len(lines)):
            channelmap_range.append(lines[i][:])

        for Nc in xrange(Ncluster):
            LOG.trace('------00------ Exec for Nth Cluster: Nc={}', Nc)
            LOG.trace('lines[Nc] = {}', lines[Nc])
            if lines[Nc][2] == False: continue
            Plane = (GridCluster[Nc] > self.Marginal) * 1
            if Plane.sum() == 0:
                lines[Nc][2] = False
                channelmap_range[Nc][2] = False
                continue
            Original = GridCluster[Nc].copy()
            # Clear GridCluster Nc-th plane
            GridCluster[Nc] *= 0.0

            # Clean isolated grids
            MemberList, Realmember = self.CleanIsolation(nra, ndec, Original, Plane, GridMember)
            if len(MemberList) == 0: continue

            # Blur each SubCluster with the radius of sqrt(Nmember/Pi) * ratio
            ratio = rules.ClusterRule['BlurRatio']
            # Set-up SubCluster
            for Ns in xrange(len(MemberList)): # Ns: SubCluster number
                LOG.trace('------01------ Ns={}', Ns)
                SubPlane = numpy.zeros((nra, ndec), dtype=numpy.float32)
                for (x, y) in MemberList[Ns]: SubPlane[x][y] = Original[x][y]
                ValidPlane, BlurPlane = self.DoBlur(Realmember[Ns], nra, ndec, SubPlane, ratio)

                LOG.debug('GridCluster.shape = {}', list(GridCluster.shape))
                LOG.trace('Original {}', Original)
                LOG.debug('SubPlane.shape = {}', list(SubPlane.shape))
                LOG.trace('SubPlane {}', SubPlane)
                LOG.debug('BlurPlane.shape = {}', list(BlurPlane.shape))
                LOG.trace('BlurPlane {}', BlurPlane)
                LOG.trace('ValidPlane {}', ValidPlane)
                LOG.trace('GridClusterV {}', self.GridClusterValidation[Nc])


                # 2D fit for each Plane
                # Use the data for fit if GridCluster[Nc][x][y] > self.Valid
                # Not use for fit but apply the value at the border if GridCluster[Nc][x][y] > self.Marginal

                (ylen, xlen) = self.GridClusterValidation[Nc].shape
                # 2017/9/21 GridClusterValidation should not be used for order determination
                # 0 <= xorder0,yorder0 <= 5, swap xorder0 and yorder0
                if xorder < 0: xorder0 = max(min(numpy.sum(ValidPlane, axis=0).max()-1, 5), 0)
                if yorder < 0: yorder0 = max(min(numpy.sum(ValidPlane, axis=1).max()-1, 5), 0)
                #if xorder < 0: xorder0 = max(min(numpy.sum(ValidPlane, axis=1).max()-1, 5), 0)
                #if yorder < 0: yorder0 = max(min(numpy.sum(ValidPlane, axis=0).max()-1, 5), 0)
                LOG.trace('(X,Y)order, order0 = ({}, {}) ({}, {})', xorder, yorder, xorder0, yorder0)

                # clear Flag
                for i in xrange(len(category)): Region[i][5] = 1

                #while ExceptionLinAlg:
                FitData = []
                LOG.trace('------02-1---- category={}, len(category)={}, ClusterNumber(Nc)={}, SubClusterNumber(Ns)={}', category, len(category), Nc, Ns)
                #Region format:([row, line[0], line[1], RA, DEC, flag])
                dummy = [tuple(Region[i][:5]) for i in xrange(len(category)) 
                         if category[i] == Nc and Region[i][5] == 1 and \
                            SubPlane[int((Region[i][3] - x0)/grid_ra)][int((Region[i][4] - y0)/grid_dec)] > self.Valid]
                LOG.trace('------02-2----- len(dummy)={}, dummy={}', len(dummy), dummy)
                if len(dummy) == 0: continue

                # same signal can be detected in a single row with multiple binning
                # in that case, take maximum width as a representative width (Lmax-Lmin)
                (Lrow, Lmin, Lmax, LRA, LDEC) = dummy[0]
                for i in xrange(1, len(dummy)):
                    if Lrow == dummy[i][0]:
                        Lmin, Lmax = (min(Lmin, dummy[i][1]), max(Lmax, dummy[i][2]))
                    else:
                        FitData.append([Lmin, Lmax, LRA, LDEC, 1])
                        (Lrow, Lmin, Lmax, LRA, LDEC) = dummy[i]
                FitData.append([Lmin, Lmax, LRA, LDEC, 1])
                del dummy
                # FitData format: [Chan0, Chan1, RA, DEC, flag]
                LOG.trace('FitData = {}', FitData)

                # Instantiate SVD solver
                #LOG.trace('2D Fit Order: xorder0={} yorder0={}', xorder0, yorder0)
                #solver = SVDSolver2D(xorder0, yorder0)

                # TN refactoring
                # Comment out the following if statement since 
                # 1) len(FitData) is always greater than 0. 
                #    Lee the code just above start of iteration.
                # 2) SingularMatrix is always False in this 
                #    loop. Exit the loop whenever SingularMatrix 
                #    is set to False. See code below.
                #if len(FitData) == 0 or SingularMatrix: break

                SingularMatrix = False
                for iteration in xrange(3): # iteration loop for 2D fit sigma flagging
                    LOG.trace('------03------ iteration={}', iteration)
                    # effective components of FitData
                    effective = [i for i in xrange(len(FitData)) if FitData[i][4] == 1]

                    # assertion
                    assert len(effective) > 0
                    
                    # prepare data for SVD fit
                    #xdata = numpy.array([FitData[i][2] for i in effective], dtype=numpy.float64)
                    #ydata = numpy.array([FitData[i][3] for i in effective], dtype=numpy.float64)
                    #lmindata = numpy.array([FitData[i][0] for i in effective], dtype=numpy.float64)
                    #lmaxdata = numpy.array([FitData[i][1] for i in effective], dtype=numpy.float64)
                    # 2017/9/26 Repeat solver.find_good_solution until not through exception by reducing xorder and yorder
                    SVDsolver = True
                    while(1):
                        LOG.trace('2D Fit Order: xorder0={} yorder0={}', xorder0, yorder0)
                        if(SVDsolver):
                            # Instantiate SVD solver
                            solver = SVDSolver2D(xorder0, yorder0)
                            # prepare data for SVD fit
                            xdata = numpy.array([FitData[i][2] for i in effective], dtype=numpy.float64)
                            ydata = numpy.array([FitData[i][3] for i in effective], dtype=numpy.float64)
                            lmindata = numpy.array([FitData[i][0] for i in effective], dtype=numpy.float64)
                            lmaxdata = numpy.array([FitData[i][1] for i in effective], dtype=numpy.float64)
                        else:
                            # 2017/9/28 old code is incerted for validation purpose
                            # make arrays for coefficient calculation
                            # Matrix    MM x A = B  ->  A = MM^-1 x B
                            M0 = numpy.zeros((xorder0 * 2 + 1) * (yorder0 * 2 + 1), dtype=numpy.float64)
                            M1 = numpy.zeros((xorder0 * 2 + 1) * (yorder0 * 2 + 1), dtype=numpy.float64)
                            B0 = numpy.zeros((xorder0 + 1) * (yorder0 + 1), dtype=numpy.float64)
                            B1 = numpy.zeros((xorder0 + 1) * (yorder0 + 1), dtype=numpy.float64)
                            MM0 = numpy.zeros([(xorder0 + 1) * (yorder0 + 1), (xorder0 + 1) * (yorder0 + 1)], dtype=numpy.float64)
                            MM1 = numpy.zeros([(xorder0 + 1) * (yorder0 + 1), (xorder0 + 1) * (yorder0 + 1)], dtype=numpy.float64)
                            for (Width, Center, x, y, flag) in FitData:
                                if flag == 1:
                                    for k in range(yorder0 * 2 + 1): 
                                        for j in range(xorder0 * 2 + 1): 
                                            M0[j + k * (xorder0 * 2 + 1)] += math.pow(x, j) * math.pow(y, k)
                                    for k in range(yorder0 + 1): 
                                        for j in range(xorder0 + 1): 
                                            B0[j + k * (xorder0 + 1)] += math.pow(x, j) * math.pow(y, k) * Center
                                    for k in range(yorder0 * 2 + 1): 
                                        for j in range(xorder0 * 2 + 1): 
                                            M1[j + k * (xorder0 * 2 + 1)] += math.pow(x, j) * math.pow(y, k)
                                    for k in range(yorder0 + 1): 
                                        for j in range(xorder0 + 1):
                                            B1[j + k * (xorder0 + 1)] += math.pow(x, j) * math.pow(y, k) * Width
                            # make Matrix MM0,MM1 and calculate A0,A1
                            for K in range((xorder0 + 1) * (yorder0 + 1)):
                                k0 = K % (xorder0 + 1)
                                k1 = int(K / (xorder0 + 1))
                                for J in range((xorder0 + 1) * (yorder0 + 1)):
                                    j0 = J % (xorder0 + 1)
                                    j1 = int(J / (xorder0 + 1))
                                    MM0[J, K] = M0[j0 + k0 + (j1 + k1) * (xorder0 * 2 + 1)]
                                    MM1[J, K] = M1[j0 + k0 + (j1 + k1) * (xorder0 * 2 + 1)]
                            LOG.trace('OLD:MM0 = {}', MM0.tolist())
                            LOG.trace('OLD:B0 = {}', B0.tolist())

                        try:
                            if(SVDsolver):
                                solver.set_data_points(xdata, ydata)
                                A0 = solver.find_good_solution(lmaxdata)
                                A1 = solver.find_good_solution(lmindata)
                                LOG.trace('SVD: A0={}', A0.tolist())
                                LOG.trace('SVD: A1={}', A1.tolist())
                                break
                                # verification
#                               diff_lmin = numpy.zeros(len(effective), dtype=numpy.float64)
#                               diff_lmax = numpy.zeros(len(effective), dtype=numpy.float64)
#                               for ieff in xrange(len(effective)):
#                                   eff = effective[ieff]
#                                   lmin_ex = FitData[eff][0]
#                                   lmax_ex = FitData[eff][1]
#                                   x = FitData[eff][2]
#                                   y = FitData[eff][3]
#                                   lmin_fit, lmax_fit = _eval_poly(xorder0+1, yorder0+1, x, y, A0, A1) 
#                                   diff_lmin[ieff] = abs((lmin_fit - lmin_ex) / lmin_ex)
#                                   diff_lmax[ieff] = abs((lmax_fit - lmax_ex) / lmax_ex)
#                               LOG.trace('SVD: Lmin difference: max %s, min %s, mean %s, std %s'%(diff_lmin.max(), diff_lmin.min(), diff_lmin.mean(), diff_lmin.std()))
#                               LOG.trace('SVD: Lmax difference: max %s, min %s, mean %s, std %s'%(diff_lmax.max(), diff_lmax.min(), diff_lmax.mean(), diff_lmax.std()))

                            else:
                                A0 = LA.solve(MM0, B0)
                                A1 = LA.solve(MM1, B1)
                                LOG.trace('OLD: A0={}', A0.tolist())
                                LOG.trace('OLD: A1={}', A1.tolist())
                                del MM0, MM1, B0, B1, M0, M1
                                break

                        except Exception, e:
                            LOG.trace('------04------ in exception loop SingularMatrix={}', SingularMatrix)
                            import traceback
                            LOG.trace(traceback.format_exc(e))
                            if xorder0 != 0 or yorder0 != 0:
                                xorder0 = max(xorder0 - 1, 0)
                                yorder0 = max(yorder0 - 1, 0)
                                LOG.info('Fit failed. Trying lower order ({}, {})', xorder0, yorder0)
                            else:
                                SingularMatrix = True
                                break
                    if SingularMatrix: break

                    # Calculate Sigma
                    # Sigma should be calculated in the upper stage
                    # Fit0: Center or Lmax, Fit1: Width or Lmin
                    Diff = []
                    # TN refactoring
                    # Calculation of Diff is duplicated here and 
                    # following clipping stage. So, evalueate Diff 
                    # only once here and reuse it in clipping.
                    for (Width, Center, x, y, flag) in FitData:
                        LOG.trace('{} {} {} {} {} {}', xorder0,yorder0,x,y,A0,A1)
                        (Fit0, Fit1) = _eval_poly(xorder0+1, yorder0+1, x, y, A0, A1)
                        Fit0 -= Center
                        Fit1 -= Width
                        Diff.append(sqrt(Fit0*Fit0 + Fit1*Fit1))
                    if len(effective) > 1:
                        npdiff = numpy.array(Diff)[effective]
                        Threshold = npdiff.mean()
                        Threshold += sqrt(numpy.square(npdiff - Threshold).mean()) * self.nsigma
                    else: 
                        Threshold = Diff[effective[0]] * 2.0
                    LOG.trace('Diff = {}', [Diff[i] for i in effective])
                    LOG.trace('2D Fit Threshold = {}', Threshold)

                    # Sigma Clip
                    NFlagged = 0
                    Number = len(FitData)
                    ### 2011/05/15
                    for i in xrange(Number):
                        # Reuse Diff
                        if Diff[i] <= Threshold:
                            FitData[i][4] = 1
                        else:
                            FitData[i][4] = 0
                            NFlagged += 1

                    LOG.trace('2D Fit Flagged/All = ({}, {})', NFlagged, Number)
                    #2009/10/15 compare the number of the remainder and fitting order
                    if (Number - NFlagged) <= max(xorder0, yorder0) or Number == NFlagged:
                        SingularMatrix = True
                        break
                    #2017/9/27 exit if there is update (no flag, or flag number is the same to the previous iteration)
                    if NFlagged == 0: break
                    if iteration == 0: NFlagOrg = NFlagged
                    elif NFlagOrg == NFlagged: break
                # Iteration End

                ### 2011/05/15 Fitting is no longer (Width, Center) but (minchan, maxChan)
                LOG.trace('------06------ End of Iteration: SingularMatrix={}', SingularMatrix)
                if SingularMatrix: # skip for next Ns (SubCluster)
                    LOG.trace('------06b----- Skip for the next SubCluster')
                    continue

                LOG.trace('------07------ SingularMatrix=False')
                # Clear FitData and restore all relevant data
                # FitData: [(Chan0, Chan1, RA, DEC, Flag)]
                FitData = []
                for i in range(len(category)):
                    if category[i] == Nc and Region[i][5] == 1 and SubPlane[int((Region[i][3] - x0)/grid_ra)][int((Region[i][4] - y0)/grid_dec)] > self.Valid:
                        FitData.append(tuple(Region2[i][:5]))
                if len(FitData) == 0: continue

                # for Channel Map velocity range determination 2014/1/12
                (MaskMin, MaskMax) = (10000.0, 0.0)
                # Calculate Fit for each position
                LOG.trace('------08------ Calc Fit for each pos')
                for x in xrange(nra):
                    for y in xrange(ndec):
                        if ValidPlane[x][y] == 1:
                            LOG.trace('------09------ in ValidPlane x={} y={}', x, y)
                            for PID in Grid2SpectrumID[x][y]:
                                ID = index_list[PID]
                                ### 2011/05/15 (Width, Center) -> (minchan, maxChan)
                                (Chan1, Chan0) = _eval_poly(xorder0+1, yorder0+1, PosList[0][PID], PosList[1][PID], A0, A1) 
                                Fit0 = 0.5 * (Chan0 + Chan1)
                                Fit1 = (Chan1 - Chan0) + 1.0
                                LOG.trace('Fit0, Fit1 = {}, {}', Fit0, Fit1)
                                # 2015/04/23 remove MaxFWHM check
                                if (Fit1 >= MinFWHM): # and (Fit1 <= MaxFWHM):
                                    ProtectMask = self.calc_ProtectMask(Fit0, Fit1, self.nchan, MinFWHM, MaxFWHM)
                                    # for Channel map velocity range determination 2014/1/12
                                    MaskCen = (ProtectMask[0] + ProtectMask[1]) / 2.0
                                    if MaskMin > MaskCen: MaskMin = max(0, MaskCen)
                                    if MaskMax < MaskCen: MaskMax = min(self.nchan - 1, MaskCen)

                                    if RealSignal.has_key(ID):
                                        RealSignal[ID][2].append(ProtectMask)
                                    else:
                                        RealSignal[ID] = [PosList[0][PID], PosList[1][PID] ,[ProtectMask]]
                                else: LOG.trace('------10------ out of range Fit0={} Fit1={}', Fit0,Fit1)
                        elif BlurPlane[x][y] == 1:
                            LOG.trace('------11------ in BlurPlane x={} y={}', x, y)
                            # in Blur Plane, Fit is not extrapolated, 
                            # but use the nearest value in Valid Plane
                            # Search the nearest Valid Grid
                            Nearest = []
                            square_aspect = grid_ra / grid_dec
                            square_aspect *= square_aspect
                            Dist2 = numpy.inf
                            for xx in range(nra):
                                for yy in range(ndec):
                                    if ValidPlane[xx][yy] == 1:
                                        Dist3 = (xx-x)*(xx-x)*square_aspect + (yy-y)*(yy-y)
                                        if Dist2 > Dist3:
                                            Nearest = [xx, yy]
                                            Dist2 = Dist3
                            (RA0, DEC0) = (x0 + grid_ra * (x + 0.5), y0 + grid_dec * (y + 0.5))
                            (RA1, DEC1) = (x0 + grid_ra * (Nearest[0] + 0.5), y0 + grid_dec * (Nearest[1] + 0.5))

                            # Setup the position near the border
                            RA2 = RA1 - (RA1 - RA0) * HalfGrid / sqrt(Dist2)
                            DEC2 = DEC1 - (DEC1 - DEC0) * HalfGrid / sqrt(Dist2)
                            LOG.trace('[X,Y],[XX,YY] = [{},{}],{}', x,y,Nearest)
                            LOG.trace('(RA0,DEC0),(RA1,DEC1),(RA2,DEC2) = ({:.5},{:.5}),({:.5},{:.5}),({:.5},{:.5})',RA0,DEC0,RA1,DEC1,RA2,DEC2)
                            # Calculate Fit and apply same value to all the spectra in the Blur Grid
                            ### 2011/05/15 (Width, Center) -> (minchan, maxChan)
                            # Border case
                            #(Chan0, Chan1) = _eval_poly(xorder0+1, yorder0+1, RA2, DEC2, A0, A1)
                            # Center case
                            (Chan1, Chan0) = _eval_poly(xorder0+1, yorder0+1, RA1, DEC1, A0, A1)
                            Fit0 = 0.5 * (Chan0 + Chan1)
                            Fit1 = (Chan1 - Chan0)
                            LOG.trace('Fit0, Fit1 = {}, {}', Fit0, Fit1)
                            # 2015/04/23 remove MaxFWHM check
                            if (Fit1 >= MinFWHM): # and (Fit1 <= MaxFWHM):
                                ProtectMask = self.calc_ProtectMask(Fit0, Fit1, self.nchan, MinFWHM, MaxFWHM)

                                for PID in Grid2SpectrumID[x][y]:
                                    ID = index_list[PID]
                                    if RealSignal.has_key(ID):
                                        RealSignal[ID][2].append(ProtectMask)
                                    else:
                                        RealSignal[ID] = [PosList[0][PID], PosList[1][PID] ,[ProtectMask]]
                            else:
                                LOG.trace('------12------ out of range Fit0={} Fit1={}', Fit0,Fit1)
                                continue

                # Add every SubClusters to GridCluster just for Plot
                GridCluster[Nc] += BlurPlane
                #if not SingularMatrix: GridCluster[Nc] += BlurPlane

                if ((GridCluster[Nc] > 0.5)*1).sum() < self.Questionable or MaskMax == 0.0:
                    lines[Nc][2] = False
                    channelmap_range[Nc][2] = False
                # for Channel map velocity range determination 2014/1/12 arbitrary factor 0.8
                #channelmap_range[Nc][1] = (MaskMax - MaskMin - 10) * 0.8
                #channelmap_range[Nc][1] = MaskMax - MaskMin + lines[Nc][1] / 2.0
                # MaskMax-MaskMin is an maximum offset of line center
                channelmap_range[Nc][1] = MaskMax - MaskMin + lines[Nc][1]
                LOG.info('Nc, MaskMax, Min: {}, {}, {}', Nc, MaskMax, MaskMin)
                LOG.info('channelmap_range[Nc]: {}', channelmap_range[Nc])
                LOG.info('lines[Nc]: {}', lines[Nc])

                for x in range(nra):
                    for y in range(ndec):
                        if Original[x][y] > self.Valid: GridCluster[Nc][x][y] = 2.0
                        elif GridCluster[Nc][x][y] > 0.5: GridCluster[Nc][x][y] = 1.0

        threshold = [1.5, 0.5, 0.5, 0.5]
        self.__update_cluster_flag('final', GridCluster, threshold, 1000)
        
        return (RealSignal, lines, channelmap_range)

    def CleanIsolation(self, nra, ndec, Original, Plane, GridMember):
        """
        Clean isolated cluster
        return: if no cluster is left after the cleaning, n == 0
          MemberList contains positions of cluster member with Marginal+Valid detection
          MemberList[n]: [[(x00,y00),(x01,y01),........,(x0k-1,y0k-1)],
                          [(x10,y10),(x11,y11),..,(x1i-1,y1i-1)],
                                  ......
                          [(xn-10,yn-10),(xn-11,yn-11),..,(xn-1i-1,yn-1j-1)]]
          Realmember contains positions of cluster member with only Valid detection
          Realmember[n]: [[(x00,y00),(x01,y01),..,(x0a-1,y0a-1)],
                          [(x10,y10),(x11,y11),..,(x1b-1,y1b-1)],
                                  ......
                          [(xn-10,yn-10),(xn-11,yn-11),..,(xn-1c-1,yn-1c-1)]]
        """
        Nmember = []    # number of positions where value > self.Marginal in each SubCluster
        Realmember = [] # number of positions where value > self.Valid in each SubCluster
        MemberList = []
        NsubCluster = 0
        # Separate cluster members into several SubClusters by spacial connection
        for x in xrange(nra):
            for y in xrange(ndec):
                if Plane[x][y] == 1:
                    Plane[x][y] = 2
                    SearchList = [(x, y)]
                    M = 1
                    if Original[x][y] > self.Valid: MM = 1
                    else: MM = 0
                    MemberList.append([(x, y)])
                    while(len(SearchList) != 0):
                        cx, cy = SearchList[0]
                        #for dx in [-1, 0, 1]:
                        for dx in xrange(-min(1,cx),min(2,nra-cx)):
                            #for dy in [-1, 0, 1]:
                            for dy in xrange(-min(1,cy),min(2,ndec-cy)):
                                (nx, ny) = (cx + dx, cy + dy)
                                #if 0 <= nx < nra and 0 <= ny < ndec and Plane[nx][ny] == 1:
                                if Plane[nx][ny] == 1:
                                    Plane[nx][ny] = 2
                                    SearchList.append((nx, ny))
                                    M += 1
                                    if Original[nx][ny] > self.Valid: MM += 1
                                    MemberList[NsubCluster].append((nx, ny))
                        del SearchList[0]
                    Nmember.append(M)
                    Realmember.append(MM)
                    NsubCluster += 1

        if len(Nmember) > 0:
            # Threshold is set to half the number of the largest cluster in the plane
            #Threshold = max(Nmember) / 2.0
            Threshold = min(0.5 * max(Realmember), 3)
            for n in xrange(NsubCluster -1, -1, -1):
                # isolated cluster made from single spectrum should be omitted
                if Nmember[n] == 1:
                    (x, y) = MemberList[n][0]
                    if GridMember[x][y] <= 1:
                        Nmember[n] = 0
                # Sub-Cluster whose member below the threshold is cleaned
                if Nmember[n] < Threshold:
                    for (x, y) in MemberList[n]:
                        Plane[x][y] == 0
                    del Nmember[n], Realmember[n], MemberList[n]
        return MemberList, Realmember

    def DoBlur(self, Realmember, nra, ndec, SubPlane, ratio):
        # Calculate Blur radius
        BlurF = sqrt(Realmember / 3.141592653) * ratio + 1.5
        Blur = int(BlurF)
        # Set-up kernel for convolution
        # caution: if nra < (Blur*2+1) and ndec < (Blur*2+1)
        #  => dimension of SPC.convolve2d(Sub,kernel) gets not (nra,ndec) but (Blur*2+1,Blur*2+1)
        if nra < (Blur * 2 + 1) and ndec < (Blur * 2 + 1): Blur = int((max(nra, ndec) - 1) / 2)
        kernel = numpy.zeros((Blur * 2 + 1, Blur * 2 + 1),dtype=int)
        for x in xrange(Blur * 2 + 1):
            dx = Blur - x
            for y in xrange(Blur * 2 + 1):
                dy = Blur - y
                if sqrt(dx*dx + dy*dy) <= BlurF:
                    kernel[x][y] = 1
        # ValidPlane is used for fitting parameters
        # BlurPlane is not used for fitting but just extend the parameter determined in ValidPlane
        return (SubPlane > self.Valid) * 1, (convolve2d(SubPlane, kernel) > self.Marginal) * 1

    def calc_ProtectMask(self, Center, Width, nchan, MinFWHM, MaxFWHM):
        """
        return ProtectMask: [MaskL, MaskR]
        """
        #Allowance = Fit1 / 2.0 * 1.3
        # To keep broad line region, make allowance larger
        ### 2011/05/16 allowance + 0.5 ->  2.5 for sharp line
        ### 2011/05/16 factor 1.5 -> 2.0 for broad line
        #Allowance = min(Fit1 / 2.0 * 2.0, MaxFWHM / 2.0)
        ### 2011/11/22 Allowance is too narrow for new line finding algorithm
        #Allowance = min(Fit1 + 5.0, self.MaxFWHM / 2.0)
        ### 2015/04/23 Allowance=MaxFWHM at x=MaxFWHM, Allowance=2xMinFWHM+10 at x=MinFWHM
        Allowance = ((MaxFWHM-Width)*(2.0*MinFWHM+10.0) + (Width-MinFWHM)*MaxFWHM) / (MaxFWHM-MinFWHM) / 2.0
        ### 2011/10/21 left side mask exceeded nchan
        ProtectMask = [min(max(int(Center - Allowance), 0), nchan - 1), min(int(Center + Allowance), nchan - 1)]
        #Allowance = Fit1 / 2.0 * 1.5
        #ProtectMask = [max(int(Fit0 - Allowance + 0.5), 0), min(int(Fit0 + Allowance + 0.5), nchan - 1)]
        LOG.trace('Allowance = %s ProtectMask = %s' % (Allowance, ProtectMask))
        return ProtectMask
        
    def __merge_lines(self, lines, nchan):
        nlines = len(lines)
        if nlines < 1:
            return []
        elif nlines < 2:
            return lines
        else:
            # TN refactoring
            # New line merge algorithm that doesn't require 1-d array with 
            # length of nchan+2. It will be faster if nchan is large while 
            # it would be slow when number of lines is (extremely) large.
            nlines *= 2
            flat_lines = numpy.array(lines).reshape((nlines))
            sorted_index = flat_lines.argsort()
            flag = -1
            left_edge = flat_lines[sorted_index[0]]
            nedges=0
            for i in xrange(1,nlines-2):
                if sorted_index[i] % 2 == 0:
                    flag -= 1
                else:
                    #flag = min(0, flag + 1)
                    flag += 1
                if flag == 0 and flat_lines[sorted_index[i]] != flat_lines[sorted_index[i+1]]:
                    sorted_index[nedges] = left_edge
                    sorted_index[nedges+1] = flat_lines[sorted_index[i]]
                    nedges += 2
                    left_edge = flat_lines[sorted_index[i+1]]
            sorted_index[nedges] = left_edge
            sorted_index[nedges+1] = flat_lines[sorted_index[-1]]
            nedges += 2
            return sorted_index[:nedges].reshape((nedges/2,2)).tolist()
            #region = numpy.ones(nchan + 2, dtype=int)
            #for [chan0, chan1] in lines:
            #    region[chan0 + 1:chan1 + 1] = 0
            #dummy = (region[1:] - region[:-1]).nonzero()[0]
            #return dummy.reshape((len(dummy)/2,2)).tolist()

    def __update_cluster_flag(self, stage, GridCluster, threshold, factor):
        cluster_flag = self.cluster_info['cluster_flag']
        for t in threshold:
            cluster_flag = cluster_flag + factor * (GridCluster > t)
        self.cluster_info['cluster_flag'] = cluster_flag
        self.cluster_info['%s_threshold'%(stage)] = threshold
        LOG.trace('cluster_flag = {}', cluster_flag)


def convolve2d( data, kernel, mode='nearest', cval=0.0 ):
    """
    2d convolution function.

    mode = 'nearest'  use nearest pixel value for pixels beyond the edge
           'constant' use cval for pixels beyond the edge
    """
    (ndx,ndy) = data.shape
    (nkx,nky) = kernel.shape
    edgex = int( 0.5 * ( nkx - 1 ) )
    edgey = int( 0.5 * ( nky - 1 ) )
    dummy = numpy.ones( (ndx+2*edgex,ndy+2*edgey), dtype=numpy.float64 ) * cval
    dummy[edgex:ndx+edgex,edgey:ndy+edgey] = data
    if ( mode == 'nearest' ):
        dummy[0:edgex,0:edgey] = data[0][0]
        dummy[0:edgex,edgey+ndy:] = data[0][ndy-1]
        dummy[edgex+ndx:,0:edgey] = data[ndx-1][0]
        dummy[edgex+ndx:,edgey+ndy:] = data[ndx-1][ndy-1]
        for i in xrange(ndx):
            dummy[i+edgex,0:edgey] = data[i][0]
            dummy[i+edgex,edgey+ndy:] = data[i][ndy-1]
        for i in xrange(ndy):
            dummy[0:edgex,i+edgey] = data[0][i]
            dummy[edgex+ndx:,i+edgey] = data[ndx-1][i]
    cdata = numpy.zeros( (ndx,ndy), dtype=numpy.float64 ) 
    for ix in xrange(ndx):
        for iy in xrange(ndy):
            for jx in xrange( nkx ):
                for jy in xrange( nky ):
                    idx = ix + jx
                    idy = iy + jy
                    val = dummy[idx][idy]
                    cdata[ix][iy] += kernel[jx][jy] * val
    return cdata


def _eval_poly(xorder, yorder, x, y, xcoeff, ycoeff):
    xpoly = 0.0
    ypoly = 0.0
    yk = 1.0
    idx = 0
    for k in xrange(yorder):
        xjyk = yk
        for j in xrange(xorder):
            #xjyk = math.pow(x, j) * math.pow(y, k)
            #xpoly += xjyk * xcoeff[j + k * xorder]
            #ypoly += xjyk * ycoeff[j + k * xorder]
            xpoly += xjyk * xcoeff[idx]
            ypoly += xjyk * ycoeff[idx]
            xjyk *= x
            idx += 1
        yk *= y
    return (xpoly, ypoly)


def _to_validated_lines(detect_lines):
    # conversion from [chmin, chmax] to [center, width, T/F]
    lines = []
    for line_prop in detect_lines.values():
        for line in line_prop[2]:
            if line not in lines:
                lines.append(line)
    lines_withflag = map(lambda x: [0.5*sum(x), x[1]-x[0], True], lines)
    return lines_withflag


class SVDSolver2D(object):
    CONDITION_NUMBER_LIMIT = 1.0e-12
    
    def __init__(self, xorder, yorder):
        self.xorder = xorder
        self.yorder = yorder

        assert 0 <= self.xorder 
        assert 0 <= self.yorder

        # internal storage for solver
        self.N = 0
        self.L = (self.xorder + 1) * (self.yorder + 1)

        # design matrix
        self.storage = numpy.empty(self.N * self.L, dtype=numpy.float64)
        self.G = None 

        # for SVD
        self.Vs = numpy.empty((self.L, self.L), dtype=numpy.float64)
        self.B = numpy.empty(self.L, dtype=numpy.float64)
        self.U = None
    
    def set_data_points(self, x, y):
        nx = len(x)
        ny = len(y)
        LOG.trace('nx, ny = {}, {}', nx, ny)
        assert nx == ny
        if self.N < nx:
            self.storage.resize(nx * self.L)
            #self.G.resize((nx, self.L))
        self.N = nx
        assert self.L <= self.N
        
        self.G = self.storage[:self.N * self.L].reshape((self.N, self.L))
        
        # matrix operation
        self._set_design_matrix(x, y)
        #self._svd()
    
    def _set_design_matrix(self, x, y):
        # The design matrix G is a basis array that stores gj(xi)
        # where g0  = 1,   g1  = x,     g2  = x^2      g3  = x^3,
        #       g4  = y,   g5  = x y,   g6  = x^2 y,   g7  = x^3 y
        #       g8  = y^2, g9  = x y^2, g10 = x^2 y^2, g11 = x^3 y^2
        #       g12 = y^3, g13 = x y^3, g14 = x^2 y^3, g15 = x^3 y^3
        # if xorder = 3 and yorder = 3
        for k in xrange(self.N):
            yp = 1.0
            for i in xrange(self.yorder + 1):
                xp = 1.0
                for j in xrange(self.xorder + 1):
                    l = j + (self.xorder + 1) * i
                    self.G[k,l] = xp * yp
                    xp *= x[k]
                yp *= y[k]
                
    def _do_svd(self):
        LOG.trace('G.shape={}', self.G.shape)
        self.U, self.s, self.Vh = LA.svd(self.G, full_matrices=False)
        LOG.trace('U.shape={} (N,L)=({},{})', self.U.shape, self.N, self.L)
        LOG.trace('s.shape={}', self.s.shape)
        LOG.trace('Vh.shape={}', self.Vh.shape)
        LOG.trace('s = {}', self.s)
        assert self.U.shape == (self.N, self.L)
        assert len(self.s) == self.L
        assert self.Vh.shape == (self.L, self.L)
        
    def _svd_with_mask(self, nmask=0):
        if not hasattr(self, 's'):
            # do SVD
            self._do_svd()
            
        assert nmask < self.L
        
        for icol in xrange(self.L):
            if self.L - 1 - icol < nmask:
                sinv = 0.0
            else:
                sinv = 1.0 / self.s[icol]
            for irow in xrange(self.L):
                self.Vs[irow, icol] = self.Vh[icol, irow] * sinv
                        
    def _svd_with_eps(self, eps=1.0e-7):
        if not hasattr(self, 's'):
            # do SVD
            self._do_svd()
            
        assert 0.0 < eps
        
        # max value of s is always s[0] since it is sorted 
        # in descendent order
        #threshold = self.s.max() * eps
        threshold = self.s[0] * eps
        for icol in xrange(self.L):
            if self.s[icol] < threshold:
                sinv = 0.0
            else:
                sinv = 1.0 / self.s[icol]
            for irow in xrange(self.L):
                self.Vs[irow, icol] = self.Vh[icol, irow] * sinv
                        
    def _svd(self, eps):
        LOG.trace('G.shape={}', self.G.shape)
        self.U, s, Vh = LA.svd(self.G, full_matrices=False)
        LOG.trace('U.shape={} (N,L)=({},{})', self.U.shape, self.N, self.L)
        LOG.trace('s.shape={}', s.shape)
        LOG.trace('Vh.shape={}', Vh.shape)
        #LOG.trace('U=%s'%(self.U))
        #LOG.trace('s=%s'%(s))
        #LOG.trace('Vh=%s'%(Vh))
        assert self.U.shape == (self.N, self.L)
        assert len(s) == self.L
        assert Vh.shape == (self.L, self.L)
        assert 0.0 < eps
        
#         absolute_s = abs(s)
#         condition_number = absolute_s.min() / absolute_s.max()
#         if condition_number < self.CONDITION_NUMBER_LIMIT:
#             LOG.trace('smax {}, smin {}, condition_number is {}', absolute_s.max(), absolute_s.min(), condition_number)
#             raise RuntimeError('singular matrix')
        
        threshold = s.max() * eps
        for i in xrange(self.L):
            if s[i] < threshold:
                s[i] = 0.0
            else:
                s[i] = 1.0 / s[i]
        for icol in xrange(self.L):
            for irow in xrange(self.L):
                self.Vs[irow, icol] = Vh[icol, irow] * s[icol]
        
    def _eval_poly_from_G(self, row, coeff):
        idx = 0 
        poly = 0.0
        for k in xrange(self.yorder + 1):
            for j in xrange(self.xorder + 1):
                poly += self.G[row,idx] * coeff[idx]
                idx += 1
        #LOG.trace('poly=%s'%(poly))
        return poly   
                
    def solve_with_mask(self, z, out=None, nmask=0):
        nz = len(z)
        assert nz == self.N
        
        self._svd_with_mask(nmask)
        
        if out is None:
            A = numpy.zeros(self.L, dtype=numpy.float64)
        else:
            A = out
            A[:] = 0
            assert len(A) == self.L
        self.B[:] = 0
        for i in xrange(self.L):
            for k in xrange(self.N):
                self.B[i] += self.U[k, i] * z[k]
        for i in xrange(self.L):
            for k in xrange(self.L):
                A[i] += self.Vs[i, k] * self.B[k] 
                
        #fit = numpy.fromiter((self._eval_poly_from_G(i, A) for i in xrange(self.N)), dtype=numpy.float64)   
        #LOG.trace('fit=%s'%(fit))
        #LOG.trace('diff=%s'%(abs(fit - z)/z))
        return A    
    
    def solve_with_eps(self, z, out=None, eps=1.0e-7):
        assert 0.0 <= eps
        
        nz = len(z)
        assert nz == self.N
        
        self._svd_with_eps(eps)
        
        if out is None:
            A = numpy.zeros(self.L, dtype=numpy.float64)
        else:
            A = out
            A[:] = 0
            assert len(A) == self.L
        self.B[:] = 0
        for i in xrange(self.L):
            for k in xrange(self.N):
                self.B[i] += self.U[k, i] * z[k]
        for i in xrange(self.L):
            for k in xrange(self.L):
                A[i] += self.Vs[i, k] * self.B[k] 
                
        #fit = numpy.fromiter((self._eval_poly_from_G(i, A) for i in xrange(self.N)), dtype=numpy.float64)   
        #LOG.trace('fit=%s'%(fit))
        #LOG.trace('diff=%s'%(abs(fit - z)/z))
        return A        
        
    def solve_for(self, z, out=None, eps=1.0e-7):
        assert 0.0 <= eps
        
        nz = len(z)
        assert nz == self.N
        
        self._svd(eps)
        
        if out is None:
            A = numpy.zeros(self.L, dtype=numpy.float64)
        else:
            A = out
            A[:] = 0
            assert len(A) == self.L
        self.B[:] = 0
        for i in xrange(self.L):
            for k in xrange(self.N):
                self.B[i] += self.U[k, i] * z[k]
        for i in xrange(self.L):
            for k in xrange(self.L):
                A[i] += self.Vs[i, k] * self.B[k] 
                
        #fit = numpy.fromiter((self._eval_poly_from_G(i, A) for i in xrange(self.N)), dtype=numpy.float64)   
        #LOG.trace('fit=%s'%(fit))
        #LOG.trace('diff=%s'%(abs(fit - z)/z))
        return A
    
    def find_good_solution(self, z, threshold=0.05):
        assert 0.0 <= threshold
        eps_list = map(lambda x: 10**x, xrange(-11, -3))
        
        best_score = 1e30
        best_eps = eps_list[0]
        intlog = lambda x: int(numpy.log10(x))
        ans = numpy.empty(self.L, dtype=numpy.float64)
        best_ans = numpy.empty(self.L, dtype=numpy.float64)
        diff = numpy.empty(self.N, dtype=numpy.float64)
        
        # do SVD
        self._do_svd()
        
        for eps in eps_list:
            ans = self.solve_with_eps(z, out=ans, eps=eps)
            for i in xrange(self.N):
                fit = self._eval_poly_from_G(i, ans)
                if z[i] != 0:
                    diff[i] = abs((fit - z[i]) / z[i])
                else:
                    diff[i] = fit
            #score = diff.max()
            score = diff.mean()
            LOG.trace('eps={}, score={}', intlog(eps),score)
            if best_ans is None or score < best_score:
                best_ans[:] = ans
                best_score = score
                best_eps = eps
        if 1.0 <= best_score:
            raise RuntimeError('No good solution is found.')
        elif threshold < best_score:
            LOG.trace('Score is higher than given threshold (threshold {}, score {})', threshold, best_score)
        
        LOG.trace('best eps: {} (score {})', intlog(best_eps), best_score)
        return best_ans
    
