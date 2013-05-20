from __future__ import absolute_import

import numpy
import math
from math import sqrt
import time
import scipy.cluster.vq as VQ
import numpy.linalg as LA

import pipeline.infrastructure as infrastructure
from . import rules

LOG = infrastructure.get_logger(__name__)

class ValidateLineSinglePointing(object):

    def __init__(self, datatable):
        self.datatable = datatable

    def execute(self, ResultTable, DetectSignal, vIF, nChan, idxList, GridSpaceRA, GridSpaceDEC, ITER, Nsigma=3.0, Xorder=-1, Yorder=-1, BroadComponent=False, LogLevel=2, LogFile=False, ShowPlot=True, FigFileDir=False, FigFileRoot=False):
        """
        ValidateLine class for single-pointing or multi-pointing (collection of 
        fields with single-pointing). Accept all detected lines without 
        clustering analysis.

         DetectSignal = {ID1: [RA, DEC, [[LineStartChannel1, LineEndChannel1],
                                         [LineStartChannel2, LineEndChannel2],
                                         [LineStartChannelN, LineEndChannelN]]],
                         IDn: [RA, DEC, [[LineStartChannel1, LineEndChannel1],
                                         [LineStartChannelN, LineEndChannelN]]]}

        Lines: output parameter
           [LineCenter, LineWidth, Validity]  OK: Validity = True; NG: Validity = False
        """
        # Dictionary for final output
        Lines = []

        LOG.info('Accept all detected lines without clustering analysis.')

        # First cycle
        #if len(ResultTable) == 0:
        if ITER == 0:
            for row in idxList:
                mask_list = self.datatable.getcell('MASKLIST',row)
                no_change = self.datatable.getcell('NOCHANGE',row)
                LOG.debug('DataTable = %s, DetectSignal = %s, OldFlag = %s' % (mask_list, DetectSignal[row][2], no_change))
                self.datatable.putcell('MASKLIST',row,DetectSignal[row][2])
                self.datatable.putcell('NOCHANGE',row,False)

        # Iteration case
        else:
            for row in idxList:
                mask_list = self.datatable.getcell('MASKLIST',row)
                no_change = self.datatable.getcell('NOCHANGE',row)
                LOG.debug('DataTable = %s, DetectSignal = %s, OldFlag = %s' % (mask_list, DetectSignal[0][2], no_change))
                if mask_list == DetectSignal[0][2]:
                    if type(no_change) != int:
                        # 2013/05/17 TN
                        # Put ITER itself instead to subtract 1 since
                        # iteration counter is incremented *after* the
                        # baseline subtraction in refactorred code.
                        #self.datatable.putcell('NOCHANGE',row,ITER - 1)
                        self.datatable.putcell('NOCHANGE',row,ITER)
                else:
                    self.datatable.putcell('MASKLIST',row,DetectSignal[0][2])
                    self.datatable.putcell('NOCHANGE',row,False)
        return Lines

