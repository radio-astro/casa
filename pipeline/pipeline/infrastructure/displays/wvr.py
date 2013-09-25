from __future__ import absolute_import
import os
import types

import matplotlib
import matplotlib.pyplot as pyplot
import numpy

import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.utils as utils

LOG = infrastructure.get_logger(__name__)


def _areEqual(a, b, msg=None):
    """
    Return True if the contents of the given arrays are equal.
    """
    return len(a) == len(b) and len(a) == sum([1 for i,j in zip(a,b) if i==j])


class WVRScoreFinder(object):
    def __init__(self, delegate):
        self._delegate = delegate
        
    @utils.memoized
    def get_score(self, spw, antenna):
        spw_id = spw.id
        antenna_id = antenna.id
        
        spw_viewlist = [viewlist for viewlist in self._delegate.view.values() 
                        if viewlist[0].spw==spw_id]
        assert len(spw_viewlist) is 1, ('Unexpected number of views for spw '
                                        '%s. Expected %s but got '
                                        '%s' % (spw_id, 1, len(spw_viewlist)))
        LOG.todo('Is the QA2 score the first or last viewlist?')
        spw_imageresult = spw_viewlist[0][-1]
        
        antenna_axis = [axis for axis in spw_imageresult.axes if axis.name == 'Antenna'][0]
        antenna_idx = list(antenna_axis.data).index(antenna_id)
        
        scores_for_antenna = spw_imageresult.data[antenna_idx]

        return scores_for_antenna[0]


class CaltableWrapper(object):
    @staticmethod
    def from_caltable(filename):
        with casatools.TableReader(filename) as tb:
            time_mjd = tb.getcol('TIME')
            antenna1 = tb.getcol('ANTENNA1')
            gain = tb.getcol('CPARAM')
            spw = tb.getcol('SPECTRAL_WINDOW_ID')
            scan = tb.getcol('SCAN_NUMBER')
            flag = tb.getcol('FLAG')
            phase = numpy.arctan2(numpy.imag(gain),
                                  numpy.real(gain)) * 180.0 / numpy.pi

            # shape of phase and flag arrays is number of corrs, 1, number of
            # rows. Remove the middle dimension.
            phases = phase[:,0]
            flags = flag[:,0]
            data = numpy.ma.MaskedArray(phases, mask=flags)

            # convert MJD times stored in caltable to matplotlib equivalent
            time_unix = utils.mjd_seconds_to_datetime(time_mjd)
            time_matplotlib = matplotlib.dates.date2num(time_unix)

            return CaltableWrapper(data, time_matplotlib, antenna1, spw, scan)

    def __init__(self, data, time, antenna, spw, scan):
        # tag the extra metadata columns onto our data array 
        self.data = data
        self.time = time
        self.antenna = antenna
        self.spw = spw
        self.scan = scan
    
        # get list of which spws, antennas and scans we have data for
        self._spws = frozenset(spw)
        self._antennas = frozenset(antenna)
        self._scans = frozenset(scan)

    def _get_mask(self, allowed, data):
        mask = numpy.zeros_like(data)
        for a in allowed:
            if a not in data:
                raise KeyError, '%s is not in caltable data' % a
            mask = (mask==1) | (data==a)
        return mask

    def filter(self, spw=None, antenna=None, scan=None):
        if spw is None:
            spw = self._spws
        if antenna is None:
            antenna = self._antennas
        if scan is None:
            scan = self._scans
            
        # get data selection mask for each selection parameter
        antenna_mask = self._get_mask(antenna, self.antenna)
        spw_mask = self._get_mask(spw, self.spw)
        scan_mask = self._get_mask(scan, self.scan)
        
        # combine masks to create final data selection mask
        mask = (antenna_mask==1) & (spw_mask==1) & (scan_mask==1)

        # find data for the selection mask 
        data = self.data[:,mask]
        time = self.time[mask]
        antenna = self.antenna[mask]
        spw = self.spw[mask]
        scan = self.scan[mask]
        
        # create new object for the filtered data
        return CaltableWrapper(data, time, antenna, spw, scan)


