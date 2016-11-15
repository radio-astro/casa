import os
import re

import pipeline.infrastructure.logging as logging
import pipeline.infrastructure.displays.bandpass as bandpass
import pipeline.infrastructure.displays.common as common
import pipeline.infrastructure.renderer.logger as logger
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
        self._figroot = self._figroot.replace('.png', '-%s.png'%(self.field_name))
        new_prefix = self._figroot.replace('.png', '')
        
        self._update_figfile(old_prefix, new_prefix)
            
        if not self._kwargs.has_key('field'):
            self._kwargs['field'] = self.field_id
            
    def add_field_identifier(self, plots):
        for plot in plots:
            if not plot.parameters.has_key('field'):
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
        for spw_id, figfile in self._figfile.items():
            print 'create plot for', spw_id
            if os.path.exists(figfile):
                print figfile, 'exists'
                task = self.create_task(spw_id, '')
                wrapper = logger.Plot(figfile,
                                      x_axis=self._xaxis,
                                      y_axis=self._yaxis,
                                      parameters={'vis' : self._vis_basename,
                                                  'spw' : spw_id,
                                                  'field' : self.field_name},
                                      command=str(task))
                wrappers.append(wrapper)
            else:
                LOG.trace('No plotbandpass summary plot found for spw '
                          '%s' % spw_id)
                
        return wrappers

    def _update_figfile(self, old_prefix, new_prefix):
        for (spw_id, figfile) in self._figfile.items():
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
        super(SingleDishSkyCalAmpVsFreqDetailChart, self).__init__(context, result,
                                                          xaxis='freq', yaxis='amp', showatm=True,
                                                          overlay='time')
        
        self.init_with_field(context, result, field)
    
    def plot(self):
        wrappers = super(SingleDishSkyCalAmpVsFreqDetailChart, self).plot()
         
        self.add_field_identifier(wrappers)
        
        return wrappers
    
    def _update_figfile(self, old_prefix, new_prefix):
        for spw_id in self._figfile.keys():
            for antenna_id, figfile in self._figfile[spw_id].items():
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
        LOG.debug('field: ID %s Name \'%s\''%(self.field_id, self.field_name))

        ants = ms.get_antenna(self.antenna)
        assert len(ants) == 1
        self.antenna_name = ants[0].name
        self.antenna_id = ants[0].id
        self.antenna_selection = '{antenna}&&&'.format(antenna=self.antenna_name)
        LOG.debug('antenna: ID %s Name \'%s\''%(self.antenna_id, self.antenna_name))

        self._figroot = os.path.join(context.report_dir, 
                                     'stage%s' % result.stage_number)
        
    def plot(self):
        prefix = '{caltable}-{y}_vs_{x}-{field}-{antenna}'.format(caltable=os.path.basename(self.caltable), 
                                                                  y=self.yaxis, x=self.xaxis,
                                                                  field=self.field_name,
                                                                  antenna=self.antenna_name)
        title = '{caltable} \nField "{field}" Antenna {antenna}'.format(caltable=os.path.basename(self.caltable), 
                                                                    field=self.field_name,
                                                                    antenna=self.antenna_name)
        if len(self.spw) > 0:
            prefix += '-spw{spw}'.format(spw=self.spw)
            title += ' Spw {spw}'.format(spw=self.spw)
            
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
                     'antenna': self.antenna_selection,
                     'title': title,
                     'showlegend': True,
                     'averagedata': True,
                     'avgchannel': '1e8'}
        
        return casa_tasks.plotms(**task_args)
    
    def _get_plot_object(self, figfile, task):
        parameters = {'vis': os.path.basename(self.vis),
                      'ant': self.antenna_name,
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
    
class SingleDishSkyCalAmpVsTimeSummaryChart(SingleDishPlotmsAntComposite):
    def __init__(self, context, result, calapp):
        super(SingleDishSkyCalAmpVsTimeSummaryChart, self).__init__(context, result, calapp,
                                                                    xaxis='time', yaxis='amp', 
                                                                    coloraxis='spw')
        
class SingleDishSkyCalAmpVsTimeDetailChart(SingleDishPlotmsAntSpwComposite):
    def __init__(self, context, result, calapp):
        super(SingleDishSkyCalAmpVsTimeDetailChart, self).__init__(context, result, calapp, 
                                                                   xaxis='time', yaxis='amp',
                                                                   coloraxis='corr')
        