class ValidateLineRaster(object):

    CLUSTER_WHITEN = 1.0

    Valid = rules.ClusterRule['ThresholdValid']
    Marginal = rules.ClusterRule['ThresholdMarginal']
    Questionable = rules.ClusterRule['ThresholdQuestionable']
    #2010/6/9 Delete global parameter Min/MaxFWHM
    MinFWHM = rules.LineFinderRule['MinFWHM']
    MaxFWHM = rules.LineFinderRule['MaxFWHM']

    def __init__(self, datatable):
        self.datatable = datatable

    def execute(self, ResultTable, DetectSignal, vIF, nChan, idxList, GridSpaceRA, GridSpaceDEC, ITER, Nsigma=3.0, Xorder=-1, Yorder=-1, BroadComponent=False, LogLevel=2, LogFile=False, ShowPlot=True, FigFileDir=False, FigFileRoot=False):
        """
        2D fit line characteristics calculated in Process3
        Sigma clipping iterations will be applied if Nsigma is positive
        order < 0 : automatic determination of fitting order (max = 5)

         DetectSignal = {ID1: [RA, DEC, [[LineStartChannel1, LineEndChannel1],
                                         [LineStartChannel2, LineEndChannel2],
                                         [LineStartChannelN, LineEndChannelN]]],
                         IDn: [RA, DEC, [[LineStartChannel1, LineEndChannel1],
                                         [LineStartChannelN, LineEndChannelN]]]}

        Lines: output parameter
           [LineCenter, LineWidth, Validity]  OK: Validity = True; NG: Validity = False
        """


        #Abcissa = self.Abcissa[vIF]
        
        ProcStartTime = time.time()
        LOG.info('2D fit the line characteristics...')

        #tMASKLIST = None
        #tNOCHANGE = None
        #tRA = self.datatable.getcol('RA')
        #tDEC = self.datatable.getcol('DEC')
        tSFLAG = self.datatable.getcol('FLAG_SUMMARY')
        
        TotalLines = 0
        RMS0 = 0.0
        #nChan = self.datatable.getcell('NCHAN',idxList[0])
        #nChan = self.context.observing_run[0].spectral_window[vIF].nchan
        Lines = []

        # First cycle
        #if len(ResultTable) == 0:
        #    ROWS = idxList
        # Iteration case
        #else:
        #    ROWS = range(len(ResultTable))
        ROWS = range(len(ResultTable))

        # RASTER CASE
        # Read data from Table to generate ID -> RA, DEC conversion table
        Region = []
        dummy = []
        flag = 1
        Npos = 0
        #MaxLines = 0
        ### 2011/05/13 Calculate median line width
        Width = []

        for row in ROWS:
            #if len(DetectSignal[row][2]) > MaxLines: MaxLines = len(DetectSignal[row][2])
            #MaxLines = max(MaxLines, len(DetectSignal[row][2]))

            if DetectSignal[row][2][0][0] != -1: Npos += 1

            for line in DetectSignal[row][2]:
                # Check statistics flag added by G.K. 2008/1/17
                # Bug fix 2008/5/29
                if (line[0] != line[1]) and ((len(ResultTable) == 0 and tSFLAG[row] == 1) or len(ResultTable) != 0):
                    #Region.append([row, line[0], line[1], DetectSignal[row][0], DetectSignal[row][1], flag])
                    Region.append([row, line[0], line[1], DetectSignal[row][0], DetectSignal[row][1], flag])
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
        #LOG.debug('MaxLines = %s' % MaxLines)
        LOG.debug('Npos = %s' % Npos)
        ###LOG.debug('MedianWidth = %s' % MedianWidth)
        # 2010/6/9 for non-detection
        if Npos == 0: return Lines

        # 2008/9/20 Dec Effect was corrected
        PosList = numpy.array([numpy.take(self.datatable.getcol('RA'),idxList),
                               numpy.take(self.datatable.getcol('DEC'),idxList)])
        #DecCorrection = 1.0 / math.cos(tDEC[0] / 180.0 * 3.141592653)
        DecCorrection = 1.0 / math.cos(PosList[1][0] / 180.0 * 3.141592653)
        GridSpaceRA *= DecCorrection
        # Calculate Parameters for Gridding
        #PosList = numpy.array([numpy.take(tRA,idxList),
        #                    numpy.take(tDEC,idxList)])

        wra = PosList[0].max() - PosList[0].min()
        wdec = PosList[1].max() - PosList[1].min()
        cra = PosList[0].min() + wra/2.0
        cdec = PosList[1].min() + wdec/2.0
        # 2010/6/11 +1.0 -> +1.01: if wra is n x GridSpaceRA (n is a integer), int(wra/GridSpaceRA) is not n in some cases because of the lack of accuracy.
        nra = 2 * (int((wra/2.0 - GridSpaceRA/2.0)/GridSpaceRA) + 1) + 1
        ndec = 2 * (int((wdec/2.0 - GridSpaceDEC/2.0)/GridSpaceDEC) + 1) + 1
        x0 = cra - GridSpaceRA/2.0 - GridSpaceRA*(nra-1)/2.0
        y0 = cdec - GridSpaceDEC/2.0 - GridSpaceDEC*(ndec-1)/2.0
        LOG.debug('Grid = %d x %d\n' % (nra, ndec))

        # Create Space for storing the list of spectrum (ID) in the Grid
        # 2013/03/27 TN
        # Grid2SpectrumID stores index of idxList instead of row numbers 
        # that are held by idxList.
        #Grid2SpectrumID = []
        #for x in range(nra):
        #    Grid2SpectrumID.append([])
        #    for y in range(ndec):
        #        Grid2SpectrumID[x].append([])
        Grid2SpectrumID = [[[] for y in xrange(ndec)] for x in xrange(nra)]
        for i in range(len(idxList)):
            Grid2SpectrumID[int((PosList[0][i] - x0)/GridSpaceRA)][int((PosList[1][i] - y0)/GridSpaceDEC)].append(i)
        LOG.debug('Grid2SpectrumID = %s' % Grid2SpectrumID)

        ProcEndTime = time.time()
        LOG.info('Clustering: Initialization End: Elapsed time = %s sec' % (ProcEndTime - ProcStartTime))

        # K-mean Clustering Analysis with LineWidth and LineCenter
        # Max number of protect regions are SDC.SDParam['Cluster']['MaxCluster'] (Max lines)
        ProcStartTime = time.time()
        LOG.info('K-mean Clustering Analaysis Start')

        (Ncluster, BestLines, BestCategory, Region) = self.clustering_analysis(Region, Region2, Nsigma)

        ProcEndTime = time.time()
        #LOG.info('Final: Ncluster = %s, Score = %s, Lines = %s' % (Ncluster, BestScore, Lines))
        LOG.info('K-mean Cluster Analaysis End: Elapsed time = %s sec' % (ProcEndTime - ProcStartTime))

        # Sort Lines and Category by LineCenter: Lines[][0]
        LineIndex = numpy.argsort([line[0] for line in BestLines[:Ncluster]])
        Lines = [BestLines[i] for i in LineIndex]

        ### 2011/05/17 Scaling back the line width
        Region2[:,0] = Region2[:,0] * self.CLUSTER_WHITEN
        for Nc in range(Ncluster):
            Lines[Nc][1] *= self.CLUSTER_WHITEN

        LineIndex2 = numpy.argsort(LineIndex)
        #for i in range(len(BestCategory)): category[i] = LineIndex2[BestCategory[i]]
        category = [LineIndex2[bc] for bc in BestCategory]

        ######## Clustering: Detection Stage ########
        ProcStartTime = time.time()
        LOG.info('Clustering: Detection Stage Start')

        (GridCluster, GridMember) = self.detection_stage(Ncluster, nra, ndec, x0, y0, GridSpaceRA, GridSpaceDEC, category, Region, DetectSignal)

        ProcEndTime = time.time()
        LOG.info('Clustering: Detection Stage End: Elapsed time = %s sec' % (ProcEndTime - ProcStartTime))

        ######## Clustering: Validation Stage ########
        ProcStartTime = time.time()
        LOG.info('Clustering: Validation Stage Start')

        (GridCluster, GridMember, Lines) = self.validation_stage(GridCluster, GridMember, Lines, ITER)

        LOG.info('Lines after validation: %s'%(Lines))
        LOG.info('GridCluster after validation: %s'%(GridCluster))
        
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

        (GridCluster, Lines) = self.smoothing_stage(GridCluster, Lines)

        ProcEndTime = time.time()
        LOG.info('Clustering: Smoothing Stage End: Elapsed time = %s sec' % (ProcEndTime - ProcStartTime))

        LOG.info('Lines after smoothing: %s'%(Lines))
        LOG.info('GridCluster after smoothing: %s'%(GridCluster))
        
        ######## Clustering: Final Stage ########
        ProcStartTime = time.time()
        LOG.info('Clustering: Final Stage Start')

        (RealSignal, Lines) = self.final_stage(GridCluster, GridMember, Region, Region2, Lines, category, GridSpaceRA, GridSpaceDEC, BroadComponent, Xorder, Yorder, x0, y0, Nsigma, nChan, Grid2SpectrumID, idxList, PosList)

        ProcEndTime = time.time()
        LOG.info('Clustering: Final Stage End: Elapsed time = %s sec' % (ProcEndTime - ProcStartTime))

        LOG.info('Lines after final: %s'%(Lines))

        # Merge masks if possible
        ProcStartTime = time.time()
        # RealSignal should have all row's as its key
        tmp_index = 0
        for row in idxList:
            if RealSignal.has_key(row):
                signal = self.__merge_lines(RealSignal[row][2], nChan)
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
            #LOG.debug('DataTable = %s, RealSignal = %s' % (tMASKLIST, RealSignal[row][2]))
            LOG.debug('DataTable = %s, RealSignal = %s' % (tMASKLIST, signal))
            if tMASKLIST == signal:
                #if type(tNOCHANGE) != int:
                if tNOCHANGE < 0:
                    # 2013/05/17 TN
                    # Put ITER itself instead to subtract 1 since iteration
                    # counter is incremented *after* baseline subtraction
                    # in refactorred code.
                    #self.datatable.tb2.putcell('NOCHANGE',row,ITER - 1)
                    self.datatable.tb2.putcell('NOCHANGE', row, ITER)
            else:
                #self.datatable.putcell('NOCHANGE',row,False)
                #self.datatable.tb2.putcell('MASKLIST',row,numpy.array(RealSignal[row][2]))
                self.datatable.tb2.putcell('MASKLIST',row,numpy.array(signal))
                self.datatable.tb2.putcell('NOCHANGE',row,-1)
        del GridCluster, RealSignal
        ProcEndTime = time.time()
        LOG.info('Clustering: Merging End: Elapsed time = %s sec' % (ProcEndTime - ProcStartTime))

        return Lines

    def clustering_analysis(self, Region, Region2, Nsigma):
        MedianWidth = numpy.median(Region2[:,0])
        LOG.debug('MedianWidth = %s' % MedianWidth)

        #MaxCluster = int(min(rules.ClusterRule['MaxCluster'], max(MaxLines + 1, (Npos ** 0.5)/2)))
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
                LOG.debug('codebook=%s'%(codebook))
                # Do iteration until no merging of clusters to be found
                while(NclusterNew != len(codebook)):
                    category, distance = VQ.vq(Region2, codebook)
                    LOG.info('Cluster Category&Distance %s, distance = %s' % (category, distance))

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

                    # Nsigma clipping/flagging with cluster distance 
                    # (set flag to 0)
                    Outlier = 0.0
                    #MaxDistance = []
                    for Nc in xrange(NclusterNew):
                        ValueList = distance[(category == Nc).nonzero()[0]]
                        Stddev = ValueList.std()
                        ### 2011/05/17 Strict the threshold
                        Threshold = ValueList.mean() + Stddev * Nsigma
                        #Threshold = Stddev * Nsigma
                        LOG.debug('Cluster Clipping Threshold = %s, Stddev = %s' % (Threshold, Stddev))
                        ### 2011/05/17 clipping iteration
                        #ValueList = ValueList.take((ValueList < Threshold).nonzero())[0]
                        #Stddev = ValueList.std()
                        #Threshold = Stddev * Nsigma
                        del ValueList
                        #LOG.debug('Cluster Clipping Threshold = %s, Stddev = %s' % (Threshold, Stddev))
                        for i in ((distance * (category == Nc)) > Threshold).nonzero()[0]:
                            Region[i][5] = 0
                            Outlier += 1.0
                        #MaxDistance.append(max(distance * ((distance < Threshold) * (category == Nc))))
                        LOG.debug('Region = %s' % Region)
                    MemberRate = (len(Region) - Outlier)/float(len(Region))
                    LOG.debug('MemberRate = %f' % MemberRate)

                    # Calculate Cluster Characteristics
                    Lines = []
                    for NN in xrange(NclusterNew):
