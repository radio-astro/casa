from asap import *
# comment out all numarry dependencies
#import numarray as NA
import numpy as NP
#import numarray.ma as NMA
#import numarray.mlab as NML
# up to here
#import numarray.fft as FFT
import numpy.fft as FFTP
#import numarray.linear_algebra as LA
import numpy.linalg as LA
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
MaxFWHM = 900
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


def  Process1(SpStorage, DataTable, rows, FFTORDER=INITFFTORDER, edge=(0, 0), LogLevel=2, LogFile=False):
    
    NROW = len(rows)

    StartTime = time.time()

    # FFT > set lower order to 0 > InverseFFT
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process1  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Start: %s' % time.ctime(StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='FFT, Reject lower freqency feature, InvFFT...')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d spectra...' % NROW)
    SDT.LogMessage('TIMER', LogLevel, LogFile, Msg='|' + '='*36 + ' 100% ' + '=' * 36 + '|')
    StatLowFreqPower = []
    STEP = HASH = HASH0 = 0
    # Set edge mask region
    if len(edge) == 2:
        (EdgeL, EdgeR) = edge
    else:
        EdgeR = edge[0]
        EdgeL = edge[0]
    Nedge = EdgeR + EdgeL
    
    # 2009/2/11 for different channel numbers for spectrum
    NCHAN = DataTable[rows[0]][13]
    #NCHAN = DataTable[0][13]
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
        #SpFFT = FFT.fft(SpStorage[0][rows[x]])
        SpFFT = FFTP.fft(SpStorage[0][rows[x]])
        # Calculate power (RMS) indicator for lower frequencies (exclude bias level)
        ##DataTable[rows[x]][16][0] = math.sqrt(2.0) * NA.sum(abs(SpFFT[1:HighLowFreqPartition])) / float((NCHAN - Nedge))
        DataTable[rows[x]][16][0] = math.sqrt(2.0) * NP.sum(abs(SpFFT[1:HighLowFreqPartition])) / float((NCHAN - Nedge))
        # Zero lower order coefficients
        SpFFT[0:FFTORDER] = 0.0
        # Restore to original domain
        #SpStorage[1][rows[x]] = FFT.inverse_fft(SpFFT).real
        SpStorage[1][rows[x]] = FFTP.ifft(SpFFT).real
        # 2009/2/10 Subtract from the original spectrum (for test)
        #SpStorage[0][rows[x]] = FFTP.ifft(SpFFT).real

    EndTime = time.time()
    #SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='End: %s, Elapsed time = %.1f sec' % (time.ctime(EndTime), (EndTime - StartTime)))
    return


def Process2(SpStorage, DataTable, PosDict, rows, SpWinRange=[], LogLevel=2, LogFile=False):

    # The process does map and combine spectrum for each position
    # Format of PosDict:
    #     PosDict[row0] = [row0,row1,row2,...,rowN]
    #     PosDict[row1] = [row1,...,rowM]
    #  where row0,row1,...,rowN should be combined to one for better S/N spectra
    #       or
    #     PosDict[rowM] = [-1, ref]
    #  in case that the position of "rowM" is the same to that of "ref"
    # 'rows' represent spectra which required to be processed

    # 2009/2/7 check Pre-Defined windows
    # Pre-Defined Spectrum Window
    if len(SpWinRange) != 0:
        SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Skip Process2  ' + '>'*15)
        return

    NROW = len(rows)

    StartTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process2  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Start: %s' % time.ctime(StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Accumulate nearby spectrum for each position...')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d spectra...' % NROW)
    SDT.LogMessage('TIMER', LogLevel, LogFile, Msg='|' + '='*36 + ' 100% ' + '=' * 36 + '|')
    STEP = HASH = HASH0 = 0
    Processed = []
    Pending = []
    # 2009/2/6 delete
    #teststore = NP.zeros(len(rows))

    #for row in rows:
    for index in range(len(rows)):
        row = rows[index]
        ttt = DataTable[row][0]
        #if PosDict[row][0] == -1:
        if PosDict[ttt][0] == -1:
            # Check the existence of the reference
            # Format: "ROW:-1,reference"
            #if not PosDict[row][1] in rows:
            if not PosDict[ttt][1] in rows:
                # Reference to non-existing row
                pass
            #elif not PosDict[row][1] in Processed:
            elif not PosDict[ttt][1] in Processed:
                # Reference Not ready
                Pending.append(row)
            else:
                # Copy referece spectrum
                #SpStorage[2][row] = SpStorage[2][PosDict[row][1]].copy()
                SpStorage[2][row] = SpStorage[2][PosDict[ttt][1]].copy()
                Processed.append(row)
                STEP = STEP + 1
        else:
            # Combine listed spectra
            rowlist = []
            #for y in PosDict[row]:
            for y in PosDict[ttt]:
                # Check whether the row in the list is in rows
                #  and Tsys value is valid
                if (y in rows) and (DataTable[y][14] > 0.1): rowlist.append(y)
            SDT.LogMessage('LONG', LogLevel, LogFile, Msg="Combine spectrum %s" % rowlist)
            if len(rowlist) == 0:
                SDT.LogMessage('WARNING', LogLevel, LogFile, Msg="No Valid Data at the Position")
                STEP = STEP + 1
            elif len(rowlist) == 1:
                SpStorage[2][row] = SpStorage[1][row].copy()
            else:
                # Flag out the spectrum with larger RMS (Low frequency)
                StatLowFreqPower = []
                myspstore = []
                for x in rowlist:
                    StatLowFreqPower.append(DataTable[x][16][0])
                ##LowFreqPower = NA.array(StatLowFreqPower)
                LowFreqPower = NP.array(StatLowFreqPower)
                ##Sigma = LowFreqPower.stddev()
                Sigma = LowFreqPower.std(dtype=NP.float64)
                Mean = LowFreqPower.mean()
                MaxThreshold = Mean + 3.0 * Sigma
                for x in range(len(rowlist) - 1, -1, -1):
                    if DataTable[rowlist[x]][16][0] > MaxThreshold:
                        DataTable[rowlist[x]][17][0] = 0
                        del rowlist[x]
                Weight = []
                for x in rowlist:
                    # Weight = sqrt(Exptime/Tsys)
                    Weight.append([(DataTable[x][8]/DataTable[x][14])**0.5])
                ##Factor = NA.array(Weight)
                Factor = NP.array(Weight)
#                SpStorage[2][row] = NML.median(SpStorage[1][rowlist])
                #SpStorage[2][row] = NA.average(SpStorage[1][rowlist])
                ##SpStorage[2][row] = NA.sum(SpStorage[1][rowlist]*Factor)/Factor.sum()
                SpStorage[2][row] = NP.sum(SpStorage[1][rowlist]*Factor,axis=0)/Factor.sum()
                myspstore.append(SpStorage[1][rowlist]*Factor)
                #teststore[row] = NP.sum(SpStorage[1][rowlist]*Factor,dtype=NP.float64)
                # 2009/2/6 deleted teststore
                #print '!!!! row,index,ttt,len(teststore)=', row, index, ttt, len(teststore)
                #teststore[ttt] = NP.sum(SpStorage[1][rowlist]*Factor,dtype=NP.float64)
                Processed.append(row)
                STEP = STEP + 1
                SDT.LogMessage('INFO', LogLevel, LogFile, Msg="Combine %d spectra at (RA,Dec)=(%.2f deg., %.2f deg.)" % (len(rowlist), DataTable[row][9],DataTable[row][10]))

        # Debug (TT)
        #if row==0:
        #    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Weight=%s, Factor=%s, rowlist=%s' % (Weight, Factor, rowlist))
        #    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='SpStorage[1][rowlist]=%s' % SpStorage[1][rowlist])
        #    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='SpStorage[1][rowlist]*Factor=%s' % myspstore[row])
        #    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='sum(SpStorage[1][rowlist]*Factor)=%s' % teststore[row])
        #    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='row=0 SpStorage[2]=%s' % SpStorage[2][row])
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
            #if not PosDict[row][1] in Processed:
            if not PosDict[ttt][1] in Processed:
                # Reference Not ready
                SDT.LogMessage('WARNING', LogLevel, LogFile, Msg="Wrong reference detected!!!")
            else:
                # Copy referece spectrum
                #SpStorage[2][row] = SpStorage[2][PosDict[row][1]].copy()
                SpStorage[2][row] = SpStorage[2][PosDict[ttt][1]].copy()
                Processed.append(row)
            STEP = STEP + 1
            if LogLevel > 0:
                HASH = int(80 * STEP / NROW)
                if (HASH0 != HASH):
                    print '\b' + '*' * (HASH - HASH0),
                    sys.stdout.flush()
                    HASH0 = HASH

    EndTime = time.time()
    #SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='End: %s, Elapsed time = %.1f sec' % (time.ctime(EndTime), (EndTime - StartTime)))
    return


def Process3(SpStorage2, dummyscan, DataTable, ResultTable, PosDict, rows, SpWinRange=[], Threshold=LineFindThreshold, edge=(0, 0), BroadComponent=True, LogLevel=2, LogFile=False):

    # The process finds emission lines and determines protection regions for baselinefit
    # 'dummyscan' is a dummy scantable in order to use ASAP linefinder class

    NROW = len(rows)
    NCHAN = len(SpStorage2[rows[0]])
    DetectSignal = {}

    # Pre-Defined Spectrum Window
    if len(SpWinRange) != 0:
        SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Skip Process3  ' + '>'*15)
        for row in rows:
            DetectSignal[row] = [DataTable[row][9], DataTable[row][10], SpWinRange]
            DataTable[row][21] = SpWinRange
        return DetectSignal

    StartTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process3  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Start: %s' % time.ctime(StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Search regions for protection against the background subtraction...')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d spectra...' % NROW)
    #SDT.LogMessage('TIMER', LogLevel, LogFile, Msg='|' + '='*36 + ' 100% ' + '=' * 36 + '|')
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

    scan = dummyscan.copy()
    LF = linefinder()
    BoxSize = [min(2.0*MaxFWHM/(NCHAN - Nedge), 0.5, (NCHAN - Nedge)/float(NCHAN)*0.9), min(max(MaxFWHM/(NCHAN - Nedge)/4.0, 0.1), (NCHAN - Nedge)/float(NCHAN)/2.0)]
    Thre = [Threshold, Threshold * math.sqrt(2)]
    AvgLimit = [MinFWHM * 16, MinFWHM * 4]
    Processed = []
    Pending = []

    if BroadComponent: Start = 0
    else: Start = 1

    for row in rows:
        #Debug (TT)
        #SDT.LogMessage('INFO', LogLevel, LogFile, Msg='>>>>>>>>New Row %d:Nrow=%d' % (row, len(rows)))
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
            SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Row %d: Skipped - Reference spectrum' % row)
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
            SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Row %d: No spectrum' % row)
            # No spectrum
            ProtectRegion[2].append([-1, -1])
            DetectSignal[row] = ProtectRegion
            Processed.append(row)
        else:
            ProcStartTime = time.time()
            SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Start Row %d: %s' % (row, time.ctime(ProcStartTime)))
            ##import numarray as NA
            ##scan._setspectrum(NA.array(SpStorage2[row],type=NA.Float32))
            scan._setspectrum(NP.array(SpStorage2[row],dtype=NP.float64))
            #Debug (TT)
            #if row == 0:
            #    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='SpStorage[0]=%s ' % SpStorage2[row])
            #    scan.save('spstorage2.asap',overwrite=True)

            # Try to detect broader component and narrow component separately
            for y in range(Start, 2):
                SDT.LogMessage('INFO', LogLevel, LogFile, Msg='line detection parameters: ')
                SDT.LogMessage('INFO', LogLevel, LogFile, Msg='threshold (S/N per channel)=%.1f,' % Thre[y] \
                               + 'min_nchan for detection=%d, running mean box size (in fraction of spectrum)=%s, ' % (MinFWHM, BoxSize[y]) \
                               + 'upper limit for averaging=%s channels, edges to be dropped=[%s, %s]' % (AvgLimit[y], EdgeL, EdgeR) )
                LF.set_scan(scan)
                LF.set_options(threshold=Thre[y], box_size=BoxSize[y], min_nchan=MinFWHM, avg_limit=AvgLimit[y])
                nlines = LF.find_lines(edge=(EdgeL, EdgeR))
                ### Debug TT
                ###SDT.LogMessage('INFO', LogLevel, LogFile, Msg='NLINES=%s, EdgeL=%s, EdgeR=%s' % (nlines, EdgeL, EdgeR))
                # No-line is detected
                if (nlines == 0):
                    ProtectRegion[2].append([-1, -1])
                # Single line is detected
                elif (nlines == 1):
                    Ranges = LF.get_ranges(False)
                    Width = Ranges[1] - Ranges[0]
                    allowance = int(Width/5)
                    #allowance = int(Width/10)
                    #Debug (TT)
                    #SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Ranges=%s, Width=%s, allowance=%s' % (Ranges, Width, allowance))
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
                        allowance = int(Width/5)
                        #allowance = int(Width/10)
                        if Width >= MinFWHM and Width <= MaxFWHM and \
                           Ranges[y*2] > (EdgeL + allowance + 1) and \
                           Ranges[y*2+1] < (NCHAN - 2 - allowance - EdgeR):
                            linestat.append((Ranges[y*2] - allowance, Ranges[y*2+1] + allowance, SpStorage2[row][Ranges[y*2]:Ranges[y*2+1]].max() - SpStorage2[row][Ranges[y*2]:Ranges[y*2+1]].min()))
                    # No candidate lines are left
                    if len(linestat) == 0:
                        ProtectRegion[2].append([-1, -1])
                    # More than or equal to one line are left
                    else:
                        #Threshold = NA.transpose(NA.array(linestat))[2].max() / 30.0
                        for y in range(len(linestat)):
                            # Store line if max intensity exceeds 1/30 of the strongest one
                            #if linestat[y][2] > Threshold:
                            #    ProtectRegion[2].append([linestat[y][0], linestat[y][1]])
                            ProtectRegion[2].append([linestat[y][0], linestat[y][1]])
                        # 2007/09/01 Merge lines into one if two lines are close
                        flag = True
                        for y in range(len(linestat) - 1):
                            if (linestat[y+1][0] - linestat[y][1]) < (min((linestat[y][1]-linestat[y][0]),(linestat[y+1][1]-linestat[y+1][0]))/4.0):
                                if flag == True:
                                    if linestat[y][1] < linestat[y+1][1] and linestat[y][0] < linestat[y+1][0] and (linestat[y+1][1] - linestat[y][0]) < MaxFWHM:
                                        ProtectRegion[2].append([linestat[y][0], linestat[y+1][1]])
                                        Line0 = linestat[y][0]
                                    else: continue
                                else:
                                    if (linestat[y+1][1] - Line0) < MaxFWHM:
                                        ProtectRegion[2].pop()
                                        ProtectRegion[2].append([Line0, linestat[y+1][1]])
                                    else:
                                        flag = True
                                        continue
                                flag = False
                            else: flag = True
            DetectSignal[row] = ProtectRegion
            Processed.append(row)
            ProcEndTime = time.time()
            SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Channel ranges of detected lines for Row %s: %s' % (row, DetectSignal[row][2]))

            SDT.LogMessage('INFO', LogLevel, LogFile, Msg='End Row %d: %s (Elapsed Time=%.1f sec)' % (row, time.ctime(ProcEndTime), ProcEndTime - ProcStartTime) )

    if len(Pending) != 0 and len(ResultTable) == 0:
        ProcStartTime = time.time()
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg="Process pending list %s: %s" % (Pending, time.ctime(ProcStartTime)))
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

        ProcEndTime = time.time()
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg="Pending list process End: %s (Elapsed time = %.1f sec)" % (time.ctime(ProcEndTime), (ProcEndTime - ProcStartTime)) )
        
    EndTime = time.time()
    #SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    SDT.LogMessage('TIMER', LogLevel, LogFile, Msg='|' + '='*36 + ' 100% ' + '=' * 36 + '|')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='End: %s, Elapsed time = %.1f sec' % (time.ctime(EndTime), (EndTime - StartTime)))
    return DetectSignal




