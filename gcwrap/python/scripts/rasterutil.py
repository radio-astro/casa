from taskinit import casalog, gentools, qa
import pylab as pl
import numpy
import os
import contextlib

import asap as sd

tb = gentools(['tb'])[0]

def asdatestring(mjd, digit, timeonly=False):
    datedict = qa.splitdate(qa.quantity(mjd, 'd'))
    if digit > 10 : digit = 10
    sstr_tmp = str(numpy.round(datedict['s'], digit)).split('.')
    sstr = sstr_tmp[0] + '.' + sstr_tmp[1][0:digit]
    if timeonly:
        return '%s:%s:%s'%(datedict['hour'],datedict['min'],sstr)
    else:
        return '%s/%s/%s/%s:%s:%s'%(datedict['year'],datedict['month'],datedict['monthday'],datedict['hour'],datedict['min'],sstr)

def astimerange(tmargin, mjd0, mjd1):
    digit = numpy.abs(int(numpy.floor(numpy.log10(tmargin))))
    if int(mjd0) == int(mjd1): # same date, different time
        timeonly=True
    else: # different date
        timeonly=False
    return '%s~%s'%(asdatestring(mjd0,digit), asdatestring(mjd1,digit,timeonly=timeonly))

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
        self.mjd_range_nomargin = None
        self.mjd_range_nomargin_raster = None
        self.spw = None
        self.pol = None
        self.margin = None
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
            mjd_range_nomargin = self.mjd_range_nomargin
            daterangestring = astimerange(self.margin, *self.mjd_range_nomargin)
            casalog.post(formatline(i, daterangestring))

        casalog.post(separator)
        header = formatline('RASTER', 'TIMERANGE')
        casalog.post(header)
        for i in xrange(self.nraster):
            self.select(rasterid=i)
            mjd_range_nomargin_raster = self.mjd_range_nomargin_raster
            daterangestring = astimerange(self.margin, *self.mjd_range_nomargin_raster)
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
            mean_interval = numpy.array(s.get_inttime()).mean()
            self.margin = 0.1 * mean_interval
            mjd_margin = self.margin / 86400.0

        if rowid is not None:
            times = alltimes[self.gaplist[rowid]:self.gaplist[rowid+1]]
        else:
            times = alltimes[self.gaplist_raster[rasterid]:self.gaplist_raster[rasterid+1]]

        tmp_mjd_range = (times.min() - mjd_margin, times.max() + mjd_margin,)
        tmp_mjd_range_nomargin = (times.min(), times.max(),)
        casalog.post('time range: %s ~ %s'%(tmp_mjd_range_nomargin), priority='DEBUG')

        if rowid is not None:
            self.mjd_range = tmp_mjd_range
            self.mjd_range_nomargin = tmp_mjd_range_nomargin
        else:
            self.mjd_range_raster = tmp_mjd_range
            self.mjd_range_nomargin_raster = tmp_mjd_range_nomargin
            
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
            taql = 'TIME > %s && TIME < %s && SRCTYPE == 0'%(self.mjd_range_raster)
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

def _get_sampling(alldir,row_gap_idx):
    """
    Returns sampling sampling interval of raster scan.
    Input
      pointing direction array in unit of radian ([[ra0,ra1,...],[dec0,dec1,...]])
      indices of raster row gap
    Output
        sampling interval along and orthogonal to scan direction (arcsec) with
        the position angle (deg) of scan direction.
    """
    alldir = numpy.array(alldir)
    dra = alldir[0,1:]-alldir[0,:-1]
    ddec = alldir[1,1:]-alldir[1,:-1]
    dpos2 = dra**2+ddec**2
    alongScan = numpy.sqrt(numpy.median(dpos2))
    dra = numpy.array([ numpy.sign(val)*max(abs(val),1.e-8) for val in dra ])
    dtan = ddec/dra
    pa_rad = numpy.arctan(numpy.median(dtan))
    # orthogonal unit vector
    uvec = [numpy.cos(pa_rad+numpy.pi*0.5), numpy.sin(pa_rad+numpy.pi*0.5)]
    # direction vector at row gap
    rowmed_ra = []
    rowmed_dec = []
    for gap_idx in range(len(row_gap_idx)-1):
        #typical direction for each raster row
        start_idx = row_gap_idx[gap_idx]
        end_idx = row_gap_idx[gap_idx+1]
        rowmed_ra.append(numpy.median(alldir[0,start_idx:end_idx]))
        rowmed_dec.append(numpy.median(alldir[1,start_idx:end_idx]))
    gap_vec = [[rowmed_ra[idx+1]-rowmed_ra[idx],
                rowmed_dec[idx+1]-rowmed_dec[idx]]
                for idx in range(len(rowmed_ra)-1) ]
#    gap_vec = [[alldir[0,idx]-alldir[0,idx-1],
#                alldir[1,idx]-alldir[1,idx-1]] for idx in row_gap_idx[1:-2] ]
    orth_dist = [abs(numpy.dot(vec, uvec)) for vec in gap_vec]
    betweenScan = numpy.median(orth_dist)
    return alongScan, betweenScan, pa_rad*180./numpy.pi
