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
import numpy as np

class hlinefinder:
    """
    """
    def __init__(self):
        """
        """
        self._threshold = 7.0

    def set_spectrum(self, spectrum):
        """
        Set data that you want to process.

        spectrum: 1-d array
        """
        self._spectrum = np.array(spectrum)

    def find_lines(self, mask=[], edge=None):
        """
        """
        if len(mask) == 0:
            mask = np.ones(len(self._spectrum),dtype=np.int)
        else:
            mask = np.array(mask, np.int)
        if edge is not None:
            if len(edge) == 1:
                if edge[0] != 0:
                    mask[:edge[0]] = 0
                    mask[-edge[0]:] = 0
                    self._edge=(edge[0], len(self._spectrum)-edge[0])
                else:
                    self._edge=(0, len(self._spectrum))
            else:
                mask[:edge[0]] = 0
                if edge[1] != 0:
                    mask[-edge[1]:] = 0
                self._edge=(edge[0], len(self._spectrum)-edge[1])
        else: self._edge=(0, len(self._spectrum))
        indeces = np.arange(len(self._spectrum))

        iterate = True
        previous_line_indeces = np.array([], np.int)
        previous_mad = 1e6
        iteration = 0

        while iterate:
            #print 'iteration', iteration
            iteration += 1
            iteration_mask = np.array(mask)
            iteration_mask[previous_line_indeces] = 0
            variances = np.abs(self._spectrum -
             np.median(self._spectrum[iteration_mask==1]))

            good_variances = variances[iteration_mask==1]
            good_variances.sort()
            good_variances = good_variances[:int(0.8*len(good_variances))]
            mad = np.median(good_variances)
            #print mad

            #line_indeces = indeces[np.logical_and(mask==1, variances > 7*mad)]
            line_indeces = indeces[np.logical_and(mask==1, variances > self._threshold*mad)]
            #print line_indeces, previous_line_indeces,\
            # line_indeces==previous_line_indeces

            if mad > previous_mad:
                iterate = False
            elif list(line_indeces) == list(previous_line_indeces):
                iterate = False
            elif iteration > 10:
                iterate = False
            else:
                previous_mad = mad
                previous_line_indeces = np.array(line_indeces)

        self._ranges = []
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
                self._ranges += [range_start, range_end]
                range_start = i
                range_end = i
            else:
                range_start = i
                range_end = i

        if range_start is not None and (range_end - range_start + 1 > 2):
            self._ranges += [range_start, range_end]
        #print 'result', previous_line_indeces
        #print self._ranges
        if self._tweak: self.tweak_lines()

        return len(self._ranges)/2

    def get_ranges(self, ignore=False):
        """
        """
        return self._ranges

    #def set_options(self, threshold=10.0, min_nchan=3, avg_limit=2, box_size=2):
    def set_options(self, threshold=7.0, min_nchan=3, avg_limit=2, box_size=2, tweak=False):
        """
        """
        self._threshold = threshold
        self._tweak = tweak
        return

    def tweak_lines(self, n_ignore=1):
        """
        """
        med = np.median(self._spectrum)
        #print med
        mask = np.array(self._spectrum) >= med
        for i in range(0, len(self._ranges), 2):
            if self._spectrum[self._ranges[i]] > med:
                # Emission Feature
                Mask = True
            else:
                # Absorption Feature
                Mask = False
            ignore = 0
            for j in range(self._ranges[i], self._edge[0]-1, -1):
                if (self._spectrum[j]-self._spectrum[j+1] > 0) == Mask: ignore += 1
                if (mask[j] != Mask) or (ignore > n_ignore):
                   self._ranges[i] = j
                   break
            ignore = 0
            for j in range(self._ranges[i+1], self._edge[1]):
                if (self._spectrum[j]-self._spectrum[j-1] > 0) == Mask: ignore += 1
                if (mask[j] != Mask) or (ignore > n_ignore):
                   self._ranges[i+1] = j
                   break

