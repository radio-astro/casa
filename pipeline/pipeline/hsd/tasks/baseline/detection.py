from __future__ import absolute_import

import os
import time
import numpy
import math
import pylab as PL

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.vdp as vdp
import pipeline.h.heuristics as heuristics
import pipeline.domain.measures as measures
from pipeline.domain.datatable import DataTableImpl as DataTable
from .. import common
from ..common import utils
from . import rules

NoData = common.NoData

_LOG = infrastructure.get_logger(__name__)
LOG = utils.OnDemandStringParseLogger(_LOG)


class DetectLineInputs(vdp.StandardInputs):
    window = vdp.VisDependentProperty(default=[])
    edge = vdp.VisDependentProperty(default=(0,0))
    broadline = vdp.VisDependentProperty(default=True)
    
    def __init__(self, context, group_id=None, window=None, edge=None, broadline=None):
        super(DetectLineInputs, self).__init__()
        
        self.context = context
        self.group_id = group_id
        self.window = window
        self.edge = edge
        self.broadline = broadline


class DetectLineResults(common.SingleDishResults):
    def __init__(self, task=None, success=None, outcome=None):
        super(DetectLineResults, self).__init__(task, success, outcome)

    def merge_with_context(self, context):
        LOG.debug('DetectLineResults.merge_with_context')
        super(DetectLineResults, self).merge_with_context(context)
        # exporting datatable should be done within the parent task
#         if type(self.outcome) is types.DictType and 'datatable' in self.outcome:
#             datatable = self.outcome.pop('datatable')
#             start_time = time.time()
#             LOG.debug('Start exporting datatable (minimal): {}', start_time)
#             datatable.exportdata(minimal=True)
#             end_time = time.time()
#             LOG.debug('End exporting datatable (minimal): {} ({} sec)', end_time, end_time - start_time)
         
    @property
    def signals(self):
        return self._get_outcome('signals')
        
    def _outcome_name(self):
        return ''


