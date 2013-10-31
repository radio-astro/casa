from __future__ import absolute_import
import re
import os

import matplotlib
import matplotlib.pyplot as pyplot
import matplotlib.ticker as ticker

import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.utils as utils
from pipeline.infrastructure import casa_tasks
from . import wvr

LOG = infrastructure.get_logger(__name__)


class NullScoreFinder(object):
    def get_score(self, spw, antenna):
        return None


class BandpassPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.caltable = result.final[0].gaintable

    def plot(self):
        wrapper = wvr.CaltableWrapper.from_caltable(self.caltable)
        caltable_antennas = [int(i) for i in set(wrapper.antenna)]
        domain_antennas = [a for a in self.ms.antennas if a.id in caltable_antennas]

        plots = []
        for antenna in domain_antennas:
            plots.append(self.get_plot_wrapper(antenna))
        
        return [p for p in plots if p is not None]

    def create_plot(self, antenna):
        figfile = self.get_figfile()

        task_args = {'vis'         : self.ms.name,
                     'caltable'    : self.caltable,
                     'xaxis'       : 'freq',
                     'yaxis'       : 'amp',
#                      'overlay'     : 'spw',                     
                     'interactive' : False,
                     'antenna'     : antenna.id,
                     'showatm'     : True,
                     'subplot'     : 11,
                     'figfile'     : figfile}

        task = casa_tasks.plotbandpass(**task_args)
        task.execute(dry_run=False)

    def get_figfile(self):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            '%s-bandpass.png' % self.ms.basename)

    def get_plot_wrapper(self, antenna):
        figfile = self.get_figfile()

        # plotbandpass injects antenna name, spw ID and t0 into every plot filename
        root, ext = os.path.splitext(figfile)
        real_figfile = '%s.%s.t0%s' % (root, antenna.name, ext)
        
        wrapper = logger.Plot(real_figfile,
                              x_axis='freq',
                              y_axis='amp',
                              parameters={'vis' : self.ms.basename,
                                          'ant' : antenna.name})
        
        if not os.path.exists(real_figfile):
            LOG.trace('Tsys per antenna plot for antenna %s not found. '
                      'Creating new plot.' % antenna.name)
            try:
                self.create_plot(antenna)
            except Exception as ex:
                LOG.error('Could not create Tsys plot for antenna %s.'
                          '' % antenna.name)
                LOG.exception(ex)
                return None

        # the Tsys plot may not be created if all data for that antenna are
        # flagged
        if os.path.exists(real_figfile):
            return wrapper            
        return None
    

class BandpassSummaryChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.caltable = result.final[0].gaintable

    def plot(self):
        wrapper = wvr.CaltableWrapper.from_caltable(self.caltable)
        caltable_spws = ','.join([str(spw) for spw in set(wrapper.spw)])
        domain_spws = self.ms.get_spectral_windows(caltable_spws)
        plots = [self.get_plot_wrapper(spw) for spw in domain_spws]
        return [p for p in plots if p is not None]

    def create_plot(self, spw):
        figfile = self.get_figfile()

        task_args = {'vis'         : self.ms.name,
                     'caltable'    : self.caltable,
                     'xaxis'       : 'freq',
                     'yaxis'       : 'amp',
                     'overlay'     : 'antenna',
                     'interactive' : False,
                     'spw'         : spw.id,
                     'showatm'     : True,
                     'subplot'     : 11,
                     'figfile'     : figfile}

        task = casa_tasks.plotbandpass(**task_args)
        task.execute(dry_run=False)

    def get_figfile(self):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            '%s-bandpass-summary.png' % self.ms.basename)

    def get_plot_wrapper(self, spw):
        figfile = self.get_figfile()

        # plotbandpass injects spw ID and t0 into every plot filename
        root, ext = os.path.splitext(figfile)
        real_figfile = '%s.spw%0.2d.t0%s' % (root, spw.id, ext)
        
        wrapper = logger.Plot(real_figfile,
                              x_axis='freq',
                              y_axis='tsys',
                              parameters={'vis'      : self.ms.basename,
                                          'spw'      : spw.id,
                                          'tsys_spw' : spw})

        if not os.path.exists(real_figfile):
            LOG.trace('Summary plot for spw %s not found. Creating new '
                      'plot.' % spw.id)
            try:
                self.create_plot(spw)
            except Exception as ex:
                LOG.error('Could not create summary plot for spw %s'
                          '' % spw.id)
                LOG.exception(ex)
                return None
            
        return wrapper
  
        

