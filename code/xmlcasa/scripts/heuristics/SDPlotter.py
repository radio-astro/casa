from asap import *
#import numarray as NA
import numpy as NP
import pylab as PL
import math
import os
import casac
from matplotlib.ticker import FuncFormatter, MultipleLocator, AutoLocator
from matplotlib.font_manager import FontProperties 


# 0:DebugPlot 1:TPlotRADEC 2:TPlotAzEl 3:TPlotCluster 4:TplotFit 5:TPlotMultiSP 6:TPlotSparseSP 7:TPlotChannelMap 8:TPlotFlag 9:TPlotIntermediate
MATPLOTLIB_FIGURE_ID = [8904, 8905, 8906, 8907, 8908, 8909, 8910, 8911, 8912, 8913]

MAX_NhPanel = 5
MAX_NvPanel = 5
MAX_NhPanelFit = 3
MAX_NvPanelFit = 5
#NChannelMap = 12
NChannelMap = 15

#DPIDetail = 120
DPIDetail = 130
DPISummary = 90
RArotation = 90
DECrotation = 0

NoData = -32767.0
NoDataThreshold = NoData + 10000.0


def Deg2HMS(x, arrowance):
    # Transform degree to HHMMSS.sss format
    xx = x % 360 + arrowance
    h = int(xx / 15)
    m = int((xx % 15) * 4)
    s = ((xx % 15) * 4 - m) * 60.0
    return (h, m, s)

def HHMM(x, pos):
    # HHMM format
    (h, m, s) = Deg2HMS(x, 1/40.0)
    return '%02dh%02dm' % (h, m)

def HHMMSS(x, pos):
    # HHMMSS format
    (h, m, s) = Deg2HMS(x, 1/2400.0)
    return '%02dh%02dm%02ds' % (h, m, s)

def HHMMSSs(x, pos):
    # HHMMSS.s format
    (h, m, s) = Deg2HMS(x, 1/24000.0)
    return '%02dh%02dm%04.1fs' % (h, m, s)

def HHMMSSss(x, pos):
    # HHMMSS.ss format
    (h, m, s) = Deg2HMS(x, 1/240000.0)
    return '%02dh%02dm%05.2fs' % (h, m, s)

def HHMMSSsss(x, pos):
    # HHMMSS.sss format
    (h, m, s) = Deg2HMS(x, 1/2400000.0)
    return '%02dh%02dm%06.3fs' % (h, m, s)


def Deg2DMS(x, arrowance):
    # Transform degree to +ddmmss.ss format
    xxx = (x + 90) % 180 - 90
    xx = abs(xxx) + arrowance
    if xxx < 0: sign = -1
    else: sign = 1
    d = int(xx * sign)
    m = int((xx % 1) * 60)
    s = ((xx % 1) * 60 - m) * 60.0
    return (d, m, s)

def DDMM(x, pos):
    # +DDMM format
    (d, m, s) = Deg2DMS(x, 1/600.0)
    return '%+02dd%02dm' % (d, m)

def DDMMSS(x, pos):
    # +DDMMSS format
    (d, m, s) = Deg2DMS(x, 1/36000.0)
    return '%+02dd%02dm%02ds' % (d, m, s)

def DDMMSSs(x, pos):
    # +DDMMSS.s format
    (d, m, s) = Deg2DMS(x, 1/360000.0)
    return '%+02dd%02dm%04.1fs' % (d, m, s)

def DDMMSSss(x, pos):
    # +DDMMSS.ss format
    (d, m, s) = Deg2DMS(x, 1/3600000.0)
    return '%+02dd%02dm%05.2fs' % (d, m, s)

def RADEClabel(span):
    """
    return (RAlocator, DEClocator, RAformatter, DECformatter)
    """
    RAtick = [15.0, 5.0, 2.5, 1.25, 1/2.0, 1/4.0, 1/12.0, 1/24.0, 1/48.0, 1/120.0, 1/240.0, 1/480.0, 1/1200.0, 1/2400.0, 1/4800.0, 1/12000.0, 1/24000.0, 1/48000.0, -1.0]
    DECtick = [20.0, 10.0, 5.0, 2.0, 1.0, 1/3.0, 1/6.0, 1/12.0, 1/30.0, 1/60.0, 1/180.0, 1/360.0, 1/720.0, 1/1800.0, 1/3600.0, 1/7200.0, 1/18000.0, 1/36000.0, -1.0]
    for RAt in RAtick:
        if span > (RAt * 3.0) and RAt > 0:
            RAlocator = MultipleLocator(RAt)
            break
    #if RAt < 0: RAlocator = MultipleLocator(1/96000.)
    if RAt < 0: RAlocator = AutoLocator()
    for DECt in DECtick:
        if span > (DECt * 3.0) and DECt > 0:
            DEClocator = MultipleLocator(DECt)
            break
    #if DECt < 0: DEClocator = MultipleLocator(1/72000.0)
    if DECt < 0: DEClocator = AutoLocator()
            
    if span < 0.0001:
        RAformatter=FuncFormatter(HHMMSSsss)
        DECformatter=FuncFormatter(DDMMSSss)
    elif span < 0.001:
        RAformatter=FuncFormatter(HHMMSSss)
        DECformatter=FuncFormatter(DDMMSSs)
    elif span < 0.01:
        RAformatter=FuncFormatter(HHMMSSs)
        DECformatter=FuncFormatter(DDMMSS)
    elif span < 1.0:
        RAformatter=FuncFormatter(HHMMSS)
        #DECformatter=FuncFormatter(DDMM)
        DECformatter=FuncFormatter(DDMMSS)
    else:
        RAformatter=FuncFormatter(HHMM)
        DECformatter=FuncFormatter(DDMM)

    return (RAlocator, DEClocator, RAformatter, DECformatter)


