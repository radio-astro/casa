#*******************************************************************************
# ALMA - Atacama Large Millimeter Array
# Copyright (c) NAOJ - National Astronomical Observatory of Japan, 2011
# (in the framework of the ALMA collaboration).
# All rights reserved.
# 
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.
# 
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
#*******************************************************************************
#
# $Revision: 1.10.2.2.2.1 $
# $Date: 2012/09/19 20:58:59 $
# $Author: ldavis $
#
from asap import *
#import casac
from casac import casac
from SDTool import SDLogger
#import numarray as NA
import numpy as NP
import pylab as PL
import math
import os
from matplotlib.ticker import FuncFormatter, MultipleLocator, AutoLocator
from matplotlib.font_manager import FontProperties 

# 0:DebugPlot 1:TPlotRADEC 2:TPlotAzEl 3:TPlotCluster 4:TplotFit 5:TPlotMultiSP 6:TPlotSparseSP 7:TPlotChannelMap 8:TPlotFlag 9:TPlotIntermediate
MATPLOTLIB_FIGURE_ID = [8904, 8905, 8906, 8907, 8908, 8909, 8910, 8911, 8912, 8913]

MAX_NhPanel = 5
MAX_NvPanel = 5
#MAX_NhPanelFit = 3
MAX_NhPanelFit = 4
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

dsyb = '$^\circ$'
hsyb = ':'
msyb = ':'

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
    #return '%02dh%02dm' % (h, m)
    return '%02d%s%02d' % (h, hsyb, m)

def HHMMSS(x, pos):
    # HHMMSS format
    (h, m, s) = Deg2HMS(x, 1/2400.0)
    #return '%02dh%02dm%02ds' % (h, m, s)
    return '%02d%s%02d%s%02d' % (h, hsyb, m, msyb, s)

def HHMMSSs(x, pos):
    # HHMMSS.s format
    (h, m, s) = Deg2HMS(x, 1/24000.0)
    #return '%02dh%02dm%04.1fs' % (h, m, s)
    return '%02d%s%02d%s%04.1f' % (h, hsyb, m, msyb, s)

def HHMMSSss(x, pos):
    # HHMMSS.ss format
    (h, m, s) = Deg2HMS(x, 1/240000.0)
    #return '%02dh%02dm%05.2fs' % (h, m, s)
    return '%02d%s%02d%s%05.2f' % (h, hsyb, m, msyb, s)

def HHMMSSsss(x, pos):
    # HHMMSS.sss format
    (h, m, s) = Deg2HMS(x, 1/2400000.0)
    #return '%02dh%02dm%06.3fs' % (h, m, s)
    return '%02d%s%02d%s%06.3f' % (h, hsyb, m, msyb, s)


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
    #return '%+02dd%02dm' % (d, m)
    return '%+02d%s%02d\'' % (d, dsyb, m)

def DDMMSS(x, pos):
    # +DDMMSS format
    (d, m, s) = Deg2DMS(x, 1/36000.0)
    #return '%+02dd%02dm%02ds' % (d, m, s)
    return '%+02d%s%02d\'%02d\"' % (d, dsyb, m, s)

def DDMMSSs(x, pos):
    # +DDMMSS.s format
    (d, m, s) = Deg2DMS(x, 1/360000.0)
    #return '%+02dd%02dm%04.1fs' % (d, m, s)
    sint = int(s)
    sstr = ('%3.1f'%(s-int(s))).lstrip('0')
    return '%+02d%s%02d\'%02d\"%s' % (d, dsyb, m, sint, sstr)

def DDMMSSss(x, pos):
    # +DDMMSS.ss format
    (d, m, s) = Deg2DMS(x, 1/3600000.0)
    #return '%+02dd%02dm%05.2fs' % (d, m, s)
    sint = int(s)
    sstr = ('%4.2f'%(s-int(s))).lstrip('0')
    return '%+02d%s%02d\'%02d\"%s' % (d, dsyb, m, sint, sstr)

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
        os.system('convert %s -trim %s' % (OldPlot, NewPlot))

    del data, ScaleOut
    return


def DrawMultiSpectra(DataIn, Table, Abcissa, rows, mode='mode', chan0=-1, chan1=-1, FixScale=True, scale_max=1, scale_min=0, connect=True, Pass=False, ShowPlot=True, FigFileDir=False, FigFileRoot=False):
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

    # read data to storage
    NROW = len(rows)
##     NCHAN = len(Abcissa[1])
##     data = NP.zeros((NROW, NCHAN), dtype=NP.float32)
##     _tbtool = casac.homefinder.find_home_by_name('tableHome')
##     _tb = _tbtool.create()
##     _tb.open(DataIn)
##     for x in range(NROW):
##         data[x] = _tb.getcell('SPECTRA', x)
##     _tb.close()
##     del _tbtool, _tb
    (NCHAN,data,Frequency,Velocity) = GetDataFromFile( DataIn, NROW, False )

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
        ROWS = NP.zeros(NH * NV * NhPanel * NvPanel, dtype=NP.int) - 1
        # 2010/6/15 GK Change the plotting direction: UpperLeft->UpperRight->OneLineDown repeat...
        for row in rows:
            GlobalPosX = (Xmax - Table[row][2]) / NhPanel
            GlobalOfstX = (Xmax - Table[row][2]) % NhPanel
            GlobalPosY = (Ymax - Table[row][3]) / NvPanel
            GlobalOfstY = NvPanel - 1 - (Ymax - Table[row][3]) % NvPanel
            #GlobalPosY = (Table[row][3] - Ymin) / NvPanel
            #GlobalOfstY = (Table[row][3] - Ymin) % NvPanel
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
##     xmin = min(Abcissa[1][chan0], Abcissa[1][chan1])
##     xmax = max(Abcissa[1][chan0], Abcissa[1][chan1])
    xmin = min(Frequency[chan0], Frequency[chan1])
    xmax = max(Frequency[chan0], Frequency[chan1])

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
        ymax = NP.sort(ListMax)[len(ListMax) - len(ListMax)/10 - 1]
        ymin = NP.sort(ListMin)[len(ListMin)/10]
        ymax = ymax + (ymax - ymin) * 0.2
        ymin = ymin - (ymax - ymin) * 0.1
        del ListMax, ListMin
    else:
        ymin = scale_min
        ymax = scale_max

    if mode.upper() != 'RASTER': ROWS = rows[:]

    NSp = 0
