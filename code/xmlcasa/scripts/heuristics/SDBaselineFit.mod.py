from asap import *
import numarray as NA
import numarray.ma as NMA
import numarray.mlab as NML
import numarray.fft as FFT
import numarray.linear_algebra as LA
import pylab as PL
import sys
import time
import math
    
import SDTool as SDT
import SDPlotter as SDP

# Parameters
POLYNOMIAL_FIT = 'ASAP'
#POLYNOMIAL_FIT = 'SIMPLE'
#POLYNOMIAL_FIT = 'CLIPPING'
CLIP_CYCLE = 2

INITFFTORDER = 3
LineFindThreshold = 3
MaxFWHM = 500
MinFWHM = 5
SqrtMaxFWHM = math.sqrt(MaxFWHM)
SqrtMinFWHM = math.sqrt(MinFWHM)
Dilution = 5
HighLowFreqPartition = 9
MaxDominantFreq = 15
MAX_FREQ = 9
MAX_WINDOW_FRAGMENTATION = 3
MIN_CHANNELS = 512
NoData = -32767.0

# Old Parameters
FWHM_ALLOWANCE = [0.1, 10.0]
MASK_FWHM_FACTOR = 1.5
INITFWHM = 50.0
DEFAULT_POLY_ORDER = 10


def Process1(SpStorage, DataTable, rows, FFTORDER=INITFFTORDER, edge=(0, 0), LogLevel=2, LogFile=False):
    
    NROW = len(rows)

    # FFT > set lower order to 0 > InverseFFT
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process1  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='FFT, Reject lower freqency feature, InvFFT...')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d spectrum...' % NROW)
    SDT.LogMessage('TIMER', LogLevel, LogFile, Msg='|' + '='*36 + ' 100% ' + '=' * 36 + '|')
    StartTime = time.time()
    StatLowFreqPower = []
    STEP = HASH = HASH0 = 0
    # Set edge mask region
    if len(edge) == 2:
        (EdgeL, EdgeR) = edge
    else:
        EdgeR = edge[0]
        EdgeL = edge[0]
    Nedge = EdgeR + EdgeL
    
    NCHAN = DataTable[0][13]
    if Nedge >= NCHAN:
        SDT.LogMessage('ERROR', LogLevel, LogFile, Msg='Error: Edge masked region too large...')
        return False

    for x in range(NROW):
        STEP = STEP + 1
        if LogLevel > 0:
            HASH = int(80 * STEP / NROW)
            if (HASH0 != HASH):
                print '\b' + '*' * (HASH - HASH0),
                sys.stdout.flush()
                HASH0 = HASH
        # Set edge mask region to 0
        if EdgeL > 0: SpStorage[0][rows[x]][:EdgeL] = 0.0
        if EdgeR > 0: SpStorage[0][rows[x]][-EdgeR:] = 0.0
        # Apply FFT to the spectra
        SpFFT = FFT.fft(SpStorage[0][rows[x]])
        # Calculate power (RMS) indicator for lower frequencies (exclude bias level)
        DataTable[rows[x]][16] = math.sqrt(2.0) * NA.sum(abs(SpFFT[1:HighLowFreqPartition])) / float((NCHAN - Nedge))
        # Zero lower order coefficients
        SpFFT[0:FFTORDER] = 0.0
        # Restore to original domain
        SpStorage[1][rows[x]] = FFT.inverse_fft(SpFFT).real

    EndTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    return


def Process2(SpStorage, DataTable, PosDict, rows, LogLevel=2, LogFile=False):

    # The process does map and combine spectrum for each position
    # Format of PosDict:
    #     PosDict[row0] = [row0,row1,row2,...,rowN]
    #     PosDict[row1] = [row1,...,rowM]
    #  where row0,row1,...,rowN should be combined to one for better S/N spectra
    #       or
    #     PosDict[rowM] = [-1, ref]
    #  in case that the position of "rowM" is the same to that of "ref"
    # 'rows' represent spectra which required to be processed

    NROW = len(rows)
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process2  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Accumulate nearby spectrum for each position...')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d spectrum...' % NROW)
    SDT.LogMessage('TIMER', LogLevel, LogFile, Msg='|' + '='*36 + ' 100% ' + '=' * 36 + '|')
    StartTime = time.time()
    STEP = HASH = HASH0 = 0
    Processed = []
    Pending = []

    for row in rows:
        if PosDict[row][0] == -1:
            # Check the existence of the reference
            # Format: "ROW:-1,reference"
            if not PosDict[row][1] in rows:
                # Reference to non-existing row
                pass
            elif not PosDict[row][1] in Processed:
                # Reference Not ready
                Pending.append(row)
            else:
                # Copy referece spectrum
                SpStorage[2][row] = SpStorage[2][PosDict[row][1]].copy()
                Processed.append(row)
                STEP = STEP + 1
        else:
            # Combine listed spectra
            rowlist = []
            for y in PosDict[row]:
                # Check whether the row in the list is in rows
                if y in rows: rowlist.append(y)
            SDT.LogMessage('LONG', LogLevel, LogFile, Msg="Combine spectrum %s" % rowlist)
            if len(rowlist) == 0:
                SDT.LogMessage('WARNING', LogLevel, LogFile, Msg="Wrong format detected!!!")
                STEP = STEP + 1
            else:
                # Flag out the spectrum with larger RMS (Low frequency)
                StatLowFreqPower = []
                for x in rowlist:
                    StatLowFreqPower.append(DataTable[x][16])
                LowFreqPower = NA.array(StatLowFreqPower)
                Sigma = LowFreqPower.stddev()
                Mean = LowFreqPower.mean()
                MaxThreshold = Mean + 3.0 * Sigma
                for x in range(len(rowlist) - 1, -1, -1):
                    if DataTable[rowlist[x]][16] > MaxThreshold:
                        del rowlist[x]
#                SpStorage[2][row] = NML.median(SpStorage[1][rowlist])
                SpStorage[2][row] = NA.average(SpStorage[1][rowlist])
                Processed.append(row)
                STEP = STEP + 1

        if LogLevel > 0:
            HASH = int(80 * STEP / NROW)
            if (HASH0 != HASH):
                print '\b' + '*' * (HASH - HASH0),
                sys.stdout.flush()
                HASH0 = HASH

    if len(Pending) != 0:
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg="Process pending list %s" % Pending)
        # Deal with pending list
        for row in Pending:
            if not PosDict[row][1] in Processed:
                # Reference Not ready
                SDT.LogMessage('WARNING', LogLevel, LogFile, Msg="Wrong reference detected!!!")
            else:
                # Copy referece spectrum
                SpStorage[2][row] = SpStorage[2][PosDict[row][1]].copy()
                Processed.append(row)
            STEP = STEP + 1
            if LogLevel > 0:
                HASH = int(80 * STEP / NROW)
                if (HASH0 != HASH):
                    print '\b' + '*' * (HASH - HASH0),
                    sys.stdout.flush()
                    HASH0 = HASH

    EndTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    return


def Process3(SpStorage2, dummyscan, DataTable, ResultTable, PosDict, rows, Threshold=LineFindThreshold, edge=(0, 0), BroadComponent=True, LogLevel=2, LogFile=False):

    # The process find emission lines and determine protection regions for baselinefit
    # 'dummyscan' is a dummy scantable in order to use ASAP linefinder class

    NROW = len(rows)
    NCHAN = len(SpStorage2[rows[0]])
    scan = dummyscan.copy()
    LF = linefinder()
    BoxSize = [min(2.0*MaxFWHM/len(SpStorage2[0]), 0.5), max(MaxFWHM/len(SpStorage2[0])/4.0, 0.1)]
    Thre = [Threshold, Threshold * math.sqrt(2)]
    AvgLimit = [MinFWHM * 16, MinFWHM * 4]
    Processed = []
    Pending = []
    DetectSignal = {}
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process3  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Search regions for protection against the background subtraction...')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d spectrum...' % NROW)
    SDT.LogMessage('TIMER', LogLevel, LogFile, Msg='|' + '='*36 + ' 100% ' + '=' * 36 + '|')
    StartTime = time.time()
    STEP = HASH = HASH0 = 0
    # Set edge mask region
    if len(edge) == 2:
        (EdgeL, EdgeR) = edge
    else:
        EdgeR = edge[0]
        EdgeL = edge[0]
    Nedge = EdgeR + EdgeL
    if Nedge >= NCHAN:
        SDT.LogMessage('ERROR', LogLevel, LogFile, Msg='Error: Edge masked region too large...')
        return False

    if BroadComponent: Start = 0
    else: Start = 1

    for row in rows:
        STEP = STEP + 1
        if LogLevel > 0:
            HASH = int(80 * STEP / NROW)
            if (HASH0 != HASH):
                print '\b' + '*' * (HASH - HASH0),
                sys.stdout.flush()
                HASH0 = HASH

        if len(ResultTable) == 0:
            ProtectRegion = [DataTable[row][9], DataTable[row][10], []]
        else:
            ProtectRegion = [ResultTable[row][4], ResultTable[row][5], []]
            #               [RA,                DEC,                []]
        if len(ResultTable) == 0 and PosDict[row][0] == -1:
            # Reference to other spectrum
            X = int(PosDict[row][1])
            if X in Processed:
                DetectSignal[row] = DetectSignal[X]
                Processed.append(row)
                continue
            else:
                Pending.append(row)
                STEP = STEP - 1
                continue
        elif len(ResultTable) != 0 and ResultTable[row][6] == 0:
            # No spectrum
            ProtectRegion[2].append([-1, -1])
            DetectSignal[row] = ProtectRegion
            Processed.append(row)
        else:
            scan._setspectrum(SpStorage2[row])
            # Try to detect broader component and narrow component separately
            for y in range(Start, 2):
                LF.set_scan(scan)
                LF.set_options(threshold=Thre[y], box_size=BoxSize[y], min_nchan=MinFWHM, avg_limit=AvgLimit[y])
                nlines = LF.find_lines(edge=(EdgeL, EdgeR))
                # No-line is detected
                if (nlines == 0):
                    ProtectRegion[2].append([-1, -1])
                # Single line is detected
                elif (nlines == 1):
                    Ranges = LF.get_ranges(False)
                    Width = Ranges[1] - Ranges[0]
                    allowance = int(Width/10)
                    if Width >= MinFWHM and Width <= MaxFWHM and \
                       Ranges[0] > (EdgeL + allowance + 1) and \
                       Ranges[1] < (NCHAN - 2 - allowance - EdgeR):
                        ProtectRegion[2].append([Ranges[0] - allowance, Ranges[1] + allowance])
                    else:
                        ProtectRegion[2].append([-1, -1])
                # Multipule lines (candidates) are detected
                else:
                    linestat = []
                    Ranges = LF.get_ranges(False)
                    for y in range(nlines):
                        Width = Ranges[y*2+1] - Ranges[y*2]
                        allowance = int(Width/10)
                        if Width >= MinFWHM and Width <= MaxFWHM and \
                           Ranges[y*2] > (EdgeL + allowance + 1) and \
                           Ranges[y*2+1] < (NCHAN - 2 - allowance - EdgeR):
                            linestat.append((Ranges[y*2] - allowance, Ranges[y*2+1] + allowance, SpStorage2[row][Ranges[y*2]:Ranges[y*2+1]].max() - SpStorage2[row][Ranges[y*2]:Ranges[y*2+1]].min()))
                    # No candidate lines are left
                    if len(linestat) == 0:
                        ProtectRegion[2].append([-1, -1])
                    # More than or equal to one line are left
                    else:
                        Threshold = NA.transpose(NA.array(linestat))[2].max() / 10.0
                        for y in range(len(linestat)):
                            # Store line if max intensity exceeds 1/2 of the strongest one
                            if linestat[y][2] > Threshold:
                                ProtectRegion[2].append([linestat[y][0], linestat[y][1]])
            DetectSignal[row] = ProtectRegion
            Processed.append(row)
    if len(Pending) != 0 and len(ResultTable) == 0:
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg="Process pending list %s" % Pending)
        # Deal with pending list
        for row in Pending:
            if not PosDict[row][1] in Processed:
                # Reference Not ready
                SDT.LogMessage('WARNING', LogLevel, LogFile, Msg="Wrong reference detected!!!")
            else:
                # Copy referece detections
                DetectSignal[row] = DetectSignal[PosDict[row][1]]
                Processed.append(row)
            STEP = STEP + 1
            if LogLevel > 0:
                HASH = int(80 * STEP / NROW)
                if (HASH0 != HASH):
                    print '\b' + '*' * (HASH - HASH0),
                    sys.stdout.flush()
                    HASH0 = HASH

    EndTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    return DetectSignal