def Process4(DetectSignal, DataTable, ResultTable, GridTable, PosDict, Abcissa, rows, SpWin, Pattern, GridSpaceRA, GridSpaceDEC, ITER, Nsigma=4.0, Xorder=-1, Yorder=-1, LogLevel=2, LogFile=False, ShowPlot=True, FigFileDir=False, FigFileRoot=False):

    # 2D fit line characteristics calculated in Process3
    # Sigma clipping iterations will be applied if Nsigma is positive
    # order < 0 : automatic determination of fitting order (max = 5)
    # SpWin: Pre-Defined Spectrum Window
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
    #
    # Lines: output parameter
    #    [LineCenter, LineWidth, Validity]  OK: Validity = True; NG: Validity = False

    import numpy
    import scipy.cluster.vq as VQ

    ##### scipy.stsci.convolve should be used. however it is not installed as a default.
    # trying to deal with this a bit more properly 
    try:
        import scipy.stsci.convolve as SPC
        #import scipy.fftpack.convolve as SPC
        #import scipy.signal.signaltools as SPC
    except ImportError:
        try:
            import numarray.convolve as SPC
        except ImportError:
            msg= "no scipy.stsci.convolve or numarray.convolve!"
            raise Exception, msg 
   
    #import numarray.convolve as SPC
    #import scipy.stsci.convolve as SPC

    import SDpipelineControl as SDC
    reload(SDC)

    Valid = SDC.SDParam['Cluster']['ThresholdValid']
    Marginal = SDC.SDParam['Cluster']['ThresholdMarginal']
    Questionable = SDC.SDParam['Cluster']['ThresholdQuestionable']

    # for Pre-Defined Spectrum Window
    if len(SpWin) != 0:
        SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Skip Process4  ' + '>'*15)
        return SpWin

    StartTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process4  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Start: %s' % time.ctime(StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='2D fit the line characteristics...')
    TotalLines = 0
    RMS0 = 0.0
    Xorder0 = Xorder
    Yorder0 = Yorder
    # Dictionary for final output
    RealSignal = {}
    nChan = DataTable[0][13]
    Lines = []

    # First cycle
    if len(ResultTable) == 0:
        ROWS = rows
        if Pattern.upper() == 'SINGLE-POINT':
            for row in rows:
                SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='DataTable = %s, DetectSignal = %s, OldFlag = %s' % (DataTable[row][21], DetectSignal[row][2], DataTable[row][22]))
                DataTable[row][21] = DetectSignal[row][2]
                DataTable[row][22] = False
            return Lines
        elif Pattern.upper() == 'MULTI-POINT':
            for x in range(len(GridTable)):
                for ID in DetectSignal.keys():
                    # Check position RA and DEC
                    if DetectSignal[ID][0] == GridTable[x][4] and \
                       DetectSignal[ID][1] == GridTable[x][5]:
                        for rowlist in GridTable[x][6]:
                            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='DataTable = %s, DetectSignal = %s, OldFlag = %s' % (DataTable[rowlist[0]][21], DetectSignal[rowlist[0]][2], DataTable[rowlist[0]][22]))
                            DataTable[rowlist[0]][21] = DetectSignal[rowlist[0]][2]
                            DataTable[rowlist[0]][22] = False
                        break
            return Lines

    # Iteration case
    else:
        ROWS = range(len(ResultTable))
        if Pattern.upper() == 'SINGLE-POINT':
            for row in rows:
                RealSignal[row] = DetectSignal[0]
                SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='DataTable = %s, DetectSignal = %s, OldFlag = %s' % (DataTable[row][21], DetectSignal[0][2], DataTable[row][22]))
                if DataTable[row][21] == DetectSignal[0][2]:
                    if type(DataTable[row][22]) != int:
                        DataTable[row][22] = ITER - 1
                else:
                    DataTable[row][21] = DetectSignal[0][2]
                    DataTable[row][22] = False
            return Lines
        elif Pattern.upper() == 'MULTI-POINT':
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='DetectSignal = %s' % DetectSignal)
            for x in range(len(GridTable)):
                for ID in DetectSignal.keys():
                    # Check position RA and DEC
                    if DetectSignal[ID][0] == GridTable[x][4] and \
                       DetectSignal[ID][1] == GridTable[x][5]:
                        for rowlist in GridTable[x][6]:
                            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='rowlist = %s' % rowlist)
                            RealSignal[rowlist[0]] = DetectSignal[ID]
                            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='DataTable = %s, DetectSignal = %s, OldFlag = %s' % (DataTable[rowlist[0]][21], DetectSignal[x][2], DataTable[rowlist[0]][22]))
                            if DataTable[rowlist[0]][21] == DetectSignal[x][2]:
                                if type(DataTable[rowlist[0]][22]) != int:
                                    DataTable[rowlist[0]][22] = ITER
                            else:
                                DataTable[rowlist[0]][21] = DetectSignal[x][2]
                                DataTable[rowlist[0]][22] = False
                        break
            return Lines

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
            # Check statistics flag added by G.K. 2008/1/17
            # Bug fix 2008/5/29
            if (line[0] != line[1]) and ((len(ResultTable) == 0 and DataTable[row][19] == 1) or len(ResultTable) != 0):
                Region.append([row, line[0], line[1], DetectSignal[row][0], DetectSignal[row][1], flag])
                dummy.append([float(line[1] - line[0]), (line[0] + line[1]) / 2.0])
    ##Region2 = numpy.array(dummy)
    Region2 = NP.array(dummy)
    del dummy
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='MaxLines = %s' % MaxLines)
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Npos = %s' % Npos)

    # 2008/9/20 Dec Effect was corrected
    DecCorrection = 1.0 / math.cos(DataTable[0][10] / 180.0 * 3.141592653)
    GridSpaceRA *= DecCorrection
    # Calculate Parameters for Gridding
    dummy = [[],[]]
    for row in rows:
        dummy[0].append(DataTable[row][9])
        dummy[1].append(DataTable[row][10])
    ##PosList = NA.array(dummy)
    PosList = NP.array(dummy)
    del dummy

    wra = PosList[0].max() - PosList[0].min()
    wdec = PosList[1].max() - PosList[1].min()
    nra = int(wra / GridSpaceRA + 1)
    ndec = int(wdec / GridSpaceDEC + 1)
    x0 = PosList[0].min() - (nra * GridSpaceRA - wra) / 2.0
    y0 = PosList[1].min() - (ndec * GridSpaceDEC - wdec) / 2.0
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Grid = %d x %d\n' % (nra, ndec))

    # K-mean Clustering Analysis with LineWidth and LineCenter
    # Max number of protect regions are SDC.SDParam['Cluster']['MaxCluster'] (Max lines)
    ProcStartTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='K-mean Clustering Analaysis Start: %s' % time.ctime(ProcStartTime))
    MaxCluster = int(min(SDC.SDParam['Cluster']['MaxCluster'], max(MaxLines + 1, (Npos ** 0.5)/2)))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Maximum number of clusters (MaxCluster) = %s' % MaxCluster)
    # Whiten is no more necessary 2007/2/12
    # whitened = VQ.whiten(Region2)
    TmpList = []
    # Determin the optimum number of clusters
    BestScore = -1.0
    for Ncluster in range(1, MaxCluster + 1):
        # Fix the random seed 2008/5/23
        numpy.random.seed((1234,567))
        # Try multiple times to supress random selection effect 2007/09/04
        for Multi in range(10):
            #codebook, diff = VQ.kmeans(whitened, Ncluster)
            #codebook, diff = VQ.kmeans(Region2, Ncluster)
            codebook, diff = VQ.kmeans(Region2, Ncluster, iter=50)
            NclusterNew = 0
            # Do iteration until no merging of clusters to be found
            while(NclusterNew != len(codebook)):
                NclusterNew = len(codebook)
                category, distance = VQ.vq(Region2, codebook)
                for x in range(NclusterNew - 1, -1, -1):
                    # Remove a cluster without any members
                    ##if sum(NA.equal(category, x) * 1.0) == 0:
                    if sum(NP.equal(category, x) * 1.0) == 0:
                        NclusterNew -= 1
                        tmp = list(codebook)
                        del tmp[x]
                        codebook = numpy.array(tmp)

                # Clear Flag
                for i in range(len(Region)): Region[i][5] = 1
                # Nsigma clipping/flagging with cluster distance (set flag to 0)
                Outlier = 0.0
                MaxDistance = []
                for Nc in range(NclusterNew):
                    ValueList = distance[(category == Nc).nonzero()[0]]
                    Stddev = ValueList.std()
                    Threshold = ValueList.mean() + Stddev * Nsigma
                    del ValueList
                    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Cluster Clipping Threshold = %s, Stddev = %s' % (Threshold, Stddev))
                    for i in ((distance * (category == Nc)) > Threshold).nonzero()[0]:
                        Region[i][5] = 0
                        Outlier += 1.0
                    MaxDistance.append(max(distance * ((distance < Threshold) * (category == Nc))))
                    SDT.LogMessage('LONG', LogLevel, LogFile, Msg='Region = %s' % Region)
                MemberRate = (len(Region) - Outlier)/float(len(Region))
                SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='MemberRate = %f' % MemberRate)

                # Calculate Cluster Characteristics
                Lines = []
                for NN in range(NclusterNew):
                    LineCenterList = []
                    LineWidthList = []
                    for x in range(len(category)):
                        if category[x] == NN and Region[x][5] != 0:
                            LineCenterList.append(Region2[x][1])
                            LineWidthList.append(Region2[x][0])
                    #Lines.append([NA.array(LineCenterList).mean(), NA.array(LineWidthList).mean(), True])
                    ##Lines.append([NML.median(NA.array(LineCenterList)), NML.median(NA.array(LineWidthList)), True, MaxDistance[NN]])
                    Lines.append([NP.median(NP.array(LineCenterList)), NP.median(NP.array(LineWidthList)), True, MaxDistance[NN]])
                SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Lines = %s' % Lines)

                # Rating
                # 2007/09/10 More sensitive to the number of lines clipped out
                Score = (distance * numpy.transpose(numpy.array(Region))[5]).mean() * (NclusterNew+ 1.0/NclusterNew) * (((1.0 - MemberRate)**0.5 + 1.0)**2.0)
                #Score = (distance * numpy.transpose(numpy.array(Region))[5]).mean() * (NclusterNew+ 1.0/NclusterNew) / MemberRate**2.0
                SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='NclusterNew = %d, Score = %f' % (NclusterNew, Score))
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

                # Merge Clusters if Diff(LineCenter)+ Diff(LineWidth) < Threshold,
                #  where Threshold = max(LineWidth(Narrow) or LineWidth(Wide)/2)
                for x in range(NclusterNew):
                    for y in range(x + 1, NclusterNew):
                        Threshold = max(min(Lines[x][1], Lines[y][1]), max(Lines[x][1], Lines[y][1]) / 2.0)
                        SDT.LogMessage('LONG', LogLevel, LogFile, Msg='Cluster Merging Threshold = %f' % Threshold)
                        if (abs(Lines[x][0] - Lines[y][0]) + abs(Lines[x][1] - Lines[y][1])) < Threshold:
                            LineCenterList = []
                            LineWidthList = []
                            SDT.LogMessage('LONG', LogLevel, LogFile, Msg='OldCodebook = %s' % codebook)
                            for z in range(len(category)):
                                if (category[z] == x or category[z] == y) and Region[z][5] != 0:
                                    LineCenterList.append(Region2[z][1])
                                    LineWidthList.append(Region2[z][0])
                            #codebook[x][0] = NA.array(LineWidthList).mean()
                            #codebook[x][1] = NA.array(LineCenterList).mean()
                            ##codebook[x][0] = NML.median(NA.array(LineWidthList))
                            ##codebook[x][1] = NML.median(NA.array(LineCenterList))
                            codebook[x][1] = NP.median(NP.array(LineCenterList))
                            codebook[x][1] = NP.median(NP.array(LineCenterList))
                            tmp = list(codebook)
                            del tmp[y]
                            codebook = numpy.array(tmp)
                            # print 'New codebook:', codebook
                            SDT.LogMessage('LONG', LogLevel, LogFile, Msg='NewCodebook = %s' % codebook)
                            x = NclusterNew
                            break
                    if x == NclusterNew: break
            TmpList.append([NclusterNew, Score, codebook])

    Ncluster = BestNcluster
    Region = BestRegion
    category = BestCategory[:]
    Lines = []
    for x in range(Ncluster): Lines.append([BestCodebook[x][1], BestCodebook[x][0], True])
    #SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Final: Ncluster = %s, Score = %s, Category = %s, CodeBook = %s, Lines = %s' % (Ncluster, BestScore, category, BestCodebook, Lines))
    SDP.ShowClusterInChannelSpace(Region2, BestLines, ShowPlot, FigFileDir, FigFileRoot)
    ProcEndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Final: Ncluster = %s, Score = %s, Lines = %s' % (Ncluster, BestScore, Lines))
    #SDT.LogMessage('INFO', LogLevel, LogFile, Msg='BestLines = %s' % (BestLines))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='K-mean Cluster Analaysis End: %s (Elapsed time = %.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))

    # Sort Lines and Category by LineCenter: Lines[][0]
    LineCenter = []
    for Nc in range(Ncluster): LineCenter.append(BestLines[Nc][0])
    ##LineIndex = NA.argsort(NA.array(LineCenter))
    LineIndex = NP.argsort(NP.array(LineCenter))
    for Nc in range(Ncluster): Lines[Nc] = BestLines[LineIndex[Nc]]
    ##LineIndex2 = NA.argsort(LineIndex)
    LineIndex2 = NP.argsort(LineIndex)
    for i in range(len(BestCategory)): category[i] = LineIndex2[BestCategory[i]]

    # Create Grid Parameter Space (Ncluster * nra * ndec)
    ##GridCluster = NA.zeros((Ncluster, nra, ndec), type=NA.Float32)
    GridCluster = NP.zeros((Ncluster, nra, ndec), dtype=NP.float32)
    ##GridMember = NA.zeros((nra, ndec))
    GridMember = NP.zeros((nra, ndec))

    ######## Clustering: Detection Stage ########
    ProcStartTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Clustering: Detection Stage Start: %s' % time.ctime(ProcStartTime))
    # Set Cluster on the Plane
    for row in ROWS:
        # Check statistics flag added by G.K. 2008/1/17
        # Bug fix 2008/5/29
        #if DataTable[row][19] == 1:
        if ((len(ResultTable) == 0 and DataTable[row][19] == 1) or len(ResultTable) != 0):
            GridMember[int((DetectSignal[row][0] - x0)/GridSpaceRA)][int((DetectSignal[row][1] - y0)/GridSpaceDEC)] += 1
    for i in range(len(category)):
        if Region[i][5] == 1:
            try:
                GridCluster[category[i]][int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] += 1.0
            except IndexError:
                pass
    SDP.ShowCluster(GridCluster, [1.5, 0.5], Lines, Abcissa, x0, y0, GridSpaceRA, GridSpaceDEC, 'detection', ShowPlot, FigFileDir, FigFileRoot)
    ProcEndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Clustering: Detection Stage End: %s (Elapsed time = %.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))

    ######## Clustering: Validation Stage ########
    ProcStartTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Clustering: Validation Stage Start: %s' % time.ctime(ProcStartTime))
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
                if GridMember[x][y] == 0: GridCluster[Nc][x][y] = 0.0
                # if a single spectrum is inside the grid and has a feature belongs to the cluster, validity is set to 0.5 (for the initial stage) or 1.0 (iteration case).
                elif GridMember[x][y] == 1 and GridCluster[Nc][x][y] > 0.9:
                    if ITER == 0: GridCluster[Nc][x][y] = 0.5
                    else: GridCluster[Nc][x][y] = 1.0
                # if the size of population is enough large, validate it as a special case 2007/09/05
                elif ITER == 0:
                    GridCluster[Nc][x][y] = max(min(GridCluster[Nc][x][y] / GridMember[x][y]**0.5 - 1.0, 3.0), GridCluster[Nc][x][y] / float(GridMember[x][y]))
                else: GridCluster[Nc][x][y] = min(GridCluster[Nc][x][y] / GridMember[x][y]**0.5, 3.0)
                #else: GridCluster[Nc][x][y] = max(min(GridCluster[Nc][x][y] / GridMember[x][y]**0.5 - 1.0, 3.0), GridCluster[Nc][x][y] / float(GridMember[x][y]))
                # normarize validity
                #else: GridCluster[Nc][x][y] /= float(GridMember[x][y])

        if ((GridCluster[Nc] > Questionable)*1).sum() == 0: Lines[Nc][2] = False
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='After:  GridCluster[%s] = %s' % (Nc, GridCluster[Nc]))
    SDP.ShowCluster(GridCluster, [Valid, Marginal, Questionable], Lines, Abcissa, x0, y0, GridSpaceRA, GridSpaceDEC, 'validation', ShowPlot, FigFileDir, FigFileRoot)

    ProcEndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Clustering: Validation Stage End: %s (Elapsed time = %.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))
    ######## Clustering: Smoothing Stage ########
    # Rating:  [0.0, 0.4, 0.5, 0.4, 0.0]
    #          [0.4, 0.7, 1.0, 0.7, 0.4]
    #          [0.5, 1.0, 6.0, 1.0, 0.5]
    #          [0.4, 0.7, 1.0, 0.7, 0.4]
    #          [0.0, 0.4, 0.5, 0.4, 0.0]
    # Rating = 1.0 / (Dx**2 + Dy**2)**(0.5) : if (Dx, Dy) == (0, 0) rating = 6.0

    ProcStartTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Clustering: Smoothing Stage Start: %s' % time.ctime(ProcStartTime))

    for Nc in range(Ncluster):
        if Lines[Nc][2] != False:
            ##GridScore = NA.zeros((2, nra, ndec), type=NA.Float32)
            GridScore = NP.zeros((2, nra, ndec), dtype=NP.float32)
            for x in range(nra):
                for y in range(ndec):
                    for dx in [-2, -1, 0, 1, 2]:
                       for dy in [-2, -1, 0, 1, 2]:
                           if (abs(dx) + abs(dy)) <= 3:
                               (nx, ny) = (x + dx, y + dy)
                               if 0 <= nx < nra and 0 <= ny < ndec:
                                   if dx == 0 and dy == 0: Rating = 6.0
                                   else: Rating = 1.0 / (dx**2.0 + dy**2.0) ** 0.5
                                   GridScore[0][x][y] += Rating * GridCluster[Nc][nx][ny]
                                   GridScore[1][x][y] += Rating
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Score :  GridScore[%s][0] = %s' % (Nc, GridScore[0]))
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Rating:  GridScore[%s][1] = %s' % (Nc, GridScore[1]))
            GridCluster[Nc] = GridScore[0] / GridScore[1]
            del GridScore
        if ((GridCluster[Nc] > Questionable)*1).sum() < 0.1: Lines[Nc][2] = False
    SDP.ShowCluster(GridCluster, [Valid, Marginal, Questionable], Lines, Abcissa, x0, y0, GridSpaceRA, GridSpaceDEC, 'smoothing', ShowPlot, FigFileDir, FigFileRoot)

    ProcEndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Clustering: Smoothing Stage End: %s (Elapsed time = %.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))

    ######## Clustering: Final Stage ########
    ProcStartTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Clustering: Final Stage Start: %s' % time.ctime(ProcStartTime))

    HalfGrid = (GridSpaceRA ** 2 + GridSpaceDEC ** 2) ** 0.5 / 2.0
    # Clean isolated grids
    for Nc in range(Ncluster):
        if Lines[Nc][2] != False:
            Plane = (GridCluster[Nc] > Marginal) * 1
            if Plane.sum() == 0:
                Lines[Nc][2] = False
                continue
            Original = GridCluster[Nc].copy()
            # Clear GridCluster Nc-th plane
            GridCluster[Nc] *= 0.0
            Nmember = []
            Realmember = []
            MemberList = []
            NsubCluster = 0
            for x in range(nra):
                for y in range(ndec):
                    if Plane[x][y] == 1:
                        Plane[x][y] = 2
                        SearchList = [(x, y)]
                        M = 1
                        if Original[x][y] > Valid: MM = 1
                        #if Original[x][y] > Marginal: MM = 1
                        else: MM = 0
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
                                        if Original[nx][ny] > Valid: MM += 1
                                        #if Original[nx][ny] > Marginal: MM += 1
                                        MemberList[NsubCluster].append((nx, ny))
                            del SearchList[0]
                        Nmember.append(M)
                        Realmember.append(MM)
                        NsubCluster += 1

            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Nmember = %s' % Nmember)
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='MemberList = %s' % MemberList)
            # If no members left, skip to next cluster
            if len(Nmember) == 0: continue
            # Threshold is set to half the number of the largest cluster in the plane
            #Threshold = max(Nmember) / 2.0
            Threshold = min(max(Realmember) / 2.0, 3)
            for n in range(NsubCluster -1, -1, -1):
                # isolated cluster made from single spectrum should be omitted
                if Nmember[n] == 1:
                    (x, y) = MemberList[n][0]
                    if GridMember[x][y] <= 1:
                        Nmember[n] = 0
                # Sub-Cluster whose member below the threshold is cleaned
                if Nmember[n] < Threshold:
                    for (x, y) in MemberList[n]:
                        Plane[x][y] == 0
                    del Nmember[n]
                    del MemberList[n]
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Cluster Member = %s' % Nmember)

            # Blur each SubCluster with the radius of sqrt(Nmember/Pi) * ratio
            ratio = SDC.SDParam['Cluster']['BlurRatio']
            # Set-up SubCluster
            for n in range(len(Nmember)):
                ##SubPlane = NA.zeros((nra, ndec), type=NA.Float32)
                SubPlane = NP.zeros((nra, ndec), dtype=NP.float32)
                xlist = []
                ylist = []
                for (x, y) in MemberList[n]:
                    SubPlane[x][y] = Original[x][y]
                    xlist.append(x)
                    ylist.append(y)
                # Calculate Blur radius
                Blur = int((Realmember[n] / 3.141592653) ** 0.5 * ratio + 0.5)
                # Set-up kernel for convolution
                # caution: if nra < (Blur*2+1) and ndec < (Blur*2+1)
                #  => dimension of SPC.convolve2d(Sub,kernel) gets not (nra,ndec) but (Blur*2+1,Blur*2+1)
                if nra < (Blur * 2 + 1) and ndec < (Blur * 2 + 1): Blur = int((max(nra, ndec) - 1) / 2)
                ##kernel = NA.zeros((Blur * 2 + 1, Blur * 2 + 1))
                kernel = NP.zeros((Blur * 2 + 1, Blur * 2 + 1),dtype=int)
                for x in range(Blur * 2 + 1):
                    for y in range(Blur * 2 + 1):
                        if abs(Blur - x) + abs(Blur - y) <= Blur:
                            kernel[x][y] = 1
                BlurPlane = (SPC.convolve2d(SubPlane, kernel) > Marginal) * 1
                ValidPlane = (SubPlane > Valid) * 1
                SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='kernel.shape = %s' % list(kernel.shape))
                SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='GridCluster.shape = %s' % list(GridCluster.shape))
                SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Plane.shape = %s' % list(Plane.shape))
                SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='SubPlane.shape = %s' % list(SubPlane.shape))
                SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='BlurPlane.shape = %s' % list(BlurPlane.shape))
                for x in range(len(Plane)):
                    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg=' %d : %s' % (x, list(Plane[x])))
                for x in range(len(BlurPlane)):
                    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg=' %d : %s' % (x, list(BlurPlane[x])))

                # 2D fit for each Plane
                # Use the data for fit if GridCluster[Nc][x][y] > Valid
                # Not use for fit but apply the value at the border if GridCluster[Nc][x][y] > Marginal

                # Determine fitting order if not specified
                ##if Xorder < 0: Xorder0 = min(((NA.sum(ValidPlane, axis=0) > 0.5)*1).sum() - 1, 5)
                if Xorder < 0: Xorder0 = min(((NP.sum(ValidPlane, axis=0) > 0.5)*1).sum() - 1, 5)
                ##if Yorder < 0: Yorder0 = min(((NA.sum(ValidPlane, axis=1) > 0.5)*1).sum() - 1, 5)
                if Yorder < 0: Yorder0 = min(((NP.sum(ValidPlane, axis=1) > 0.5)*1).sum() - 1, 5)
                #if Xorder < 0: Xorder0 = min(max(max(xlist) - min(xlist), 0), 5)
                #if Yorder < 0: Yorder0 = min(max(max(ylist) - min(ylist), 0), 5)
                SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='(X,Y)order = (%d, %d)' % (Xorder0, Yorder0))

                # clear Flag
                for i in range(len(category)): Region[i][5] = 1

                for iteration in range(3):
                    # FitData: [(Width, Center, RA, DEC)]
                    FitData = []
                    for i in range(len(category)):
                        if category[i] == Nc and Region[i][5] == 1 and SubPlane[int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] > Valid:
                            FitData.append((Region2[i][0], Region2[i][1], Region[i][3], Region[i][4]))
                    if len(FitData) == 0: break

                    # make arrays for coefficient calculation
                    # Matrix    MM x A = B  ->  A = MM^-1 x B
                    ##M0 = NA.zeros((Xorder0 * 2 + 1) * (Yorder0 * 2 + 1), type=NA.Float64)
                    M0 = NP.zeros((Xorder0 * 2 + 1) * (Yorder0 * 2 + 1), dtype=NP.float64)
                    ##M1 = NA.zeros((Xorder0 * 2 + 1) * (Yorder0 * 2 + 1), type=NA.Float64)
                    M1 = NP.zeros((Xorder0 * 2 + 1) * (Yorder0 * 2 + 1), dtype=NP.float64)
                    ##B0 = NA.zeros((Xorder0 + 1) * (Yorder0 + 1), type=NA.Float64)
                    B0 = NP.zeros((Xorder0 + 1) * (Yorder0 + 1), dtype=NP.float64)
                    ##B1 = NA.zeros((Xorder0 + 1) * (Yorder0 + 1), type=NA.Float64)
                    B1 = NP.zeros((Xorder0 + 1) * (Yorder0 + 1), dtype=NP.float64)
                    ##MM0 = NA.zeros([(Xorder0 + 1) * (Yorder0 + 1), (Xorder0 + 1) * (Yorder0 + 1)], type=NA.Float64)
                    MM0 = NP.zeros([(Xorder0 + 1) * (Yorder0 + 1), (Xorder0 + 1) * (Yorder0 + 1)], dtype=NP.float64)
                    ##MM1 = NA.zeros([(Xorder0 + 1) * (Yorder0 + 1), (Xorder0 + 1) * (Yorder0 + 1)], type=NA.Float64)
                    MM1 = NP.zeros([(Xorder0 + 1) * (Yorder0 + 1), (Xorder0 + 1) * (Yorder0 + 1)], dtype=NP.float64)
                    for (Width, Center, x, y) in FitData:
                        for k in range(Yorder0 * 2 + 1):
                            for j in range(Xorder0 * 2 + 1):
                                M0[j + k * (Xorder0 * 2 + 1)] += math.pow(x, j) * math.pow(y, k)
                        for k in range(Yorder0 + 1):
                            for j in range(Xorder0 + 1):
                                B0[j + k * (Xorder0 + 1)] += math.pow(x, j) * math.pow(y, k) * Center
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
                    ##A0 = LA.solve_linear_equations(MM0, B0)
                    ##A1 = LA.solve_linear_equations(MM1, B1)
                    A0 = LA.solve(MM0, B0)
                    A1 = LA.solve(MM1, B1)
                    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='A0 = %s' % A0)
                    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='M[0] = %s' % M0[0])

                    # Calculate Sigma
                    # Sigma should be calculated in the upper stage
                    Diff = []
                    Number = []
                    for (Width, Center, x, y) in FitData:
                        Fit0 = Fit1 = 0.0
                        for k in range(Yorder0 + 1):
                            for j in range(Xorder0 + 1):
                                Coef = math.pow(x, j) * math.pow(y, k)
                                Fit0 += Coef * A0[j + k * (Xorder0 + 1)]
                                Fit1 += Coef * A1[j + k * (Xorder0 + 1)]
                        Diff.append(((Fit0 - Center)**2.0 + (Fit1 - Width)**2.0)**0.5)
                    ##if len(Diff) > 1: Threshold = NA.array(Diff).stddev() * Nsigma
                    if len(Diff) > 1: Threshold = NP.array(Diff).std() * Nsigma
                    else: Threshold *= 2.0
                    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='2D Fit Iteration = %d' % iteration)
                    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='2D Fit Threshold = %s' % Threshold)

                    # Sigma Clip
                    NFlagged = 0
                    Number = 0
                    for i in range(len(category)):
                        if category[i] == Nc and SubPlane[int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] > Valid:
                            #FitData.append((Region2[i][0], Region2[i][1], Region[i][3], Region[i][4]))
                            #for (Width, Center, x, y) in FitData:
                            Number += 1
                            Fit0 = Fit1 = 0.0
                            for k in range(Yorder0 + 1):
                                for j in range(Xorder0 + 1):
                                    #Coef = math.pow(x, j) * math.pow(y, k)
                                    Coef = math.pow(Region[i][3], j) * math.pow(Region[i][4], k)
                                    Fit0 += Coef * A0[j + k * (Xorder0 + 1)]
                                    Fit1 += Coef * A1[j + k * (Xorder0 + 1)]
                            if ((Fit0 - Region2[i][1])**2.0 + (Fit1 - Region2[i][0])**2.0)**0.5 < Threshold: Region[i][5] = 1
                            else:
                                Region[i][5] = 0
                                NFlagged += 1
                    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='2D Fit Flagged/All = (%s, %s)' % (NFlagged, Number))
                # Iteration End

                # FitData: [(Width, Center, RA, DEC)]
                FitData = []
                for i in range(len(category)):
                    if category[i] == Nc and Region[i][5] == 1 and SubPlane[int((Region[i][3] - x0)/GridSpaceRA)][int((Region[i][4] - y0)/GridSpaceDEC)] > Valid:
                        FitData.append((Region2[i][0], Region2[i][1], Region[i][3], Region[i][4]))
                if len(FitData) == 0: continue
                    
                # Calculate Fit for each position
                for x in range(nra):
                    for y in range(ndec):
                        if ValidPlane[x][y] == 1:
                            for ID in Grid2SpectrumID[x][y]:
                                Fit0 = Fit1 = 0.0
                                PID = rows.index(ID)
                                for k in range(Yorder0 + 1):
                                    for j in range(Xorder0 + 1):
                                        Coef = math.pow(PosList[0][PID], j) * math.pow(PosList[1][PID], k)
                                        Fit0 += Coef * A0[j + k * (Xorder0 + 1)]
                                        Fit1 += Coef * A1[j + k * (Xorder0 + 1)]
                                if (Fit1 >= MinFWHM) and (Fit1 <= MaxFWHM):
                                    # Allowance = Fit1 / 2.0 * 1.3
                                    # To keep broad line region, make allowance larger
                                    Allowance = Fit1 / 2.0 * 1.5
                                    Protect = [max(int(Fit0 - Allowance), 0), min(int(Fit0 + Allowance), nChan - 1)]
                                    if RealSignal.has_key(ID):
                                        tmplist = RealSignal[ID][2]
                                        tmplist.append(Protect)
                                        RealSignal[ID][2] = tmplist
                                    else:
                                        RealSignal[ID] = [DataTable[ID][9], DataTable[ID][10] ,[Protect]]
                        elif BlurPlane[x][y] == 1:
                            # in Blur Plane, Fit is not extrapolated, but use the nearest value in Valid Plane
                            # Search the nearest Valid Grid
                            Nearest = []
                            Dist2 = []
                            for xx in range(nra):
                                for yy in range(ndec):
                                    if ValidPlane[xx][yy] == 1:
                                        Dist2.append(((xx - x)*GridSpaceRA)**2 + ((yy - y)*GridSpaceDEC)**2)
                                        Nearest.append([xx, yy])
                            ##ID = NA.argmin(NA.array(Dist2))
                            ID = NP.argmin(NP.array(Dist2))
                            (RA0, DEC0) = (x0 + GridSpaceRA * (x + 0.5), y0 + GridSpaceDEC * (y + 0.5))
                            (RA1, DEC1) = (x0 + GridSpaceRA * (Nearest[ID][0] + 0.5), y0 + GridSpaceDEC * (Nearest[ID][1] + 0.5))

                            # Setup the position near the border
                            RA2 = RA1 - (RA1 - RA0) * HalfGrid / (Dist2[ID] ** 0.5)
                            DEC2 = DEC1 - (DEC1 - DEC0) * HalfGrid / (Dist2[ID] ** 0.5)
                            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='[X,Y],[XX,YY] = [%d,%d],%s' % (x,y,Nearest[ID]))
                            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='(RA0,DEC0),(RA1,DEC1),(RA2,DEC2) = (%.5f,%.5f),(%.5f,%.5f),(%.5f,%.5f)' % (RA0,DEC0,RA1,DEC1,RA2,DEC2))
                            # Calculate Fit and apply same value to all the spectra in the Blur Grid
                            Fit0 = Fit1 = 0.0
                            for k in range(Yorder0 + 1):
                                for j in range(Xorder0 + 1):
                                    # Border case
                                    #Coef = math.pow(RA2, j) * math.pow(DEC2, k)
                                    # Center case
                                    Coef = math.pow(RA1, j) * math.pow(DEC1, k)
                                    Fit0 += Coef * A0[j + k * (Xorder0 + 1)]
                                    Fit1 += Coef * A1[j + k * (Xorder0 + 1)]
                            if (Fit1 >= MinFWHM) and (Fit1 <= MaxFWHM):
                                #Allowance = Fit1 / 2.0 * 1.3
                                # To keep broad line region, make allowance larger
                                Allowance = Fit1 / 2.0 * 1.5
                                Protect = [max(int(Fit0 - Allowance + 0.5), 0), min(int(Fit0 + Allowance + 0.5), nChan - 1)]
                                for ID in Grid2SpectrumID[x][y]:
                                    if RealSignal.has_key(ID):
                                        tmplist = RealSignal[ID][2]
                                        tmplist.append(Protect)
                                        RealSignal[ID][2] = tmplist
                                    else:
                                        RealSignal[ID] = [DataTable[ID][9], DataTable[ID][10] ,[Protect]]
                            else: continue
                # for Plot
                GridCluster[Nc] += BlurPlane
            if ((GridCluster[Nc] > 0.5)*1).sum() < Questionable: Lines[Nc][2] = False
            for x in range(nra):
                for y in range(ndec):
                    if GridCluster[Nc][x][y] > 0.5: GridCluster[Nc][x][y] = 1.0
                    if Original[x][y] > Valid: GridCluster[Nc][x][y] = 2.0
    SDP.ShowCluster(GridCluster, [1.5, 0.5, 0.5, 0.5], Lines, Abcissa, x0, y0, GridSpaceRA, GridSpaceDEC, 'regions', ShowPlot, FigFileDir, FigFileRoot)
    ProcEndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Clustering: Final Stage End: %s (Elapsed time = %.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))

    # Merge masks if possible
    for row in rows:
        if RealSignal.has_key(row):
            if len(RealSignal[row][2]) != 1:
                ##Region = NA.ones(nChan + 2)
                Region = NP.ones(nChan + 2, dtype=int)
                for [chan0, chan1] in RealSignal[row][2]:
                    Region[chan0 + 1:chan1 + 1] = 0
                dummy = (Region[1:] - Region[:-1]).nonzero()
                RealSignal[row][2] = []
                for y in range(0, len(dummy[0]), 2):
                    RealSignal[row][2].append([dummy[0][y], dummy[0][y + 1]])
        else:
            RealSignal[row] = [DataTable[row][9], DataTable[row][10], [[-1, -1]]]
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='DataTable = %s, RealSignal = %s' % (DataTable[row][21], RealSignal[row][2]))
        if DataTable[row][22] == RealSignal[row][2]:
            if type(DataTable[row][22]) != int:
                DataTable[row][22] = ITER - 1
        else:
            DataTable[row][21] = RealSignal[row][2]
            DataTable[row][22] = False

    del GridCluster
    del RealSignal
    EndTime = time.time()
    #SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='End %s, Elapsed time = %.1f sec' % (time.ctime(EndTime), (EndTime - StartTime)))
    return Lines



