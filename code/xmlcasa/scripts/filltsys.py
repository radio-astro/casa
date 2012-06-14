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
from scipy.interpolate import interp1d
from taskinit import tbtool
_tb = tbtool.create()
import numpy

scaling={'GHz':1.0e-9,
         'MHz':1.0e-6,
         'kHz':1.0e-3,
         'Hz':1.0}

#
# fillTsys( filename, specif, tsysif, mode )
# 
# high level function to fill Tsys on spectral data
#
# Tsys is processed along the following three steps:
#   1. average within scan
#   2. if possible, linearly interpolate in time.
#   3. interpolate in frequency with specified mode
#
def fillTsys( filename, specif, tsysif=None, mode='linear',extrap=False ):
    """
    high level function to fill Tsys on spectral data
    
    Tsys is processed along the following three steps:
       1. average within scan
       2. if possible, linearly interpolate in time.
       3. interpolate in frequency with specified mode

    filename -- input filename
    specif -- IFNO for spectral data
    tsysif -- IFNO for calibration (Tsys) data
              default: None (try to find automatically)
              options: any integer
    mode -- interpolation mode along frequency axis
            default: 'linear'
            options: 'linear',,'nearest','zero',
                     'slinear','quadratic','cubic'
                     any integer specifying an order of
                     spline interpolation
    """
    filler = TsysFiller( filename=filename, specif=specif, tsysif=tsysif, extrap=extrap )
    polnos = filler.getPolarizations()
    for pol in polnos:
        filler.setPolarization( pol )
        filler.fillScanAveragedTsys( mode=mode )
    del filler

#
# Utility functions
#
def interpolateInFrequency( a, b, x, mode='linear' ):
    """
    Interpolate 1d array 'b', which is based on axis 'a',
    to 'x'.

    Inputs:
       a -- base axis (1-d array)
       b -- base array (1-d array)
       x -- transfer axis (1-d array)
       mode -- interpolation mode
               default: 'linear'
               options: 'linear','nearest','zero',
                        'slinear','quadratic','cubic'

    Returns:
       interpolated 1d array
    """
    flipped = False
    if a[1] < a[0]:
        flipped = True
        a = a[::-1]
        b = b[::-1]
        x = x[::-1]
    f = interp1d( a, b, kind=mode )
    y = f( x )
    if flipped:
        y = y[::-1]
    return y

def interpolateInTime( t1, y1, t2, y2, t ):
    """
    Linearly interpolate between y1 and y2, which are defined
    at t1 and t2 respectively, to t, where t1 <= t <= t2.

    Inputs:
       t1, t2 -- base time stamp
       y1, y2 -- the value at t1 and t2
                 (either scalar or conformed numpy array)
       t -- transfer time stamp

    Returns:
       interpolated value
    """
    if t == t1:
        return y1
    elif t == t2:
        return y2
    else:
        dt1 = t - t1
        dt2 = t2 - t
        y = ( y2 * dt1 + y1 * dt2 ) / ( dt1 + dt2 )
        return y

