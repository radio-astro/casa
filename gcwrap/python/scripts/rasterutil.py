from taskinit import casalog, gentools, qa
import pylab as pl
import numpy
import os
import contextlib

import asap as sd

tb = gentools(['tb'])[0]

def asdatestring(mjd, timeonly=False):
    datedict = qa.splitdate(qa.quantity(mjd, 'd'))
    if timeonly:
        return '%s:%s:%s'%(datedict['hour'],datedict['min'],datedict['sec'])
    else:
        return '%s/%s/%s/%s:%s:%s'%(datedict['year'],datedict['month'],datedict['monthday'],datedict['hour'],datedict['min'],datedict['sec'])

def astimerange(mjd0, mjd1):
    if int(mjd0) == int(mjd1):
        # same date, different time
        return '%s~%s'%(asdatestring(mjd0), asdatestring(mjd1,timeonly=True))
    else:
        # different date
        return '%s~%s'%(tuple(map(asdatestring,[mjd0,mjd1])))

@contextlib.contextmanager
def selection_manager(scantab, original_selection, **kwargs):
    sel = sd.selector(original_selection)
    for (k,v) in kwargs.items():
        method_name = 'set_%s'%(k)
        if hasattr(sel, method_name):
            getattr(sel, method_name)(v)
    scantab.set_selection(sel)
    yield scantab
    scantab.set_selection(original_selection)
    