class PhaseVsBaselineData(object):
    def __init__(self, data, ms, corr_id):
        # While it is possible to do so, we shouldn't calculate statistics for
        # mixed antennas/spws/scans.
        assert len(set(data.antenna)) is 1, 'Data slice contains multiple antennas'
        assert len(set(data.spw)) is 1, 'Data slice contains multiple spws'
        assert len(set(data.scan)) is 1, 'Data slice contains multiple scans'

        self.data = data
        self.ms = ms
        self.corr = corr_id
        self.data_for_corr = self.data.data[corr_id]

    @property
    def num_corr_axes(self):
        return len(self.data.data)

    @property
    @utils.memoized
    def baselines(self):
        """
        Get the baselines for the antenna in this data selection in metres.
        """
        antenna_ids = set(self.data.antenna)
        baselines = [float(b.length.to_units(measures.DistanceUnits.METRE))
                    for b in self.ms.antenna_array.baselines
                     if b.antenna1.id in antenna_ids 
                     or b.antenna2.id in antenna_ids]                
        return baselines
    
    @property
    @utils.memoized
    def median_baseline(self):
        """
        Return the median baseline for this antenna in metres.
        """
        return numpy.median(self.baselines)

    @property          
    @utils.memoized  
    def mean_baseline(self):
        """
        Return the mean baseline for this antenna in metres.
        """
        return numpy.mean(self.baselines)

    @property
    @utils.memoized
    def unwrapped_data(self):
        rads = numpy.deg2rad(self.data_for_corr)
        unwrapped_rads = numpy.unwrap(rads)
        unwrapped_degs = numpy.rad2deg(unwrapped_rads)
        return unwrapped_degs
        
    @property
    @utils.memoized
    def offsets_from_median(self):
        unwrapped_degs = self.unwrapped_data
        deg_offsets = unwrapped_degs - numpy.ma.median(unwrapped_degs)
        # the operation above removed the mask, so add it back.
        remasked = numpy.ma.MaskedArray((deg_offsets), 
                                        mask=self.data_for_corr.mask)
        return remasked
        
    @property
    @utils.memoized
    def rms_offset(self):
        saved_handler = None
        saved_err = None
        try:
            return numpy.ma.std(self.offsets_from_median)
        except FloatingPointError:
            def err_handler(t, flag):
                ant = set(self.data.antenna).pop()
                spw = set(self.data.spw).pop()
                scan = set(self.data.scan).pop()                
                LOG.warn('Floating point error (%s) calculating RMS offset for'
                         ' Scan %s Spw %s Ant %s.' % (t, scan, spw, ant))
            saved_handler = numpy.seterrcall(err_handler)
            saved_err = numpy.seterr(all='call')
            return numpy.ma.std(self.offsets_from_median)
        finally:
            if saved_handler:
                numpy.seterrcall(saved_handler)
                numpy.seterr(**saved_err)
        
    @property
    @utils.memoized
    def median_offset(self):            
        abs_offset = numpy.ma.abs(self.offsets_from_median)
        return numpy.ma.median(abs_offset)

class DataRatio(object):
    def __init__(self, before, after):
        assert not set(before.data.antenna).isdisjoint(after.data.antenna), 'Data slices are for different antennas'
        assert not set(before.data.spw).isdisjoint(after.data.spw), 'Data slices are for different spws'
        assert not set(before.data.scan).isdisjoint(after.data.scan), 'Data slices are for different scans'
        assert before.corr == after.corr, 'Data slices are for different scans'

        self.before = before
        self.after = after

        self.antennas = set(before.data.antenna).union(set(after.data.antenna))
        self.spws = set(before.data.spw).union(set(after.data.spw))
        self.scans = set(before.data.scan).union(set(after.data.scan))
        self.corr = set((before.corr, after.corr))

    @property            
    def median_baseline(self):
        return self.before.median_baseline

    @property            
    def mean_baseline(self):
        """
        Return the average baseline for this antenna in metres.
        """
        return self.before.mean_baseline

    @property
    def num_corr_axes(self):
        # having ensured the before/after data are for the same scan and spw,
        # they should have the same number of correlations
        return len(self.before.data)

    @property
    @utils.memoized
    def ratio(self):
        rms_before = self.before.rms_offset
        rms_after = self.after.rms_offset
        
        if None in (rms_before, rms_after):
            return None
        # avoid divide by zero
        if rms_after == 0:
            return None
        return rms_before / rms_after

    
class WVRPlotBase(object):
    def get_symbol_and_colour(self, pol, state='BEFORE'):
        """
        Get the plot symbol and colour for this polarization and bandtype.
        """
        d = {'BEFORE' : {'L' : ('-', 'orange', 0.6),
                         'R' : ('--', 'sandybrown', 0.6),
                         'X' : ('-', 'lightslategray', 0.6),
                         'Y' : ('--', 'lightslategray', 0.6),
                         'XX' : ('-', 'lightslategray', 0.6),
                         'YY' : ('--', 'lightslategray', 0.6)},
             'AFTER' : {'L' : ('-', 'green', 0.6),
                        'R' : ('-', 'red', 0.6),
                        'X' : ('-', 'green', 0.6),
                        'Y' : ('-', 'red', 0.6),
                        'XX' : ('-', 'green', 0.6),
                        'YY' : ('-', 'red', 0.6)}}
    
        return d.get(state, {}).get(pol, ('x', 'grey'))

    def _load_caltables(self):
        if self._caltables_loaded:
            return
        
        # Get phases before and after
        data_before = CaltableWrapper.from_caltable(self._table_before)
        data_after = CaltableWrapper.from_caltable(self._table_after)
        
        # some sanity checks, as unequal caltables have bit me before
        assert _areEqual(data_before.time, data_after.time), 'Time columns are not equal'
        assert _areEqual(data_before.antenna, data_after.antenna), 'Antenna columns are not equal'
        assert _areEqual(data_before.spw, data_after.spw), 'Spw columns are not equal'
        assert _areEqual(data_before.scan, data_after.scan), 'Scan columns are not equal'

        self._data_before = data_before
        self._data_after = data_after
        self._caltables_loaded = True

    def _get_qa2_intents(self):
        return set(self.result.inputs['qa2_intent'].split(','))
    
    def _get_qa2_scans(self):
        qa2_intents = self._get_qa2_intents()
        return [scan for scan in self.ms.scans 
                if not qa2_intents.isdisjoint(scan.intents)]
    