#
# class TsysFiller
#
# Working class to fill TSYS columns for spectral data and
# its channel averaged data.
#
# Basic Usage:
#    filler = TsysFiller( filename, specif, tsysif )
#    polnos = filler.getPolarizations()
#    for pol in polnos:
#        filler.setPolarization( pol )
#        filler.fillScanAveragedTsys( mode=mode )
# 
class TsysFiller:
    """
    Fill Tsys
    """
    def __init__( self, filename, specif, tsysif=None, extrap=False ):
        """
        Constructor

        filename -- input Scantable
        specif -- IFNO for spectral data
        tsysif -- IFNO for calibration scans (Tsys)
        """
        self.table = None
        self.stab = None
        self.ttab = None
        self.polno = None
        self.beamno = 0
        self.filename = filename.rstrip('/')
        self.table = tbtool.create()
        self.table.open(self.filename,nomodify=False)
        self.specif = specif
        self.stab = self._select( self.specif )
        self.abcsp = self._constructAbcissa( self.specif )
        self.extend = extrap
        if tsysif is None:
            self.tsysif = None
            self.abctsys = None
            self._setupTsysConfig()
        else:
            self.tsysif = tsysif
            self.ttab = self._select( self.tsysif )
            self.abctsys = self._constructAbcissa( self.tsysif )
            if not self.extend and not self.__checkCoverage( self.abctsys, self.abcsp ):
                raise Exception( "Invalid specification of SPW for Tsys: it must cover SPW for target" )
        if not self.extend:
            self.extend = self.__checkChannels( self.abctsys, self.abcsp )
        print 'spectral IFNO %s: corresponding Tsys IFNO is %s'%(self.specif,self.tsysif) 

    def __del__( self ):
        """
        Destructor
        """
        if self.stab is not None:
            self.stab.close()
        if self.ttab is not None:
            self.ttab.close()
        if self.table is not None:
            self.table.close()

    def _setupTsysConfig( self ):
        """
        Set up configuration for Tsys
           - determine IFNO for Tsys
           - set self.ttab
           - set self.abctsys
        """
        ftab=self.table.getkeyword('FREQUENCIES').split()[-1]
        ifnos = numpy.unique(self.table.getcol('IFNO'))
        nif = len(ifnos)
        for i in xrange(nif):
            tbsel=self.table.query('IFNO==%s'%(i))
            if tbsel.nrows() == 0:
                continue
            nchan=len(tbsel.getcell('SPECTRA',0))
            if nchan == 1:
                continue
            else:
                abc = self._constructAbcissa( i )
##                 if abc.min() <= self.abcsp.min() \
##                    and abc.max() >= self.abcsp.max():
                if self.__checkCoverage( abc, self.abcsp ):
                    self.tsysif = i
                    self.abctsys = abc
                    self.ttab = self._select( i )
                    break

    def __checkCoverage( self, a, b ):
        """
        Check frequency coverage
        """
        ea = self.__edge( a )
        eb = self.__edge( b )
        # 2012/05/09 TN
        # This is workaround for the issue that frequency coverage
        # doesn't match in LSRK frame although it exactly match in TOPO.
        ret = ( ea[0] < eb[0] and ea[1] > eb[1] ) \
              or ( abs((ea[0]-eb[0])/ea[0]) < 1.0e-5 \
                   and abs((ea[1]-eb[1])/ea[1]) < 1.0e-5 )
        return ret