def Process4(DetectSignal, DataTable, ResultTable, GridTable, PosDict, rows, Pattern, GridSpaceRA, GridSpaceDEC, Nsigma=3.0, Xorder=-1, Yorder=-1, LogLevel=2, LogFile=False, FigFileDir=False, FigFileRoot=False):

    # 2D fit line characteristics calculated in Process3
    # Sigma clipping iterations will be applied if Nsigma is positive
    # order < 0 : automatic determination of fitting order (max = 5)
    #  DetectSignal = {ID1: [RA, DEC, [[LineStartChannel1, LineEndChannel1],
    #                                  [LineStartChannel2, LineEndChannel2],
    #                                  [LineStartChannelN, LineEndChannelN]]],
    #                  IDn: [RA, DEC, [[LineStartChannel1, LineEndChannel1],
    #                                  [LineStartChannelN, LineEndChannelN]]]}
    # GridTable format:
    #   [[IF,POL,0,0,RAcent,DECcent,[[row0,r0,RMS0],[row1,r1,RMS1],..,[rowN,rN,RMSN]]]
    #    [IF,POL,0,1,RAcent,DECcent,[[row0,r0,RMS0],[row1,r1,RMS1],..,[rowN,rN,RMSN]]]
    #                 ......
    #    [IF,POL,M,N,RAcent,DECcent,[[row0,r0,RMS0],[row1,r1,RMS1],..,[rowN,rN,RMSN]]]]

    import numpy
    import scipy.cluster.vq as VQ
    import numarray.convolve as NAC

    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process4  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='2D fit the line characteristics...')
    StartTime = time.time()
    TotalLines = 0
    RMS0 = 0.0
    Xorder0 = Xorder
    Yorder0 = Yorder
    # Dictionary for final output
    RealSignal = {}
    nChan = DataTable[0][13]
    Lines = []

    # First case
    if len(ResultTable) == 0:
        ROWS = rows
        if Pattern.upper() != 'RASTER':
            return (DetectSignal, Lines)
    # Iteration case
    else:
        ROWS = range(len(ResultTable))
        if Pattern.upper() == 'SINGLE-POINT':
            for row in rows:
                RealSignal[row] = DetectSignal[0]
            return (RealSignal, Lines)
        elif Pattern.upper() == 'MULTI-POINT':
            for x in range(len(GridTable)):
                for ID in DetectSignal.keys():
                    # Check position RA and DEC
                    if DetectSignal[ID][0] == GridTable[x][4] and \
                       DetectSignal[ID][1] == GridTable[x][5]:
                        for rowlist in GridTable[x][6]:
                            RealSignal[rowlist[0]] = DetectSignal[ID]
                        break
            return (DetectSignal, Lines)

    # RASTER CASE
    # Read data from Table to generate ID -> RA, DEC conversion table
    Region = []
    dummy = []
    flag = 1
    Npos = 0
    MaxLines = 0
    for row in ROWS:
        if len(DetectSignal[row][2]) > MaxLines: MaxLines = len(DetectSignal[row][2])
        if DetectSignal[row][2][0][0] != -1: Npos += 1
        for line in DetectSignal[row][2]:
            if line[0] != line[1]:
                Region.append([row, line[0], line[1], DetectSignal[row][0], DetectSignal[row][1], flag])
                dummy.append([float(line[1] - line[0]), (line[0] + line[1]) / 2.0])
    Region2 = numpy.array(dummy)
    del dummy
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='MaxLines = %s' % MaxLines)
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Npos = %s' % Npos)

    # Calculate Parameters for Gridding
    dummy = [[],[]]
    for row in rows:
        dummy[0].append(DataTable[row][9])
        dummy[1].append(DataTable[row][10])
    PosList = NA.array(dummy)
    del dummy

    wra = PosList[0].max() - PosList[0].min()
    wdec = PosList[1].max() - PosList[1].min()
    nra = int(wra / GridSpaceRA + 1)
    ndec = int(wdec / GridSpaceDEC + 1)
    x0 = PosList[0].min() - (nra * GridSpaceRA - wra) / 2.0
    y0 = PosList[1].min() - (ndec * GridSpaceDEC - wdec) / 2.0
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Grid = %d x %d\n' % (nra, ndec))

    # K-mean Clustering Analysis with LineWidth and LineCenter
    # Max 10 protect regions (Max 10 lines)
    # MaxCluster = 10
    MaxCluster = int(min(10, max(MaxLines + 1, (Npos ** 0.5)/2)))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='MaxCluster = %s' % MaxCluster)
    # Whiten is no more necessary 2007/2/12
    # whitened = VQ.whiten(Region2)
    TmpList = []
    # Determin the optimum number of clusters
    for Ncluster in range(1, MaxCluster + 1):
        #codebook, diff = VQ.kmeans(whitened, Ncluster)
        codebook, diff = VQ.kmeans(Region2, Ncluster)
        NclusterNew = 0
        # Do iteration until no merging of clusters to be found
        while(NclusterNew != len(codebook)):
            NclusterNew = len(codebook)
            category, distance = VQ.vq(Region2, codebook)
            for x in range(NclusterNew - 1, -1, -1):
                # Remove a cluster without any members
                if sum(NA.equal(category, x) * 1.0) == 0:
                    NclusterNew -= 1
                    tmp = list(codebook)
                    del tmp[x]
                    codebook = numpy.array(tmp)

            # Clear Flag
            for i in range(len(Region)): Region[i][5] = 1
            # Nsigma clipping/flagging with cluster distance (set flag to 0)
            for Nc in range(NclusterNew):
                ValueList = distance[(category == Nc).nonzero()[0]]
                Threshold = ValueList.mean() + ValueList.std() * Nsigma
                del ValueList
                SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Threshold = %s' % Threshold)
                for i in ((distance * (category == Nc)) > Threshold).nonzero()[0]:
                    Region[i][5] = 0
                SDT.LogMessage('LONG', LogLevel, LogFile, Msg='Region = %s' % Region)

            # Calculate Cluster Characteristics
            Lines = []
            for NN in range(NclusterNew):
                LineCenterList = []
                LineWidthList = []
                for x in range(len(category)):
                    if category[x] == NN and Region[x][5] != 0:
                        LineCenterList.append(Region2[x][1])
                        LineWidthList.append(Region2[x][0])
                Lines.append((NA.array(LineCenterList).mean(), NA.array(LineWidthList).mean()))
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Lines = %s' % Lines)

            # Rating
            Score = (distance * numpy.transpose(numpy.array(Region))[5]).mean() * (NclusterNew+ 1.0)
            if Ncluster == 1:
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

            # Merge Clusters if Diff(LineCenter) < Threshold
            #               and Diff(LineWidth) < Threshold,
            #  where Threshold = LineWidth / 4.0
            for x in range(NclusterNew):
                for y in range(x + 1, NclusterNew):
                    Threshold = max(Lines[x][1], Lines[y][1]) / 4.0
                    if abs(Lines[x][0] - Lines[y][0]) < Threshold and\
                       abs(Lines[x][1] - Lines[y][1]) < Threshold:
                        LineCenterList = []
                        LineWidthList = []
                        # print 'Old codebook:', codebook
                        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='OldCodebook = %s' % codebook)
                        for z in range(len(category)):
                            if (category[z] == x or category[z] == y) and Region[z][5] != 0:
                                LineCenterList.append(Region2[z][1])
                                LineWidthList.append(Region2[z][0])
                        codebook[x][0] = NA.array(LineWidthList).mean()
                        codebook[x][1] = NA.array(LineCenterList).mean()
                        tmp = list(codebook)
                        del tmp[y]
                        codebook = numpy.array(tmp)
                        # print 'New codebook:', codebook
                        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='NewCodebook = %s' % codebook)
                        x = NclusterNew
                        break
                if x == NclusterNew: break

        TmpList.append([NclusterNew, Score, codebook])
    # print    
    SDT.LogMessage('LONG', LogLevel, LogFile, Msg='Score = %s' % TmpList)
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Best = %s, %s, %s, %s, %s' % (BestNcluster, BestScore, BestCategory, BestCodebook, Lines))

    Ncluster = BestNcluster
    category = BestCategory
    Region = BestRegion
    Lines = BestLines

    # Create Grid Parameter Space (Ncluster * nra * ndec)
    GridCluster = NA.zeros((Ncluster, nra, ndec))
    GridMember = NA.zeros((nra, ndec))
    # Set Cluster on the Plane
    for i in range(len(category)):
        if Region[i][5] == 1:
            try:
                GridCluster[category[i]][int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] += 1
                GridMember[int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] += 1
            except IndexError:
                pass
    SDP.ShowCluster(GridCluster, Lines, 'init', FigFileDir, FigFileRoot)
    # Create Space for storing the list of spectrum (ID) in the Grid
    Grid2SpectrumID = []
    for x in range(nra):
        Grid2SpectrumID.append([])
        for y in range(ndec):
            Grid2SpectrumID[x].append([])
    for i in range(len(rows)):
        Grid2SpectrumID[int((PosList[0][i] - x0)/GridSpaceRA)][int((PosList[1][i] - y0)/GridSpaceDEC)].append(rows[i])
    SDT.LogMessage('LONG', LogLevel, LogFile, Msg='Grid2SpectrumID = %s' % Grid2SpectrumID)
    # Effective if number of spectrum which contains feature belongs to the cluster is greater or equal to the half number of spectrum in the Grid
    for Nc in range(Ncluster):
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Before: GridCluster[%s] = %s' % (Nc, GridCluster[Nc]))
        for x in range(nra):
            for y in range(ndec):
                if GridCluster[Nc][x][y] > 0:
                    if GridCluster[Nc][x][y] >= ((GridMember[x][y]) / 2.0):
                        GridCluster[Nc][x][y] = 1
                    else:
                        GridCluster[Nc][x][y] = 0
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='After:  GridCluster[%s] = %s' % (Nc, GridCluster[Nc]))
    SDP.ShowCluster(GridCluster, Lines, 'valid', FigFileDir, FigFileRoot)

    # Clean isolated grids
    for Nc in range(Ncluster):
        Plane = GridCluster[Nc].copy()
        # Clear GridCluster Nc-th plane
        GridCluster[Nc] *= 0
        Nmember = []
        MemberList = []
        NsubCluster = 0
        for x in range(nra):
            for y in range(ndec):
                if Plane[x][y] == 1:
                    Plane[x][y] = 2
                    SearchList = [(x, y)]
                    M = 1
                    MemberList.append([(x, y)])
                    while(len(SearchList) != 0):
                        cx, cy = SearchList[0]
                        for dx in [-1, 0, 1]:
                            for dy in [-1, 0, 1]:
                                (nx, ny) = (cx + dx, cy + dy)
                                if 0 <= nx < nra and 0 <= ny < ndec and Plane[nx][ny] == 1:
                                    Plane[nx][ny] = 2
                                    SearchList.append((nx, ny))
                                    M += 1
                                    MemberList[NsubCluster].append((nx, ny))
                        del SearchList[0]
                    Nmember.append(M)
                    NsubCluster += 1

        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Nmember = %s' % Nmember)
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='MemberList = %s' % MemberList)
        # If no members left, skip to next cluster
        if len(Nmember) == 0: continue
        # Threshold is set to half the number of the largest cluster in the plane
        Threshold = max(Nmember) / 2.0
        for n in range(NsubCluster -1, -1, -1):
            # isolated cluster made from single spectrum should be omitted
            if Nmember[n] == 1:
                (x, y) = MemberList[n][0]
                if GridMember[x][y] < 2:
                    Nmember[n] = 0
            # Sub-Cluster whose member below the threshold is cleaned
            if Nmember[n] < Threshold:
                for (x, y) in MemberList[n]:
                    Plane[x][y] == 0
                del Nmember[n]
                del MemberList[n]
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Cluster Member = %s' % Nmember)

        # Blur each SubCluster with the radius of sqrt(Nmember/Pi) * ratio
        ratio = 0.1
        # Set-up SubCluster
        for n in range(len(Nmember)):
            SubPlane = NA.zeros((nra, ndec))
            for (x, y) in MemberList[n]:
                SubPlane[x][y] = 1
            # Calculate Blur radius
            Blur = int((Nmember[n] / 3.141592653) ** 0.5 * ratio + 1)
            # Set-up kernel for convolution
            kernel = NA.zeros((Blur * 2 + 1, Blur * 2 + 1))
            for x in range(Blur * 2 + 1):
                for y in range(Blur * 2 + 1):
                    if abs(Blur - x) + abs(Blur - y) <= Blur:
                        kernel[x][y] = 1
            BlurPlane = (NAC.convolve2d(SubPlane, kernel) != 0)
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='GridCluster.shape = %s' % list(GridCluster.shape))
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Plane.shape = %s' % list(Plane.shape))
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='SubPlane.shape = %s' % list(SubPlane.shape))
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='BlurPlane.shape = %s' % list(BlurPlane.shape))

            # 2D fit for each Plane
            # Use the data for fit if GridCluster[Nc][x][y] == 1
            # Not use for fit but extrapolate the fit if GridCluster[Nc][x][y] == 2
            # Ignore if GridCluster[Nc][x][y] == 0
            # FitData: [(Width, Center, RA, DEC)]
            FitData = []
            for i in range(len(category)):
                if category[i] == Nc and Region[i][5] == 1 and SubPlane[int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] == 1:
                    FitData.append((Region2[i][0], Region2[i][1], Region[i][3], Region[i][4]))
            if len(FitData) == 0: continue

            # Determine fitting order if not specified
            if Xorder < 0: Xorder0 = min(int(math.sqrt(Nmember[n])/4.0), 5)
            if Yorder < 0: Yorder0 = min(int(math.sqrt(Nmember[n])/4.0), 5)

            # make arrays for coefficient calculation
            # Matrix    MM x A = B  ->  A = MM^-1 x B
            M0 = NA.zeros((Xorder0 * 2 + 1) * (Yorder0 * 2 + 1), type=NA.Float64)
            M1 = NA.zeros((Xorder0 * 2 + 1) * (Yorder0 * 2 + 1), type=NA.Float64)
            B0 = NA.zeros((Xorder0 + 1) * (Yorder0 + 1), type=NA.Float64)
            B1 = NA.zeros((Xorder0 + 1) * (Yorder0 + 1), type=NA.Float64)
            MM0 = NA.zeros([(Xorder0 + 1) * (Yorder0 + 1), (Xorder0 + 1) * (Yorder0 + 1)], type=NA.Float64)
            MM1 = NA.zeros([(Xorder0 + 1) * (Yorder0 + 1), (Xorder0 + 1) * (Yorder0 + 1)], type=NA.Float64)
            Z0 = 0.0
            Z1 = 0.0
            for (Width, Center, x, y) in FitData:
                Z0 += (Center ** 2)
                for k in range(Yorder0 * 2 + 1):
                    for j in range(Xorder0 * 2 + 1):
                        M0[j + k * (Xorder0 * 2 + 1)] += math.pow(x, j) * math.pow(y, k)
                for k in range(Yorder0 + 1):
                    for j in range(Xorder0 + 1):
                        B0[j + k * (Xorder0 + 1)] += math.pow(x, j) * math.pow(y, k) * Center
                Z1 += (Width ** 2)
                for k in range(Yorder0 * 2 + 1):
                    for j in range(Xorder0 * 2 + 1):
                        M1[j + k * (Xorder0 * 2 + 1)] += math.pow(x, j) * math.pow(y, k)
                for k in range(Yorder0 + 1):
                    for j in range(Xorder0 + 1):
                        B1[j + k * (Xorder0 + 1)] += math.pow(x, j) * math.pow(y, k) * Width

            # make Matrix MM0,MM1 and calculate A0,A1
            for K in range((Xorder0 + 1) * (Yorder0 + 1)):
                k0 = K % (Xorder0 + 1)
                k1 = int(K / (Xorder0 + 1))
                for J in range((Xorder0 + 1) * (Yorder0 + 1)):
                    j0 = J % (Xorder0 + 1)
                    j1 = int(J / (Xorder0 + 1))
                    MM0[J, K] = M0[j0 + k0 + (j1 + k1) * (Xorder0 * 2 + 1)]
                    MM1[J, K] = M1[j0 + k0 + (j1 + k1) * (Xorder0 * 2 + 1)]
            A0 = LA.solve_linear_equations(MM0, B0)
            A1 = LA.solve_linear_equations(MM1, B1)
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='A0 = %s' % A0)
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='M[0] = %s' % M0[0])
            if LogLevel > 2:
                OutPlane = SubPlane.copy()
            # Calculate Fit for each position
            for x in range(nra):
                for y in range(ndec):
                    if BlurPlane[x][y] == 1:
                        for ID in Grid2SpectrumID[x][y]:
                            Fit0 = Fit1 = 0.0
                            PID = rows.index(ID)
                            for k in range(Yorder0 + 1):
                                for j in range(Xorder0 + 1):
                                    Coef = math.pow(PosList[0][PID], j) * math.pow(PosList[1][PID], k)
                                    Fit0 += Coef * A0[j + k * (Xorder0 + 1)]
                                    Fit1 += Coef * A1[j + k * (Xorder0 + 1)]
                            if (Fit1 >= MinFWHM) and (Fit1 <= MaxFWHM):
                                Allowance = Fit1 / 2.0 * 1.3
                                if RealSignal.has_key(ID):
                                    tmplist = RealSignal[ID][2]
                                    tmplist.append([max(int(Fit0 - Allowance), 0), min(int(Fit0 + Allowance), nChan - 1)])
                                    RealSignal[ID][2] = tmplist
                                else:
                                    RealSignal[ID] = [DataTable[ID][9], DataTable[ID][10] ,[[max(int(Fit0 - Allowance), 0), min(int(Fit0 + Allowance), nChan - 1)]]]
                            if LogLevel > 2: OutPlane[x][y] = Fit1
                    elif LogLevel > 2: OutPlane[x][y] = 0
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='FinalCluster:\n%s' % OutPlane)
            if LogLevel > 2:
                del OutPlane
            # for Plot
            GridCluster[Nc] += BlurPlane
    SDP.ShowCluster(GridCluster, Lines, 'final', FigFileDir, FigFileRoot)
    # Merge masks if possible
    for row in rows:
        if RealSignal.has_key(row):
            if len(RealSignal[row][2]) != 1:
                Region = NA.ones(nChan + 2)
                for [chan0, chan1] in RealSignal[row][2]:
                    Region[chan0 + 1:chan1 + 1] = 0
                dummy = (Region[1:] - Region[:-1]).nonzero()
                RealSignal[row][2] = []
                for y in range(0, len(dummy[0]), 2):
                    RealSignal[row][2].append([dummy[0][y], dummy[0][y + 1]])
        else:
            RealSignal[row] = [DataTable[row][9], DataTable[row][10], [[-1, -1]]]

    del GridCluster
    EndTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    return (RealSignal, Lines)