class DetectLine(basetask.StandardTaskTemplate):
    Inputs = DetectLineInputs
    LineFinder = heuristics.HeuristicsLineFinder
    ThresholdFactor = 3.0

    def __init__(self, inputs):
        super(DetectLine, self).__init__(inputs)
        self.line_finder = self.LineFinder()

    def prepare(self, datatable_dict=None, grid_table=None, spectral_data=None):
        """
        The process finds emission lines and determines protection regions for baselinefit
        """
        assert grid_table is not None
        assert datatable_dict is not None
        spectra = spectral_data
        masks = (spectra != NoData)
        window = self.inputs.window
        edge = self.inputs.edge
        broadline = self.inputs.broadline
        
        detect_signal = {}

        # Pre-Defined Spectrum Window
        if len(window) != 0:
            LOG.info('Skip line detection since line window is set.')
            nrow = len(grid_table)
            assert nrow > 0
            spw = grid_table[0][0] if len(grid_table) > 0 else -1
            predefined_window = self._get_predefined_window(spw, window)
            LOG.trace('predefined_window={0}'.format(predefined_window))
            for row in xrange(nrow):
                grid_info = grid_table[row]
                ra = grid_info[4]
                dec = grid_info[5]
                detect_signal[row] = [ra, dec, predefined_window]
            for datatable in datatable_dict.itervalues():
                for dt_row in xrange(datatable.nrow):
                    datatable.putcell('MASKLIST', dt_row, predefined_window)
                
            result = DetectLineResults(task=self.__class__,
                                       success=True,
                                       outcome={'signals': detect_signal})

            result.task = self.__class__

            return result

        # move assertion for spectral_data here since spectral_data is 
        # not necessary when line window is specified
        assert spectral_data is not None

        #(nchan,nrow) = spectra.shape
        (nrow, nchan) = spectra.shape

        LOG.info('Search regions for protection against the background subtraction...')
        LOG.info('DetectLine: Processing {} spectra...', nrow)

        # Set edge mask region
        (EdgeL, EdgeR) = common.parseEdge(edge)
        Nedge = EdgeR + EdgeL
        LOG.info('edge={}', edge)
        LOG.info('EdgeL, EdgeR={}, {}', EdgeL, EdgeR)
        LOG.info('Nedge={}', Nedge)
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
        Timer = common.ProgressTimer(80, nrow, LOG.logger.level)
        # 100.0: minimum number of channels for binned spectrum to detect lines
        MinChanBinSp = 50.0
        TmpRange = [4**i for i in xrange(int(math.ceil(math.log(len(spectra[0])/MinChanBinSp)/math.log(4))))]
        BinningRange = []
        for i in TmpRange:
            BinningRange.append([i, 0])
            if i > 1:
                BinningRange.append([i, i/2])
        for row in xrange(nrow):
            # Countup progress timer
            Timer.count()

            ProcStartTime = time.time()
            Protected = []
            if len(grid_table[row][6]) == 0:
                LOG.debug('Row {}: No spectrum', row)
                # No spectrum
                Protected = [[-1, -1, 1]]
            else:
                LOG.debug('Start Row {}', row)
                for [BINN, offset] in BinningRange:
                    MinNchan = (MinFWHM-2) / BINN + 2
                    SP = self.SpBinning(spectra[row], BINN, offset)
                    MSK = self.MaskBinning(masks[row], BINN, offset)
    
                    protected = self._detect(spectrum=SP,
                                             mask=MSK,
                                             threshold=Thre+math.sqrt(BINN)-1.0,
                                             tweak=True,
                                             edge=(EdgeL, EdgeR))

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
                            Protected.append([-1, -1, BINN])

                # plot to check detected lines
                #self.plot_detectrange(spectra[row], Protected, 'SpPlot0%04d.png' % row)

            detect_signal[row] = [grid_table[row][4],  # RA
                                  grid_table[row][5],  # DEC
                                  Protected]           # Protected Region
            ProcEndTime = time.time()
            LOG.info('Channel ranges of detected lines for Row {}: {}', row, detect_signal[row][2])
            
            LOG.debug('End Row {}: Elapsed Time={:.1} sec', row, (ProcEndTime - ProcStartTime))
        del Timer

        #LOG.debug('DetectSignal = %s'%(detect_signal))
        result = DetectLineResults(task=self.__class__,
                                   success=True,
                                   outcome={'signals': detect_signal})
                
        result.task = self.__class__
                
        return result
    
    def plot_detectrange(self, sp, protected, fname):
        print(protected, fname)
        PL.clf()
        PL.plot(sp)
        ymin, ymax = PL.ylim()
        for i in range(len(protected)):
            y = ymin + (ymax - ymin)/30.0 * (i + 1)
            PL.plot(protected[i], (y, y), 'r')
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
        LOG.trace('threshold (S/N per channel)={}, channels, edges to be dropped=[{}, {}]', 
                  threshold, EdgeL, EdgeR)
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
            LOG.debug('Ranges={}, Width={}', line_ranges[y*2:y*2+2], Width)
            if (Width >= MinFWHM and Width <= MaxFWHM and line_ranges[y*2] > EdgeL and
                    line_ranges[y*2+1] < (nchan - 1 - EdgeR)):
                protected.append([line_ranges[y*2], line_ranges[y*2+1]])
        if len(protected) == 0:
            protected = [[-1, -1]]
        elif len(protected) > 1:
            # 2007/09/01 add merged lines to the list if two lines are close enough
            flag = True
            for y in range(len(protected) - 1):
                curr0, curr1 = protected[y][0], protected[y][1]
                next0, next1 = protected[y+1][0], protected[y+1][1]
                if (next0 - curr1) < (0.25*min((curr1-curr0), (next1-next0))):
                    if flag:
                        if curr1 < next1 and curr0 < next0 and (next1 - curr0) <= MaxFWHM:
                            protected.append([curr0, next1])
                            Line0 = curr0
                        else:
                            continue
                    else:
                        if (next1 - Line0) <= MaxFWHM:
                            protected.pop()
                            protected.append([Line0, next1])
                        else:
                            flag = True
                            continue
                    flag = False
                else:
                    flag = True
        return protected

    def _get_predefined_window(self, spw, window):
        # CAS-10764 flexible line masking -- supported format
        #  - integer list [chmin, chmax]
        #  - nested integer list [[chmin, chmax], [chmin, chmax], ...] 
        #  - float list [fmin, fmax]
        #  - nested float list [[fmin, fmax], [fmin, fmax], ...]
        #  - string list ['XGHz', 'YGHz']
        #  - nested string list [['XGHz', 'YGHz'], ['aMHz', 'bMHz'], ...]
        #  - channel selection string 'A:chmin~chmax;chmin~chmax,B:fmin~fmax,...'
        group_id = self.inputs.group_id
        group_desc = self.inputs.context.observing_run.ms_reduction_group[group_id]
        ms = group_desc[0].ms
        field_id = group_desc[0].field_id
        parser = LineWindowParser(ms, window)
        parser.parse(field_id)
        new_window = parser.get_result(spw)
        
        # TODO
        # Channel range could be specified by LSRK frequency. This means that 
        # effective channnel range in TOPO frame may be different between MSs. 
        # This effect is not taken into account because (1) line detection 
        # and validation stage doesn't support frequency change over MSs 
        # (i.e. gridding is done in TOPO frame), and (2) data format 
        # for detected lines doesn't support per-MS line lists. This have to be 
        # considered later.
        
        return new_window
    
