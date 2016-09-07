#*******************************************************************************
# ALMA - Atacama Large Millimeter Array
# Copyright (c) ATC - Astronomy Technology Center - Royal Observatory Edinburgh, 2011
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
import os
import numpy as np

import pipeline.infrastructure.api as api
import pipeline.infrastructure.casatools as casatools

from asap.asaplinefind import linefinder
from asap import _asap, scantable, rcParams

class HeuristicsLineFinder(api.Heuristic):
    """
    """
    def calculate(self, spectrum, threshold=7.0, min_nchan=3, avg_limit=2, box_size=2, tweak=False, mask=[], edge=None):
        _spectrum = np.array(spectrum)
        if len(mask) == 0:
            mask = np.ones(len(_spectrum),dtype=np.int)
        else:
            mask = np.array(mask, np.int)
        if edge is not None:
            if len(edge) == 1:
                if edge[0] != 0:
                    mask[:edge[0]] = 0
                    mask[-edge[0]:] = 0
                    _edge=(edge[0], len(_spectrum)-edge[0])
                else:
                    _edge=(0, len(_spectrum))
            else:
                mask[:edge[0]] = 0
                if edge[1] != 0:
                    mask[-edge[1]:] = 0
                _edge=(edge[0], len(_spectrum)-edge[1])
        else: _edge=(0, len(_spectrum))
        indeces = np.arange(len(_spectrum))

        previous_line_indeces = np.array([], np.int)
        previous_mad = 1e6
        iteration = 0
        max_iteration = 10

        while iteration <= max_iteration:
            #print 'iteration', iteration
            iteration += 1
            iteration_mask = np.array(mask)
            iteration_mask[previous_line_indeces] = 0
            variances = np.abs(_spectrum -
             np.median(_spectrum[iteration_mask==1]))

            good_variances = variances[iteration_mask==1]
            good_variances.sort()
            good_variances = good_variances[:int(0.8*len(good_variances))]
            mad = np.median(good_variances)
            #print mad

            #line_indeces = indeces[np.logical_and(mask==1, variances > 7*mad)]
            line_indeces = indeces[np.logical_and(mask==1, variances > threshold*mad)]
            #print line_indeces, previous_line_indeces,\
            # line_indeces==previous_line_indeces

            if mad > previous_mad or list(line_indeces) == list(previous_line_indeces):
                break
            else:
                previous_mad = mad
                previous_line_indeces = np.array(line_indeces)

        ranges = []
        range_start = None
        range_end = None

        for i in previous_line_indeces:
            if range_start is None:
                range_start = i
                range_end = i
            elif i == range_end + 1:
                range_end = i
#            elif range_end - range_start + 1 > 2:
            elif range_end - range_start + 1 > 1:
                ranges += [range_start, range_end]
                range_start = i
                range_end = i
            else:
                range_start = i
                range_end = i

        if range_start is not None and (range_end - range_start + 1 > 2):
            ranges += [range_start, range_end]
        #print 'result', previous_line_indeces
        #print ranges
        if tweak: ranges = self.tweak_lines(_spectrum, ranges, _edge)

        #return len(ranges)/2
        return ranges

    def tweak_lines(self, spectrum, ranges, edge, n_ignore=1):
        """
        """
        med = np.median(spectrum)
        #print med
        mask = np.array(spectrum) >= med
        for i in range(0, len(ranges), 2):
            if spectrum[ranges[i]] > med:
                # Emission Feature
                Mask = True
            else:
                # Absorption Feature
                Mask = False
            ignore = 0
            for j in range(ranges[i], edge[0]-1, -1):
                if (spectrum[j]-spectrum[j+1] > 0) == Mask: ignore += 1
                if (mask[j] != Mask) or (ignore > n_ignore):
                   ranges[i] = j
                   break
            ignore = 0
            for j in range(ranges[i+1], edge[1]):
                if (spectrum[j]-spectrum[j-1] > 0) == Mask: ignore += 1
                if (mask[j] != Mask) or (ignore > n_ignore):
                   ranges[i+1] = j
                   break
        return ranges

class AsapLineFinder(api.Heuristic):
    """
    """
    def __init__( self, nchan=None ):
        """
        constructor
        """
        self.lf = alinefinder(nchan)
        self.lf.init()

    def calculate(self, spectrum, threshold=1.7320508075688772, min_nchan=3, avg_limit=8, box_size=0.2, noise_box='all', noise_stat='mean80', tweak=False, mask=[], edge=None):
        self.lf.set_spectrum(spectrum)
        self.lf.set_options(threshold, min_nchan, avg_limit, box_size, noise_box, noise_stat, tweak)
        _edge = [0,0] if edge is None else edge
        nlines = self.lf.find_lines(nRow=0, mask=mask, edge=_edge)
        ranges = self.lf.get_ranges()
        return ranges
        
class alinefinder( linefinder ):
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

        #self._tbtool=casac.homefinder.find_home_by_name('tableHome')
        #self._tb=self._tbtool.create()

        # create dummy scantable
        self.init()


    def __del__( self ):
        """
        destructor
        """
        #del self._tbtool
        #del self._tb
        
    def init( self ):
        """
        Create dummy scantable to work with linefinder.
        """
        if os.path.exists(self.name):
            os.system('\\rm -rf %s'%self.name)
        s=_asap.Scantable(False)
        s._save(self.name)
        del s
        #self._tb.open(self.name,nomodify=False)
        with casatools.TableReader(self.name, nomodify=False) as tb:
            #self._tb.addrows(1)
            tb.addrows(1)
            if self.nchan is not None:
                #self._tb.putcell('SPECTRA',0,np.zeros(self.nchan,float))
                #self._tb.putcell('FLAGTRA',0,np.zeros(self.nchan,int))
                tb.putcell('SPECTRA',0,np.zeros(self.nchan,float))
                tb.putcell('FLAGTRA',0,np.zeros(self.nchan,int))
        #self._tb.close()

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

