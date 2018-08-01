import os
import pylab as pl
import numpy

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.renderer.logger as logger
from pipeline.h.tasks.common.displays import common as common
from pipeline.h.tasks.common.displays import bandpass as bandpass
from ..common import display as sd_display
from pipeline.infrastructure import casa_tasks

LOG = logging.get_logger(__name__)


class SingleDishSkyCalDisplayBase(object):
    def init_with_field(self, context, result, field):
        vis = self._vis 
        ms = context.observing_run.get_ms(vis)
        num_fields = len(ms.fields)
        if field.isdigit() and int(field) < num_fields:
            self.field_id = int(field)
            self.field_name = ms.get_fields(self.field_id)[0].clean_name
        else:
            self.field_name = field
            fields = ms.get_fields(name=field)
            assert len(fields) == 1
            self.field_id = fields[0].id
        LOG.debug('field: ID %s Name \'%s\''%(self.field_id, self.field_name))
        old_prefix = self._figroot.replace('.png', '')
        self._figroot = self._figroot.replace('.png', '-%s.png' % (self.field_name))
        new_prefix = self._figroot.replace('.png', '')
        
        self._update_figfile(old_prefix, new_prefix)
            
        if 'field' not in self._kwargs:
            self._kwargs['field'] = self.field_id
            
    def add_field_identifier(self, plots):
        for plot in plots:
            if 'field' not in plot.parameters:
                plot.parameters['field'] = self.field_name
                
    def _update_figfile(self):
        raise NotImplementedError()


class SingleDishSkyCalAmpVsFreqSummaryChart(common.PlotbandpassDetailBase, SingleDishSkyCalDisplayBase):
    def __init__(self, context, result, field):
        super(SingleDishSkyCalAmpVsFreqSummaryChart, self).__init__(context, result, 
                                                                    'freq', 'amp', 
                                                                    showatm=True,
                                                                    overlay='antenna',
                                                                    solutionTimeThresholdSeconds=3600.)

        self.spw_ids = self._figfile.keys()
        self._figfile = dict(((spw_id, self._figfile[spw_id][0]) \
                              for spw_id in self.spw_ids))
        self.init_with_field(context, result, field)
        
    def plot(self):
        missing = [spw_id
                   for spw_id in self.spw_ids
                   if not os.path.exists(self._figfile[spw_id])]
        if missing:
            LOG.trace('Executing new plotbandpass job for missing figures')
            #ant_ids = ','.join([str(ant_id) for ant_id in missing])
            for spw_id in missing:
                try:
                    task = self.create_task(spw_id, '')
                    task.execute(dry_run=False)
                except Exception as ex:
                    LOG.error('Could not create plotbandpass summary plots')
                    LOG.exception(ex)
                    return None

        wrappers = []
        for spw_id, figfile in self._figfile.iteritems():
            print('create plot for {}'.format(spw_id))
            if os.path.exists(figfile):
                print('{} exists'.format(figfile))
                task = self.create_task(spw_id, '')
                wrapper = logger.Plot(figfile,
                                      x_axis=self._xaxis,
                                      y_axis=self._yaxis,
                                      parameters={'vis': self._vis_basename,
                                                  'spw': spw_id,
                                                  'field': self.field_name},
                                      command=str(task))
                wrappers.append(wrapper)
            else:
                LOG.trace('No plotbandpass summary plot found for spw '
                          '%s' % spw_id)
                
        return wrappers

    def _update_figfile(self, old_prefix, new_prefix):
        for (spw_id, figfile) in self._figfile.iteritems():
            self._figfile[spw_id] = figfile.replace(old_prefix, new_prefix)
            spw_indicator = 'spw{}'.format(spw_id)
            pieces = self._figfile[spw_id].split('.')
            try:
                spw_index = pieces.index(spw_indicator)
            except:
                spw_index = -3
            pieces.pop(spw_index - 1)
            self._figfile[spw_id] = '.'.join(pieces)