#         if len(window) == 0:
#             return []
#         else:
#             if hasattr(window[0], '__iter__'):
#                 return [self._get_linerange(spw, w) for w in window]
#             else:
#                 return [self._get_linerange(spw, window)]
# 
#     def _get_linerange(self, spwid, window):
#         if spwid < 0:
#             raise RuntimeError("Invalid spw id ({})".format(spwid))
#         
#         ms = self.inputs.context.observing_run.measurement_sets[0]
#         parsed_window = get_linerange(window, spwid, ms)
#         return parsed_window
#     
# 
# def get_linerange(window, spwid, ms):
#     if len(window) == 2:
#         # [chmin, chmax] form
#         return window
#     elif len(window) == 3:
#         # [center_freq, velmin, velmax] form
#         spw = ms.spectral_windows[spwid]
#         center_freq = window[0] * 1.0e9  # GHz -> Hz
#         target_fields = ms.get_fields(intent='TARGET')
#         source_id = target_fields[0].source_id
#         restfreq = utils.get_restfrequency(ms.name, spwid, source_id)
#         if restfreq is None:
#             restfreq = float(spw.ref_frequency.to_units(measures.FrequencyUnits.HERTZ).value)
#         #restfreq = spw.refval if len(spw.rest_frequencies) == 0 else spw.rest_frequencies[0]
#         dfreq = map(lambda x: restfreq * abs(x) / 299792.458, window[1:])
#         freq_range = [center_freq + dfreq[0], center_freq - dfreq[1]]
#         refpix = 0
#         refval = spw.channels.chan_freqs.start
#         increment = spw.channels.chan_freqs.delta
#         window = map(lambda x: refpix + (x - refval) / increment, freq_range)
#         window.sort()
#         return window
#     else:
#         raise RuntimeError('Invalid linewindow format')


