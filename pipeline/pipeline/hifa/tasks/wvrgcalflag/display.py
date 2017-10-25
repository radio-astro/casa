from __future__ import absolute_import
import collections
import os
import operator

import cachetools
import matplotlib
import matplotlib.pyplot as pyplot
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.utils as utils
from pipeline.infrastructure.displays import common
from pipeline.infrastructure.displays import phaseoffset

LOG = infrastructure.get_logger(__name__)


class WVRScoreFinder(object):
    def __init__(self, delegate):
        self._delegate = delegate
        self._cache = cachetools.LRUCache(maxsize=1000)
        
    @cachetools.cachedmethod(operator.attrgetter('_cache'))    
    def get_score(self, spw, antenna):
        spw_id = spw.id
        antenna_id = antenna.id
        
        spw_viewlist = [viewlist for viewlist in self._delegate.view.values() 
                        if viewlist[0].spw == spw_id]

        if len(spw_viewlist) <= 0:
            return 0.0

        LOG.todo('Is the QA score the first or last viewlist?')
        spw_imageresult = spw_viewlist[0][-1]
        
        antenna_axis = [axis for axis in spw_imageresult.axes if axis.name == 'Antenna'][0]
        antenna_idx = list(antenna_axis.data).index(antenna_id)
        
        scores_for_antenna = spw_imageresult.data[antenna_idx]

        return scores_for_antenna[0]

    
class WVRPhaseVsBaselineChart(object):
    class WvrChartHelper(object):
        def __init__(self, antennas):
            self._antennas = antennas

        def get_antennas(self):
            return self._antennas[:]
                        
        def label_antenna(self, axes):
            pyplot.title('All Antennas', size=10)

        @property
        def antenna_filename_component(self):
            return ''

    def _load_caltables(self):
        if self._caltables_loaded:
            return
        
        # Get phases before and after
        data_before = common.CaltableWrapper.from_caltable(self._table_before)
        data_after = common.CaltableWrapper.from_caltable(self._table_after)
        
        # some sanity checks, as unequal caltables have bit me before
        # TODO with- and without wvr plots have different times for X16b,
        # causing these assertions to fail. We need to understand why.