def ShowResult(mode='init', DataTable=[], rows=[], TimeGap=[], PosGap=[], threshold=[], Nsigma=[], Nmean=5, title='Statistics Plot', ShowPlot=True, FigFileDir=False, FigFileRoot=False):

    # mode = 'all'/'init'/'plot' : case sensitive
    # if mode == 'online':
    #     plotdata: [rows[], NewRMS[], OldRMS[]] => DataTable[0],[16][1],[16][2]
    # if mode == 'final':
    #     init + plot
    #     plotdata: [rows[], NewRMS[], OldRMS[], NewRMSdiff[], OldRMSdiff[], flag[], flag[], flag[], flag[], Nmask[]] => DataTable[0],[16][1],[16][2],[16][3],[16][4],[17][1-4]
    # if mode == 'init':
    #     setup frames
    # threshold is necessary for mode='final'

    # Currently flag by deviation from running mean is not activated so 'Diff' plots 
    # are not shown (ShowDiffPlot=False)  - 2007Nov20 TT
    ShowDiffPlot = False

    if ShowPlot == False and FigFileDir == False: return
    if ShowPlot: PL.ion()
    PL.figure(MATPLOTLIB_FIGURE_ID[9])
    if ShowPlot: PL.ioff()
    Mark = 'bo'
    Plot = []
    
    if (mode.upper() == 'FINAL') or (mode.upper() == 'INIT'):
        PL.cla()
        PL.clf()
        if ShowDiffPlot:
            Plot.append(PL.subplot(311))
        else:
            Plot.append(PL.subplot(211))
            
        PL.subplots_adjust(hspace=0.3)
        PL.ylabel('Baseline RMS (K)', size=7)
        PL.title(title, size=7)
        PL.xticks(size=6)
        PL.yticks(size=6)
        if ShowDiffPlot:
            Plot.append(PL.subplot(313))
        else:
            Plot.append(PL.subplot(212))
        PL.xlabel('row (spectrum)\n\nGreen V-lines represent where larger pointing gap exist; Cyan V-lines represent where larger time gap exist\nPoints outside Red H-line represent out of scale', size=6)
        PL.ylabel('Tsys (K)', size=7)
        PL.title('Tsys (K)', size=6)
        PL.xticks(size=6)
        PL.yticks(size=6)
    if mode.upper() == 'INIT': return Plot

    # Collect plotdata from DataTable
    ##plotdata = NA.zeros((11, len(rows)), type=NA.Float32)
    plotdata = NP.zeros((11, len(rows)), dtype=NP.float32)
    # plotdata: row, NewRMS, OldRMS, NewDiffRMS, OldDiffRMS, Tsys, flag1, flag2, flag3, flag4, flagTsys
    #             0       1       2           3           4     5      6      7      8      9        10
    for x in range(len(rows)):
        plotdata[0][x] = DataTable[rows[x]][0]
        for y in range(4):
            plotdata[y + 1][x] = DataTable[rows[x]][16][y + 1]
        plotdata[5][x] = DataTable[rows[x]][14]
        for y in range(5):
            plotdata[y + 6][x] = DataTable[rows[x]][17][y + 1]

    if mode.upper() == 'ONLINE':
        if ShowPlot == False: return
        if ShowDiffPlot:
            PL.subplot(311)
        else:
            PL.subplot(211)

        PL.plot(plotdata[0], plotdata[2], Mark, markersize=3, markeredgecolor='b', markerfacecolor='b')
        PL.plot(plotdata[0], plotdata[1], Mark, markersize=3, markeredgecolor='c', markerfacecolor='c')
        PL.xticks(size=5)
        PL.yticks(size=5)
        for row in PosGap:
            if row in plotdata[0]:
                PL.axvline(x=row, linewidth=0.5, color='g', ymin=0.9)
        for row in TimeGap:
            if row in plotdata[0]:
                PL.axvline(x=row, linewidth=0.5, color='c', ymin=0.8, ymax=0.9)

        if ShowDiffPlot:
            PL.subplot(313)
        else:
            PL.subplot(212)
        PL.plot(plotdata[0], plotdata[5], Mark, markersize=3, markeredgecolor='b', markerfacecolor='b')
        PL.xticks(size=5)
        PL.yticks(size=5)

        PL.draw()
        del plotdata
        return

    if mode.upper() == 'FINAL':
        if ShowDiffPlot:
	    Plot.append(PL.subplot(312))
	    PL.ylabel('Baseline RMS (K)', size=7)
            PL.title('RMS (K) for Baseline Deviation from the running mean (Nmean=%d)\nBlue dots: raw data, Blue with Red circle: deviator, Blue H-line %.1f sigma threshold;  Cyan dots: baselined data, Cyan with Red circle: deviator, Cyan H-line :%.1f sigma threshold' % (Nmean, Nsigma[2], Nsigma[3]), size=6)
	    PL.xticks(size=6)
	    PL.yticks(size=6)


	if FigFileDir != False:
	    if os.access(FigFileDir+'listofplots.txt', os.F_OK):
		BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
	    else:
		BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
		print >> BrowserFile, 'TITLE: BF_Stat'
		print >> BrowserFile, 'FIELDS: Stat IF POL Iteration Page'
		print >> BrowserFile, 'COMMENT: Statistics of spectra'
	    print >> BrowserFile, FigFileRoot+'_0.png' 
	    print >> BrowserFile, FigFileRoot+'_1.png' 
	    #print >> BrowserFile, FigFileRoot+'_0.gif'
	    #print >> BrowserFile, FigFileRoot+'_1.gif'
	    BrowserFile.close()

	colordata = []
	for z in range(10):
	    colordata.append([])
	    for y in range(2): colordata[z].append([])
	ymax0 = max(threshold[0], threshold[1]) * 1.5
	ymax1 = max(threshold[2], threshold[3]) * 1.5
	ym2 = threshold[4] - threshold[5]
	ymax2 = threshold[4] * 1.5
	ymin2 = threshold[5] - ym2 * 0.1
	ScaleOutVal = [ymax0 * 0.96, ymax0 * 0.925, ymax1 * 0.96, ymax1 * 0.925, ymax2 * 0.96, ymin2 / 2.0]
	ScaleOutThre= [ymax0 * 0.9, ymax0 * 0.9, ymax1 * 0.9, ymax1 * 0.9, ymax2 * 0.925, 0.0]
	#ymax2 = threshold[4] + ym2 * 0.5
	#ymin2 = threshold[5] - ym2 * 0.5
	#ScaleOutVal = [ymax0 * 0.96, ymax0 * 0.925, ymax1 * 0.96, ymax1 * 0.925, ymax2-ym2*0.05, ymin2+ym2*0.05]
	#ScaleOutThre= [ymax0 * 0.9, ymax0 * 0.9, ymax1 * 0.9, ymax1 * 0.9, ymax2-ym2*0.1, ymin2+ym2*0.1]
	# Color the data by flag
        # *** Note added by TT ***
        # colordata[][0] data with flag=good; colordata[][1] data with flag = bad, set the values to 
        # ScaleOutVal if the value > ScaleOutVal 
        # colordata: row, NewRMS, row, OldRMS, row, Newdiff, row, Olddiff, row, Tsys  
        #              0      1     2    3       4     5       6     7      8    9 
	for x in range(len(rows)):
	    for y in range(4):
		if plotdata[y+6][x] == 1:
		    colordata[y*2][0].append(plotdata[0][x])
		    colordata[y*2+1][0].append(plotdata[y+1][x])
		else:
		    colordata[y*2][1].append(plotdata[0][x])
		    if plotdata[y+1][x] > ScaleOutThre[y]:
			colordata[y*2+1][1].append(ScaleOutVal[y])
		    else:
			colordata[y*2+1][1].append(plotdata[y+1][x])
	    if plotdata[10][x] == 1:
		colordata[8][0].append(plotdata[0][x])
		colordata[9][0].append(plotdata[5][x])
	    else:
		colordata[8][1].append(plotdata[0][x])
		if plotdata[5][x] > ScaleOutThre[4]:
		    colordata[9][1].append(ScaleOutVal[4])
		elif plotdata[5][x] < ScaleOutThre[5]:
		    colordata[9][1].append(ScaleOutVal[5])
		else:
		    colordata[9][1].append(plotdata[5][x])

        if ShowDiffPlot:
	    PL.subplot(311)
        else:
	    PL.subplot(211)
	PL.title(title+'\n\nBaseline RMS (K)\nBlue dots: raw data, Blue with Red circle: deviator, Blue H-line: %.1f sigma threshold;  Cyan dots: baselined data, Cyan with Red circle: deviator, Cyan H-line: %.1f sigma threshold' % (Nsigma[0], Nsigma[1]), size=6)
	PL.axhline(y=threshold[0], linewidth=1, color='b')
	PL.axhline(y=threshold[1], linewidth=1, color='c')
        PL.axhline(y=ScaleOutThre[0], linewidth=1, color='r')
        for row in PosGap:
            if row in plotdata[0]:
                PL.axvline(x=row, linewidth=0.5, color='g', ymin=0.95)
        for row in TimeGap:
            PL.axvline(x=row, linewidth=0.5, color='c', ymin=0.9, ymax=0.95)
        PL.plot(colordata[2][0], colordata[3][0], Mark, markersize=3, markeredgecolor='b', markerfacecolor='b')
        PL.plot(colordata[2][1], colordata[3][1], Mark, markersize=3, markeredgecolor='r', markerfacecolor='b')
        PL.plot(colordata[0][0], colordata[1][0], Mark, markersize=3, markeredgecolor='c', markerfacecolor='c')
        PL.plot(colordata[0][1], colordata[1][1], Mark, markersize=3, markeredgecolor='r', markerfacecolor='c')
        PL.xticks(size=5)
        PL.yticks(size=5)
        [xmin, xmax, ymin, ymax] = PL.axis()
        PL.axis([xmin, xmax, ymin, ymax0])

        if ShowDiffPlot:  
            PL.subplot(312)
	    PL.axhline(y=threshold[2], linewidth=1, color='b')
	    PL.axhline(y=threshold[3], linewidth=1, color='c')
	    PL.axhline(y=ScaleOutThre[2], linewidth=1, color='r')
	    for row in PosGap:
		if row in plotdata[0]:
		    PL.axvline(x=row, linewidth=0.5, color='g', ymin=0.95)
	    for row in TimeGap:
		PL.axvline(x=row, linewidth=0.5, color='c', ymin=0.9, ymax=0.95)
	    PL.plot(colordata[6][0], colordata[7][0], Mark, markersize=3, markeredgecolor='b', markerfacecolor='b')
	    PL.plot(colordata[6][1], colordata[7][1], Mark, markersize=3, markeredgecolor='r', markerfacecolor='b')
	    PL.plot(colordata[4][0], colordata[5][0], Mark, markersize=3, markeredgecolor='c', markerfacecolor='c')
	    PL.plot(colordata[4][1], colordata[5][1], Mark, markersize=3, markeredgecolor='r', markerfacecolor='c')
	    PL.xticks(size=5)
	    PL.yticks(size=5)
	    [xmin, xmax, ymin, ymax] = PL.axis()
            PL.axis([xmin, xmax, ymin, ymax1])

        if ShowDiffPlot:  
            PL.subplot(313)
        else:
            PL.subplot(212)
        # Omit the Tsys threshold line 
        #PL.title('Tsys (K)\nBlue with Red circle shows deviator; Blue H-lines %.1f sigma threshold' % (Nsigma[4]), size=6)
        PL.title('Tsys (K)\nBlue with Red circle shows deviator', size=6)
        #PL.axhline(y=threshold[4], linewidth=1, color='b')
        #PL.axhline(y=threshold[5], linewidth=1, color='b')
        PL.axhline(y=ScaleOutThre[4], linewidth=1, color='r')
        PL.axhline(y=ScaleOutThre[5], linewidth=1, color='r')
        PL.plot(colordata[8][0], colordata[9][0], Mark, markersize=3, markeredgecolor='b', markerfacecolor='b')
        PL.plot(colordata[8][1], colordata[9][1], Mark, markersize=3, markeredgecolor='r', markerfacecolor='b')
        PL.xticks(size=5)
        PL.yticks(size=5)
        [xmin, xmax, ymin, ymax] = PL.axis()
        PL.axis([xmin, xmax, ymin2, ymax2])

        for row in PosGap:
            if row in plotdata[0]:
                PL.axvline(x=row, linewidth=0.5, color='g', ymin=0.95)
        for row in TimeGap:
            PL.axvline(x=row, linewidth=0.5, color='c', ymin=0.9, ymax=0.95)

        if FigFileDir != False:
            PL.savefig(FigFileDir+FigFileRoot+'_0.png', format='png', dpi=DPIDetail)
            #os.rename(FigFileDir+FigFileRoot+'_0.png', FigFileDir+FigFileRoot+'_0.gif')

            #colordata: colordata[i][0] all flags are =1 (good); 
            #           colordata[i][1] at least one of the flags =0 (bad) 
            #           colordata[i][2] 2 or more flags 
            #           i=0 - RMSreduced, 1 - RMSoriginal, ... 
            colordata = []
            for z in range(4):
                colordata.append([])
                for y in range(3): colordata[z].append([])
            # Seems to me the following original lines (with ##) have bugs
            # flags for RMS and diffRMS are stored in plotdata[6][] through plodata[9][]
            for x in range(len(plotdata[0])):
                ##if (plotdata[5][x]+plotdata[6][x]+plotdata[7][x]+plotdata[8][x]) < 3:
                #if (plotdata[6][x]+plotdata[7][x]+plotdata[8][x]+plotdata[9][x]) < 3:
                # -both NewRMS and OldRMS are flagged bad
                if (plotdata[6][x]+plotdata[7][x]) < 1:
                    for y in range(4):
                        colordata[y][2].append(plotdata[y+1][x])
                ##elif (plotdata[5][x]*plotdata[6][x]*plotdata[7][x]*plotdata[8][x]) == 0:
                #elif (plotdata[6][x]*plotdata[7][x]*plotdata[8][x]*plotdata[9][x]) == 0:
                # But we currently turning off running mean based flags. So instead use the following line.
                # -one of the RMSs are flagged bad 
                elif (plotdata[6][x]*plotdata[7][x]) == 0:
                    for y in range(4):
                        colordata[y][1].append(plotdata[y+1][x])
                else:
                    for y in range(4):
                        colordata[y][0].append(plotdata[y+1][x])
            PL.cla()
            PL.clf()
            Plot.append(PL.subplot(221))
            PL.xlabel('RMS_original', size=7)
            PL.ylabel('RMS_reduced', size=7)
            PL.title('RMS reduced v.s. RMS original', size=7)
            PL.xticks(size=7)
            PL.yticks(size=7)
            Plot.append(PL.subplot(222))
            PL.xlabel('RMS_original', size=7)
            PL.ylabel('RMS_reduced', size=7)
            PL.title('RMS reduced v.s. RMS original (Zoom)', size=7)
            PL.xticks(size=7)
            PL.yticks(size=7)
            if ShowDiffPlot:
		Plot.append(PL.subplot(223))
		PL.xlabel('Diff_original', size=7)
		PL.ylabel('Diff_reduced ', size=7)
		PL.title('Diff reduced v.s. Diff original', size=7)
		PL.xticks(size=7)
		PL.yticks(size=7)
		Plot.append(PL.subplot(224))
		PL.xlabel('Diff_original', size=7)
		PL.ylabel('Diff_reduced ', size=7)
		PL.title('Diff reduced v.s. Diff original (Zoom)', size=7)
		PL.xticks(size=7)
		PL.yticks(size=7)

            PL.subplot(221)
            PL.plot(colordata[1][0], colordata[0][0], Mark, markersize=3, markeredgecolor='b', markerfacecolor='b')
            PL.plot(colordata[1][1], colordata[0][1], Mark, markersize=3, markeredgecolor='r', markerfacecolor='b')
            PL.plot(colordata[1][2], colordata[0][2], Mark, markersize=3, markeredgecolor='r', markerfacecolor='r')
            PL.axvline(x=threshold[0], linewidth=1, color='b')
            PL.axhline(y=threshold[1], linewidth=1, color='b')
            PL.xticks(size=6)
            PL.yticks(size=6)

            PL.subplot(222)
            PL.plot(colordata[1][0], colordata[0][0], Mark, markersize=3, markeredgecolor='b', markerfacecolor='b')
            PL.plot(colordata[1][1], colordata[0][1], Mark, markersize=3, markeredgecolor='r', markerfacecolor='b')
            PL.plot(colordata[1][2], colordata[0][2], Mark, markersize=3, markeredgecolor='r', markerfacecolor='r')
            PL.axvline(x=threshold[0], linewidth=1, color='b')
            #PL.axvline(x=threshold[6], linewidth=1, color='b')
            PL.axhline(y=threshold[1], linewidth=1, color='b')
            #PL.axhline(y=threshold[7], linewidth=1, color='b')
            minX = plotdata[2].min()
            minY = plotdata[1].min()
            marginX = (threshold[0] - minX) * 0.1
            marginY = (threshold[1] - minY) * 0.1
            PL.axis([minX - marginX, threshold[0] + marginX * 3.0, minY - marginY, threshold[1] + marginY * 3.0])
            PL.xticks(size=6)
            PL.yticks(size=6)

            if ShowDiffPlot:
		PL.subplot(223)
		PL.plot(colordata[3][0], colordata[2][0], Mark, markersize=3, markeredgecolor='b', markerfacecolor='b')
		PL.plot(colordata[3][1], colordata[2][1], Mark, markersize=3, markeredgecolor='r', markerfacecolor='b')
		PL.plot(colordata[3][2], colordata[2][2], Mark, markersize=3, markeredgecolor='r', markerfacecolor='r')
		PL.axvline(x=threshold[2], linewidth=1, color='b')
		PL.axhline(y=threshold[3], linewidth=1, color='b')
		PL.xticks(size=6)
		PL.yticks(size=6)

		PL.subplot(224)
		PL.plot(colordata[3][0], colordata[2][0], Mark, markersize=3, markeredgecolor='b', markerfacecolor='b')
		PL.plot(colordata[3][1], colordata[2][1], Mark, markersize=3, markeredgecolor='r', markerfacecolor='b')
		PL.plot(colordata[3][2], colordata[2][2], Mark, markersize=3, markeredgecolor='r', markerfacecolor='r')
		PL.axvline(x=threshold[2], linewidth=1, color='b')
		PL.axhline(y=threshold[3], linewidth=1, color='b')
		minX = plotdata[4].min()
		minY = plotdata[3].min()
		marginX = (threshold[2] - minX) * 0.1
		marginY = (threshold[3] - minY) * 0.1
		PL.axis([minX - marginX, threshold[2] + marginX * 3.0, minY - marginY, threshold[3] + marginY * 3.0])
		PL.xticks(size=6)
                PL.yticks(size=6)
            if ShowPlot != False: PL.draw()
            if FigFileDir != False: PL.savefig(FigFileDir+FigFileRoot+'_1.png', format='png', dpi=DPIDetail)
            #os.rename(FigFileDir+FigFileRoot+'_1.png', FigFileDir+FigFileRoot+'_1.gif')

        del plotdata, colordata
        return