#                        LineCenterList = []
#                        LineWidthList = []
#                        for x in range(len(category)):
#                            if category[x] == NN and Region[x][5] != 0:
#                                LineCenterList.append(Region2[x][1])
#                                LineWidthList.append(Region2[x][0])
#                        Lines.append([numpy.median(numpy.array(LineCenterList)), numpy.median(numpy.array(LineWidthList)), True, MaxDistance[NN]])
                        MaxDistance = max(distance * ((distance < Threshold) * (category == NN)))
                        # Region2: numpy.array
                        # [[line_center0, line_width0], 
                        #  [line_center1, line_width1],
                        #  ...]
                        indices = [x for x in xrange(len(category)) 
                                   if category[x] == NN and Region[x][5] != 0]
                        properties = Region2.take(indices, axis=0)
                        median_props = numpy.median(properties, axis=0)
                        Lines.append([median_props[1], median_props[0], True, MaxDistance])
                    LOG.debug('Lines = %s' % Lines)

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
                        BestLines = Lines[:]
                    elif Score < BestScore:
                        BestNcluster = NclusterNew
                        BestScore = Score
                        BestCategory = category.copy()
                        BestCodebook = codebook.copy()
                        BestRegion = Region[:]
                        BestLines = Lines[:]
                # TN refactoring
                # TmpList is no more used.
                #TmpList.append([NclusterNew, Score, codebook])
            ListBestScore.append(min(ListScore[index0:]))
            LOG.info('Ncluster = %d, BestScore = %f' % (NclusterNew, ListBestScore[-1]))
            if len(ListBestScore) > 3 and \
               ListBestScore[-4] <= ListBestScore[-3] and \
               ListBestScore[-4] <= ListBestScore[-2] and \
               ListBestScore[-4] <= ListBestScore[-1]:
                LOG.info('Determined the Number of Clusters to be %d' % (BestNcluster))
                break

        #Ncluster = BestNcluster
        #Region = BestRegion
        #category = BestCategory[:]
        Lines = [[book[1], book[0], True] for book in BestCodebook[:Ncluster]]
        #LOG.debug('Final: Ncluster = %s, Score = %s, Category = %s, CodeBook = %s, Lines = %s' % (Ncluster, BestScore, category, BestCodebook, Lines))
        # 2010/6/15 Plot the score along the number of the clusters
        LOG.todo('All plots should go into dispalys module')
        #SDP.ShowClusterScore(ListNcluster, ListScore, ShowPlot, FigFileDir, FigFileRoot)
        #SDP.ShowClusterInChannelSpace(Region2, BestLines, self.CLUSTER_WHITEN, ShowPlot, FigFileDir, FigFileRoot)
        LOG.info('Final: Ncluster = %s, Score = %s, Lines = %s' % (Ncluster, BestScore, Lines))

        return (BestNcluster, BestLines, BestCategory, BestRegion)

    def detection_stage(self, Ncluster, nra, ndec, x0, y0, GridSpaceRA, GridSpaceDEC, category, Region, DetectSignal):
        # Create Grid Parameter Space (Ncluster * nra * ndec)
        GridCluster = numpy.zeros((Ncluster, nra, ndec), dtype=numpy.float32)
        GridMember = numpy.zeros((nra, ndec))

        # Set Cluster on the Plane
        for row in xrange(len(DetectSignal)):
            # Check statistics flag added by G.K. 2008/1/17
            # Bug fix 2008/5/29
            #if DataTable[row][DT_SFLAG] == 1:
            # ResultTable is always created 
            #if ((len(ResultTable) == 0 and tSFLAG[row] == 1) or len(ResultTable) != 0):
                #print 'nra, ndec, row', nra, ndec, row
                #print 'GridMember', int((DetectSignal[row][0] - x0)/GridSpaceRA)
                #print 'GridMember', int((DetectSignal[row][1] - y0)/GridSpaceDEC)
            GridMember[int((DetectSignal[row][0] - x0)/GridSpaceRA)][int((DetectSignal[row][1] - y0)/GridSpaceDEC)] += 1
        for i in xrange(len(category)):
            if Region[i][5] == 1:
                try:
                    GridCluster[category[i]][int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] += 1.0
                except IndexError:
                    pass
        LOG.todo('Plots of clustering analysis should go into display module')
        #SDP.ShowCluster(GridCluster, [1.5, 0.5], Lines, Abcissa, x0, y0, GridSpaceRA, GridSpaceDEC, 'detection', ShowPlot, FigFileDir, FigFileRoot)

        return (GridCluster, GridMember)

    def validation_stage(self, GridCluster, GridMember, Lines, ITER):
                # Effective if number of spectrum which contains feature belongs to the cluster is greater or equal to the half number of spectrum in the Grid

        (Ncluster,nra,ndec) = GridCluster.shape

        for Nc in range(Ncluster):
            LOG.debug('Before: GridCluster[%s] = %s' % (Nc, GridCluster[Nc]))
            for x in range(nra):
                for y in range(ndec):
                    if GridMember[x][y] == 0: GridCluster[Nc][x][y] = 0.0
                    # if a single spectrum is inside the grid and has a feature belongs to the cluster, validity is set to 0.5 (for the initial stage) or 1.0 (iteration case).
                    elif GridMember[x][y] == 1 and GridCluster[Nc][x][y] > 0.9:
                        # 2013/05/20 TN
                        # Temporal workaround that line validation fails on
                        # test data if ITER is consistently handled.
                        #if ITER == 0: GridCluster[Nc][x][y] = 0.5
                        #else: GridCluster[Nc][x][y] = 1.0
                        GridCluster[Nc][x][y] = 1.0
                    # if the size of population is enough large, validate it as a special case 2007/09/05
                    # 2013/05/20 TN
                    # Temporal workaround that line validation fails on
                    # test data if ITER is consistently handled.
                    #elif ITER == 0:
                    #    GridCluster[Nc][x][y] = max(min(GridCluster[Nc][x][y] / sqrt(GridMember[x][y]) - 1.0, 3.0), GridCluster[Nc][x][y] / float(GridMember[x][y]))
                    else: GridCluster[Nc][x][y] = min(GridCluster[Nc][x][y] / sqrt(GridMember[x][y]), 3.0)
                    #else: GridCluster[Nc][x][y] = max(min(GridCluster[Nc][x][y] / GridMember[x][y]**0.5 - 1.0, 3.0), GridCluster[Nc][x][y] / float(GridMember[x][y]))
                    # normarize validity
                    #else: GridCluster[Nc][x][y] /= float(GridMember[x][y])

            if ((GridCluster[Nc] > self.Questionable)*1).sum() == 0: Lines[Nc][2] = False
            LOG.debug('After:  GridCluster[%s] = %s' % (Nc, GridCluster[Nc]))
        LOG.todo('Plots of clustering analysis should go into display module')
        #SDP.ShowCluster(GridCluster, [self.Valid, self.Marginal, self.Questionable], Lines, Abcissa, x0, y0, GridSpaceRA, GridSpaceDEC, 'validation', ShowPlot, FigFileDir, FigFileRoot)

        return (GridCluster, GridMember, Lines)

    def smoothing_stage(self, GridCluster, Lines):
        # Rating:  [0.0, 0.4, 0.5, 0.4, 0.0]
        #          [0.4, 0.7, 1.0, 0.7, 0.4]
        #          [0.5, 1.0, 6.0, 1.0, 0.5]
        #          [0.4, 0.7, 1.0, 0.7, 0.4]
        #          [0.0, 0.4, 0.5, 0.4, 0.0]
        # Rating = 1.0 / (Dx**2 + Dy**2)**(0.5) : if (Dx, Dy) == (0, 0) rating = 6.0
        (Ncluster,nra,ndec) = GridCluster.shape
        GridScore = numpy.zeros((2, nra, ndec), dtype=numpy.float32)
        for Nc in xrange(Ncluster):
            if Lines[Nc][2] != False:
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
                LOG.debug('Score :  GridScore[%s][0] = %s' % (Nc, GridScore[0]))
                LOG.debug('Rating:  GridScore[%s][1] = %s' % (Nc, GridScore[1]))
                GridCluster[Nc] = GridScore[0] / GridScore[1]
            if ((GridCluster[Nc] > self.Questionable)*1).sum() < 0.1: Lines[Nc][2] = False
        LOG.todo('Plots of clustering analysis should go into display module')
        #SDP.ShowCluster(GridCluster, [self.Valid, self.Marginal, self.Questionable], Lines, Abcissa, x0, y0, GridSpaceRA, GridSpaceDEC, 'smoothing', ShowPlot, FigFileDir, FigFileRoot)

        return (GridCluster, Lines)

    def final_stage(self, GridCluster, GridMember, Region, Region2, Lines, category, GridSpaceRA, GridSpaceDEC, BroadComponent, Xorder, Yorder, x0, y0, Nsigma, nChan, Grid2SpectrumID, idxList, PosList):
                
        (Ncluster, nra, ndec) = GridCluster.shape
        Xorder0 = Xorder
        Yorder0 = Yorder

        # Dictionary for final output
        RealSignal = {}

        #HalfGrid = (GridSpaceRA ** 2 + GridSpaceDEC ** 2) ** 0.5 / 2.0
        HalfGrid = 0.5 * sqrt(GridSpaceRA*GridSpaceRA + GridSpaceDEC*GridSpaceDEC)

        LOG.info('Ncluster=%s'%(Ncluster))
        
        # Clean isolated grids
        for Nc in xrange(Ncluster):
            #print '\nNc=', Nc
            LOG.info('Lines[%s][2]=%s'%(Nc,Lines[Nc][2]))
            if Lines[Nc][2] != False:
                Plane = (GridCluster[Nc] > self.Marginal) * 1
                LOG.info('Plane = %s'%(Plane))
                if Plane.sum() == 0:
                    Lines[Nc][2] = False
                    #print 'Lines[Nc][2] -> False'
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

                LOG.debug('Nmember = %s' % Nmember)
                LOG.debug('MemberList = %s' % MemberList)
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
                LOG.debug('Cluster Member = %s' % Nmember)

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
                    LOG.debug('Blur = %d' % Blur)
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
                    LOG.debug('kernel.shape = %s' % list(kernel.shape))
                    LOG.debug('GridCluster.shape = %s' % list(GridCluster.shape))
                    LOG.debug('Plane.shape = %s' % list(Plane.shape))
                    LOG.debug('SubPlane.shape = %s' % list(SubPlane.shape))
                    LOG.debug('BlurPlane.shape = %s' % list(BlurPlane.shape))
                    for x in xrange(len(Plane)):
                        LOG.debug(' %d : %s' % (x, list(Plane[x])))
                    for x in xrange(len(BlurPlane)):
                        LOG.debug(' %d : %s' % (x, list(BlurPlane[x])))
                    for x in xrange(len(ValidPlane)):
                        LOG.debug(' %d : %s' % (x, list(ValidPlane[x])))

                    # 2D fit for each Plane
                    # Use the data for fit if GridCluster[Nc][x][y] > self.Valid
                    # Not use for fit but apply the value at the border if GridCluster[Nc][x][y] > self.Marginal

                    # 2009/9/10 duplication of the position was taken into account (BroadComponent=True)
                    Bfactor = 1.0
                    if BroadComponent: Bfactor = 2.0
                    # Determine fitting order if not specified
                    #if Xorder < 0: Xorder0 = min(((numpy.sum(ValidPlane, axis=0) > 0.5)*1).sum() - 1, 5)
                    #if Yorder < 0: Yorder0 = min(((numpy.sum(ValidPlane, axis=1) > 0.5)*1).sum() - 1, 5)
                    if Xorder < 0: Xorder0 = int(min(((numpy.sum(ValidPlane, axis=0) > 0.5)*1).sum()/Bfactor - 1, 5))
                    if Yorder < 0: Yorder0 = int(min(((numpy.sum(ValidPlane, axis=1) > 0.5)*1).sum()/Bfactor - 1, 5))
                    #if Xorder < 0: Xorder0 = min(max(max(xlist) - min(xlist), 0), 5)
                    #if Yorder < 0: Yorder0 = min(max(max(ylist) - min(ylist), 0), 5)
                    LOG.debug('(X,Y)order = (%d, %d)' % (Xorder0, Yorder0))

                    # clear Flag
                    for i in xrange(len(category)): Region[i][5] = 1

                    if Xorder0 < 0 or Yorder0 < 0:
                        SingularMatrix = True
                        ExceptionLinAlg = False
                    else:
                        SingularMatrix = False
                        ExceptionLinAlg = True

                    while ExceptionLinAlg:
                        FitData = []
                        ### 2011/05/15 One parameter (Width, Center) for each spectra
                        #Region format:([row, line[0], line[1], DetectSignal[row][0], DetectSignal[row][1], flag])
                        dummy = [tuple(Region[i][:5]) for i in xrange(len(category))
                                 if category[i] == Nc and Region[i][5] == 1 and SubPlane[int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] > self.Valid]
                        (Lrow, Lmin, Lmax, LRA, LDEC) = dummy[0]
                        for i in xrange(1,len(dummy)):
                            if Lrow == dummy[i][0]:
                                Lmin = max(Lmin, dummy[i][1])
                                Lmax = min(Lmax, dummy[i][2])
                            else:
                                FitData.append([Lmin, Lmax, LRA, LDEC, 1])
                                (Lrow, Lmin, Lmax, LRA, LDEC) = dummy[i]
                        FitData.append([Lmin, Lmax, LRA, LDEC, 1])
                        del dummy
                        # TN refactoring
                        # make arrays for coefficient calculation
                        # Matrix    MM x A = B  ->  A = MM^-1 x B
                        # It is OK to prepare storage outside the loop 
                        # since Xorder0 and Yorder0 will not change inside 
                        # the loop. Whenever Xorder0 or Yorder0 changes, 
                        # exit the loop. See code below.
                        M0 = numpy.zeros((Xorder0 * 2 + 1) * (Yorder0 * 2 + 1), dtype=numpy.float64)
                        B0 = numpy.zeros((Xorder0 + 1) * (Yorder0 + 1), dtype=numpy.float64)
                        B1 = numpy.zeros((Xorder0 + 1) * (Yorder0 + 1), dtype=numpy.float64)
                        MM0 = numpy.zeros([(Xorder0 + 1) * (Yorder0 + 1), (Xorder0 + 1) * (Yorder0 + 1)], dtype=numpy.float64)
                        for iteration in xrange(3):
                            LOG.debug('2D Fit Iteration = %d' % iteration)

                            ### Commented out three lines 2011/05/15
                            # FitData format: [Width, Center, RA, DEC]
                            #for i in range(len(category)):
                            #    if category[i] == Nc and Region[i][5] == 1 and SubPlane[int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] > self.Valid:
                            #        FitData.append((Region2[i][0], Region2[i][1], Region[i][3], Region[i][4]))

                            # TN refactoring
                            # Comment out the following if statement since 
                            # 1) len(FitData) is always greater than 0. 
                            #    Lee the code just above start of iteration.
                            # 2) SingularMatrix is always False in this 
                            #    loop. Exit the loop whenever SingularMatrix 
                            #    is set to False. See code below.
                            #if len(FitData) == 0 or SingularMatrix: break
                            LOG.debug('FitData = %s' % FitData)

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
                                for k in xrange(Yorder0 * 2 + 1):
                                    xjyk = yk
                                    for j in xrange(Xorder0 * 2 + 1):
                                        M0[idx] += xjyk
                                        xjyk *= x
                                        idx += 1
                                    yk *= y
                                yk = 1.0
                                idx = 0
                                for k in xrange(Yorder0 + 1):
                                    xjyk = yk
                                    for j in xrange(Xorder0 + 1):
                                        B0[idx] += xjyk * Center
                                        B1[idx] += xjyk * Width
                                        xjyk *= x
                                        idx += 1
                                    yk *= y
                            LOG.debug('M0=%s\nB0=%s\nM1=%s\nB1=%s'%(M0,B0,M0,B1))

                            # make Matrix MM0,MM1 and calculate A0,A1
                            for K in xrange((Xorder0 + 1) * (Yorder0 + 1)):
                                k0 = K % (Xorder0 + 1)
                                k1 = int(K / (Xorder0 + 1))
                                for J in xrange((Xorder0 + 1) * (Yorder0 + 1)):
                                    j0 = J % (Xorder0 + 1)
                                    j1 = int(J / (Xorder0 + 1))
                                    MM0[J, K] = M0[j0 + k0 + (j1 + k1) * (Xorder0 * 2 + 1)]
                            LOG.debug('MM0 = %s' % MM0)
                            LOG.debug('B0 = %s' % B0)
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
                                if Xorder0 != 0 or Yorder0 != 0:
                                    ExceptionLinAlg = True
                                    LOG.debug('Xorder0,Yorder0 = %s,%s' % (Xorder0, Yorder0))
                                    Xorder0 = max(Xorder0 - 1, 0)
                                    Yorder0 = max(Yorder0 - 1, 0)
                                    LOG.debug('Exception Raised: Xorder0,Yorder0 = %s,%s' % (Xorder0, Yorder0))
                                else:
                                    SingularMatrix = True
                                    LOG.debug('SingularMatrix = True')
                                break

                            LOG.debug('A0 = %s' % A0)
                            LOG.debug('M[0] = %s' % M0[0])

                            # Calculate Sigma
                            # Sigma should be calculated in the upper stage
                            # Fit0: Center or Lmax, Fit1: Width or Lmin
                            Diff = []
                            # TN refactoring
                            # Calculation of Diff is duplicated here and 
                            # following clipping stage. So, evalueate Diff 
                            # only once here and reuse it in clipping.
                            for (Width, Center, x, y, flag) in FitData:
                                (Fit0, Fit1) = _eval_poly(Xorder0+1, Yorder0+1, x, y, A0, A1)
                                Fit0 -= Center
                                Fit1 -= Width
                                Diff.append(sqrt(Fit0*Fit0 + Fit1*Fit1))
                            #if len(Diff) > 1: Threshold = numpy.array(Diff).mean() + numpy.array(Diff).std() * Nsigma
                            #if len(Diff) > 1:
                            if len(effective) > 1:
                                npdiff = numpy.array(Diff)[effective]
                                Threshold = npdiff.mean()
                                #Threshold += npdiff.std()
                                Threshold += sqrt(numpy.square(npdiff - Threshold).mean()) * Nsigma
                            #if len(Diff) > 1: Threshold = numpy.array(Diff).std() * Nsigma
                            else: Threshold *= 2.0
                            LOG.debug('Diff = %s' % [Diff[i] for i in effective])
                            LOG.debug('2D Fit Threshold = %s' % Threshold)

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

                            LOG.debug('2D Fit Flagged/All = (%s, %s)' % (NFlagged, Number))
                            #2009/10/15 compare the number of the remainder and fitting order
                            if (Number - NFlagged) <= max(Xorder0, Yorder0) or Number == NFlagged:
                                SingularMatrix = True
                                LOG.debug('SingularMatrix = True')
                                break
                    # Iteration End
                    ### 2011/05/15 Fitting is no longer (Width, Center) but (minChan, maxChan)

                    # FitData: [(Width, Center, RA, DEC)]
                    if not SingularMatrix:
                        FitData = []
                        for i in range(len(category)):
                            if category[i] == Nc and Region[i][5] == 1 and SubPlane[int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] > self.Valid:
                                #FitData.append((Region2[i][0], Region2[i][1], Region[i][3], Region[i][4]))
                                FitData.append(tuple(Region2[i][:5]))
                        if len(FitData) == 0: continue

                        # Calculate Fit for each position
                        for x in xrange(nra):
                            for y in xrange(ndec):
                                if ValidPlane[x][y] == 1:
                                    for PID in Grid2SpectrumID[x][y]:
                                        ID = idxList[PID]
                                        ### 2011/05/15 (Width, Center) -> (minChan, maxChan)
                                        (Chan1, Chan0) = _eval_poly(Xorder0+1, Yorder0+1, PosList[0][PID], PosList[1][PID], A0, A1) 
                                        Fit0 = 0.5 * (Chan0 + Chan1)
                                        Fit1 = (Chan1 - Chan0) + 1.0
                                        LOG.debug('Fit0, Fit1 = %s, %s' % (Fit0, Fit1))
                                        if (Fit1 >= self.MinFWHM) and (Fit1 <= self.MaxFWHM):
                                            # Allowance = Fit1 / 2.0 * 1.3
                                            # To keep broad line region, make allowance larger
                                            ### 2011/05/16 allowance + 0.5 ->  2.5 for sharp line
                                            ### 2011/05/16 factor 1.5 -> 2.0 for broad line
                                            #Allowance = min(Fit1 / 2.0 * 2.0, MaxFWHM / 2.0)
                                            ### 2011/11/22 Allowance is too narrow for new line finding algorithm
                                            Allowance = min(Fit1 + 5.0, self.MaxFWHM / 2.0)
                                            ### 2011/10/21 left side mask exceeded nChan
                                            Protect = [min(max(int(Fit0 - Allowance), 0), nChan - 1), min(int(Fit0 + Allowance), nChan - 1)]
                                            #Allowance = Fit1 / 2.0 * 1.5
                                            #Protect = [max(int(Fit0 - Allowance - 0.5), 0), min(int(Fit0 + Allowance + 0.5), nChan - 1)]
                                            LOG.debug('0 Allowance = %s Protect = %s' % (Allowance, Protect))
                                            if RealSignal.has_key(ID):
                                                RealSignal[ID][2].append(Protect)
                                            else:
                                                RealSignal[ID] = [PosList[0][PID], PosList[1][PID] ,[Protect]]
                                elif BlurPlane[x][y] == 1:
                                    # in Blur Plane, Fit is not extrapolated, 
                                    # but use the nearest value in Valid Plane
                                    # Search the nearest Valid Grid
                                    Nearest = []
                                    square_aspect = GridSpaceRA / GridSpaceDEC
                                    square_aspect *= square_aspect
                                    Dist2 = numpy.inf
                                    for xx in range(nra):
                                        for yy in range(ndec):
                                            if ValidPlane[xx][yy] == 1:
                                                Dist3 = (xx-x)*(xx-x)*square_aspect + (yy-y)*(yy-y)
                                                if Dist2 > Dist3:
                                                    Nearest = [xx, yy]
                                                    Dist2 = Dist3
                                    (RA0, DEC0) = (x0 + GridSpaceRA * (x + 0.5), y0 + GridSpaceDEC * (y + 0.5))
                                    (RA1, DEC1) = (x0 + GridSpaceRA * (Nearest[0] + 0.5), y0 + GridSpaceDEC * (Nearest[1] + 0.5))

                                    # Setup the position near the border
                                    RA2 = RA1 - (RA1 - RA0) * HalfGrid / sqrt(Dist2)
                                    DEC2 = DEC1 - (DEC1 - DEC0) * HalfGrid / sqrt(Dist2)
                                    LOG.debug('[X,Y],[XX,YY] = [%d,%d],%s' % (x,y,Nearest))
                                    LOG.debug('(RA0,DEC0),(RA1,DEC1),(RA2,DEC2) = (%.5f,%.5f),(%.5f,%.5f),(%.5f,%.5f)' % (RA0,DEC0,RA1,DEC1,RA2,DEC2))
                                    # Calculate Fit and apply same value to all the spectra in the Blur Grid
                                    ### 2011/05/15 (Width, Center) -> (minChan, maxChan)
                                    # Border case
                                    #(Chan0, Chan1) = _eval_poly(Xorder0+1, Yorder0+1, RA2, DEC2, A0, A1)
                                    # Center case
                                    (Chan1, Chan0) = _eval_poly(Xorder0+1, Yorder0+1, RA1, DEC1, A0, A1)
                                    Fit0 = 0.5 * (Chan0 + Chan1)
                                    Fit1 = (Chan1 - Chan0)
                                    LOG.debug('Fit0, Fit1 = %s, %s' % (Fit0, Fit1))
                                    if (Fit1 >= self.MinFWHM) and (Fit1 <= self.MaxFWHM):
                                        #Allowance = Fit1 / 2.0 * 1.3
                                        # To keep broad line region, make allowance larger
                                        ### 2011/05/16 allowance + 0.5 ->  2.5 for sharp line
                                        ### 2011/05/16 factor 1.5 -> 2.0 for broad line
                                        #Allowance = min(Fit1 / 2.0 * 2.0, MaxFWHM / 2.0)
                                        ### 2011/11/22 Allowance is too narrow for new line finding algorithm
                                        Allowance = min(Fit1 + 5.0, self.MaxFWHM / 2.0)
                                        ### 2011/10/21 left side mask exceeded nChan
                                        Protect = [min(max(int(Fit0 - Allowance), 0), nChan - 1), min(int(Fit0 + Allowance), nChan - 1)]
                                        #Allowance = Fit1 / 2.0 * 1.5
                                        #Protect = [max(int(Fit0 - Allowance + 0.5), 0), min(int(Fit0 + Allowance + 0.5), nChan - 1)]

                                        LOG.debug('1 Allowance = %s Protect = %s' % (Allowance, Protect))
                                        for PID in Grid2SpectrumID[x][y]:
                                            ID = idxList[PID]
                                            if RealSignal.has_key(ID):
                                                RealSignal[ID][2].append(Protect)
                                            else:
                                                RealSignal[ID] = [PosList[0][PID], PosList[1][PID] ,[Protect]]
                                    else: continue
                    # for Plot
                    if not SingularMatrix: GridCluster[Nc] += BlurPlane
                LOG.info('GridCluster=%s'%(GridCluster))
                LOG.info('((GridCluster[%s] > 0.5)*1).sum()=%s'%(Nc,((GridCluster[Nc] > 0.5)*1).sum()))
                if ((GridCluster[Nc] > 0.5)*1).sum() < self.Questionable: Lines[Nc][2] = False
                for x in range(nra):
                    for y in range(ndec):
                        #if GridCluster[Nc][x][y] > 0.5: GridCluster[Nc][x][y] = 1.0
                        #if Original[x][y] > self.Valid: GridCluster[Nc][x][y] = 2.0
                        if Original[x][y] > self.Valid: GridCluster[Nc][x][y] = 2.0
                        elif GridCluster[Nc][x][y] > 0.5: GridCluster[Nc][x][y] = 1.0
                        
        LOG.todo('Plots of clustering analysis should go into display module')
        #SDP.ShowCluster(GridCluster, [1.5, 0.5, 0.5, 0.5], Lines, Abcissa, x0, y0, GridSpaceRA, GridSpaceDEC, 'regions', ShowPlot, FigFileDir, FigFileRoot)

        return (RealSignal, Lines)

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
            flat_lines = numpy.array(lines).reshape((nelem))
            sorted_index = flat_lines.argsort()
            flag = -1
            left_edge = flat_lines[sorted_index[0]]
            nedges=0
            for i in xrange(1,nelem-2):
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
            return sorted_index[:nedges].reshape((nedges/2,2))            

            #region = numpy.ones(nchan + 2, dtype=int)
            #for [chan0, chan1] in lines:
            #    region[chan0 + 1:chan1 + 1] = 0
            #dummy = (region[1:] - region[:-1]).nonzero()[0]
            #return dummy.reshape((len(dummy)/2,2)).tolist()