def Process5(SpStorage, DataTable, TimeTableList, rows, edge=(0, 0), LogLevel=2, LogFile=False):

    # Determine Baseline-Fitting order for each time-bin
    #  DataTable[row][21] = [[LineStartChannel1, LineEndChannel1],
    #                        [LineStartChannel2, LineEndChannel2],
    #                        [LineStartChannelN, LineEndChannelN]]
    #
    #  RealSignal = {ID1,[RA,DEC,[[LineStartChannel1, LineEndChannel1],
    #                             [LineStartChannel2, LineEndChannel2],
    #                             [LineStartChannelN, LineEndChannelN]]],
    #                IDn,[RA,DEC,[[LineStartChannel1, LineEndChannel1],
    #                             [LineStartChannelN, LineEndChannelN]]]}

    import SDpipelineControl as SDC
    reload(SDC)

    NROW = len(rows)
    StartTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process5  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Start: %s' % time.ctime(StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Determine Baseline-Fitting order for each time-bin...')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Applied time bin: %s' % SDC.SDParam['FittingOrder']['ApplicableDuration'])
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d spectra...' % NROW)

    # Select time gap list: 'subscan': large gap; 'raster': small gap
    if SDC.SDParam['FittingOrder']['ApplicableDuration'] == 'subscan':
        TimeTable = TimeTableList[1]
    else:
        TimeTable = TimeTableList[0]

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
    # Mask edges and detected lines out
    # Offset the spectrum to make the average to be zero
    for row in rows:
        ##mask = NA.ones(len(SpStorage[0][0]))
        mask = NP.ones(len(SpStorage[0][0]))
        if EdgeL > 0: mask[:EdgeL] = 0
        if EdgeR > 0: mask[-EdgeR:] = 0
        for line in DataTable[row][21]:
            if line[0] != -1:
                mask[line[0]:line[1]] = 0
                #SP[row][line[0]:line[1]] = 0.0
        ave = (SpStorage[0][row] * mask).sum() / float(mask.sum())
        SP[row] = (SpStorage[0][row] - ave) * mask
        del mask

    GroupList = TimeTable
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='GroupList:%s' % GroupList)

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
            # FFT in numarray has a bug! which reduce reference number
            #SpFFT.append(NA.abs(FFT.real_fft(SP[x])))
            ##SpFFT.append(NA.abs(FFTP.rfft(SP[x])))
            SpFFT.append(NP.abs(FFTP.rfft(SP[x])))
        # Average seems to be better than median
        #Power = NML.median(SpFFT)
        print 'len(SpFFT) =', len(SpFFT)
        ##Power = NA.average(SpFFT)
        Power = NP.average(SpFFT,axis=0)
        N = int(MaxDominantFreq * NCHAN / 2048.0)
        # 2007/09/01 Absolute value of power should be taken into account
        # If the power is low, it should be ignored
        # Normalize the power
        Power2 = Power / Power.mean()
        MaxPower = Power2[:N].max()
        if MaxPower < 3.0: Order = 1.0
        elif MaxPower < 5.0: Order = 1.5
        elif MaxPower < 10.0: Order = 2.0
        else:
            flag = False
            for i in range(N, -1, -1):
                if Power2[i] > 10.0: break
                if Power2[i] > 5.0: flag = True
            if flag == True: Order = float(max(2.0, i)) + 0.5
            else: Order = float(max(2.0, i))

        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Power= %s' % (Power2[:N+1]))
        for x in TGroup[y]:
            output.append([x, Order])
            SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Time bin = %s, Number of nodes determined from Power spectrum = %s' % (x, Order))

    EndTime = time.time()
    #SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='End: %s, Elapsed time = %.1f sec' % (time.ctime(EndTime), (EndTime - StartTime)))
    return dict(output)