class WVRChart(WVRPlotBase):
    class WvrChartHelper(object):
        def __init__(self, antenna):
            self._antenna = antenna

        def get_antennas(self):
            return [self._antenna,]
                        
        def label_antenna(self, fig):
            pyplot.text(0.5, 0.911, '%s' % self._antenna.name, color='k', 
                        transform=fig.transFigure, ha='center', size=10)

        @property
        def antenna_filename_component(self):
            return 'ant%s' % str(self._antenna.id)
    
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self._caltables_loaded = False

        nowvr_gaintables = set([c.gaintable for c in result.nowvr_result.final])
        assert len(nowvr_gaintables) is 1, ('Unexpected number of pre-WVR phase-up'
                                            'gaintables: %s' % nowvr_gaintables) 

        nowvr_gaintable = nowvr_gaintables.pop()
        wvr_gaintable = result.qa2.gaintable_wvr
        LOG.debug('Gaintables for WVR plots:\n'
                  'No WVR: %s\tWith WVR: %s' % (nowvr_gaintable, wvr_gaintable))    

        self._table_before = nowvr_gaintable
        self._table_after = wvr_gaintable
        self._load_caltables()
        
        self._score_retriever = WVRScoreFinder(result)

    def plot(self, spw_ids=None, antenna_ids=None, antenna_names=None):
        data_before = self._data_before
        data_after = self._data_after

        # get the windows this was tested on from the caltable.
        spw_ids = set(data_before.spw).intersection(set(data_after.spw))
        spws = [spw for spw in self.ms.spectral_windows if spw.id in spw_ids]

        qa2_scans = self._get_qa2_scans()

        # these function arguments are used for debugging the plot routines,
        # so we can call plot for a particular troublesome spw/antenna.
        if spw_ids is None:
            spw_ids = [spw.id for spw in spws]
        if antenna_ids is None:
            antenna_ids = [antenna.id for antenna in self.ms.antennas]
        if antenna_names is None:
            antenna_names = [antenna.name for antenna in self.ms.antennas]

        plots = []
        for spw in spws:
            # TODO look at the caltable for antennas too?
            for antenna in self.ms.antennas:
                if spw.id in spw_ids and antenna.id in antenna_ids and antenna.name in antenna_names:
                    plots.append(self.get_plot_wrapper(spw, qa2_scans, antenna))

        return [p for p in plots if p is not None]
    
    def create_plot(self, spw, scans, helper):
        data_before = self._data_before
        data_after = self._data_after

        # check the spw given by the spw argument is present in both caltables
        assert spw.id in data_before.spw, 'Spw %s not in %s' % (spw, self._table_before)
        assert spw.id in data_after.spw, 'Spw %s not in %s' % (spw, self._table_after)
    
        # get the fields and scan intents from the list of scans. These are
        # used in the plot title, eg. NGC123 (PHASE)
        scan_fields = set()
        for scan in scans:
            scan_fields.update([field.name for field in scan.fields])
        scan_fields = ','.join(scan_fields)

        scan_intents = set()
        for scan in scans:
            scan_intents.update(scan.intents)
        scan_intents.remove('WVR')
        scan_intents = ','.join(scan_intents)
 
        num_scans = len(scans)

        # get the polarisations for the calibration scans, assuming that
        # all scans with this calibration intent were observed with the
        # same polarisation setup
        scan = scans[0]
        corr_axes = [tuple(dd.corr_axis) for dd in scan.data_descriptions
                     if dd.spw.id == spw.id]
        # discard WVR and other strange data descriptions 
        corr_axes = set([x for x in corr_axes if x not in [(), ('I',)]])
        assert len(corr_axes) is 1, ('Data descriptions have different '
                                     'corr axes for scan %s. Got %s'
                                     '' % (scan.id, corr_axes))
        # go from set(('XX', 'YY')) to the ('XX', 'YY')
        corr_axes = corr_axes.pop()
        
        autoscale_yaxis_range = [-200, 200]
    
        fig, axes = subplots(1, num_scans, sharey=True)
        for axis in axes:
            axis.xaxis.set_ticks_position('none')
            axis.yaxis.set_ticks_position('none')
            axis.spines['left'].set_linestyle('dotted')
            axis.spines['right'].set_visible(False)                
        axes[0].spines['left'].set_visible(True)
        axes[0].spines['left'].set_linestyle('solid')
        axes[0].yaxis.set_ticks_position('left')
        axes[0].yaxis.tick_left()
        axes[0].set_ylabel('Deviation from Scan Median Phase (degrees)' % scan.id, size=10)
        axes[-1].spines['right'].set_visible(True)
        axes[-1].yaxis.set_ticks_position('right')
        axes[-1].yaxis.tick_right()                 
        
        pyplot.subplots_adjust(wspace=0.0)
             
        for scan_idx, scan in enumerate(scans):
            for antenna in helper.get_antennas():    
                axis = axes[scan_idx]
                plots = []
                legends = []
                for state, all_data in [('BEFORE', data_before), 
                                        ('AFTER', data_after)]:
                    data = all_data.filter(scan=[scan.id], 
                                           antenna=[antenna.id], 
                                           spw=[spw.id])
                    for corr_idx, corr_axis in enumerate(corr_axes):
                        phase_for_corr = data.data[corr_idx]
                        rad_phase = numpy.deg2rad(phase_for_corr)
                        unwrapped_phase = numpy.unwrap(rad_phase)
                        offset_rad = unwrapped_phase - numpy.ma.median(unwrapped_phase)
                        # the operation above removed the mask, so add it back.
                        offset_rad = numpy.ma.MaskedArray((offset_rad), mask=phase_for_corr.mask)
                        offset_deg = numpy.rad2deg(offset_rad)

                        for masked_slice in numpy.ma.clump_masked(offset_deg):
                            self._plot_flagged_data(data, masked_slice, axis)
                        
                        (symbol, color, alpha) = self.get_symbol_and_colour(corr_axis, 
                                                                            state=state)
                        axis.plot_date(data.time, offset_deg, '.',
                                       color=color, alpha=alpha)
                        p, = axis.plot_date(data.time, offset_deg, symbol,
                                            color=color, alpha=alpha)

                        legend_entry = '%s %s' % (corr_axis, state.lower())
                        if legend_entry not in legends:
                            legends.append(legend_entry)
                            plots.append(p)
    
                        # shrink the x axis range by a couple of integrations
                        # so that the first/last scan symbols are not clipped 