def Process5(SpStorage, TimeTable, rows, RealSignal, edge=(0, 0), LogLevel=2, LogFile=False):

    # Determine Baseline-Fitting order for each time-bin
    #  RealSignal = {ID1,[RA,DEC,[[LineStartChannel1, LineEndChannel1],
    #                             [LineStartChannel2, LineEndChannel2],
    #                             [LineStartChannelN, LineEndChannelN]]],
    #                IDn,[RA,DEC,[[LineStartChannel1, LineEndChannel1],
    #                             [LineStartChannelN, LineEndChannelN]]]}

    NROW = len(rows)
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process5  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Determine Baseline-Fitting order for each time-bin...')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d spectrum...' % NROW)
    StartTime = time.time()
    # Set edge mask region
    if len(edge) == 2:
        (EdgeL, EdgeR) = edge
    else:
        EdgeR = edge[0]
        EdgeL = edge[0]
    Nedge = EdgeR + EdgeL
    NCHAN = len(SpStorage[0][0]) - Nedge

    SP = SpStorage[0].copy()
    # Line Protection before FFT
    for row in rows:
        if EdgeL > 0: SP[row][:EdgeL] = 0.0
        if EdgeR > 0: SP[row][-EdgeR:] = 0.0
        for line in RealSignal[row][2]:
            if line[0] != -1:
                SP[row][line[0]:line[1]] = 0.0

    # re-map 'rows' by referring the table
