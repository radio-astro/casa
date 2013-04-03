from __future__ import absolute_import

import os
import math
import pylab as PL
import numpy

from matplotlib.ticker import FuncFormatter, MultipleLocator
from matplotlib.font_manager import FontProperties 

import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from . import common

dsyb = '$^\circ$'
hsyb = ':'
msyb = ':'
RArotation = 90
DECrotation = 0

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

class SDPointingDisplay(common.SDInspectionDisplay):
    MATPLOTLIB_FIGURE_ID = 8905

    def doplot(self, idx, stage_dir):
        st = self.context.observing_run[idx]
        parent_ms = st.ms
        spws = parent_ms.get_spectral_windows(science_windows_only=True)
        spwid = spws[0].id
        beam_size = casatools.quanta.convert(self.context.observing_run[idx].beam_size[spwid], 'deg')
        obs_pattern = st.pattern[spwid].values()[0]
        rows = self.datatable.get_row_index(idx, spwid)
        plotfile = os.path.join(stage_dir, 'pointing_%s.png'%(st.basename))
        self.draw_radec(rows, circle=[0.5*beam_size['value']], ObsPattern=obs_pattern, plotfile=plotfile)
        parameters = {}
        parameters['intent'] = 'TARGET'
        parameters['spw'] = spwid
        parameters['pol'] = 'XXYY'
        parameters['ant'] = st.antenna.name
        parameters['type'] = 'sd'
        parameters['file'] = st.basename
        plot = logger.Plot(plotfile,
          x_axis='RA', y_axis='Dec',
          field=parent_ms.fields[0].name,
          parameters=parameters)
        return plot
        
    def draw_radec(self, rows, connect=True, circle=[], ObsPattern=False, show_plot=True, plotfile=False):
        """
        Draw loci of the telescope pointing
        xaxis: extension header keyword for RA
        yaxis: extension header keyword for DEC
        connect: connect points if True
        """
        datatable = self.datatable

        if show_plot == False and plotfile == False: return

        ROW = datatable.getcol('ROW')
        tRA = datatable.getcol('RA')
        tDEC = datatable.getcol('DEC')
        tNCHAN = datatable.getcol('NCHAN')
        
        # Extract RA and DEC
        RA = []
        DEC = []
        for row in rows:
            if tNCHAN[row] > 1:
                RA.append(tRA[row])
                DEC.append(tDEC[row])
        span = max(max(RA) - min(RA), max(DEC) - min(DEC))
        xmax = min(RA) - span / 10.0
        xmin = max(RA) + span / 10.0
        ymax = max(DEC) + span / 10.0
        ymin = min(DEC) - span / 10.0
        (RAlocator, DEClocator, RAformatter, DECformatter) = RADEClabel(span)

        # 2008/9/20 DEC Effect
        Aspect = 1.0 / math.cos(tDEC[0] / 180.0 * 3.141592653)

        # Plotting routine
        if show_plot: PL.ion()
        PL.figure(self.MATPLOTLIB_FIGURE_ID)
        if show_plot: PL.ioff()
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
        PL.axis([xmin, xmax, ymin, ymax])
        if show_plot != False: PL.draw()
        if plotfile != False: PL.savefig(plotfile, format='png', dpi=common.DPISummary)

        del RA, DEC, x, y
        return
