from __future__ import absolute_import
import os

import matplotlib
import matplotlib.pyplot as pyplot
import matplotlib.gridspec as gridspec
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.utils as utils
from . import common

LOG = infrastructure.get_logger(__name__)


class PhaseOffsetPlotHelper(object):
    colour_map = {'BEFORE' : {'L' : ('-', 'orange', 0.6),
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

    """
    caltable_map should be a dictionary mapping the state to caltable
    """
    def __init__(self, rootdir, prefix, caltable_map=None, plot_per_antenna=True):
        assert set(PhaseOffsetPlotHelper.colour_map.keys()).issuperset(set(caltable_map.keys())), 'caltables argument defines states not in colour_map'
        self._rootdir = rootdir
        self._prefix = prefix
        self.caltable_map = caltable_map
        self.plot_per_antenna = plot_per_antenna
        
    def get_symbol_and_colour(self, pol, state):
        return self.colour_map[state][pol]

    def get_figfile(self, spw, antennas):
        if len(antennas) is 1:
            antenna = '.ant%s' % antennas[0].name
        else:
            antenna = ''
        return os.path.join(self._rootdir, 
                            '%s%s.spw%0.2d.png' % (self._prefix, antenna, spw.id))

    def group_antennas(self, antennas):
        if self.plot_per_antenna:
            return [[ant] for ant in antennas]
        else:
            return [antennas]
                        
    def label_antenna(self, fig, antennas):
        if self.plot_per_antenna:
            text = '%s' % antennas[0].name
        else:
            text = 'All Antennas'

        pyplot.text(0.5, 0.911, '%s' % text, color='k', 
                    transform=fig.transFigure, ha='center', size=10)
            


class PhaseOffsetPlot(object):
    def __init__(self, context, ms, plothelper, scan_intent=None, scan_id=None, score_retriever=None):
        self._context = context
        self._ms = ms        
        self._plothelper = plothelper        
        self._scans = ms.get_scans(scan_id=scan_id, scan_intent=scan_intent)
        self._score_retriever = score_retriever if score_retriever else common.NullScoreFinder()
        
        self._caltables_loaded = False
        self._load_caltables(plothelper.caltable_map)

    def _load_caltables(self, caltable_map):
        if self._caltables_loaded:
            return
        
        data = [(state, common.CaltableWrapper.from_caltable(c)) for state, c in caltable_map.items()]

        # some sanity checks, as unequal caltables have bit me before
        # this doesn't work when WVR data are missing and should be interpolated over
#         if len(data) > 1:
#             wrapper1 = data[0][1]
#             for state, wrapper2 in data[1:]:
#                 assert utils.areEqual(wrapper1.time, wrapper2.time), 'Time columns are not equal'
#                 assert utils.areEqual(wrapper1.antenna, wrapper2.antenna), 'Antenna columns are not equal'
#                 assert utils.areEqual(wrapper1.spw, wrapper2.spw), 'Spw columns are not equal'
#                 assert utils.areEqual(wrapper1.scan, wrapper2.scan), 'Scan columns are not equal'

        self.data = data
        self._caltables_loaded = True
        
    def plot(self, spw_ids=None, antenna_ids=None, antenna_names=None):
        # these function arguments are used for debugging the plot routines,
        # so we can call plot for a particular troublesome spw/antenna.
        ms = self._ms
        plothelper = self._plothelper

        # get the spw IDs common to all caltables..
        all_spw_ids = [set(data.spw) for _, data in self.data]
        common_spw_ids = reduce(lambda x, y: x.intersection(y), all_spw_ids)        
        # .. filter to match those specified as function arguments..
        if spw_ids is not None:
            common_spw_ids = common_spw_ids.intersection(set(spw_ids))
        # .. before converting to domain objects
        spws = [spw for spw in ms.spectral_windows 
                if spw.id in common_spw_ids]

        # Do the same for antenna IDs, finding those common to all caltables..
        all_antenna_ids = [set(data.antenna) for _, data in self.data]
        common_antenna_ids = reduce(lambda x, y: x.intersection(y), all_antenna_ids)
        # .. filtering to match those specified as function arguments..
        if antenna_ids is not None:
            common_antenna_ids = common_antenna_ids.intersection(set(antenna_ids))
        if antenna_names is not None:
            named_antenna_ids = set([ant.id for ant in ms.antennas
                                     if ant.name in antenna_names])
            common_antenna_ids = common_antenna_ids.intersection(named_antenna_ids)
        # .. before converting to domain objects
        antennas = [ant for ant in ms.antennas
                    if ant.id in common_antenna_ids] 

        assert len(spws) > 0, 'No common spws to plot in %s' % utils.commafy([d.filename for d in self.data])
        assert len(antennas) > 0, 'No common antennas to plot in %s' % utils.commafy([d.filename for d in self.data])

        plots = []
        for spw in spws:
            for antenna_group in plothelper.group_antennas(antennas):
                plots.append(self.get_plot_wrapper(spw, self._scans, antenna_group))

        return [p for p in plots if p is not None]
    
    def create_plot(self, spw, scans, antennas):
        # get the fields and scan intents from the list of scans. These are
        # used in the plot title, eg. NGC123 (PHASE)
        scan_fields = set()
        for scan in scans:
            scan_fields.update([field.name for field in scan.fields])
        scan_fields = ','.join(scan_fields)

        scan_intents = set()
        for scan in scans:
            scan_intents.update(scan.intents)
        scan_intents.discard('WVR')
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

        fig = pyplot.figure()

        # size subplots proportional to the scan time
        integration_times = [int(s.time_on_source.total_seconds()) for s in scans]        
        gs = gridspec.GridSpec(1, num_scans, width_ratios=integration_times)
        ax0 = fig.add_subplot(gs[0])
        axes = [fig.add_subplot(gs[i], sharey=ax0) for i in range(1, num_scans)]
        for axis in axes:
            for label in axis.get_yticklabels():
                label.set_visible(False)

        axes.insert(0, ax0)
        
        # if num_scans is 1, axes will be a scalar instead of a list
        if not isinstance(axes, (tuple, list, numpy.ndarray)):
            axes = [axes]
            
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
             
        plothelper = self._plothelper
        flag_annotate = len(antennas) is 1
        for scan_idx, scan in enumerate(scans):
            for antenna in antennas:    
                axis = axes[scan_idx]
                plots = []
                legends = []
                for state, state_data in self.data:
                    try:
                        data = state_data.filter(scan=[scan.id], 
                                                 antenna=[antenna.id], 
                                                 spw=[spw.id])
                    except KeyError:
                        # scan/antenna/id not in caltable, probably flagged.
                        
                        # create fake masked slices and data arrays so we can 
                        # plot flagged annotation 
                        class dummy(object):
                            pass
                        
                        dummy_slice = dummy()
                        dummy_slice.start = 0
                        dummy_slice.stop = 1

                        dummy_time = dummy()
                        start_dt = utils.get_epoch_as_datetime(scan.start_time)
                        end_dt = utils.get_epoch_as_datetime(scan.end_time)
                        dummy_time.time = [matplotlib.dates.date2num(start_dt), 
                                           matplotlib.dates.date2num(end_dt)]

                        dummy_data = numpy.ma.MaskedArray(data=[0,1],
                                                          mask=True)
 
                        axis.plot_date(dummy_time.time, dummy_data, '.')
                        p, = axis.plot_date(dummy_time.time, dummy_data)

                        self._plot_flagged_data(dummy_time, dummy_slice, axis,
                                                True, annotation='NO DATA')
                        
                        axis.set_xlim(dummy_time.time[0], dummy_time.time[-1])
                        axis.set_ylim(autoscale_yaxis_range)
                        
                        continue
                    
                    for corr_idx, corr_axis in enumerate(corr_axes):
                        if len(data.time) is 0:
                            LOG.info('No data to plot for antenna %s scan %s corr %s' % (antenna.name, scan.id, corr_axis))
                            continue
                        
                        phase_for_corr = data.data[corr_idx][0]
                        rad_phase = numpy.deg2rad(phase_for_corr)
                        unwrapped_phase = numpy.unwrap(rad_phase)
                        offset_rad = unwrapped_phase - numpy.ma.median(unwrapped_phase)
                        # the operation above removed the mask, so add it back.
                        offset_rad = numpy.ma.MaskedArray((offset_rad), mask=phase_for_corr.mask)
                        offset_deg = numpy.rad2deg(offset_rad)

                        for masked_slice in numpy.ma.clump_masked(offset_deg):
                            self._plot_flagged_data(data, masked_slice, axis, flag_annotate)
                        
                        (symbol, color, alpha) = plothelper.get_symbol_and_colour(corr_axis, 
                                                                                  state)
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
        plothelper.label_antenna(fig, antennas)
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
        
        figfile = plothelper.get_figfile(spw, antennas)
        pyplot.savefig(figfile)
        pyplot.close()

    def get_plot_wrapper(self, spw, scans, antennas):
        plothelper = self._plothelper
        
        antenna_names = [ant.name for ant in antennas]
        
        figfile = plothelper.get_figfile(spw, antennas)
        wrapper = logger.Plot(figfile,
                              x_axis='scan',
                              y_axis='phase',
                              parameters={'spw' : spw.id,
                                          'ant' : antenna_names})
        
        if plothelper.plot_per_antenna and len(antennas) is 1:
            wrapper.qa_score = self._score_retriever.get_score(spw, antennas[0])
        
        if not os.path.exists(figfile):
            LOG.trace('Phase offset plot for antenna %s spw %s not found.'
                      ' Creating new plot: %s' % (utils.commafy(antenna_names, quotes=False), spw.id, figfile))
            try:
                self.create_plot(spw, scans, antennas)
            except Exception as ex:
                LOG.error('Could not create phase offset plot for antenna'
                          ' %s spw %s' % (utils.commafy(antenna_names, quotes=False), spw.id))
                LOG.exception(ex)
                return None

        # the plot may not be created if all data for that antenna are flagged
        if os.path.exists(figfile):
            return wrapper            
        return None
    
    def _plot_flagged_data(self, data, masked_slice, axis, annotate=True,
                           annotation='FLAGGED'):
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
        
        if annotate:
            axis.text(start + width/2, 0.5, annotation, color='k', transform=trans,
                      size=10, ha='center', va='center', rotation=90)                            