#    infile = open(GroupTimeTable, 'r')
#    GroupList = []
#    while 1:
#        line = infile.readline()
#        if not line: break
#        elif line[0] == '#': continue
#        elif 'GROUP:' in line.upper():
#            Member = [int(x) for x in line.split(':')[1].split(',')]
#            GroupList.append(Member)
#    infile.close()
    # create list for time-separated group members in rows

    GroupList = TimeTable

    TGroup = []
    for y in range(len(GroupList)): TGroup.append([])
    N = 0
    for x in rows:
        for y in range(len(GroupList)):
            if int(x) in GroupList[y]:
                TGroup[y].append(int(x))
                N += 1
                break
    if N != len(rows):
        SDT.LogMessage('ERROR', LogLevel, LogFile, Msg='Missing member in TimeTable')
        return False

    STEP = HASH = HASH0 = 0
    SDT.LogMessage('TIMER', LogLevel, LogFile, Msg='|' + '='*36 + ' 100% ' + '=' * 36 + '|')
    output = []
    for y in range(len(TGroup)):
        SpFFT = []
        for x in TGroup[y]:
            STEP = STEP + 1
            if LogLevel > 0:
                HASH = int(80 * STEP / NROW)
                if (HASH0 != HASH):
                    print '\b' + '*' * (HASH - HASH0),
                    sys.stdout.flush()
                    HASH0 = HASH
            # Apply FFT to the spectra
            SpFFT.append(NA.abs(FFT.real_fft(SP[x])))
        Power = NML.median(SpFFT)
        # Normalize to 2K case
        N = int(MaxDominantFreq * NCHAN / 2048.0)
        for x in range(N):
            Power[x] *= (x / float(len(Power)))
        P2 = Power[:N].argsort()
        xsum = psum = 0.0
        for x in range(N-1, N-4, -1):
            psum += Power[P2[x]] * P2[x]
            xsum += Power[P2[x]]
        # Calculate order
        Order = psum / float(xsum)
        for x in TGroup[y]:
            output.append([x, Order])

    EndTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    return dict(output)



def GroupByTime(DataTable, basedata, rows, LogLevel=2, LogFile=False):
    '''
    Grouping by time sequence
    TimeTable: [[row0, row1,..., rowN], [rowXX,..., rowXXX],...,[,,]]
    TimeGap: [rowX1, rowX2,...,rowXN]
    '''
    
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  GroupByTime  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Grouping by Time...')

    plotdata = [[],[]]
    for ID in basedata:
        plotdata[0].append(DataTable[ID][0])
        plotdata[1].append(DataTable[ID][7])
    Nrows = len(basedata)

    DeltaT = NA.array(plotdata[1]) - NA.array([plotdata[1][0]] + plotdata[1][:-1])
    ThresholdT = NML.median(DeltaT) * 10.0
    SDT.LogMessage('LONG', LogLevel, LogFile, Msg='plotdata[1] = %s' % plotdata[1])
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Threshold = %s' % ThresholdT)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='MaxDeltaT = %s' % DeltaT.max())
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='MinDeltaT = %s' % DeltaT.min())

    TimeTable = []
    SubTable = []
    TimeGap = []
    for x in range(Nrows):
        if plotdata[0][x] in rows:
            if DeltaT[x] <= ThresholdT:
                SubTable.append(plotdata[0][x])
            else:
                TimeTable.append(SubTable)
                SubTable = [plotdata[0][x]]
                TimeGap.append(plotdata[0][x])
    if len(SubTable) > 0: TimeTable.append(SubTable)
    del SubTable, plotdata
    
    return (TimeTable, TimeGap)



def GroupByPosition(DataTable, rows, CombineRadius, AllowanceRadius, LogLevel=2, LogFile=False):
    '''
    Grouping by RA/DEC position
    PosGap: [rowX1, rowX2,...,rowXN]
    '''
    
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  GroupByPosition  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Grouping by Position...')

    plotdata = [[],[],[]]
    for ID in rows:
        plotdata[0].append(DataTable[ID][0])
        plotdata[1].append(DataTable[ID][9])
        plotdata[2].append(DataTable[ID][10])
    Nrows = len(rows)

    Dict = {}
    ThresholdR = CombineRadius * CombineRadius
    ThresholdAR = AllowanceRadius * AllowanceRadius

    ArrayRA = NA.array(plotdata[1])
    ArrayDEC = NA.array(plotdata[2])
    for x in range(Nrows):
        if not Dict.has_key(plotdata[0][x]):
            DeltaRA = ArrayRA - plotdata[1][x]
            DeltaDEC = ArrayDEC - plotdata[2][x]
            Delta2 = DeltaRA * DeltaRA + DeltaDEC * DeltaDEC
            Unique = NA.less_equal(Delta2, ThresholdAR)
            Select = NA.less_equal(Delta2, ThresholdR)
            for y in range(x + 1, Nrows):
                if Unique[y] == 1 and (not Dict.has_key(plotdata[0][y])):
                    Dict[plotdata[0][y]] = [-1,plotdata[0][x]]
            line = []
            for y in range(Nrows):
                if Select[y] == 1: line.append(plotdata[0][y])
            Dict[plotdata[0][x]] = line

    DeltaP = NA.sqrt((ArrayRA[1:] - ArrayRA[:-1])**2.0 + (ArrayDEC[1:] - ArrayDEC[:-1])**2.0)
    ThresholdP = NML.median(DeltaP) * 10.0
    PosGap = []
    for x in range(1, Nrows):
        if DeltaP[x - 1] > ThresholdP:
            PosGap.append(rows[x])

    return (Dict, PosGap)



def ObsPatternAnalysis(PosDict, rows, LogLevel=2, LogFile=False):
    '''
    Analyze pointing pattern
    '''
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  ObsPatternAnalysis  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Analyze Scan Pattern by Positions...')
    nPos = 0
    MaxIntegration = 0
    for row in rows:
        if len(PosDict[row]) > MaxIntegration:
            MaxIntegration = len(PosDict[row])
        if PosDict[row][0] != -1:
            nPos += 1
        if nPos == 0: nPos = 1
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Number of Spectra: %d,   Number of independent position: %d' % (len(rows), nPos))
    if nPos > math.sqrt(len(rows)) and MaxIntegration < len(rows) / 4: ret = 'RASTER'
    elif nPos == 1: ret = 'SINGLE-POINT'
    else: ret = 'MULTI-POINT'
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Pattern is %s' % (ret))
    return ret



