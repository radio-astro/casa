from __future__ import absolute_import
import os

import matplotlib
import matplotlib.pyplot as pyplot
import pipeline.extern.ordereddict as ordereddict

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.utils as utils
from pipeline.infrastructure import casa_tasks
from . import common
from . import phaseoffset

LOG = infrastructure.get_logger(__name__)


class GaincalPerAntennaChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self.caltable = result.final[0].gaintable

    def plot(self):
        wrapper = common.CaltableWrapper.from_caltable(self.caltable)
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
                     'overlay'     : 'antenna',                     
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
    

class PhaseVsBaselineChart(common.PlotBase):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        self._caltables_loaded = False

        phaseup_gain = result.final[0].gaintable

        self._load_caltables(phaseup_gain)
        
        self._score_retriever = common.NullScoreFinder()

        refant_name = result.inputs['refant'].split(',')[0]
        self.refant = self.ms.get_antenna(refant_name)[0]

    def get_scans(self):
        intents = self.get_plot_intents()
        return [scan for scan in self.ms.scans 
                if not intents.isdisjoint(scan.intents)]
    
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


    def get_xlim(self, points):
        data = [p.x for p in points]
        return [0, max(data)]

    def get_ylim(self, points):
        data = [p.y for p in points]
        # data could contain None where data was flagged, but that's ok as 
        # max ignores it.
        return [0, max(data)]

    def plot(self):
        data_before = self._data_before
        data_after = self._data_after

        # get the windows this was tested on from the caltable.
        spw_ids = set(data_before.spw).intersection(set(data_after.spw))
        spws = [spw for spw in self.ms.spectral_windows if spw.id in spw_ids]

        scans = self.get_scans()

        # phase offsets are plotted per corr, spw and scan. We cannot index
        # the phase arrays with multiple corr/spw/scans as the unwrapped
        # phases would be for the whole data, not for the corr/spw/scan 
        # combination we want to plot
        LOG.debug('Finding data ranges for all spws/corrs/antennas')
        wrappers = []
        for spw in spws:
            # find the data description for this scan. Just one dd expected.
            dd = [dd for dd in scans[0].data_descriptions 
                  if dd.spw.id == spw.id][0]
            # we expect the number and identity of the caltable 
            # correlations for this scan to match those in the MS, so we
            # can enumerate over the correlations in the MS scan.  
            for corr_id, _ in enumerate(dd.corr_axis):
                for antenna in self.ms.antennas:
                    selection = data_before.filter(antenna=[antenna.id], spw=[spw.id])
                    try:
                        wrapper = self.get_data_object(selection, corr_id)
                    except ValueError:
                        continue
                    
                    wrappers.append(wrapper)
                     
        xlim = self.get_xlim(wrappers)
        ylim = self.get_ylim(wrappers)                        
        LOG.trace('X range for %s = %s' % (self.ms.basename, xlim))
        LOG.trace('Y range for %s = %s' % (self.ms.basename, ylim))

        plots = []
        for spw in spws:
            plots.append(self.get_plot_wrapper(spw, scans, self.ms.antennas,
                                               xlim, ylim, wrappers))
#                     return [p for p in plots if p is not None]

        return [p for p in plots if p is not None]

    def create_plot(self, spw, scans, helper, xlim, ylim, data):
#         
#         # check the spw given by the spw argument is present in both caltables
#         
#         assert spw.id in data_before.spw, 'Spw %s not in %s' % (spw, self._table_before)
#         assert spw.id in data_after.spw, 'Spw %s not in %s' % (spw, self._table_after)
    
        # get the scan intents from the list of scans
        scan_intents = set()
        for scan in scans:
            scan_intents.update(scan.intents)
        scan_intents.discard('WVR')
        scan_intents = ','.join(scan_intents)
 
        # get the fields from the list of scans
        scan_fields = set()
        for scan in scans:
            scan_fields.update([field.name for field in scan.fields])
        scan_fields = ','.join(scan_fields)
            
        # get the polarisations for the calibration scans, assuming that
        # all scans with this calibration intent were observed with the
        # same polarisation setup
        corr_axes = set()
        for scan in scans:
            scan_corrs = [tuple(dd.corr_axis) for dd in scan.data_descriptions
                              if dd.spw.id == spw.id]
            # discard WVR and other strange data descriptions 
            scan_corrs = set([x for x in scan_corrs 
                              if x not in [(), ('I',)]])
            corr_axes.update(scan_corrs)

        assert len(corr_axes) is 1, ('Data descriptions have different corr '
                                     'axes. Got %s' % corr_axes)
        # go from set(('XX', 'YY')) to the ('XX', 'YY')
        corr_axes = corr_axes.pop()

        # create the figure: 1 rows x 1 column
        fig, ((ax)) = common.subplots(1, 1, sharex=True)

        # create bottom plot: phase offset vs baseline
        legend = []
        plots = []
        for corr_idx, corr_axis in enumerate(corr_axes):
            wrappers = [w for w in data
                        if spw.id == w.spw
                        and corr_idx == w.corr]

            unflagged_wrappers = [w for w in wrappers if w.y is not None]

            # plot: abs(median offset from median phase)
            x = [float(w.x) for w in unflagged_wrappers]
            y = [w.y for w in unflagged_wrappers]
            (symbol, color, alpha) = self.get_symbol_and_colour(corr_axis, state='AFTER')                    
            p, = ax.plot(x, y, symbol, color=color, alpha=alpha)
            plots.append(p)
            legend.append('%s' % corr_axis)
        
        ax.set_xlim(xlim)
        ax.set_ylim(ylim)
    
        # shrink the y height slightly to make room for the legend
        box2 = ax.get_position()
        ax.set_position([box2.x0, box2.y0 + box2.height * 0.02,
                         box2.width, box2.height])

        ax.set_xlabel(self.get_xlabel(), size=10)
        ax.set_ylabel(self.get_ylabel(), size=10)

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
         
        # print 'Scans 4, 8, 12 ... 146' if there are too many scans to list
        if len(scans) > max_scans_for_msg:
            start = ','.join(scan_ids[0:max_scans_for_msg-1])
            end = scan_ids[-1]
            scan_txt = 's %s ... %s' % (start, end)
        else:
            scan_txt = utils.commafy(scan_ids, multi_prefix='s', 
                                     quotes=False, separator=',')
        pyplot.text(1.0, 1.026, 'Scan%s' % scan_txt, color='k', ha='right', 
                    transform=ax.transAxes, size=10)

        pyplot.text(0.012, 0.97, self.get_title(), 
                    color='k', transform=ax.transAxes, ha='left', va='top', 
                    size=10)

        figfile = self.get_figfile(spw, scans, helper.antenna_filename_component)
        pyplot.savefig(figfile)
        pyplot.close()

    def get_plot_wrapper(self, spw, scans, antenna, xlim, ylim, wrappers):
        wrapper = self.get_plot_object(spw, scans, antenna)
        figfile = wrapper.abspath
        scan_ids = utils.commafy([s.id for s in scans], quotes=False, multi_prefix='s')

        if not os.path.exists(figfile):
            LOG.trace('Plot for spw %s scan%s not found. Creating new '
                      'plot: %s' % (spw.id, scan_ids, figfile))
            helper = self.ChartHelper(antenna)
            try:
                self.create_plot(spw, scans, helper, xlim, ylim, wrappers)
            except Exception as ex:
                LOG.error('Could not create WVR phase vs baseline plot for'
                          ' spw %s scan %s' % (spw.id, scan_ids))
                LOG.exception(ex)
                return None

        # the plot may not be created if all data for that antenna are flagged
        if os.path.exists(figfile):
            return wrapper            
        return None
    
    def get_figfile(self, spw, scans, antennas):
        raise NotImplementedError()

    def get_plot_object(self, spw, scans, antennas):
        raise NotImplementedError()

    def get_plot_intents(self):
        raise NotImplementedError()
    
    def get_xlabel(self):
        raise NotImplementedError()
    
    def get_ylabel(self):
        raise NotImplementedError()
            
    def get_title(self):
        raise NotImplementedError()

    def get_data_object(self, data, corr_id):
        raise NotImplementedError()