def GroupByTime(DataTable, basedata, rows, LogLevel=2, LogFile=False):
    '''
    Grouping by time sequence
    TimeTable: [[[row0, row1,..., rowN], [rowXX,..., rowXXX],...,[,,]], [[],[],[]]]
    TimeTable[0]: small gap (for process5 and 7)
    TimeTable[1]: large gap (for process5 and 7)
    TimeGap: [[rowX1, rowX2,...,rowXN], [rowY1, rowY2,...,rowYN]]
    TimeGap[0]: small gap (for plot)
    TimeGap[1]: large gap (for plot)
    '''
    
    StartTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  GroupByTime  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Start: %s' % time.ctime(StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Grouping by Time...')

    plotdata = [[],[]]
    for ID in basedata:
        plotdata[0].append(DataTable[ID][0])
        plotdata[1].append(DataTable[ID][7])
    Nrows = len(basedata)

    ##DeltaT = NA.array(plotdata[1]) - NA.array([plotdata[1][0]] + plotdata[1][:-1])
    DeltaT = NP.array(plotdata[1]) - NP.array([plotdata[1][0]] + plotdata[1][:-1])
    ##Threshold1 = NML.median(DeltaT) * 5.0
    #Threshold1 = NP.median(DeltaT) * 5.0
    # 2009/2/5 for multi beam
    DeltaT1 = NP.take(DeltaT, NP.nonzero(DeltaT)[0])
    Threshold1 = NP.median(DeltaT1) * 5.0
    ##DeltaT2 = NA.take(DeltaT, NA.nonzero(DeltaT > Threshold1)[0])
    #DeltaT2 = NP.take(DeltaT, NP.nonzero(DeltaT > Threshold1)[0])
    DeltaT2 = NP.take(DeltaT1, NP.nonzero(DeltaT1 > Threshold1)[0])
    ##Threshold2 = NML.median(DeltaT2) * 5.0
    if len(DeltaT2) > 0:
        Threshold2 = NP.median(DeltaT2) * 5.0
    else:
        Threshold2 = Threshold1

    #ThresholdT = NML.median(DeltaT) * 10.0
    SDT.LogMessage('LONG', LogLevel, LogFile, Msg='plotdata[1] = %s' % plotdata[1])
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Threshold1 = %s sec' % Threshold1)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Threshold2 = %s sec' % Threshold2)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='MaxDeltaT = %s sec' % DeltaT1.max())
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='MinDeltaT = %s sec' % DeltaT1.min())

    TimeTable = [[],[]]
    SubTable1 = []
    SubTable2 = []
    TimeGap = [[],[]]
    for x in range(Nrows):
        if plotdata[0][x] in rows:
            if DeltaT[x] <= Threshold1:
                SubTable1.append(plotdata[0][x])
            else:
                TimeTable[0].append(SubTable1)
                SubTable1 = [plotdata[0][x]]
                TimeGap[0].append(plotdata[0][x])
                SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Small Time Gap: %s sec at row=%d' % (DeltaT[x], x))
            if DeltaT[x] <= Threshold2:
                SubTable2.append(plotdata[0][x])
            else:
                TimeTable[1].append(SubTable2)
                SubTable2 = [plotdata[0][x]]
                TimeGap[1].append(plotdata[0][x])
                SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Large Time Gap: %s sec at row=%d' % (DeltaT[x], x))
    if len(SubTable1) > 0: TimeTable[0].append(SubTable1)
    if len(SubTable2) > 0: TimeTable[1].append(SubTable2)
    del SubTable1, SubTable2, plotdata, DeltaT, DeltaT1, DeltaT2
    
    if len(TimeGap[0])==0: 
        TimeGapMsg = 'Found no time gap'
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg=TimeGapMsg)
    else:
        TimeGapMsg1 = 'Found %d small time gap(s)' % len(TimeGap[0])
        TimeGapMsg2 = 'Found %d large time gap(s)' % len(TimeGap[1])
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg=TimeGapMsg1)
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg=TimeGapMsg2)
    EndTime = time.time()
    #SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='End: %s,  Elapsed time = %.1f sec' % (time.ctime(EndTime), (EndTime - StartTime)))

    return (TimeTable, TimeGap)



def GroupByPosition(DataTable, rows, CombineRadius, AllowanceRadius, OutDict=True, LogLevel=2, LogFile=False):
    '''
    Grouping by RA/DEC position
    PosGap: [rowX1, rowX2,...,rowXN]
    '''
    
    StartTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  GroupByPosition  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Start %s' % time.ctime(StartTime))
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

    ##ArrayRA = NA.array(plotdata[1])
    ##ArrayDEC = NA.array(plotdata[2])
    ArrayRA = NP.array(plotdata[1])
    ArrayDEC = NP.array(plotdata[2])

    # 2009/2/6 to be quick
    #if OutDict:
    #    for x in range(Nrows):
    #        if not Dict.has_key(plotdata[0][x]):
    #            DeltaRA = ArrayRA - plotdata[1][x]
    #            DeltaDEC = ArrayDEC - plotdata[2][x]
    #            Delta2 = DeltaRA * DeltaRA + DeltaDEC * DeltaDEC
    #            ##Unique = NA.less_equal(Delta2, ThresholdAR)
    #            ##Select = NA.less_equal(Delta2, ThresholdR)
    #            Unique = NP.less_equal(Delta2, ThresholdAR)
    #            Select = NP.less_equal(Delta2, ThresholdR)
    #            for y in range(x + 1, Nrows):
    #                if Unique[y] == 1 and (not Dict.has_key(plotdata[0][y])):
    #                    Dict[plotdata[0][y]] = [-1,plotdata[0][x]]
    #            line = []
    #            for y in range(Nrows):
    #                if Select[y] == 1: line.append(plotdata[0][y])
    #            Dict[plotdata[0][x]] = line
    #    del DeltaRA, DeltaDEC, Delta2, Unique, Select, line
    # 2009/2/10 Quicker Method
    if OutDict:
        #UniqueDict = {}
        SelectDict = {}
        MinRA = ArrayRA.min()
        MaxRA = ArrayRA.max()
        MinDEC = ArrayDEC.min()
        MaxDEC = ArrayDEC.max()
        for x in range(Nrows):
           #uRA = int((ArrayRA[x] - MinRA) / AllowanceRadius / 2.0)
           #uDEC = int((ArrayDEC[x] - MinDEC) / AllowanceRadius / 2.0)
           sRA = int((ArrayRA[x] - MinRA) / CombineRadius / 2.0)
           sDEC = int((ArrayDEC[x] - MinDEC) / CombineRadius / 2.0)

           #if not UniqueDict.has_key(uRA): UniqueDict[uRA] = {}
           #if not UniqueDict[uRA].has_key(uDEC): UniqueDict[uRA][uDEC] = [plotdata[0][x]]
           #else: UniqueDict[uRA][uDEC].append(plotdata[0][x])

           if not SelectDict.has_key(sRA): SelectDict[sRA] = {}
           if not SelectDict[sRA].has_key(sDEC): SelectDict[sRA][sDEC] = [plotdata[0][x]]
           else: SelectDict[sRA][sDEC].append(plotdata[0][x])

        #SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='UniqueDict.keys() : %s' % UniqueDict.keys())
        #for uRA in UniqueDict.keys():
            #SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='len(UniqueDict[%s].keys() : %s' % (uRA, len(UniqueDict[uRA].keys()))
            #for uDEC in UniqueDict[uRA].keys():
                #if len(UniqueDict[uRA][uDEC]) != 1:
                    #for x in UniqueDict[uRA][uDEC][1:]:
                        #Dict[x] = [-1,UniqueDict[uRA][uDEC][0]]

        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='SelectDict.keys() : %s' % SelectDict.keys())
        for sRA in SelectDict.keys():
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='len(SelectDict[%s].keys()) : %s' % (sRA, len(SelectDict[sRA].keys())))
            for sDEC in SelectDict[sRA].keys():
                Dict[SelectDict[sRA][sDEC][0]] = SelectDict[sRA][sDEC]
                if len(SelectDict[sRA][sDEC]) != 1:
                    for x in SelectDict[sRA][sDEC][1:]:
                        Dict[x] = [-1,SelectDict[sRA][sDEC][0]]

        #del UniqueDict
        del SelectDict
                    

    ##DeltaP = NA.sqrt((ArrayRA[1:] - ArrayRA[:-1])**2.0 + (ArrayDEC[1:] - ArrayDEC[:-1])**2.0)
    DeltaP = NP.sqrt((ArrayRA[1:] - ArrayRA[:-1])**2.0 + (ArrayDEC[1:] - ArrayDEC[:-1])**2.0)
    DeltaQ = NP.take(DeltaP, NP.nonzero(DeltaP > ThresholdAR)[0])
    if len(DeltaQ) != 0:
        ThresholdP = NP.median(DeltaQ) * 10.0
    else:
        ThresholdP = 0.0
    #ThresholdP = NP.median(DeltaP) * 10.0
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='threshold:%s deg' % ThresholdP)
    PosGap = []
    for x in range(1, Nrows):
        if DeltaP[x - 1] > ThresholdP:
            PosGap.append(rows[x])
            SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Position Gap %s deg at row=%d' % (DeltaP[x-1], rows[x]))

    nPosGap = len(PosGap)
    if nPosGap == 0:
        PosGapMsg = 'Found no position gap'
    else:
        PosGapMsg = 'Found %d position gap(s)' % nPosGap

    EndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg=PosGapMsg)
    #SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='End: %s, Elapsed time = %.1f sec' % (time.ctime(EndTime), (EndTime - StartTime)))
    
    return (Dict, PosGap)