def Process6(SpStorage, DataTable, rows, scan, DictFitOrder, RealSignal, TimeGap=[], PosGap=[], showspectrum=True, showplot=True, fixscale=True, stepbystep=False, showevery=10, saveparams=True, savefile='BaselineFitRating.csv', edge=(0, 0), LogLevel=2, LogFile=False, FigFileDir=False, FigFileRoot=False):
    
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process6  ' + '>'*15)
    while saveparams and (savefile == ''):
        print 'Name for output parameter file? ==>'
        savefile = raw_input()

    if stepbystep: showevery = 1
    if showplot: SDP.ShowResult(mode='init', title='Statistics Plot: Baseline RMS for Raw and Processed data')

    ########## Baseline fit for each spectrum ##########

    # Duplicate scantable object for output
    dummyscan = scan.copy()
    del scan

    # Create Logging file
    if saveparams:
        output = open(savefile, 'w')
        output.write("#parameters,filename,FWHM_ALLOWANCE_MIN,FWHM_ALLOWANCE_MAX,MASK_FWHM_FACTOR,INITFWHM,MAX_FREQ,DEFAULT_POLY_ORDER,MAX_WINDOW_FRAGMENTATION\n")
        output.write("#parameters,%d,%d,%f,%f,%d,%d,%d\n" % (FWHM_ALLOWANCE[0],FWHM_ALLOWANCE[1],MASK_FWHM_FACTOR,INITFWHM,MAX_FREQ,DEFAULT_POLY_ORDER,MAX_WINDOW_FRAGMENTATION))
        output.write("!Row,Channels,nMask,Fragment,nWindow,PolyOrder,Modification,WinPolyOrder,InitRMS,RMS,InitDiff,Diff,Score,Peak,FWHM,Cent,GaussChi2,LorPeak,LorFWHM,LorentzChi2,RawRating,FitRating\n")
        output.close()

    # Initialize variables
    n = 0
    Rows = []
    InitRMS = []
    RMS = []
    InitPower = []
    Power = []
    RawRating = []
    FitRating = []
    NROW = len(rows)

    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Baseline Fit: background subtraction...')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d spectrum...' % NROW)
    StartTime = time.time()

    # Set edge mask region
    if len(edge) == 2:
        (EdgeL, EdgeR) = edge
    else:
        EdgeR = edge[0]
        EdgeL = edge[0]
    Nedge = EdgeR + EdgeL
    edge = (EdgeL, EdgeR)

    # Main loop for baseline fit
    DrawFlag = False
    counter = 0
    for row in rows:
        counter += 1
        SDT.LogMessage('TIMER', LogLevel, LogFile, Msg='=' * 30 + 'Processing at row = %s' % row  + '=' * 30)
        # modification is polynomial order offset value for manual override
        modification = 0
        Rows.append(row)
        dummyscan._setspectrum(SpStorage[0][row])
        NCHAN = len(SpStorage[0][row])
        polyorder = int(DictFitOrder[row] * 3.0)
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Masked Region = %s' % RealSignal[row])
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='polyorder = %d edge= %s' % (polyorder, edge))
        (SpStorage[1][row], NewRMS, OldRMS, nmask, win_polyorder, fragment, nwindow) = CalcBaselineFit(dummyscan, RealSignal[row][2], polyorder, NCHAN, 0, edge)
        ##(SpStorage[1][row], NewRMS, OldRMS, nmask, win_polyorder, fragment, nwindow) = CalcBaselineFit(dummyscan, RealSignal[row][2], polyorder, NCHAN, 0, edge, 3, LogFile)
        BestScore = NewRMS * polyorder

        # Plot result spectra
        if showspectrum:
            if row == rows[-1]: DrawFlag = True
            xaxis = NA.arange(NCHAN)
            YMIN = min(SpStorage[0][row].min(), SpStorage[1][row].min())
            YMAX = max(SpStorage[0][row].max(), SpStorage[1][row].max())
            Yrange = [YMIN-(YMAX-YMIN)/10.0, YMAX+(YMAX-YMIN)/10.0]
            Xrange = [0, NCHAN]

            fitparams = 'poly_order=%d : %d  nwindow=%d : %d     ' % (polyorder, win_polyorder, fragment, nwindow)
            statistics = 'RMS=%.2f, Score=%.2f' % (OldRMS, BestScore)
            SDP.DrawFitSpectrum(xaxis, SpStorage[0][row], SpStorage[1][row], row, counter, NROW, fitparams, statistics, Xrange, Yrange, RealSignal[row][2], DrawFlag, FigFileDir, FigFileRoot)

        # Interactive fitting and rating
        rawrating = 9
        fitrating = 9
        if stepbystep:
            quit = False
            while 1:
                print 'If series of characters are set, first character for Raw data rating and 2nd for Fit data rating'
                print 'Raw data Rating: 0 or <Ret> - Good,  1 - Acceptable,  2 - Unacceptable,  s - skip,  q - quit'
                print 'Need retry?  (+) raise fitting order by 1,   (-) drop fitting order by 1'
                print '   Sample input: 10<ret> for acceptable raw data and Good fit'
                rating = raw_input()
                if len(rating) == 0: rating = '00'
                elif len(rating) == 1: rating = rating + '0'

                if rating.upper()[:2].find('Q') != -1:
                    quit = True
                    break
                if rating.upper()[:2].find('S') != -1:
                    break

                if rating[:2].find('+') != -1:
                    modification = modification + 1
                elif rating[:2].find('-') != -1:
                    modification = modification - 1
                    if (modification + polyorder) < 0:
                        modification = modification + 1
                        print 'fitting order reached Zero'
                else:
                    if rating[0] == '1': rawrating = 1
                    elif rating[0] == '2': rawrating = 2
                    else: rawrating = 0
                    if rating[1] == '1': fitrating = 1
                    elif rating[1] == '2': fitrating = 2
                    else: fitrating = 0
                    break

                # Re-fit with modified polynomial order (order = polyorder + modification)
                (SpStorage[1][row], NewRMS, OldRMS, nmask, win_polyorder, fragment, nwindow) = CalcBaselineFit(dummyscan, RealSignal[row][2], polyorder, NCHAN, modification, edge)
                ##(SpStorage[1][row], NewRMS, OldRMS, nmask, win_polyorder, fragment, nwindow) = CalcBaselineFit(dummyscan, RealSignal[row][2], polyorder, NCHAN, modification, edge, 2, LogFile)
                BestScore = NewRMS * (polyorder + modification)
                print 'InitRMS, RMS, Score =', OldRMS, NewRMS, BestScore
                if showspectrum:
                    fitparams = 'poly_order=%d :  %d : %d  nwindow=%d : %d     ' % (polyorder, modification, win_polyorder, fragment, nwindow)
                    statistics = 'RMS=%.2f  Score=%.2f' % (NewRMS, BestScore)
                    SDP.DrawFitSpectrum(xaxis, SpStorage[0][row], SpStorage[1][row], row, 1, 1, fitparams, statistics, Xrange, Yrange, RealSignal[row][2])
            if quit: break

        # Store result spectra and several information
        RawRating.append(rawrating)
        FitRating.append(fitrating)
        InitRMS.append(OldRMS)
        RMS.append(NewRMS)
        # The following will be deleted
        OldPower = NewPower = 0.0
        InitPower.append(OldPower)
        Power.append(NewPower)

        # Save Parameters
        if saveparams:
            output = open(savefile, 'a')
            output.write("%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%d,%d\n" % (row, NCHAN, nmask, fragment, nwindow, polyorder, modification, win_polyorder, InitRMS[n], RMS[n], InitPower[n], Power[n], BestScore, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, rawrating, fitrating))
            output.close()

        DataTable[row][17] = NewRMS
        DataTable[row][18] = OldRMS
        # Plot result statistics
        if ((n + 1) % showevery == 0 or row == rows[-1]) and showplot:
            N1 = n + 1 - showevery
            N2 = n + 1
            UpdateRows = Rows[N1:N2]
            SDP.ShowResult('online', DataTable, UpdateRows, TimeGap, PosGap)

        # Add Spectra position index (offset)
        n = n + 1
            
    del dummyscan
    EndTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))

    return