class RMSOffsetVsRefAntDistanceChart(PhaseVsBaselineChart):
    class ChartHelper(object):
        def __init__(self, antennas):
            self._antennas = antennas

        def get_antennas(self):
            return self._antennas[:]
                        
        def label_antenna(self, axes):
            pyplot.title('All Antennas', size=10)

        @property
        def antenna_filename_component(self):
            return ''

    def get_plot_intents(self):
        return set(['PHASE'])
    
    def get_xlabel(self):
        return 'Distance to Reference Antenna (m)'
    
    def get_ylabel(self):
        return 'Unwrapped Phase RMS (degrees)'
            
    def get_title(self):
        return ''                

    def get_data_object(self, data, corr_id):
        delegate =  common.PhaseVsBaselineData(data, self.ms, corr_id,
                                               self.refant.id)
        return common.XYData(delegate, 
                             'distance_to_refant', 
                             'unwrapped_rms')

    def get_figfile(self, spw, scans, antennas):
        vis = os.path.basename(self.result.inputs['vis'])
        scan_ids = '_'.join(['%0.2d' % scan.id for scan in scans])
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            '%s.unwrapped_rms_vs_refant_distance.spw%0.2d.scan%s.png' % (vis, spw.id, scan_ids))

    def get_plot_object(self, spw, scans, antenna):
        figfile = self.get_figfile(spw, scans, antenna)
        scan_ids = ','.join([str(scan.id) for scan in scans])
        return logger.Plot(figfile,
                           x_axis='baseline length',
                           y_axis='phase offset',
                           parameters={'spw'  : spw.id,
                                       'scan' : scan_ids})
    
    
class GaincalPhaseOffsetPlotHelper(phaseoffset.PhaseOffsetPlotHelper):
    def __init__(self, context, result):
        calapp = result.final[0]
        
        rootdir = os.path.join(context.report_dir, 
                               'stage%s' % result.stage_number)
        prefix = '%s.phase_offset' % os.path.basename(calapp.vis)
        caltable_map = ordereddict.OrderedDict()
        caltable_map['AFTER'] = calapp.gaintable

        super(GaincalPhaseOffsetPlotHelper, self).__init__(rootdir, prefix, caltable_map)


class GaincalPhaseOffsetPlot(phaseoffset.PhaseOffsetPlot):
    def __init__(self, context, result):
        calapp = result.final[0]
        vis = os.path.basename(calapp.vis)
        ms = context.observing_run.get_ms(vis)
        plothelper = GaincalPhaseOffsetPlotHelper(context, result)        
        super(GaincalPhaseOffsetPlot, self).__init__(context, ms, plothelper, scan_intent='PHASE', score_retriever=common.NullScoreFinder())


class GaincalAmpVsTimeSummaryChart(common.PlotcalXVsYSummaryChart):
    xaxis = 'time'
    yaxis = 'amp'
    iteration = 'antenna'

    def __init__(self, context, result):
        super(GaincalAmpVsTimeSummaryChart, self).__init__(context, result)


class GaincalPhaseVsTimeSummaryChart(common.PlotcalXVsYSummaryChart):
    xaxis = 'time'
    yaxis = 'phase'
    iteration = 'antenna'

    def __init__(self, context, result):
        super(GaincalPhaseVsTimeSummaryChart, self).__init__(context, result)    
      