def MergeGapTables(DataTable, TimeGap, TimeTable, PosGap, LogLevel=2, LogFile=False):
    '''
    Merge TimeGap Table and PosGap Table. PosGap is merged into TimeGap Table[0]: Small gap
    TimeTable[0]: small gap (for process5 and 7)
    TimeTable[1]: large gap (for process5 and 7)
    '''
    StartTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  MergeGapTables  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Start %s' % time.ctime(StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Merging Position and Time Gap tables...')

    rows = []
    for i in range(len(TimeTable[0])):
        rows += TimeTable[0][i]
    ##ROWS = list(NA.sort(NA.array(rows)))
    ROWS = list(NP.sort(NP.array(rows)))
    row = -1
    ##tmpGap = list(NA.sort(NA.array(TimeGap[0] + PosGap)))
    tmpGap = list(NP.sort(NP.array(TimeGap[0] + PosGap)))
    t = tmpGap[0]
    NewGap = []
    for row in tmpGap[1:]:
        if t != row and t in ROWS:
            NewGap.append(t)
            t = row
    if row in ROWS:
        NewGap.append(row)
    TimeGap[0] = NewGap

    SubTable = []
    TimeTable[0] = []
    for row in ROWS:
        if row in TimeGap[0]:
            TimeTable[0].append(SubTable)
            SubTable = [row]
            SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Small Time Gap at row=%d' % row)
        else:
            SubTable.append(row)
    if len(SubTable) > 0: TimeTable[0].append(SubTable)

    #print TimeTable
    #print TimeGap

    # 2009/2/6 Divide TimeTable in accordance with the Beam
    TimeTable2 = TimeTable[:]
    TimeTable = [[],[]]
    for i in range(len(TimeTable2)):
        for rows in TimeTable2[i]:
            BeamDict = {}
            for row in rows:
                if BeamDict.has_key(DataTable[row][4]):
                    BeamDict[DataTable[row][4]].append(row)
                else:
                    BeamDict[DataTable[row][4]] = [row]
            BeamList = BeamDict.values()
            for beam in BeamList:
                TimeTable[i].append(beam)

    del BeamDict, BeamList, TimeTable2
    EndTime = time.time()
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='TimeTable = %s' % (TimeTable))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='End: %s, Elapsed time = %.1f sec' % (time.ctime(EndTime), (EndTime - StartTime)))
    return(TimeTable, TimeGap)


def ObsPatternAnalysis(PosDict, rows, LogLevel=2, LogFile=False):
    '''
    Analyze pointing pattern
    '''
    StartTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  ObsPatternAnalysis  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Start: %s' % time.ctime(StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Analyze Scan Pattern by Positions...')

    nPos = 0
    #MaxIntegration = 0
    for row in rows:
    #    if len(PosDict[row]) > MaxIntegration:
    #        MaxIntegration = len(PosDict[row])
        if PosDict[row][0] != -1:
            nPos += 1
        if nPos == 0: nPos = 1
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Number of Spectra: %d,   Number of independent position > %d' % (len(rows), nPos))
    if nPos > math.sqrt(len(rows)) or nPos > 10: ret = 'RASTER'
    elif nPos == 1: ret = 'SINGLE-POINT'
    else: ret = 'MULTI-POINT'
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Pattern is %s' % (ret))
    EndTime = time.time()
    #SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='Done: Elapsed time = %.1f sec' % (EndTime - StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='End: %s, Elapsed time = %.1f sec' % (time.ctime(EndTime), (EndTime - StartTime)))
    return ret



def Process6(SpStorage, DataTable, rows, scan, DictFitOrder, Abcissa, TimeGapList=[[],[]], fixscale=True, stepbystep=False, showevery=10, saveparams=True, savefile='BaselineFitRating.csv', edge=(0, 0), LogLevel=2, LogFile=False, ShowRMS=True, ShowPlot=True, FigFileDir=False, FigFileRoot=False):
    
    import SDpipelineControl as SDC
    reload(SDC)

    MaxPolynomialOrder = SDC.SDParam['FittingOrder']['MaxPolynomialOrder']
    # MaxPolynomialOrder = 'none' or any integer

    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process6  ' + '>'*15)

    TimeGap = TimeGapList[1]
    PosGap = TimeGapList[0]

    if stepbystep: showevery = 1
    if ShowRMS: SDP.ShowResult(mode='init', title='Statistics Plot: Baseline RMS for Raw and Processed data')

    ########## Baseline fit for each spectrum ##########

    # Duplicate scantable object for output
    dummyscan = scan.copy()
    #del scan

    # Initialize variables
    n = 0
    Rows = []
    NROW = len(rows)

    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Baseline Fit: background subtraction...')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d spectra...' % NROW)
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
    # Common to All plot
    xaxis = Abcissa[1]
    Xrange = [min(Abcissa[1][0], Abcissa[1][-1]), max(Abcissa[1][0], Abcissa[1][-1])]

    for row in rows:
        counter += 1
        SDT.LogMessage('TIMER', LogLevel, LogFile, Msg='=' * 30 + 'Processing at row = %s' % row  + '=' * 30)
        Rows.append(row)
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='row = %s, Flag = %s' % (row, DataTable[row][22]))

        if type(DataTable[row][22]) != int:
            ##dummyscan._setspectrum(SpStorage[0][row])
            dummyscan._setspectrum(NP.array(SpStorage[0][row], dtype=NP.float64))
            NCHAN = len(SpStorage[0][row])
            #polyorder = int(DictFitOrder[row] * 3.0)
            #polyorder = int((DictFitOrder[row] + 1) * 2.0 + 0.5)
            polyorder = int(DictFitOrder[row] * 2.0 + 1.5)
            # 2008/9/23 to limit maximum polyorder by user input
            if MaxPolynomialOrder != 'none':
                polyorder = min(MaxPolynomialOrder, polyorder)

            maxwidth = 1
            for [Chan0, Chan1] in DataTable[row][21]:
                if Chan1 - Chan0 >= maxwidth: maxwidth = int((Chan1 - Chan0 + 1) / 1.4)
            # allowance in Process3 is 1/5: (1 + 1/5 + 1/5)^(-1) = (5/7)^(-1) = 7/5 = 1.4
            maxpolyorder = int((NCHAN - edge[0] - edge[1]) / maxwidth + 1)

            SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Masked Region from previous processes = %s' % DataTable[row][22])
            #SDT.LogMessage('INFO', LogLevel, LogFile, Msg='polyorder = %d edge= %s' % (polyorder, edge))
            SDT.LogMessage('INFO', LogLevel, LogFile, Msg='global edge parameters= (%s,%s)' % edge)
            SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Polynomial order = %d  Max Polynomial order = %d' % (polyorder, maxpolyorder))
            #(SpStorage[1][row], NewRMS, OldRMS, nmask, win_polyorder, fragment, nwindow) = CalcBaselineFit(dummyscan, DataTable[row][21], polyorder, NCHAN, 0, edge, LogLevel=LogLevel, LogFile=LogFile)
            (SpStorage[1][row], NewRMS, OldRMS, nmask, win_polyorder, fragment, nwindow) = CalcBaselineFit(dummyscan, DataTable[row][21], min(polyorder, maxpolyorder), NCHAN, 0, edge, LogLevel=LogLevel, LogFile=LogFile)
            DataTable[row][16][1] = NewRMS
            DataTable[row][16][2] = OldRMS
            fitparams = 'poly_order=%d : %d  nwindow=%d : %d     ' % (polyorder, win_polyorder, fragment, nwindow)
        else: fitparams = ''
        statistics = 'Pre-Fit RMS=%.2f, Post-Fit RMS=%.2f' % (DataTable[row][16][2], DataTable[row][16][1])

        # Plot result spectra
        if ShowPlot or FigFileDir != False:
            if row == rows[-1]: DrawFlag = True
            YMIN = min(SpStorage[0][row].min(), SpStorage[1][row].min())
            YMAX = max(SpStorage[0][row].max(), SpStorage[1][row].max())
            Yrange = [YMIN-(YMAX-YMIN)/10.0, YMAX+(YMAX-YMIN)/10.0]
            Mask = []
            for xx in range(len(DataTable[row][21])):
                Mask.append([Abcissa[1][int(DataTable[row][21][xx][0])], Abcissa[1][int(DataTable[row][21][xx][1])]])

            SDP.DrawFitSpectrum(xaxis, SpStorage[0][row], SpStorage[1][row], row, counter, NROW, fitparams, statistics, Xrange, Yrange, Mask, DataTable[row][22], DrawFlag, ShowPlot, FigFileDir, FigFileRoot)

        # Plot result statistics
        if ((n + 1) % showevery == 0 or row == rows[-1]) and ShowRMS:
            N1 = n + 1 - showevery
            N2 = n + 1
            UpdateRows = Rows[N1:N2]
            SDP.ShowResult('online', DataTable, UpdateRows, TimeGap, PosGap)

        # Add Spectra position index (offset)
        n = n + 1
            
    del dummyscan
    EndTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='End: %s, Elapsed time = %.1f sec' % (time.ctime(EndTime), (EndTime - StartTime)) )

    return



