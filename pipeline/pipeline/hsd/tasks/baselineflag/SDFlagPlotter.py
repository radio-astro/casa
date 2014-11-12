'''
Created on 2013/07/02

@author: kana
'''
#import numpy as NP
import pylab as PL
#import math
import os
from matplotlib.ticker import FuncFormatter, MultipleLocator, AutoLocator
from matplotlib.font_manager import FontProperties 

from .worker import INVALID_STAT

## 0:DebugPlot 1:TPlotRADEC 2:TPlotAzEl 3:TPlotCluster 4:TplotFit 5:TPlotMultiSP 6:TPlotSparseSP 7:TPlotChannelMap 8:TPlotFlag 9:TPlotIntermediate
MATPLOTLIB_FIGURE_ID = [8904, 8905, 8906, 8907, 8908, 8909, 8910, 8911, 8912, 8913]
DPIDetail = 130
FIGSIZE_INCHES = (7.0, 2.8)
    
def StatisticsPlot(PlotData, FigFileDir=False, FigFileRoot=False):
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

    if FigFileDir == False: return
    PL.ioff()
    PL.figure(MATPLOTLIB_FIGURE_ID[8])
    
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
    figsize_org = PL.gcf().get_size_inches()
    PL.gcf().set_size_inches(FIGSIZE_INCHES)
    #PL.subplot(211)
    PL.subplot(111)
    #PL.subplots_adjust(hspace=0.3)
    PL.subplots_adjust(top=0.88, left=0.1, right=0.98)
    PL.title(PlotData['title'], size=7)
    PL.xlabel(PlotData['xlabel'], size=6)
    PL.ylabel(PlotData['ylabel'], size=7)
    PL.xticks(size=6)
    PL.yticks(size=6)
    if PlotData['isActive']:
        PL.figtext(0.01, 0.98, "ACTIVE", horizontalalignment='left', verticalalignment='top', color='green', size=18, style='italic', weight='bold')
    else:
        PL.figtext(0.01, 0.98, "INACTIVE", horizontalalignment='left', verticalalignment='top', color='red', size=18, style='italic', weight='bold')

    # X-scale
    xmin = min(PlotData['row'])
    xmax = max(PlotData['row'])
    # For NO DATA
    if PlotData['data'] is None:
        if PlotData['isActive']:
            raise Exception, "Got no valid data for active flag type." 
        PL.axis([xmin, xmax, 0.0, 1.0])
        PL.figtext(0.5, 0.5, "NO DATA", horizontalalignment='center', verticalalignment='center', color='Gray', size=24, style='normal', weight='bold')
        PL.ion()
        PL.draw()
        if FigFileDir != False:
            OldPlot = FigFileDir+FigFileRoot+'.png'
            NewPlot = FigFileDir+FigFileRoot+'_trim.png'
            PL.savefig(OldPlot, format='png', dpi=DPIDetail)
            #os.system('convert %s -trim %s' % (OldPlot, NewPlot))
        PL.gcf().set_size_inches(figsize_org)
        return
        

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
    # Make Plot Data
    x = 0
    data = [[],[],[],[],[],[]]

    for Pflag in PlotData['permanentflag']:
        if Pflag == 0: # Flag-out case
            data[4].append(PlotData['row'][x])
            if PlotData['data'][x] > ScaleOut[0][0] or PlotData['data'][x] == INVALID_STAT:
                data[5].append(ScaleOut[0][1])
            elif LowRange and PlotData['data'][x] < ScaleOut[1][0]:
                data[5].append(ScaleOut[1][1])
            else:
                data[5].append(PlotData['data'][x])
        elif PlotData['flag'][x] == 0: # Flag-out case
            data[2].append(PlotData['row'][x])
            if PlotData['data'][x] > ScaleOut[0][0] or PlotData['data'][x] == INVALID_STAT:
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

    PL.ion()
    PL.draw()
    if FigFileDir != False:
        OldPlot = FigFileDir+FigFileRoot+'.png'
        NewPlot = FigFileDir+FigFileRoot+'_trim.png'
        PL.savefig(OldPlot, format='png', dpi=DPIDetail)
        #os.system('convert %s -trim %s' % (OldPlot, NewPlot))
    PL.gcf().set_size_inches(figsize_org)

    del data, ScaleOut
    return