class Raster(object):
    def __init__(self, scantab):
        self.scantab = scantab
        self.original_selection = self.scantab.get_selection()
        self.rows = None
        self.rasters = None
        self.mjd_range = None
        self.mjd_range_raster = None
        self.spw = None
        self.pol = None
        self.gaplist = []
        self.gaplist_raster = []
        self.ngap = 0
        self.ngap_raster = 0

    @property
    def nrow(self):
        return self.ngap - 1

    @property
    def nraster(self):
        return self.ngap_raster - 1

    @property
    def nominal_spw(self):
        if hasattr(self, '_nominal_spw') and getattr(self, '_nominal_spw') is not None:
            return self._nominal_spw
        
        self._nominal_spw = None
        with selection_manager(self.scantab, self.original_selection, types=0) as s:
            spw_list = s.getifnos()
        print spw_list
        for spw in spw_list:
            with selection_manager(self.scantab, self.original_selection, ifs=spw) as s:
                if self.scantab.nrow() > 0:
                    nchan = len(self.scantab._getspectrum(0))
                    if nchan > 4: # ignore channel-averaged and WVR
                        self._nominal_spw = spw
                        break
        return self._nominal_spw

    @property
    def nominal_pol(self):
        if hasattr(self, '_nominal_pol') and getattr(self, '_nominal_pol') is not None:
            return self._nominal_pol

        self._nominal_pol = None
        nominal_spw = self.nominal_spw
        with selection_manager(self.scantab, self.original_selection, types=0, ifs=nominal_spw) as s:
            if self.scantab.nrow() > 0:
                self._nominal_pol = self.scantab.getpolnos()[0]
        return self._nominal_pol

    def reset_selection(self):
        self.scantab.set_selection(self.original_selection)
    
    def detect(self, spw=None, pol=None):
        if spw is None:
            self.spw = self.nominal_spw
        else:
            self.spw = spw
        if pol is None:
            self.pol = self.nominal_pol
        else:
            self.pol = pol
        casalog.post('spw, pol = %s, %s'%(self.spw, self.pol), priority='DEBUG')
        self.gaplist, self.gaplist_raster = detect_gap(self.scantab, self.spw, self.pol)
        self.ngap = len(self.gaplist)
        self.ngap_raster = len(self.gaplist_raster)

        self.summarize()

    def summarize(self):
        if self.spw is None:
            self.detect()
        casalog.post('gaplist=%s (length %s)'%(self.gaplist, self.ngap), priority='DEBUG')
        separator = '-' * 35
        splitter = ' ' * 1
        indent = splitter * 1
        width = int(numpy.ceil(numpy.log10(self.nrow))) + 4
        column0 = '%{digit}s'.format(digit=width)
        formatline = lambda x,y: indent + column0%(x) + splitter + y
        casalog.post(separator)
        casalog.post('Raster Row/Raster Detection Summary')
        casalog.post(separator)
        headertitles = ['Filename', 'Nominal Spw for Detection', 'Nominal Pol for Detection', 'Number of Raster Rows', 'Number of Rasters']
        headervalues = ['', self.spw, self.pol, self.nrow, self.nraster]
        headertemplate = '%-{digit}s: %s'.format(digit=max(map(len,headertitles)))
        for (t,v) in zip(headertitles, headervalues):
            ht = t
            casalog.post(headertemplate%(ht,v))
        casalog.post(separator)
        header = formatline('ROW', 'TIMERANGE')
        casalog.post(header)
        for i in xrange(self.nrow):
            self.select(rowid=i)
            mjd_range = self.mjd_range
            daterangestring = astimerange(*self.mjd_range)
            casalog.post(formatline(i, daterangestring))

        casalog.post(separator)
        header = formatline('RASTER', 'TIMERANGE')
        casalog.post(header)
        for i in xrange(self.nraster):
            self.select(rasterid=i)
            mjd_range_raster = self.mjd_range_raster
            daterangestring = astimerange(*self.mjd_range_raster)
            casalog.post(formatline(i, daterangestring))



    def select(self, rowid=None, rasterid=None):
        if not ((rowid is None) ^ (rasterid is None)):
            raise RuntimeError('one of rowid or rasterid must be specified')

        if (rowid is not None) and (rowid >= self.nrow):
            raise IndexError('row index %s is out of range (number of rows detected: %s)'%(rowid,self.nrow))
        if (rasterid is not None) and (rasterid >= self.nraster):
            raise IndexError('row index %s is out of range (number of rasters detected: %s)'%(rasterid,self.nraster))

        with selection_manager(self.scantab, self.original_selection, types=0, ifs=self.spw, pols=self.pol) as s:
            alltimes = numpy.array(map(lambda x: qa.quantity(x)['value'], s.get_time(prec=16)))
            allintervals = numpy.array(s.get_inttime())

        if rowid is not None:
            times = alltimes[self.gaplist[rowid]:self.gaplist[rowid+1]]
            mean_interval = allintervals.mean() / 86400.0

            self.mjd_range = (times.min() - 0.1 * mean_interval, times.max() + 0.1 * mean_interval,)
            casalog.post('time range: %s ~ %s'%(self.mjd_range), priority='DEBUG')
            
        else:
            times = alltimes[self.gaplist_raster[rasterid]:self.gaplist_raster[rasterid+1]]
            mean_interval = allintervals.mean() / 86400.0

            self.mjd_range_raster = (times.min() - 0.1 * mean_interval, times.max() + 0.1 * mean_interval,)
            casalog.post('time range: %s ~ %s'%(self.mjd_range_raster), priority='DEBUG')
            
    def asscantable(self, rowid=None, rasterid=None):
        s = sd.scantable(self.infile, average=False)
        sel = self.asselector(rowid=rowid, rasterid=rasterid)
        s.set_selection(sel)
        return s

    def astaql(self, rowid=None, rasterid=None):
        if not ((rowid is None) ^ (rasterid is None)):
            raise RuntimeError('one of rowid or rasterid must be specified')

        if rowid is not None:
            rows = self.select(rowid=rowid)
            taql = 'TIME > %s && TIME < %s'%(self.mjd_range)
        else:
            rasters = self.select(rasterid=rasterid)
            taql = 'TIME > %s && TIME < %s'%(self.mjd_range_raster)
        return taql

    def asselector(self, rowid=None, rasterid=None, input_selector=None):
        taql = self.astaql(rowid=rowid, rasterid=rasterid)
        
        if input_selector is None:
            sel = sd.selector(query=taql)
        else:
            sel = sd.selector(input_selector)
            input_query = sel.get_query()
            if len(input_query) == 0:
                sel.set_query(taql)
            else:
                new_query = '(%s) && (%s)'%(input_query,taql)
                sel.set_query(new_query)
        return sel

    def plot_row(self, idx, rastermode='row'):
        """
        Plot detected raster or scan row points.
        Parameters
            idx        : index of raster row or visit to plot
            rastermode : detect either a raster vist ('raster') or
                         raster scan row ('row')
        """
        if rastermode.upper() == 'RASTER': (rowid, rasterid) = (None, idx) 
        elif rastermode.upper() == 'ROW': (rowid, rasterid) = (idx, None)
        else: raise ValueError("Invalid rastermode (should be 'row' or 'raster'")
        taql = self.astaql(rowid=rowid,rasterid=rasterid)

        with selection_manager(self.scantab, self.original_selection, types=0, ifs=self.spw, pols=self.pol) as s:
            alldir = numpy.array([s.get_directionval(i) for i in xrange(s.nrow())]).transpose()

        with selection_manager(self.scantab, self.original_selection, query=taql) as s:
            dirs = numpy.array([s.get_directionval(i) for i in xrange(s.nrow())]).transpose()
   
        pl.clf()
        pl.plot(alldir[0], alldir[1], 'o', color='#aaaaaa', markeredgewidth=0)
        pl.plot(dirs[0], dirs[1], 'o', color='g')
        pl.xlabel('RA [rad]')
        pl.ylabel('Declination [rad]')
        pl.gca().set_aspect('equal')

