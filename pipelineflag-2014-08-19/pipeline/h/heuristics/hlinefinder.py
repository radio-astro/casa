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
# hlinefinder.py
#

import os
import numpy
import casac
from asap.asaplinefind import linefinder
from asap import _asap, scantable, rcParams

#
# class hlinefinder
#
# DESCRIPTION:
#    The hlinefinder class is an inheritance of asap linefinder class.
#    It enable to set data as 1-d array instead of scantable so that
#    it would be generic to use, especially for the user who is not
#    familiar with asap software.
#
#    Besic usage is as follows:
#
#       import hlinefinder
#       lf=hlinefinder.hlinefinder()
#       lf.set_options(threshold=threshold,min_nchan=min_nchan,
#                      avg_limit=avg_limit,box_size=box_size)
#       lf.set_spectrum(spectrum)
#       nlines=lf.find_lines(mask=mask,edge=edge)
#       lines=lf.get_ranges(False)
#
#    set_spectrum(): Set the data that you want to process.
#
#    set_options():  Specify linefinder options. Those options
#                    have default values but should be optimized.
#
#    find_lines():   Examine the data and returns number of lines
#                    found. You can mask specific channels by using
#                    mask and edge parameters. The mask should be
#                    boolean array of equal length with spectrum
#                    (True is unmasked, False is masked), while edge
#                    should be integer tuple with length of 1 or 2.  
#                    The edge specifies how many channels that are
#                    masked from left and right edge.
#
#    get_ranges():   Return list of line ranges. The length of the list
#                    is 2*nlines and contents of the list is minimum and
#                    maximum channels for each lines, i.e.
#
#                       [min0,max0,min1,max1,...]
#
#    init():         initialization.
#
#    See help of each function for more detail (e.g. description of
#    linefinder options).
#
#
# CAUTION:
#    If you set spectrum that has different number of channels from
#    previous one, linefinder options are reset to default value.
#    In such case, you may have to call set_options() again.
#
class hlinefinder( linefinder ):
    """
    The hlinefinder class is an inheritance of asap linefinder class.
    It enable to set data as 1-d array instead of scantable so that
    it would be generic to use, especially for the user who is not
    familiar with asap software.
    
    Besic usage is as follows:

       import hlinefinder
       lf=hlinefinder.hlinefinder()
       lf.set_options(threshold=threshold,min_nchan=min_nchan,
                      avg_limit=avg_limit,box_size=box_size)
       lf.set_spectrum(spectrum)
       nlines=lf.find_lines(mask=mask,edge=edge)
       lines=lf.get_ranges(False)

    The find_lines() method returns number of lines found.
    The get_ranges() method returns a list of line ranges that consists
    of minimum and maximum channel for each line,

       [min0,max0,min1,max1,...]
    
    If you set spectrum that has different number of channels from
    previous one, linefinder options are reset to default value.
    In such case, you may have to call set_options() again.
    """
    
    def __init__( self, nchan=None ):
        """
        constructor
        """
        linefinder.__init__( self )
        self.nchan=nchan
        self.name='hlinefinder.tmp.asap'
        self.scantab=None

        self.lines_merged=[]

        self._tbtool=casac.homefinder.find_home_by_name('tableHome')
        self._tb=self._tbtool.create()

        # create dummy scantable
        self.init()


    def __del__( self ):
        """
        destructor
        """
        del self._tbtool
        del self._tb
        
    def init( self ):
        """
        Create dummy scantable to work with linefinder.
        """
        if os.path.exists(self.name):
            os.system('\\rm -rf %s'%self.name)
        s=_asap.Scantable(False)
        s._save(self.name)
        del s
        self._tb.open(self.name,nomodify=False)
        self._tb.addrows(1)
        if self.nchan is not None:
            self._tb.putcell('SPECTRA',0,numpy.zeros(self.nchan,float))
            self._tb.putcell('FLAGTRA',0,numpy.zeros(self.nchan,int))
        self._tb.close()

        # make sure dummy scantable is loaded on memory
        storageorg=rcParams['scantable.storage']
        rcParams['scantable.storage']='memory'
        self.scantab=scantable(self.name,False)
        os.system('\\rm -rf %s'%self.name)
        rcParams['scantable.storage']=storageorg

    def set_spectrum( self, spectrum ):
        """
        Set data that you want to process.

        spectrum: 1-d array 
        """
        if self.nchan is None:
            #print 'call init() again'
            self.nchan=len(spectrum)
            self.init()
            self.scantab._setspectrum(spectrum)
        elif self.nchan != len(spectrum):
            #print 'nchan differ, call init() again'
            self.nchan=len(spectrum)
            self.init()
            self.scantab._setspectrum(spectrum)
            del self.finder
            self.finder=_asap.linefinder()
            self.set_options()
        else:
            #print 'set spectrum'
            self.scantab._setspectrum(spectrum)

        linefinder.set_scan(self,self.scantab)

    def merge_lines( self, lines, frac=0.25 ):
        """
        Merge lines if those are close enough.

        lines: line list detected by linefinder algorithm
        frac: criterion for merge as a fraction of line width
              for narrower line
        """
        nlines = len(lines)/2

        #print 'nlines = ', nlines
        if nlines == 0 or nlines == 1:
            return lines

        self.lines_merged = []
        merge = []
        idx = 0
        for i in xrange(nlines-1):
            width = min( (lines[2*i+1]-lines[2*i]), (lines[2*i+3]-lines[2*i+2]) )
            sep = lines[2*i+2] - lines[2*i+1]
            if sep < width * frac:
                merge.append( True )
            else:
                merge.append( False )
        #print 'merge = ', merge
        if merge.count( True ) == 0:
            self.lines_merged = lines
            return nlines
        else:
            self.lines_merged.append( lines[0] )
            for i in xrange(len(merge)):
                if merge[i]:
                    continue
                else:
                    self.lines_merged.append( lines[2*i+1] )
                    self.lines_merged.append( lines[2*i+2] )
            self.lines_merged.append( lines[2*nlines-1] )

        return len(self.lines_merged)
        
        
    def get_merged_ranges( self ):
        """
        Get a list of merged line ranges.
        """
        return self.lines_merged

    def set_options(self, threshold=1.7320508075688772, min_nchan=3, avg_limit=8, box_size=0.2, noise_box='all', noise_stat='mean80', tweak=False):
        linefinder.set_options(self, threshold=threshold, min_nchan=min_nchan, avg_limit=avg_limit, box_size=box_size, noise_box=noise_box, noise_stat=noise_stat)

    set_options.__doc__ = linefinder.set_options.__doc__