def StatisticsPlot(PlotData, ShowPlot=True, FigFileDir=False, FigFileRoot=False):
    # PlotData = {
    #             row:  [row], # row number of the spectrum
    #             data: [data],
    #             flag: [flag], # 0: flag out, 1: normal, 2: exclude from the plot
    #             sigma: "Clipping Sigma"
    #             thre: [threshold(s) max, min(if any)], # holizontal line
    #             gap:  [gap(s)], # vertical tick: [[PosGap], [TimeGap]]
    #             title: "title",
    #             xlabel: "xlabel",
    #             ylabel: "ylabel"
    #             permanentflag: [PermanentFlag rows]
    #             isActive: True/False
    #             threType: "line" or "plot" # if "plot" then thre should be a list
    #                           having the equal length of row
    #            }

    if ShowPlot == False and FigFileDir == False: return
    if ShowPlot: PL.ion()
    PL.figure(MATPLOTLIB_FIGURE_ID[8])
    if ShowPlot: PL.ioff()
    
    if FigFileDir != False:
        if os.access(FigFileDir+'listofplots.txt', os.F_OK):
            BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
        else:
            BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
            print >> BrowserFile, 'TITLE: BF_Stat'
            print >> BrowserFile, 'FIELDS: Stat IF POL Iteration Page'
            print >> BrowserFile, 'COMMENT: Statistics of spectra'
        print >> BrowserFile, FigFileRoot+'.png' 
        BrowserFile.close()

    PL.cla()
    PL.clf()
    PL.subplot(211)
    PL.subplots_adjust(hspace=0.3)
    PL.title(PlotData['title'], size=7)
    PL.xlabel(PlotData['xlabel'], size=6)
    PL.ylabel(PlotData['ylabel'], size=7)
    PL.xticks(size=6)
    PL.yticks(size=6)
    if PlotData['isActive']:
        PL.figtext(0.05, 0.95, "ACTIVE", horizontalalignment='left', verticalalignment='top', color='green', size=18, style='italic', weight='bold')
    else:
        PL.figtext(0.05, 0.95, "INACTIVE", horizontalalignment='left', verticalalignment='top', color='red', size=18, style='italic', weight='bold')

    if len(PlotData['thre']) > 1: LowRange = True
    else:
        LowRange = False
        if PlotData['threType'] != "plot":
            PlotData['thre'].append(min(PlotData['data']))
        else:
            PlotData['thre'].append(min(min(PlotData['data']), min(PlotData['thre'][0])))

    # Calculate Y-scale
    if PlotData['threType'] == "plot":
        yrange = max(PlotData['thre'][0]) - PlotData['thre'][1]
        if yrange == 0.0: yrange = 1.0
        ymin = max(0.0, PlotData['thre'][1] - yrange * 0.5)
        ymax = (max(PlotData['thre'][0]) - ymin) * 1.3333333333 + ymin
    else:
        yrange = PlotData['thre'][0] - PlotData['thre'][1]
        if yrange == 0.0: yrange = 1.0
        ymin = max(0.0, PlotData['thre'][1] - yrange * 0.5)
        ymax = (PlotData['thre'][0] - ymin) * 1.3333333333 + ymin
    yy = ymax - ymin
    ScaleOut = [[ymax - yy * 0.1, ymax - yy * 0.04], \
                [ymin + yy * 0.1, ymin + yy * 0.04]]
    xmin = min(PlotData['row'])
    xmax = max(PlotData['row'])
    #print 'ymax=', ymax, 'ymin=', ymin
    #print PlotData['threType']
    #print PlotData['thre']
    #print PlotData['data']
    #print PlotData['flag']

    # Make Plot Data
    x = 0
    data = [[],[],[],[],[],[]]
    #print len(PlotData['permanentflag'])
    #print len(PlotData['flag'])

    for Pflag in PlotData['permanentflag']:
        if Pflag == 0: # Flag-out case
            data[4].append(PlotData['row'][x])
            if PlotData['data'][x] > ScaleOut[0][0]:
                data[5].append(ScaleOut[0][1])
            elif LowRange and PlotData['data'][x] < ScaleOut[1][0]:
                data[5].append(ScaleOut[1][1])
            else:
                data[5].append(PlotData['data'][x])
        elif PlotData['flag'][x] == 0: # Flag-out case
            data[2].append(PlotData['row'][x])
            if PlotData['data'][x] > ScaleOut[0][0]:
                data[3].append(ScaleOut[0][1])
            elif LowRange and PlotData['data'][x] < ScaleOut[1][0]:
                data[3].append(ScaleOut[1][1])
            else:
                data[3].append(PlotData['data'][x])
        else: # Normal case
            data[0].append(PlotData['row'][x])
            data[1].append(PlotData['data'][x])
        x += 1

    # Plot
    PL.plot(data[0], data[1], 'bo', markersize=3, markeredgecolor='b', markerfacecolor='b')
    PL.plot(data[2], data[3], 'bo', markersize=3, markeredgecolor='r', markerfacecolor='r')
    PL.plot(data[4], data[5], 's', markersize=3, markeredgecolor='0.5', markerfacecolor='0.5')
    PL.axhline(y=ScaleOut[0][0], linewidth=1, color='r')
    if PlotData['threType'] != "plot":
        PL.axhline(y=PlotData['thre'][0], linewidth=1, color='c')
        if LowRange:
            PL.axhline(y=PlotData['thre'][1], linewidth=1, color='c')
            PL.axhline(y=ScaleOut[1][0], linewidth=1, color='r')
    else:
        PL.plot(PlotData['row'], PlotData['thre'][0], '-', linewidth=1, color='c')

    PL.axis([xmin, xmax, ymin, ymax])

    if len(PlotData['gap']) > 0:
        for row in PlotData['gap'][0]:
            PL.axvline(x=row, linewidth=0.5, color='g', ymin=0.95)
    if len(PlotData['gap']) > 1:
        for row in PlotData['gap'][1]:
            PL.axvline(x=row, linewidth=0.5, color='c', ymin=0.9, ymax=0.95)

    PL.axis([xmin, xmax, ymin, ymax])

    if ShowPlot != False: PL.draw()
    if FigFileDir != False:
        OldPlot = FigFileDir+FigFileRoot+'.png'
        NewPlot = FigFileDir+FigFileRoot+'_trim.png'
        PL.savefig(OldPlot, format='png', dpi=DPIDetail)
        #PL.savefig(FigFileDir+FigFileRoot+'.png', format='png', dpi=DPIDetail)
        os.system('convert %s -trim %s' % (OldPlot, NewPlot))

    del data, ScaleOut
    return


def DependenceAnalysis(plotfile ='filename', plotmatrix='defaultmatrix.csv'):
    # matrix file format
    # Num,ItemNameX,ItemNameY,PlotTitle,Xlabel,Ylabel,ColoringStrategy
    #    Num: a number of 3 figures: e.g., 235 => nrow:2, ncol:3, 5th sub-window => lower-left side
    #    ColoringStrategy: 0 => use rating for raw data  1 => use rating for fitted data
    #    e.g., 235,Row,RMS,RMS v.s. rows,Spectra Number,RMS,1<ret>

    Mark = 'bo'
    Col1 = ['b', 'k', 'r', 'b', 'k', 'r']
    Col2 = ['c', 'k', 'r', 'c', 'k', 'r']
    Col3 = ['g', 'k', 'r', 'g', 'k', 'r']

    matrix = []
    matrixfile = open(plotmatrix, 'r')
    while 1:
        line = matrixfile.readline()
        if not line: break
        elif line[0] == '#': continue
        elif line.find(',') != -1: matrix.append(line[:-1].split(','))
    matrixfile.close()

    # Store Item names in keys without duplication
    keys = []
    for x in range(len(matrix)): keys.extend([matrix[x][1].upper().replace(' ', ''), matrix[x][2].upper().replace(' ', '')])
    for x in range(len(keys) - 1):
        key0 = keys[0]
        del(keys[0])
        try:
            keys.index(key0)
        except ValueError:
            keys.append(key0)
    keys.extend(['RAWRATING', 'FITRATING'])
    # print 'keys =', keys

    # Read and store required data
    infile = open(plotfile , 'r')
    while 1:
        line = infile.readline()
        if not line: break
        elif line[0] == '#': continue
        elif line[0] == '!':
            key = line[1:-1].upper().replace(' ', '').split(',')
            nkey = len(key)
            keyindex = []
            plotdata = []
            n = 0
            try:
                for x in keys:
                    keyindex.append((n, key.index(x)))
                    plotdata.append([])
                    n = n + 1
            except ValueError:
                print 'ERROR: Key %s could not find in the header' % (x)
                return
        else:
            valuelist = line[:-1].replace(' ', '').split(',')
            if len(valuelist) != nkey: continue
            for (n, i) in keyindex: plotdata[n].append(float(valuelist[i]))
    infile.close()

    # Re-arrange data for the coloring
    colordata = [[],[],[],[],[],[]]
    for z in range(6):
        for y in range(len(keys) - 2): colordata[z].append([])
    for y in range(len(keys) - 2):
        for x in range(len(plotdata[0])):
            if int(plotdata[len(keys) - 2][x] + 0.5) == 1:   colordata[1][y].append(plotdata[y][x])
            elif int(plotdata[len(keys) - 2][x] + 0.5) == 2: colordata[2][y].append(plotdata[y][x])
            else:                                            colordata[0][y].append(plotdata[y][x])
            if int(plotdata[len(keys) - 1][x] + 0.5) == 1:   colordata[4][y].append(plotdata[y][x])
            elif int(plotdata[len(keys) - 1][x] + 0.5) == 2: colordata[5][y].append(plotdata[y][x])
            else:                                            colordata[3][y].append(plotdata[y][x])

    # Plot
    PL.cla()
    PL.clf()
    for x in range(len(matrix)):
        PL.subplot(int(matrix[x][0]))
        PL.title(matrix[x][3], size=7)
        PL.xlabel(matrix[x][4], size=6)
        PL.ylabel(matrix[x][5], size=6)

        if int(matrix[x][6]) == 0: data = range(0,3)
        else: data = range(3,6)
        dx = keys.index(matrix[x][1].upper().replace(' ', ''))
        dy = keys.index(matrix[x][2].upper().replace(' ', ''))
        for z in data:
            PL.plot(colordata[z][dx], colordata[z][dy], Mark, markersize=3, markeredgecolor=Col1[z], markerfacecolor=Col1[z])
        PL.xticks(size=5)
        PL.yticks(size=5)
        PL.draw()


def DrawFitSpectrum(xaxis, rawdata, resultdata, row, counter, Nrow, fitparam, statistics, Xrange, Yrange, Mask, NoChange=False, DrawFlag=True, ShowPlot=True, FigFileDir=False, FigFileRoot=False):

    if ShowPlot == False and FigFileDir == False: return
    # Variables for Panel
    if ((Nrow-1) / (MAX_NhPanelFit*MAX_NvPanelFit)+1) > 1:
        (NhPanel, NvPanel) = (MAX_NhPanelFit, MAX_NvPanelFit)
    elif Nrow == 1: (NhPanel, NvPanel) = (1, 1)
    elif Nrow == 2: (NhPanel, NvPanel) = (1, 2)
    elif Nrow <= 4: (NhPanel, NvPanel) = (2, 2)
    elif Nrow <= 6: (NhPanel, NvPanel) = (2, 3)
    elif Nrow <= 9: (NhPanel, NvPanel) = (3, 3)
    elif Nrow < 13: (NhPanel, NvPanel) = (3, 4)
    else: (NhPanel, NvPanel) = (MAX_NhPanelFit, MAX_NvPanelFit)
    NSpFit = NhPanel * NvPanel
    NSp = (counter - 1) % NSpFit
    Npanel = int((counter - 1) / NSpFit)

    if counter % NSpFit == 0:
        DrawFlag = True
    if counter % NSpFit == 1 and FigFileDir != False:
        if os.access(FigFileDir+'listofplots.txt', os.F_OK):
            BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
        else:
            BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
            print >> BrowserFile, 'TITLE: BF_Fitting'
            print >> BrowserFile, 'FIELDS: FIT IF POL Iteration Page'
            print >> BrowserFile, 'COMMENT: Baseline Fit'
        print >> BrowserFile, FigFileRoot+'_%s.png' % Npanel
        BrowserFile.close()

    if ShowPlot: PL.ion()
    PL.figure(MATPLOTLIB_FIGURE_ID[4])
    if ShowPlot: PL.ioff()

    if type(NoChange) != int:
        TitleColor = 'k'
        AddTitle = ''
    else:
        TitleColor = 'g'
        AddTitle = 'No Change since Cycle%d' % NoChange
    TickSize = 12 - NhPanel * 2
    xtick = abs(Xrange[1] - Xrange[0]) / 3.0
    Order = int(math.floor(math.log10(xtick)))
    NewTick = int(xtick / (10**Order) + 1) * (10**Order)
    FreqLocator = MultipleLocator(NewTick)
    #if Order < 0: FMT = '%%.%dfG' % (-Order)
    #else: FMT = '%.2fG'
    if Order < 0: FMT = '%%.%df' % (-Order)
    else: FMT = '%.2f'
    Format = PL.FormatStrFormatter(FMT)

    fitdata = rawdata - resultdata

    if NhPanel == 1 and NvPanel == 1:
        PL.cla()
        PL.clf()
        PL.subplots_adjust(hspace=0.3)
        PL.subplot(121)
        PL.xlabel('Channel', size=10)
        #PL.ylabel('Flux Density', size=10)
        PL.ylabel('Intensity (K)', size=10)
        PL.title('%s\nRaw and Fit data : row = %d' % (AddTitle, row), size=10, color=TitleColor)
        PL.plot(xaxis, rawdata, color='b', linestyle='-', linewidth=0.2)
        PL.plot(xaxis, fitdata, color='r', linestyle='-', linewidth=0.8)
        for x in range(len(Mask)):
            PL.plot([Mask[x][0], Mask[x][0]], Yrange, color='c', linestyle='-', linewidth=0.8)
            PL.plot([Mask[x][1], Mask[x][1]], Yrange, color='c', linestyle='-', linewidth=0.8)
        PL.axis([Xrange[0], Xrange[1], Yrange[0], Yrange[1]])
        PL.figtext(0.05, 0.015, fitparam + statistics, size=10)

        PL.subplot(122)
        PL.xlabel('Channel', size=10)
        #PL.ylabel('Flux Density', size=10)
        PL.ylabel('Intensity (K)', size=10)
        PL.title('%s\nReduced data : row = %d' % (AddTitle, row), size=10, color=TitleColor)
        PL.plot(xaxis, resultdata, color='b', linestyle='-', linewidth=0.2)
        for x in range(len(Mask)):
            PL.plot([Mask[x][0], Mask[x][0]], Yrange, color='c', linestyle='-', linewidth=0.8)
            PL.plot([Mask[x][1], Mask[x][1]], Yrange, color='c', linestyle='-', linewidth=0.8)
        PL.axis([Xrange[0], Xrange[1], Yrange[0], Yrange[1]])
    else:
        if NSp == 0:
            PL.cla()
            PL.clf()
        x = NSp % NhPanel
        y = int(NSp / NhPanel)
        x00 = 1.0 / float(NhPanel) * (x + 0.1 + 0.05)
        x01 = 1.0 / float(NhPanel) * 0.4
        x10 = 1.0 / float(NhPanel) * (x + 0.5 + 0.05)
        x11 = 1.0 / float(NhPanel) * 0.4
        y0 = 1.0 / float(NvPanel) * (y + 0.1)
        y1 = 1.0 / float(NvPanel) * 0.8

        a0 = PL.axes([x00, y0, x01, y1])
#        PL.xlabel('Channel', size=TickSize)
        #PL.ylabel('Flux Density', size=TickSize)
        PL.ylabel('Intensity (K)', size=TickSize)
        PL.title('Fit: row = %d' % row, size=TickSize, color=TitleColor)
        PL.plot(xaxis, rawdata, color='b', linestyle='-', linewidth=0.2)
        PL.plot(xaxis, fitdata, color='r', linestyle='-', linewidth=0.8)
        a0.xaxis.set_major_formatter(Format)
        a0.xaxis.set_major_locator(FreqLocator)
        for x in range(len(Mask)):
            PL.plot([Mask[x][0], Mask[x][0]], Yrange, color='c', linestyle='-', linewidth=0.2)
            PL.plot([Mask[x][1], Mask[x][1]], Yrange, color='c', linestyle='-', linewidth=0.2)
        #PL.xticks(size=TickSize)
        for t in a0.get_xticklabels():
            t.set_fontsize((TickSize-1))
        PL.yticks(size=TickSize)
        PL.axis([Xrange[0], Xrange[1], Yrange[0], Yrange[1]])