def Process7(SpStorage, DataTable, Abcissa, rows, TimeGapList=[[],[]], TimeTableList=[[],[]], Iteration=5, interactive=True, edge=(0,0), UserFlag=[], LogLevel=2, LogFile=False, ShowPlot=True, FigFileDir=False, FigFileRoot=False):

    import SDpipelineControl as SDC
    reload(SDC)

    ThreNewRMS = SDC.SDParam['RmsPostFitFlag']['Threshold']
    ThreOldRMS = SDC.SDParam['RmsPreFitFlag']['Threshold']
    ThreNewDiff = SDC.SDParam['RunMeanPostFitFlag']['Threshold']
    ThreOldDiff = SDC.SDParam['RunMeanPreFitFlag']['Threshold']
    ThreTsys = SDC.SDParam['TsysFlag']['Threshold']
    Nmean = SDC.SDParam['RunMeanPreFitFlag']['Nmean']

    # Select time gap list: 'subscan': large gap; 'raster': small gap
    if SDC.SDParam['Flagging']['ApplicableDuration'] == "subscan":
        TimeTable = TimeTableList[1]
    else:
        TimeTable = TimeTableList[0]
    TimeGap = TimeGapList[1]
    PosGap = TimeGapList[0]

    # Calculate RMS and Diff from running mean

    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process7  ' + '>'*15)
    NROW = len(rows)
    NCHAN = len(SpStorage[0][rows[0]])
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Calculate RMS and Diff from running mean for Pre/Post fit...')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d spectra...' % NROW)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Nchan for running mean=%s' % Nmean)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Flagging thresholds for: (post-fit, pre-fit, post-fit diff, pre-fit diff, Tsys )= (%s, %s, %s, %s, %s)*RMS' \
                   % (ThreNewRMS, ThreOldRMS, ThreNewDiff, ThreOldDiff, ThreTsys))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Applied time bin for the running mean calculation: %s' % SDC.SDParam['Flagging']['ApplicableDuration'])
    SDT.LogMessage('TIMER', LogLevel, LogFile, Msg='|' + '='*36 + ' 100% ' + '=' * 36 + '|')
    StartTime = time.time()
    STEP = HASH = HASH0 = 0
    data = []
    flag = 1
    Threshold = [ThreNewRMS, ThreOldRMS, ThreNewDiff, ThreOldDiff, ThreTsys]

    ProcStartTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='RMS and diff caluculation Start:%s' % time.ctime(ProcStartTime))
    for chunk in TimeTable:
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing spectra = %s' % chunk)
        NROW = len(chunk)
        START = 0
        for row in chunk:
            STEP += 1
            START += 1
            if LogLevel > 0:
                HASH = int(80 * STEP / NROW)
                if (HASH0 != HASH):
                    print '\b' + '*' * (HASH - HASH0),
                    sys.stdout.flush()
                    HASH0 = HASH
            ##mask = NA.ones(NCHAN, NA.Int)
            mask = NP.ones(NCHAN, NP.int)
            for [m0, m1] in DataTable[row][21]: mask[m0:m1] = 0
            if edge[0] > 0: mask[:edge[0]] = 0
            if edge[1] > 0: mask[-edge[1]:] = 0
            ##Nmask = int(NCHAN - NA.sum(mask * 1.0))
            Nmask = int(NCHAN - NP.sum(mask * 1.0))

            # Calculate RMS after/before fitting
            MaskedData = SpStorage[1][row] * mask
            ##StddevMasked = MaskedData.stddev()
            StddevMasked = MaskedData.std()
            MeanMasked = MaskedData.mean()
            NewRMS = math.sqrt(abs(NCHAN * StddevMasked ** 2 / (NCHAN - Nmask) - \
                            NCHAN * Nmask * MeanMasked ** 2 / ((NCHAN - Nmask) ** 2)))
            MaskedData = SpStorage[0][row] * mask
            ##StddevMasked = MaskedData.stddev()
            StddevMasked = MaskedData.std()
            MeanMasked = MaskedData.mean()
            OldRMS = math.sqrt(abs(NCHAN * StddevMasked ** 2 / (NCHAN - Nmask) - \
                            NCHAN * Nmask * MeanMasked ** 2 / ((NCHAN - Nmask) ** 2)))
            # Calculate Diff from the running mean
            if NROW == 1:
                NewRMSdiff = 0.0
                OldRMSdiff = 0.0
            else:
                if START == 1:
                    ##Rmask = NA.zeros(NCHAN, NA.Int)
                    Rmask = NP.zeros(NCHAN, NP.int)
                    ##Rdata0 = NA.zeros(NCHAN, NA.Float64)
                    Rdata0 = NP.zeros(NCHAN, NP.float64)
                    ##Rdata1 = NA.zeros(NCHAN, NA.Float64)
                    Rdata1 = NP.zeros(NCHAN, NP.float64)
                    NR = 0
                    for x in range(1, min(Nmean + 1, NROW)):
                        NR += 1
                        Rdata0 += SpStorage[0][chunk[x]]
                        Rdata1 += SpStorage[1][chunk[x]]
                        ##mask0 = NA.ones(NCHAN, NA.Int)
                        mask0 = NP.ones(NCHAN, NP.int)
                        for [m0, m1] in DataTable[chunk[x]][21]: mask0[m0:m1] = 0
                        Rmask += mask0
                elif START > (NROW - Nmean):
                    NR -= 1
                    Rdata0 -= SpStorage[0][row]
                    Rdata1 -= SpStorage[1][row]
                    Rmask -= mask
                else:
                    Rdata0 -= (SpStorage[0][row] - SpStorage[0][chunk[START + Nmean - 1]])
                    Rdata1 -= (SpStorage[1][row] - SpStorage[1][chunk[START + Nmean - 1]])
                    ##mask0 = NA.ones(NCHAN, NA.Int)
                    mask0 = NP.ones(NCHAN, NP.int)
                    for [m0, m1] in DataTable[chunk[START + Nmean - 1]][21]: mask0[m0:m1] = 0
                    Rmask += (mask0 - mask)
                if START == 1:
                    ##Lmask = NA.zeros(NCHAN, NA.Int)
                    ##Ldata0 = NA.zeros(NCHAN, NA.Float64)
                    ##Ldata1 = NA.zeros(NCHAN, NA.Float64)
                    Lmask = NP.zeros(NCHAN, NP.int)
                    Ldata0 = NP.zeros(NCHAN, NP.float64)
                    Ldata1 = NP.zeros(NCHAN, NP.float64)
                    NL = 0
                elif START <= (Nmean + 1):
                    NL += 1
                    Ldata0 += SpStorage[0][chunk[START - 2]]
                    Ldata1 += SpStorage[1][chunk[START - 2]]
                    ##mask0 = NA.ones(NCHAN, NA.Int)
                    mask0 = NP.ones(NCHAN, NP.int)
                    for [m0, m1] in DataTable[chunk[START - 2]][21]: mask0[m0:m1] = 0
                    Lmask += mask0
                else:
                    Ldata0 += (SpStorage[0][chunk[START - 2]] - SpStorage[0][chunk[START - 2 - Nmean]])
                    Ldata1 += (SpStorage[1][chunk[START - 2]] - SpStorage[1][chunk[START - 2 - Nmean]])
                    ##mask0 = NA.ones(NCHAN, NA.Int)
                    mask0 = NP.ones(NCHAN, NP.int)
                    for [m0, m1] in DataTable[chunk[START - 2]][21]: mask0[m0:m1] = 0
                    Lmask += mask0
                    ##mask0 = NA.ones(NCHAN, NA.Int)
                    mask0 = NP.ones(NCHAN, NP.int)
                    for [m0, m1] in DataTable[chunk[START - 2 - Nmean]][21]: mask0[m0:m1] = 0
                    Lmask -= mask0

                diff0 = (Ldata0 + Rdata0) / float(NL + NR) - SpStorage[0][row]
                diff1 = (Ldata1 + Rdata1) / float(NL + NR) - SpStorage[1][row]
                mask0 = (Rmask + Lmask + mask) / (NL + NR + 1)
                #Nmask = int(NCHAN - NA.sum(mask0 * 1.0))

                # Calculate RMS after/before fitting
                MaskedData = diff1 * mask0
                ##StddevMasked = MaskedData.stddev()
                StddevMasked = MaskedData.std()
                MeanMasked = MaskedData.mean()
                print row, StddevMasked, MeanMasked, NCHAN-Nmask
                #NewRMSdiff = math.sqrt(NCHAN * StddevMasked ** 2 / (NCHAN - Nmask) - \
                #                NCHAN * Nmask * MeanMasked ** 2 / ((NCHAN - Nmask) ** 2))
                NewRMSdiff = math.sqrt(abs((NCHAN * StddevMasked ** 2 - Nmask * MeanMasked ** 2 )/ (NCHAN - Nmask)))
                MaskedData = diff0 * mask0
                ##StddevMasked = MaskedData.stddev()
                StddevMasked = MaskedData.std()
                MeanMasked = MaskedData.mean()
                print row, StddevMasked, MeanMasked, NCHAN-Nmask
                #OldRMSdiff = math.sqrt(NCHAN * StddevMasked ** 2 / (NCHAN - Nmask) - \
                #                NCHAN * Nmask * MeanMasked ** 2 / ((NCHAN - Nmask) ** 2))
                OldRMSdiff = math.sqrt(abs((NCHAN * StddevMasked ** 2 - Nmask * MeanMasked ** 2 )/ (NCHAN - Nmask)))

            data.append([row, NewRMS, OldRMS, NewRMSdiff, OldRMSdiff, DataTable[row][14], flag, flag, flag, flag, flag, Nmask])
            DataTable[row][16][1] = NewRMS
            DataTable[row][16][2] = OldRMS
            DataTable[row][16][3] = NewRMSdiff
            DataTable[row][16][4] = OldRMSdiff
            DataTable[row][20] = Nmask
            SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Row=%d, pre-fit RMS= %.2f , Post-fit RMS= %.2f' % (row, OldRMS, NewRMS))
            SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Row=%d, pre-fit diff RMS= %.2f , Post-fit diff RMS= %.2f' % (row, OldRMSdiff, NewRMSdiff))

    ProcEndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='RMS and diff caluculation End: %s, Elapse time = %.1f sec' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime))

    ProcStartTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Apply flags Start: %s' % time.ctime(ProcStartTime))
    ##tmpdata = NA.transpose(data)
    tmpdata = NP.transpose(data)
    Ndata = len(tmpdata[0])
    for cycle in range(Iteration + 1):
        threshold = []
        for x in range(5):
            ##Unflag = int(NA.sum(tmpdata[x + 6] * 1.0))
            Unflag = int(NP.sum(tmpdata[x + 6] * 1.0))
            FlaggedData = tmpdata[x + 1] * tmpdata[x + 6]
            ##StddevFlagged = FlaggedData.stddev()
            StddevFlagged = FlaggedData.std()
            if StddevFlagged == 0: StddevFlagged = tmpdata[x + 1][0] / 100.0
            MeanFlagged = FlaggedData.mean()
            AVE = MeanFlagged / float(Unflag) * float(Ndata)
            RMS = math.sqrt(Ndata * StddevFlagged ** 2 / Unflag - \
                            Ndata * (Ndata - Unflag) * MeanFlagged ** 2 / (Unflag ** 2))
            ThreP = AVE + RMS * Threshold[x]
            #ThreM = AVE - RMS * Threshold[x]
            if x == 4:
                # Tsys case
                ThreM = 0.0
            else: ThreM = -1.0
            threshold.append([ThreM, ThreP])
            for y in range(len(tmpdata[0])):
                if ThreM < tmpdata[x + 1][y] <= ThreP: tmpdata[x + 6][y] = 1
                else: tmpdata[x + 6][y] = 0
            
    N = 0
    for row in rows:
        DataTable[row][17][1] = tmpdata[6][N]
        DataTable[row][17][2] = tmpdata[7][N]
        DataTable[row][17][3] = tmpdata[8][N]
        DataTable[row][17][4] = tmpdata[9][N]
        DataTable[row][18][1] = tmpdata[10][N]
        N += 1

    # FLagging based on expected RMS
    # TODO: Include in normal flags scheme

    # The expected RMS according to the radiometer formula sometimes needs
    # special scaling factors to account for meta data conventions (e.g.
    # whether Tsys is given for DSB or SSB mode) and for backend specific
    # setups (e.g. correlator, AOS, etc. noise scaling). These factors are
    # not saved in the data sets' meta data. Thus we have to read them from
    # a special file. TODO: This needs to be changed for ALMA later on.
    try:
        fd = open('%s.exp_rms_factors' % (DataTable['FileName']), 'r')
        sc_fact_list = fd.readlines()
        fd.close()
        sc_fact_dict = {}
        for sc_fact in sc_fact_list:
            sc_fact_key, sc_fact_value = sc_fact.replace('\n','').split()
            sc_fact_dict[sc_fact_key] = float(sc_fact_value)
        tsys_fact = sc_fact_dict['tsys_fact']
        nebw_fact = sc_fact_dict['nebw_fact']
        integ_time_fact = sc_fact_dict['integ_time_fact']
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg="Using scaling factors tsys_fact=%f, nebw_fact=%f and integ_time_fact=%f for flagging based on expected RMS." % (tsys_fact, nebw_fact, integ_time_fact))
    except:
        SDT.LogMessage('WARNING', LogLevel, LogFile, Msg="Cannot read scaling factors for flagging based on expected RMS. Using 1.0.")
        tsys_fact = 1.0
        nebw_fact = 1.0
        integ_time_fact = 1.0
        

    # TODO: Make threshold a parameter
    # This needs to be quite strict to catch the ripples in the bad Orion
    # data. Maybe this is due to underestimating the total integration time.
    # Check again later.
    # 2008/10/31 divided the category into two
    #ThreExpectedRMS = SDC.SDParam['RmsExpectedFlag']['Threshold']
    ThreExpectedRMSPreFit = SDC.SDParam['RmsExpectedPreFitFlag']['Threshold']
    ThreExpectedRMSPostFit = SDC.SDParam['RmsExpectedPostFitFlag']['Threshold']
    #ThreExpectedRMS = 1.333

    # The noise equivalent bandwidth is proportional to the channel width
    # but may need a scaling factor. This factor was read above.
    noiseEquivBW = abs(Abcissa[1][1]-Abcissa[1][0]) * 1e9 * nebw_fact

    nrow = 0
    for row in rows:
        # The HHT and APEX test data show the "on" time only in the CLASS
        # header. To get the total time, at least a factor of 2 is needed,
        # for OTFs and rasters with several on per off even higher, but this
        # cannot be automatically determined due to lacking meta data. We
        # thus use a manually supplied scaling factor.
        integTimeSec = DataTable[row][8] * integ_time_fact
        # The Tsys value can be saved for DSB or SSB mode. A scaling factor
        # may be needed. This factor was read above.
        currentTsys = DataTable[row][14] * tsys_fact
        if ((noiseEquivBW * integTimeSec) > 0.0):
            expectedRMS = currentTsys / math.sqrt(noiseEquivBW * integTimeSec)
            # 2008/10/31
            # Comparison with both pre- and post-BaselineFit RMS
            PostFitRMS = DataTable[row][16][1]
            PreFitRMS = DataTable[row][16][2]
            print 'DEBUG_DM: Row: %d Expected RMS: %f PostFit RMS: %f PreFit RMS: %f' % (row, expectedRMS, PostFitRMS, PreFitRMS)
            DataTable[row][16][5] = expectedRMS * ThreExpectedRMSPostFit
            DataTable[row][16][6] = expectedRMS * ThreExpectedRMSPreFit
            if (PostFitRMS > ThreExpectedRMSPostFit * expectedRMS):
                DataTable[row][17][5] = 0
            else: DataTable[row][17][5] = 1
            if (PreFitRMS > ThreExpectedRMSPreFit * expectedRMS):
                DataTable[row][17][6] = 0
            else: DataTable[row][17][6] = 1
        nrow+=1


    # Plot result statistics
    FlaggedRowsCategory = [[],[],[],[],[],[],[],[],[]]
    FlaggedRows = []
    PermanentFlag = []
    pdata = [[],[],[],[],[],[],[]]
    pflag = [[],[],[],[],[],[],[]]
    prows = [[],[]]
    for row in rows:
        # Update User Flag 2008/6/4
        try:
            Index = UserFlag.index(row)
            DataTable[row][18][2] = 0
        except ValueError:
            DataTable[row][18][2] = 1
        # Check every flags to create summary flag
        Flag = 1
        if (DataTable[row][18][0] == 0 and SDC.SDParam['WeatherFlag']['isActive']) or \
           (DataTable[row][18][1] == 0 and SDC.SDParam['TsysFlag']['isActive']) or \
           (DataTable[row][18][2] == 0 and SDC.SDParam['UserFlag']['isActive']):
            Flag = 0
        PermanentFlag.append(Flag)
        if Flag == 0 or \
           (DataTable[row][17][1] == 0 and SDC.SDParam['RmsPostFitFlag']['isActive']) or \
           (DataTable[row][17][2] == 0 and SDC.SDParam['RmsPreFitFlag']['isActive']) or \
           (DataTable[row][17][3] == 0 and SDC.SDParam['RunMeanPostFitFlag']['isActive']) or \
           (DataTable[row][17][4] == 0 and SDC.SDParam['RunMeanPreFitFlag']['isActive']) or \
           (DataTable[row][17][5] == 0 and SDC.SDParam['RmsExpectedPostFitFlag']['isActive']) or \
           (DataTable[row][17][6] == 0 and SDC.SDParam['RmsExpectedPreFitFlag']['isActive']):
            DataTable[row][19] = 0
            FlaggedRows.append(row)
        else:
            DataTable[row][19] = 1
        # Tsys flag
        pdata[0].append(DataTable[row][14])
        pflag[0].append(DataTable[row][18][1])
        prows[0].append(row)
        Valid = True
        if DataTable[row][18][1] == 0:
            FlaggedRowsCategory[0].append(row)
            Valid = False
        # Weather flag
        if DataTable[row][18][0] == 0:
            FlaggedRowsCategory[1].append(row)
            Valid = False
        # User flag
        if DataTable[row][18][2] == 0:
            FlaggedRowsCategory[2].append(row)
            Valid = False
        
        #else:
        #if Valid:
        prows[1].append(row)
        # RMS flag before baseline fit
        pdata[1].append(DataTable[row][16][2])
        pflag[1].append(DataTable[row][17][2])
        if DataTable[row][17][2] == 0:
            FlaggedRowsCategory[4].append(row)
        # RMS flag after baseline fit
        pdata[2].append(DataTable[row][16][1])
        pflag[2].append(DataTable[row][17][1])
        if DataTable[row][17][1] == 0:
            FlaggedRowsCategory[3].append(row)
        # Running mean flag before baseline fit
        pdata[3].append(DataTable[row][16][4])
        pflag[3].append(DataTable[row][17][4])
        if DataTable[row][17][4] == 0:
            FlaggedRowsCategory[6].append(row)
        # Running mean flag after baseline fit
        pdata[4].append(DataTable[row][16][3])
        pflag[4].append(DataTable[row][17][3])
        if DataTable[row][17][3] == 0:
            FlaggedRowsCategory[5].append(row)
        # Expected RMS flag before baseline fit
        pdata[5].append(DataTable[row][16][6])
        pflag[5].append(DataTable[row][17][6])
        if DataTable[row][17][6] == 0:
            FlaggedRowsCategory[8].append(row)
        # Expected RMS flag after baseline fit
        pdata[6].append(DataTable[row][16][5])
        pflag[6].append(DataTable[row][17][5])
        if DataTable[row][17][5] == 0:
            FlaggedRowsCategory[7].append(row)

    # Tsys flag
    PlotData = {'row': prows[0], 'data': pdata[0], 'flag': pflag[0], \
                'thre': [threshold[4][1], 0.0], \
                'gap': [PosGap, TimeGap], \
			'title': "Tsys (K)\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % SDC.SDParam['TsysFlag']['Threshold'], \
                'xlabel': "row (spectrum)", \
                'ylabel': "Tsys (K)", \
                'permanentflag': PermanentFlag, \
                'isActive': SDC.SDParam['TsysFlag']['isActive'], \
                'threType': "line"}
    SDP.StatisticsPlot(PlotData, ShowPlot, FigFileDir, FigFileRoot+'_0')

    # RMS flag before baseline fit
    PlotData['row'] = prows[1]
    PlotData['data'] = pdata[1]
    PlotData['flag'] = pflag[1]
    PlotData['thre'] = [threshold[1][1]]
    PlotData['title'] = "Baseline RMS (K) before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % SDC.SDParam['RmsPreFitFlag']['Threshold']
    PlotData['ylabel'] = "Baseline RMS (K)"
    PlotData['isActive'] = SDC.SDParam['RmsPreFitFlag']['isActive']
    SDP.StatisticsPlot(PlotData, ShowPlot, FigFileDir, FigFileRoot+'_1')

    # RMS flag after baseline fit
    PlotData['data'] = pdata[2]
    PlotData['flag'] = pflag[2]
    PlotData['thre'] = [threshold[0][1]]
    PlotData['title'] = "Baseline RMS (K) after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % SDC.SDParam['RmsPostFitFlag']['Threshold']
    PlotData['isActive'] = SDC.SDParam['RmsPostFitFlag']['isActive']
    SDP.StatisticsPlot(PlotData, ShowPlot, FigFileDir, FigFileRoot+'_2')

    # Running mean flag before baseline fit
    PlotData['data'] = pdata[3]
    PlotData['flag'] = pflag[3]
    PlotData['thre'] = [threshold[3][1]]
    PlotData['title'] = "RMS (K) for Baseline Deviation from the running mean (Nmean=%d) before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % (SDC.SDParam['RunMeanPreFitFlag']['Nmean'], SDC.SDParam['RunMeanPreFitFlag']['Threshold'])
    PlotData['isActive'] = SDC.SDParam['RunMeanPreFitFlag']['isActive']
    SDP.StatisticsPlot(PlotData, ShowPlot, FigFileDir, FigFileRoot+'_3')

    # Running mean flag after baseline fit
    PlotData['data'] = pdata[4]
    PlotData['flag'] = pflag[4]
    PlotData['thre'] = [threshold[2][1]]
    PlotData['title'] = "RMS (K) for Baseline Deviation from the running mean (Nmean=%d) after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: %.1f sigma threshold, Red H-line(s): out of vertical scale limit(s)" % (SDC.SDParam['RunMeanPostFitFlag']['Nmean'], SDC.SDParam['RunMeanPostFitFlag']['Threshold'])
    PlotData['isActive'] = SDC.SDParam['RunMeanPostFitFlag']['isActive']
    SDP.StatisticsPlot(PlotData, ShowPlot, FigFileDir, FigFileRoot+'_4')

    # Expected RMS flag before baseline fit
    PlotData['data'] = pdata[1]
    PlotData['flag'] = pflag[5]
    PlotData['thre'] = [pdata[5]]
    PlotData['title'] = "Baseline RMS (K) compared with the expected RMS calculated from Tsys before baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: threshold with the scaling factor of %.1f, Red H-line(s): out of vertical scale limit(s)" % ThreExpectedRMSPreFit
    PlotData['isActive'] = SDC.SDParam['RmsExpectedPreFitFlag']['isActive']
    PlotData['threType'] = "plot"
    SDP.StatisticsPlot(PlotData, ShowPlot, FigFileDir, FigFileRoot+'_5')

    # Expected RMS flag after baseline fit
    PlotData['data'] = pdata[2]
    PlotData['flag'] = pflag[6]
    PlotData['thre'] = [pdata[6]]
    PlotData['title'] = "Baseline RMS (K) compared with the expected RMS calculated from Tsys after baseline subtraction\nBlue dots: data points, Red dots: deviator, Cyan H-line: threshold with the scaling factor of %.1f" % ThreExpectedRMSPostFit
    PlotData['isActive'] = SDC.SDParam['RmsExpectedPostFitFlag']['isActive']
    PlotData['threType'] = "plot"
    SDP.StatisticsPlot(PlotData, ShowPlot, FigFileDir, FigFileRoot+'_6')

    # Create Flagging Summary Page
    if FigFileDir != False:
        Filename = FigFileDir+FigFileRoot+'.html'
        if os.access(Filename, os.F_OK): os.remove(Filename)
        Out = open(Filename, 'w')
        print >> Out, '<html>\n<head>\n<style>'
        print >> Out, '.ttl{font-size:20px;font-weight:bold;}'
        print >> Out, '.stt{font-size:18px;font-weight:bold;color:white;background-color:navy;}'
        print >> Out, '.stp{font-size:18px;font-weight:bold;color:black;background-color:gray;}'
        print >> Out, '.stc{font-size:16px;font-weight:normal;}'
        print >> Out, '</style>\n</head>\n<body>'
        print >> Out, '<p class="ttl">Flagging Status</p>'
        print >> Out, '<table border="1">'
        print >> Out, '<tr align="center" class="stt"><th>&nbsp</th><th>isActive?</th><th>SigmaThreshold<th>Flagged spectra</th></tr>'
        print >> Out, '<tr align="center" class="stp"><th>%s</th><th>%s</th><th>%s</th><th>%s</th></tr>' % ('User', SDC.SDParam['UserFlag']['isActive'], SDC.SDParam['UserFlag']['Threshold'], len(FlaggedRowsCategory[2]))
        print >> Out, '<tr align="center" class="stp"><th>%s</th><th>%s</th><th>%s</th><th>%s</th></tr>' % ('Weather', SDC.SDParam['WeatherFlag']['isActive'], SDC.SDParam['WeatherFlag']['Threshold'], len(FlaggedRowsCategory[1]))
        print >> Out, '<tr align="center" class="stp"><th>%s</th><th>%s</th><th>%s</th><th>%s</th></tr>' % ('Tsys', SDC.SDParam['TsysFlag']['isActive'], SDC.SDParam['TsysFlag']['Threshold'], len(FlaggedRowsCategory[0]))
        print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th></tr>' % ('RMS baseline (pre-fit)', SDC.SDParam['RmsPreFitFlag']['isActive'], SDC.SDParam['RmsPreFitFlag']['Threshold'], len(FlaggedRowsCategory[4]))
        print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th></tr>' % ('RMS baseline (post-fit)', SDC.SDParam['RmsPostFitFlag']['isActive'], SDC.SDParam['RmsPostFitFlag']['Threshold'], len(FlaggedRowsCategory[3]))
        print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th></tr>' % ('Running Mean (pre-fit)', SDC.SDParam['RunMeanPreFitFlag']['isActive'], SDC.SDParam['RunMeanPreFitFlag']['Threshold'], len(FlaggedRowsCategory[6]))
        print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th></tr>' % ('Running Mean (post-fit)', SDC.SDParam['RunMeanPostFitFlag']['isActive'], SDC.SDParam['RunMeanPostFitFlag']['Threshold'], len(FlaggedRowsCategory[5]))
        print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th></tr>' % ('Expected RMS (pre-fit)', SDC.SDParam['RmsExpectedPreFitFlag']['isActive'], SDC.SDParam['RmsExpectedPreFitFlag']['Threshold'], len(FlaggedRowsCategory[8]))
        print >> Out, '<tr align="center" class="stc"><th>%s</th><th>%s</th><th>%s</th><th>%s</th></tr>' % ('Expected RMS (post-fit)', SDC.SDParam['RmsExpectedPostFitFlag']['isActive'], SDC.SDParam['RmsExpectedPostFitFlag']['Threshold'], len(FlaggedRowsCategory[7]))
        print >> Out, '<tr align="center" class="stt"><th>%s</th><th>%s</th><th>%s</th><th>%s</th></tr>' % ('Total Flagged', '-', '-', len(FlaggedRows))
        print >> Out, '<tr><td colspan=4>%s</td></tr>' % ("Note: flags in grey background are permanent, <br> which are not reverted or changed during the iteration cycles.") 
        print >> Out, '</table>\n</body>\n</html>'
        Out.close()

    EndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='End: %s, Elapsed time = %.1f sec' % (time.ctime(EndTime), (EndTime - StartTime)))


    if len(FlaggedRowsCategory[2]) > 0:
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Flagged rows by User =%s ' % FlaggedRowsCategory[2])
    if len(FlaggedRowsCategory[1]) > 0:
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Flagged rows by Weather =%s ' % FlaggedRowsCategory[1])
    if len(FlaggedRowsCategory[0]) > 0:
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Flagged rows by Tsys =%s ' % FlaggedRowsCategory[0])
    if len(FlaggedRowsCategory[4]) > 0:
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Flagged rows by the baseline fluctuation (pre-fit) =%s ' % FlaggedRowsCategory[4])
    if len(FlaggedRowsCategory[3]) > 0:
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Flagged rows by the baseline fluctuation (post-fit) =%s ' % FlaggedRowsCategory[3])
    if len(FlaggedRowsCategory[6]) > 0:
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Flagged rows by the difference from running mean (pre-fit) =%s ' % FlaggedRowsCategory[6])
    if len(FlaggedRowsCategory[5]) > 0:
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Flagged rows by the difference from running mean (post-fit) =%s ' % FlaggedRowsCategory[5])
    if len(FlaggedRowsCategory[8]) > 0:
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Flagged rows by the expected RMS (pre-fit) =%s ' % FlaggedRowsCategory[8])
    if len(FlaggedRowsCategory[7]) > 0:
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Flagged rows by the expected RMS (post-fit) =%s ' % FlaggedRowsCategory[7])
    if len(FlaggedRows) > 0:
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Final Flagged rows by all active categories =%s ' % FlaggedRows)

    ProcEndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Apply flags End: %s, Elapsed time = %.1f sec' % (time.ctime(ProcEndTime), (ProcEndTime - ProcStartTime)) )

    del tmpdata, threshold, pdata, pflag, PlotData, FlaggedRows, FlaggedRowsCategory

    return