#         assert utils.areEqual(data_before.time, data_after.time), 'Time columns are not equal'
#         assert utils.areEqual(data_before.antenna, data_after.antenna), 'Antenna columns are not equal'
#         assert utils.areEqual(data_before.spw, data_after.spw), 'Spw columns are not equal'
#         assert utils.areEqual(data_before.scan, data_after.scan), 'Scan columns are not equal'

        self._data_before = data_before
        self._data_after = data_after
        self._caltables_loaded = True

    def _get_plot_intents(self):
        return set(self.result.dataresult.inputs['qa_intent'].split(','))
    
    def _get_plot_scans(self):
        plot_intents = self._get_plot_intents()
        return [scan for scan in self.ms.scans 
                if not plot_intents.isdisjoint(scan.intents)]

    def get_symbol_and_colour(self, pol, state='BEFORE'):
        """
        Get the plot symbol and colour for this polarization and bandtype.
        """
        d = {'BEFORE': {'L': ('-', 'orange', 0.3),
                        'R': ('--', 'sandybrown', 0.3),
                        'X': ('^', 'lightslategray', 0.3),
                        'Y': ('o', 'lightslategray', 0.3),
                        'XX': ('^', 'lightslategray', 0.3),
                        'YY': ('o', 'lightslategray', 0.3)},
             'AFTER': {'L': ('-', 'green', 0.6),
                       'R': ('-', 'red', 0.6),
                       'X': ('^', 'green', 0.6),
                       'Y': ('o', 'red', 0.6),
                       'XX': ('^', 'green', 0.6),
                       'YY': ('o', 'red', 0.6)}}
    
        return d.get(state, {}).get(pol, ('x', 'grey'))

    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self._caltables_loaded = False

        nowvr_gaintables = set([c.gaintable for c in result.dataresult.nowvr_result.pool])
        assert len(nowvr_gaintables) is 1, ('Unexpected number of pre-WVR phase-up'
                                            'gaintables: %s' % nowvr_gaintables) 

        nowvr_gaintable = nowvr_gaintables.pop()
        wvr_gaintable = result.dataresult.qa_wvr.gaintable_wvr
        LOG.debug('Gaintables for WVR plots:\n'
                  'No WVR: %s\tWith WVR: %s' % (nowvr_gaintable, wvr_gaintable))    

        self._table_before = nowvr_gaintable
        self._table_after = wvr_gaintable
        
        self._score_retriever = WVRScoreFinder(result.viewresult)

        self._wrappers = []

        refant_name = result.dataresult.nowvr_result.inputs['refant'].split(',')[0]
        self._refant = self.ms.get_antenna(refant_name)[0]        

    def get_data_object(self, data, corr_id):
        delegate = common.PhaseVsBaselineData(data, self.ms, corr_id, 
                                              self._refant.id)
        return common.XYData(delegate, 'distance_to_refant', 'median_offset')

    def plot(self):
        self._load_caltables()
        data_before = self._data_before
        data_after = self._data_after

        # get the windows this was tested on from the caltable.
        spw_ids = set(data_before.spw).intersection(set(data_after.spw))
        spws = set([spw for spw in self.ms.spectral_windows if spw.id in spw_ids])

        plot_scans = self._get_plot_scans()

        # phase offsets are plotted per corr, spw and scan. We cannot index
        # the phase arrays with multiple corr/spw/scans as the unwrapped
        # phases would be for the whole data, not for the corr/spw/scan 
        # combination we want to plot
        LOG.debug('Finding maximum phase offset over all scans/spws/corrs/antennas')
        for scan in plot_scans:
            # scan may not have all spws, so just process those present
            for spw in scan.spws.intersection(spws):
                # find the data description for this scan. Just one dd 
                # expected.
                dds = [dd for dd in scan.data_descriptions 
                       if dd.spw.id is spw.id]
                if len(dds) is not 1:
                    LOG.info('Bypassing plot generation for %s scan %s spw '
                             '%s. Expected 1 matching data description but '
                             'got %s.', 
                             self.ms.basename, scan.id, spw.id, len(dds))
                    continue

                dd = dds[0]
                # we expect the number and identity of the caltable 
                # correlations for this scan to match those in the MS, so we
                # can enumerate over the correlations in the MS scan.  
                for corr_id, _ in enumerate(dd.polarizations):
                    for antenna in self.ms.antennas:
                        # we don't want the phase RMS for the reference antenna as it
                        # doesn't make any sense, plus it often 'spikes' the scale with
                        # an extreme value
                        if antenna.id is self._refant.id:
                            continue
                        
                        try:                        
                            caltable = data_before.filename
                            selection_before = data_before.filter(scan=[scan.id], 
                                                                  antenna=[antenna.id], 
                                                                  spw=[spw.id])
                            baseline_data_before = self.get_data_object(selection_before, 
                                                                        corr_id)
                            
                            caltable = data_after.filename
                            selection_after = data_after.filter(scan=[scan.id], 
                                                                antenna=[antenna.id], 
                                                                spw=[spw.id])
                            baseline_data_after = self.get_data_object(selection_after, 
                                                                       corr_id)

                        except (ValueError, KeyError):
                            # We can't construct data objects for completely
                            # flagged selections 
                            LOG.debug('Could not evaluate data for %s '
                                      'antenna %s spw %s scan %s. Data '
                                      'completely flagged?' % (caltable,
                                                               antenna.name,
                                                               spw.id,
                                                               scan.id))

                            continue

                        wrapper = common.DataRatio(baseline_data_before, 
                                                   baseline_data_after)
                        self._wrappers.append(wrapper)

        offsets = [w.before.y for w in self._wrappers]
        offsets.extend([w.after.y for w in self._wrappers])
        # offsets could contain None where data was flagged, but that's ok as 
        # max ignores it.
        self._max_phase_offset = numpy.ma.max(offsets)
        LOG.trace('Maximum phase offset for %s = %s' % (self.ms.basename, 
                                                        self._max_phase_offset))

        # Extract ratios, excluding any that are set to None.
        ratios = [w.y for w in self._wrappers if w.y is not None]

        # Convert to masked array that masks out invalid numbers (NaN, Inf).
        ratios = numpy.ma.masked_invalid(ratios)

        # Determine whether an alternate refant might have been used by
        # assessing if the minimum ratio was 0.
        self._alt_refant_used = (ratios.min() == 0.0)

        # If a minimum ratio of 0 was found, raise warning about alternate
        # refant, and update ratios to also mask out values of 0.
        if self._alt_refant_used:
            LOG.warning('Phase ratio of 0 suggests that alternate refant was '
                        'used during gaincal. This plot might be misleading!')
            ratios = numpy.ma.masked_equal(ratios, 0)

        self._max_ratio = ratios.max()
        self._min_ratio = ratios.min()
        self._median_ratio = numpy.ma.median(ratios)
        LOG.trace('Maximum phase ratio for %s = %s' % (self.ms.basename, 
                                                       self._max_ratio))
        LOG.trace('Minimum phase ratio for %s = %s' % (self.ms.basename, 
                                                       self._min_ratio))

        distances = [w.x for w in self._wrappers]
        self._max_distance = numpy.ma.max(distances)
        LOG.trace('Maximum distance for %s = %s' % (self.ms.basename, 
                                                    self._max_distance))

        plots = []
        for spw in spws:
            # plot scans individually as plotting multiple scans on one plot
            # creates an unintelligible mess. 
            for scan in plot_scans:
                # if spw.id == 17 and scan.id == 3:
                plots.append(self.get_plot_wrapper(spw, [scan, ],
                                                   self.ms.antennas))

        return [p for p in plots if p is not None]

    def create_plot(self, spw, scans, helper):
        data_before = self._data_before
        data_after = self._data_after

        # check the spw given by the spw argument is present in both caltables
        assert spw.id in data_before.spw, 'Spw %s not in %s' % (spw, self._table_before)
        assert spw.id in data_after.spw, 'Spw %s not in %s' % (spw, self._table_after)
    
        # get the scan intents from the list of scans
        scan_intents = set()
        for scan in scans:
            scan_intents.update(scan.intents)
        scan_intents.remove('WVR')
        scan_intents = ','.join(scan_intents)
 
        # get the fields from the list of scans
        scan_fields = set()
        for scan in scans:
            scan_fields.update([field.name for field in scan.fields])
        scan_fields = ','.join(scan_fields)
            
        # create the figure: 2 rows x 1 column, sharing the X axis (baseline
        # length) 
        fig, ((ax1, ax2)) = common.subplots(2, 1, sharex=True)
        ax1.set_yscale('log')
        pyplot.subplots_adjust(hspace=0.0)

        trans1 = matplotlib.transforms.blended_transform_factory(ax1.transAxes, 
                                                                 ax1.transData)
        ax1.axhspan(self._min_ratio, 1, facecolor='k', linewidth=0.0, alpha=0.04)
        ax1.text(0.012, numpy.sqrt(self._min_ratio), 'No Improvement',
                 transform=trans1, color='k', ha='left', va='center', size=8, alpha=0.4)

        ax1.axhline(y=self._median_ratio, color='k', ls='dotted', alpha=0.4)
        ax1.text(0.012, self._median_ratio, 'Median', transform=trans1,
                 color='k', ha='left', va='baseline', size=8, alpha=0.4)
  
        # create bottom plot: phase offset vs baseline
        legend = []
        plots = []
        for scan in scans:

            # get the polarisations for the scan
            corr_axes = [tuple(dd.polarizations) for dd in scan.data_descriptions
                         if dd.spw.id == spw.id]
            # the scan may not necessarily contain the spw, as in multi-receiver EBs
            if not corr_axes:
                continue

            # discard WVR and other strange data descriptions
            corr_axes = set([x for x in corr_axes if x not in [(), ('I',)]])
            assert len(corr_axes) is 1, ('Data descriptions have different '
                                         'corr axes for scan %s. Got %s'
                                         '' % (scan.id, corr_axes))
            # go from set(('XX', 'YY')) to the ('XX', 'YY')
            corr_axes = corr_axes.pop()

            for corr_idx, corr_axis in enumerate(corr_axes):
                wrappers = [w for w in self._wrappers
                            if scan.id in w.scans 
                            and spw.id in w.spws
                            and corr_idx in w.corr]
    
                unflagged_wrappers = [w for w in wrappers if w.y is not None]
    
                # upper plot: ratio improvement
                x = [float(w.x) for w in unflagged_wrappers]
                y = [w.y for w in unflagged_wrappers]
                (symbol, color, alpha) = self.get_symbol_and_colour(corr_axis, state='AFTER')
                _, = ax1.plot(x, y, symbol, color=color, alpha=alpha)
    
                # lower plot: abs(median offset from median phase)
                x = [float(w.x) for w in unflagged_wrappers]
                y = [w.before.y for w in unflagged_wrappers]
                (symbol, color, alpha) = self.get_symbol_and_colour(corr_axis, state='BEFORE')                    
                p, = ax2.plot(x, y, symbol, color=color, alpha=alpha)
                plots.append(p)
                legend.append('%s %s' % (corr_axis, 'before'))

                y = [w.after.y for w in unflagged_wrappers]
                (symbol, color, alpha) = self.get_symbol_and_colour(corr_axis, state='AFTER')                    
                p, = ax2.plot(x, y, symbol, color=color, alpha=alpha)
                plots.append(p)
                legend.append('%s %s' % (corr_axis, 'after'))
        
        ax1.set_xlim(0, self._max_distance)
        ax1.set_ylim(self._min_ratio, self._max_ratio)
        ax2.set_ylim(0, self._max_phase_offset)
    
        # shrink the y height slightly to make room for the legend
        box1 = ax1.get_position()
        ax1.set_position([box1.x0, box1.y0 + box1.height * 0.06,
                          box1.width, box1.height * 0.94])
        box2 = ax2.get_position()
        ax2.set_position([box2.x0, box2.y0 + box2.height * 0.06,
                          box2.width, box2.height])

        ax1.set_ylabel('ratio', size=10)
        # CAS-7955: hif_timegaincal weblog: add refant next to phase(uvdist) plot
        x_axis_title = 'Distance to Reference Antenna {!s} (m)'.format(self._refant.name)
        ax2.set_xlabel(x_axis_title, size=10)
        ax2.set_ylabel('degrees', size=10)

        try:
            ax1.tick_params(labelsize=8, labelright='off')
            ax2.tick_params(labelsize=8, labelright='off')
        except:
            # CASA on Linux comes with old version of Matplotlib
            matplotlib.pyplot.setp(ax1.get_xticklabels(), fontsize=8)
            matplotlib.pyplot.setp(ax1.get_yticklabels(), fontsize=8)
            matplotlib.pyplot.setp(ax2.get_xticklabels(), fontsize=8)
            matplotlib.pyplot.setp(ax2.get_yticklabels(), fontsize=8)

        # CASA is using an old matplotlib, so we can't specify fontsize as
        # a property
        try:
            ax2.legend(plots, legend, prop={'size': 10}, numpoints=1,
                       loc='upper center', bbox_to_anchor=(0.5, -0.12),
                       frameon=False, ncol=len(legend))
        except TypeError:
            # old matplotlib doesn't expect frameon either
            l = ax2.legend(plots, legend, prop={'size': 10}, numpoints=1,
                           loc='upper center', bbox_to_anchor=(0.5, -0.12),
                           ncol=len(legend))
            l.draw_frame(False)
         
        spw_msg = 'SPW %s Correlation%s' % (spw.id, 
                utils.commafy(corr_axes, quotes=False, multi_prefix='s'))
        pyplot.text(0.0, 1.026, spw_msg, color='k', 
                    transform=ax1.transAxes, size=10)
        pyplot.text(0.5, 1.110, '%s (%s)' % (scan_fields, scan_intents), 
                    color='k', transform=ax1.transAxes, ha='center', size=10)
        pyplot.text(0.5, 1.026, 'All Antennas', color='k', 
                    transform=ax1.transAxes, ha='center', size=10)

        scan_ids = [str(s.id) for s in scans]
        max_scans_for_msg = 8
         