def detect_gap(scantab, spw, pol):
    with selection_manager(scantab, scantab.get_selection(), types=0, ifs=spw, pols=pol) as s:
        alldir = numpy.array([s.get_directionval(i) for i in xrange(s.nrow())]).transpose()
        timestamp = numpy.array(map(lambda x: qa.quantity(x)['value'], s.get_time(prec=16)))

    row_gap = _detect_gap(timestamp)
    ras_gap = _detect_gap_raster(timestamp, alldir, row_gap=row_gap)
    return row_gap, ras_gap

def _detect_gap(timestamp, threshold=None):
    nrow = len(timestamp)

    if nrow == 0:
        gaplist = []
    elif nrow == 1:
        gaplist = [0,1]
    else:
        dt=(timestamp[1:]-timestamp[:-1])*86400.0
        if threshold is None:
            # detect gap with threshold of 5 * median value
            dtm = 5.0 * numpy.median(dt[dt.nonzero()])
        else:
            dtm = threshold
        gaplist=[0]
        gap=(dt>dtm)
        gaplist.extend(numpy.where(dt > dtm)[0] + 1)
        if gaplist[-1] != nrow:
            gaplist.append(nrow)

    return gaplist

"""
def _detect_gap_raster_old(timestamp, alldir, row_gap=None, threshold_row=None, threshold_raster=None):
    nrow = len(timestamp)
    if row_gap is None:
        row_gaplist = _detect_gap(timestamp, threshold_row)
    else:
        row_gaplist = row_gap
    nrow_gaplist = len(row_gaplist)

    if nrow_gaplist == 0:
        raster_gaplist = []
    else:
        dd0 = alldir[:,1:]-alldir[:,:-1]
        dd = numpy.sqrt(dd0[0]*dd0[0]+dd0[1]*dd0[1])
        if threshold_raster is None:
            ddm = 5.0 * numpy.median(dd[dd.nonzero()])
        else:
            ddm = threshold_raster
        raster_gaplist=[0]
        for i in row_gaplist[1:-1]:
            if (i > 0) and (dd[i-1] > ddm):
                raster_gaplist.append(i)
        if raster_gaplist[-1] != nrow:
            raster_gaplist.append(nrow)

    return raster_gaplist
"""

def _detect_gap_raster(timestamp, alldir, row_gap=None, threshold_row=None, threshold_raster=None):
    nrow = len(timestamp)
    if row_gap is None:
        row_gaplist = _detect_gap(timestamp, threshold_row)
    else:
        row_gaplist = row_gap
    nrow_gaplist = len(row_gaplist)

    if nrow_gaplist == 0:
        raster_gaplist = []
    else:
        #pointing gaps in the first raster row, calc its median, and rotate 90 degrees
        rot = numpy.matrix(((0.0,1.0),(-1.0,0.0)))
        inext = row_gaplist[1]
        dd_row0 = numpy.median((alldir[:,1:inext]-alldir[:,:inext-1])[0])
        dd_row1 = numpy.median((alldir[:,1:inext]-alldir[:,:inext-1])[1])
        dd_row_rot = numpy.array(numpy.dot(numpy.array([dd_row0, dd_row1]), rot))

        #inner product of poiting gap and rotated median pointing gap of the first raster row
        dd0 = alldir[:,1:]-alldir[:,:-1]
        dd = numpy.dot(dd_row_rot, dd0)
        
        ppgap = []
        for i in row_gaplist:
            if (i > 0) and (i < row_gaplist[-1]):
                ppgap.append(dd[0][i-1])
        med_ppgap = numpy.median(numpy.array(ppgap))
        if med_ppgap < 0.0:
            dd = -dd
        factor = -1.0
        if threshold_raster is None:
            ddm = factor * abs(med_ppgap)
        else:
            ddm = factor * abs(threshold_raster)

        raster_gaplist=[0]
        for i in row_gaplist[1:-1]:
            if (i > 0) and (dd[0][i-1]<ddm):
                raster_gaplist.append(i)
        if raster_gaplist[-1] != nrow:
            raster_gaplist.append(nrow)

    return raster_gaplist