#                         delta = numpy.mean(numpy.diff(data.time)) / 2
                        axis.set_xlim(data.time[0], data.time[-1])
                        axis.set_ylim(autoscale_yaxis_range)

                # for old versions of matplotlib (CASA/Linux), we can only edit
                # the axes after the data have been plotted, otherwise some 
                # settings do not take effect. 
                axis.set_xlabel('%s' % scan.id, size=8)
                try:
                    axis.tick_params(axis='x', labelbottom='off')
                    axis.tick_params(axis='y', labelsize=8, labelright='off')
                except:
                    # CASA on Linux comes with old version matplotlib
                    LOG.trace('matplotlib.axes.tick_params() missing. Using workaround.')

                    if axis == axes[0]:
                        matplotlib.pyplot.setp(axis.get_yticklabels(), fontsize=8)
                    if axis == axes[-1]:
                        # had real trouble disabling y axis labels for the last subplot
                        # best I can do is reset the ticks
                        axis.yaxis.set_ticks_position('default')

                    axis.xaxis.set_ticks([])                        
                    axis.xaxis.set_major_formatter(matplotlib.ticker.NullFormatter())
    
        # shrink the y height slightly to make room for the legend
        for axis in axes:
            box = axis.get_position()
            axis.set_position([box.x0, box.y0 + box.height * 0.03,
                               box.width, box.height * 0.97])