#         # print 'Scans 4, 8, 12 ... 146' if there are too many scans to 
#         # print
        if len(scans) > max_scans_for_msg:
            start = ','.join(scan_ids[0:max_scans_for_msg-1])
            end = scan_ids[-1]
            scan_txt = 's %s ... %s' % (start, end)
        else:
            scan_txt = utils.commafy(scan_ids, multi_prefix='s', 
                                     quotes=False, separator=',')
        pyplot.text(1.0, 1.026, 'Scan%s' % scan_txt, color='k', ha='right', 
                    transform=ax1.transAxes, size=10)

        pyplot.text(0.012, 0.97, 'Median Absolute Deviation from Median Phase', 
                    color='k', transform=ax2.transAxes, ha='left', va='top', 
                    size=10)
        pyplot.text(0.012, 0.97, 'Phase RMS without WVR / Phase RMS with WVR', 
                    color='k', transform=ax1.transAxes, ha='left', va='top', 
                    size=10)
        if self._alt_refant_used:
            pyplot.text(0.012, 0.90, 'Warning! Use of alternate refant detected; x-axis values may be unreliable',
                        color='r', transform=ax1.transAxes, ha='left', va='top',
                        size=10)

        # We need to draw the canvas, otherwise the labels won't be positioned and
        # won't have values yet.
        fig.canvas.draw()
        # omit the last y axis tick label from the lower plot
        ax2.set_yticklabels([t.get_text() for t in ax2.get_yticklabels()[0:-1]])
                
        figfile = self.get_figfile(spw, scans, helper.antenna_filename_component)
        pyplot.savefig(figfile)
        pyplot.close()

    def get_figfile(self, spw, scans, antennas):
        vis = os.path.basename(self.result.vis)
        scan_ids = '_'.join(['%0.2d' % scan.id for scan in scans])
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            '%s.phase_vs_baseline.spw%0.2d.scan%s.png' % (vis, spw.id, scan_ids))

    def get_plot_wrapper(self, spw, scans, antenna):
        figfile = self.get_figfile(spw, scans, antenna)
        scan_ids = ','.join([str(scan.id) for scan in scans])
        wrapper = logger.Plot(figfile,
                              x_axis='baseline length',
                              y_axis='phase offset',
                              parameters={'vis': os.path.basename(self.result.vis),
                                          'spw': spw.id,
                                          'scan': scan_ids})
        
        if not os.path.exists(figfile):
            LOG.trace('WVR phase vs baseline plot for spw %s scan %s not found. Creating new '
                      'plot: %s' % (spw.id, scan_ids, figfile))
            helper = WVRPhaseVsBaselineChart.WvrChartHelper(antenna)
            try:
                self.create_plot(spw, scans, helper)
            except Exception as ex:
                LOG.error('Could not create WVR phase vs baseline plot for'
                          ' spw %s scan %s' % (spw.id, scan_ids))
                LOG.exception(ex)
                # close figure just in case state is transferred between calls
                matplotlib.pyplot.clf()
                return None

        # the plot may not be created if all data for that antenna are flagged
        if os.path.exists(figfile):
            return wrapper            
        return None