class SingleDishSkyCalAmpVsFreqDetailChart(bandpass.BandpassDetailChart, SingleDishSkyCalDisplayBase):
    def __init__(self, context, result, field):
        super(SingleDishSkyCalAmpVsFreqDetailChart, self).__init__(
            context, result, xaxis='freq', yaxis='amp', showatm=True, overlay='time')
        
        self.init_with_field(context, result, field)
    
    def plot(self):
        wrappers = super(SingleDishSkyCalAmpVsFreqDetailChart, self).plot()
         
        self.add_field_identifier(wrappers)
        
        return wrappers
    
    def _update_figfile(self, old_prefix, new_prefix):
        for spw_id in self._figfile.keys():
            for antenna_id, figfile in self._figfile[spw_id].iteritems():
                new_figfile = figfile.replace(old_prefix, new_prefix)
                self._figfile[spw_id][antenna_id] = new_figfile 


class SingleDishPlotmsLeaf(object):
    """
    Class to execute plotms and return a plot wrapper. Task arguments for plotms 
    is customized for single dish usecase.
    """
    def __init__(self, context, result, calapp, xaxis, yaxis, spw='', ant='', coloraxis='', **kwargs):
        LOG.debug('__init__(caltable={caltable}, spw={spw}, ant={ant})'.format(caltable=calapp.gaintable, spw=spw, ant=ant))
        self.xaxis = xaxis
        self.yaxis = yaxis
        self.field = calapp.gainfield
        self.caltable = calapp.gaintable
        self.vis = calapp.vis
        self.spw = str(spw)
        self.antenna = str(ant)
        self.coloraxis = coloraxis

        ms = context.observing_run.get_ms(self.vis)

        num_fields = len(ms.fields)
        if self.field.isdigit() and int(self.field) < num_fields:
            self.field_id = int(self.field)
            self.field_name = ms.get_fields(self.field_id)[0].clean_name
        else:
            self.field_name = self.field
            fields = ms.get_fields(name=self.field)
            assert len(fields) == 1
            self.field_id = fields[0].id

        LOG.debug('field: ID %s Name \'%s\'' % (self.field_id, self.field_name))

        self.antmap = dict((a.id, a.name) for a in ms.antennas)
        if len(self.antenna) == 0:
            self.antenna_selection = 'summary'
        else:
            self.antenna_selection = self.antmap.values()[int(self.antenna)]
        LOG.info('antenna: ID %s Name \'%s\'' % (self.antenna, self.antenna_selection))
#        self.antenna_selection = '*&&&'

        self._figroot = os.path.join(context.report_dir, 
                                     'stage%s' % result.stage_number)
        
    def plot(self):

        prefix = '{caltable}-{y}_vs_{x}-{field}-{ant}-spw{spw}'.format(
            caltable=os.path.basename(self.caltable), y=self.yaxis, x=self.xaxis, field=self.field_name, ant=self.antenna_selection, spw=self.spw)

        title = '{caltable} \nField "{field}" Antenna {ant} Spw {spw}'.format(
            caltable=os.path.basename(self.caltable), field=self.field_name, ant=self.antenna_selection, spw=self.spw)

        figfile = os.path.join(self._figroot, '{prefix}.png'.format(prefix=prefix))

        task = self._create_task(title, figfile)
        if os.path.exists(figfile):
            LOG.debug('Returning existing plot')
        else:
            task.execute()
            
        return [self._get_plot_object(figfile, task)]
        
    def _create_task(self, title, figfile):
        task_args = {'vis': self.caltable,
                     'xaxis': self.xaxis,
                     'yaxis': self.yaxis,
                     'plotfile': figfile,
                     'coloraxis': self.coloraxis,
                     'showgui': False,
                     'field': self.field,
                     'spw': self.spw,
                     'antenna': self.antenna,
                     'title': title,
                     'showlegend': True,
                     'averagedata': True,
                     'avgchannel': '1e8'}
        
        return casa_tasks.plotms(**task_args)
    
    def _get_plot_object(self, figfile, task):
        parameters = {'vis': os.path.basename(self.vis),
                      'ant': self.antenna_selection,
                      'spw': self.spw,
                      'field': self.field_name}
        
        return logger.Plot(figfile,
                           x_axis='Time',
                           y_axis='Amplitude',
                           parameters=parameters,
                           command=str(task))


class SingleDishPlotmsAntComposite(common.AntComposite):
    leaf_class = SingleDishPlotmsLeaf


class SingleDishPlotmsSpwComposite(common.SpwComposite):
    leaf_class = SingleDishPlotmsLeaf
    