def Process7(SpStorage, DataTable, rows, RealSignal, TimeGap=[], PosGap=[], Nmean=5, ThreNewRMS=3.0, ThreOldRMS=5.0, ThreNewDiff=5.0, ThreOldDiff=5.0, Iteration=5, showplot=True, interactive=True, edge=(0,0), LogLevel=2, LogFile=False, FigFileDir=False, FigFileRoot=False):

    # Calculate RMS and Diff from running mean

    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process7  ' + '>'*15)
    NROW = len(rows)
    NCHAN = len(SpStorage[0][rows[0]])
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Calculate RMS and Diff from running mean for Pre/Post fit...')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d spectrum...' % NROW)
    SDT.LogMessage('TIMER', LogLevel, LogFile, Msg='|' + '='*36 + ' 100% ' + '=' * 36 + '|')
    StartTime = time.time()
    STEP = HASH = HASH0 = 0
    data = []
    flag = 1
    Threshold = [ThreNewRMS, ThreOldRMS, ThreNewDiff, ThreOldDiff]

    for row in rows:
        STEP = STEP + 1
        if LogLevel > 0:
            HASH = int(80 * STEP / NROW)
            if (HASH0 != HASH):
                print '\b' + '*' * (HASH - HASH0),
                sys.stdout.flush()
                HASH0 = HASH
        mask = NA.ones(NCHAN, NA.Int)
        for [m0, m1] in RealSignal[row][2]: mask[m0:m1] = 0
        if edge[0] > 0: mask[:edge[0]] = 0
        if edge[1] > 0: mask[-edge[1]:] = 0
        Nmask = int(NCHAN - NA.sum(mask * 1.0))

        # Calculate RMS after/before fitting
        MaskedData = SpStorage[1][row] * mask
        StddevMasked = MaskedData.stddev()
        MeanMasked = MaskedData.mean()
        NewRMS = math.sqrt(NCHAN * StddevMasked ** 2 / (NCHAN - Nmask) - \
                        NCHAN * Nmask * MeanMasked ** 2 / ((NCHAN - Nmask) ** 2))
        MaskedData = SpStorage[0][row] * mask
        StddevMasked = MaskedData.stddev()
        MeanMasked = MaskedData.mean()
        OldRMS = math.sqrt(NCHAN * StddevMasked ** 2 / (NCHAN - Nmask) - \
                        NCHAN * Nmask * MeanMasked ** 2 / ((NCHAN - Nmask) ** 2))
        if STEP == 1:
            Rmask = NA.zeros(NCHAN, NA.Int)
            Rdata0 = NA.zeros(NCHAN, NA.Float64)
            Rdata1 = NA.zeros(NCHAN, NA.Float64)
            NR = 0
            for x in range(1, min(Nmean + 1, NROW)):
                NR += 1
                Rdata0 += SpStorage[0][rows[x]]
                Rdata1 += SpStorage[1][rows[x]]
                mask0 = NA.ones(NCHAN, NA.Int)
                for [m0, m1] in RealSignal[rows[x]][2]: mask0[m0:m1] = 0
                Rmask += mask0
        elif STEP > (NROW - Nmean):
            NR -= 1
            Rdata0 -= SpStorage[0][row]
            Rdata1 -= SpStorage[1][row]
            Rmask -= mask
        else:
            Rdata0 -= (SpStorage[0][row] - SpStorage[0][rows[STEP + Nmean - 1]])
            Rdata1 -= (SpStorage[1][row] - SpStorage[1][rows[STEP + Nmean - 1]])
            mask0 = NA.ones(NCHAN, NA.Int)
            for [m0, m1] in RealSignal[rows[STEP + Nmean - 1]][2]: mask0[m0:m1] = 0
            Rmask += (mask0 - mask)
        if STEP == 1:
            Lmask = NA.zeros(NCHAN, NA.Int)
            Ldata0 = NA.zeros(NCHAN, NA.Float64)
            Ldata1 = NA.zeros(NCHAN, NA.Float64)
            NL = 0
        elif STEP <= (Nmean + 1):
            NL += 1
            Ldata0 += SpStorage[0][rows[STEP - 2]]
            Ldata1 += SpStorage[1][rows[STEP - 2]]
            mask0 = NA.ones(NCHAN, NA.Int)
            for [m0, m1] in RealSignal[rows[STEP - 2]][2]: mask0[m0:m1] = 0
            Lmask += mask0
        else:
            Ldata0 += (SpStorage[0][rows[STEP - 2]] - SpStorage[0][rows[STEP - 2 - Nmean]])
            Ldata1 += (SpStorage[1][rows[STEP - 2]] - SpStorage[1][rows[STEP - 2 - Nmean]])
            mask0 = NA.ones(NCHAN, NA.Int)
            for [m0, m1] in RealSignal[rows[STEP - 2]][2]: mask0[m0:m1] = 0
            Lmask += mask0
            mask0 = NA.ones(NCHAN, NA.Int)
            for [m0, m1] in RealSignal[rows[STEP - 2 - Nmean]][2]: mask0[m0:m1] = 0
            Lmask -= mask0

        diff0 = (Ldata0 + Rdata0) / float(NL + NR) - SpStorage[0][row]
        diff1 = (Ldata1 + Rdata1) / float(NL + NR) - SpStorage[1][row]
        mask0 = (Rmask + Lmask + mask) / (NL + NR + 1)
        Nmask = int(NCHAN - NA.sum(mask0 * 1.0))
            
        # Calculate RMS after/before fitting
        MaskedData = diff1 * mask0
        StddevMasked = MaskedData.stddev()
        MeanMasked = MaskedData.mean()
        NewRMSdiff = math.sqrt(NCHAN * StddevMasked ** 2 / (NCHAN - Nmask) - \
                        NCHAN * Nmask * MeanMasked ** 2 / ((NCHAN - Nmask) ** 2))
        MaskedData = diff0 * mask0
        StddevMasked = MaskedData.stddev()
        MeanMasked = MaskedData.mean()
        OldRMSdiff = math.sqrt(NCHAN * StddevMasked ** 2 / (NCHAN - Nmask) - \
                        NCHAN * Nmask * MeanMasked ** 2 / ((NCHAN - Nmask) ** 2))

        data.append([row, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, flag, flag, flag, flag, Nmask])
        DataTable[row][17] = NewRMS
        DataTable[row][18] = OldRMS
        DataTable[row][19] = NewRMSdiff
        DataTable[row][20] = OldRMSdiff
        DataTable[row][25] = Nmask

    tmpdata = NA.transpose(data)
    Ndata = len(tmpdata[0])
    for cycle in range(Iteration + 1):
        threshold = []
        for x in range(4):
            Unflag = int(NA.sum(tmpdata[x + 5] * 1.0))
            FlaggedData = tmpdata[x + 1] * tmpdata[x + 5]
            StddevFlagged = FlaggedData.stddev()
            MeanFlagged = FlaggedData.mean()
            AVE = MeanFlagged / float(Unflag) * float(Ndata)
            RMS = math.sqrt(Ndata * StddevFlagged ** 2 / Unflag - \
                            Ndata * (Ndata - Unflag) * MeanFlagged ** 2 / (Unflag ** 2))
            Thre = AVE + RMS * Threshold[x]
            threshold.append(Thre)
            for y in range(len(tmpdata[0])):
                if tmpdata[x + 1][y] > Thre: tmpdata[x + 5][y] = 0
                else: tmpdata[x + 5][y] = 1
            
    tmpdata2 = NA.sum(tmpdata[5:9])
    SDT.LogMessage('INFO', LogLevel, LogFile, \
            Msg='NoFlag=%d  1Flag=%d  2Flags=%d  3Flags=%d  AllFlagged=%d' % \
           (int(NA.sum(NA.equal(tmpdata2, 4)*1.0)), \
            int(NA.sum(NA.equal(tmpdata2, 3)*1.0)), \
            int(NA.sum(NA.equal(tmpdata2, 2)*1.0)), \
            int(NA.sum(NA.equal(tmpdata2, 1)*1.0)), \
            int(NA.sum(NA.equal(tmpdata2, 0)*1.0))))

    N = 0
    for row in rows:
        DataTable[row][21] = tmpdata[5][N]
        DataTable[row][22] = tmpdata[6][N]
        DataTable[row][23] = tmpdata[7][N]
        DataTable[row][24] = tmpdata[8][N]
        N += 1

    # Plot result statistics
    if showplot:
        title = 'Statistics Plot (Iteration=%d)\n Baseline RMS for (Raw, Processed) data w Threshold (%.1f, %.1f)\n Deviation from running mean (N=%d) for (Raw, Processed) data w Threshold (%.1f, %.1f) ' % (Iteration, ThreOldRMS, ThreNewRMS, Nmean, ThreOldDiff, ThreNewDiff)
        SDP.ShowResult('final', DataTable, rows, TimeGap, PosGap, [threshold[1],threshold[0],threshold[3],threshold[2]], title, FigFileDir, FigFileRoot)

    EndTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))

    del tmpdata, tmpdata2, threshold

    if interactive:
        # not implemented yet....
        cid = PL.connect('button_press_event', SelectData)
        print '<Left Click> to show information'
        print '<Right Click> to plot spectrum'
        raw_input('Press <Enter> key to terminate')
        PL.disconnect()

    return



def GroupForGrid(DataTable, rows, vIF, vPOL, CombineRadius, Allowance, GridSpacing, ObsPattern, LogLevel=2, LogFile=False):
    '''
    Gridding by RA/DEC position
    '''
    # Re-Gridding

    NROW = len(rows)
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  GroupForGrid  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='ObsPattern = %s' % ObsPattern)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d positions...' % NROW)
    StartTime = time.time()

    GridTable = []
    RAs = []
    DECs = []
    RMSs = []

    for row in rows:
        RAs.append(DataTable[row][9])
        DECs.append(DataTable[row][10])
        RMSs.append(DataTable[row][17])

    if ObsPattern.upper() == 'RASTER':
        ThresholdR = CombineRadius * CombineRadius
        MinRA = min(RAs)
        MinDEC = min(DECs)
        NGridRA = int((max(RAs) - MinRA) / GridSpacing) + 1
        NGridDEC = int((max(DECs) - MinDEC) / GridSpacing) + 1
        for y in range(NGridDEC):
            DEC = MinDEC + GridSpacing * y
            DeltaDEC = NA.array(DECs) - DEC
            for x in range(NGridRA):
                RA = MinRA + GridSpacing * x
                DeltaRA = NA.array(RAs) - RA
                Delta = DeltaDEC * DeltaDEC + DeltaRA * DeltaRA
                Select = NA.less_equal(Delta, ThresholdR)
                line = [vIF, vPOL, x, y, RA, DEC, []]
                for i in range(len(rows)):
                    if Select[i] == 1:
                        line[6].append([rows[i], math.sqrt(Delta[i]), RMSs[i]])
                GridTable.append(line)
                SDT.LogMessage('LONG', LogLevel, LogFile, Msg="GridTable: %s" % line)

    elif ObsPattern.upper() == 'SINGLE-POINT':
        NGridRA = 1
        NGridDEC = 1
        CenterRA = (NA.array(RAs)).mean()
        CenterDEC = (NA.array(DECs)).mean()
        line = [vIF, vPOL, 0, 0, CenterRA, CenterDEC, []]
        for x in range(len(rows)):
            Delta = math.sqrt((RAs[x] - CenterRA) ** 2.0 + (DECs[x] - CenterDEC) ** 2.0)
            if Delta <= Allowance:
                line[6].append([rows[x], Delta, RMSs[x]])
        GridTable.append(line)
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg="GridTable: %s" % line)

    elif ObsPattern.upper() == 'MULTI-POINT':
        NGridRA = 0
        NGridDEC = 1
        Flag = NA.ones(len(rows))
        while Flag.sum() > 0:
            for x in range(len(rows)):
                if Flag[x] == 1:
                    RA = RAs[x]
                    DEC = DECs[x]
                    RAtmp = [RA]
                    DECtmp = [DEC]
                    for y in range(x + 1, len(rows)):
                        if Flag[y] == 1:
                            Delta = math.sqrt((RA - RAs[y]) ** 2.0 + (DEC - DECs[y]) ** 2.0)
                            if Delta <= Allowance:
                                RAtmp.append(RAs[y])
                                DECtmp.append(DECs[y])
                    CenterRA = (NA.array(RAtmp)).mean()
                    CenterDEC = (NA.array(DECtmp)).mean()
                    line = [vIF, vPOL, 0, NGridRA, CenterRA, CenterDEC, []]
                    NGridRA += 1
                    for x in range(len(rows)):
                        if Flag[x] == 1:
                            Delta = math.sqrt((RAs[x] - CenterRA) ** 2.0 + (DECs[x] - CenterDEC) ** 2.0)
                            if Delta <= Allowance:
                                line[6].append([rows[x], Delta, RMSs[x]])
                                Flag[x] = 0
                    GridTable.append(line)
                    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg="GridTable: %s" % line)
        del Flag
    else:
        SDT.LogMessage('ERROR', LogLevel, LogFile, Msg='Error: ObsPattern not defined...')
        return False

    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))
    EndTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    return GridTable