#         # sort legend and associated plots by legend text 
#         legends, plots = zip(*sorted(zip(legends, plots)))
 
        # CASA is using an old matplotlib, so we can't specify fontsize as
        # a property
        try:
            axes[-1].legend(plots, legends, prop={'size':10}, numpoints=1,
                            loc='upper center', bbox_to_anchor=(0.5, 0.07),
                            frameon=False, ncol=len(legends), 
                            bbox_transform = pyplot.gcf().transFigure)
        except TypeError:
            # old matplotlib doesn't expect frameon kwarg
            l = axes[-1].legend(plots, legends, prop={'size':10}, numpoints=1,
                                loc='upper center', bbox_to_anchor=(0.5, 0.07),
                                ncol=len(legends), 
                                bbox_transform = pyplot.gcf().transFigure)
            l.draw_frame(False)
        
        spw_msg = 'SPW %s Correlation%s' % (spw.id, 
                utils.commafy(corr_axes, quotes=False, multi_prefix='s'))
        pyplot.text(0.0, 1.013, spw_msg, color='k', 
                    transform=axes[0].transAxes, size=10)
        pyplot.text(0.5, 0.945, '%s (%s)' % (scan_fields, scan_intents), 
                    color='k', transform=fig.transFigure, ha='center', size=10)
        helper.label_antenna(fig)
        pyplot.text(0.5, 0.07, 'Scan', color='k', transform=fig.transFigure,
                    ha='center', size=10)
                
        scan_ids = [str(s.id) for s in scans]
        max_scans_for_msg = 8
        
        # print 'Scans 4, 8, 12 ... 146' if there are too many scans to 
        # print
        if num_scans > max_scans_for_msg:
            start = ','.join(scan_ids[0:max_scans_for_msg-1])
            end = scan_ids[-1]
            scan_txt = 's %s ... %s' % (start, end)
        else:
            scan_txt = utils.commafy(scan_ids, multi_prefix='s', 
                                     quotes=False, separator=',')
        pyplot.text(1.0, 1.013, 'Scan%s' % scan_txt, color='k', ha='right', 
                    transform=axes[-1].transAxes, size=10)
        
        figfile = self.get_figfile(spw, helper.antenna_filename_component)
        pyplot.savefig(figfile)
        pyplot.close()

    def get_figfile(self, spw, antenna):
        if not isinstance(antenna, types.StringType):
            antenna = 'ant%s' % antenna.id
        vis = os.path.basename(self.result.vis)
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            '%s.phase_offset.%s.spw%0.2d.png' % (vis, antenna, spw.id))

    def get_plot_wrapper(self, spw, scans, antenna):
        figfile = self.get_figfile(spw, antenna)
        wrapper = logger.Plot(figfile,
                              x_axis='scan',
                              y_axis='phase',
                              parameters={'spw' : spw.id,
                                          'ant' : antenna.name})
        wrapper.qa2_score = self._score_retriever.get_score(spw, antenna)
        
        if not os.path.exists(figfile):
            LOG.trace('WVR phase offset plot for antenna %s spw %s not found.'
                      ' Creating new plot: %s' % (antenna.name, spw.id, figfile))
            helper = WVRChart.WvrChartHelper(antenna)
            try:
                self.create_plot(spw, scans, helper)
            except Exception as ex:
                LOG.error('Could not create WVR phase offset plot for antenna'
                          ' %s spw %s' % (antenna.id, spw.id))
                LOG.exception(ex)
                return None

        # the plot may not be created if all data for that antenna are flagged
        if os.path.exists(figfile):
            return wrapper            
        return None
    
    def _plot_flagged_data(self, data, masked_slice, axis):
        """
        Plot flagged data.
        
        data -- the CaltableWrapper for the data selection
        masked_slice -- the Slice object defining the flagged extent
        axis -- the Axis object to be used for plotting
        """
        idx_start = max(masked_slice.start-1, 0)
        idx_stop = min(masked_slice.stop, len(data.time)-1)
        start = data.time[idx_start]
        stop = data.time[idx_stop]
        width = stop - start
        
        # the x coords of this transformation are data, and the
        # y coord are axes
        trans = matplotlib.transforms.blended_transform_factory(axis.transData,
                                                                axis.transAxes)

        # We want x to be in data coordinates and y to
        # span from 0..1 in axes coords
        rect = matplotlib.patches.Rectangle((start,0), width=width, height=1,
                                            transform=trans, color='#EEEEEE',
                                            alpha=0.2)
        
        axis.add_patch(rect)
        axis.text(start + width/2, 0.5, 'FLAGGED', color='k', transform=trans,
                  size=10, ha='center', va='center', rotation=90)                            
    