class SingleDishPlotmsAntSpwComposite(common.AntSpwComposite):
    leaf_class = SingleDishPlotmsSpwComposite
    

class SingleDishSkyCalAmpVsTimeSummaryChart(SingleDishPlotmsSpwComposite):
    def __init__(self, context, result, calapp):
        super(SingleDishSkyCalAmpVsTimeSummaryChart, self).__init__(context, result, calapp,
                                                                    xaxis='time', yaxis='amp', 
                                                                    coloraxis='ant1')


class SingleDishSkyCalAmpVsTimeDetailChart(SingleDishPlotmsAntSpwComposite):
    def __init__(self, context, result, calapp):
        super(SingleDishSkyCalAmpVsTimeDetailChart, self).__init__(context, result, calapp, 
                                                                   xaxis='time', yaxis='amp',
                                                                   coloraxis='corr')
        
def plot_elevation_difference(context, result, eldiff, threshold=3.0, perantenna=False):
    """
    context 
    result
    eldiff -- dictionary whose value is ElevationDifference named tuple instance that holds 
                  'timeon': timestamp for ON-SOURCE pointings
                  'elon': ON-SOURCE elevation
                  'timecal': timestamp for OFF-SOURCE pointings
                  'elcal': OFF-SOURCE elevation 
                  'time0': timestamp for preceding OFF-SOURCE pointings 
                  'eldiff0': elevation difference between ON-SOURCE and preceding OFF-SOURCE
                  'time1': timestamp for subsequent OFF-SOURCE pointings
                  'eldiff1': elevation difference between ON-SOURCE and subsequent OFF-SOURCE
              eldiff is a nested dictionary whose first key is FIELD_ID while the second one 
              is ANTENNA_ID. 
    threhshold -- Elevation threshold for QA (default 3deg)
    perantenna -- Generate plots per antenna or not. If False, overlay all the data into one plot
    """
    calapp = result.final[0]
    vis = calapp.calto.vis
    ms = context.observing_run.get_ms(vis)
    
    figroot = os.path.join(context.report_dir, 
                           'stage%s' % result.stage_number)
    field_ids = eldiff.keys()
    assert len(field_ids) == 1
    field_id = field_ids[0]
    field = ms.fields[field_id]
    field_name = field.name
    
    eldiff_field = eldiff[field_id]
    antenna_ids = eldiff_field.keys()
    rep_time = eldiff_field[antenna_ids[0]].timeon
    start_time = rep_time.min()
    end_time = rep_time.max()
    
    figure = pl.figure()
    
    def init_figure():
        pl.clf()
        a0 = pl.axes([0.125, 0.51, 0.775, 0.39])
        a0.xaxis.set_major_locator(sd_display.utc_locator(start_time=start_time, end_time=end_time))
        a0.xaxis.set_major_formatter(pl.NullFormatter())
        pl.ylabel('Elevation [deg]')
        a1 = pl.axes([0.125, 0.11, 0.775, 0.39])
        a1.xaxis.set_major_locator(sd_display.utc_locator(start_time=start_time, end_time=end_time))
        a1.xaxis.set_major_formatter(sd_display.utc_formatter())
        pl.ylabel('Elevation Difference [deg]')
        pl.xlabel('UTC')
        return a0, a1
    a0, a1 = init_figure()
    
    thresh_line = None
    setlabel = True
    plots = []
    for antenna_id, eld in eldiff_field.items():
        antenna_name = ms.antennas[antenna_id].name
        
        if len(eld.timeon) == 0:
            continue
        
        # Elevation vs. Time
        pl.gcf().sca(a0)
        lon = pl.plot(sd_display.mjd_to_plotval(eld.timeon), eld.elon, '.', color='black', 
                mew=0)
        loff = pl.plot(sd_display.mjd_to_plotval(eld.timecal), eld.elcal, '.-', color='blue', 
                mew=0)
        
        dolabel = perantenna == True or (perantenna == False and setlabel == True)
        if dolabel == True:
            for l in lon:
                l.set_label('ON')
            for l in loff:
                l.set_label('OFF')
        pl.legend(loc='best', numpoints=1, prop={'size': 'small'})

        # Elevation Difference vs. Time
        pl.gcf().sca(a1)
        time0 = eld.time0
        eldiff0 = eld.eldiff0
        time1 = eld.time1
        eldiff1 = eld.eldiff1
        io0 = numpy.where(numpy.abs(eldiff0) < threshold)[0]
        ix0 = numpy.where(numpy.abs(eldiff0) >= threshold)[0]
        io1 = numpy.where(numpy.abs(eldiff1) < threshold)[0]
        ix1 = numpy.where(numpy.abs(eldiff1) >= threshold)[0]
        x = time0[io0]
        y = eldiff0[io0]
        pl.plot(sd_display.mjd_to_plotval(x), numpy.abs(y), 'g.', mew=0)
        x = time1[io1]
        y = eldiff1[io1]
        pl.plot(sd_display.mjd_to_plotval(x), numpy.abs(y), 'g.', mew=0)
        if len(ix0) > 0:
            x = time0[ix0]
            y = eldiff0[ix0]
            pl.plot(sd_display.mjd_to_plotval(x), numpy.abs(y), 'rs', mew=0)
        if len(ix1) > 0:
            x = time1[ix1]
            y = eldiff1[ix1]
            pl.plot(sd_display.mjd_to_plotval(x), numpy.abs(y), 'rs', mew=0)

        if thresh_line is None:
            pl.axhline(threshold, color='red')
            xmin, xmax = pl.xlim()
            dx = xmax - xmin
            x = xmax - 0.01 * dx
            y = threshold - 0.05
            pl.text(x, y, 'QA threshold', ha='right', va='top', color='red', size='small')

        if perantenna == True:
            # rescale y-axis
            pl.gcf().sca(a1)
            ymin, ymax = pl.ylim()
            dy = ymax - ymin
            pl.ylim([0, max(ymax + 0.1 * dy, threshold + 0.1)])
            
            # save plot and clear
            pl.gcf().sca(a0)
            pl.title('Elevation Difference between ON and OFF\n{} Field {} Antenna {}'.format(ms.basename, 
                                                                                              field_name, 
                                                                                              antenna_name))
            vis_prefix = '.'.join(ms.basename.split('.')[:-1])
            figfile = 'elevation_difference_{}_{}_{}.png'.format(vis_prefix, field.clean_name, antenna_name)
            figpath = os.path.join(figroot, figfile)
            #LOG.info('figpath={}'.format(figpath))
            pl.savefig(figpath)
            if os.path.exists(figpath):
                parameters = {'intent': 'TARGET',
                              'spw': '',
                              'pol': '',
                              'ant': antenna_name,
                              'vis': ms.basename,
                              'type': 'Elevation Difference vs. Time',
                              'file': ms.basename}
                plot = logger.Plot(figpath,
                                   x_axis='Time',
                                   y_axis='Elevation Difference',
                                   field=field_name,
                                   parameters=parameters)
                plots.append(plot)
            
            a0, a1 = init_figure()
            
        else:
            setlabel = False

    if perantenna == False:
        assert len(plots) == 0
        
        # rescale y-axis
        pl.gcf().sca(a1)
        ymin, ymax = pl.ylim()
        dy = ymax - ymin
        pl.ylim([0, max(ymax + 0.1 * dy, threshold + 0.1)])
        
        # save plot and clear
        pl.gcf().sca(a0)
        pl.title('Elevation Difference between ON and OFF\n{} Field {} Antenna {}'.format(ms.basename, 
                                                                                          field_name, 
                                                                                          'ALL'))
        vis_prefix = '.'.join(ms.basename.split('.')[:-1])
        figfile = 'elevation_difference_{}_{}.png'.format(vis_prefix, field.clean_name)
        figpath = os.path.join(figroot, figfile)
        #LOG.info('figpath={}'.format(figpath))
        pl.savefig(figpath)
        if os.path.exists(figpath):
            parameters = {'intent': 'TARGET',
                          'spw': '',
                          'pol': '',
                          'ant': '',
                          'vis': ms.basename,
                          'type': 'Elevation Difference vs. Time',
                          'file': ms.basename}
            plot = logger.Plot(figpath,
                               x_axis='Time',
                               y_axis='Elevation Difference',
                               field=field_name,
                               parameters=parameters)
            plots.append(plot)
        
    
    return plots
    