#        PL.figtext(0.05, 0.015, fitparam + statistics, size=10)

        a1 = PL.axes([x10, y0, x11, y1])
#        PL.xlabel('Channel', size=TickSize)
        if type(NoChange) == int: PL.title('No Change since Cycle%d' % NoChange, size=TickSize, color=TitleColor)
        else: PL.title('Reduced: row = %d' % row, size=TickSize, color=TitleColor)
        PL.plot(xaxis, resultdata, color='b', linestyle='-', linewidth=0.2)
        a1.xaxis.set_major_formatter(Format)
        a1.xaxis.set_major_locator(FreqLocator)
        for x in range(len(Mask)):
            PL.plot([Mask[x][0], Mask[x][0]], Yrange, color='c', linestyle='-', linewidth=0.2)
            PL.plot([Mask[x][1], Mask[x][1]], Yrange, color='c', linestyle='-', linewidth=0.2)
        PL.axis([Xrange[0], Xrange[1], Yrange[0], Yrange[1]])
        #PL.xticks(size=TickSize)
        #newlabs = []
        for t in a1.get_xticklabels():
            #tt = t.get_text()
            #newlabs.append(tt)
            t.set_fontsize((TickSize-1))
        #PL.text(1,0,' [GHz]', size=(TickSize-1), va='bottom',transform=a1.transAxes)
        #PL.text(1,0,' [GHz]', size=(TickSize-1), ha='center',va='top',transform=a1.transAxes)
        #newlabs[len(newlabs)-1]=newlabs[len(newlabs)-1]+'GHz'
        #a1.set_xticklabels(newlabs)
        a1.yaxis.set_major_locator(PL.NullLocator())
        #a2 = PL.axes([(x10+x11-x11/15.0), y0-0.15/float(NvPanel), x11/10.0, y1/10.0])
        #a2 = PL.axes([(x10+x11-x11/10.0), y0-0.125/float(NvPanel), x11/10.0, y1/10.0])
        a2 = PL.axes([(x00-x01/5.0), y0-0.125/float(NvPanel), x01/10.0, y1/10.0])
        a2.set_axis_off()
        PL.text(0,0.5,' (GHz)', size=(TickSize-1) ,transform=a2.transAxes)

    if DrawFlag == True:
        if ShowPlot == True:
            PL.draw()
        if FigFileDir != False:
            PL.savefig(FigFileDir+FigFileRoot+'_%s.png' % Npanel, format='png', dpi=DPIDetail)
    return


def DrawMultiSpectra(data, Table, Abcissa, rows, mode='mode', chan0=-1, chan1=-1, FixScale=True, scale_max=1, scale_min=0, connect=True, Pass=False, ShowPlot=True, FigFileDir=False, FigFileRoot=False):
    """
    Draw multiple 1D spectra
    data: 2D array of Specta [row, channel]
    NhPanel, NvPanel: number of spectra shown in holizontal and vertical
    rows: rows to be plotted
    connect: connect points if True
    chan0, chan1: plotting channel range
    Table: [[IF, POL, X, Y, RAcent, DECcent, CombinedSP, flaggedSP, RMS]
                       ...
            [IF, POL, X, Y, RAcent, DECcent, CombinedSP, flaggedSP, RMS]]
    """

    if ShowPlot == False and FigFileDir == False: return
    if Pass == True:
        Npanel = 0
        PL.cla()
        PL.clf()
        Regends = PL.axes([0.1, 0.1, 0.8, 0.8])
        Regends.set_axis_off()
        PL.text(0.5, 0.5, "Baseline Fit has already been converged.\n\nPlease see panels in the  previous iteration cycle.", horizontalalignment='center', verticalalignment='center', size=10)
        if FigFileDir != False:
            PL.savefig(FigFileDir+FigFileRoot+'_%s.png' % Npanel, format='png', dpi=DPIDetail)
            if os.access(FigFileDir+'listofplots.txt', os.F_OK):
                BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
            else:
                BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
                print >> BrowserFile, 'TITLE: MultiSpectra'
                print >> BrowserFile, 'FIELDS: Result IF POL Iteration Page'
                print >> BrowserFile, 'COMMENT: Combined spectra by spatial Gridding'
            print >> BrowserFile, FigFileRoot+'_%s.png' % Npanel
            BrowserFile.close()
        return

    # Raster Case: re-arrange spectra to match RA-DEC orientation
    if mode.upper() == 'RASTER':
        for row in rows:
            if row == rows[0]:
                (Xmin, Xmax) = (Table[row][2], Table[row][2])
                (Ymin, Ymax) = (Table[row][3], Table[row][3])
            else:
                if Xmin > Table[row][2]: Xmin = Table[row][2]
                elif Xmax < Table[row][2]: Xmax = Table[row][2]
                if Ymin > Table[row][3]: Ymin = Table[row][3]
                elif Ymax < Table[row][3]: Ymax = Table[row][3]
        NhPanel = min(max(Xmax - Xmin + 1, Ymax - Ymin + 1), MAX_NhPanel)
        NvPanel = min(max(Xmax - Xmin + 1, Ymax - Ymin + 1), MAX_NvPanel)
        NH = int((Xmax - Xmin) / NhPanel + 1)
        NV = int((Ymax - Ymin) / NvPanel + 1)
        ##ROWS = NA.zeros(NH * NV * NhPanel * NvPanel, type=NA.Int) - 1
        ROWS = NP.zeros(NH * NV * NhPanel * NvPanel, dtype=NP.int) - 1
        for row in rows:
            GlobalPosX = (Xmax - Table[row][2] + Xmin) / NhPanel
            GlobalOfstX = (Xmax - Table[row][2] + Xmin) % NhPanel
            GlobalPosY = (Table[row][3] - Ymin) / NvPanel
            GlobalOfstY = (Table[row][3] - Ymin) % NvPanel
            ROWS[(GlobalPosY*NH+GlobalPosX)*NvPanel*NhPanel + GlobalOfstY*NhPanel + GlobalOfstX] = row
    else:
        NROW = len(rows)
        Npanel = (NROW - 1) / (MAX_NhPanel * MAX_NvPanel) + 1
        if Npanel > 1:  (NhPanel, NvPanel) = (MAX_NhPanel, MAX_NvPanel)
        else: (NhPanel, NvPanel) = (int((NROW - 0.1) ** 0.5) + 1, int((NROW - 0.1) ** 0.5) + 1)

    NSpFit = NhPanel * NvPanel
    NSp = 0
    Npanel = 0
    TickSize = 11 - NhPanel

    # Plotting routine
    if connect is True: Mark = '-b'
    else: Mark = 'bo'
    if ShowPlot: PL.ion()
    PL.figure(MATPLOTLIB_FIGURE_ID[5])
    if ShowPlot: PL.ioff()
    if chan1 == -1:
        chan0 = 0
        chan1 = len(data[rows[0]])
    xmin = min(Abcissa[1][chan0], Abcissa[1][chan1])
    xmax = max(Abcissa[1][chan0], Abcissa[1][chan1])

    if FixScale: 
        # to eliminate max/min value due to bad pixel or bad fitting,
        #  1/10-th value from max and min are used instead
        ListMax = []
        ListMin = []
        for row in rows:
            if Table[row][6] > 0:
                ListMax.append(data[row][chan0:chan1].max())
                ListMin.append(data[row][chan0:chan1].min())
        if len(ListMax) == 0: return
        ##ymax = NA.sort(ListMax)[len(ListMax) - len(ListMax)/10 - 1]
        ymax = NP.sort(ListMax)[len(ListMax) - len(ListMax)/10 - 1]
        ##ymin = NA.sort(ListMin)[len(ListMin)/10]
        ymin = NP.sort(ListMin)[len(ListMin)/10]
        ymax = ymax + (ymax - ymin) * 0.2
        ymin = ymin - (ymax - ymin) * 0.1
        del ListMax, ListMin
    else:
        ymin = scale_min
        ymax = scale_max

    if mode.upper() != 'RASTER': ROWS = rows[:]

    NSp = 0
    xtick = abs(Abcissa[1][-1] - Abcissa[1][0]) / 4.0
    Order = int(math.floor(math.log10(xtick)))
    NewTick = int(xtick / (10**Order) + 1) * (10**Order)
    FreqLocator = MultipleLocator(NewTick)
    if Order < 0: FMT = '%%.%dfG' % (-Order)
    else: FMT = '%.2fG'
    Format = PL.FormatStrFormatter(FMT)
    #Format = PL.FormatStrFormatter('%.3e')

    for row in ROWS:
        if NSp == 0:
            PL.cla()
            PL.clf()
        if 0 <= row < len(Table):
            title = '(IF, POL, X, Y) = (%s, %s, %s, %s)\n%s %s' % (Table[row][0], Table[row][1], Table[row][2], Table[row][3], HHMMSSss(Table[row][4], 0), DDMMSSs(Table[row][5], 0))
            x = NSp % NhPanel
            y = int(NSp / NhPanel)
            #x0 = 1.0 / float(NhPanel) * (x + 0.1)
            x0 = 1.0 / float(NhPanel) * (x + 0.22)
            #x1 = 1.0 / float(NhPanel) * 0.8
            x1 = 1.0 / float(NhPanel) * 0.75
            y0 = 1.0 / float(NvPanel) * (y + 0.15)
            #y1 = 1.0 / float(NvPanel) * 0.7
            y1 = 1.0 / float(NvPanel) * 0.65
            a = PL.axes([x0, y0, x1, y1])
            a.xaxis.set_major_formatter(Format)
            a.xaxis.set_major_locator(FreqLocator)
#            PL.xlabel('Channel', size=TickSize)
            #PL.ylabel('Flux', size=TickSize)
            PL.ylabel('Intensity (K)', size=TickSize)
            if Table[row][6] > 0:
                PL.plot(Abcissa[1], data[row], Mark, markersize=2, markeredgecolor='b', markerfacecolor='b')
            else:
                PL.text((xmin+xmax)/2.0, (ymin+ymax)/2.0, 'NO DATA', horizontalalignment='center', verticalalignment='center', size=TickSize)
            PL.title(title, size=TickSize)
            PL.xticks(size=TickSize)
            PL.yticks(size=TickSize)
            PL.axis([xmin, xmax, ymin, ymax])
        NSp += 1
        if NSp >= (NhPanel * NvPanel) or (row == rows[-1] and mode.upper() != 'RASTER'):
            NSp = 0
            if ShowPlot != False: PL.draw()
            if FigFileDir != False:
                PL.savefig(FigFileDir+FigFileRoot+'_%s.png' % Npanel, format='png', dpi=DPIDetail)
                #os.rename(FigFileDir+FigFileRoot+'_%s.png' % Npanel, FigFileDir+FigFileRoot+'_%s.gif' % Npanel)
                if os.access(FigFileDir+'listofplots.txt', os.F_OK):
                    BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
                else:
                    BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
                    print >> BrowserFile, 'TITLE: MultiSpectra'
                    print >> BrowserFile, 'FIELDS: Result IF POL Iteration Page'
                    print >> BrowserFile, 'COMMENT: Combined spectra by spatial Gridding'
                print >> BrowserFile, FigFileRoot+'_%s.png' % Npanel
                #print >> BrowserFile, FigFileRoot+'_%s.gif' % Npanel
                BrowserFile.close()
            Npanel += 1
    del ROWS
    return
            

