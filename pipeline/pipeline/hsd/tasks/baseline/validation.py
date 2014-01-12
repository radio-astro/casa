from __future__ import absolute_import

import numpy
import math
from math import sqrt
import time
import scipy.cluster.vq as VQ
import numpy.linalg as LA

import pipeline.infrastructure as infrastructure
from . import rules
from .. import common

LOG = infrastructure.get_logger(__name__)

def ValidationFactory(pattern):
    if pattern == 'RASTER':
        return ValidateLineRaster
    elif pattern == 'SINGLE-POINT' or pattern == 'MULTI-POINT':
        return ValidateLineSinglePointing
    else:
        raise ValueError, 'Invalid observing pattern'

class ValidateLineInputs(common.SingleDishInputs):
    def __init__(self, context, grid_table, detect_signal, spwid, index_list, iteration, 
                 grid_ra, grid_dec,
                 window=None, edge=None, nsigma=None, xorder=None, yorder=None, broad_component=None):
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

class ValidateLineResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(ValidateLineResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(ValidateLineResults, self).merge_with_context(context)
        # replace and export datatable to merge updated data with context
        datatable = self.outcome.pop('datatable')
        datatable.exportdata(minimal=True)

    def _outcome_name(self):
        return ''

class ValidateLineSinglePointing(common.SingleDishTaskTemplate):
    Inputs = ValidateLineInputs

    def prepare(self):
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
        LOG.debug('window=%s'%(window))

        # for Pre-Defined Spectrum Window
        if len(window) != 0:
            LOG.info('Skip clustering analysis since predefined line window is set.')
            outcome = {'lines': [],
                       'cluster_info': {},
                       'datatable': self.datatable}
            result = ValidateLineResults(task=self.__class__,
                                         success=True,
                                         outcome=outcome)
                
            result.task = self.__class__
                
            if self.context.subtask_counter is 0: 
                result.stage_number = self.context.task_counter - 1
            else:
                result.stage_number = self.context.task_counter 
                
            return result

        # Dictionary for final output
        lines = []

        LOG.info('Accept all detected lines without clustering analysis.')

        iteration = self.inputs.iteration
        index_list = self.inputs.index_list
        detect_signal = self.inputs.detect_signal

        # First cycle
        #if len(grid_table) == 0:
        if iteration == 0:
            for row in index_list:
                mask_list = self.datatable.getcell('MASKLIST',row)
                no_change = self.datatable.getcell('NOCHANGE',row)
                #LOG.debug('DataTable = %s, detect_signal = %s, OldFlag = %s' % (mask_list, detect_signal[row][2], no_change))
                self.datatable.putcell('MASKLIST',row,detect_signal[row][2])
                self.datatable.putcell('NOCHANGE',row,False)

        # Iteration case
        else:
            for row in index_list:
                mask_list = self.datatable.getcell('MASKLIST',row)
                no_change = self.datatable.getcell('NOCHANGE',row)
                #LOG.debug('DataTable = %s, detect_signal = %s, OldFlag = %s' % (mask_list, detect_signal[0][2], no_change))
                if mask_list == detect_signal[0][2]:
                    if type(no_change) != int:
                        # 2013/05/17 TN
                        # Put iteration itself instead to subtract 1 since
                        # iteration counter is incremented *after* the
                        # baseline subtraction in refactorred code.
                        #self.datatable.putcell('NOCHANGE',row,iteration - 1)
                        self.datatable.putcell('NOCHANGE',row,iteration)
                else:
                    self.datatable.putcell('MASKLIST',row,detect_signal[0][2])
                    self.datatable.putcell('NOCHANGE',row,False)
        outcome = {'lines': lines,
                   'cluster_info': {},
                   'datatable': self.datatable}
        result = ValidateLineResults(task=self.__class__,
                                     success=True,
                                     outcome=outcome)
                
        result.task = self.__class__
                
        if self.context.subtask_counter is 0: 
            result.stage_number = self.context.task_counter - 1
        else:
            result.stage_number = self.context.task_counter 
                
        return result
    
    def analyse(self, result):
        return result

class ValidateLineRaster(common.SingleDishTaskTemplate):
    Inputs = ValidateLineInputs

    CLUSTER_WHITEN = 1.0

    Valid = rules.ClusterRule['ThresholdValid']
    Marginal = rules.ClusterRule['ThresholdMarginal']
    Questionable = rules.ClusterRule['ThresholdQuestionable']
    #2010/6/9 Delete global parameter Min/MaxFWHM
    MinFWHM = rules.LineFinderRule['MinFWHM']
    MaxFWHM = rules.LineFinderRule['MaxFWHM']

    def prepare(self):
        """
        2D fit line characteristics calculated in Process3
        Sigma clipping iterations will be applied if nsigma is positive
        order < 0 : automatic determination of fitting order (max = 5)

         detect_signal = {ID1: [RA, DEC, [[LineStartChannel1, LineEndChannel1],
                                         [LineStartChannel2, LineEndChannel2],
                                         [LineStartChannelN, LineEndChannelN]]],
                         IDn: [RA, DEC, [[LineStartChannel1, LineEndChannel1],
                                         [LineStartChannelN, LineEndChannelN]]]}

        lines: output parameter
           [LineCenter, LineWidth, Validity]  OK: Validity = True; NG: Validity = False
        """
        window = self.inputs.window

        # for Pre-Defined Spectrum Window
        if len(window) != 0:
            LOG.info('Skip clustering analysis since predefined line window is set.')
            outcome = {'lines': [],
                       'cluster_info': {},
                       'datatable': self.datatable}
            result = ValidateLineResults(task=self.__class__,
                                         success=True,
                                         outcome=outcome)
                
            if self.context.subtask_counter is 0: 
                result.stage_number = self.context.task_counter - 1
            else:
                result.stage_number = self.context.task_counter 
                
            return result

        iteration = self.inputs.iteration
        grid_table = self.inputs.grid_table
        detect_signal = self.inputs.detect_signal
        index_list = self.inputs.index_list
        grid_ra = self.inputs.grid_ra
        grid_dec = self.inputs.grid_dec
        nsigma = self.inputs.nsigma
        broad_component = self.inputs.broad_component
        xorder = self.inputs.xorder
        yorder = self.inputs.yorder
        nchan = self.datatable.getcell('NCHAN', index_list[0])

        #Abcissa = self.Abcissa[vIF]
        
        ProcStartTime = time.time()
        LOG.info('2D fit the line characteristics...')

        #tMASKLIST = None
        #tNOCHANGE = None
        #tRA = self.datatable.getcol('RA')
        #tDEC = self.datatable.getcol('DEC')
        tSFLAG = self.datatable.getcol('FLAG_SUMMARY')
        
        Totallines = 0
        RMS0 = 0.0
        lines = []
        self.cluster_info = {}

        # First cycle
        #if len(grid_table) == 0:
        #    ROWS = index_list
        # Iteration case
        #else:
        #    ROWS = range(len(grid_table))
        ROWS = range(len(grid_table))

        # RASTER CASE
        # Read data from Table to generate ID -> RA, DEC conversion table
        Region = []
        dummy = []
        flag = 1
        Npos = 0
        #Maxlines = 0
        ### 2011/05/13 Calculate median line width
        Width = []

        for row in ROWS:
            #if len(detect_signal[row][2]) > Maxlines: Maxlines = len(detect_signal[row][2])
            #Maxlines = max(Maxlines, len(detect_signal[row][2]))

            if detect_signal[row][2][0][0] != -1: Npos += 1

            for line in detect_signal[row][2]:
                # Check statistics flag added by G.K. 2008/1/17
                # Bug fix 2008/5/29
                if (line[0] != line[1]) and ((len(grid_table) == 0 and tSFLAG[row] == 1) or len(grid_table) != 0):
                    #Region.append([row, line[0], line[1], detect_signal[row][0], detect_signal[row][1], flag])
                    Region.append([row, line[0], line[1], detect_signal[row][0], detect_signal[row][1], flag])
                    ### 2011/05/17 make cluster insensitive to the line width
                    dummy.append([float(line[1] - line[0]) / self.CLUSTER_WHITEN, 0.5 * float(line[0] + line[1])])
                    #dummy.append([float(line[1] - line[0]), (line[0] + line[1]) / 2.0])
                    ### 2011/05/13 Calculate median line width
                    #Width.append(float(line[1] - line[0]) / self.CLUSTER_WHITEN)
        # Region2:[Width, Center]
        Region2 = numpy.array(dummy)
        ### 2011/05/13 Calculate median line width
        #MedianWidth = numpy.median(numpy.array(Width))
        ###MedianWidth = numpy.median(Region2[:,0])
        #LOG.debug('dummy = %s' % dummy)
        #LOG.debug('Width = %s' % Width)
        del dummy#, Width
        #LOG.debug('Maxlines = %s' % Maxlines)
        LOG.debug('Npos = %s' % Npos)
        ###LOG.debug('MedianWidth = %s' % MedianWidth)
        # 2010/6/9 for non-detection
        if Npos == 0: 
            #return lines, {}
            outcome = {'lines': [],
                       'cluster_info': {},
                       'datatable': self.datatable}
            result = ValidateLineResults(task=self.__class__,
                                         success=True,
                                         outcome=outcome)
                
            result.task = self.__class__
                
            if self.context.subtask_counter is 0: 
                result.stage_number = self.context.task_counter - 1
            else:
                result.stage_number = self.context.task_counter 
                
            return result

        # 2008/9/20 Dec Effect was corrected
        PosList = numpy.array([numpy.take(self.datatable.getcol('RA'),index_list),
                               numpy.take(self.datatable.getcol('DEC'),index_list)])
        #DecCorrection = 1.0 / math.cos(tDEC[0] / 180.0 * 3.141592653)
        DecCorrection = 1.0 / math.cos(PosList[1][0] / 180.0 * 3.141592653)
        grid_ra *= DecCorrection
        # Calculate Parameters for Gridding
        #PosList = numpy.array([numpy.take(tRA,index_list),
        #                    numpy.take(tDEC,index_list)])

        wra = PosList[0].max() - PosList[0].min()
        wdec = PosList[1].max() - PosList[1].min()
        cra = PosList[0].min() + wra/2.0
        cdec = PosList[1].min() + wdec/2.0
        # 2010/6/11 +1.0 -> +1.01: if wra is n x grid_ra (n is a integer), int(wra/grid_ra) is not n in some cases because of the lack of accuracy.
        nra = 2 * (int((wra/2.0 - grid_ra/2.0)/grid_ra) + 1) + 1
        ndec = 2 * (int((wdec/2.0 - grid_dec/2.0)/grid_dec) + 1) + 1
        x0 = cra - grid_ra/2.0 - grid_ra*(nra-1)/2.0
        y0 = cdec - grid_dec/2.0 - grid_dec*(ndec-1)/2.0
        LOG.debug('Grid = %d x %d\n' % (nra, ndec))
        self.cluster_info['grid'] = {}
        self.cluster_info['grid']['ra_min'] = x0
        self.cluster_info['grid']['dec_min'] = y0
        self.cluster_info['grid']['grid_ra'] = grid_ra
        self.cluster_info['grid']['grid_dec'] = grid_dec

        # Create Space for storing the list of spectrum (ID) in the Grid
        # 2013/03/27 TN
        # Grid2SpectrumID stores index of index_list instead of row numbers 
        # that are held by index_list.
        #Grid2SpectrumID = []
        #for x in range(nra):
        #    Grid2SpectrumID.append([])
        #    for y in range(ndec):
        #        Grid2SpectrumID[x].append([])
        Grid2SpectrumID = [[[] for y in xrange(ndec)] for x in xrange(nra)]
        for i in range(len(index_list)):
            Grid2SpectrumID[int((PosList[0][i] - x0)/grid_ra)][int((PosList[1][i] - y0)/grid_dec)].append(i)

        ProcEndTime = time.time()
        LOG.info('Clustering: Initialization End: Elapsed time = %s sec' % (ProcEndTime - ProcStartTime))

        # K-mean Clustering Analysis with LineWidth and LineCenter
        # Max number of protect regions are SDC.SDParam['Cluster']['MaxCluster'] (Max lines)
        ProcStartTime = time.time()
        LOG.info('K-mean Clustering Analaysis Start')

        (Ncluster, Bestlines, BestCategory, Region) = self.clustering_analysis(Region, Region2, nsigma)

        ProcEndTime = time.time()
        #LOG.info('Final: Ncluster = %s, Score = %s, lines = %s' % (Ncluster, BestScore, lines))
        LOG.info('K-mean Cluster Analaysis End: Elapsed time = %s sec' % (ProcEndTime - ProcStartTime))

        # Sort lines and Category by LineCenter: lines[][0]
        LineIndex = numpy.argsort([line[0] for line in Bestlines[:Ncluster]])
        lines = [Bestlines[i] for i in LineIndex]

        ### 2011/05/17 Scaling back the line width
        Region2[:,0] = Region2[:,0] * self.CLUSTER_WHITEN
        for Nc in range(Ncluster):
            lines[Nc][1] *= self.CLUSTER_WHITEN

        LineIndex2 = numpy.argsort(LineIndex)
        #for i in range(len(BestCategory)): category[i] = LineIndex2[BestCategory[i]]
        category = [LineIndex2[bc] for bc in BestCategory]

        ######## Clustering: Detection Stage ########
        ProcStartTime = time.time()
        LOG.info('Clustering: Detection Stage Start')

        (GridCluster, GridMember) = self.detection_stage(Ncluster, nra, ndec, x0, y0, grid_ra, grid_dec, category, Region, detect_signal)

        ProcEndTime = time.time()
        LOG.info('Clustering: Detection Stage End: Elapsed time = %s sec' % (ProcEndTime - ProcStartTime))

        ######## Clustering: Validation Stage ########
        ProcStartTime = time.time()
        LOG.info('Clustering: Validation Stage Start')

        (GridCluster, GridMember, lines) = self.validation_stage(GridCluster, GridMember, lines, iteration)
        
        ProcEndTime = time.time()
        LOG.info('Clustering: Validation Stage End: Elapsed time = %s sec' % (ProcEndTime - ProcStartTime))
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
        LOG.info('Clustering: Smoothing Stage End: Elapsed time = %s sec' % (ProcEndTime - ProcStartTime))
        
        ######## Clustering: Final Stage ########
        ProcStartTime = time.time()
        LOG.info('Clustering: Final Stage Start')

        (RealSignal, lines, channelmap_range) = self.final_stage(GridCluster, GridMember, Region, Region2, lines, category, grid_ra, grid_dec, broad_component, xorder, yorder, x0, y0, nsigma, nchan, Grid2SpectrumID, index_list, PosList)

        ProcEndTime = time.time()
        LOG.info('Clustering: Final Stage End: Elapsed time = %s sec' % (ProcEndTime - ProcStartTime))

        # Merge masks if possible
        ProcStartTime = time.time()
        LOG.info('Clustering: Merging Start')
        # RealSignal should have all row's as its key
        tmp_index = 0
        for row in index_list:
            if RealSignal.has_key(row):
                signal = self.__merge_lines(RealSignal[row][2], nchan)
            else:
                signal = [[-1,-1]]
                #RealSignal[row] = [PosList[0][tmp_index], PosList[1][tmp_index], signal]
            tmp_index += 1

            # In the following code, access to MASKLIST and NOCHANGE columns 
            # is direct to underlying table object instead of access via 
            # datatable object's method since direct access is faster. 
            # Note that MASKLIST [[-1,-1]] represents that no masks are 
            # available, while NOCHANGE -1 indicates NOCHANGE is False.
            #tMASKLIST = self.datatable.getcell('MASKLIST',row)
            #tNOCHANGE = self.datatable.getcell('NOCHANGE',row)
            tMASKLIST = self.datatable.tb2.getcell('MASKLIST',row)
            if tMASKLIST[0][0] < 0:
                tMASKLIST = []
            else:
                tMASKLIST=tMASKLIST.tolist()#list(tMASKLIST)
            tNOCHANGE = self.datatable.tb2.getcell('NOCHANGE',row)
            #LOG.debug('DataTable = %s, RealSignal = %s' % (tMASKLIST, signal))
            if tMASKLIST == signal:
                #LOG.debug('No update on row %s: iter is %s'%(row,iteration))
                #if type(tNOCHANGE) != int:
                if tNOCHANGE < 0:
                    # 2013/05/17 TN
                    # Put iteration itself instead to subtract 1 since iteration
                    # counter is incremented *after* baseline subtraction
                    # in refactorred code.
                    #self.datatable.tb2.putcell('NOCHANGE',row,iteration - 1)
                    #self.datatable.tb2.putcell('NOCHANGE', row, iteration)
                    self.datatable.putcell('NOCHANGE', row, iteration)
            else:
                #self.datatable.putcell('NOCHANGE',row,False)
                #self.datatable.tb2.putcell('MASKLIST',row,numpy.array(RealSignal[row][2]))
                #LOG.debug('Updating row %s: signal=%s (type=%s, %s)'%(row,list(signal),type(signal),type(signal[0])))
                #self.datatable.tb2.putcell('MASKLIST',row,numpy.array(signal))
                #self.datatable.tb2.putcell('MASKLIST',row,signal)
                self.datatable.putcell('MASKLIST',row,signal)
                #self.datatable.tb2.putcell('NOCHANGE',row,-1)
                self.datatable.putcell('NOCHANGE',row,-1)
        del GridCluster, RealSignal
        ProcEndTime = time.time()
        LOG.info('Clustering: Merging End: Elapsed time = %s sec' % (ProcEndTime - ProcStartTime))
        
        outcome = {'lines': lines,
                   'channelmap_range': channelmap_range,
                   'cluster_info': self.cluster_info,
                   'datatable': self.datatable}
        result = ValidateLineResults(task=self.__class__,
                                     success=True,
                                     outcome=outcome)
                
        result.task = self.__class__
                
        if self.context.subtask_counter is 0: 
            result.stage_number = self.context.task_counter - 1
        else:
            result.stage_number = self.context.task_counter 
                
        return result
    
    def analyse(self, result):
        return result

    def clustering_analysis(self, Region, Region2, nsigma):
        MedianWidth = numpy.median(Region2[:,0])
        LOG.trace('MedianWidth = %s' % MedianWidth)

        #MaxCluster = int(min(rules.ClusterRule['MaxCluster'], max(Maxlines + 1, (Npos ** 0.5)/2)))
        MaxCluster = int(rules.ClusterRule['MaxCluster'])
        LOG.info('Maximum number of clusters (MaxCluster) = %s' % MaxCluster)
        # Whiten is no more necessary 2007/2/12
        # whitened = VQ.whiten(Region2)

        # TN refactoring
        # TmpList is no more used.
        #TmpList = []

        # Determin the optimum number of clusters
        BestScore = -1.0
        # 2010/6/15 Plot the score along the number of the clusters
        ListNcluster = []
        ListScore = []
        ListBestScore = []
        #
        elapsed = 0.0
        for Ncluster in xrange(1, MaxCluster + 1):
            index0=len(ListScore)
            # Fix the random seed 2008/5/23
            numpy.random.seed((1234,567))
            # Try multiple times to supress random selection effect 2007/09/04
            for Multi in xrange(min(Ncluster+1, 10)):
                #codebook, diff = VQ.kmeans(whitened, Ncluster)
                #codebook, diff = VQ.kmeans(Region2, Ncluster)
                codebook, diff = VQ.kmeans(Region2, Ncluster, iter=50)
                NclusterNew = 0
                LOG.trace('codebook=%s'%(codebook))
                # Do iteration until no merging of clusters to be found
                while(NclusterNew != len(codebook)):
                    category, distance = VQ.vq(Region2, codebook)
                    LOG.trace('Cluster Category&Distance %s, distance = %s' % (category, distance))

                    codebook = codebook.take([x for x in xrange(0,len(codebook)) 
                                              if any(category==x)], axis=0)
                    NclusterNew = len(codebook)
                    #for x in xrange(NclusterNew - 1, -1, -1):
                    #    # Remove a cluster without any members
                    #    #if sum(numpy.equal(category, x) * 1.0) == 0:
                    #    if all(category != x):
                    #        NclusterNew -= 1
                    #        tmp = list(codebook)
                    #        del tmp[x]
                    #        codebook = numpy.array(tmp)

                    # Clear Flag
                    for i in xrange(len(Region)): Region[i][5] = 1

                    # nsigma clipping/flagging with cluster distance 
                    # (set flag to 0)
                    Outlier = 0.0
                    #MaxDistance = []
                    for Nc in xrange(NclusterNew):
                        ValueList = distance[(category == Nc).nonzero()[0]]
                        Stddev = ValueList.std()
                        ### 2011/05/17 Strict the threshold
                        Threshold = ValueList.mean() + Stddev * nsigma
                        #Threshold = Stddev * nsigma
                        LOG.trace('Cluster Clipping Threshold = %s, Stddev = %s' % (Threshold, Stddev))
                        ### 2011/05/17 clipping iteration
                        #ValueList = ValueList.take((ValueList < Threshold).nonzero())[0]
                        #Stddev = ValueList.std()
                        #Threshold = Stddev * nsigma
                        del ValueList
                        #LOG.debug('Cluster Clipping Threshold = %s, Stddev = %s' % (Threshold, Stddev))
                        for i in ((distance * (category == Nc)) > Threshold).nonzero()[0]:
                            Region[i][5] = 0
                            Outlier += 1.0
                        #MaxDistance.append(max(distance * ((distance < Threshold) * (category == Nc))))
                        LOG.trace('Region = %s' % Region)
                    MemberRate = (len(Region) - Outlier)/float(len(Region))
                    LOG.trace('MemberRate = %f' % MemberRate)

                    # Calculate Cluster Characteristics
                    lines = []
                    for NN in xrange(NclusterNew):
#                        LineCenterList = []
#                        LineWidthList = []
#                        for x in range(len(category)):
#                            if category[x] == NN and Region[x][5] != 0:
#                                LineCenterList.append(Region2[x][1])
#                                LineWidthList.append(Region2[x][0])
#                        lines.append([numpy.median(numpy.array(LineCenterList)), numpy.median(numpy.array(LineWidthList)), True, MaxDistance[NN]])
                        MaxDistance = max(distance * ((distance < Threshold) * (category == NN)))
                        # Region2: numpy.array
                        # [[line_center0, line_width0], 
                        #  [line_center1, line_width1],
                        #  ...]
                        indices = [x for x in xrange(len(category)) 
                                   if category[x] == NN and Region[x][5] != 0]
                        properties = Region2.take(indices, axis=0)
                        median_props = numpy.median(properties, axis=0)
                        lines.append([median_props[1], median_props[0], True, MaxDistance])
                    LOG.trace('lines = %s' % lines)

                    # Rating
                    ### 2011/05/16 modified for line-width variation
                    #Score = ((distance+MedianWidth/2.0) * numpy.transpose(numpy.array(Region))[5]).mean() * (NclusterNew+ 1.0/NclusterNew) * (1.1 - MemberRate)
                    #Score = (distance * numpy.transpose(numpy.array(Region))[5]).mean() * (NclusterNew+ 1.0/NclusterNew) * (((1.0 - MemberRate)**0.5 + 1.0)**2.0)
                    ### 2011/05/12 modified for (distance==0)
                    Score = ((distance * numpy.transpose(numpy.array(Region))[5]).mean() + MedianWidth/2.0) * (NclusterNew+ 1.0/NclusterNew) * (((1.0 - MemberRate)**0.5 + 1.0)**2.0)
                    # 2007/09/10 More sensitive to the number of lines clipped out
                    #Score = (distance * numpy.transpose(numpy.array(Region))[5]).mean() * (NclusterNew+ 1.0/NclusterNew) * (((1.0 - MemberRate)**0.5 + 1.0)**2.0)
                    #Score = (distance * numpy.transpose(numpy.array(Region))[5]).mean() * (NclusterNew+ 1.0/NclusterNew) / MemberRate**2.0
                    # 2010/6/15 Plot the score along the number of the clusters
                    ListNcluster.append(Ncluster)
                    ListScore.append(Score)
                    LOG.debug('NclusterNew = %d, Score = %f' % (NclusterNew, Score))
                    if BestScore < 0:
                        BestNcluster = 1
                        BestScore = Score
                        BestCategory = category.copy()
                        BestCodebook = codebook.copy()
                        BestRegion = Region[:]
                        Bestlines = lines[:]
                    elif Score < BestScore:
                        BestNcluster = NclusterNew
                        BestScore = Score
                        BestCategory = category.copy()
                        BestCodebook = codebook.copy()
                        BestRegion = Region[:]
                        Bestlines = lines[:]
                # TN refactoring
                # TmpList is no more used.
                #TmpList.append([NclusterNew, Score, codebook])
            ListBestScore.append(min(ListScore[index0:]))
            LOG.debug('Ncluster = %d, BestScore = %f' % (NclusterNew, ListBestScore[-1]))
            if len(ListBestScore) > 3 and \
               ListBestScore[-4] <= ListBestScore[-3] and \
               ListBestScore[-4] <= ListBestScore[-2] and \
               ListBestScore[-4] <= ListBestScore[-1]:
                LOG.info('Determined the Number of Clusters to be %d' % (BestNcluster))
                break

        #Ncluster = BestNcluster
        #Region = BestRegion
        #category = BestCategory[:]
        lines = [[book[1], book[0], True] for book in BestCodebook[:Ncluster]]
        LOG.debug('Final: Ncluster = %s, Score = %s, Category = %s, CodeBook = %s, lines = %s' % (Ncluster, BestScore, category, BestCodebook, lines))
        self.cluster_info['cluster_score'] = [ListNcluster, ListScore]
        self.cluster_info['detected_lines'] = Region2
        self.cluster_info['cluster_property'] = Bestlines
        self.cluster_info['cluster_scale'] = self.CLUSTER_WHITEN
        #SDP.ShowClusterScore(ListNcluster, ListScore, ShowPlot, FigFileDir, FigFileRoot)
        #SDP.ShowClusterInchannelSpace(Region2, Bestlines, self.CLUSTER_WHITEN, ShowPlot, FigFileDir, FigFileRoot)
        LOG.info('Final: Ncluster = %s, Score = %s, lines = %s' % (Ncluster, BestScore, lines))

        return (BestNcluster, Bestlines, BestCategory, BestRegion)

    def detection_stage(self, Ncluster, nra, ndec, x0, y0, grid_ra, grid_dec, category, Region, detect_signal):
        # Create Grid Parameter Space (Ncluster * nra * ndec)
        GridCluster = numpy.zeros((Ncluster, nra, ndec), dtype=numpy.float32)
        GridMember = numpy.zeros((nra, ndec))

        # Set Cluster on the Plane
        for row in xrange(len(detect_signal)):
            # Check statistics flag added by G.K. 2008/1/17
            # Bug fix 2008/5/29
            #if DataTable[row][DT_SFLAG] == 1:
            # grid_table is always created 
            #if ((len(grid_table) == 0 and tSFLAG[row] == 1) or len(grid_table) != 0):
                #print 'nra, ndec, row', nra, ndec, row
                #print 'GridMember', int((detect_signal[row][0] - x0)/grid_ra)
                #print 'GridMember', int((detect_signal[row][1] - y0)/grid_dec)
            GridMember[int((detect_signal[row][0] - x0)/grid_ra)][int((detect_signal[row][1] - y0)/grid_dec)] += 1
        for i in xrange(len(category)):
            if Region[i][5] == 1:
                try:
                    GridCluster[category[i]][int((Region[i][3] - x0)/grid_ra)][int((Region[i][4] - y0)/grid_dec)] += 1.0
                except IndexError:
                    pass

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

    def validation_stage(self, GridCluster, GridMember, lines, iteration):
                # Effective if number of spectrum which contains feature belongs to the cluster is greater or equal to the half number of spectrum in the Grid

        (Ncluster,nra,ndec) = GridCluster.shape

        for Nc in range(Ncluster):
            for x in range(nra):
                for y in range(ndec):
                    if GridMember[x][y] == 0: GridCluster[Nc][x][y] = 0.0
                    # if a single spectrum is inside the grid and has a feature belongs to the cluster, validity is set to 0.5 (for the initial stage) or 1.0 (iteration case).
                    elif GridMember[x][y] == 1 and GridCluster[Nc][x][y] > 0.9:
                        # 2013/05/20 TN
                        # Temporal workaround that line validation fails on
                        # test data if iteration is consistently handled.
                        #if iteration == 0: GridCluster[Nc][x][y] = 0.5
                        #else: GridCluster[Nc][x][y] = 1.0
                        GridCluster[Nc][x][y] = 1.0
                    # if the size of population is enough large, validate it as a special case 2007/09/05
                    # 2013/05/20 TN
                    # Temporal workaround that line validation fails on
                    # test data if iteration is consistently handled.
                    #elif iteration == 0:
                    #    GridCluster[Nc][x][y] = max(min(GridCluster[Nc][x][y] / sqrt(GridMember[x][y]) - 1.0, 3.0), GridCluster[Nc][x][y] / float(GridMember[x][y]))
                    else: GridCluster[Nc][x][y] = min(GridCluster[Nc][x][y] / sqrt(GridMember[x][y]), 3.0)
                    #else: GridCluster[Nc][x][y] = max(min(GridCluster[Nc][x][y] / GridMember[x][y]**0.5 - 1.0, 3.0), GridCluster[Nc][x][y] / float(GridMember[x][y]))
                    # normarize validity
                    #else: GridCluster[Nc][x][y] /= float(GridMember[x][y])

            if ((GridCluster[Nc] > self.Questionable)*1).sum() == 0: lines[Nc][2] = False

        threshold = [self.Valid, self.Marginal, self.Questionable]
        self.__update_cluster_flag('validation', GridCluster, threshold, 10)
        
        return (GridCluster, GridMember, lines)

    def smoothing_stage(self, GridCluster, lines):
        # Rating:  [0.0, 0.4, 0.5, 0.4, 0.0]
        #          [0.4, 0.7, 1.0, 0.7, 0.4]
        #          [0.5, 1.0, 6.0, 1.0, 0.5]
        #          [0.4, 0.7, 1.0, 0.7, 0.4]
        #          [0.0, 0.4, 0.5, 0.4, 0.0]
        # Rating = 1.0 / (Dx**2 + Dy**2)**(0.5) : if (Dx, Dy) == (0, 0) rating = 6.0
        (Ncluster,nra,ndec) = GridCluster.shape
        GridScore = numpy.zeros((2, nra, ndec), dtype=numpy.float32)
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
                            Rating = 1.0 / abs(dy)
                            GridScore[0][x][y] += Rating * GridCluster[Nc][x][ny]
                            GridScore[1][x][y] += Rating
                        # dy = 0
                        for dx in range_x:
                            nx = x + dx
                            Rating = 1.0 / abs(dx)
                            GridScore[0][x][y] += Rating * GridCluster[Nc][nx][y]
                            GridScore[1][x][y] += Rating
                        # dx != 0 and dy != 0
                        for dx in range_x:
                            for dy in range_y:
                                if (abs(dx) + abs(dy)) <= 3:
                                    (nx, ny) = (x + dx, y + dy)
                                    Rating = 1.0 / sqrt(dx*dx + dy*dy)
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
                LOG.trace('Score :  GridScore[%s][0] = %s' % (Nc, GridScore[0]))
                LOG.trace('Rating:  GridScore[%s][1] = %s' % (Nc, GridScore[1]))
                GridCluster[Nc] = GridScore[0] / GridScore[1]
            if ((GridCluster[Nc] > self.Questionable)*1).sum() < 0.1: lines[Nc][2] = False

        threshold = [self.Valid, self.Marginal, self.Questionable]
        self.__update_cluster_flag('smoothing', GridCluster, threshold, 100)
        
        return (GridCluster, lines)

    def final_stage(self, GridCluster, GridMember, Region, Region2, lines, category, grid_ra, grid_dec, broad_component, xorder, yorder, x0, y0, nsigma, nchan, Grid2SpectrumID, index_list, PosList):
                
        (Ncluster, nra, ndec) = GridCluster.shape
        xorder0 = xorder
        yorder0 = yorder

        # Dictionary for final output
        RealSignal = {}

        #HalfGrid = (grid_ra ** 2 + grid_dec ** 2) ** 0.5 / 2.0
        HalfGrid = 0.5 * sqrt(grid_ra*grid_ra + grid_dec*grid_dec)

        LOG.info('Ncluster=%s'%(Ncluster))
        
        # Clean isolated grids
        for Nc in xrange(Ncluster):
            #print '\nNc=', Nc
            if lines[Nc][2] != False:
                Plane = (GridCluster[Nc] > self.Marginal) * 1
                if Plane.sum() == 0:
                    lines[Nc][2] = False
                    #print 'lines[Nc][2] -> False'
                    continue
                Original = GridCluster[Nc].copy()
                # Clear GridCluster Nc-th plane
                GridCluster[Nc] *= 0.0
                Nmember = []
                Realmember = []
                MemberList = []
                NsubCluster = 0
                for x in xrange(nra):
                    for y in xrange(ndec):
                        if Plane[x][y] == 1:
                            Plane[x][y] = 2
                            SearchList = [(x, y)]
                            M = 1
                            if Original[x][y] > self.Valid: MM = 1
                            #if Original[x][y] > self.Marginal: MM = 1
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
                                            #if Original[nx][ny] > self.Marginal: MM += 1
                                            MemberList[NsubCluster].append((nx, ny))
                                del SearchList[0]
                            Nmember.append(M)
                            Realmember.append(MM)
                            NsubCluster += 1

                # If no members left, skip to next cluster
                if len(Nmember) == 0: continue
                # Threshold is set to half the number of the largest cluster in the plane
                #Threshold = max(Nmember) / 2.0
                #Threshold = min(max(Realmember) / 2.0, 3)
                Threshold = min(0.5 * max(Realmember), 3)
                for n in xrange(NsubCluster -1, -1, -1):
                    # isolated cluster made from single spectrum should be omitted
                    if Nmember[n] == 1:
                        (x, y) = MemberList[n][0]
                        if GridMember[x][y] <= 1:
                            Nmember[n] = 0
                            #print '\nHere Nmember[%d] = 1' % n
                    # Sub-Cluster whose member below the threshold is cleaned
                    if Nmember[n] < Threshold:
                        #print '\nNmember[%d]=%d, Threshold=%f' % (n, Nmember[n], Threshold)
                        for (x, y) in MemberList[n]:
                            Plane[x][y] == 0
                        del Nmember[n]
                        del MemberList[n]

                # Blur each SubCluster with the radius of sqrt(Nmember/Pi) * ratio
                ratio = rules.ClusterRule['BlurRatio']
                # Set-up SubCluster
                for n in xrange(len(Nmember)):
                    SubPlane = numpy.zeros((nra, ndec), dtype=numpy.float32)
                    #xlist = []
                    #ylist = []
                    for (x, y) in MemberList[n]:
                        SubPlane[x][y] = Original[x][y]
                        #xlist.append(x)
                        #ylist.append(y)
                    # Calculate Blur radius
                    #Blur = int((Realmember[n] / 3.141592653) ** 0.5 * ratio + 0.5)
                    #BlurF = (Realmember[n] / 3.141592653) ** 0.5 * ratio + 1.5
                    BlurF = sqrt(Realmember[n] / 3.141592653) * ratio + 1.5
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
                    BlurPlane = (convolve2d(SubPlane, kernel) > self.Marginal) * 1
                    ValidPlane = (SubPlane > self.Valid) * 1
                    LOG.trace('GridCluster.shape = %s' % list(GridCluster.shape))
                    LOG.trace('Plane.shape = %s' % list(Plane.shape))
                    LOG.trace('SubPlane.shape = %s' % list(SubPlane.shape))
                    LOG.trace('BlurPlane.shape = %s' % list(BlurPlane.shape))
                    for x in xrange(len(Plane)):
                        LOG.trace(' %d : %s' % (x, list(Plane[x])))
                    for x in xrange(len(BlurPlane)):
                        LOG.trace(' %d : %s' % (x, list(BlurPlane[x])))
                    for x in xrange(len(ValidPlane)):
                        LOG.trace(' %d : %s' % (x, list(ValidPlane[x])))

                    # 2D fit for each Plane
                    # Use the data for fit if GridCluster[Nc][x][y] > self.Valid
                    # Not use for fit but apply the value at the border if GridCluster[Nc][x][y] > self.Marginal

                    # 2009/9/10 duplication of the position was taken into account (broad_component=True)
                    Bfactor = 1.0
                    if broad_component: Bfactor = 2.0
                    # Determine fitting order if not specified
                    #if xorder < 0: xorder0 = min(((numpy.sum(ValidPlane, axis=0) > 0.5)*1).sum() - 1, 5)
                    #if yorder < 0: yorder0 = min(((numpy.sum(ValidPlane, axis=1) > 0.5)*1).sum() - 1, 5)
                    if xorder < 0: xorder0 = int(min(((numpy.sum(ValidPlane, axis=0) > 0.5)*1).sum()/Bfactor - 1, 5))
                    if yorder < 0: yorder0 = int(min(((numpy.sum(ValidPlane, axis=1) > 0.5)*1).sum()/Bfactor - 1, 5))
                    #if xorder < 0: xorder0 = min(max(max(xlist) - min(xlist), 0), 5)
                    #if yorder < 0: yorder0 = min(max(max(ylist) - min(ylist), 0), 5)
                    LOG.trace('(X,Y)order = (%d, %d)' % (xorder0, yorder0))

                    # clear Flag
                    for i in xrange(len(category)): Region[i][5] = 1

                    if xorder0 < 0 or yorder0 < 0:
                        SingularMatrix = True
                        ExceptionLinAlg = False
                    else:
                        SingularMatrix = False
                        ExceptionLinAlg = True

                    while ExceptionLinAlg:
                        FitData = []
                        ### 2011/05/15 One parameter (Width, Center) for each spectra
                        #Region format:([row, line[0], line[1], detect_signal[row][0], detect_signal[row][1], flag])
                        dummy = [tuple(Region[i][:5]) for i in xrange(len(category))
                                 if category[i] == Nc and Region[i][5] == 1 and SubPlane[int((Region[i][3] - x0)/grid_ra)][int((Region[i][4] - y0)/grid_dec)] > self.Valid]
                        (Lrow, Lmin, Lmax, LRA, LDEC) = dummy[0]
                        for i in xrange(1,len(dummy)):
                            if Lrow == dummy[i][0]:
                                Lmin = min(Lmin, dummy[i][1])
                                Lmax = max(Lmax, dummy[i][2])
                            else:
                                FitData.append([Lmin, Lmax, LRA, LDEC, 1])
                                (Lrow, Lmin, Lmax, LRA, LDEC) = dummy[i]
                        FitData.append([Lmin, Lmax, LRA, LDEC, 1])
                        del dummy
                        # TN refactoring
                        # make arrays for coefficient calculation
                        # Matrix    MM x A = B  ->  A = MM^-1 x B
                        # It is OK to prepare storage outside the loop 
                        # since xorder0 and yorder0 will not change inside 
                        # the loop. Whenever xorder0 or yorder0 changes, 
                        # exit the loop. See code below.
                        M0 = numpy.zeros((xorder0 * 2 + 1) * (yorder0 * 2 + 1), dtype=numpy.float64)
                        B0 = numpy.zeros((xorder0 + 1) * (yorder0 + 1), dtype=numpy.float64)
                        B1 = numpy.zeros((xorder0 + 1) * (yorder0 + 1), dtype=numpy.float64)
                        MM0 = numpy.zeros([(xorder0 + 1) * (yorder0 + 1), (xorder0 + 1) * (yorder0 + 1)], dtype=numpy.float64)
                        for iteration in xrange(3):
                            LOG.trace('2D Fit Iteration = %d' % iteration)

                            ### Commented out three lines 2011/05/15
                            # FitData format: [Width, Center, RA, DEC]
                            #for i in range(len(category)):
                            #    if category[i] == Nc and Region[i][5] == 1 and SubPlane[int((Region[i][3] - x0)/grid_ra)][int((Region[i][4] - y0)/grid_dec)] > self.Valid:
                            #        FitData.append((Region2[i][0], Region2[i][1], Region[i][3], Region[i][4]))

                            # TN refactoring
                            # Comment out the following if statement since 
                            # 1) len(FitData) is always greater than 0. 
                            #    Lee the code just above start of iteration.
                            # 2) SingularMatrix is always False in this 
                            #    loop. Exit the loop whenever SingularMatrix 
                            #    is set to False. See code below.
                            #if len(FitData) == 0 or SingularMatrix: break
                            LOG.trace('FitData = %s' % FitData)

                            # effective components of FitData
                            effective = [i for i in xrange(len(FitData)) 
                                         if FitData[i][4] == 1]

                            # calculate coefficients
                            M0[:] = 0.0
                            B0[:] = 0.0
                            B1[:] = 0.0
                            MM0[:] = 0.0
                            for eff in effective:
                                (Width, Center, x, y, flag) = FitData[eff]
                                yk = 1.0
                                idx = 0
                                for k in xrange(yorder0 * 2 + 1):
                                    xjyk = yk
                                    for j in xrange(xorder0 * 2 + 1):
                                        M0[idx] += xjyk
                                        xjyk *= x
                                        idx += 1
                                    yk *= y
                                yk = 1.0
                                idx = 0
                                for k in xrange(yorder0 + 1):
                                    xjyk = yk
                                    for j in xrange(xorder0 + 1):
                                        B0[idx] += xjyk * Center
                                        B1[idx] += xjyk * Width
                                        xjyk *= x
                                        idx += 1
                                    yk *= y

                            # make Matrix MM0,MM1 and calculate A0,A1
                            for K in xrange((xorder0 + 1) * (yorder0 + 1)):
                                k0 = K % (xorder0 + 1)
                                k1 = int(K / (xorder0 + 1))
                                for J in xrange((xorder0 + 1) * (yorder0 + 1)):
                                    j0 = J % (xorder0 + 1)
                                    j1 = int(J / (xorder0 + 1))
                                    MM0[J, K] = M0[j0 + k0 + (j1 + k1) * (xorder0 * 2 + 1)]
                            ExceptionLinAlg = False
                            try:
                                A0 = LA.solve(MM0, B0)
                                # TN refactoring
                                # MM1 is same as MM0 and MM0 is not overwritten 
                                # so that MM1 is not necessary. We can use MM0.
                                #MM1 = MM0.copy()
                                #A1 = LA.solve(MM1, B1)
                                A1 = LA.solve(MM0, B1)
                            #except LinAlgError:
                            except:
                                if xorder0 != 0 or yorder0 != 0:
                                    ExceptionLinAlg = True
                                    LOG.trace('xorder0,yorder0 = %s,%s' % (xorder0, yorder0))
                                    xorder0 = max(xorder0 - 1, 0)
                                    yorder0 = max(yorder0 - 1, 0)
                                else:
                                    SingularMatrix = True
                                break

                            # Calculate Sigma
                            # Sigma should be calculated in the upper stage
                            # Fit0: Center or Lmax, Fit1: Width or Lmin
                            Diff = []
                            # TN refactoring
                            # Calculation of Diff is duplicated here and 
                            # following clipping stage. So, evalueate Diff 
                            # only once here and reuse it in clipping.
                            for (Width, Center, x, y, flag) in FitData:
                                (Fit0, Fit1) = _eval_poly(xorder0+1, yorder0+1, x, y, A0, A1)
                                Fit0 -= Center
                                Fit1 -= Width
                                Diff.append(sqrt(Fit0*Fit0 + Fit1*Fit1))
                            #if len(Diff) > 1: Threshold = numpy.array(Diff).mean() + numpy.array(Diff).std() * nsigma
                            #if len(Diff) > 1:
                            if len(effective) > 1:
                                npdiff = numpy.array(Diff)[effective]
                                Threshold = npdiff.mean()
                                #Threshold += npdiff.std()
                                Threshold += sqrt(numpy.square(npdiff - Threshold).mean()) * nsigma
                            #if len(Diff) > 1: Threshold = numpy.array(Diff).std() * nsigma
                            else: Threshold *= 2.0
                            LOG.trace('Diff = %s' % [Diff[i] for i in effective])
                            LOG.trace('2D Fit Threshold = %s' % Threshold)

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

                            LOG.trace('2D Fit Flagged/All = (%s, %s)' % (NFlagged, Number))
                            #2009/10/15 compare the number of the remainder and fitting order
                            if (Number - NFlagged) <= max(xorder0, yorder0) or Number == NFlagged:
                                SingularMatrix = True
                                break
                    # Iteration End
                    ### 2011/05/15 Fitting is no longer (Width, Center) but (minchan, maxChan)

                    # FitData: [(Width, Center, RA, DEC)]
                    if not SingularMatrix:
                        FitData = []
                        for i in range(len(category)):
                            if category[i] == Nc and Region[i][5] == 1 and SubPlane[int((Region[i][3] - x0)/grid_ra)][int((Region[i][4] - y0)/grid_dec)] > self.Valid:
                                #FitData.append((Region2[i][0], Region2[i][1], Region[i][3], Region[i][4]))
                                FitData.append(tuple(Region2[i][:5]))
                        if len(FitData) == 0: continue

                        # Calculate Fit for each position
                        for x in xrange(nra):
                            for y in xrange(ndec):
                                if ValidPlane[x][y] == 1:
                                    for PID in Grid2SpectrumID[x][y]:
                                        ID = index_list[PID]
                                        ### 2011/05/15 (Width, Center) -> (minchan, maxChan)
                                        (Chan1, Chan0) = _eval_poly(xorder0+1, yorder0+1, PosList[0][PID], PosList[1][PID], A0, A1) 
                                        Fit0 = 0.5 * (Chan0 + Chan1)
                                        Fit1 = (Chan1 - Chan0) + 1.0
                                        LOG.trace('Fit0, Fit1 = %s, %s' % (Fit0, Fit1))
                                        if (Fit1 >= self.MinFWHM) and (Fit1 <= self.MaxFWHM):
                                            # Allowance = Fit1 / 2.0 * 1.3
                                            # To keep broad line region, make allowance larger
                                            ### 2011/05/16 allowance + 0.5 ->  2.5 for sharp line
                                            ### 2011/05/16 factor 1.5 -> 2.0 for broad line
                                            #Allowance = min(Fit1 / 2.0 * 2.0, MaxFWHM / 2.0)
                                            ### 2011/11/22 Allowance is too narrow for new line finding algorithm
                                            Allowance = min(Fit1 + 5.0, self.MaxFWHM / 2.0)
                                            ### 2011/10/21 left side mask exceeded nchan
                                            Protect = [min(max(int(Fit0 - Allowance), 0), nchan - 1), min(int(Fit0 + Allowance), nchan - 1)]
                                            #Allowance = Fit1 / 2.0 * 1.5
                                            #Protect = [max(int(Fit0 - Allowance - 0.5), 0), min(int(Fit0 + Allowance + 0.5), nchan - 1)]
                                            LOG.trace('0 Allowance = %s Protect = %s' % (Allowance, Protect))
                                            if RealSignal.has_key(ID):
                                                RealSignal[ID][2].append(Protect)
                                            else:
                                                RealSignal[ID] = [PosList[0][PID], PosList[1][PID] ,[Protect]]
                                elif BlurPlane[x][y] == 1:
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
                                    LOG.trace('[X,Y],[XX,YY] = [%d,%d],%s' % (x,y,Nearest))
                                    LOG.trace('(RA0,DEC0),(RA1,DEC1),(RA2,DEC2) = (%.5f,%.5f),(%.5f,%.5f),(%.5f,%.5f)' % (RA0,DEC0,RA1,DEC1,RA2,DEC2))
                                    # Calculate Fit and apply same value to all the spectra in the Blur Grid
                                    ### 2011/05/15 (Width, Center) -> (minchan, maxChan)
                                    # Border case
                                    #(Chan0, Chan1) = _eval_poly(xorder0+1, yorder0+1, RA2, DEC2, A0, A1)
                                    # Center case
                                    (Chan1, Chan0) = _eval_poly(xorder0+1, yorder0+1, RA1, DEC1, A0, A1)
                                    Fit0 = 0.5 * (Chan0 + Chan1)
                                    Fit1 = (Chan1 - Chan0)
                                    LOG.trace('Fit0, Fit1 = %s, %s' % (Fit0, Fit1))
                                    if (Fit1 >= self.MinFWHM) and (Fit1 <= self.MaxFWHM):
                                        #Allowance = Fit1 / 2.0 * 1.3
                                        # To keep broad line region, make allowance larger
                                        ### 2011/05/16 allowance + 0.5 ->  2.5 for sharp line
                                        ### 2011/05/16 factor 1.5 -> 2.0 for broad line
                                        #Allowance = min(Fit1 / 2.0 * 2.0, MaxFWHM / 2.0)
                                        ### 2011/11/22 Allowance is too narrow for new line finding algorithm
                                        Allowance = min(Fit1 + 5.0, self.MaxFWHM / 2.0)
                                        ### 2011/10/21 left side mask exceeded nchan
                                        Protect = [min(max(int(Fit0 - Allowance), 0), nchan - 1), min(int(Fit0 + Allowance), nchan - 1)]
                                        #Allowance = Fit1 / 2.0 * 1.5
                                        #Protect = [max(int(Fit0 - Allowance + 0.5), 0), min(int(Fit0 + Allowance + 0.5), nchan - 1)]

                                        LOG.trace('1 Allowance = %s Protect = %s' % (Allowance, Protect))
                                        for PID in Grid2SpectrumID[x][y]:
                                            ID = index_list[PID]
                                            if RealSignal.has_key(ID):
                                                RealSignal[ID][2].append(Protect)
                                            else:
                                                RealSignal[ID] = [PosList[0][PID], PosList[1][PID] ,[Protect]]
                                    else: continue
                    # for Plot
                    if not SingularMatrix: GridCluster[Nc] += BlurPlane
                if ((GridCluster[Nc] > 0.5)*1).sum() < self.Questionable: lines[Nc][2] = False
                for x in range(nra):
                    for y in range(ndec):
                        #if GridCluster[Nc][x][y] > 0.5: GridCluster[Nc][x][y] = 1.0
                        #if Original[x][y] > self.Valid: GridCluster[Nc][x][y] = 2.0
                        if Original[x][y] > self.Valid: GridCluster[Nc][x][y] = 2.0
                        elif GridCluster[Nc][x][y] > 0.5: GridCluster[Nc][x][y] = 1.0
                        

        threshold = [1.5, 0.5, 0.5, 0.5]
        self.__update_cluster_flag('final', GridCluster, threshold, 1000)
        channelmap_range = lines
        
        return (RealSignal, lines, channelmap_range)

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