##     xtick = abs(Abcissa[1][-1] - Abcissa[1][0]) / 4.0
    xtick = abs(Frequency[-1] - Frequency[0]) / 4.0
    Order = int(math.floor(math.log10(xtick)))
    NewTick = int(xtick / (10**Order) + 1) * (10**Order)
    FreqLocator = MultipleLocator(NewTick)
    if Order < 0: FMT = '%%.%dfG' % (-Order)
    else: FMT = '%.2fG'
    Format = PL.FormatStrFormatter(FMT)

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
            PL.ylabel('Intensity (K)', size=TickSize)
            if Table[row][6] > 0:
##                 PL.plot(Abcissa[1], data[row], Mark, markersize=2, markeredgecolor='b', markerfacecolor='b')
                PL.plot(Frequency, data[row], Mark, markersize=2, markeredgecolor='b', markerfacecolor='b')
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
                if os.access(FigFileDir+'listofplots.txt', os.F_OK):
                    BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
                else:
                    BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
                    print >> BrowserFile, 'TITLE: MultiSpectra'
                    print >> BrowserFile, 'FIELDS: Result IF POL Iteration Page'
                    print >> BrowserFile, 'COMMENT: Combined spectra by spatial Gridding'
                print >> BrowserFile, FigFileRoot+'_%s.png' % Npanel
                BrowserFile.close()
            Npanel += 1
    del ROWS, data
    return
            

## def DrawSparseSpectra(DataIn, Table, Abcissa, Npanel, chan0=-1, chan1=-1, AutoScale=True, scale_max=1, scale_min=0, ShowPlot=True, FigFileDir=False, FigFileRoot=False):
def DrawSparseSpectra(DataIn, Table, Abcissa, Npanel, chan0=-1, chan1=-1, AutoScale=True, scale_max=1, scale_min=0, ShowPlot=True, FigFileDir=False, FigFileRoot=False):
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

    # read data to storage
    NROW = len(Table)
## ##     NCHAN = len(Abcissa[1])
##     s = scantable(DataIn, average=False)
##     unitorg = s.get_unit()
##     s.set_unit('GHz')
##     Abcissa = NP.array(s._getabcissa(0))
##     NCHAN = len(Abcissa)
##     s.set_unit(unitorg)
##     data = NP.zeros((NROW, NCHAN), dtype=NP.float32)
## ##     _tbtool = casac.homefinder.find_home_by_name('tableHome')
## ##     _tb = _tbtool.create()
## ##     _tb.open(DataIn)
##     for x in range(NROW):
##         data[x] = NP.array(s._getspectrum(x))
## ##         data[x] = _tb.getcell('SPECTRA', x)
## ##     _tb.close()
## ##     del _tbtool, _tb
##     del s
    (NCHAN, data, Frequency, Velocity) = GetDataFromFile( DataIn, NROW, False )

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
##     xmin = min(Abcissa[1][chan0], Abcissa[1][chan1-1])
##     xmax = max(Abcissa[1][chan0], Abcissa[1][chan1-1])
    xmin = min(Frequency[chan0], Frequency[chan1-1])
    xmax = max(Frequency[chan0], Frequency[chan1-1])

    TickSize = 10 - Npanel/2
    Format = PL.FormatStrFormatter('%.2f')

    Plot = NP.zeros((Npanel, Npanel, (chan1 - chan0)), NP.float32) + NoData
    TotalSP = NP.zeros(chan1-chan0, NP.float32)
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
        ymax = NP.sort(ListMax)[len(ListMax) - len(ListMax)/10 - 1]
        ymin = NP.sort(ListMin)[len(ListMin)/10]
        ymax = ymax + (ymax - ymin) * 0.2
        ymin = ymin - (ymax - ymin) * 0.1
        del ListMax, ListMin
    else:
        ymin = scale_min
        ymax = scale_max

    PL.cla()
    PL.clf()
    a0 = PL.subplot((NV+3)/2+3, 1, 1)
    a0.xaxis.set_major_formatter(Format)
##     PL.plot(Abcissa[1][chan0:chan1], TotalSP, color='b', linestyle='-', linewidth=0.4)
    PL.plot(Frequency[chan0:chan1], TotalSP, color='b', linestyle='-', linewidth=0.4)
    PL.xlabel('Frequency(GHz)', size=(TickSize+1))
    PL.ylabel('Intensity(K)', size=(TickSize+1))
    PL.xticks(size=TickSize)
    PL.yticks(size=TickSize)
    PL.title('Spatially Integrated Spectrum', size=(TickSize + 1))

    for x in range(NH):
        for y in range(NV):
            a1 = PL.subplot(NV+3, NH+1, (NV - 1 - y + 2) * (NH + 1) + (NH - 1 - x) + 2)
            if Plot[x][y].min() > NoDataThreshold:
##                 PL.plot(Abcissa[1][chan0:chan1], Plot[x][y], color='b', linestyle='-', linewidth=0.2)
                PL.plot(Frequency[chan0:chan1], Plot[x][y], color='b', linestyle='-', linewidth=0.2)
            else:
                PL.text((xmin+xmax)/2.0, (ymin+ymax)/2.0, 'NO DATA', horizontalalignment='center', verticalalignment='center', size=(TickSize + 1))
            a1.yaxis.set_major_locator(PL.NullLocator())
            a1.xaxis.set_major_locator(PL.NullLocator())
            PL.axis([xmin, xmax, ymin, ymax])
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
        if os.access(FigFileDir+'listofplots.txt', os.F_OK):
            BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
        else:
            BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
            print >> BrowserFile, 'TITLE: SparseSpectraMap'
            print >> BrowserFile, 'FIELDS: Result IF POL Iteration Page'
            print >> BrowserFile, 'COMMENT: Sparse Spectra Map'
        print >> BrowserFile, FigFileRoot+'_0.png'
        BrowserFile.close()
    del data

    return



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