def Process8(SpStorage, DataTable, GridTable, CombineRadius, weight='CONST', clip='none', rms_weight=False, LogLevel=2, LogFile=False):

    # The process does re-map and combine spectrum for each position
    # GridTable format:
    #   [[IF,POL,0,0,RAcent,DECcent,[[row0,r0,RMS0],[row1,r1,RMS1],..,[rowN,rN,RMSN]]]
    #    [IF,POL,0,1,RAcent,DECcent,[[row0,r0,RMS0],[row1,r1,RMS1],..,[rowN,rN,RMSN]]]
    #                 ......
    #    [IF,POL,M,N,RAcent,DECcent,[[row0,r0,RMS0],[row1,r1,RMS1],..,[rowN,rN,RMSN]]]]
    #  where row0,row1,...,rowN should be combined to one for better S/N spectra
    #        'r' is a distance from grid position
    # DataTable format:
    # DataTable[ID] = [Row,Scan,IF,Pol,Beam,Date,MJD,ElapsedTime,
    #                  ExpTime,RA,Dec,Az,El,Nchan,Tsys,TargetName,
    #                  LowFreqPower, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff,
    #                  flag1, flag2, flag3, flag4, Nmask]
    #                 [  0,   1, 2,  3,   4,   5,  6,          7,
    #                        8, 9, 10,11,12,   13,  14,        15,
    #                            16,     17,     18,         19,         20,
    #                     21,    22,    23,    24,    25]
    # 'weight' can be 'CONST', 'GAUSS', or 'LINEAR'
    # 'clip' can be 'none' or 'minmaxreject' 
    # 'rms_weight' is either True or False. If True, NewRMS is used for additional weight
    #   Number of spectra output is len(GridTable)
    # OutputTable format:
    #    [[IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp.]
    #     [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp.]
    #             ......
    #     [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp.]]

    FlagThreshold = 2
    # spectrum with sum of flags (max 4) less than or equal to this value
    #  is not used for final output
    #  Good: flag = 1, NG: flag = 0

    NROW = len(GridTable)
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process8  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Accumulate nearby spectrum for each Grid position...')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d spectrum...' % (NROW))
    SDT.LogMessage('TIMER', LogLevel, LogFile, Msg='|' + '='*36 + ' 100% ' + '=' * 36 + '|')
    StartTime = time.time()
    OutputTable = []
    NCHAN = len(SpStorage[0][0])
    StorageOut = NA.ones((NROW, NCHAN), type=NA.Float32) * NoData
    ID = 0
    STEP = HASH = HASH0 = 0

    for [IF, POL, X, Y, RAcent, DECcent, RowDelta] in GridTable:
        rowlist = []
        deltalist = []
        rmslist = []
        flagged = 0
        for [row, delta, rms] in RowDelta:
            # Check Flag
            if sum(DataTable[row][21:24]) > FlagThreshold:
                rowlist.append(row)
                deltalist.append(delta)
                rmslist.append(rms)
            else: flagged += 1
        if len(rowlist) == 0:
            # No valid Spectra at the position
            pass
        elif len(rowlist) == 1:
            # One valid Spectrum at the position
            StorageOut[ID] = SpStorage[1][row]
        else:
            # More than one valid Spectra at the position
            data = SpStorage[1][rowlist].copy()
            w = NA.ones(NA.shape(data), NA.Float64)
            # Clipping
            if clip.upper() == 'MINMAXREJECT' and n > 2:
                w[NA.argmin(data, axis=0), range(len(data[0]))] = 0.0
                w[NA.argmax(data, axis=0), range(len(data[0]))] = 0.0
            # Weight by RMS
            if rms_weight == True:
                for m in range(len(rowlist)):
                    w[m] /= rmslist[m]
            # Weight by radius
            if weight.upper() == 'GAUSS':
                # weight = exp(-ln2*((r/hwhm)**2))
                for m in range(len(rowlist)):
                    w[m] *= (math.exp(-0.69314718055994529*((deltalist[m]/CombineRadius)**2)))
            elif weight.upper() == 'LINEAR':
                # weight = 0.5 + (hwhm - r)/2/hwhm = 1.0 - r/2/hwhm
                for m in range(len(rowlist)):
                    w[m] *= (1.0 - deltalist[m]/2.0/CombineRadius)
            # Combine Spectra
            StorageOut[ID] = (NA.sum(data * w) / NA.sum(w))
        OutputTable.append([IF, POL, X, Y, RAcent, DECcent, len(rowlist), flagged])
        ID += 1

        if LogLevel > 0:
            STEP = STEP + 1
            if LogLevel > 0:
                HASH = int(80 * STEP / NROW)
                if (HASH0 != HASH):
                    print '\b' + '*' * (HASH - HASH0),
                    sys.stdout.flush()
                    HASH0 = HASH

    EndTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    return (StorageOut, OutputTable)



def SelectData(event):
    # Left Click ro show information
    if event.button == 1:
        print '<LEFT> button pressed', event.xdata, event.ydata, event.inaxes
        print 'Information of the selected point will be shown'
    # Right Click ro plot spectrum
    elif event.button == 3:
        print '<RIGHT> button pressed', event.xdata, event.ydata, event.inaxes
        print 'Spectrum plot code will be implemented here'
    return



def CalcBaselineFit(scan, masklist, polyorder, nchan, modification, edge=(0, 0), LogLevel=2, LogFile=False):

    # Initialize plot for debugging
    if LogLevel > 3: SDP.DrawDebugFit(mode='INIT')

    data = NA.array(scan._getspectrum(0))
    # set edge mask
    (EdgeMaskL, EdgeMaskR) = edge
    NCHAN = scan.nchan()
    NCHANwoEdge = NCHAN - EdgeMaskL - EdgeMaskR
    if EdgeMaskL > 0: data[:EdgeMaskL] = 0.0
    if EdgeMaskR > 0: data[-EdgeMaskR:] = 0.0

    # Create mask for line protection
    mask = NA.ones(NCHAN)
    if type(masklist) == list:
        for [m0, m1] in masklist:
            mask[m0:m1] = 0
    else: return Flase
    if EdgeMaskR > 0: Nmask = int(NCHANwoEdge - NA.sum(mask[EdgeMaskL:-EdgeMaskR] * 1.0))
    else: Nmask = int(NCHANwoEdge - NA.sum(mask[EdgeMaskL:] * 1.0))
    resultdata = []
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='NCHANwoEdge, Nmask, diff = %s, %s, %s' % (NCHANwoEdge, Nmask, NCHANwoEdge-Nmask))

    (fragment, nwindow, win_polyorder) = CalcFragmentation(polyorder, NCHANwoEdge - Nmask, 0)
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='fragment, nwindow, win_polyorder = %s, %s, %s' % (fragment, nwindow, win_polyorder))


    for WIN in range(nwindow):
        EdgeL = int(WIN * NCHANwoEdge / (fragment * 2) + EdgeMaskL)
        EdgeR = NCHAN - EdgeMaskR - int(NCHANwoEdge * (nwindow - 1 - WIN) / (fragment * 2))
        # Check EdgeL and EdgeR is inside mask region or not
        (NewEdgeL, NewEdgeR) = (EdgeL, EdgeR)
        masked = 0
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='masklist = %s' % masklist)
        for [m0, m1] in masklist:
            # All masked
            if m0 <= EdgeL and EdgeR <= m1:
                NewEdgeL = m0 - (EdgeR - EdgeL) / 2
                NewEdgeR = m1 + (EdgeR - EdgeL) / 2
                masked += (EdgeR - EdgeL)
            # mask inside windows
            elif EdgeL < m0 and m1 < EdgeR:
                masked += (m1 - m0)
                if m1 <= (EdgeL + EdgeR) / 2:
                    NewEdgeL = NewEdgeL - (m1 - m0)
                elif (EdgeL + EdgeR) / 2 <= m0:
                    NewEdgeR = NewEdgeR + (m1 - m0)
                else:
                    NewEdgeL = NewEdgeL - ((EdgeL + EdgeR) / 2 - m0)
                    NewEdgeR = NewEdgeR + (m1 - (EdgeL + EdgeR) / 2)
            # Left edge inside mask
            elif m0 <= EdgeL and EdgeL < m1:
                masked += (m1 - EdgeL)
                if m1 <= (EdgeL + EdgeR) / 2:
                    NewEdgeL = NewEdgeL - (m1 - m0)
                else:
                    NewEdgeL = m0 - (EdgeR - EdgeL) / 2
                    NewEdgeR = NewEdgeR + (m1 - (EdgeL + EdgeR) / 2)
            # Right edge inside mask
            elif m0 < EdgeR and EdgeR <= m1:
                masked += (EdgeR - m0)
                if (EdgeL + EdgeR) / 2 <= m0:
                    NewEdgeR = NewEdgeR + (m1 - m0)
                else:
                    NewEdgeL = NewEdgeL - ((EdgeL + EdgeR) / 2 - m0)
                    NewEdgeR = m1 + (EdgeR - EdgeL) / 2
            (EdgeL, EdgeR) = (NewEdgeL, NewEdgeR)

        EdgeL = max(NewEdgeL, EdgeMaskL)
        EdgeR = min(NewEdgeR, NCHAN - EdgeMaskR)

        # Calculate positions for combining fragmented spectrum
        WIN_EDGE_IGNORE_L = int(NCHANwoEdge / (fragment * win_polyorder))
        WIN_EDGE_IGNORE_R = WIN_EDGE_IGNORE_L
        PosL0 = int(WIN * NCHANwoEdge / (fragment * 2)) + NCHANwoEdge / fragment / win_polyorder + EdgeMaskL
        PosL1 = int((WIN + 1) * NCHANwoEdge / (fragment * 2)) -1 - NCHANwoEdge / fragment / win_polyorder + EdgeMaskL
        DeltaL = float(PosL1 - PosL0)
        PosR0 = int((WIN + 1) * NCHANwoEdge / (fragment * 2)) + NCHANwoEdge / fragment / win_polyorder + EdgeMaskL
        PosR1 = int((WIN + 2) * NCHANwoEdge / (fragment * 2)) -1 - NCHANwoEdge / fragment / win_polyorder + EdgeMaskL
        DeltaR = float(PosR1 - PosR0)
        if WIN == 0:
            WIN_EDGE_IGNORE_L = 0
            PosL0 = EdgeMaskL
            PosL1 = EdgeMaskL
        if WIN == (nwindow - 1):
            WIN_EDGE_IGNORE_R = 0
            PosR0 = NCHAN - EdgeMaskR
            PosR1 = NCHAN - EdgeMaskR
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='PosL0, PosL1, PosR0, PosR1 = %s, %s, %s, %s' % (PosL0, PosL1, PosR0, PosR1))
        dorder = int(max(1, (mask[PosL0:PosR1].sum() * win_polyorder / (PosR1 - PosL0) + 0.99)))
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Revised edgemask = %s:%s  Adjust polyorder = %s' % (EdgeL, EdgeR, dorder))

        # Shift scan and mask to set unmasked region starting at zero
        # Shift size is EdgeL: EdgeL -> 0
        if POLYNOMIAL_FIT == 'ASAP':
            print "start ASAP fitting..."
            edgemask = scan.create_mask([0, EdgeR - EdgeL])
            tmpscan = scan.copy()
            tmpscan._setspectrum(NA.concatenate((data[EdgeL:], NA.zeros(EdgeL))))
            tmpmask = NA.concatenate((mask[EdgeL:], NA.zeros(EdgeL)))
            # use linear fit 
            tmpfit = NA.array(tmpscan.poly_baseline(order=dorder, mask=(tmpmask & edgemask),uselin=True, insitu=False)._getspectrum(0))[:EdgeR - EdgeL]
            # use non-linear fit (did not work -didn't converge)
            #tmpfit = NA.array(tmpscan.poly_baseline(order=dorder, mask=(tmpmask & edgemask),uselin=False, insitu=False)._getspectrum(0))[:EdgeR - EdgeL]
            # tmpfit = NA.array(tmpscan.poly_baseline(order=dorder, mask=(tmpmask & edgemask), insitu=False)._getspectrum(0))[:EdgeR - EdgeL]
            # tmpfit = NA.array(tmpscan.poly_baseline(order=dorder, mask=(tmpmask & edgemask))._getspectrum(0))[:EdgeR - EdgeL]
            # tmpfit = NA.array(tmpscan.poly_baseline(order=dorder, mask=(tmpmask & edgemask))._getspectrum(0))
            # resultdata.append(list(NA.concatenate((NA.zeros(EdgeL), tmpfit[:(NCHAN - EdgeL)]))))
            del tmpscan, tmpmask, edgemask
        else:
            if POLYNOMIAL_FIT == 'CLIP':
                tmpfit = PolynomialFitClip(data[EdgeL:EdgeR], mask[EdgeL:EdgeR], order=dorder, iter=CLIP_CYCLE, hsigma=5.0, lsigma=5.0, subtract=True, LogLevel=LogLevel, LogFile=LogFile)
            else:
                tmpfit = PolynomialFit(data[EdgeL:EdgeR], mask[EdgeL:EdgeR], order=dorder, subtract=True)

        # Restore scan to the original position
        # 0 -> EdgeL
        resultdata.append(list(NA.concatenate((NA.zeros(EdgeL), tmpfit, NA.zeros(NCHAN - EdgeR)))))
        del tmpfit

        # window function: f(x) = -2x^3 + 2x^2 (0 <= x <= 1)
        for i in range(NCHAN):
            if i < PosL0:
                resultdata[WIN][i] = 0.0
            elif i <= PosL1:
                x = (i - PosL0) / DeltaL
                resultdata[WIN][i] *= (-2.0 * x ** 3.0 + 3.0 * x ** 2.0)
            elif i > PosR1:
                resultdata[WIN][i] = 0.0
            elif i >= PosR0:
                x = (i - PosR0) / DeltaR
                resultdata[WIN][i] *= (2.0 * x ** 3.0 - 3.0 * x ** 2.0 + 1.0)

        # Plot for verification use only : START
        if LogLevel > 3:
            plotdata =  data - resultdata[WIN]
            for i in range(NCHAN):
                if i < PosL0:
                    plotdata[i] = 0.0
                elif i <= PosL1:
                    x = (i - PosL0) / DeltaL
                    if x == 0 or x == 1:
                        plotdata[i] = 0.0
                    else:
                        plotdata[i] = data[i] - resultdata[WIN][i] / (-2.0 * x ** 3.0 + 3.0 * x ** 2.0)
                elif i > PosR1:
                    plotdata[i] = 0.0
                elif i >= PosR0:
                    x = (i - PosR0) / DeltaR
                    if x == 0 or x == 1:
                        plotdata[i] = 0.0
                    else:
                        plotdata[i] = data[i] - resultdata[WIN][i] / (2.0 * x ** 3.0 - 3.0 * x ** 2.0 + 1.0)
            SDP.DrawDebugFit('FIRST', range(NCHAN), plotdata)
        # Plot for verification : END
    outdata = NA.sum(resultdata)

    # Calculate RMS after/before fitting
    MaskedData = outdata * mask
    StddevMasked = MaskedData.stddev()
    MeanMasked = MaskedData.mean()
    NewRMS = math.sqrt(NCHANwoEdge * StddevMasked ** 2 / (NCHANwoEdge - Nmask) - \
                    NCHANwoEdge * Nmask * MeanMasked ** 2 / ((NCHANwoEdge - Nmask) ** 2))
    MaskedData = data * mask
    StddevMasked = MaskedData.stddev()
    MeanMasked = MaskedData.mean()
    OldRMS = math.sqrt(NCHANwoEdge * StddevMasked ** 2 / (NCHANwoEdge - Nmask) - \
                    NCHANwoEdge * Nmask * MeanMasked ** 2 / ((NCHANwoEdge - Nmask) ** 2))

    # Plot for verification use only : START
    if LogLevel > 3:
        plotdata = data - NA.sum(resultdata)
        Ymin = min(plotdata) - (max(plotdata) - min(plotdata)) * 0.15
        Ymax = max(plotdata) + (max(plotdata) - min(plotdata)) * 0.15
        SDP.DrawDebugFit('SECOND', range(NCHAN), plotdata, Ymin, Ymax)
    # Plot for verification : END

    return (outdata, NewRMS, OldRMS, Nmask, win_polyorder, fragment, nwindow)