class LineWindowParser(object):
    """
    LineWindowParser is a parser for line window parameter. 
    Supported format is as follows:
    
    [Single window list] -- apply to all spectral windows
      - integer list [chmin, chmax]
      - float list [fmin, fmax]
      - string list ['XGHz', 'YGHz']
    
    [Multiple window list] -- apply to all spectral windows
      - nested integer list [[chmin, chmax], [chmin, chmax], ...] 
      - nested float list [[fmin, fmax], [fmin, fmax], ...]
      - nested string list [['XGHz', 'YGHz'], ['aMHz', 'bMHz'], ...]
      
    [Dictionary] -- apply to selected spectral windows
      - {<spwid>: <window list>} style dictionary
      - spwid should be an integer specifying spectral window id
      - window list should be in one of the above list-type window formats

    [MS channel selection syntax] -- apply to selected spectral windows
      - channel selection string 'A:chmin~chmax;chmin~chmax,B:fmin~fmax,...'
    
    Note that frequencies are interpreted as the value in LSRK frame. 
    Note also that frequencies given as a floating point number is interpreted 
    as the value in Hz.
    """
    def __init__(self, ms, window):
        """
        Constructor
        
        ms -- ms domain object
        window -- line window parameter
        """
        self.ms = ms
        self.window = window
        self.parsed = None
        
        # science spectral windows
        self.science_spw = map(lambda x: x.id, self.ms.get_spectral_windows(science_windows_only=True))
        
        # measure tool
        self.me = casatools.measures
            
    def parse(self, field_id):
        # convert self.window into dictionary
        if type(self.window) == str:
            # shoudl be MS channel selection syntax
            # convert string into dictionary
            # then, filter out non-science spectral windows
            processed = self._exclude_non_science_spws(self._string2dict(self.window))
        elif type(self.window) == list or type(self.window) == numpy.ndarray:
            # convert string into dictionary
            # keys are all science spectral window ids
            processed = self._list2dict(self.window)
        elif type(self.window) == dict:
            # filter out non-science spectral windows
            processed = self._exclude_non_science_spws(self._dict2dict(self.window))
        else:
            # unsupported format or None
            processed = dict((spw, []) for spw in self.science_spw)
            
        # convert frequency selection into channel selection
        self.parsed = {}
        self._measure_init(field_id)
        try:
            for (spwid, _window) in processed.iteritems():
                LOG.trace('_window={0} type {1}', _window, type(_window))
                new_window = self._freq2chan(spwid, _window)
                if len(new_window) > 0 and type(new_window[0]) != list:
                    new_window = [new_window]