def GroupForGrid(DataTable, rows, vIF, vPOL, CombineRadius, Allowance, GridSpacing, ObsPattern, LogLevel=2, LogFile=False):
    '''
    Gridding by RA/DEC position
    '''
    # Re-Gridding

    StartTime = time.time()

    NROW = len(rows)
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  GroupForGrid  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Start: %s' % time.ctime(StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='ObsPattern = %s' % ObsPattern)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d positions...' % NROW)

    GridTable = []
    RAs = []
    DECs = []
    RMSs = []
    ROWs = []
    # 2008/09/20 Spacing should be identical between RA and DEC direction
    DecCorrection = 1.0 / math.cos(DataTable[0][10] / 180.0 * 3.141592653)
    #### GridSpacingRA = GridSpacing * DecCorrection

    for row in rows:
        RAs.append(DataTable[row][9])
        DECs.append(DataTable[row][10])
        RMSs.append(DataTable[row][16][1])
        ROWs.append(DataTable[row][0])
    NpRAs = NP.array(RAs)
    NpDECs = NP.array(DECs)
    NpRMSs = NP.array(RMSs)
    NpROWs = NP.array(ROWs)

    if ObsPattern.upper() == 'RASTER':
        ThresholdR = CombineRadius * CombineRadius
        MinRA = min(RAs)
        MinDEC = min(DECs)
        #NGridRA = int((max(RAs) - MinRA) / GridSpacing) + 1
        NGridRA = int((max(RAs) - MinRA) / (GridSpacing * DecCorrection)) + 1
        NGridDEC = int((max(DECs) - MinDEC) / GridSpacing) + 1

    #    for y in range(NGridDEC):
    #        DEC = MinDEC + GridSpacing * y
    #        ##DeltaDEC = NA.array(DECs) - DEC
    #        DeltaDEC = NP.array(DECs) - DEC
    #        for x in range(NGridRA):
    #            RA = MinRA + GridSpacing * DecCorrection * x
    #            #RA = MinRA + GridSpacing * x
    #            ##DeltaRA = NA.array(RAs) - RA
    #            DeltaRA = (NP.array(RAs) - RA) / DecCorrection
    #            #DeltaRA = NP.array(RAs) - RA
    #            Delta = DeltaDEC * DeltaDEC + DeltaRA * DeltaRA
    #            ##Select = NA.less_equal(Delta, ThresholdR)
    #            Select = NP.less_equal(Delta, ThresholdR)
    #            line = [vIF, vPOL, x, y, RA, DEC, []]
    #            for i in range(len(rows)):
    #                if Select[i] == 1:
    #                    line[6].append([rows[i], math.sqrt(Delta[i]), RMSs[i]])
    #            GridTable.append(line)
    #            SDT.LogMessage('LONG', LogLevel, LogFile, Msg="GridTable: %s" % line)

        for y in range(NGridDEC):
            DEC = MinDEC + GridSpacing * y
            ##DeltaDEC = NA.array(DECs) - DEC
            #DeltaDEC = NP.array(DECs) - DEC
            DeltaDEC = NpDECs - DEC
            SelectD = NP.nonzero(NP.less_equal(DeltaDEC, CombineRadius) * NP.greater_equal(DeltaDEC, -CombineRadius))[0]
            sDeltaDEC = NP.take(DeltaDEC, SelectD)
            sRA = NP.take(NpRAs, SelectD)
            sROW = NP.take(NpROWs, SelectD)
            sRMS = NP.take(NpRMSs, SelectD)
            for x in range(NGridRA):
                RA = MinRA + GridSpacing * DecCorrection * x
                #RA = MinRA + GridSpacing * x
                ##DeltaRA = NA.array(RAs) - RA
                #DeltaRA = (NP.array(RAs) - RA) / DecCorrection
                sDeltaRA = (sRA - RA) / DecCorrection
                #DeltaRA = NP.array(RAs) - RA
                Delta = sDeltaDEC * sDeltaDEC + sDeltaRA * sDeltaRA
                ##Select = NA.less_equal(Delta, ThresholdR)
                Select = NP.less_equal(Delta, ThresholdR)
                line = [vIF, vPOL, x, y, RA, DEC, []]
                #for i in range(len(rows)):
                for i in range(len(sROW)):
                    if Select[i] == 1:
                        #line[6].append([rows[i], math.sqrt(Delta[i]), RMSs[i]])
                        line[6].append([sROW[i], math.sqrt(Delta[i]), sRMS[i]])
                GridTable.append(line)
                SDT.LogMessage('LONG', LogLevel, LogFile, Msg="GridTable: %s" % line)

    elif ObsPattern.upper() == 'SINGLE-POINT':
        NGridRA = 1
        NGridDEC = 1
        ##CenterRA = (NA.array(RAs)).mean()
        ##CenterDEC = (NA.array(DECs)).mean()
        CenterRA = (NP.array(RAs)).mean()
        CenterDEC = (NP.array(DECs)).mean()
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
        ##Flag = NA.ones(len(rows))
        Flag = NP.ones(len(rows))
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
                    ##CenterRA = (NA.array(RAtmp)).mean()
                    ##CenterDEC = (NA.array(DECtmp)).mean()
                    CenterRA = (NP.array(RAtmp)).mean()
                    CenterDEC = (NP.array(DECtmp)).mean()
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

    EndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='NGridRA = %s  NGridDEC = %s' % (NGridRA, NGridDEC))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='End: %s, Elapsed time = %.1f sec' % (time.ctime(EndTime), (EndTime - StartTime)) )
    return GridTable



