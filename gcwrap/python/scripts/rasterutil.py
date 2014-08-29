from taskinit import casalog, gentools, qa
import pylab as pl
import numpy
import os

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

class Raster(object):
    def __init__(self, infile):
        self.infile = infile
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
        tb.open(self.infile)
        tsel = tb.query('SRCTYPE==0') # on-source
        spw_list = numpy.unique(tsel.getcol('IFNO'))
        for spw in spw_list:
            tsel = tb.query('IFNO==%s'%(spw))
            if tsel.nrows() > 0:
                nchan = len(tsel.getcell('SPECTRA',0))
                if nchan > 4: # ignore channel-averaged and WVR
                    self._nominal_spw = spw
                    tsel.close()
                    break
            tsel.close()
        tb.close()
        return self._nominal_spw

    @property
    def nominal_pol(self):
        if hasattr(self, '_nominal_pol') and getattr(self, '_nominal_pol') is not None:
            return self._nominal_pol

        self._nominal_pol = None
        nominal_spw = self.nominal_spw
        tb.open(self.infile)
        tsel = tb.query('SRCTYPE==0 && IFNO==%s'%(nominal_spw))
        if tsel.nrows() > 0:
            self._nominal_pol = tsel.getcell('POLNO', 0)
        tsel.close()
        tb.close()
        return self._nominal_pol

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
        self.gaplist, self.gaplist_raster = detect_gap(self.infile, self.spw, self.pol)
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
        headervalues = [self.infile, self.spw, self.pol, self.nrow, self.nraster]
        headertemplate = '%-{digit}s: %s'.format(digit=max(map(len,headertitles)))
        for (t,v) in zip(headertitles, headervalues):
            ht = t
            casalog.post(headertemplate%(ht,v))
        casalog.post(separator)
        header = formatline('ROW', 'TIMERANGE')
        casalog.post(header)
        for i in xrange(self.nrow):
            rows = self.select(rowid=i)
            mjd_range = self.mjd_range
            daterangestring = astimerange(*self.mjd_range)
            casalog.post(formatline(i, daterangestring))

        casalog.post(separator)
        header = formatline('RASTER', 'TIMERANGE')
        casalog.post(header)
        for i in xrange(self.nraster):
            rows = self.select(rasterid=i)
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

        tb.open(self.infile)
        tsel = tb.query('SRCTYPE==0 && IFNO==%s && POLNO==%s'%(self.spw,self.pol))
        allrows = tsel.rownumbers()
        alltimes = tsel.getcol('TIME')
        allintervals = tsel.getcol('INTERVAL')
        tsel.close()
        tb.close()

        if rowid is not None:
            rows = allrows[self.gaplist[rowid]:self.gaplist[rowid+1]]
            casalog.post('rownumber list for rowid %s: %s'%(rowid, rows), priority='DEBUG')
            times = alltimes[self.gaplist[rowid]:self.gaplist[rowid+1]]
            mean_interval = allintervals.mean() / 86400.0

            self.mjd_range = (times.min() - 0.1 * mean_interval, times.max() + 0.1 * mean_interval,)
            casalog.post('time range: %s ~ %s'%(self.mjd_range), priority='DEBUG')
            
            if len(rows) > 0:
                self.rows = map(int, rows)
            return rows

        else:
            rasters = allrows[self.gaplist_raster[rasterid]:self.gaplist_raster[rasterid+1]]
            casalog.post('rownumber list for rasterid %s: %s'%(rasterid, rasters), priority='DEBUG')
            times = alltimes[self.gaplist_raster[rasterid]:self.gaplist_raster[rasterid+1]]
            mean_interval = allintervals.mean() / 86400.0

            self.mjd_range_raster = (times.min() - 0.1 * mean_interval, times.max() + 0.1 * mean_interval,)
            casalog.post('time range: %s ~ %s'%(self.mjd_range_raster), priority='DEBUG')
            
            if len(rasters) > 0:
                self.rasters = map(int, rasters)
            return rasters

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

    def plot_row(self, rowid):
        taql = self.astaql(rowid=rowid)

        tb.open(self.infile)
        tsel = tb.query('SRCTYPE==0 && IFNO==%s && POLNO==%s'%(self.spw,self.pol))
        alldir = tsel.getcol('DIRECTION')
        tsel.close()
        
        tsel = tb.query(taql)
        tsel.nrows()
        dirs = tsel.getcol('DIRECTION')
        tsel.close()
        tb.close()
   
        pl.clf()
        pl.plot(alldir[0], alldir[1], 'o', color='#aaaaaa', markeredgewidth=0)
        pl.plot(dirs[0], dirs[1], 'o', color='g')        
                
def detect_gap(infile, spw, pol):
    tb.open(infile)
    tsel = tb.query('SRCTYPE==0 && IFNO==%s && POLNO==%s'%(spw,pol))
    alldir = tsel.getcol('DIRECTION')
    timestamp = tsel.getcol('TIME')
    interval = tsel.getcol('INTERVAL')
    rows = tsel.rownumbers()
    tsel.close()
    tb.close()

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
        factor = -1.0 if med_ppgap > 0.0 else 1.0
        if threshold_raster is None:
            ddm = factor * med_ppgap
        else:
            ddm = factor * threshold_raster

        raster_gaplist=[0]
        for i in row_gaplist[1:-1]:
            if (i > 0) and (((med_ppgap>0.0)and(dd[0][i-1]<ddm))or((med_ppgap<0.0)and(dd[0][i-1]>ddm))):
                raster_gaplist.append(i)
        if raster_gaplist[-1] != nrow:
            raster_gaplist.append(nrow)

    return raster_gaplist