#                 if len(new_window) > 0:
#                     tmp = []
#                     for w in new_window:
#                         if len(w) == 2:
#                             tmp.append(w)
#                     new_window = tmp
                self.parsed[spwid] = new_window
        finally:
            self._measure_done()
            
        # consistency check
        for spwid in self.science_spw:
            assert spwid in self.parsed
    
    def get_result(self, spw_id):
        if spw_id not in self.science_spw:
            LOG.info('Non-science spectral window was specified. Returning default window [].')
            return []
        
        if self.parsed is None:
            LOG.info('You need to run parse method first. Returning default window [].')
            return []
        
        if spw_id not in self.parsed:
            LOG.info('Unexpected behavior. Returning default window [].')
            return []
        
        return self.parsed[spw_id]
    
    def _string2dict(self, window):
        # utilize ms tool to convert selection string into lists
        with casatools.MSReader(self.ms.name) as ms:
            try:
                ms.msselect({'spw': window})
                idx = ms.msselectedindices()
            except RuntimeError, e:
                msg = e.message
                LOG.warn(msg)
                if msg.startswith('No valid SPW'):
                    idx = {'channel': []}
                else:
                    raise e
            
        new_window = {}
        channel_selection = idx['channel']
        for sel in channel_selection:
            assert len(sel) == 4
            
            spwid = sel[0]
            chansel = list(sel[1:3])
            if spwid not in new_window:
                new_window[spwid] = []
            
            new_window[spwid].append(chansel)
            
        for spwid in self.science_spw:
            if spwid not in new_window:
                new_window[spwid] = []
            
        return new_window
    
    def _list2dict(self, window):
        # apply given window to all science windows

        return dict((spwid, window) for spwid in self.science_spw)
    
    def _dict2dict(self, window):
        # key should be an integer
        return dict((int(spw), value) for spw, value in window.iteritems())
    
    def _exclude_non_science_spws(self, window):
        # filter non-science windows
        # set default window to science windows if not specified
        new_window = {}
        for spwid in self.science_spw:
            if spwid in window:
                new_window[spwid] = list(window[spwid])
            else:
                new_window[spwid] = []
                
        return new_window
    
    def _freq2chan(self, spwid, window):
        # window must be a list
        assert type(window) == list
        
        # return without conversion if empty list
        if len(window) == 0:
            return window
        
        item_type = type(window[0])
        
        # process recursively if item is a list
        if item_type in (list, numpy.ndarray):
            converted = []
            for w in window:
                LOG.trace('_freq2chan: w={0} type {1}', w, type(w))
                _w = self._freq2chan(spwid, w)
                LOG.trace('_freq2chan: _w={0} type {1}', _w, type(_w))
                if len(_w) == 2:
                    converted.append(_w)
            
            return converted
        
        
        # return without conversion if item is an integer
        if item_type in (int, numpy.int32, numpy.int64):
            window.sort()
            return window
        
        # convert floating-point value to quantity string
        if item_type in (float, numpy.float32, numpy.float64):
            return self._freq2chan(spwid, map(lambda x: '{0}Hz'.format(x), window))
    
        # now list item should be a quantity string
        assert item_type == str, 'unexpected item type {0}'.format(item_type)
        
        # also, length of the window should be 2
        assert len(window) == 2
        
        # frequency conversion from LSRK to TOPO
        new_window = self._lsrk2topo(spwid, window)
                
        # construct ms channel selection syntax 
        spwsel = self._construct_msselection(spwid, new_window)
        
        # channel mapping using ms tool
        processed = self._string2dict(spwsel)
        
        # target spwid should exist
        assert spwid in processed
        
        new_window = processed[spwid]
        new_window.sort()
        LOG.trace('_freq2chan: new_window={0} type {1}', new_window, type(new_window))
        if len(new_window) == 0:
            return []
        assert len(new_window) == 1
        return new_window[0]
        
    def _lsrk2topo(self, spwid, window):
        # if frequency frame for target spw is LSRK, just return input window
        spw = self.ms.get_spectral_window(spwid)
        frame = spw.frame
        if frame == 'LSRK':
            return window
        
        # assuming that measure tool is properly initialized
        qa = casatools.quanta
        qfreq = map(lambda x: qa.quantity(x), window)
        if qa.gt(qfreq[0], qfreq[1]):
            qfreq = [qfreq[1], qfreq[0]]
        mfreq = map(lambda x: self.me.frequency(rf='LSRK', v0=x), qfreq)
        new_mfreq = map(lambda x: self.me.measure(v=x, rf=frame), mfreq)
        new_window = map(lambda x: '{value}{unit}'.format(**x['m0']), new_mfreq)
        return new_window
    
    def _construct_msselection(self, spwid, window):
        return '{0}:{1}~{2}'.format(spwid, window[0], window[1])
    
    def _measure_init(self, field_id):
        self._measure_done()
        # position is an observatory position
        position = self.ms.antenna_array.position
        
        # direction is a field reference direction
        fields = self.ms.get_fields(field_id=field_id)
        direction = fields[0].mdirection
        
        # epoch is an observing start time
        epoch = self.ms.start_time
        
        # initialize the measure
        self.me.doframe(position)
        self.me.doframe(direction)
        self.me.doframe(epoch)
        
    def _measure_done(self):
        self.me.done()
    