def DrawSparseSpectra(data, Table, Abcissa, Npanel, chan0=-1, chan1=-1, AutoScale=True, scale_max=1, scale_min=0, ShowPlot=True, FigFileDir=False, FigFileRoot=False):
    """
    Draw multiple 1D spectra
    data: 2D array of Specta [row, channel]
    Npanel: number of spectra shown in holizontal and vertical
    chan0, chan1: plotting channel range
    Table: [[IF, POL, X, Y, RAcent, DECcent, CombinedSP, flaggedSP, RMS]
                       ...
            [IF, POL, X, Y, RAcent, DECcent, CombinedSP, flaggedSP, RMS]]
    """

    if ShowPlot == False and FigFileDir == False: return
    (Xmin, Xmax, Ymin, Ymax) = (Table[0][2], Table[0][2], Table[0][3], Table[0][3])
    for row in range(1, len(data)):
        if Xmin > Table[row][2]: Xmin = Table[row][2]
        elif Xmax < Table[row][2]: Xmax = Table[row][2]
        if Ymin > Table[row][3]: Ymin = Table[row][3]
        elif Ymax < Table[row][3]: Ymax = Table[row][3]
    Npanel = min(max(Xmax - Xmin + 1, Ymax - Ymin + 1), Npanel)
    STEP = int((max(Xmax - Xmin + 1, Ymax - Ymin + 1) - 1) / Npanel) + 1
    NH = (Xmax - Xmin) / STEP + 1
    NV = (Ymax - Ymin) / STEP + 1
    if chan0 == -1:
        chan0 = 0
        chan1 = len(data[0])
    xmin = min(Abcissa[1][chan0], Abcissa[1][chan1-1])
    xmax = max(Abcissa[1][chan0], Abcissa[1][chan1-1])

    TickSize = 10 - Npanel/2
    Format = PL.FormatStrFormatter('%.2f')

    ##Plot = NA.zeros((Npanel, Npanel, (chan1 - chan0)), NA.Float32) + NoData
    Plot = NP.zeros((Npanel, Npanel, (chan1 - chan0)), NP.float32) + NoData
    ##TotalSP = NA.zeros(chan1-chan0, NA.Float32)
    TotalSP = NP.zeros(chan1-chan0, NP.float32)
    ##LabelRADEC = NA.zeros((Npanel, 2, 2), NA.Float32) + NoData
    LabelRADEC = NP.zeros((Npanel, 2, 2), NP.float32) + NoData
    Nsp = 0.0
    for row in range(len(Table)):
        if Table[row][6] > 0:
            TotalSP = TotalSP + data[row][chan0:chan1]
            Nsp = Nsp + 1.0
    TotalSP /= Nsp
    for x in range(Npanel):
        for y in range(Npanel):
            Nsp = 0
            for row in range(len(Table)):
                if x * STEP <= Table[row][2] < (x + 1) * STEP and \
                   y * STEP <= Table[row][3] < (y + 1) * STEP:
                    if Table[row][6] > 0:
                        if LabelRADEC[x][0][0] == NoData:
                            LabelRADEC[x][0][0] = Table[row][4]
                            LabelRADEC[x][0][1] = Table[row][4]
                        else:
                            if LabelRADEC[x][0][0] > Table[row][4]:
                                LabelRADEC[x][0][0] = Table[row][4]
                            if LabelRADEC[x][0][1] < Table[row][4]:
                                LabelRADEC[x][0][1] = Table[row][4]
                        if LabelRADEC[y][1][0] == NoData:
                            LabelRADEC[y][1][0] = Table[row][5]
                            LabelRADEC[y][1][1] = Table[row][5]
                        else:
                            if LabelRADEC[y][1][0] > Table[row][5]:
                                LabelRADEC[y][1][0] = Table[row][5]
                            if LabelRADEC[y][1][1] < Table[row][5]:
                                LabelRADEC[y][1][1] = Table[row][5]
                        if Nsp == 0:
                            SP = data[row][chan0:chan1].copy()
                            Nsp = 1.0
                        else:
                            SP = SP + data[row][chan0:chan1]
                            Nsp += 1.0
                if Nsp > 0: Plot[x][y] = SP / Nsp


    # Plotting routine
    Mark = '-b'
    if ShowPlot: PL.ion()
    PL.figure(MATPLOTLIB_FIGURE_ID[6])
    if ShowPlot: PL.ioff()

    if AutoScale: 
        # to eliminate max/min value due to bad pixel or bad fitting,
        #  1/10-th value from max and min are used instead
        ListMax = []
        ListMin = []
        for x in range(Npanel):
            for y in range(Npanel):
                if Plot[x][y].min() > NoDataThreshold:
                    ListMax.append(Plot[x][y].max())
                    ListMin.append(Plot[x][y].min())
        if len(ListMax) == 0: return
        ##ymax = NA.sort(ListMax)[len(ListMax) - len(ListMax)/10 - 1]
        ymax = NP.sort(ListMax)[len(ListMax) - len(ListMax)/10 - 1]
        ##ymin = NA.sort(ListMin)[len(ListMin)/10]
        ymin = NP.sort(ListMin)[len(ListMin)/10]
        ymax = ymax + (ymax - ymin) * 0.2
        ymin = ymin - (ymax - ymin) * 0.1
        del ListMax, ListMin
    else:
        ymin = scale_min
        ymax = scale_max

    PL.cla()
    PL.clf()
#    a0 = PL.subplot((Npanel+1)/2+1, 1, 1)
    #a0 = PL.subplot((NV+3)/2+1, 1, 1)
    a0 = PL.subplot((NV+3)/2+3, 1, 1)
    a0.xaxis.set_major_formatter(Format)
    PL.plot(Abcissa[1][chan0:chan1], TotalSP, color='b', linestyle='-', linewidth=0.4)
    PL.xlabel('Frequency(GHz)', size=(TickSize+1))
    PL.ylabel('Intensity(K)', size=(TickSize+1))
    PL.xticks(size=TickSize)
    PL.yticks(size=TickSize)
    PL.title('Spatially Integrated Spectrum', size=(TickSize + 1))

#    for x in range(Npanel):
#        for y in range(Npanel):
#            a1 = PL.subplot(Npanel+2, Npanel, (y + 2) * Npanel + x + 1)
    for x in range(NH):
        for y in range(NV):
#            a1 = PL.subplot(NV+2, NH, (NV - 1 - y + 2) * NH + (NH - 1 - x) + 1)
            a1 = PL.subplot(NV+3, NH+1, (NV - 1 - y + 2) * (NH + 1) + (NH - 1 - x) + 2)
            if Plot[x][y].min() > NoDataThreshold:
                PL.plot(Abcissa[1][chan0:chan1], Plot[x][y], color='b', linestyle='-', linewidth=0.2)
            else:
                PL.text((xmin+xmax)/2.0, (ymin+ymax)/2.0, 'NO DATA', horizontalalignment='center', verticalalignment='center', size=(TickSize + 1))
            a1.yaxis.set_major_locator(PL.NullLocator())
            a1.xaxis.set_major_locator(PL.NullLocator())
#            PL.xlabel('Freq.(GHz)', size=TickSize)
#            PL.ylabel('Flux', size=TickSize)
            PL.axis([xmin, xmax, ymin, ymax])
#            PL.xticks(size=TickSize)
#            PL.yticks(size=TickSize)
#            PL.title('Reduced: row = %d' % row, size=(TickSize + 1))
#            a1.yaxis.set_major_locator(PL.NullLocator())
    for x in range(NH):
        a1 = PL.subplot(NV+3, NH+1, (NV + 2) * (NH + 1) + NH - x + 1)
        a1.set_axis_off()
        PL.text(0.5, 0.5, HHMMSSss((LabelRADEC[x][0][0]+LabelRADEC[x][0][1])/2.0, 0), horizontalalignment='center', verticalalignment='center', size=TickSize)
    for y in range(NV):
        a1 = PL.subplot(NV+3, NH+1, (NV + 1 - y) * (NH + 1) + 1)
        a1.set_axis_off()
        PL.text(0.5, 0.5, DDMMSSs((LabelRADEC[y][1][0]+LabelRADEC[y][1][1])/        2.0, 0), horizontalalignment='center', verticalalignment='center', size=TickSize)
    a1 = PL.subplot(NV+3, NH+1, (NV + 2) * (NH + 1) + 1)
    a1.set_axis_off()
    PL.text(0.5, 1, 'Dec', horizontalalignment='center', verticalalignment='bottom', size=(TickSize+1))
    PL.text(1, 0.5, 'RA', horizontalalignment='center', verticalalignment='center', size=(TickSize+1))
    if ShowPlot: PL.draw()
    if FigFileDir != False:
        PL.savefig(FigFileDir+FigFileRoot+'_0.png', format='png', dpi=DPIDetail)
        #os.rename(FigFileDir+FigFileRoot+'_0.png', FigFileDir+FigFileRoot+'_0.gif')
        if os.access(FigFileDir+'listofplots.txt', os.F_OK):
            BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
        else:
            BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
            print >> BrowserFile, 'TITLE: SparseSpectraMap'
            print >> BrowserFile, 'FIELDS: Result IF POL Iteration Page'
            print >> BrowserFile, 'COMMENT: Sparse Spectra Map'
        print >> BrowserFile, FigFileRoot+'_0.png'
        #print >> BrowserFile, FigFileRoot+'_0.gif'
        BrowserFile.close()
    return


def ExtractInfo(filename, output, connect=True, circle=[]):
    """
    Draw loci of the telescope pointing
    filename accepts list of MSs, i.e., ['a.ms','b.ms']
    xaxis: extension header keyword for RA
    yaxis: extension header keyword for DEC
    connect: connect points if True
    keywordSky: Object containing keywords are assumed to be Sky integration
    """

    # Convert filename into list of filenames
    if type(filename) is str: filename = [filename]
    if type(filename) is not list: return false

    # Initialize casa table tool
    tbtool = casac.homefinder.find_home_by_name('tableHome')
    tb = tbtool.create()

    RA = []
    DEC = []
    MJD = []
    for filenamein in filename:
        tb.open(filenamein + '/POINTING')
        ([RA0], [DEC0]) = tb.getcol('DIRECTION')
        RA += list(RA0)
        DEC += list(DEC0)
        MJD += tb.getcol('TIME')
        tb.close()

    return (RA, DEC, MJD)


def PlotXY(TableFile, output, Xaxis, Yaxis, Title='None', Xlabel='None', Ylabel='None', connect=True, circle=0):
    """
    XY plot of the data table
    TableFile format:
       !ID,Item1,Item2,Item3,,,,
       0,1,1,1,1,,,,
    xaxis: Column name in the Table
    yaxis: Single column name or List of column names in the Table
    connect: connect points if True
    circle: Error circle specified in radius. if 0, no circle is drawn
    """

    # Convert filename into list of filenames
    if type(Yaxis) is str: Yaxis = [Yaxis]
    if type(Yaxis) is not list: return false

    # Read data from Table
    keys = [Xaxis] + Yaxis
    infile = open(TableFile, 'r')
    while 1:
        line = infile.readline()
        if not line: break
        elif line[0] == '#': continue
        elif line[0] == '!':
            key = line[1:-1].upper().replace(' ', '').split(',')
            nkey = len(key)
            keyindex = []
            plotdata = []
            n = 0
            try:
                for x in keys:
                    keyindex.append((n, key.index(x)))
                    plotdata.append([])
                    n = n + 1
            except ValueError:
                print 'Key %s could not find in the header' % (x)
                return
        else:
            valuelist = line[:-1].replace(' ', '').split(',')
            try:
                if len(valuelist) != nkey: continue
            except UnboundLocalError:
                print 'Cannot find header in %s' % (DataTable)
                return False
            for (n, i) in keyindex: plotdata[n].append(float(valuelist[i]))
    infile.close()


    # Plotting routine
    if connect is True: Mark = 'g-o'
    else: Mark = 'bo'
    PL.cla()
    PL.clf()
    if Xlabel != 'None': PL.xlabel(Xlabel)
    if Ylabel != 'None': PL.ylabel(Ylabel)
    if Title != 'None': PL.title(Title)
    for i in range(len(plotdata) - 1):
      PL.plot(plotdata[0], plotdata[i + 1], Mark, markersize=2, markeredgecolor='b')

    if circle:
        Mark = 'g-'
        x = []
        y = []
        for t in range(50):
            x.append(plotdata[0][0] + R * math.sin(t * 0.13))
            y.append(plotdata[1][0] + R * math.cos(t * 0.13))
        PL.plot(x, y, Mark)
#    (Xmin, Xmax) = PL.xlim()
#    (Ymin, Ymax) = PL.ylim()
#    if (Xmax - Xmin) > (Ymax - Ymin):
#        PL.ylim((Ymin + Ymax) / 2.0 - (Xmax - Xmin) / 2.0, (Ymin + Ymax) / 2.0 + (Xmax - Xmin) / 2.0)
#    else:
#        PL.xlim((Xmin + Xmax) / 2.0 - (Ymax - Ymin) / 2.0, (Xmin + Xmax) / 2.0 + (Ymax - Ymin) / 2.0)
#    try:
#        PL.show()
#    PL.show()
    PL.draw()
#    PL.savefig(output)
#    except TypeError:
#        print 'Ignore matplotlib TypeError'



def DrawDebugFit(mode='Init', xaxis=[], data=[], Ymin=0, Ymax=1):

    if mode.upper() == 'INIT':
        PL.figure(MATPLOTLIB_FIGURE_ID[0])
        PL.cla()
        PL.clf()
        return
    elif mode.upper() == 'FIRST':
        PL.figure(MATPLOTLIB_FIGURE_ID[0])
        PL.subplot(121)
        PL.plot(xaxis, data, color='b', linestyle='-', linewidth=0.2)
        return
    else:
        PL.figure(MATPLOTLIB_FIGURE_ID[0])
        PL.subplot(122)
        PL.plot(xaxis, data, color='b', linestyle='-', linewidth=0.2)
        PL.axis([0, len(xaxis), Ymin, Ymax])
        PL.subplot(121)
        PL.axis([0, len(xaxis), Ymin, Ymax])

        PL.draw()
        return


def ShowClusterInChannelSpace(data, BestLines, ShowPlot=True, FigFileDir=False, FigFileRoot=False):
    """
    Plot Clusters in Channel Space (plot center and width of detected lines)
    data: numpy array: [[Wid0, Cent0], [Wid1, Cent1], ..., [WidN, CentN]]
    BestLines: [[ClusterCent0, ClusterWid0, True, Radius], ...,[]]
    """
    if ShowPlot == False and FigFileDir == False: return

    if ShowPlot: PL.ion()
    PL.figure(MATPLOTLIB_FIGURE_ID[3])
    if ShowPlot: PL.ioff()
    PL.cla()
    PL.clf()

    #BestLines.sort()
    BestLinesCopy=[]
    for item in BestLines:BestLinesCopy.append(item)
    BestLinesCopy.sort()
    ##[X, Y] = NA.array(data.transpose())
    [X, Y] = NP.array(data.transpose())
    PL.plot(Y, X, 'bs', markersize=1)
    [xmin, xmax, ymin, ymax] = PL.axis()
    Mark = 'r-'
    cx = []
    cy = []
    i = 0
    for [CX, CY, dummy, R] in BestLinesCopy:
        cx.append(CX)
        cy.append(CY)
        x = []
        y = []
        for t in range(50):
            x.append(CX + R * math.sin(t * 0.13))
            y.append(CY + R * math.cos(t * 0.13))
        PL.plot(x, y, Mark)
        PL.text(CX, CY, str(i), fontsize=10, color='red') 
        i += 1
    #PL.plot(cx, cy, 'rx', markersize=10)
    PL.xlabel('Line Center (Channel)', fontsize=11)
    PL.ylabel('Line Width (Channel)', fontsize=11)
    PL.axis([xmin, xmax, 0, ymax])
    PL.title('Clusters in the line Center-Width space\n\nRed Oval(s) shows each clustering region. Size of the oval represents cluster radius', fontsize=11)
    #MSG = 'Clusters in the line Center-Width space\n\nRed Oval(s) show each clustering region. Size of the oval represents cluster radius'
    #PL.text(0.5, 0.5, MSG, horizontalalignment='center', verticalalignment='center', size=8)

    if ShowPlot != False: PL.draw()

    if FigFileDir != False:
        PL.savefig(FigFileDir+FigFileRoot+'_ChannelSpace.png', format='png', dpi=DPIDetail)
        #if os.access(FigFileDir+'listofplots.txt', os.F_OK):
        #    BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
        #else:
        #    BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
        #    print >> BrowserFile, 'TITLE: Clustering'
        #    print >> BrowserFile, 'FIELDS: Cluster IF POL Iteration process'
        #    print >> BrowserFile, 'COMMENT: '
        #print >> BrowserFile, FigFileRoot+'_ChannelSpace.png'
        #BrowserFile.close()
    return