##         return (ea[0] <= eb[0] and ea[1] >= eb[1])

    def __checkChannels( self, a, b ):
        """
        Check location of first and last channels
        """
        return ( a.min() > b.min() or a.max() < b.max() )

    def __edge( self, abc ):
        """
        return left and right edge values.
        """
        incr = abc[1] - abc[0]
        ledge = abc[0] - 0.5 * incr
        redge = abc[-1] + 0.5 * incr
        if ledge > redge:
            return (redge,ledge)
        else:
            return (ledge,redge)
    
    def _select( self, ifno, polno=None, beamno=0, scanno=None ):
        """
        Select data by IFNO, POLNO, BEAMNO, and SCANNO

        ifno -- IFNO
        polno -- POLNO
        beamno -- BEAMNO
        scanno -- SCANNO
        """
        keys=['IFNO']
        vals=[ifno]
        if polno is not None:
            keys.append('POLNO')
            vals.append(polno)
        if beamno is not None:
            keys.append('BEAMNO')
            vals.append(beamno)
        if scanno is not None:
            keys.append('SCANNO')
            vals.append(scanno)
        return self.__select( keys, vals )

    def __select( self, keys, vals, logic='&&' ):
        """
        Select data by selection criteria specified by (keys,vals) pair.
        Query string is constructed as,

           '<keys[0]>==<vals[0]><logic><keys[1]>==<vals[1]><logic>...'
           
        keys -- list of column names
        vals -- list of column values

        Example:
           keys=['IFNO','POLNO']
           vals=[0,0]
           logic='&&'
           __select( keys, vals, logic )

           it creates the following query string,

           'IFNO==0&&POLNO==0'
        """
        qstr = ''
        for i in xrange(len(keys)):
            if i == len(keys)-1:
                qstr += '%s==%s'%(keys[i],vals[i])
            else:
                qstr += '%s==%s%s'%(keys[i],vals[i],logic)
        #print 'qstr=',qstr
        tab = self.table.query( qstr, sortlist='TIME' )
        if tab.nrows() == 0:
            tab.close()
            tab = None
        return tab
        
    def _constructAbcissa( self, ifno ):
        """
        Construct abcissa array from REFPIX, REFVAL, INCREMENT
        """
        ftab=self.table.getkeyword('FREQUENCIES').split()[-1]
        tbsel = self.table.query('IFNO==%s'%(ifno))
        nchan = len(tbsel.getcell('SPECTRA',0))
        fid = tbsel.getcell('FREQ_ID',0)
        tbsel.close()
        del tbsel
        _tb.open(ftab)
        refpix = _tb.getcell('REFPIX',fid)
        refval = _tb.getcell('REFVAL',fid)
        increment = _tb.getcell('INCREMENT',fid)
        _tb.close()
        fstart = refval-refpix*increment
        fend = refval+(nchan-1-refpix+0.5)*increment
        return numpy.arange(fstart,fend,increment)

    def getPolarizations( self ):
        """
        Get list of POLNO's
        """
        return numpy.unique( self.stab.getcol('POLNO') )

    def setPolarization( self, polno ):
        """
        Set POLNO to be processed

        polno -- POLNO
        """
        self.polno = polno
        self.stab = self._select( ifno=self.specif,
                                  polno=self.polno,
                                  beamno=self.beamno )
        self.ttab = self._select( ifno=self.tsysif,
                                  polno=self.polno,
                                  beamno=self.beamno )

    def setBeam( self, beamno ):
        """
        Set BEAMNO to be processed
        Since default BEAMNO is 0, this method is usually
        not necessary.

        beamno -- BEAMNO
        """
        self.beamno = beamno
        self.stab = self._select( ifno=self.specif,
                                  polno=self.polno,
                                  beamno=self.beamno )
        self.ttab = self._select( ifno=self.tsysif,
                                  polno=self.polno,
                                  beamno=self.beamno )

    def getSpecAbcissa( self, unit='GHz' ):
        """
        Get abcissa for spectral data

        unit -- spectral unit
                default: 'GHz'
                options: 'channel', 'GHz', 'MHz', 'kHz', 'Hz'
        """
        abc = self.abcsp
        if unit == 'channel':
            return numpy.arange(0,len(abc),1)
        else:
            return abc * scaling[unit]

    def getTsysAbcissa( self, unit='GHz' ):
        """
        Get abcissa for Tsys data
        
        unit -- spectral unit
                default: 'GHz'
                options: 'channel', 'GHz', 'MHz', 'kHz', 'Hz'
        """
        abc = self.abctsys
        if unit == 'channel':
            return numpy.arange(0,len(abc),1)
        else:
            return abc * scaling[unit]

    def getSpectrum( self, irow ):
        """
        Get spectral data

        irow -- row index
        """
        return self.stab.getcell('SPECTRA',irow)

    def getTsys( self, irow ):
        """
        Get Tsys

        irow -- row index
        """
        return self.ttab.getcell('TSYS',irow)

    def getSpecTime( self, irow ):
        """
        Get time for spectral data

        irow -- row index
        """
        return self.stab.getcell('TIME',irow)

    def getTsysTime( self, irow ):
        """
        Get time for calibration data (Tsys)

        irow -- row index
        """
        return self.ttab.getcell('TIME',irow)

    def searchTsysRows( self, irow, startrow=0 ):
        """
        Search Tsys rows by using TIME value in spectral data

        irow -- row index for spectral data

        Returns:
           list of two row indexes for Tsys data, [row0,row1]
           row0 is the nearest row in previous calibration
           row1 is the nearest row in subsequent calibration

        Note:
           row0 can be used to start position of next search
           if both spectral and Tsys data are sorted by TIME
        """
        tsp = self.stab.getcell( 'TIME', irow )
        ret = [-1,-1]
        drow = 1000
        row0 = 0
        row1 = 0
        nrow = self.ttab.nrows()

        tmp = self.ttab.getcell( 'TIME', startrow )
        if ( tmp > tsp ):
            print 'reset search'
            cleat = 0
        elif ( tmp == tsp ):
            ret[0] = startrow
            ret[1] = startrow
            return ret
        else:
            cleat = startrow
        
        while ( row1 < nrow ):
            row0 = row1
            row1 += drow 
            if row1 < startrow:
                print 'skip %s-%s'%(row0,row1)
                continue
            idx = max( 0, cleat-row0 )
            #print 'self.ttab.getcol( \'TIME\', %s, %s )'%(row0,drow)
            tcol = self.ttab.getcol( 'TIME', row0, drow )
            n = len( tcol )
            row1 = row0 + n
            print 'row0 = %s, row1 = %s'%(row0,row1)
            idx = self._search( tcol, tsp, idx )
            if idx == n:
                continue
            else:
                if tcol[idx] == tsp:
                    ret[0] = idx+row0
                    ret[1] = ret[0]
                elif idx == 0 and row0 == 0:
                    ret[1] = 0
                else:
                    ret[0] = idx+row0-1
                    ret[1] = idx+row0
                print 'idx found: tcol[%s] = %s, tsp = %s'%(idx,tcol[idx],tsp)
                break
        if ret[0] == -1 and ret[1] == -1:
            ret[1] = int(nrow-1)
            
        return ret

    def searchTsys( self ):
        """
        """
        ret = []
        nrow = self.stab.nrows()
        cleat = 0
        for irow in xrange(nrow):
            rows = self.searchTsysRows( irow, cleat )
            cleat = max( rows[0], 0 )
            ret.append( rows )
        return ret 

    def fillTsys( self, mode='linear' ):
        """
        Fill Tsys

        mode -- interpolation mode for Tsys in frequency axis
        """
        nrow = self.stab.nrows()
        cleat = 0
        # assume IFNO for channel averaged data is
        # (IFNO for sp data)+1 
        tptab = self._select( self.specif+1, self.polno, self.beamno )
        
        for irow in xrange(nrow):
            rows = self.searchTsysRows( irow, cleat )
            cleat = max( rows[0], 0 )
            if rows[0] == -1:
                tsys = self.getTsys( rows[1] )
            elif rows[1] == -1:
                tsys = self.getTsys( rows[0] )
            elif rows[0] == rows[1]:
                tsys = self.getTsys( rows[0] )
            else:
                t0 = self.getTsysTime( rows[0] )
                t1 = self.getTsysTime( rows[1] )
                tsys0 = self.getTsys( rows[0] )
                tsys1 = self.getTsys( rows[1] )
                t = self.getSpecTime( irow )
                tsys = interpolateInTime( t0, tsys0, t1, tsys1, t )
            if len(tsys) == len(self.abctsys):
                newtsys = interpolateInFrequency( self.abctsys,
                                                  tsys,
                                                  self.abcsp,
                                                  mode )
            else:
                # Tsys seems to be channel-averaged
                newtsys = tsys
            self.stab.putcell( 'TSYS', irow, newtsys )
            tptab.putcell( 'TSYS', irow, newtsys.mean() )

        tptab.close()
        del tptab

    def getScanAveragedTsys( self, scanno ):
        """
        Get Tsys averaged within scan

        scanno -- SCANNO
        """
        tbsel = self._select( self.tsysif, self.polno, self.beamno, scanno )
        if tbsel is None:
            return None
        else:
            tsys = tbsel.getcol('TSYS')
            tbsel.close()
            return tsys.mean(axis=1)

    def getScanAveragedTsysTime( self, scanno ):
        """
        Get scan time (mid-point of the scan)

        scanno -- SCANNO
        """
        tbsel = self._select( self.tsysif, self.polno, self.beamno, scanno )
        if tbsel is None:
            return None
        else:
            t = tbsel.getcol('TIME')
            tbsel.close()
            return t.mean()

    def fillScanAveragedTsys( self, mode='linear' ):
        """
        Fill Tsys

        Tsys is averaged over scan first. Then, Tsys is
        interpolated in time and frequency. Finally,
        averaged and interpolated Tsys is used to fill
        TSYS field for spectral data.
        """        
        # assume IFNO for channel averaged data is
        # (IFNO for sp data)+1 
        tptab = self._select( self.specif+1, self.polno, self.beamno )

        # get scan numbers for calibration scan (Tsys)
        calscans = numpy.unique( self.ttab.getcol('SCANNO') )
        calscans.sort()
        nscan = len(calscans)
        print 'nscan = ', nscan

        # get scan averaged Tsys and time
        atsys = []
        caltime = []
        for scan in calscans:
            atsys.append( self.getScanAveragedTsys( scan ) )
            caltime.append( self.getScanAveragedTsysTime( scan ) )

        # extend tsys if necessary
        if self.extend:
            (abctsys,atsys)=self.__extend(self.abctsys,self.abcsp,atsys)
        else:
            abctsys = self.abctsys

        # process all rows
        nrow = self.stab.nrows()
        cleat = 0
        for irow in xrange(nrow):
            print 'process row %s'%(irow)
            t = self.getSpecTime( irow )
            if t < caltime[0]:
                print 'No Tsys available before this scan, use first Tsys'
                tsys = atsys[0]
            elif t > caltime[nscan-1]:
                print 'No Tsys available after this scan, use last Tsys'
                tsys = atsys[nscan-1]
            else:
                idx = self._search( caltime, t, cleat )
                cleat = max( 0, idx-1 )
                if caltime[idx] == t:
                    tsys = atsys[idx]
                else:
                    t0 = caltime[idx-1]
                    t1 = caltime[idx]
                    tsys0 = atsys[idx-1]
                    tsys1 = atsys[idx]
                    tsys = interpolateInTime( t0, tsys0, t1, tsys1, t )
            if len(tsys) == len(abctsys):
                newtsys = interpolateInFrequency( abctsys,
                                                  tsys,
                                                  self.abcsp,
                                                  mode )
            else:
                newtsys = tsys
            self.stab.putcell( 'TSYS', irow, newtsys )
            tptab.putcell( 'TSYS', irow, newtsys.mean() )

        tptab.close()
        del tptab

    def __extend( self, a, aref, b ):
        """
        Extend spectra
        """
        incr_ref = aref[1] - aref[0]
        incr = a[1] - a[0]
        ext0 = 0
        ext1 = 0
        l0 = aref[0] - 0.5 * incr_ref
        r0 = aref[-1] + 0.5 * incr_ref
        l = a[0]
        r = a[-1]
        #print 'l0=%s,l=%s'%(l0,l)
        #print 'r0=%s,r=%s'%(r0,r)
        if incr_ref > 0.0:
            while l > l0:
                ext0 += 1
                l -= incr
            while r < r0:
                ext1 += 1
                r += incr
        else:
            while l < l0:
                ext0 += 1
                l -= incr
            while r > r0:
                ext1 += 1
                r += incr
        if ext0 == 0 and ext1 == 0:
            return (a,b)
        #print 'ext0=%s,ext1=%s'%(ext0,ext1)
        abctsys = numpy.zeros(len(a)+ext0+ext1,dtype=a.dtype)
        for i in xrange(ext0):
            abctsys[i] = a[0] - incr * (ext0-i)
        for i in xrange(ext1):
            abctsys[i+len(a)+ext0] = a[-1] + incr * (1+i)
        abctsys[ext0:len(abctsys)-ext1] = a
        #print 'aref[0]=%s,abctsys[0]=%s'%(aref[0],abctsys[0])
        #print 'aref[-1]=%s,abctsys[-1]=%s'%(aref[-1],abctsys[-1])
        atsys = numpy.zeros( (len(b),len(abctsys)), dtype=type(b[0][0]) )
        for i in xrange(len(b)):
            atsys[i][0:ext0] = b[i][0]
            atsys[i][len(abctsys)-ext1:] = b[i][-1]
            atsys[i][ext0:len(abctsys)-ext1] = b[i]
        return (abctsys,atsys)
                
    def _search( self, tcol, t, startpos=0 ):
        """
        Simple search
        
        Return minimum index that satisfies tcol[index] > t.
        If such index couldn't be found, return -1.

        tcol -- array
        t -- target value
        startpos -- optional start position (default 0)
        """
        n = len(tcol)
        idx = min( n-1, max( 0, startpos ) )
        if tcol[idx] > t:
            idx = 0
        while ( idx < n and tcol[idx] < t ):
            #print '%s: tcol[%s] = %s, t = %s'%(idx,idx,tcol[idx],t)
            idx += 1
        if idx == n:
            idx = -1
            print 'Index not found, return -1'
        else:
            print 'found index %s: time[%s] = %s, target = %s'%(idx,idx,tcol[idx],t)

        return idx