def CalcFragmentation(polyorder, nchan, modification=0):

    polyorder += modification

    # Number of fitting window is determined according to the polynomial order
    fragment = int(min(polyorder / MAX_FREQ + 1, max(nchan / MIN_CHANNELS, 1)))
    if fragment > MAX_WINDOW_FRAGMENTATION: fragment = MAX_WINDOW_FRAGMENTATION
#    fragment = 1
#    for n in range(int(MAX_WINDOW_FRAGMENTATION * nchan / 2048)):
#        if polyorder <= (MAX_FREQ - n) * (n + 1):
#            fragment = n + 1
#            break
#        else: fragment = int(MAX_WINDOW_FRAGMENTATION * nchan / 2048)
    nwindow = fragment * 2 - 1
    win_polyorder = min(int(polyorder / fragment) + fragment, MAX_FREQ)

    return (fragment, nwindow, win_polyorder)



def PolynomialFitClip(data, mask, order=2, iter=3, hsigma=3.0, lsigma=3.0, subtract=True, LogLevel=2, LogFile=False):
    """
    mask format: array([1D flag list])
        flag is either True (Valid) or False (Invalid).
         len(data) should equal to len(flag)
    order: order of polynomial (int)
    iter: number of iteration (int)
    hsigma: factor for clipping. Residual greater than
        (hsigma * sigma) will be clipped out and a flag
        will be set to True.
    lsigma: factor for clipping. Residual lesser than
        (-lsigma * sigma) will be clipped out and a flag
        will be set to True.
    subtract: output data is residual (True)
              output data is fitting (False)

    return:
    1D array or False
    """
    flag = mask
    newflag = mask.copy()
    abscissa = NA.array([x for x in xrange(len(data))], NA.Float64)
    # Store the number of initially rejected channels
    reject_org = len(flag) - sum(flag)
    reject_old = -1
    sigma_old = -1.0
    for niter in xrange(iter):
    # inside iteration cycle for channel rejection
        if (sum(newflag) < order):
            SDT.LogMessage('ERROR', LogLevel, LogFile, Msg="Error: Too few good data points")
            return False
        # Initialize arrays to store summation
        xlist = NA.zeros(order * 2 + 1, NA.Float64)
        ymatrix = NA.zeros(order + 1, NA.Float64)
        yy = 0.0
        # Calculate Summation
        for [x, y, f] in NA.transpose(NA.concatenate([[abscissa], \
                [data], [newflag]])):
            if f:
                xlist += [x**k for k in xrange(order * 2 + 1)]
                ymatrix += [y*x**k for k in xrange(order + 1)]
                yy += y*y
        # Make Summation matrix
        xmatrix = NA.array([xlist[k:k+order+1] for k in xrange(order+1)])
        # Calculate Coefficients and Standard Deviation
        coeff = NA.add.reduce(LA.inverse(xmatrix) * ymatrix, axis=1)

        sigma = math.sqrt((yy - 2.0 * NA.sum(ymatrix * coeff)
            + NA.sum(NA.add.reduce(xmatrix * coeff, axis=1) * coeff))
            / (xmatrix[0][0] - 1.0))
        # Count newly rejected channels
        reject = len(newflag) - sum(newflag)
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg="iteration = %s/%s" % (niter + 1, iter))
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg="sigma = %s" % sigma)
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg="reject(org) = %s/%s (%s)" % (reject - reject_org, len(newflag), reject_org))
        # Calculate Fitting profile
        fitting = NA.array([NA.sum((NA.array([x**k for k in \
                xrange(order + 1)]) * coeff)) for x in abscissa])
        if iter == (niter + 1) or \
                (sigma == sigma_old and reject == reject_old):
            # iteration stops when the number of iteration exceeded
            #  niter, or sigma and channel rejection get stable

            # Subtract the fit from original?
            if(subtract == True): 
                NewData = data - fitting
            else:
                NewData = fitting.copy()

            del fitting, newflag, abscissa, xlist, ymatrix, coeff, xmatrix
            return NewData
        sigma_old = sigma
        reject_old = reject
        for [f,y,pos] in NA.transpose(NA.concatenate([[fitting], \
                [data], [NA.arange(len(data))]])):
            # Re-flag data: Oritinally flagged out channel should
            #  remain to be flagged out
            newflag[int(pos)] = ((f + hsigma * sigma) >= y >= \
                    (f - lsigma * sigma))
        newflag = newflag * flag
    


def PolynomialFit(data, mask, order=2, subtract=True):
    """
    mask format: array([1D flag list])
        flag is either True (Valid) or False (Invalid).
         len(data) should equal to len(flag)
    order: order of polynomial (int)
    subtract: output data is residual (True)
              output data is fitting (False)

    return: 1D array
    """
    abscissa = NA.array([x for x in xrange(len(data))], NA.Float64)

    # Initialize arrays to store summation
    xlist = NA.zeros(order * 2 + 1, NA.Float64)
    ymatrix = NA.zeros(order + 1, NA.Float64)
    yy = 0.0
    # Calculate Summation
    for [x, y, f] in NA.transpose(NA.concatenate([[abscissa], [data], [mask]])):
        if f:
            xlist += [x**k for k in xrange(order * 2 + 1)]
            ymatrix += [y*x**k for k in xrange(order + 1)]
            yy += y*y
    # Make Summation matrix
    xmatrix = NA.array([xlist[k:k+order+1] for k in xrange(order+1)])
    # Calculate Coefficients and Standard Deviation
    coeff = NA.add.reduce(LA.inverse(xmatrix) * ymatrix, axis=1)

    # Calculate Fitting profile
    fitting = NA.array([NA.sum((NA.array([x**k for k in \
            xrange(order + 1)]) * coeff)) for x in abscissa])

    # Subtract the fit from original?
    if(subtract == True): 
        NewData = data - fitting
    else: NewData = fitting.copy()

    del fitting, xmatrix, ymatrix, xlist, coeff, abscissa
    return NewData