def Process8(SpStorage, DataTable, GridTable, CombineRadius, weight='CONST', LogLevel=2, LogFile=False):

    # The process does re-map and combine spectrum for each position
    # GridTable format:
    #   [[IF,POL,0,0,RAcent,DECcent,[[row0,r0,RMS0],[row1,r1,RMS1],..,[rowN,rN,RMSN]]]
    #    [IF,POL,0,1,RAcent,DECcent,[[row0,r0,RMS0],[row1,r1,RMS1],..,[rowN,rN,RMSN]]]
    #                 ......
    #    [IF,POL,M,N,RAcent,DECcent,[[row0,r0,RMS0],[row1,r1,RMS1],..,[rowN,rN,RMSN]]]]
    #  where row0,row1,...,rowN should be combined to one for better S/N spectra
    #        'r' is a distance from grid position
    # 'weight' can be 'CONST', 'GAUSS', or 'LINEAR'
    # 'clip' can be 'none' or 'minmaxreject' 
    # 'rms_weight' is either True or False. If True, NewRMS is used for additional weight
    #   Number of spectra output is len(GridTable)
    # OutputTable format:
    #    [[IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp.]
    #     [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp.]
    #             ......
    #     [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp.]]

    import SDpipelineControl as SDC
    reload(SDC)
    clip = SDC.SDParam['Gridding']['Clipping']
    rms_weight = SDC.SDParam['Gridding']['WeightRMS']
    tsys_weight = SDC.SDParam['Gridding']['WeightTsysExptime']
    #weight = SDC.SDParam['Gridding']['WeightDistance']

    NROW = len(GridTable)
    StartTime = time.time()
    SDT.LogMessage('TITLE', LogLevel, LogFile, Msg='<'*15 + '  Process8  ' + '>'*15)
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Start: %s' % time.ctime(StartTime))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Accumulate nearby spectrum for each Grid position...')
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Processing %d spectra...' % (NROW))
    SDT.LogMessage('TIMER', LogLevel, LogFile, Msg='|' + '='*36 + ' 100% ' + '=' * 36 + '|')
    OutputTable = []
    NCHAN = len(SpStorage[0][0])
    ##StorageOut = NA.ones((NROW, NCHAN), type=NA.Float32) * NoData
    StorageOut = NP.ones((NROW, NCHAN), dtype=NP.float32) * NoData
    ID = 0
    STEP = HASH = HASH0 = 0

    for [IF, POL, X, Y, RAcent, DECcent, RowDelta] in GridTable:
        rowlist = []
        deltalist = []
        rmslist = []
        flagged = 0
        for [row, delta, rms] in RowDelta:
            # Check Summary Flag
            #if sum(DataTable[row][17][1:3]) > 1.5 and DataTable[row][17][5] > 0.5:
            if DataTable[row][19] == 1:
                rowlist.append(row)
                deltalist.append(delta)
                rmslist.append(rms)
            else: flagged += 1
        if len(rowlist) == 0:
            # No valid Spectra at the position
            RMS = 0.0
            pass
        elif len(rowlist) == 1:
            # One valid Spectrum at the position
            StorageOut[ID] = SpStorage[1][rowlist[0]]
            RMS = rmslist[0]
        else:
            # More than one valid Spectra at the position
            data = SpStorage[1][rowlist].copy()
            ##w = NA.ones(NA.shape(data), NA.Float64)
            w = NP.ones(NP.shape(data), NP.float64)
            ##weightlist = NA.ones(len(rowlist), NA.Float64)
            weightlist = NP.ones(len(rowlist), NP.float64)
            # Clipping
            if clip.upper() == 'MINMAXREJECT' and len(rowlist) > 2:
                ##w[NA.argmin(data, axis=0), range(len(data[0]))] = 0.0
                ##w[NA.argmax(data, axis=0), range(len(data[0]))] = 0.0
                w[NP.argmin(data, axis=0), range(len(data[0]))] = 0.0
                w[NP.argmax(data, axis=0), range(len(data[0]))] = 0.0
            # Weight by RMS
            # Weight = 1/(RMS**2)
            if rms_weight == True:
                for m in range(len(rowlist)):
                    if rmslist[m] != 0.0:
                        w[m] /= (rmslist[m]**2)
                        weightlist[m] /= (rmslist[m]**2)
                    else:
                        w[m] *= 0.0
                        weightlist[m] *= 0.0
            # Weight by Exptime & Tsys
            # RMS = n * Tsys/sqrt(Exptime)
            # Weight = 1/(RMS**2) = (Exptime/(Tsys**2))
            if tsys_weight == True:
                for m in range(len(rowlist)):
                    # 2008/9/21 Bug fix
                    if DataTable[rowlist[m]][14] > 0.5:
                        w[m] *= (DataTable[rowlist[m]][8]/(DataTable[rowlist[m]][14]**2))
                        weightlist[m] *= (DataTable[rowlist[m]][8]/(DataTable[rowlist[m]][14]**2))
                    #if DataTable[m][14] > 0.5:
                    #    w[m] *= (DataTable[m][8]/(DataTable[m][14]**2))
                    #    weightlist[m] *= (DataTable[m][8]/(DataTable[m][14]**2))
                    else:
                        w[m] *= 0.0
                        weightlist[m] = 0.0
            # Weight by radius
            if weight.upper() == 'GAUSS':
                # weight = exp(-ln2*((r/hwhm)**2))
                for m in range(len(rowlist)):
                    w[m] *= (math.exp(-0.69314718055994529*((deltalist[m]/CombineRadius)**2)))
                    weightlist[m] *= (math.exp(-0.69314718055994529*((deltalist[m]/CombineRadius)**2)))
            elif weight.upper() == 'LINEAR':
                # weight = 0.5 + (hwhm - r)/2/hwhm = 1.0 - r/2/hwhm
                for m in range(len(rowlist)):
                    w[m] *= (1.0 - deltalist[m]/2.0/CombineRadius)
                    weightlist[m] *= (1.0 - deltalist[m]/2.0/CombineRadius)
            # Combine Spectra
            ##if w.sum() != 0: StorageOut[ID] = (NA.sum(data * w) / NA.sum(w))
            if w.sum() != 0: StorageOut[ID] = (NP.sum(data * w, axis=0) / NP.sum(w,axis=0))
            # Calculate RMS of the spectrum
            r0 = 0.0
            r1 = 0.0
            for m in range(len(rowlist)):
                r0 += (rmslist[m] * weightlist[m]) ** 2
                r1 += weightlist[m]
            RMS = (r0**0.5) / r1
            
        OutputTable.append([IF, POL, X, Y, RAcent, DECcent, len(rowlist), flagged, RMS])
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
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='End: %s, Elapsed time = %.1f sec' % (time.ctime(EndTime), (EndTime - StartTime)))
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

    StartTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='CalcBaselineFit Start:%s' % time.ctime(StartTime))
    # Initialize plot for debugging
    if LogLevel > 3: SDP.DrawDebugFit(mode='INIT')

    ##data = NA.array(scan._getspectrum(0))
    data = NP.array(scan._getspectrum(0))
    # set edge mask
    (EdgeMaskL, EdgeMaskR) = edge
    NCHAN = scan.nchan()
    NCHANwoEdge = NCHAN - EdgeMaskL - EdgeMaskR
    if EdgeMaskL > 0: data[:EdgeMaskL] = 0.0
    if EdgeMaskR > 0: data[-EdgeMaskR:] = 0.0

    # Create mask for line protection
    ##mask = NA.ones(NCHAN)
    mask = NP.ones(NCHAN, dtype=int)
    if type(masklist) == list:
        for [m0, m1] in masklist:
            mask[m0:m1] = 0
    else: return False
    ##if EdgeMaskR > 0: Nmask = int(NCHANwoEdge - NA.sum(mask[EdgeMaskL:-EdgeMaskR] * 1.0))
    ##else: Nmask = int(NCHANwoEdge - NA.sum(mask[EdgeMaskL:] * 1.0))
    if EdgeMaskR > 0: Nmask = int(NCHANwoEdge - NP.sum(mask[EdgeMaskL:-EdgeMaskR] * 1.0))
    else: Nmask = int(NCHANwoEdge - NP.sum(mask[EdgeMaskL:] * 1.0))
    resultdata = []
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='NCHANwoEdge, Nmask, diff = %s, %s, %s' % (NCHANwoEdge, Nmask, NCHANwoEdge-Nmask))

    (fragment, nwindow, win_polyorder) = CalcFragmentation(polyorder, NCHANwoEdge - Nmask, 0)
    SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='fragment, nwindow, win_polyorder = %s, %s, %s' % (fragment, nwindow, win_polyorder))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Number of subdivided segments= %s' % nwindow)


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
            DeltaL = 1.0
        if WIN == (nwindow - 1):
            WIN_EDGE_IGNORE_R = 0
            PosR0 = NCHAN - EdgeMaskR
            PosR1 = NCHAN - EdgeMaskR
            DeltaR = 1.0
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='PosL0, PosL1, PosR0, PosR1 = %s, %s, %s, %s' % (PosL0, PosL1, PosR0, PosR1))
        NNmask = float((PosR1 - PosL0) - mask[PosL0:PosR1].sum())
        dorder = int(max(1, ((PosR1 - PosL0 - NNmask*0.5) * win_polyorder / (PosR1 - PosL0) + 0.5)))
        #dorder = int(max(1, (mask[PosL0:PosR1].sum() * win_polyorder / (PosR1 - PosL0) + 0.99)))
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg='Revised edgemask = %s:%s  Adjust polyorder = %s' % (EdgeL, EdgeR, dorder))
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Segment %d: Revised edgemask = %s:%s  Adjust polyorder used in individual fit= %s' % (WIN, EdgeL, EdgeR, dorder))
        FitStartTime = time.time()
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Fitting Start: %s' % time.ctime(FitStartTime))

        # Shift scan and mask to set unmasked region starting at zero
        # Shift size is EdgeL: EdgeL -> 0
        if POLYNOMIAL_FIT == 'ASAP':
            print "start ASAP fitting..."
            edgemask = scan.create_mask([0, EdgeR - EdgeL])
            tmpscan = scan.copy()
            ##tmpscan._setspectrum(NA.concatenate((data[EdgeL:], NA.zeros(EdgeL))))
            ##tmpmask = NA.concatenate((mask[EdgeL:], NA.zeros(EdgeL)))
            tmpscan._setspectrum(NP.concatenate((data[EdgeL:], NP.zeros(EdgeL))))
            tmpmask = NP.concatenate((mask[EdgeL:], NP.zeros(EdgeL, dtype=int)))
            # use linear fit 
            ##tmpfit = NA.array(tmpscan.poly_baseline(order=dorder, mask=(tmpmask & edgemask),uselin=True, insitu=False)._getspectrum(0))[:EdgeR - EdgeL]
            tmpfit0 = tmpscan.poly_baseline(order=dorder, mask=(tmpmask & edgemask),uselin=True, insitu=False)._getspectrum(0)
            tmpfit = NP.array(tmpfit0)[:EdgeR - EdgeL]
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
        ##resultdata.append(list(NA.concatenate((NA.zeros(EdgeL), tmpfit, NA.zeros(NCHAN - EdgeR)))))
        resultdata.append(list(NP.concatenate((NP.zeros(EdgeL), tmpfit, NP.zeros(NCHAN - EdgeR)))))
        del tmpfit
        FitEndTime = time.time()
        SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Fitting End: %s (Elapsed Time= %.1f)' % (time.ctime(FitEndTime),FitEndTime-FitStartTime) )

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
    ##outdata = NA.sum(resultdata)
    outdata = NP.sum(resultdata, axis=0)

    ProcStartTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='RMS before/after fitting calculation Start: %s' % time.ctime(ProcStartTime) )
    # Calculate RMS after/before fitting
    MaskedData = outdata * mask
    ##StddevMasked = MaskedData.stddev()
    StddevMasked = MaskedData.std()
    MeanMasked = MaskedData.mean()
    NewRMS = math.sqrt(NCHANwoEdge * StddevMasked ** 2 / (NCHANwoEdge - Nmask) - \
                    NCHANwoEdge * Nmask * MeanMasked ** 2 / ((NCHANwoEdge - Nmask) ** 2))
    MaskedData = data * mask
    ##StddevMasked = MaskedData.stddev()
    StddevMasked = MaskedData.std()
    MeanMasked = MaskedData.mean()

    OldRMS = math.sqrt(abs(NCHANwoEdge * StddevMasked ** 2 / (NCHANwoEdge - Nmask) - \
                    NCHANwoEdge * Nmask * MeanMasked ** 2 / ((NCHANwoEdge - Nmask) ** 2)))
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='Pre-fit RMS= %.2f, Post-fit RMS= %.2f' % (OldRMS, NewRMS) )
    ProcEndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='RMS before/after fitting calculation End: %s (Elapsed time = %.1f sec)' % (time.ctime(ProcEndTime), ProcEndTime - ProcStartTime) )

    # Plot for verification use only : START
    if LogLevel > 3:
        ##plotdata = data - NA.sum(resultdata)
        plotdata = data - NP.sum(resultdata)
        Ymin = min(plotdata) - (max(plotdata) - min(plotdata)) * 0.15
        Ymax = max(plotdata) + (max(plotdata) - min(plotdata)) * 0.15
        SDP.DrawDebugFit('SECOND', range(NCHAN), plotdata, Ymin, Ymax)
    # Plot for verification : END
    EndTime = time.time()
    SDT.LogMessage('INFO', LogLevel, LogFile, Msg='CalcBaselineFit End: %s (Elapsed time = %.1f sec)' % (time.ctime(EndTime), (EndTime - StartTime)) )

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
    win_polyorder = min(int(polyorder / fragment) + fragment - 1, MAX_FREQ)

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
    ##abscissa = NA.array([x for x in xrange(len(data))], NA.Float64)
    abscissa = NP.array([x for x in xrange(len(data))], NP.float64)
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
        ##xlist = NA.zeros(order * 2 + 1, NA.Float64)
        ##ymatrix = NA.zeros(order + 1, NA.Float64)
        xlist = NP.zeros(order * 2 + 1, NP.float64)
        ymatrix = NP.zeros(order + 1, NP.float64)
        yy = 0.0
        # Calculate Summation
        ##for [x, y, f] in NA.transpose(NA.concatenate([[abscissa], \
        for [x, y, f] in NP.transpose(NP.concatenate([[abscissa], \
                [data], [newflag]])):
            if f:
                xlist += [x**k for k in xrange(order * 2 + 1)]
                ymatrix += [y*x**k for k in xrange(order + 1)]
                yy += y*y
        # Make Summation matrix
        ##xmatrix = NA.array([xlist[k:k+order+1] for k in xrange(order+1)])
        xmatrix = NP.array([xlist[k:k+order+1] for k in xrange(order+1)])
        # Calculate Coefficients and Standard Deviation
        ##coeff = NA.add.reduce(LA.inverse(xmatrix) * ymatrix, axis=1)
        coeff = NP.add.reduce(LA.inv(xmatrix) * ymatrix, axis=1)

        ##sigma = math.sqrt((yy - 2.0 * NA.sum(ymatrix * coeff)
        ##    + NA.sum(NA.add.reduce(xmatrix * coeff, axis=1) * coeff))
        ##    / (xmatrix[0][0] - 1.0))
        sigma = math.sqrt((yy - 2.0 * NP.sum(ymatrix * coeff)
            + NP.sum(NP.add.reduce(xmatrix * coeff, axis=1) * coeff))
            / (xmatrix[0][0] - 1.0))
        # Count newly rejected channels
        reject = len(newflag) - sum(newflag)
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg="iteration = %s/%s" % (niter + 1, iter))
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg="sigma = %s" % sigma)
        SDT.LogMessage('DEBUG', LogLevel, LogFile, Msg="reject(org) = %s/%s (%s)" % (reject - reject_org, len(newflag), reject_org))
        # Calculate Fitting profile
        ##fitting = NA.array([NA.sum((NA.array([x**k for k in \
        fitting = NP.array([NP.sum((NP.array([x**k for k in \
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
        ##for [f,y,pos] in NA.transpose(NA.concatenate([[fitting], \
        ##        [data], [NA.arange(len(data))]])):
        for [f,y,pos] in NP.transpose(NP.concatenate([[fitting], \
                [data], [NP.arange(len(data))]])):
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
    ##abscissa = NA.array([x for x in xrange(len(data))], NA.Float64)
    abscissa = NP.array([x for x in xrange(len(data))], NP.float64)

    # Initialize arrays to store summation
    ##xlist = NA.zeros(order * 2 + 1, NA.Float64)
    ##ymatrix = NA.zeros(order + 1, NA.Float64)
    xlist = NP.zeros(order * 2 + 1, NP.float64)
    ymatrix = NP.zeros(order + 1, NP.float64)
    yy = 0.0
    # Calculate Summation
    ##for [x, y, f] in NA.transpose(NA.concatenate([[abscissa], [data], [mask]])):
    for [x, y, f] in NP.transpose(NP.concatenate([[abscissa], [data], [mask]])):
        if f:
            xlist += [x**k for k in xrange(order * 2 + 1)]
            ymatrix += [y*x**k for k in xrange(order + 1)]
            yy += y*y
    # Make Summation matrix
    ##xmatrix = NA.array([xlist[k:k+order+1] for k in xrange(order+1)])
    xmatrix = NP.array([xlist[k:k+order+1] for k in xrange(order+1)])
    # Calculate Coefficients and Standard Deviation
    ##coeff = NA.add.reduce(LA.inverse(xmatrix) * ymatrix, axis=1)
    coeff = NP.add.reduce(LA.inv(xmatrix) * ymatrix, axis=1)

    # Calculate Fitting profile
    ##fitting = NA.array([NA.sum((NA.array([x**k for k in \
    fitting = NP.array([NP.sum((NP.array([x**k for k in \
            xrange(order + 1)]) * coeff)) for x in abscissa])

    # Subtract the fit from original?
    if(subtract == True): 
        NewData = data - fitting
    else: NewData = fitting.copy()

    del fitting, xmatrix, ymatrix, xlist, coeff, abscissa
    return NewData