class WVRSummaryChart(WVRChart):
    class WvrChartHelper(object):
        def __init__(self, antennas):
            self._antennas = antennas

        def get_antennas(self):
            return self._antennas[:]
                        
        def label_antenna(self, fig):
            pyplot.text(0.5, 0.911, 'All Antennas', color='k', 
                        transform=fig.transFigure, ha='center', size=10)

        @property
        def antenna_filename_component(self):
            return ''
    
    def __init__(self, context, result):
        super(WVRSummaryChart, self).__init__(context, result)

    def get_figfile(self, spw, antennas):
        vis = os.path.basename(self.result.vis)
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            '%s.phase_offset_summary.spw%s.png' % (vis, spw.id))

    def plot(self):
        data_before = self._data_before
        data_after = self._data_after

        # get the windows this was tested on from the caltable.
        spw_ids = set(data_before.spw).intersection(set(data_after.spw))
        spws = [spw for spw in self.ms.spectral_windows if spw.id in spw_ids]

        qa2_scans = self._get_qa2_scans()

        plots = []
        for spw in spws:
            # TODO look at the caltable for antennas too?
            plots.append(self.get_plot_wrapper(spw, qa2_scans, 
                                               self.ms.antennas))

        return [p for p in plots if p is not None]

    def get_plot_wrapper(self, spw, scans, antenna):
        figfile = self.get_figfile(spw, antenna)
        wrapper = logger.Plot(figfile,
                              x_axis='scan',
                              y_axis='phase',
                              parameters={'spw' : spw.id})
        
        if not os.path.exists(figfile):
            LOG.trace('WVR summary plot for spw %s not found. Creating new '
                      'plot.' % spw.id)
            helper = WVRSummaryChart.WvrChartHelper(antenna)
            try:
                self.create_plot(spw, scans, helper)
            except Exception as ex:
                LOG.error('Could not create WVR phase offset summary plot for'
                          ' spw %s' % spw.id)
                LOG.exception(ex)
                return None

        # the plot may not be created if all data for that antenna are flagged
        if os.path.exists(figfile):
            return wrapper            
        return None

    def _plot_flagged_data(self, data, masked_slice, axis):
        """
        Plot flagged data.
        
        data -- the CaltableWrapper for the data selection
        masked_slice -- the Slice object defining the flagged extent
        axis -- the Axis object to be used for plotting
        """
        idx_start = max(masked_slice.start-1, 0)
        idx_stop = min(masked_slice.stop, len(data.time)-1)
        start = data.time[idx_start]
        stop = data.time[idx_stop]
        width = stop - start
        
        # the x coords of this transformation are data, and the
        # y coord are axes
        trans = matplotlib.transforms.blended_transform_factory(axis.transData,
                                                                axis.transAxes)

        # We want x to be in data coordinates and y to
        # span from 0..1 in axes coords
        rect = matplotlib.patches.Rectangle((start,0), width=width, height=1,
                                            transform=trans, color='#EEEEEE',
                                            alpha=0.2)        
        axis.add_patch(rect)


class WVRPhaseVsBaselineChart(WVRPlotBase):
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

    def get_symbol_and_colour(self, pol, state='BEFORE'):
        """
        Get the plot symbol and colour for this polarization and bandtype.
        """
        d = {'BEFORE' : {'L' : ('-', 'orange', 0.3),
                         'R' : ('--', 'sandybrown', 0.3),
                         'X' : ('^', 'lightslategray', 0.3),
                         'Y' : ('o', 'lightslategray', 0.3),
                         'XX' : ('^', 'lightslategray', 0.3),
                         'YY' : ('o', 'lightslategray', 0.3)},
             'AFTER' : {'L' : ('-', 'green', 0.6),
                        'R' : ('-', 'red', 0.6),
                        'X' : ('^', 'green', 0.6),
                        'Y' : ('o', 'red', 0.6),
                        'XX' : ('^', 'green', 0.6),
                        'YY' : ('o', 'red', 0.6)}}
    
        return d.get(state, {}).get(pol, ('x', 'grey'))


    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self._caltables_loaded = False

        nowvr_gaintables = set([c.gaintable for c in result.nowvr_result.final])
        assert len(nowvr_gaintables) is 1, ('Unexpected number of pre-WVR phase-up'
                                            'gaintables: %s' % nowvr_gaintables) 

        nowvr_gaintable = nowvr_gaintables.pop()
        wvr_gaintable = result.qa2.gaintable_wvr
        LOG.debug('Gaintables for WVR plots:\n'
                  'No WVR: %s\tWith WVR: %s' % (nowvr_gaintable, wvr_gaintable))    

        self._table_before = nowvr_gaintable
        self._table_after = wvr_gaintable
        
        self._score_retriever = WVRScoreFinder(result)

        self._wrappers = []

    def plot(self):
        self._load_caltables()
        data_before = self._data_before
        data_after = self._data_after

        # get the windows this was tested on from the caltable.
        spw_ids = set(data_before.spw).intersection(set(data_after.spw))
        spws = [spw for spw in self.ms.spectral_windows if spw.id in spw_ids]

        qa2_scans = self._get_qa2_scans()

        # phase offsets are plotted per corr, spw and scan. We cannot index
        # the phase arrays with multiple corr/spw/scans as the unwrapped
        # phases would be for the whole data, not for the corr/spw/scan 
        # combination we want to plot
        LOG.debug('Finding maximum phase offset over all scans/spws/corrs/antennas')
        for scan in qa2_scans:
            for spw in spws:
                # find the data description for this scan. Just one dd expected.
                dd = [dd for dd in scan.data_descriptions 
                      if dd.spw.id == spw.id][0]
                # we expect the number and identity of the caltable 
                # correlations for this scan to match those in the MS, so we
                # can enumerate over the correlations in the MS scan.  
                for corr_id, _ in enumerate(dd.corr_axis):
                    for antenna in self.ms.antennas:
                        selection_before = data_before.filter(scan=[scan.id], 
                                                              antenna=[antenna.id], 
                                                              spw=[spw.id])
                        baseline_data_before = PhaseVsBaselineData(selection_before,
                                                                   self.ms,
                                                                   corr_id)
                        
                        selection_after = data_after.filter(scan=[scan.id], 
                                                            antenna=[antenna.id], 
                                                            spw=[spw.id])
                        baseline_data_after = PhaseVsBaselineData(selection_after,
                                                                  self.ms,
                                                                  corr_id)
                        
                        wrapper = DataRatio(baseline_data_before, 
                                            baseline_data_after)
                        self._wrappers.append(wrapper)
                        
        offsets = [w.before.median_offset for w in self._wrappers]
        offsets.extend([w.after.median_offset for w in self._wrappers])
        # offsets could contain None where data was flagged, but that's ok as 
        # max ignores it.
        self._max_phase_offset = max(offsets)
        LOG.trace('Maximum phase offset for %s = %s' % (self.ms.basename, 
                                                        self._max_phase_offset))

        ratios = [w.ratio for w in self._wrappers]
        self._max_ratio = max(ratios)
        LOG.trace('Maximum phase ratio for %s = %s' % (self.ms.basename, 
                                                       self._max_ratio))

        plots = []
        for spw in spws:
            # plot scans individually as plotting multiple scans on one plot 
            # creates an unintelligible mess. 
            for scan in qa2_scans:
#                 if spw.id == 9 and scan.id == 4:
                    plots.append(self.get_plot_wrapper(spw, [scan,], 
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
            
        # get the polarisations for the calibration scans, assuming that
        # all scans with this calibration intent were observed with the
        # same polarisation setup
        corr_axes = [tuple(dd.corr_axis) for dd in scan.data_descriptions
                     if dd.spw.id == spw.id]
        # discard WVR and other strange data descriptions 
        corr_axes = set([x for x in corr_axes if x not in [(), ('I',)]])
        assert len(corr_axes) is 1, ('Data descriptions have different '
                                     'corr axes for scan %s. Got %s'
                                     '' % (scan.id, corr_axes))
        # go from set(('XX', 'YY')) to the ('XX', 'YY')
        corr_axes = corr_axes.pop()

        # create the figure: 2 rows x 1 column, sharing the X axis (baseline 
        # length) 
        fig, ((ax1, ax2)) = subplots(2, 1, sharex=True)
        pyplot.subplots_adjust(hspace=0.0)        

        # create bottom plot: phase offset vs baseline
        legend = []
        plots = []
        for _, scan in enumerate(scans):
            for corr_idx, corr_axis in enumerate(corr_axes):
                wrappers = [w for w in self._wrappers
                            if scan.id in w.scans 
                            and spw.id in w.spws
                            and corr_idx in w.corr]
    
                unflagged_wrappers = [w for w in wrappers if w.ratio is not None]
    
                # upper plot: ratio improvement
                x = [float(w.mean_baseline) for w in unflagged_wrappers]
                y = [w.ratio for w in unflagged_wrappers]
                (symbol, color, alpha) = self.get_symbol_and_colour(corr_axis, state='AFTER')
                _, = ax1.plot(x, y, symbol, color=color, alpha=alpha)
    
                # lower plot: abs(median offset from median phase)
                x = [float(w.mean_baseline) for w in unflagged_wrappers]
                y = [w.before.median_offset for w in unflagged_wrappers]
                (symbol, color, alpha) = self.get_symbol_and_colour(corr_axis, state='BEFORE')                    
                p, = ax2.plot(x, y, symbol, color=color, alpha=alpha)
                plots.append(p)
                legend.append('%s %s' % (corr_axis, 'before'))

                y = [w.after.median_offset for w in unflagged_wrappers]
                (symbol, color, alpha) = self.get_symbol_and_colour(corr_axis, state='AFTER')                    
                p, = ax2.plot(x, y, symbol, color=color, alpha=alpha)
                plots.append(p)
                legend.append('%s %s' % (corr_axis, 'after'))
        
        max_baseline = max([b.length for b in self.ms.antenna_array.baselines])
        max_baseline_m = float(max_baseline.to_units(measures.DistanceUnits.METRE))
        ax1.set_xlim(0, max_baseline_m)
        ax1.set_ylim(0, self._max_ratio)
        ax2.set_ylim(0, self._max_phase_offset)
    
        # shrink the y height slightly to make room for the legend
        box1 = ax1.get_position()
        ax1.set_position([box1.x0, box1.y0 + box1.height * 0.06,
                          box1.width, box1.height * 0.94])
        box2 = ax2.get_position()
        ax2.set_position([box2.x0, box2.y0 + box2.height * 0.06,
                          box2.width, box2.height])

        ax1.set_ylabel('ratio', size=10)
        ax2.set_xlabel('Mean Baseline (m)', size=10)
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
            ax2.legend(plots, legend, prop={'size':10}, numpoints=1,
                       loc='upper center', bbox_to_anchor=(0.5, -0.12),
                       frameon=False, ncol=len(legend))
        except TypeError:
            # old matplotlib doesn't expect frameon either
            l = ax2.legend(plots, legend, prop={'size':10}, numpoints=1,
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
        pyplot.text(0.012, 0.97, 'Phase RMS with WVR / Phase RMS without WVR', 
                    color='k', transform=ax1.transAxes, ha='left', va='top', 
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
                              parameters={'spw'  : spw.id,
                                          'scan' : scan_ids})
        
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
                return None

        # the plot may not be created if all data for that antenna are flagged
        if os.path.exists(figfile):
            return wrapper            
        return None
    
    
    
    
    
    
    
    
# linux matplotlib is stuck on v0.99.3, which is missing the subplots function    
def subplots(nrows=1, ncols=1, sharex=False, sharey=False, squeeze=True,
                subplot_kw=None, **fig_kw):
    """Create a figure with a set of subplots already made.

    This utility wrapper makes it convenient to create common layouts of
    subplots, including the enclosing figure object, in a single call.

    Keyword arguments:

    nrows : int
      Number of rows of the subplot grid.  Defaults to 1.

    ncols : int
      Number of columns of the subplot grid.  Defaults to 1.

    sharex : bool
      If True, the X axis will be shared amongst all subplots.  If
      True and you have multiple rows, the x tick labels on all but
      the last row of plots will have visible set to False

    sharey : bool
      If True, the Y axis will be shared amongst all subplots. If
      True and you have multiple columns, the y tick labels on all but
      the first column of plots will have visible set to False

    squeeze : bool
      If True, extra dimensions are squeezed out from the returned axis object:

        - if only one subplot is constructed (nrows=ncols=1), the
          resulting single Axis object is returned as a scalar.

        - for Nx1 or 1xN subplots, the returned object is a 1-d numpy
          object array of Axis objects are returned as numpy 1-d
          arrays.

        - for NxM subplots with N>1 and M>1 are returned as a 2d
          array.

      If False, no squeezing at all is done: the returned axis object is always
      a 2-d array contaning Axis instances, even if it ends up being 1x1.

    subplot_kw : dict
      Dict with keywords passed to the add_subplot() call used to create each
      subplots.

    fig_kw : dict
      Dict with keywords passed to the figure() call.  Note that all keywords
      not recognized above will be automatically included here.

    Returns:

    fig, ax : tuple

      - fig is the Matplotlib Figure object

      - ax can be either a single axis object or an array of axis
        objects if more than one supblot was created.  The dimensions
        of the resulting array can be controlled with the squeeze
        keyword, see above.

    **Examples:**

    x = np.linspace(0, 2*np.pi, 400)
    y = np.sin(x**2)

    # Just a figure and one subplot
    f, ax = plt.subplots()
    ax.plot(x, y)
    ax.set_title('Simple plot')

    # Two subplots, unpack the output array immediately
    f, (ax1, ax2) = plt.subplots(1, 2, sharey=True)
    ax1.plot(x, y)
    ax1.set_title('Sharing Y axis')
    ax2.scatter(x, y)

    # Four polar axes
    plt.subplots(2, 2, subplot_kw=dict(polar=True))
    """

    if subplot_kw is None:
        subplot_kw = {}

    fig = pyplot.figure(**fig_kw)

    # Create empty object array to hold all axes.  It's easiest to make it 1-d
    # so we can just append subplots upon creation, and then
    nplots = nrows*ncols
    axarr = numpy.empty(nplots, dtype=object)

    # Create first subplot separately, so we can share it if requested
    ax0 = fig.add_subplot(nrows, ncols, 1, **subplot_kw)
    if sharex:
        subplot_kw['sharex'] = ax0
    if sharey:
        subplot_kw['sharey'] = ax0
    axarr[0] = ax0

    # Note off-by-one counting because add_subplot uses the MATLAB 1-based
    # convention.
    for i in range(1, nplots):
        axarr[i] = fig.add_subplot(nrows, ncols, i+1, **subplot_kw)



    # returned axis array will be always 2-d, even if nrows=ncols=1
    axarr = axarr.reshape(nrows, ncols)


    # turn off redundant tick labeling
    if sharex and nrows>1:
        # turn off all but the bottom row
        for ax in axarr[:-1,:].flat:
            for label in ax.get_xticklabels():
                label.set_visible(False)


    if sharey and ncols>1:
        # turn off all but the first column
        for ax in axarr[:,1:].flat:
            for label in ax.get_yticklabels():
                label.set_visible(False)

    if squeeze:
        # Reshape the array to have the final desired dimension (nrow,ncol),
        # though discarding unneeded dimensions that equal 1.  If we only have
        # one subplot, just return it instead of a 1-element array.
        if nplots==1:
            ret = fig, axarr[0,0]
        else:
            ret = fig, axarr.squeeze()
    else:
        # returned axis array will be always 2-d, even if nrows=ncols=1
        ret = fig, axarr.reshape(nrows, ncols)

    return ret