def test_parser(ms):
    target_fields = ms.get_fields(intent='TARGET')
    field_id = target_fields[0].id
    science_spws = ms.get_spectral_windows(science_windows_only=True)
    science_spw_ids = map(lambda x: x.id, science_spws)
    # alias for science_spw_ids
    spwids = science_spw_ids
    chan_freq0 = science_spws[0].channels.chan_freqs.start
    increment0 = science_spws[0].channels.chan_freqs.delta
    get_chan_freq0 = lambda x: chan_freq0 + increment0 * x
    get_chan_qfreq0 = lambda x: '{0}Hz'.format(get_chan_freq0(x))
    chan_freq1 = science_spws[-1].channels.chan_freqs.start
    increment1 = science_spws[-1].channels.chan_freqs.delta
    get_chan_freq1 = lambda x: chan_freq1 + increment1 * x
    get_chan_qfreq1 = lambda x: '{0}Hz'.format(get_chan_freq1(x))
    if increment0 > 0:
        f0 = get_chan_qfreq0(100)
        f1 = get_chan_qfreq0(200)
    else:
        f0 = get_chan_qfreq0(200)
        f1 = get_chan_qfreq0(100)
    if increment1 > 0:
        f2 = get_chan_qfreq1(100)
        f3 = get_chan_qfreq1(200)
        f4 = get_chan_qfreq1(500)
        f5 = get_chan_qfreq1(700)
    else:
        f2 = get_chan_qfreq1(200)
        f3 = get_chan_qfreq1(100)
        f4 = get_chan_qfreq1(700)
        f5 = get_chan_qfreq1(500)
        
    test_cases = [
        # single global window (channel)
        [100, 200], 
        # multiple global window (channel)
        [[100, 200], [500,700]], 
        # per spw windows (channel)
        {spwids[0]: [100, 200], spwids[-1]: [[100, 200], [500,700]]},
        # single global window (frequency value)
        [get_chan_freq0(100), get_chan_freq0(200)],
        # multiple global window (frequency value)
        [[get_chan_freq0(100), get_chan_freq0(200)], [get_chan_freq0(500), get_chan_freq0(700)]],
        # per spw windows (frequency vaule)
        {spwids[0]: [get_chan_freq0(100), get_chan_freq0(200)],
         spwids[-1]: [[get_chan_freq1(100), get_chan_freq1(200)], [get_chan_freq1(500), get_chan_freq1(700)]]},
        # single global window (frequency quantity)
        [get_chan_qfreq0(100), get_chan_qfreq0(200)],
        # multiple global window (frequency quantity)
        [[get_chan_qfreq0(100), get_chan_qfreq0(200)], [get_chan_qfreq0(500), get_chan_qfreq0(700)]],
        # per spw windows (frequency quantity)
        {spwids[0]: [get_chan_qfreq0(100), get_chan_qfreq0(200)],
         spwids[-1]: [[get_chan_qfreq1(100), get_chan_qfreq1(200)], [get_chan_qfreq1(500), get_chan_qfreq1(700)]]},
        # per spw windows (string key)
        {str(spwids[0]): [get_chan_qfreq0(100), get_chan_qfreq0(200)],
         str(spwids[-1]): [[get_chan_qfreq1(100), get_chan_qfreq1(200)], [get_chan_qfreq1(500), get_chan_qfreq1(700)]]},
        # per spw windows (mixed)
        {spwids[0]: [100, 200],
         spwids[-1]: [[get_chan_qfreq1(100), get_chan_qfreq1(200)], [get_chan_qfreq1(500), get_chan_qfreq1(700)]]},
        # MS channel selection string (channel)
        '{0}:{1}~{2},{3}:{4}~{5};{6}~{7}'.format(spwids[0], 100, 200, spwids[-1], 100, 200, 500, 700),
        # MS channel selection string (frequency)
        '{0}:{1}~{2},{3}:{4}~{5};{6}~{7}'.format(spwids[0], f0, f1, 
                                                 spwids[-1], f2, f3, f4, f5)        
        ]
    
    results = []
    for window in test_cases:
        s = 'INPUT WINDOW: {0} (type {1})\n'.format(window, type(window))
        print(s)
        parser = LineWindowParser(ms, window)
        parser.parse(field_id)
        for spwid in spwids:
            parsed = parser.get_result(spwid)
            s += '\tSPW {0}: PARSED WINDOW = {1}\n'.format(spwid, parsed)
        results.append(s)
        
    print('=== TEST RESULTS ===')
    for s in results:
        print(s)

# def get_restfrequency(vis, spwid, source_id):
#     source_table = os.path.join(vis, 'SOURCE')
#     with casatools.TableReader(source_table) as tb:
#         tsel = tb.query('SOURCE_ID == {} && SPECTRAL_WINDOW_ID == {}'.format(source_id, spwid))
#         try:
#             if tsel.nrows() == 0:
#                 return None
#             else:
#                 if tsel.iscelldefined('REST_FREQUENCY', 0):
#                     return tsel.getcell('REST_FREQUENCY', 0)[0]
#                 else:
#                     return None
#         finally:
#             tsel.close()
