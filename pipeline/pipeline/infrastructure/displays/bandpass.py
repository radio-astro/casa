from __future__ import absolute_import
import re
import os

import matplotlib.pyplot as pyplot
import matplotlib.ticker as ticker

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
from pipeline.infrastructure import casa_tasks
from . import common

LOG = infrastructure.get_logger(__name__)


class NullScoreFinder(object):
    def get_score(self, spw, antenna):
        return None


class BandpassDetailChart(common.PlotbandpassAntSpwComposite):
    """
    Base class for executing plotbandpass per spw and antenna
    """
    def __init__(self, context, result, xaxis, yaxis):
        # identify the bandpass solution for the target
        calapps = [c for c in result.final
                   if (c.intent == '' or 'TARGET' in c.intent)]

        assert len(calapps) is 1, 'Target bandpass solutions != 1'
        calapp = calapps[0]
        
        # request plots per spw, overlaying all antennas
        super(BandpassDetailChart, self).__init__(
                context, result, calapp, xaxis=xaxis, yaxis=yaxis)
            

class BandpassAmpVsFreqDetailChart(BandpassDetailChart):
    """
    Create an amp vs freq plot for each spw/antenna combination.
    """
    def __init__(self, context, result):
        # request plots per spw, overlaying all antennas
        super(BandpassAmpVsFreqDetailChart, self).__init__(
                context, result, xaxis='freq', yaxis='amp')


class BandpassPhaseVsFreqDetailChart(BandpassDetailChart):
    """
    Create an amp vs freq plot for each spw/antenna combination.
    """
    def __init__(self, context, result):
        # request plots per spw, overlaying all antennas
        super(BandpassPhaseVsFreqDetailChart, self).__init__(
                context, result, xaxis='freq', yaxis='phase')


class BandpassSummaryChart(common.PlotbandpassAntComposite):
    """
    Base class for executing plotbandpass per spw and antenna
    """
    def __init__(self, context, result, xaxis, yaxis):
        # identify the bandpass solution for the target
        calapps = [c for c in result.final
                   if (c.intent == '' or 'TARGET' in c.intent)]

        assert len(calapps) is 1, 'Target bandpass solutions != 1'
        calapp = calapps[0]
        
        # request plots per spw, overlaying all antennas
        super(BandpassSummaryChart, self).__init__(
                context, result, calapp, xaxis=xaxis, yaxis=yaxis,
                overlay='baseband', showatm=False)


class BandpassAmpVsFreqSummaryChart(BandpassSummaryChart):
    """
    Create an amp vs freq plot for each antenna
    """
    def __init__(self, context, result):
        # request plots per spw, overlaying all antennas
        super(BandpassAmpVsFreqSummaryChart, self).__init__(
                context, result, xaxis='freq', yaxis='amp')


class BandpassPhaseVsFreqSummaryChart(BandpassSummaryChart):
    """
    Create an phase vs freq plot for each antenna
    """
    def __init__(self, context, result):
        # request plots per spw, overlaying all antennas
        super(BandpassPhaseVsFreqSummaryChart, self).__init__(
                context, result, xaxis='freq', yaxis='phase')        


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









    