def ShowClusterScore(Ncluster, Score, ShowPlot=True, FigFileDir=False, FigFileRoot=False):
    """
    """
    if ShowPlot == False and FigFileDir == False: return

    if ShowPlot: PL.ion()
    PL.figure(MATPLOTLIB_FIGURE_ID[3])
    if ShowPlot: PL.ioff()
    PL.cla()
    PL.clf()

    PL.plot(Ncluster, Score, 'bx', markersize=10)
    [xmin, xmax, ymin, ymax] = PL.axis()
    PL.xlabel('Number of Clusters', fontsize=11)
    PL.ylabel('Score (Lower is better)', fontsize=11)
    PL.title('Score are plotted versus number of the cluster', fontsize=11)
    PL.axis([0, xmax+1, ymin, ymax])

    if ShowPlot != False: PL.draw()

    if FigFileDir != False:
        PL.savefig(FigFileDir+FigFileRoot+'_ClusterScore.png', format='png', dpi=DPIDetail)
    return


def ShowClusterInChannelSpace(data, BestLines, Scale=1.0, ShowPlot=True, FigFileDir=False, FigFileRoot=False):
    """
    Plot Clusters in Channel Space (plot center and width of detected lines)
    data: numpy array: [[Wid0, Cent0], [Wid1, Cent1], ..., [WidN, CentN]]
    BestLines: [[ClusterCent0, ClusterWid0, True, Radius], ...,[]]
    Scale: Vertical scaling factor for the clustering circle
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
    [X, Y] = NP.array(data.transpose())
    PL.plot(Y, X, 'bs', markersize=1)
    [xmin, xmax, ymin, ymax] = PL.axis()
    Mark = 'r-'
    #cx = []
    #cy = []
    i = 0
    for [CX, CY, dummy, R] in BestLinesCopy:
        #cx.append(CX)
        #cy.append(CY)
        x = []
        y = []
        for t in range(50):
            x.append(CX + R * math.sin(t * 0.13))
            y.append(CY * Scale + R * math.cos(t * 0.13) * Scale)
        PL.plot(x, y, Mark)
        PL.text(CX, CY * Scale, str(i), fontsize=10, color='red') 
        i += 1
    PL.xlabel('Line Center (Channel)', fontsize=11)
    PL.ylabel('Line Width (Channel)', fontsize=11)
    PL.axis([xmin, xmax, 0, ymax])
    PL.title('Clusters in the line Center-Width space\n\nRed Oval(s) shows each clustering region. Size of the oval represents cluster radius', fontsize=11)

    if ShowPlot != False: PL.draw()

    if FigFileDir != False:
        PL.savefig(FigFileDir+FigFileRoot+'_ChannelSpace.png', format='png', dpi=DPIDetail)
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
        y0 = 0.8 / float(NvPanel) * (NvPanel - y - 1 + 0.3)
        
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
	PL.title('Cluster%s: Center=%.4f GHz Width=%.1f km/s' % \
	     (Cluster, Abcissa[1][int(Lines[Cluster][0]+0.5)], abs(Abcissa[2][int(Lines[Cluster][1]+0.5)] - Abcissa[2][0])), size=(TickSize + 1))
	PL.xlabel('RA', size=TickSize)
        PL.ylabel('Dec', size=TickSize)
        if Lines[Cluster][2] != False:
            for i in range(len(Threshold)):
                PL.plot(xdata[i], ydata[i], Marks[4 - len(Threshold) + i], markersize=msize)
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
        (RAtext, DECtext, Unit) = (ScaleRA/Aspect, ScaleDEC, 'degree')
    elif 1.0 <= ScaleRA * 60.0:
        (RAtext, DECtext, Unit) = (ScaleRA/Aspect * 60.0, ScaleDEC * 60.0, 'arcmin')
    else:
        (RAtext, DECtext, Unit) = (ScaleRA/Aspect * 3600.0, ScaleDEC * 3600.0, 'arcsec')
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
        if os.access(FigFileDir+'listofplots.txt', os.F_OK):
            BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
        else:
            BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
            print >> BrowserFile, 'TITLE: Clustering'
            print >> BrowserFile, 'FIELDS: Cluster IF POL Iteration process'
            print >> BrowserFile, 'COMMENT: '
        print >> BrowserFile, FigFileRoot+'_%s.png' % mode.lower()
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
            PL.plot(x, y, Mark, markersize=4, markeredgecolor='r', markerfacecolor='r')
    # Swap RA direction
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
    #_qatool = casac.homefinder.find_home_by_name('quantaHome')
    #_qa = _qatool.create()
    _qa = casac.quanta()

    for gap in TimeGap:
        if gap > rows[-1]: break
	if gap == 0: continue
        TGap.append((Table[gap - 1][6] + Table[gap][6]) / 2.)
    for gap in TimeGap:
        if gap > rows[-1]: break
	if gap == 0: continue
        PGap.append((Table[gap - 1][6] + Table[gap][6]) / 2.)
    for ID in rows:
        Az.append(Table[ID][11])
        El.append(Table[ID][12])
        MJD.append(Table[ID][6])
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
            UTdata = (NP.array(MJDArr[nd])-int(MJDArr[nd][0]))*24.0
            if nd == 0:
                UTmin = min(UTdata)
                UTmax = max(UTdata)
            else:
                if min(UTdata) < UTmin: UTmin = min(UTdata)
                if max(UTdata) > UTmax: UTmax = max(UTdata)

            #date = _qa.quantity(MJDArr[nd][0],'d')
            date = _qa.quantity(str(MJDArr[nd][0])+'d')
            (datelab,rest) = _qa.time(date,form='dmy').split('/')  
            
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
            UTdata = (NP.array(MJDArr[nd])-int(MJDArr[nd][0]))*24.0
            date = _qa.quantity(str(MJDArr[nd][0])+'d')
            (datelab,rest) = _qa.time(date,form='dmy').split('/')  
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
    #del _qatool, _qa
    del _qa
    return



def DrawImage(DataIn, Table, Abcissa, Lines, beamsize=0, gridsize=1, scale_max=False, scale_min=False, colormap='gray', ShowPlot=True, FigFileDir=False, FigFileRoot=False):
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

    # Read data
    NROW = len(Table)
## ##     NCHAN = len(Abcissa[1])
##     s = scantable(DataIn, average=False)
##     unitorg = s.get_unit()
##     s.set_unit('GHz')
##     Abcissa = NP.array(s._getabcissa(0))
##     NCHAN = len(Abcissa)
##     RestFreq = 0.0
##     RestFreqs = s.get_restfreqs().values()[0]
##     if ( len(RestFreqs) != 0 ):
##         RestFreq = RestFreqs[0]/1.0e9
##     else:
##         RestFreq = Abcissa[0]
##     Velocity = (1.0 - (Abcissa / RestFreq)) * 299792.458

##     s.set_unit(unitorg)
##     data = NP.zeros((NROW, NCHAN), dtype=NP.float32)
## ##     _tbtool = casac.homefinder.find_home_by_name('tableHome')
## ##     _tb = _tbtool.create()
## ##     _tb.open(DataIn)
##     for x in range(NROW):
##         data[x] = s._getspectrum(x)
## ##         data[x] = _tb.getcell('SPECTRA', x)
## ##     _tb.close()
## ##     del _tbtool, _tb
##     del s
    (NCHAN, data, Frequency, Velocity) = GetDataFromFile( DataIn, NROW, True )

    # Number of the clusters determined in Process4
    Ncluster = len(Lines)
    # 2010/6/9 in the case of non-detection of the lines
    if Ncluster == 0: return

    # Determine Plotting Area Size
    (Xmin, Xmax, Ymin, Ymax) = (Table[0][2], Table[0][2], Table[0][3], Table[0][3])
    (RAmin, RAmax, DECmin, DECmax) = (Table[0][4], Table[0][4], Table[0][5], Table[0][5])
    for row in range(1, NROW):
        if   Xmin > Table[row][2]: (Xmin, RAmin) = (Table[row][2], Table[row][4])
        elif Xmax < Table[row][2]: (Xmax, RAmax) = (Table[row][2], Table[row][4])
        if   Ymin > Table[row][3]: (Ymin, DECmin) = (Table[row][3], Table[row][5])
        elif Ymax < Table[row][3]: (Ymax, DECmax) = (Table[row][3], Table[row][5])

    # Set data
    Map = NP.zeros((NChannelMap, (Ymax - Ymin + 1), (Xmax - Xmin + 1)), dtype=NP.float32)
    RMSMap = NP.zeros(((Ymax - Ymin + 1), (Xmax - Xmin + 1)), dtype=NP.float32)
    Total = NP.zeros(((Ymax - Ymin + 1), (Xmax - Xmin + 1)), dtype=NP.float32)
    # ValidSp: SQRT of Number of combined spectra for the weight
    ValidSp = NP.zeros(NROW, dtype=NP.float32)
    for row in range(NROW): ValidSp[row] = math.sqrt(Table[row][6])
    # Integrated Spectrum
    Sp = NP.sum(NP.transpose((ValidSp * NP.transpose(data))),axis=0)/NP.sum(ValidSp,axis=0)
##     (F0, F1) = (min(Abcissa[1][0], Abcissa[1][NCHAN - 1]), max(Abcissa[1][0], Abcissa[1][NCHAN - 1]))
    (F0, F1) = (min(Frequency[0], Frequency[NCHAN - 1]), max(Frequency[0], Frequency[NCHAN - 1]))

    # Swap (x,y) to match the clustering result
    ExtentCM = ((Xmax+0.5)*gridsize*3600., (Xmin-0.5)*gridsize*3600., (Ymin-0.5)*gridsize*3600., (Ymax+0.5)*gridsize*3600.)
    Extent = (RAmax+gridsize/2.0, RAmin-gridsize/2.0, DECmin-gridsize/2.0, DECmax+gridsize/2.0)
    span = max(RAmax - RAmin + gridsize, DECmax - DECmin + gridsize)
    (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

    # How to coordinate the map
    if NChannelMap == 12: (NhPanel, NvPanel) = (4, 3)
    elif NChannelMap == 15: (NhPanel, NvPanel) = (5, 3)
    else: (NhPanel, NvPanel) = (5, 3)
    TickSize = 6
    if ShowPlot: PL.ion()
    PL.figure(MATPLOTLIB_FIGURE_ID[7])
    if ShowPlot: PL.ioff()
    # 2008/9/20 Dec Effect has been taken into account
    Aspect = 1.0 / math.cos(Table[0][5] / 180. * 3.141592653)

    # Check the direction of the Velocity axis
##     if Abcissa[2][0] < Abcissa[2][1]: Reverse = True
    if Velocity[0] < Velocity[1]: Reverse = True
    else: Reverse = False
    
    ValidCluster = 0
    for Nc in range(Ncluster):
        if Lines[Nc][2] != True: continue
        Title = []
        N = 0
        ChanC = int(Lines[Nc][0] + 0.5)
        #VelC = Abcissa[2][ChanC]
        if float(ChanC) == Lines[Nc][0]:
##             VelC = Abcissa[2][ChanC]
            VelC = Velocity[ChanC]
        else:
##             VelC = 0.5 * ( Abcissa[2][ChanC] + Abcissa[2][ChanC-1] )
             VelC = 0.5 * ( Velocity[ChanC] + Velocity[ChanC-1] )
        if ChanC > 0:
##             ChanVelWidth = abs(Abcissa[2][ChanC] - Abcissa[2][ChanC - 1])
            ChanVelWidth = abs(Velocity[ChanC] - Velocity[ChanC - 1])
        else:
##             ChanVelWidth = abs(Abcissa[2][ChanC] - Abcissa[2][ChanC + 1])
            ChanVelWidth = abs(Velocity[ChanC] - Velocity[ChanC + 1])

        # 2007/9/13 Change the magnification factor 1.2 to your preference (to Dirk)
        # be sure the width of one channel map is integer
        ChanW = max(int(Lines[Nc][1] * 1.4 / NChannelMap + 0.5), 1)
        #ChanB = int(ChanC - NChannelMap / 2.0 * ChanW)
        ChanB = int(ChanC - NChannelMap / 2.0 * ChanW + 0.5)
        # 2007/9/10 remedy for 'out of index' error
        #print '\nDEBUG0: Nc, ChanB, ChanW, NchanMap', Nc, ChanB, ChanW, NChannelMap
        if ChanB < 0:
            ChanW = int(ChanC * 2.0 / NChannelMap)
            if ChanW == 0: continue
            ChanB = int(ChanC - NChannelMap / 2.0 * ChanW)
        elif ChanB + ChanW * NChannelMap > NCHAN:
            ChanW = int((NCHAN - 1 - ChanC) * 2.0 / NChannelMap)
            if ChanW == 0: continue
            ChanB = int(ChanC - NChannelMap / 2.0 * ChanW)
        #print 'DEBUG1: Nc, ChanB, ChanW, NchanMap', Nc, ChanB, ChanW, NChannelMap, '\n'

        # Draw Total Intensity Map
        for row in range(NROW):
            if Table[row][6] > 0:
                Total[int(Ymax-(Table[row][3]-Ymin))][int(Xmax-(Table[row][2]-Xmin))] = data[row][int(Lines[Nc][0]-Lines[Nc][1]/2):int(Lines[Nc][0]+Lines[Nc][1]/2)].sum() * ChanVelWidth
            else:
                Total[int(Ymax-1-(Table[row][3]-Ymin))][int(Xmax-1-(Table[row][2]-Xmin))] = 0.0
        PL.cla()
        PL.clf()

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

##         PL.title('Total Intensity: CenterFreq.= %.3f GHz' % Abcissa[1][ChanC], size=TickSize)
        PL.title('Total Intensity: CenterFreq.= %.3f GHz' % Frequency[ChanC], size=TickSize)

        Format = PL.FormatStrFormatter('%.2f')
        # Plot Integrated Spectrum #1
        x0 = 1.0 / 3.0 + 0.1 / 3.0
        a = PL.axes([x0, y0, x1, y1])
        a.xaxis.set_major_formatter(Format)
##         PL.plot(Abcissa[1], Sp, '-b', markersize=2, markeredgecolor='b', markerfacecolor='b')
##         PL.axvline(x = Abcissa[1][ChanB], linewidth=0.3, color='r')
##         PL.axvline(x = Abcissa[1][ChanB + NChannelMap * ChanW], linewidth=0.3, color='r')
        PL.plot(Frequency, Sp, '-b', markersize=2, markeredgecolor='b', markerfacecolor='b')
        PL.axvline(x = Frequency[ChanB], linewidth=0.3, color='r')
        PL.axvline(x = Frequency[ChanB + NChannelMap * ChanW], linewidth=0.3, color='r')
        #print 'DEBUG: Freq0, Freq1', Frequency[ChanB], Frequency[ChanB + NChannelMap * ChanW]
        PL.xticks(size=TickSize)
        PL.yticks(size=TickSize)
        PL.xlabel('Frequency (GHz)', size=TickSize)
        PL.ylabel('Intensity (K)', size=TickSize)
        #PL.setp(xlabels, 'rotation', 45, fontsize=TickSize)
        Range = PL.axis()
        PL.axis([F0, F1, Range[2], Range[3]])
        PL.title('Integrated Spectrum', size=TickSize)

        # Plot Integrated Spectrum #2
        x0 = 0.1 / 3.0
        a = PL.axes([x0, y0, x1, y1])
        #chan0 = max(int(Lines[Nc][0]-Lines[Nc][1]*1.3), 0)
        #chan1 = min(int(Lines[Nc][0]+Lines[Nc][1]*1.3), NCHAN)
##         V0 = min(Abcissa[2][chan0], Abcissa[2][chan1 - 1]) - VelC
##         V1 = max(Abcissa[2][chan0], Abcissa[2][chan1 - 1]) - VelC
        #V0 = min(Velocity[chan0], Velocity[chan1 - 1]) - VelC
        #V1 = max(Velocity[chan0], Velocity[chan1 - 1]) - VelC
        #print 'DEBUG: chan0, chan1', chan0, chan1
        #print 'DEBUG: V0, V1', V0, V1
        # 2011/11/21
        chan0 = max(ChanB-1, 0)
        chan1 = min(ChanB + NChannelMap*ChanW, NCHAN-1)
        V0 = min(Velocity[chan0], Velocity[chan1]) - VelC
        V1 = max(Velocity[chan0], Velocity[chan1]) - VelC
        #print 'chan0, chan1, V0, V1, VelC =', chan0, chan1, V0, V1, VelC
        #print Abcissa[2][chan0:chan1]
        #print Sp[chan0:chan1]
        #print Table
##         PL.plot(Abcissa[2][chan0:chan1] - VelC, Sp[chan0:chan1], '-b', markersize=2, markeredgecolor='b', markerfacecolor='b')
        PL.plot(Velocity[chan0:chan1] - VelC, Sp[chan0:chan1], '-b', markersize=2, markeredgecolor='b', markerfacecolor='b')
        for i in range(NChannelMap + 1):
            ChanL = int(ChanB + i*ChanW)
            #if 0 <= ChanL and ChanL < NCHAN:
            #    PL.axvline(x = Abcissa[2][ChanL] - VelC, linewidth=0.3, color='r')
            if 0 < ChanL and ChanL < NCHAN:
##                 PL.axvline(x = 0.5*(Abcissa[2][ChanL]+Abcissa[2][ChanL-1]) - VelC, linewidth=0.3, color='r')
                PL.axvline(x = 0.5*(Velocity[ChanL]+Velocity[ChanL-1]) - VelC, linewidth=0.3, color='r')
            elif ChanL == 0:
##                 PL.axvline(x = 0.5*(Abcissa[2][ChanL]-Abcissa[2][ChanL+1]) - VelC, linewidth=0.3, color='r')
                PL.axvline(x = 0.5*(Velocity[ChanL]-Velocity[ChanL+1]) - VelC, linewidth=0.3, color='r')
            #print 'DEBUG: Vel[ChanL]', i, (Velocity[ChanL]+Velocity[ChanL-1])/2.0 - VelC
        PL.xticks(size=TickSize)
        PL.yticks(size=TickSize)
        Range = PL.axis()
        PL.axis([V0, V1, Range[2], Range[3]])
        PL.xlabel('Relative Velocity w.r.t. Window Center (km/s)', size=TickSize)
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
            if C0 < 0 or C1 >= NCHAN - 1: continue
##             V0 = (Abcissa[2][C0] + Abcissa[2][C1-1]) / 2.0 - VelC
##             V1 = abs(Abcissa[2][C0] - Abcissa[2][C1])
            V0 = (Velocity[C0] + Velocity[C1-1]) / 2.0 - VelC
            V1 = abs(Velocity[C0] - Velocity[C1])
            Title.append('(Vel,Wid) = (%.1f, %.1f) (km/s)' % (V0, V1))
            NMap += 1
            for row in range(NROW):
                Intensity = data[row][C0:C1].sum() * ChanVelWidth
                # Check the number of accumulated spectra
                if Table[row][6] > 0:
                    if N == 0: (Vmax0, Vmin0) = (Intensity, Intensity)
                    elif Intensity > Vmax0: Vmax0 = Intensity
                    elif Intensity < Vmin0: Vmin0 = Intensity
                    N += 1
                # Swapped (x,y) to match the clustering result
                Map[i][Ymax - (Table[row][3] - Ymin)][Xmax - (Table[row][2] - Xmin)] = Intensity
        if type(scale_max) == bool: Vmax = Vmax0 - (Vmax0 - Vmin0) * 0.1
        else:                       Vmax = scale_max
        if type(scale_min) == bool: Vmin = Vmin0 + (Vmax0 - Vmin0) * 0.1
        else:                       Vmin = scale_min

        if Vmax == 0 and Vmin == 0: 
            print "No data to create channel maps. Check the flagging criteria."
            return

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
                if colormap == 'gray': PL.gray()
                else: PL.jet()
                #if x == (NhPanel - 1): PL.colorbar()
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
            if os.access(FigFileDir+'listofplots.txt', os.F_OK):
                BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
            else:
                BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
                print >> BrowserFile, 'TITLE: Gridding'
                print >> BrowserFile, 'FIELDS: Result IF POL Iteration Page'
                print >> BrowserFile, 'COMMENT: Combined spectra by spatial Gridding'
            print >> BrowserFile, FigFileRoot+'_%s.png' % ValidCluster
            BrowserFile.close()
        ValidCluster += 1


    # Draw RMS Map
    for row in range(NROW):
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
           cb=PL.colorbar(shrink=0.8)
           cb.ax.set_title('[K]')
           lab = cb.ax.title

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

    del data

    return


def DrawTPImage(DataIn, Table, beamsize=0, gridsize=1, scale_max=False, scale_min=False, colormap='gray', ShowPlot=True, FigFileDir=False, FigFileRoot=False):
    '''
    data: numarray[NROW][NCHAN]
    Table: Table explaining data property
        [[IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]
         [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]
                 ......
         [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]]
    beamsize: size of the main beam radius in degree
    colormap: 'gray', 'color'
    '''

    if ShowPlot == False and FigFileDir == False: return

    # Read data
    NROW = len(Table)
##     NCHAN = 1
##     data = NP.zeros((NROW, NCHAN), dtype=NP.float32)
##     _tbtool = casac.homefinder.find_home_by_name('tableHome')
##     _tb = _tbtool.create()
##     _tb.open(DataIn)
##     for x in range(NROW):
##         data[x] = _tb.getcell('SPECTRA', x)
##     _tb.close()
##     del _tbtool, _tb
    (NCHAN,data,Abcissa,Velocity) = GetDataFromFile( DataIn, NROW, False )

    # Determine Plotting Area Size
    (Xmin, Xmax, Ymin, Ymax) = (Table[0][2], Table[0][2], Table[0][3], Table[0][3])
    (RAmin, RAmax, DECmin, DECmax) = (Table[0][4], Table[0][4], Table[0][5], Table[0][5])
    for row in range(1, NROW):
        if   Xmin > Table[row][2]: (Xmin, RAmin) = (Table[row][2], Table[row][4])
        elif Xmax < Table[row][2]: (Xmax, RAmax) = (Table[row][2], Table[row][4])
        if   Ymin > Table[row][3]: (Ymin, DECmin) = (Table[row][3], Table[row][5])
        elif Ymax < Table[row][3]: (Ymax, DECmax) = (Table[row][3], Table[row][5])

    # Set data
    Total = NP.zeros((Ymax - Ymin + 1, Xmax - Xmin + 1), dtype=NP.float32)
    # If the data is valid, set the value. Otherwise set Zero
    for row in range(NROW):
        if Table[row][6] > 0:
            Total[int(Ymax-(Table[row][3]-Ymin))][int(Xmax-(Table[row][2]-Xmin))] = data[row][0]
        else:
            Total[int(Ymax-1-(Table[row][3]-Ymin))][int(Xmax-1-(Table[row][2]-Xmin))] = 0.0

    # Swap (x,y) to match the clustering result
    ExtentCM = ((Xmax+0.5)*gridsize*3600., (Xmin-0.5)*gridsize*3600., (Ymin-0.5)*gridsize*3600., (Ymax+0.5)*gridsize*3600.)
    Extent = (RAmax+gridsize/2.0, RAmin-gridsize/2.0, DECmin-gridsize/2.0, DECmax+gridsize/2.0)
    span = max(RAmax - RAmin + gridsize, DECmax - DECmin + gridsize)
    (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

    # Plotting
    TickSize = 6
    if ShowPlot: PL.ion()
    PL.figure(MATPLOTLIB_FIGURE_ID[7])
    if ShowPlot: PL.ioff()
    # 2008/9/20 Dec Effect has been taken into account
    Aspect = 1.0 / math.cos(Table[0][5] / 180. * 3.141592653)

    PL.cla()
    PL.clf()

    (x0, x1, y0, y1) = (0.25, 0.5, 0.25, 0.5)
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

    PL.title('Total Power', size=TickSize)

    if ShowPlot != False: PL.draw()
    if FigFileDir != False:
        PL.savefig(FigFileDir+FigFileRoot+'_TP.png', format='png', dpi=DPIDetail)
        if os.access(FigFileDir+'listofplots.txt', os.F_OK):
            BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
        else:
            BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
            print >> BrowserFile, 'TITLE: Gridding'
            print >> BrowserFile, 'FIELDS: Result IF POL Iteration Page'
            print >> BrowserFile, 'COMMENT: Combined spectra by spatial Gridding'
        print >> BrowserFile, FigFileRoot+'_TP.png'
        BrowserFile.close()

    del data

    return


def DrawContImage(DataIn, Table, Abcissa, beamsize=0, gridsize=1, scale_max=False, scale_min=False, colormap='gray', ShowPlot=True, FigFileDir=False, FigFileRoot=False):
    '''
    data: numarray[NROW][NCHAN]
    Table: Table explaining data property
        [[IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]
         [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]
                 ......
         [IF, POL, X, Y, RA, DEC, # of Combined Sp., # of flagged Sp., RMS]]
    beamsize: size of the main beam radius in degree
    colormap: 'gray', 'color'
    '''

    if ShowPlot == False and FigFileDir == False: return

    # Read data
    NROW = len(Table)
##     NCHAN = len(Abcissa[1])
##     s = scantable(DataIn, average=False)
    
##     data = NP.zeros((NROW, NCHAN), dtype=NP.float32)
##     _tbtool = casac.homefinder.find_home_by_name('tableHome')
##     _tb = _tbtool.create()    
##     _tb.open(DataIn)
##     for x in range(NROW):
##         data[x] = _tb.getcell('SPECTRA', x)
##     _tb.close()
##     del _tbtool, _tb
    (NCHAN, data, Frequency, Velocity) = GetDataFromFile( DataIn, NROW, False )

    (NChannelMap, NhPanel, NvPanel) = (3, 3, 1)

    # Determine Plotting Area Size
    (Xmin, Xmax, Ymin, Ymax) = (Table[0][2], Table[0][2], Table[0][3], Table[0][3])
    (RAmin, RAmax, DECmin, DECmax) = (Table[0][4], Table[0][4], Table[0][5], Table[0][5])
    for row in range(1, NROW):
        if   Xmin > Table[row][2]: (Xmin, RAmin) = (Table[row][2], Table[row][4])
        elif Xmax < Table[row][2]: (Xmax, RAmax) = (Table[row][2], Table[row][4])
        if   Ymin > Table[row][3]: (Ymin, DECmin) = (Table[row][3], Table[row][5])
        elif Ymax < Table[row][3]: (Ymax, DECmax) = (Table[row][3], Table[row][5])

    # Set Data
    Map = NP.zeros((NChannelMap, (Ymax - Ymin + 1), (Xmax - Xmin + 1)), dtype=NP.float32)
    Total = NP.zeros(((Ymax - Ymin + 1), (Xmax - Xmin + 1)), dtype=NP.float32)
    # ValidSp: SQRT of Number of combined spectra for the weight
    ValidSp = NP.zeros(NROW, dtype=NP.float32)
    for row in range(NROW): ValidSp[row] = math.sqrt(Table[row][6])
    # Integrated Spectrum
    Sp = NP.sum(NP.transpose((ValidSp * NP.transpose(data))),axis=0)/NP.sum(ValidSp,axis=0)
##     (F0, F1) = (min(Abcissa[1][0], Abcissa[1][NCHAN - 1]), max(Abcissa[1][0], Abcissa[1][NCHAN - 1]))
    (F0, F1) = (min(Frequency[0], Frequency[NCHAN - 1]), max(Frequency[0], Frequency[NCHAN - 1]))
    
    # Swap (x,y) to match the clustering result
    ExtentCM = ((Xmax+0.5)*gridsize*3600., (Xmin-0.5)*gridsize*3600., (Ymin-0.5)*gridsize*3600., (Ymax+0.5)*gridsize*3600.)
    Extent = (RAmax+gridsize/2.0, RAmin-gridsize/2.0, DECmin-gridsize/2.0, DECmax+gridsize/2.0)
    span = max(RAmax - RAmin + gridsize, DECmax - DECmin + gridsize)
    (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

    # Plotting
    TickSize = 6
    if ShowPlot: PL.ion()
    PL.figure(MATPLOTLIB_FIGURE_ID[7])
    if ShowPlot: PL.ioff()
    PL.cla()
    PL.clf()
    # 2008/9/20 Dec Effect has been taken into account
    Aspect = 1.0 / math.cos(Table[0][5] / 180. * 3.141592653)

    Title = []
    N = 0
    ChanC = int(NCHAN/2)
    ChanW = int(NCHAN/(NChannelMap + 1))
    ChanB = int(ChanC - NChannelMap / 2.0 * ChanW)

    Format = PL.FormatStrFormatter('%.2f')

    # Plot Integrated Spectrum at the Top
    (x0, x1, y0, y1) = (0.5/3.0+0.1/3.0, 2.0/3.0*0.8, 1.0/2.0+0.2/2.0, 1.0/2.0*0.7)
    a = PL.axes([x0, y0, x1, y1])
    a.xaxis.set_major_formatter(Format)
##     PL.plot(Abcissa[1], Sp, '-b', markersize=2, markeredgecolor='b', markerfacecolor='b')
    PL.plot(Frequency, Sp, '-b', markersize=2, markeredgecolor='b', markerfacecolor='b')
    for i in range(NChannelMap + 1):
##         PL.axvline(x = Abcissa[1][ChanB+ChanW*i], linewidth=0.3, color='r')
        PL.axvline(x = Frequency[ChanB+ChanW*i], linewidth=0.3, color='r')
    PL.xticks(size=TickSize)
    PL.yticks(size=TickSize)
    PL.xlabel('Frequency (GHz)', size=TickSize)
    PL.ylabel('Intensity (K)', size=TickSize)
    #PL.setp(xlabels, 'rotation', 45, fontsize=TickSize)
    Range = PL.axis()
    PL.axis([F0, F1, Range[2], Range[3]])
    PL.title('Line-Free Continuum Integrated Spectrum', size=TickSize)

    # Draw Channel Map
    # Check the direction of the Velocity axis
##     if Abcissa[1][0] > Abcissa[1][1]: Reverse = True
    if Frequency[0] > Frequency[1]: Reverse = True
    else: Reverse = False
    # Scale for the color bar
    Vmax0 = Vmin0 = 0
    for i in range(NChannelMap):
        if Reverse: ii=NChannelMap-1-i
        else: ii=i
        C0 = ChanB + ChanW*ii
        C1 = C0 + ChanW
##         Title.append('Freq = (%.1f: %.1f) (GHz)' % (min(Abcissa[1][C0], Abcissa[1][C1]), max(Abcissa[1][C0], Abcissa[1][C1])))
        Title.append('Freq = (%.1f: %.1f) (GHz)' % (min(Frequency[C0], Frequency[C1]), max(Frequency[C0], Frequency[C1])))
        for row in range(NROW):
            Intensity = data[row][C0:C1].sum() / float(ChanW)
            # Check the number of accumulated spectra
            if Table[row][6] > 0:
                if N == 0: (Vmax0, Vmin0) = (Intensity, Intensity)
                elif Intensity > Vmax0: Vmax0 = Intensity
                elif Intensity < Vmin0: Vmin0 = Intensity
                N += 1
            # Swap (x,y) to match the clustering result
            Map[ii][Ymax - (Table[row][3] - Ymin)][Xmax - (Table[row][2] - Xmin)] = Intensity
    if type(scale_max) == bool: Vmax = Vmax0 - (Vmax0 - Vmin0) * 0.1
    else:                       Vmax = scale_max
    if type(scale_min) == bool: Vmin = Vmin0 + (Vmax0 - Vmin0) * 0.1
    else:                       Vmin = scale_min

    if Vmax == 0 and Vmin == 0: 
        print "No data to create channel maps. Check the flagging criteria."
        return

    for ii in range(NChannelMap):
        x = ii % NhPanel
        y = NvPanel - int(ii / NhPanel) - 1
        x0 = 1.0 / float(NhPanel) * (x + 0.05)
        x1 = 1.0 / float(NhPanel) * 0.9
        y0 = 1.0 / float((NvPanel+0.5)) * (y + 0.05)
        y1 = 1.0 / float((NvPanel+2)) * 0.85
        a = PL.axes([x0, y0, x1, y1])
        a.set_aspect('equal')
        a.xaxis.set_major_locator(PL.NullLocator())
        a.yaxis.set_major_locator(PL.NullLocator())
        #im = PL.imshow(Map[i], vmin=Vmin, vmax=Vmax, interpolation='bilinear', aspect='equal', extent=Extent)
        if not (Vmax==Vmin):
            im = PL.imshow(Map[ii], vmin=Vmin, vmax=Vmax, interpolation='nearest', aspect='equal', extent=ExtentCM)
            if colormap == 'gray': PL.gray()
            else: PL.jet()

            if x == (NhPanel - 1):
                cb=PL.colorbar()
                for t in cb.ax.get_yticklabels():
                    newfontsize = t.get_fontsize()*0.5
                    t.set_fontsize(newfontsize)
                #cb.ax.set_title('[K Channel]')
                cb.ax.set_title('[K]')
                lab=cb.ax.title
                lab.set_fontsize(newfontsize)

            PL.title(Title[ii], size=TickSize)
        
    if ShowPlot != False: PL.draw()
    if FigFileDir != False:
        PL.savefig(FigFileDir+FigFileRoot+'_cont.png', format='png', dpi=DPIDetail)
        if os.access(FigFileDir+'listofplots.txt', os.F_OK):
            BrowserFile = open(FigFileDir+'listofplots.txt', 'a')
        else:
            BrowserFile = open(FigFileDir+'listofplots.txt', 'w')
            print >> BrowserFile, 'TITLE: Gridding'
            print >> BrowserFile, 'FIELDS: Result IF POL Iteration Page'
            print >> BrowserFile, 'COMMENT: Combined spectra by spatial Gridding'
        print >> BrowserFile, FigFileRoot+'_cont.png'
        BrowserFile.close()

    return


def GetDataFromFile( DataIn, NROW, getVelocity=False ):
    s = scantable(DataIn, average=False)
    unitorg = s.get_unit()
    s.set_unit('GHz')
    Frequency = NP.array(s._getabcissa(0))
    NCHAN = len(Frequency)
    Velocity=None
    if getVelocity:
        RestFreq = 0.0
        RestFreqs = s.get_restfreqs().values()[0]
        if ( len(RestFreqs) != 0 ):
            RestFreq = RestFreqs[0]/1.0e9
        else:
            RestFreq = Frequency[0]
        Velocity = (1.0 - (Frequency / RestFreq)) * 299792.458
    s.set_unit(unitorg)
    data = NP.zeros((NROW, NCHAN), dtype=NP.float32)
    for x in range(NROW):
        data[x] = s._getspectrum(x)
    del s

    return (NCHAN,data,Frequency,Velocity)