class WVRPhaseOffsetPlotHelper(phaseoffset.PhaseOffsetPlotHelper):
    def __init__(self, context, result, plot_per_antenna=True):
        calapp = result.pool[0]
        
        rootdir = os.path.join(context.report_dir, 
                               'stage%s' % result.stage_number)
        prefix = '%s.phase_offset' % os.path.basename(calapp.vis)

        nowvr_gaintables = set([c.gaintable for c in result.nowvr_result.pool])
        assert len(nowvr_gaintables) is 1, ('Unexpected number of pre-WVR phase-up'
                                            'gaintables: %s' % nowvr_gaintables) 

        nowvr_gaintable = nowvr_gaintables.pop()
        wvr_gaintable = result.qa_wvr.gaintable_wvr
        LOG.debug('Gaintables for WVR plots:\n'
                  'No WVR: %s\tWith WVR: %s' % (nowvr_gaintable, wvr_gaintable))    

        caltable_map = collections.OrderedDict()
        caltable_map['BEFORE'] = nowvr_gaintable
        caltable_map['AFTER'] = wvr_gaintable

        super(WVRPhaseOffsetPlotHelper, self).__init__(rootdir, prefix, caltable_map, plot_per_antenna=plot_per_antenna)


class WVRPhaseOffsetPlot(phaseoffset.PhaseOffsetPlot):
    def __init__(self, context, result):
        vis = os.path.basename(result.dataresult.pool[0].vis)
        ms = context.observing_run.get_ms(vis)
        plothelper = WVRPhaseOffsetPlotHelper(context, result.dataresult)
        scan_intent = result.dataresult.inputs['qa_intent']
        score_retriever = WVRScoreFinder(result.viewresult)

        super(WVRPhaseOffsetPlot, self).__init__(
            context, ms, plothelper, scan_intent=scan_intent, score_retriever=score_retriever)


class WVRPhaseOffsetSummaryPlotHelper(WVRPhaseOffsetPlotHelper):
    def __init__(self, context, result):
        super(WVRPhaseOffsetSummaryPlotHelper, self).__init__(context, result, plot_per_antenna=False)


class WVRPhaseOffsetSummaryPlot(phaseoffset.PhaseOffsetPlot):
    def __init__(self, context, result):
        vis = os.path.basename(result.dataresult.pool[0].vis)
        ms = context.observing_run.get_ms(vis)
        plothelper = WVRPhaseOffsetSummaryPlotHelper(context, result.dataresult)
        scan_intent = result.dataresult.inputs['qa_intent']
        score_retriever = WVRScoreFinder(result.viewresult)

        super(WVRPhaseOffsetSummaryPlot, self).__init__(
            context, ms, plothelper, scan_intent=scan_intent, score_retriever=score_retriever)