class BandpassDisplayInputs(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result

        
class BandpassDisplay(object):
    Inputs = BandpassDisplayInputs

    def __init__(self, inputs):
        self.inputs = inputs

    def plot(self):
        inputs = self.inputs
        if not inputs.result.final:
            return []

        result = inputs.result
        context = inputs.context
        report_dir = context.report_dir

        for calapp in result.final:
            ms = context.observing_run.get_ms(calapp.vis)
            spws = ms.get_spectral_windows(calapp.spw)
            antennas = ms.get_antenna(calapp.antenna)
            caltable = calapp.gaintable
        
            outdir = os.path.join(report_dir,
                                  'stage%s' % result.stage_number)
        
            match = re.search('(?<=\.solint)\w+', caltable)
            solint = match.group(0).replace('_','.') if match else None
            
            plots = []        
            for spw in spws:
                dd = ms.get_data_description(spw=spw)
    
                for y_axis in ('amp', 'phase'):
                    for antenna in antennas:
                        basename = os.path.basename(caltable)
                        (root, _) = os.path.splitext(basename)
                        png = ('{root}.{y_axis}-v-freq.ant{antenna_id}.spw{spw}'
                               '.png'.format(root=root,
                                             y_axis=y_axis, 
                                             antenna_id=antenna.id,
                                             spw=spw.id))
                        filename = os.path.join(outdir, png)

                        plot = logger.Plot(filename,
                                           x_axis='freq',
                                           y_axis=y_axis,
                                           field='Unknown source',
                                           parameters={ 'ant'    : antenna.id,
                                                        'solint' : solint,  
                                                        'spw'    : spw.id      })
                        plots.append(plot)
                        
                        if os.path.exists(filename):
                            continue
                        
                        pyplot.figure()                    
    
                        self.overplot = False
                        self.y_limits = None
                        self.y_offset = 0.25
                        
#                        for t in [caltable for caltable in basis 
#                                  if caltable.spw == str(spw.id)]:
#                            self._plot_data(t, dd, antenna, y_axis)
#                        for t in [caltable for caltable in caltables
#                                  if caltable.spw == str(spw.id)]:
#                            self._plot_data(t, dd, antenna, y_axis)

                        self._plot_data(caltable, dd, antenna, y_axis)
                        
                        if self.y_limits:
                            pyplot.ylim(self.y_limits)
                            
                        title = ('Bandpass Calibration: '
                                 'Antenna {0}, SpW {1}'.format(antenna.name, 
                                                               spw.id))
                        pyplot.title(title)                    
                        pyplot.savefig(filename=filename)
                        pyplot.clf()
                        pyplot.close()

        return plots
        
    def _get_symbol_and_colour(self, pol, bandtype='B'):
        """
        Get the plot symbol and colour for this polarization and bandtype.
        """
        d = {'B'     : {'L' : ('3', 'orange'),
                        'R' : ('4', 'sandybrown'),
                        'X' : ('2', 'lime'),
                        'Y' : ('1', 'olive')},
             'BPOLY' : {'L' : ('<', 'lightsteelblue'),
                        'R' : ('>', 'steelblue'),
                        'X' : ('^', 'lightslategray'),
                        'Y' : ('v', 'slategray')}}

        return d.get(bandtype, {}).get(pol, ('x', 'grey'))
     
    def _plot_data(self, caltable, dd, antenna, y_axis):
        for pol in dd.polarizations:
#            bandtype = caltable.job.kw.get('bandtype', 'B')
            bandtype = 'B'            
            (plotsymbol, plotcolor) = self._get_symbol_and_colour(pol, 
                                                                  bandtype)
        
            plotcal_args = { 'caltable'   : caltable,
                             'antenna'    : str(antenna.id),
                             'spw'        : str(dd.spw.id),
                             'poln'       : pol,
                             'xaxis'      : 'freq',
                             'yaxis'      : y_axis,
                             'overplot'   : self.overplot,
                             'showflags'  : False,
                             'plotsymbol' : plotsymbol,
                             'plotcolor'  : plotcolor,
                             'markersize' : 5.0, 
                             'fontsize'   : 15,
                             'showgui'    : False                     }
    
            casa_tasks.plotcal(**plotcal_args).execute()
            self.overplot = True

            # channel plots can have crazy y limits due to poor fitting at the
            # bandpass edges, so make a memo of reasonable limits set by
            # channel plots
            if bandtype is 'B':
                self.y_limits = pyplot.ylim()

            self.y_offset = self._plot_key(plotcal_args, self.y_offset, 
                                           bandtype)

        # stop ticks being marked relative to an offset which can be confusing
        xaxis = pyplot.gca().xaxis
        xaxis.set_major_formatter(ticker.ScalarFormatter(useOffset=False))
        
    def _plot_key(self, plotcal_args, y_offset, bandtype='B'):
        # pyplot calls seem to screw up x-axis limits so read them here and
        # restore them later
        x_limits = pyplot.xlim()
            
        plot_args = { 'linestyle'  : 'None',
                      'marker'     : plotcal_args['plotsymbol'],
                      'markersize' : 1.5 * plotcal_args['markersize'],
                      'color'      : plotcal_args['plotcolor'],
                      'transform'  : pyplot.gca().transAxes             }
        pyplot.plot([0.42], [y_offset+0.01], **plot_args)

        bandtype = 'channel' if bandtype is 'B' else 'polynomial'

        text = "{0} {1} result".format(plotcal_args['poln'], bandtype)
        text_args = { 'fontsize'  : 10.0,
                      'transform' : pyplot.gca().transAxes }
        pyplot.text(0.45, y_offset, text, **text_args)

        if x_limits:
            pyplot.xlim(x_limits)
        
        return y_offset - 0.045








class PlotBase(object):    
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
        data_before = wvr.CaltableWrapper.from_caltable(self._table_before)
        data_after = wvr.CaltableWrapper.from_caltable(self._table_after)

        # some sanity checks, as unequal caltables have bit me before
        if self._table_before:
            assert utils.areEqual(data_before.time, data_after.time), 'Time columns are not equal'
            assert utils.areEqual(data_before.antenna, data_after.antenna), 'Antenna columns are not equal'
            assert utils.areEqual(data_before.spw, data_after.spw), 'Spw columns are not equal'
            assert utils.areEqual(data_before.scan, data_after.scan), 'Scan columns are not equal'

        self._data_before = data_before
        self._data_after = data_after
        self._caltables_loaded = True


class PhaseVsBaselineChart(PlotBase):
    # override base 
    def _get_plot_intents(self):
        return set(self.result.inputs['intent'].split(','))
    
    def _get_scans(self):
        intents = self._get_plot_intents()
        return [scan for scan in self.ms.scans 
                if not intents.isdisjoint(scan.intents)]

    
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

        # get the phase-up caltable name from the preceding CalApplication
        phaseup_gain = result.preceding[0].gaintable

        # use the same table for before and after values. We won't plot the
        # ratio anyway, though we will needlessly read the same caltable.
        self._table_before = phaseup_gain
        self._table_after = phaseup_gain
        
        self._score_retriever = NullScoreFinder(result)

        self._wrappers = []

    def plot(self):
        self._load_caltables()
        data_before = self._data_before
        data_after = self._data_after

        # get the windows this was tested on from the caltable.
        spw_ids = set(data_before.spw).intersection(set(data_after.spw))
        spws = [spw for spw in self.ms.spectral_windows if spw.id in spw_ids]

        scans = self._get_scans()

        # phase offsets are plotted per corr, spw and scan. We cannot index
        # the phase arrays with multiple corr/spw/scans as the unwrapped
        # phases would be for the whole data, not for the corr/spw/scan 
        # combination we want to plot
        LOG.debug('Finding maximum phase offset over all scans/spws/corrs/antennas')
        for scan in scans:
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
                        baseline_data_before = wvr.PhaseVsBaselineData(selection_before,
                                                                       self.ms,
                                                                       corr_id)
                        
                        selection_after = data_after.filter(scan=[scan.id], 
                                                            antenna=[antenna.id], 
                                                            spw=[spw.id])
                        baseline_data_after = wvr.PhaseVsBaselineData(selection_after,
                                                                      self.ms,
                                                                      corr_id)
                        
                        wrapper = wvr.DataRatio(baseline_data_before, 
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
            for scan in scans:
                    plots.append(self.get_plot_wrapper(spw, [scan,], 
                                                       self.ms.antennas))
#                     return [p for p in plots if p is not None]
                    

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

        # create the figure: 1 rows x 1 column
        fig, ((ax)) = wvr.subplots(1, 1, sharex=True)

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
    
                # plot: abs(median offset from median phase)
                x = [float(w.mean_baseline) for w in unflagged_wrappers]
                y = [w.after.median_offset for w in unflagged_wrappers]
                (symbol, color, alpha) = self.get_symbol_and_colour(corr_axis, state='AFTER')                    
                p, = ax.plot(x, y, symbol, color=color, alpha=alpha)
                plots.append(p)
                legend.append('%s' % corr_axis)
        
        max_baseline = max([b.length for b in self.ms.antenna_array.baselines])
        max_baseline_m = float(max_baseline.to_units(measures.DistanceUnits.METRE))
        ax.set_xlim(0, max_baseline_m)
        ax.set_ylim(0, self._max_phase_offset)
    
        # shrink the y height slightly to make room for the legend
        box2 = ax.get_position()
        ax.set_position([box2.x0, box2.y0 + box2.height * 0.02,
                         box2.width, box2.height])

        ax.set_xlabel('Mean Baseline (m)', size=10)
        ax.set_ylabel('degrees', size=10)

        try:
            ax.tick_params(labelsize=8, labelright='off')
        except:
            # CASA on Linux comes with old version of Matplotlib
            matplotlib.pyplot.setp(ax.get_xticklabels(), fontsize=8)
            matplotlib.pyplot.setp(ax.get_yticklabels(), fontsize=8)

        # CASA is using an old matplotlib, so we can't specify fontsize as
        # a property
        try:
            ax.legend(plots, legend, prop={'size':10}, numpoints=1,
                       loc='upper center', bbox_to_anchor=(0.5, -0.07),
                       frameon=False, ncol=len(legend))
        except TypeError:
            # old matplotlib doesn't expect frameon either
            l = ax.legend(plots, legend, prop={'size':10}, numpoints=1,
                           loc='upper center', bbox_to_anchor=(0.5, -0.07),
                           ncol=len(legend))
            l.draw_frame(False)
         
        spw_msg = 'SPW %s Correlation%s' % (spw.id, 
                utils.commafy(corr_axes, quotes=False, multi_prefix='s'))
        pyplot.text(0.0, 1.026, spw_msg, color='k', 
                    transform=ax.transAxes, size=10)
        pyplot.text(0.5, 1.110, '%s (%s)' % (scan_fields, scan_intents), 
                    color='k', transform=ax.transAxes, ha='center', size=10)
        pyplot.text(0.5, 1.026, 'All Antennas', color='k', 
                    transform=ax.transAxes, ha='center', size=10)

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
                    transform=ax.transAxes, size=10)

        pyplot.text(0.012, 0.97, 'Median Absolute Deviation from Median Phase', 
                    color='k', transform=ax.transAxes, ha='left', va='top', 
                    size=10)

        figfile = self.get_figfile(spw, scans, helper.antenna_filename_component)
        pyplot.savefig(figfile)
        pyplot.close()

    def get_figfile(self, spw, scans, antennas):
        vis = os.path.basename(self.result.inputs['vis'])
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
            helper = PhaseVsBaselineChart.WvrChartHelper(antenna)
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
    