def ShowCluster(data, Threshold, Lines, Abcissa, RA0, DEC0, ScaleRA, ScaleDEC, mode='detection', ShowPlot=True, FigFileDir=False, FigFileRoot=False):
    """
    Plot Cluster analysis result
    data: 3D array of Cluster [cluster][ra-grid][dec-grid]
    Threshold: list of thresholds for display
       e.g., [t1, t2] => data > t1: color1, data > t2: color2
    Lines: list of lines
        Lines[Ncluster] = [LineCenter, LineWidth, Validity]
        Validity: True or False
    ScaleRA, ScaleDEC: unit degree
    RA0, DEC0: minimum RA and DEC in the region: unit degree
    mode: 'detection', 'validation', 'smoothing', 'regions'
    """

    if ShowPlot == False and FigFileDir == False: return

    # Variables for Panel
    Ncluster = len(data)
    (NhPanel, NvPanel) = (int((Ncluster - 0.1) ** 0.5) + 1, int((Ncluster - 0.1) ** 0.5) + 1)

    # 2008/9/20 DEC Effect
    Aspect = 1.0 / math.cos(DEC0 / 180.0 * 3.141592653)

    # Plotting routine
    #Marks = ['ys', 'cs', 'bs', 'gs']
    Marks = ['gs', 'bs', 'cs', 'ys']
    if ShowPlot: PL.ion()
    PL.figure(MATPLOTLIB_FIGURE_ID[3])
    if ShowPlot: PL.ioff()
    X = len(data[0])
    Y = len(data[0][0])

    xmin = RA0
    xmax = X * ScaleRA + xmin
    ymin = DEC0
    ymax = Y * ScaleDEC + ymin
    msize = int(300.0 / (max(X, Y*1.414) * NhPanel) + 1.0)
    TickSize = int(6 + (1/NhPanel) * 2)

    PL.cla()
    PL.clf()

    # HHMMSS label
    span = max(xmax - xmin, ymax - ymin)
    (RAlocator, DEClocator, RAformatter, DECformatter)= RADEClabel(span)

    for Cluster in range(Ncluster):
        x = Cluster % NhPanel
        y = int(Cluster / NhPanel)
        if x == 0:
            x0 = 1.0 / float(NhPanel) * (x + 0.1)
        else:
            #x0 = 1.0 / float(NhPanel) * (x + 0.15)
            x0 = 1.0 / float(NhPanel) * (x + 0.1)
        x1 = 1.0 / float(NhPanel) * 0.8
        #y0 = 0.8 / float(NvPanel) * (NvPanel - y - 1 + 0.2)
        #if y == 0:
        #    y0 = 0.8 / float(NvPanel) * (NvPanel - y - 1 + 0.3)
        #else:
        #    y0 = 0.8 / float(NvPanel) * (NvPanel - y - 1 + 0.3)
        ##    y0 = 0.8 / float(NvPanel) * (NvPanel - y - 1 + 0.4)
        y0 = 0.8 / float(NvPanel) * (NvPanel - y - 1 + 0.3)
        
        #y1 = 0.8 / float(NvPanel) * 0.7
        if NvPanel>2:
            y1 = 0.8 / float(NvPanel) * 0.5
        else:
            y1 = 0.8 / float(NvPanel) * 0.6
        
        a = PL.axes([x0, y0, x1, y1])
        # 2008/9/20 DEC Effect
        a.set_aspect(Aspect)
        #a.set_aspect('equal')
        xdata = []
        ydata = []
        for i in range(len(Threshold)):
            xdata.append([])
            ydata.append([])
        for RA in range(X):
            for DEC in range(Y):
                for i in range(len(Threshold)):
                    if data[Cluster][RA][DEC] > Threshold[i]:
                        xdata[i].append(xmin + (0.5 + RA)*ScaleRA)
                        ydata[i].append(ymin + (0.5 + DEC)*ScaleDEC)
                        break
        # Convert Channel to Frequency and Velocity
        #PL.title('Cluster%s: %s: Center=%.2f GHz Width=%.1f km/s' % \
	PL.title('Cluster%s: Center=%.4f GHz Width=%.1f km/s' % \
	     (Cluster, Abcissa[1][int(Lines[Cluster][0]+0.5)], abs(Abcissa[2][int(Lines[Cluster][1]+0.5)] - Abcissa[2][0])), size=(TickSize + 1))
	PL.xlabel('RA', size=TickSize)
        PL.ylabel('Dec', size=TickSize)
        if Lines[Cluster][2] != False:
            for i in range(len(Threshold)):
                PL.plot(xdata[i], ydata[i], Marks[4 - len(Threshold) + i], markersize=msize)
        #if mode.upper() == 'DETECTION':
        #    PL.plot(xdata1, ydata1, Marks[0], markersize=msize)
        #    PL.plot(xdata2, ydata2, Marks[1], markersize=msize)
        #elif mode.upper() == 'VALIDATION':
        #    PL.plot(xdata1, ydata1, Marks[1], markersize=msize)
        #    PL.plot(xdata2, ydata2, Marks[2], markersize=msize)
        #elif mode.upper() == 'CONTINUITY':
        #    PL.plot(xdata1, ydata1, Marks[1], markersize=msize)
        #    PL.plot(xdata2, ydata2, Marks[2], markersize=msize)
        #else:
        #    PL.plot(xdata1, ydata1, Marks[3], markersize=msize)
        a.xaxis.set_major_formatter(RAformatter)
        a.yaxis.set_major_formatter(DECformatter)
        a.xaxis.set_major_locator(RAlocator)
        a.yaxis.set_major_locator(DEClocator)
        xlabels = a.get_xticklabels()
        PL.setp(xlabels, 'rotation', RArotation, fontsize=TickSize)
        ylabels = a.get_yticklabels()
        PL.setp(ylabels, 'rotation', DECrotation, fontsize=TickSize)

        if Lines[Cluster][2] != True:
            if NhPanel >2: 
		PL.text((xmin+xmax)/2.0, (ymin+ymax)/2.0, 'INVALID CLUSTER', horizontalalignment='center', verticalalignment='center', size=TickSize)
            else:
		PL.text((xmin+xmax)/2.0, (ymin+ymax)/2.0, 'INVALID CLUSTER', horizontalalignment='center', verticalalignment='center', size=(TickSize+1))
        PL.xticks(size=TickSize)
        PL.yticks(size=TickSize)
        # Swap RA direction
        PL.axis([xmax, xmin, ymin, ymax])

    # Regends
    #Regends = PL.axes([0.0, 0.8, 1.0, 0.2])
    Regends = PL.axes([0.0, 0.85, 1.0, 0.15])
    Regends.set_axis_off()
    if 1.0 <= ScaleRA:
        (RAtext, DECtext, Unit) = (ScaleRA, ScaleDEC, 'degree')
    elif 1.0 <= ScaleRA * 60.0:
        (RAtext, DECtext, Unit) = (ScaleRA * 60.0, ScaleDEC * 60.0, 'arcmin')
    else:
        (RAtext, DECtext, Unit) = (ScaleRA * 3600.0, ScaleDEC * 3600.0, 'arcsec')
    ScaleMSG = 'Scale of the Square (Grid): %.1f x %.1f (%s)' % (RAtext, DECtext, Unit)
    if mode.upper() == 'DETECTION':
        MSG = 'Clustering Analysis at Detection stage\n\nYellow Square: Single spectrum is detected in the grid\nCyan Square: More than one spectra are detected in the grid\n%s' % ScaleMSG
    elif mode.upper() == 'VALIDATION':
        MSG = 'Clustering Analysis at Validation stage\n\nValidation by the rate (Number of clustering member [Nmember] v.s. Number of total spectra belong to the Grid [Nspectra])\n Blue Square: Validated: Nmember > %.1f x Nspectra\nCyan Square: Marginally validated: Nmember > %.1f x Nspectra\nYellow Square: Questionable: Nmember > %.1f x Nspectrum\n%s' % (Threshold[0],Threshold[1],Threshold[2],ScaleMSG)
    elif mode.upper() == 'SMOOTHING':
        MSG = 'Clustering Analysis at Smoothing stage\n\nBlue Square: Passed continuity check\nCyan Square: Border\nYellow Square: Questionable\n%s' % ScaleMSG
    else:
        MSG = 'Clustering Analysis at Final stage\n\nGreen Square: Final Grid where the line protection channels are calculated and applied to the baseline subtraction\nBlue Square: Final Grid where the calculated line protection channels are applied to the baseline subtraction\n\nIsolated Grids are eliminated.\n%s' % ScaleMSG
    PL.text(0.5, 0.5, MSG, horizontalalignment='center', verticalalignment='center', size=8)

    if ShowPlot != False: PL.draw()

    if FigFileDir != False:
        PL.savefig(FigFileDir+FigFileRoot+'_%s.png' % mode.lower(), format='png', dpi=DPIDetail)
        #os.rename(FigFileDir+FigFileRoot+'_%s.png' % mode.lower(), FigFileDir+FigFileRoot+'_%s.gif' % mode.lower())
        if os.access(FigFileDir+'listofplots.txt', os.F_OK):
            BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
        else:
            BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
            print >> BrowserFile, 'TITLE: Clustering'
            print >> BrowserFile, 'FIELDS: Cluster IF POL Iteration process'
            print >> BrowserFile, 'COMMENT: '
        print >> BrowserFile, FigFileRoot+'_%s.png' % mode.lower()
        #print >> BrowserFile, FigFileRoot+'_%s.gif' % mode.lower()
        BrowserFile.close()



