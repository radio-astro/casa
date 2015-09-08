from __future__ import absolute_import

import os
import time
from math import sqrt
import numpy
import math
import pylab as PL

import pipeline.infrastructure as infrastructure
import pipeline.h.heuristics as heuristics
from .. import common
from . import rules


LOG = infrastructure.get_logger(__name__)

class DetectLineInputs(common.SingleDishInputs):
    def __init__(self, context, grid_table, spectra, masks, window=None, edge=None, broadline=None):
        self._init_properties(vars())
        
    @property
    def spw(self):
        return self.grid_table[0][0] if len(self.grid_table) > 0 else -1
    
    @property
    def window(self):
        return [] if self._window is None else self._window
    
    @window.setter
    def window(self, value):
        self._window = value
        
    @property
    def edge(self):
        return (0,0) if self._edge is None else self._edge
    
    @edge.setter
    def edge(self, value):
        self._edge = value
        
    @property
    def broadline(self):
        return False if self._broadline is None else self._broadline
    
    @broadline.setter
    def broadline(self, value):
        self._broadline = value
        
class DetectLineResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(DetectLineResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        super(DetectLineResults, self).merge_with_context(context)
        
    def _outcome_name(self):
        return ''

class DetectLine(common.SingleDishTaskTemplate):
    Inputs = DetectLineInputs
    #LineFinder = heuristics.AsapLineFinder
    LineFinder = heuristics.HeuristicsLineFinder
    ThresholdFactor = 3.0

    def __init__(self, inputs):
        super(DetectLine, self).__init__(inputs)
        self.line_finder = self.LineFinder()

    @common.datatable_setter
    def prepare(self):
        """
        The process finds emission lines and determines protection regions for baselinefit
        """
        spectra = self.inputs.spectra
        masks = self.inputs.masks
        grid_table = self.inputs.grid_table
        window = self.inputs.window
        edge = self.inputs.edge
        broadline = self.inputs.broadline
        
        detect_signal = {}

        #(nchan,nrow) = spectra.shape
        (nrow,nchan) = spectra.shape

        # Pre-Defined Spectrum Window
        if len(window) != 0:
            LOG.info('Skip line detection since line window is set.')
            tRA = self.datatable.getcol('RA')
            tDEC = self.datatable.getcol('DEC')
            predefined_window = self._get_predefined_window(window)
            for row in xrange(nrow):
                detect_signal[row] = [tRA[row], tDEC[row], predefined_window]
                for row in range(len(self.datatable)):
                    self.datatable.putcell('MASKLIST', row, predefined_window)
                    
            result = DetectLineResults(task=self.__class__,
                                       success=True,
                                       outcome=detect_signal)

            result.task = self.__class__

            if self.context.subtask_counter is 0: 
                result.stage_number = self.context.task_counter - 1
            else:
                result.stage_number = self.context.task_counter 

            return result

        LOG.info('Search regions for protection against the background subtraction...')
        LOG.info('DetectLine: Processing %d spectra...' % nrow)

        # Set edge mask region
        (EdgeL, EdgeR) = common.parseEdge(edge)
        Nedge = EdgeR + EdgeL
        LOG.info('edge=%s'%(list(edge)))
        LOG.info('EdgeL, EdgeR=%s, %s'%(EdgeL, EdgeR))
        LOG.info('Nedge=%s'%(Nedge))
        if Nedge >= nchan:
            message = 'Error: Edge masked region too large...'
            LOG.error(message)
            raise RuntimeError(message)

        #2015/04/23 MaxFWHM < nchan/3.0
        MaxFWHM = int(min(rules.LineFinderRule['MaxFWHM'], (nchan - Nedge)/3.0))
        #rules.LineFinderRule['MaxFWHM'] = MaxFWHM
        MinFWHM = int(rules.LineFinderRule['MinFWHM'])
        Threshold = rules.LineFinderRule['Threshold']        
        EdgeMin = int(nchan * rules.LineFinderRule['IgnoreEdge'])
        EdgeMax = int(nchan * (1.0 - rules.LineFinderRule['IgnoreEdge']) - 1)

        # 2011/05/17 TN
        # Switch to use either ASAP linefinder or John's linefinder
        ### Only 'Thre' is effective for Heuristics Linefinder. BoxSize, AvgLimit, and MinFWHM are ignored
        Thre = Threshold * self.ThresholdFactor

        # Create progress timer
        Timer = common.ProgressTimer(80, nrow, LOG.level)
        # 100.0: minimum number of channels for binned spectrum to detect lines
        MinChanBinSp = 50.0
        TmpRange = [4**i for i in xrange(int(math.ceil(math.log(len(spectra[0])/MinChanBinSp)/math.log(4))))]
        BinningRange = []
        for i in TmpRange:
           BinningRange.append([i, 0])
           if i>1: BinningRange.append([i, i/2])
        for row in xrange(nrow):
            # Countup progress timer
            Timer.count()

            ProcStartTime = time.time()
            Protected = []
            if len(grid_table[row][6]) == 0:
                LOG.debug('Row %d: No spectrum' % row)
                # No spectrum
                Protected = [[-1, -1, 1]]
            else:
                LOG.debug('Start Row %d' % (row))
                for [BINN, offset]  in BinningRange:
                    MinNchan = (MinFWHM-2) / BINN + 2
                    SP = self.SpBinning(spectra[row], BINN, offset)
                    MSK = self.MaskBinning(masks[row], BINN, offset)
    
                    protected = self._detect(spectrum = SP,
                                             mask = MSK,
                                             threshold=Thre+math.sqrt(BINN)-1.0,
                                             tweak=True,
                                             edge=(EdgeL,EdgeR))

                    MaxLineWidth = MaxFWHM
                    #MaxLineWidth = int((nchan - Nedge)/3.0)
                    MinLineWidth = rules.LineFinderRule['MinFWHM']
                    for i in range(len(protected)):
                        if protected[i][0] != -1:
                            Chan0 = protected[i][0]*BINN+offset
                            Chan1 = protected[i][1]*BINN-1+offset
                            ChanW = Chan1 - Chan0
                            if(EdgeMin < Chan0) and (Chan1 < EdgeMax) and (MinLineWidth <= ChanW) and (ChanW <= MaxLineWidth):
                                Protected.append([Chan0, Chan1, BINN])
                        else:
                            Protected.append([-1,-1, BINN])

                # plot to check detected lines
                #self.plot_detectrange(spectra[row], Protected, 'SpPlot0%04d.png' % row)

            detect_signal[row] = [grid_table[row][4], # RA
                                  grid_table[row][5], # DEC
                                  Protected]          # Protected Region
            ProcEndTime = time.time()
            LOG.info('Channel ranges of detected lines for Row %s: %s' % (row, detect_signal[row][2]))
            
            LOG.debug('End Row %d: Elapsed Time=%.1f sec' % (row, (ProcEndTime - ProcStartTime)) )
        del Timer

        #LOG.debug('DetectSignal = %s'%(detect_signal))
        result = DetectLineResults(task=self.__class__,
                                   success=True,
                                   outcome=detect_signal)
                
        result.task = self.__class__
                
        if self.context.subtask_counter is 0: 
            result.stage_number = self.context.task_counter - 1
        else:
            result.stage_number = self.context.task_counter 
                
        return result
    
    def plot_detectrange(self, sp, protected, fname):
        print protected, fname
        PL.clf()
        PL.plot(sp)
        ymin,ymax = PL.ylim()
        for i in range(len(protected)):
            y = ymin + (ymax - ymin)/30.0 * (i + 1)
            PL.plot(protected[i],(y,y),'r')
        PL.savefig(fname, format='png')

    def MaskBinning(self, data, Bin, offset=0):
        if Bin == 1: 
            return data
        else:
            return numpy.array([data[i:i+Bin].min() for i in xrange(offset, len(data)-Bin+1, Bin)], dtype=numpy.bool)

    def SpBinning(self, data, Bin, offset=0):
        if Bin == 1: 
            return data
        else:
            return numpy.array([data[i:i+Bin].mean() for i in xrange(offset, len(data)-Bin+1, Bin)], dtype=numpy.float)

    def analyse(self, result):
        return result


    def _detect(self, spectrum, mask, threshold, tweak, edge):
        nchan = len(spectrum)
        (EdgeL, EdgeR) = edge
        Nedge = EdgeR + EdgeL
        #2015/04/23 0.5 -> 1/3.0
        MaxFWHM = int(min(rules.LineFinderRule['MaxFWHM'], (nchan - Nedge)/3.0))
        MinFWHM = int(rules.LineFinderRule['MinFWHM'])

        LOG.trace('line detection parameters: ')
        LOG.trace('threshold (S/N per channel)=%.1f,' % threshold \
                       + 'channels, edges to be dropped=[%s, %s]' % (EdgeL, EdgeR) )
        line_ranges = self.line_finder(spectrum=spectrum,
                                       threshold=threshold, 
                                       tweak=True,
                                       mask=mask,
                                       edge=(int(EdgeL), int(EdgeR)))
        # line_ranges = [line0L, line0R, line1L, line1R, ...]
        nlines = len(line_ranges) / 2

        ### Debug TT
        #LOG.info('NLINES=%s, EdgeL=%s, EdgeR=%s' % (nlines, EdgeL, EdgeR))
        #LOG.debug('ranges=%s'%(line_ranges))

        protected = []
        for y in xrange(nlines):
            Width = line_ranges[y*2+1] - line_ranges[y*2] + 1
            ### 2011/05/16 allowance was moved to clustering analysis
            #allowance = int(Width/5)
            LOG.debug('Ranges=%s, Width=%s' % (line_ranges[y*2:y*2+2], Width))
            if Width >= MinFWHM and Width <= MaxFWHM and \
               line_ranges[y*2] > EdgeL and \
               line_ranges[y*2+1] < (nchan - 1 - EdgeR):
                protected.append([line_ranges[y*2], line_ranges[y*2+1]])
        if len(protected) == 0:
            protected = [[-1, -1]]
        elif(len(protected) > 1):
            # 2007/09/01 add merged lines to the list if two lines are close enough
            flag = True
            for y in range(len(protected) - 1):
                curr0, curr1 = protected[y][0], protected[y][1]
                next0, next1 = protected[y+1][0], protected[y+1][1]
                if (next0 - curr1) < (0.25*min((curr1-curr0),(next1-next0))):
                    if flag == True:
                        if curr1 < next1 and curr0 < next0 and (next1 - curr0) <= MaxFWHM:
                            protected.append([curr0, next1])
                            Line0 = curr0
                        else: continue
                    else:
                        if (next1 - Line0) <= MaxFWHM:
                            protected.pop()
                            protected.append([Line0, next1])
                        else:
                            flag = True
                            continue
                    flag = False
                else: flag = True
        return protected


    def _get_predefined_window(self, window):
        if len(window) == 0:
            return []
        else:
            if hasattr(window[0], '__iter__'):
                return map(self._get_linerange, window)
            else:
                return [self._get_linerange(window)]
            

    def _get_linerange(self, window):
        if len(window) == 2:
            # [chmin, chmax] form
            return window
        elif len(window) == 3:
            # [center_freq, velmin, velmax] form
            spwid = self.inputs.spw
            spw = self.inputs.context.observing_run[0].spectral_window[spwid]
            center_freq = window[0] * 1.0e9 # GHz -> Hz
            restfreq = spw.refval if len(spw.rest_frequencies) == 0 else spw.rest_frequencies[0]
            dfreq = map(lambda x: restfreq * abs(x) / 299792.458, window[1:])
            freq_range = [center_freq + dfreq[0], center_freq - dfreq[1]]
            window = map(lambda x: spw.refpix + (x - spw.refval) / spw.increment, freq_range)
            window.sort()
            return window
        else:
            raise RuntimeError('Invalid linewindow format')
    