class ValidateLine(object):
    
    Patterns = {'SINGLE-POINT': ValidateLineSinglePointing,
                'MULTI-POINT': ValidateLineSinglePointing,
                'RASTER': ValidateLineRaster}

    def __init__(self, datatable):
        self.datatable = datatable

    def execute(self, ResultTable, DetectSignal, vIF, nChan, idxList, SpWin, Pattern, GridSpaceRA, GridSpaceDEC, ITER, Nsigma=3.0, Xorder=-1, Yorder=-1, BroadComponent=False, LogLevel=2, LogFile=False, ShowPlot=True, FigFileDir=False, FigFileRoot=False):
        """
        2D fit line characteristics calculated in Process3
        Sigma clipping iterations will be applied if Nsigma is positive
        order < 0 : automatic determination of fitting order (max = 5)
        SpWin: Pre-Defined Spectrum Window
         DetectSignal = {ID1: [RA, DEC, [[LineStartChannel1, LineEndChannel1],
                                         [LineStartChannel2, LineEndChannel2],
                                         [LineStartChannelN, LineEndChannelN]]],
                         IDn: [RA, DEC, [[LineStartChannel1, LineEndChannel1],
                                         [LineStartChannelN, LineEndChannelN]]]}

        Lines: output parameter
           [LineCenter, LineWidth, Validity]  OK: Validity = True; NG: Validity = False
        """

        # for Pre-Defined Spectrum Window
        if len(SpWin) != 0:
            LOG.info('Skip clustering analysis since predefined line window is set.')
            return SpWin

        # generate worker instance depending on observing pattern
        worker_cls = self.Patterns[Pattern]
        worker = worker_cls(self.datatable)
        return worker.execute(ResultTable, DetectSignal, vIF, nChan, idxList, GridSpaceRA, GridSpaceDEC, ITER, Nsigma=3.0, Xorder=-1, Yorder=-1, BroadComponent=False, LogLevel=2, LogFile=False, ShowPlot=True, FigFileDir=False, FigFileRoot=False)

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