def DrawRADEC(Table, rows, connect=True, circle=[], ObsPattern=False, ShowPlot=True, FigFile=False):
    """
    Draw loci of the telescope pointing
    xaxis: extension header keyword for RA
    yaxis: extension header keyword for DEC
    connect: connect points if True
    """

    if ShowPlot == False and FigFile == False: return
    # Extract RA and DEC
    RA = []
    DEC = []
    for ID in rows:
        RA.append(Table[ID][9])
        DEC.append(Table[ID][10])
    span = max(max(RA) - min(RA), max(DEC) - min(DEC))
    xmax = min(RA) - span / 10.0
    xmin = max(RA) + span / 10.0
    ymax = max(DEC) + span / 10.0
    ymin = min(DEC) - span / 10.0
    (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

    # 2008/9/20 DEC Effect
    Aspect = 1.0 / math.cos(Table[0][10] / 180.0 * 3.141592653)

    # Plotting routine
    if ShowPlot: PL.ion()
    PL.figure(MATPLOTLIB_FIGURE_ID[1])
    if ShowPlot: PL.ioff()
    if connect is True: Mark = 'g-o'
    else: Mark = 'bo'
    PL.cla()
    PL.clf()
    a = PL.axes([0.15, 0.2, 0.7, 0.7])
    # 2008/9/20 DEC Effect
    a.set_aspect(Aspect)
    #a.set_aspect('equal')
    PL.xlabel('RA')
    PL.ylabel('Dec')
    if ObsPattern == False:
        PL.title('Telescope Pointing on the Sky')
    else:
        PL.title('Telescope Pointing on the Sky\nPointing Pattern = %s' % ObsPattern)
    PL.plot(RA, DEC, Mark, markersize=2, markeredgecolor='b', markerfacecolor='b')
    a.xaxis.set_major_formatter(RAformatter)
    a.yaxis.set_major_formatter(DECformatter)
    a.xaxis.set_major_locator(RAlocator)
    a.yaxis.set_major_locator(DEClocator)
    xlabels = a.get_xticklabels()
    PL.setp(xlabels, 'rotation', RArotation, fontsize=8)
    ylabels = a.get_yticklabels()
    PL.setp(ylabels, 'rotation', DECrotation, fontsize=8)
    # plot starting position with beam and end position 
    if len(circle) != 0:
        for R in circle:
            Mark = 'r-'
            x = []
            y = []
            for t in range(50):
                # 2008/9/20 DEC Effect
                x.append(RA[0] + R * math.sin(t * 0.13)  * Aspect)
                #x.append(RA[0] + R * math.sin(t * 0.13))
                y.append(DEC[0] + R * math.cos(t * 0.13))
            PL.plot(x, y, Mark)
            #Mark = 'm-'
            Mark = 'ro'
            x = []
            y = []
            x.append(RA[-1])
            y.append(DEC[-1])
            #for t in range(50):
            #    x.append(RA[-1] + R*.9 * math.sin(t * 0.13))
            #    y.append(DEC[-1] + R*.9 * math.cos(t * 0.13))
            PL.plot(x, y, Mark, markersize=4, markeredgecolor='r', markerfacecolor='r')
    # Swap RA direction
#    [xmin, xmax, ymin, ymax] = PL.axis()
#    PL.axis([xmax, xmin, ymin, ymax])
    PL.axis([xmin, xmax, ymin, ymax])
    if ShowPlot != False: PL.draw()
    if FigFile != False: PL.savefig(FigFile, format='png', dpi=DPISummary)

    del RA, DEC, x, y
    return


def DrawAzEl(Table, TimeGapList, rows, ShowPlot=True, FigFile=False):
    """
    Plot Az El v.s. Time
    Table: DataTable
    TimeGapList: [[rowX1, rowX2,...,rowXN],[rowX1, rowX2,...,rowXN]]
    """
    # Select large time gap (equivalent to one set of raster scan)
    TimeGap = TimeGapList[1]

    # if DoStack is true plot will be stacked with different dates.
    DoStack = True
    if ShowPlot == False and FigFile == False: return
    # Extract Az, El, and MJD
    Az = []
    AzArr = []
    El = []
    ElArr = []
    MJD = []
    MJDArr = []
    TGap = []
    PGap = []

    # create local CASA quanta tool
    qatool = casac.homefinder.find_home_by_name('quantaHome')
    qa = qatool.create()

    for gap in TimeGap:
        if gap > rows[-1]: break
        TGap.append((Table[gap - 1][6] + Table[gap][6]) / 2.)
    for gap in TimeGap:
        if gap > rows[-1]: break
        PGap.append((Table[gap - 1][6] + Table[gap][6]) / 2.)
    for ID in rows:
        Az.append(Table[ID][11])
        El.append(Table[ID][12])
        MJD.append(Table[ID][6])
    ##MJDmin = NA.array(MJD).min()
    ##MJDmax = NA.array(MJD).max()
    MJDmin = NP.array(MJD).min()
    MJDmax = NP.array(MJD).max()
    Extend = (MJDmax - MJDmin) * 0.05
    MJDmin -= Extend
    MJDmax += Extend
    ELmin = min(El)
    if min(Az) < 0:
        for row in range(len(Az)):
            if Az[row] < 0: Az[row] += 360.0

    if DoStack:
        # find number of days 
        ndays = 0
        for n in range(len(MJD)):
            if n == 0:
                ndays +=1
                MJDArr.append([])
                AzArr.append([])
                ElArr.append([])
                MJDArr[0].append(MJD[0])
                AzArr[0].append(Az[0])
                ElArr[0].append(El[0])
            else:
                delt = int(MJD[n]) - int(MJD[n-1])
                if delt >= 1:
                    ndays += 1
                    MJDArr.append([])
                    AzArr.append([])
                    ElArr.append([])
       
                MJDArr[ndays-1].append(MJD[n])
                AzArr[ndays-1].append(Az[n])
                ElArr[ndays-1].append(El[n])

    # Plotting routine
    if ShowPlot: PL.ion()
    PL.figure(MATPLOTLIB_FIGURE_ID[2])
    if ShowPlot: PL.ioff()
    PL.cla()
    PL.clf()

    if DoStack:
        markercolorbase = ['b', 'm', 'y', 'k', 'r']
        ##m=NA.ceil(ndays*1.0/len(markercolorbase))
        m=NP.ceil(ndays*1.0/len(markercolorbase))
        markercolors = markercolorbase*int(m)
        markerbase = ['o','x','^','s','v']
        markers = []
        for mrk in markerbase:
            for i in range(len(markercolorbase)):
                if len(markers)<len(markercolors): 
                    markers.append(markercolors[i]+mrk)

        PL.axes([0.1, 0.55, 0.8, 0.35])
        PL.ylabel('Elevation (deg)')
        PL.title('Elevation Plot v.s. Time (with Detected large Gaps)')
        PL.xlabel('Time (UT)')
        for nd in range(ndays):
            ##UTdata = (NA.array(MJDArr[nd])-int(MJDArr[nd][0]))*24.0
            UTdata = (NP.array(MJDArr[nd])-int(MJDArr[nd][0]))*24.0
            if nd == 0:
                UTmin = min(UTdata)
                UTmax = max(UTdata)
            else:
                if min(UTdata) < UTmin: UTmin = min(UTdata)
                if max(UTdata) > UTmax: UTmax = max(UTdata)

            #date = qa.quantity(MJDArr[nd][0],'d')
            date = qa.quantity(str(MJDArr[nd][0])+'d')
            (datelab,rest) = qa.time(date,form='dmy').split('/')  
            #PL.plot(UTdata, ElArr[nd], 'bo', markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
            
            PL.plot(UTdata, ElArr[nd], markers[nd], markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
            PL.legend(prop=FontProperties(size='smaller'),markerscale=1.0,numpoints=1)
            for Time in TGap:
                if int(Time) == int(MJDArr[nd][0]):
                    modTime = (Time - int(Time))*24
                    PL.axvline(x=modTime, linewidth=0.5, color='c',label='_nolegend_')
        Extend = (UTmax - UTmin) * 0.05
        UTmin -= Extend
        UTmax += Extend
        if ELmin < 0: PL.axis([UTmin, UTmax, -90, 90])
        else: PL.axis([UTmin, UTmax, 0, 90])

        PL.axes([0.1, 0.1, 0.8, 0.35])
        PL.ylabel('Azimuth (deg)')
        PL.title('Azimuth Plot v.s. Time (with Detected large Gaps)')
        PL.xlabel('Time (UT)')
        for nd in range(ndays):
            ##UTdata = (NA.array(MJDArr[nd])-int(MJDArr[nd][0]))*24.0
            UTdata = (NP.array(MJDArr[nd])-int(MJDArr[nd][0]))*24.0
            date = qa.quantity(str(MJDArr[nd][0])+'d')
            (datelab,rest) = qa.time(date,form='dmy').split('/')  
            #PL.plot(UTdata, AzArr[nd], 'bo', markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
            PL.plot(UTdata, AzArr[nd], markers[nd], markersize=2, markeredgecolor=markercolors[nd], markerfacecolor=markercolors[nd],label=datelab)
            PL.legend(prop=FontProperties(size='smaller'),markerscale=0.8,numpoints=1)
            for Time in PGap:
                if int(Time) == int(MJDArr[nd][0]):
                    modTime = (Time - int(Time))*24
                    PL.axvline(x=modTime, linewidth=0.5, color='g', label='_nolegend_')
        
        PL.axis([UTmin, UTmax, 0, 360])
    else:
        PL.axes([0.1, 0.55, 0.8, 0.35])
        PL.ylabel('Elevation (deg)')
        PL.title('Elevation Plot v.s. Time (with Detected large Gaps)')
        PL.xlabel('MJD (Day)')
        for Time in TGap:
            PL.axvline(x=Time, linewidth=0.5, color='c')
        PL.plot(MJD, El, 'bo', markersize=2, markeredgecolor='b', markerfacecolor='b')
        if ELmin < 0: PL.axis([MJDmin, MJDmax, -90, 90])
        else: PL.axis([MJDmin, MJDmax, 0, 90])

        PL.axes([0.1, 0.1, 0.8, 0.35])
        PL.ylabel('Azimuth (deg)')
        PL.title('Azimuth Plot v.s. Time (with Detected large Gaps)')
        PL.xlabel('MJD (Day)')
        for Time in PGap:
            PL.axvline(x=Time, linewidth=0.5, color='g')
        PL.plot(MJD, Az, 'bo', markersize=2, markeredgecolor='b', markerfacecolor='b')
        PL.axis([MJDmin, MJDmax, 0, 360])

    if ShowPlot != False: PL.draw()
    if FigFile != False: PL.savefig(FigFile, format='png', dpi=DPISummary)

    del MJD, Az, El, TGap, PGap
    return



def DrawImage(data, Table, Abcissa, Lines, beamsize=0, gridsize=1, scale_max=False, scale_min=False, colormap='gray', ShowPlot=True, FigFileDir=False, FigFileRoot=False):
    '''
    data: numarray[NROW][NCHAN]
    Table: Table explaining data property
        [[IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]
         [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]
                 ......
         [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]]
    Lines: list of lines to be shown
        Lines[Ncluster] = [LineCenter, LineWidth, Validity]
        Validity: True or False
    beamsize: size of the main beam radius in degree
    colormap: 'gray', 'color'
    '''

    if ShowPlot == False and FigFileDir == False: return
    Nrow = len(data)
    Ncluster = len(Lines)
    Nchan = len(data[0])
    if NChannelMap == 12: (NhPanel, NvPanel) = (4, 3)
    elif NChannelMap == 15: (NhPanel, NvPanel) = (5, 3)
    else: (NhPanel, NvPanel) = (5, 3)
    TickSize = 6
    if ShowPlot: PL.ion()
    PL.figure(MATPLOTLIB_FIGURE_ID[7])
    if ShowPlot: PL.ioff()
    # 2008/9/20 Dec Effect has been taken into account
    Aspect = 1.0 / math.cos(Table[0][5] / 180. * 3.141592653)

    # Plotting routine
    (Xmin, Xmax, Ymin, Ymax) = (Table[0][2], Table[0][2], Table[0][3], Table[0][3])
    (RAmin, RAmax, DECmin, DECmax) = (Table[0][4], Table[0][4], Table[0][5], Table[0][5])
    for row in range(1, Nrow):
        if Xmin > Table[row][2]:
            Xmin = Table[row][2]
            RAmin = Table[row][4]
        elif Xmax < Table[row][2]:
            Xmax = Table[row][2]
            RAmax = Table[row][4]
        if Ymin > Table[row][3]:
            Ymin = Table[row][3]
            DECmin = Table[row][5]
        elif Ymax < Table[row][3]:
            Ymax = Table[row][3]
            DECmax = Table[row][5]
    # Swapped (x,y) to match the clustering result
    ExtentCM = ((Xmax+0.5)*gridsize*3600., (Xmin-0.5)*gridsize*3600., (Ymin-0.5)*gridsize*3600., (Ymax+0.5)*gridsize*3600.)
    Extent = (RAmax+gridsize/2.0, RAmin-gridsize/2.0, DECmin-gridsize/2.0, DECmax+gridsize/2.0)
    span = max(RAmax - RAmin + gridsize, DECmax - DECmin + gridsize)
    (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

    # Map = NA.zeros((NChannelMap, (Xmax - Xmin + 1), (Ymax - Ymin + 1)), type=NA.Float32)
    ##Map = NA.zeros((NChannelMap, (Ymax - Ymin + 1), (Xmax - Xmin + 1)), type=NA.Float32)
    Map = NP.zeros((NChannelMap, (Ymax - Ymin + 1), (Xmax - Xmin + 1)), dtype=NP.float32)
    ##RMSMap = NA.zeros(((Ymax - Ymin + 1), (Xmax - Xmin + 1)), type=NA.Float32)
    RMSMap = NP.zeros(((Ymax - Ymin + 1), (Xmax - Xmin + 1)), dtype=NP.float32)
    ##Total = NA.zeros(((Ymax - Ymin + 1), (Xmax - Xmin + 1)), type=NA.Float32)
    Total = NP.zeros(((Ymax - Ymin + 1), (Xmax - Xmin + 1)), dtype=NP.float32)
    ##ValidSp = NA.zeros(Nrow, type=NA.Float32)
    ValidSp = NP.zeros(Nrow, dtype=NP.float32)
    for row in range(Nrow):
        ValidSp[row] = math.sqrt(Table[row][6])
    ##Sp = NA.sum(NA.transpose((ValidSp * NA.transpose(data))))/NA.sum(ValidSp)
    Sp = NP.sum(NP.transpose((ValidSp * NP.transpose(data))),axis=0)/NP.sum(ValidSp,axis=0)
    F0 = min(Abcissa[1][0], Abcissa[1][Nchan - 1])
    F1 = max(Abcissa[1][0], Abcissa[1][Nchan - 1])
    # Check the direction Velocity axis
    if Abcissa[2][0] < Abcissa[2][1]: Reverse = True
    else: Reverse = False
    
    ValidCluster = 0
    for Nc in range(Ncluster):
        if Lines[Nc][2] != True: continue
        Title = []
        N = 0
        ChanC = int(Lines[Nc][0] + 0.5)
        VelC = Abcissa[2][ChanC]
        if ChanC > 0:
            ChanVelWidth = abs(Abcissa[2][ChanC] - Abcissa[2][ChanC - 1])
        else:
            ChanVelWidth = abs(Abcissa[2][ChanC] - Abcissa[2][ChanC + 1])

        # 2007/9/13 Change the magnification factor 1.2 to your preference (to Dirk)
        # be sure the width of one channel map is integer
        ChanW = max(int(Lines[Nc][1] * 1.4 / NChannelMap + 0.5), 1)
        ChanB = int(ChanC - NChannelMap / 2.0 * ChanW)
        # 2007/9/10 remedy for 'out of index' error
        if ChanB < 0:
            ChanW = int(ChanC * 2.0 / NChannelMap)
            if ChanW == 0: continue
            ChanB = int(ChanC - NChannelMap / 2.0 * ChanW)
        elif ChanB + ChanW * NChannelMap > Nchan:
            ChanW = int((Nchan - 1 - ChanC) * 2.0 / NChannelMap)
            if ChanW == 0: continue
            ChanB = int(ChanC - NChannelMap / 2.0 * ChanW)

        # Draw Total Intensity Map
        for row in range(Nrow):
            if Table[row][6] > 0:
                Total[int(Ymax-(Table[row][3]-Ymin))][int(Xmax-(Table[row][2]-Xmin))] = data[row][int(Lines[Nc][0]-Lines[Nc][1]/2):int(Lines[Nc][0]+Lines[Nc][1]/2)].sum() * ChanVelWidth
            else:
                Total[int(Ymax-1-(Table[row][3]-Ymin))][int(Xmax-1-(Table[row][2]-Xmin))] = 0.0
        PL.cla()
        PL.clf()
#        x = (NhPanel - 1) / 2.0 + 1.2
#        x0 = 1.0 / float(NhPanel) * (x + 0.1)
#        x1 = 1.0 / float(NhPanel) * 0.8
#        y = NvPanel
#        y0 = 1.0 / float((NvPanel+1)) * (y + 0.1)
#        y1 = 1.0 / float((NvPanel+1)) * 0.8
        x0 = 2.0 / 3.0 + 0.15 / 3.0
        x1 = 1.0 / 3.0 * 0.8
        y0 = 2.0 / 3.0 + 0.2 / 3.0
        y1 = 1.0 / 3.0 * 0.7
        a = PL.axes([x0, y0, x1, y1])
        # 2008/9/20 DEC Effect
        im = PL.imshow(Total, interpolation='nearest', aspect=Aspect, extent=Extent)
        #im = PL.imshow(Total, interpolation='nearest', aspect='equal', extent=Extent)

        a.xaxis.set_major_formatter(RAformatter)
        a.yaxis.set_major_formatter(DECformatter)
        a.xaxis.set_major_locator(RAlocator)
        a.yaxis.set_major_locator(DEClocator)
        xlabels = a.get_xticklabels()
        PL.setp(xlabels, 'rotation', RArotation, fontsize=TickSize)
        ylabels = a.get_yticklabels()
        PL.setp(ylabels, 'rotation', DECrotation, fontsize=TickSize)

#        PL.xticks(size=TickSize)
#        PL.yticks(size=TickSize)
        PL.xlabel('RA', size=TickSize)
        PL.ylabel('DEC', size=TickSize)
        if colormap == 'gray': PL.gray()
        else: PL.jet()
        
        # colorbar
        #print "min=%s, max of Total=%s" % (Total.min(),Total.max())
        if not (Total.min() == Total.max()): 
            if not ((Ymax == Ymin) and (Xmax == Xmin)): 
               cb=PL.colorbar(shrink=0.8)
               for t in cb.ax.get_yticklabels():
                   newfontsize = t.get_fontsize()*0.5
                   t.set_fontsize(newfontsize)
               cb.ax.set_title('[K km/s]')
               lab = cb.ax.title
               lab.set_fontsize(newfontsize)

        # draw beam pattern
        if beamsize > 0:
            Mark = 'r-'
            #R = beamsize / gridsize
            R = beamsize
            x = []
            y = []
            for t in range(50):
                # 2008/9/20 DEC Effect
                x.append(R * (math.sin(t * 0.13) + 1.0) * Aspect + RAmin)
                #x.append(R * (math.sin(t * 0.13) + 1.0) + RAmin)
                y.append(R * (math.cos(t * 0.13) + 1.0) + DECmin)
            PL.plot(x, y, Mark)

        #if not ((Ymax == Ymin) and (Xmax == Xmin)): PL.colorbar(ticks=[], shrink=0.8)
        #if not ((Ymax == Ymin) and (Xmax == Xmin)): 
        #    cb=PL.colorbar(shrink=0.8)
        #    for t in cb.ax.get_yticklabels():
        #        newfontsize = t.get_fontsize*0.8
        #        t.set_fontsize(newfotsize)
        PL.title('Total Intensity: CenterFreq.= %.3f GHz' % Abcissa[1][ChanC], size=TickSize)

        #Format = PL.FormatStrFormatter('%.3e')
        Format = PL.FormatStrFormatter('%.2f')
        # Plot Integrated Spectrum #1
#        x = (NhPanel - 1) / 2.0 - 1.2
#        x0 = 1.0 / float(NhPanel) * (x + 0.1)
#        x1 = 1.0 / float(NhPanel) * 0.8
        x0 = 1.0 / 3.0 + 0.1 / 3.0
        a = PL.axes([x0, y0, x1, y1])
        a.xaxis.set_major_formatter(Format)
        PL.plot(Abcissa[1], Sp, '-b', markersize=2, markeredgecolor='b', markerfacecolor='b')
        PL.axvline(x = Abcissa[1][ChanB], linewidth=0.3, color='r')
        PL.axvline(x = Abcissa[1][ChanB + NChannelMap * ChanW], linewidth=0.3, color='r')
        PL.xticks(size=TickSize)
        PL.yticks(size=TickSize)
        PL.xlabel('Frequency (GHz)', size=TickSize)
        PL.ylabel('Intensity (K)', size=TickSize)
        #PL.setp(xlabels, 'rotation', 45, fontsize=TickSize)
        Range = PL.axis()
        PL.axis([F0, F1, Range[2], Range[3]])
        PL.title('Integrated Spectrum', size=TickSize)

        # Plot Integrated Spectrum #2
#        x = (NhPanel - 1) / 2.0
#        x0 = 1.0 / float(NhPanel) * (x + 0.1)
#        x1 = 1.0 / float(NhPanel) * 0.8
        x0 = 0.1 / 3.0
        a = PL.axes([x0, y0, x1, y1])
        chan0 = max(int(Lines[Nc][0]-Lines[Nc][1]*1.3), 0)
        chan1 = min(int(Lines[Nc][0]+Lines[Nc][1]*1.3), Nchan)
        V0 = min(Abcissa[2][chan0], Abcissa[2][chan1 - 1]) - VelC
        V1 = max(Abcissa[2][chan0], Abcissa[2][chan1 - 1]) - VelC
        PL.plot(Abcissa[2][chan0:chan1] - VelC, Sp[chan0:chan1], '-b', markersize=2, markeredgecolor='b', markerfacecolor='b')
        for i in range(NChannelMap + 1):
            ChanL = int(ChanB + i*ChanW)
            if 0 <= ChanL and ChanL < Nchan:
                PL.axvline(x = Abcissa[2][ChanL] - VelC, linewidth=0.3, color='r')
        PL.xticks(size=TickSize)
        PL.yticks(size=TickSize)
        Range = PL.axis()
        PL.axis([V0, V1, Range[2], Range[3]])
        PL.xlabel('Relative Velocity (km/s)', size=TickSize)
        PL.ylabel('Intensity (K)', size=TickSize)
        PL.title('Integrated Spectrum (zoom)', size=TickSize)

        # Draw Channel Map
        NMap = 0
        Vmax0 = Vmin0 = 0
        for i in range(NChannelMap):
            if Reverse: ii = i
            else: ii = NChannelMap - i - 1
            C0 = ChanB + ChanW*ii
            C1 = C0 + ChanW
            if C0 < 0 or C1 >= Nchan - 1: continue
            V0 = (Abcissa[2][C0] + Abcissa[2][C1-1]) / 2.0 - VelC
            V1 = abs(Abcissa[2][C0] - Abcissa[2][C1])
            Title.append('(Vel,Wid) = (%.1f, %.1f) (km/s)' % (V0, V1))
            NMap += 1
            for row in range(Nrow):
                Intensity = data[row][C0:C1].sum() * ChanVelWidth
                if Table[row][6] > 0:
                    if N == 0: (Vmax0, Vmin0) = (Intensity, Intensity)
                    elif Intensity > Vmax0: Vmax0 = Intensity
                    elif Intensity < Vmin0: Vmin0 = Intensity
                    N += 1
                # Swapped (x,y) to match the clustering result
                # Map[i][Table[row][2] - Xmin][Table[row][3] - Ymin] = Intensity
                Map[i][Ymax - (Table[row][3] - Ymin)][Xmax - (Table[row][2] - Xmin)] = Intensity
        if type(scale_max) == bool: Vmax = Vmax0 - (Vmax0 - Vmin0) * 0.1
        else: Vmax = scale_max
        if type(scale_min) == bool: Vmin = Vmin0 + (Vmax0 - Vmin0) * 0.1
        else: Vmin = scale_min

        if Vmax == 0 and Vmin == 0: 
            print "No data to create channel maps. Check the flagging criteria."
            return
#        for i in range(NChannelMap):
        for i in range(NMap):
            x = i % NhPanel
            y = NvPanel - int(i / NhPanel) - 1
            x0 = 1.0 / float(NhPanel) * (x + 0.05)
            x1 = 1.0 / float(NhPanel) * 0.9
            y0 = 1.0 / float((NvPanel+2)) * (y + 0.05)
            y1 = 1.0 / float((NvPanel+2)) * 0.85
            a = PL.axes([x0, y0, x1, y1])
            a.set_aspect('equal')
            a.xaxis.set_major_locator(PL.NullLocator())
            a.yaxis.set_major_locator(PL.NullLocator())
            #im = PL.imshow(Map[i], vmin=Vmin, vmax=Vmax, interpolation='bilinear', aspect='equal', extent=Extent)
            if not (Vmax==Vmin):
                im = PL.imshow(Map[i], vmin=Vmin, vmax=Vmax, interpolation='nearest', aspect='equal', extent=ExtentCM)
#            PL.xticks(size=TickSize)
#            PL.yticks(size=TickSize)
#            PL.xlabel('RA (relative arcsec)', size=TickSize)
#            PL.ylabel('DEC (relative arcsec)', size=TickSize)
                if colormap == 'gray': PL.gray()
                else: PL.jet()
            #if x == (NhPanel - 1): PL.colorbar()
#            if i == (NChannelMap - 1): PL.colorbar()
                if x == (NhPanel - 1):
                    cb=PL.colorbar()
                    for t in cb.ax.get_yticklabels():
                        newfontsize = t.get_fontsize()*0.5
                        t.set_fontsize(newfontsize)
                    cb.ax.set_title('[K km/s]')
                    lab=cb.ax.title
                    lab.set_fontsize(newfontsize)

                PL.title(Title[i], size=TickSize)
            

        if ShowPlot != False: PL.draw()
        if FigFileDir != False:
            PL.savefig(FigFileDir+FigFileRoot+'_%s.png' % ValidCluster, format='png', dpi=DPIDetail)
            #os.rename(FigFileDir+FigFileRoot+'_%s.png' % ValidCluster, FigFileDir+FigFileRoot+'_%s.gif' % ValidCluster)
            if os.access(FigFileDir+'listofplots.txt', os.F_OK):
                BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
            else:
                BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
                print >> BrowserFile, 'TITLE: Gridding'
                print >> BrowserFile, 'FIELDS: Result IF POL Iteration Page'
                print >> BrowserFile, 'COMMENT: Combined spectra by spatial Gridding'
            print >> BrowserFile, FigFileRoot+'_%s.png' % ValidCluster
            #print >> BrowserFile, FigFileRoot+'_%s.gif' % ValidCluster
            BrowserFile.close()
        ValidCluster += 1


    # Draw RMS Map
    for row in range(Nrow):
        if Table[row][6] > 0:
            RMSMap[Ymax-(Table[row][3]-Ymin)][Xmax-(Table[row][2]-Xmin)] = Table[row][8]
        else:
            RMSMap[Ymax-1-(Table[row][3]-Ymin)][Xmax-1-(Table[row][2]-Xmin)] = 0.0
    PL.cla()
    PL.clf()
    a = PL.axes([0.25, 0.25, 0.5, 0.5])
    # 2008/9/20 DEC Effect
    im = PL.imshow(RMSMap, interpolation='nearest', aspect=Aspect, extent=Extent)
    #im = PL.imshow(RMSMap, interpolation='nearest', aspect='equal', extent=Extent)

    a.xaxis.set_major_formatter(RAformatter)
    a.yaxis.set_major_formatter(DECformatter)
    a.xaxis.set_major_locator(RAlocator)
    a.yaxis.set_major_locator(DEClocator)
    xlabels = a.get_xticklabels()
    PL.setp(xlabels, 'rotation', RArotation, fontsize=8)
    ylabels = a.get_yticklabels()
    PL.setp(ylabels, 'rotation', DECrotation, fontsize=8)

    PL.xlabel('RA', size=12)
    PL.ylabel('DEC', size=12)
    if colormap == 'gray': PL.gray()
    else: PL.jet()
        
    # colorbar
    if not (RMSMap.min() == RMSMap.max()): 
        if not ((Ymax == Ymin) and (Xmax == Xmin)): 
           #cb=PL.colorbar()
           cb=PL.colorbar(shrink=0.8)
           #for t in cb.ax.get_yticklabels():
           #    newfontsize = t.get_fontsize()*0.5
           #    t.set_fontsize(newfontsize)
           cb.ax.set_title('[K]')
           lab = cb.ax.title
           #lab.set_fontsize(newfontsize)

    # draw beam pattern
    if beamsize > 0:
        Mark = 'r-'
        #R = beamsize / gridsize
        R = beamsize
        x = []
        y = []
        for t in range(50):
            # 2008/9/20 DEC Effect
            x.append(R * (math.sin(t * 0.13) + 1.0) * Aspect + RAmin)
            #x.append(R * (math.sin(t * 0.13) + 1.0) + RAmin)
            y.append(R * (math.cos(t * 0.13) + 1.0) + DECmin)
        PL.plot(x, y, Mark)

    PL.title('Baseline RMS Map', size=12)

    if ShowPlot != False: PL.draw()
    if FigFileDir != False:
        PL.savefig(FigFileDir+FigFileRoot+'_rmsmap.png', format='png', dpi=DPISummary)
        if os.access(FigFileDir+'listofplots.txt', os.F_OK):
            BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
        else:
            BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
            print >> BrowserFile, 'TITLE: Gridding'
            print >> BrowserFile, 'FIELDS: Result IF POL Iteration Page'
            print >> BrowserFile, 'COMMENT: Combined spectra by spatial Gridding'
        print >> BrowserFile, FigFileRoot+'_rmsmap.png'
        BrowserFile.close()


    return
