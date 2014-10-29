from __future__ import absolute_import
import collections
import os
import string

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.utils as utils
from pipeline.infrastructure import casa_tasks
from . import common


LOG = infrastructure.get_logger(__name__)


class PlotmsLeaf(object):
    """
    Class to execute plotms and return a plot wrapper. It passes the spw and
    ant arguments through to plotms without further manipulation, creating
    exactly one plot. 
    """
    def __init__(self, context, result, calto, xaxis, yaxis,  
                 spw='', ant='', field='', scan='', intent='', **plot_args):
        self._context = context
        self._result = result

        self._vis = calto.vis
        self._ms = context.observing_run.get_ms(self._vis)

        self._xaxis = xaxis
        self._yaxis = yaxis

        self._spw = spw
        self._intent = intent
        
        # convert intent to scan selection
        if intent != '':
            domain_fields = self._ms.get_fields(field)
            domain_spws = self._ms.get_spectral_windows(spw)
            scans = [s for s in self._ms.get_scans(scan_intent=intent)
                     if s.fields.intersection(domain_fields)
                     and s.spws.intersection(domain_spws)]
            scan = ','.join([str(s.id) for s in scans])
        self._scan = scan

        # use field name rather than ID where possible
        field_label = ''
        if field != '':
            matching = self._ms.get_fields(field)

            if len(matching) is 1:
                field_name = matching[0].name
                field_id = matching[0].id
                # in case the arg is an ID, check that returned field doesn't
                # contain a duplicate name
                matching_by_name = self._ms.get_fields(field_name)
                if len(matching_by_name) is 1:
                    # if the argument was the field ID but the field name is 
                    # unique. Therefore, we can use the field name.
                    field = field_name
                    field_label = field_name
                    LOG.trace('One field found for search term %r and name '
                              'appears unique. Using %r as field identifier',
                              field, field_label)
                else:
                    # if the argument was the field ID but the field name is 
                    # not unique, prepend the ID with the field name
                    field_label = '%s (field #%s)' % (field_name, field_id)
                    LOG.trace('One field found for search term %r but name '
                              'also refers to other fields. Using %r as field '
                              'identifier', field, field_label)
            else:
                LOG.trace('More than one field found for search term %r. '
                          'Using %r as field identifier', field, field)
                field_label = field

        self._field = str(field)
        self._field_label = str(field_label)
        
        # use antenna name rather than ID if possible
        if ant != '':
            domain_antennas = self._ms.get_antenna(ant)
            idents = [a.name if a.name else a.id for a in domain_antennas]
            ant = ','.join(idents)
        self._ant = ant

        # get baseband from extra kwargs, deleting it after copying so it
        # doesn't affect the plotms call
        self._baseband = plot_args.get('baseband', '')
        if self._baseband:
            del plot_args['baseband']

        self._plot_args = plot_args
        self._plotfile = self._get_plotfile()

    def plot(self):
        plots = [self._get_plot_wrapper()]
        return [p for p in plots if p is not None]
            
    def _get_plotfile(self):
        fileparts = {
            'vis'      : os.path.basename(self._vis),
            'x'        : self._xaxis,
            'y'        : self._yaxis,
            'ant'      : '' if self._ant == '' else 'ant%s-' % self._ant.replace(',','_'),
            'field'    : '' if self._field_label == '' else '-%s' % filenamer.sanitize(self._field_label.replace(',','_')),
            'intent'   : '' if self._intent == '' else '%s-' % self._intent.replace(',','_')
        }

        if self._spw == '':
            fileparts['spw'] = ''
        else:
            # format spws for filename sorting
            spws = ['%0.2d' % int(spw) for spw in string.split(str(self._spw), ',')]
            fileparts['spw'] = 'spw%s-' % '_'.join(spws)

        if self._baseband:
            fileparts['spw'] = 'bb%s-' % self._baseband

        png = '{vis}{field}-{spw}{ant}{intent}{y}_vs_{x}.png'.format(**fileparts)

        # Maximum filename size for Lustre filesystems is 255 bytes. Mosaics
        # can exceed this limit due to including the names of all the field.
        # Truncate over-long field components while keeping them unique by
        # replacing them with the hash of the component          
        if len(png) > 251: # 255 - '.png'
            png_hash = str(hash(png)) 
            LOG.info('Truncating plot filename to avoid filesystem limit.\n'
                     'Old: %s\nNew: %s', png, png_hash)
            png = '%s.png' % png_hash

        return os.path.join(self._context.report_dir, 
                            'stage%s' % self._result.stage_number,
                            png)

    def _get_plot_wrapper(self):
        if not os.path.exists(self._plotfile):
            LOG.trace('Creating new plot: %s' % self._plotfile)
            try:
                self._create_plot()
            except Exception as ex:
                LOG.error('Could not create plot %s' % self._plotfile)
                LOG.exception(ex)
                return None

        parameters={'vis' : self._vis}

        if self._field != '':
            parameters['field'] = self._field_label

        if self._baseband != '':
            parameters['baseband'] = self._baseband

        for attr in ['spw', 'ant', 'intent', 'scan']:
            val = getattr(self, '_%s' % attr)
            if val != '':
                parameters[attr] = val 
            
        wrapper = logger.Plot(self._plotfile,
                              x_axis=self._xaxis,
                              y_axis=self._yaxis,
                              parameters=parameters)
            
        return wrapper

    def _create_plot(self):
        task_args = {'vis'             : self._ms.name,
                     'xaxis'           : self._xaxis,
                     'yaxis'           : self._yaxis,
                     'field'           : str(self._field),
                     'spw'             : str(self._spw),
                     'scan'            : str(self._scan),
                     'antenna'         : self._ant,
                     'plotfile'        : self._plotfile,
                     'clearplots'      : True,
                     'showgui'         : False}

        task_args.update(**self._plot_args)
        
        task = casa_tasks.plotms(**task_args)
        task.execute(dry_run=False)
        
        if not os.path.exists(self._plotfile):
            LOG.info('The last plotms call did not generate an output file. '
                     'If the data selection was flagged, this is to be ' 
                     'expected.')


class SpwComposite(common.LeafComposite):
    """
    Create a PlotLeaf for each spw in the applied calibration.
    """
    # reference to the PlotLeaf class to call
    leaf_class = None

    def __init__(self, context, result, calto, xaxis, yaxis, ant='', field='',
                 intent='', **kwargs):
        ms = context.observing_run.get_ms(calto.vis)
        
        children = []
        for spw in ms.get_spectral_windows(calto.spw):
            if intent != '':
                wanted = set(intent.split(','))
                if spw.intents.isdisjoint(wanted):
                    continue

            kwargs_copy = dict(kwargs)
            kwargs_copy['avgchannel'] = kwargs.get('avgchannel', 
                                                   str(spw.num_channels))

            leaf_obj = self.leaf_class(context, result, calto, xaxis, yaxis,
                                       spw=spw.id, ant=ant, field=field, 
                                       intent=intent, **kwargs_copy)
            children.append(leaf_obj)
                 
        super(SpwComposite, self).__init__(children)


class BasebandComposite(common.LeafComposite):
    """
    Create a PlotLeaf for each baseband in the applied calibration.
    """
    # reference to the PlotLeaf class to call
    leaf_class = None

    def __init__(self, context, result, calto, xaxis, yaxis, ant='', field='',
                 intent='', **kwargs):
        ms = context.observing_run.get_ms(calto.vis)
        
        basebands = collections.defaultdict(list)        
        for spw in ms.get_spectral_windows(calto.spw):
            if intent != '':
                wanted = set(intent.split(','))
                if spw.intents.isdisjoint(wanted):
                    continue
                
            basebands[spw.baseband].append(spw.id)

        children = []
        for baseband_id, spw_ids in basebands.items():
            spws = ','.join([str(i) for i in spw_ids])
            leaf_obj = self.leaf_class(context, result, calto, xaxis, yaxis,
                                       spw=spws, ant=ant, field=field, 
                                       intent=intent, baseband=str(baseband_id),
                                       **kwargs)
            children.append(leaf_obj)

        super(BasebandComposite, self).__init__(children)


class AntComposite(common.LeafComposite):
    """
    Create a PlotLeaf for each antenna in the applied calibration.
    """
    # reference to the PlotLeaf class to call
    leaf_class = None

    def __init__(self, context, result, calto, xaxis, yaxis, spw='', field='',
                 intent='', **kwargs):
        ms = context.observing_run.get_ms(calto.vis)
        antennas = [int(a.id) for a in ms.get_antenna(calto.antenna)]
        
        children = [self.leaf_class(context, result, calto, xaxis, yaxis,
                                    ant=ant, spw=spw, field=field,
                                    intent=intent, **kwargs)
                    for ant in antennas]
        super(AntComposite, self).__init__(children)


class FieldComposite(common.LeafComposite):
    """
    Create a PlotLeaf for each field in the applied calibration.
    """
    # reference to the PlotLeaf class to call
    leaf_class = None

    def __init__(self, context, result, calto, xaxis, yaxis, spw='', ant='',
                 intent='', **kwargs):
        ms = context.observing_run.get_ms(calto.vis)
        
        children = []
        for field in ms.get_fields(calto.field):
            wanted = set(intent.split(','))
            intersection = field.intents.intersection(wanted)
            if not intersection:
                continue

            leaf_obj = self.leaf_class(context, result, calto, xaxis, yaxis,
                                        ant=ant, spw=spw, 
                                        intent=','.join(intersection), 
                                        field=str(field.id), **kwargs)
            children.append(leaf_obj)

        super(FieldComposite, self).__init__(children)


class FieldSpwComposite(common.LeafComposite):
    """
    Create a PlotLeaf for each field and spw in the applied calibration
    """
    leaf_class = None

    def __init__(self, context, result, calto, xaxis, yaxis, intent='', **kwargs):
        ms = context.observing_run.get_ms(calto.vis)

        wanted = set(intent.split(','))
        children = []
        for field in ms.get_fields(calto.field):
            intersection = field.intents.intersection(wanted)
            if not intersection:
                continue
                             
            leaf_obj = self.leaf_class(context, result, calto, xaxis, yaxis,
                                        intent=','.join(intersection), 
                                        field=str(field.id), **kwargs)
            children.append(leaf_obj)
        
        super(FieldSpwComposite, self).__init__(children)


class SpwAntComposite(common.LeafComposite):
    """
    Create a PlotLeaf for each spw and antenna in the applied calibration
    """
    leaf_class = None

    def __init__(self, context, result, calto, xaxis, yaxis, intent='', **kwargs):
        ms = context.observing_run.get_ms(calto.vis)

        children = []
        for spw in ms.get_spectral_windows(calto.spw):
            leaf_obj = self.leaf_class(context, result, calto, xaxis, yaxis,
                                       intent=intent, spw=spw.id,
                                       **kwargs)
            children.append(leaf_obj)
        
        super(SpwAntComposite, self).__init__(children)


class FieldSpwAntComposite(common.LeafComposite):
    """
    Create a PlotLeaf for each field, spw and antenna in the applied calibration
    """
    leaf_class = None

    def __init__(self, context, result, calto, xaxis, yaxis, intent='', **kwargs):
        ms = context.observing_run.get_ms(calto.vis)

        wanted = set(intent.split(','))
        children = []
        for field in ms.get_fields(calto.field):
            intersection = field.intents.intersection(wanted)
            if not intersection:
                continue
                             
            leaf_obj = self.leaf_class(context, result, calto, xaxis, yaxis,
                                        intent=','.join(intersection), 
                                        field=str(field.id), **kwargs)
            children.append(leaf_obj)
        
        super(FieldSpwAntComposite, self).__init__(children)


class CalappComposite(common.LeafComposite):
    """
    Create a PlotLeaf for each applied calibration
    """
    leaf_class = None

    def __init__(self, context, result, xaxis, yaxis, ant='', spw='', 
                 field='', intent='', **kwargs):
        children = [self.leaf_class(context, result, calapp.calto, xaxis, 
                                    yaxis, ant=ant, spw=spw, field=field,
                                    intent=intent, **kwargs)
                    for calapp in result.applied]
        super(CalappComposite, self).__init__(children)


class CalappFieldSpwComposite(common.LeafComposite):
    """
    Create a PlotLeaf for each field and spw in the applied calibration
    """
    leaf_class = None

    def __init__(self, context, result, xaxis, yaxis, intent='', **kwargs):
        children = [self.leaf_class(context, result, calapp.calto, xaxis, 
                                    yaxis, intent=intent, **kwargs)
                    for calapp in result.applied]
        super(CalappFieldSpwComposite, self).__init__(children)


class PlotmsAntComposite(AntComposite):
    leaf_class = PlotmsLeaf


class PlotmsSpwComposite(SpwComposite):
    leaf_class = PlotmsLeaf


class PlotmsBasebandComposite(BasebandComposite):
    leaf_class = PlotmsLeaf


class PlotmsFieldComposite(FieldComposite):
    leaf_class = PlotmsLeaf


class PlotmsCalappComposite(CalappComposite):
    leaf_class = PlotmsLeaf


class PlotmsFieldSpwComposite(FieldSpwComposite):
    leaf_class = PlotmsSpwComposite


class PlotmsCalappFieldSpwComposite(CalappFieldSpwComposite):
    leaf_class = PlotmsFieldSpwComposite


class PlotmsSpwAntComposite(SpwAntComposite):
    leaf_class = PlotmsAntComposite


class PlotmsFieldSpwAntComposite(FieldSpwAntComposite):
    leaf_class = PlotmsSpwAntComposite


def _get_summary_args(context, result, intent, **kwargs):
    spw = _get_calapp_arg(result, 'spw')
    field = _get_calapp_arg(result, 'field')
    antenna = _get_calapp_arg(result, 'antenna')
    intent = ','.join(intent)

    vis = set([calapp.vis for calapp in result.applied])
    assert(len(vis) is 1)
    vis = vis.pop()

    wanted = set(intent.split(','))
    fields_with_intent = set()
    for f in context.observing_run.get_ms(vis).get_fields(field):
        intersection = f.intents.intersection(wanted)
        if not intersection:
            continue
        fields_with_intent.add(f.name)
    field = ','.join(fields_with_intent)

    calto = callibrary.CalTo(vis, field, spw, antenna, intent)

    return (calto, intent)

def _get_calapp_arg(result, arg):
    s = set()
    for calapp in result.applied:
        s.update(utils.safe_split(getattr(calapp, arg)))
    return ','.join(s)


class SpwSummaryChart(PlotmsSpwComposite):
    """
    Base class for executing plotms per calibrator field and spw
    """
    def __init__(self, context, result, xaxis, yaxis, intent, **kwargs):
        (calto, intent) = _get_summary_args(context, result, intent)
        LOG.info('%s vs %s plot: %s' % (yaxis, xaxis, calto))
        
        # request plots per spw, overlaying all antennas
        super(SpwSummaryChart, self).__init__(
                context, result, calto, xaxis, yaxis, intent=intent, 
                field=calto.field, **kwargs)


class BasebandSummaryChart(PlotmsBasebandComposite):
    """
    Base class for executing plotms per baseband
    """
    def __init__(self, context, result, xaxis, yaxis, intent, **kwargs):
        (calto, intent) = _get_summary_args(context, result, intent)
        LOG.info('%s vs %s plot: %s' % (yaxis, xaxis, calto))

        if 'field' in kwargs:
            field = kwargs['field']
            del kwargs['field']
            LOG.debug('Override for %s vs %s plot: field=%s' % (yaxis, xaxis, field))
        else:
            field = calto.field

        # request plots per spw, overlaying all antennas
        # if field is specified in kwargs, it will override the calto.field
        # selection
        super(BasebandSummaryChart, self).__init__(
                context, result, calto, xaxis, yaxis, intent=intent, 
                field=field, **kwargs)


class AmpVsUVSummaryChart(SpwSummaryChart):
    """
    Create an amplitude vs UV distance plot for each spw, overplotting by antenna.
    """
    def __init__(self, context, result, intent='', ydatacolumn='corrected'):
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'avgtime'     : '',
                     'avgscan'     : False,
                     'avgbaseline' : False,
                     'plotrange'   : [0, 0, 0, 0],
                     'coloraxis'   : 'corr',
                     'overwrite'   : True}
        
        super(AmpVsUVSummaryChart, self).__init__(
                context, result, xaxis='uvdist', yaxis='amp', intent=intent, 
                **plot_args)


class AmpVsUVBasebandSummaryChart(BasebandSummaryChart):
    """
    Create an amplitude vs UV distance plot for each spw, overplotting by antenna.
    """
    def __init__(self, context, result, intent='', ydatacolumn='corrected',
                 **kwargs):
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'avgtime'     : '',
                     'avgscan'     : False,
                     'avgbaseline' : False,
                     'plotrange'   : [0, 0, 0, 0],
                     'coloraxis'   : 'corr',
                     'overwrite'   : True}
        plot_args.update(kwargs)

        super(AmpVsUVBasebandSummaryChart, self).__init__(
                context, result, xaxis='uvdist', yaxis='amp', intent=intent, 
                **plot_args)


class PhaseVsUVSummaryChart(SpwSummaryChart):
    """
    Create an amplitude vs UV distance plot for each spw, overplotting by antenna.
    """
    def __init__(self, context, result, intent='', ydatacolumn='corrected'):
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'avgtime'     : '',
                     'avgscan'     : False,
                     'avgbaseline' : False,
                     'coloraxis'   : 'corr',
                     'plotrange'   : [0, 0, -90, 90],
                     'overwrite'   : True}
        
        super(PhaseVsUVSummaryChart, self).__init__(
                context, result, xaxis='uvdist', yaxis='phase', intent=intent, 
                **plot_args)


class AmpVsTimeSummaryChart(SpwSummaryChart):
    """
    Create an amplitude vs time plot for each spw, overplotting by antenna.
    """
    def __init__(self, context, result, intent='', ydatacolumn='corrected'):
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'avgtime'     : '',
                     'avgscan'     : False,
                     'avgbaseline' : False,
                     'plotrange'   : [0, 0, 0, 0],
                     'coloraxis'   : 'field',
                     'overwrite'   : True}
        
        super(AmpVsTimeSummaryChart, self).__init__(
                context, result, xaxis='time', yaxis='amp', intent=intent,
                **plot_args)


class PhaseVsTimeSummaryChart(SpwSummaryChart):
    """
    Create an amplitude vs time plot for each spw, overplotting by antenna.
    """
    def __init__(self, context, result, intent='', ydatacolumn='corrected'):
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'avgtime'     : '',
                     'avgscan'     : False,
                     'avgbaseline' : False,
                     'coloraxis'   : 'field',
                     'plotrange'   : [0, 0, -90, 90],
                     'overwrite'   : True}
        
        super(PhaseVsTimeSummaryChart, self).__init__(
                context, result, xaxis='time', yaxis='phase', intent=intent,
                **plot_args)


class AmpVsFrequencySummaryChart(BasebandSummaryChart):
    """
    Create an amplitude vs time plot for each spw, overplotting by antenna.
    """
    def __init__(self, context, result, intent='', ydatacolumn='corrected', 
                 **kwargs):
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'avgchannel'  : '',
                     'avgtime'     : '1e8',
                     'avgscan'     : True,
                     'avgantenna'  : True,
                     'plotrange'   : [0, 0, 0, 0],
                     'correlation' : '',
                     'coloraxis'   : 'antenna1',
                     'overwrite'   : True}
        plot_args.update(kwargs)
        
        super(AmpVsFrequencySummaryChart, self).__init__(
                context, result, xaxis='freq', yaxis='amp', intent=intent, 
                **plot_args)


class PhaseVsFrequencySummaryChart(BasebandSummaryChart):
    """
    Create an amplitude vs time plot for each spw, overplotting by antenna.
    """
    def __init__(self, context, result, intent='', ydatacolumn='corrected',
                 **kwargs):
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'avgchannel'  : '',
                     'avgtime'     : '1e8',
                     'avgscan'     : True,
                     'avgantenna'  : True,
                     'plotrange'   : [0, 0, -90, 90],
                     'correlation' : '',
                     'coloraxis'   : 'antenna1',
                     'overwrite'   : True}
        plot_args.update(kwargs)
        
        super(PhaseVsFrequencySummaryChart, self).__init__(
                context, result, xaxis='freq', yaxis='phase', intent=intent, 
                **plot_args)


class SpwAntDetailChart(PlotmsSpwAntComposite):
    """
    Base class for executing plotms per calibrator field and spw
    """
    def __init__(self, context, result, xaxis, yaxis, intent, **kwargs):
        (calto, intent) = _get_summary_args(context, result, intent)
        LOG.info('%s vs %s plot: %s' % (yaxis, xaxis, calto))

        # request plots per spw, overlaying all antennas
        super(SpwAntDetailChart, self).__init__(
                context, result, calto, xaxis, yaxis, intent=intent, 
                field=calto.field, **kwargs)


class FieldSpwAntDetailChart(PlotmsFieldSpwAntComposite):
    """
    Base class for executing plotms per calibrator field, spw and antenna
    """
    def __init__(self, context, result, xaxis, yaxis, intent, **kwargs):
        (calto, intent) = _get_summary_args(context, result, intent)
        LOG.info('%s vs %s plot: %s' % (yaxis, xaxis, calto))

        # request plots per spw, overlaying all antennas
        super(FieldSpwAntDetailChart, self).__init__(
                context, result, calto, xaxis, yaxis, intent=intent, 
                **kwargs)


class AmpVsFrequencyDetailChart(FieldSpwAntDetailChart):
    """
    Create an amplitude vs frequency plot for each spw and antenna, 
    overplotting by field.
    """
    def __init__(self, context, result, intent='', ydatacolumn='corrected', 
                 **overrides):
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'avgchannel'  : '',
                     'avgtime'     : '1e8',
                     'avgscan'     : True,
                     'avgantenna'  : False,
                     'plotrange'   : [0, 0, 0, 0],
                     'coloraxis'   : 'corr',
                     'overwrite'   : True}
        plot_args.update(**overrides)
        
        super(AmpVsFrequencyDetailChart, self).__init__(
                context, result, xaxis='freq', yaxis='amp', intent=intent, 
                **plot_args)


class PhaseVsFrequencyDetailChart(FieldSpwAntDetailChart):
    """
    Create a phase vs frequency plot for each spw and antenna, overplotting
    by field.
    """
    def __init__(self, context, result, intent='', ydatacolumn='corrected',
                 **overrides):
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'avgchannel'  : '',
                     'avgtime'     : '1e8',
                     'avgscan'     : True,
                     'avgantenna'  : False,
                     'plotrange'   : [0, 0, -180, 180],
                     'coloraxis'   : 'corr',
                     'overwrite'   : True}
        plot_args.update(**overrides)
        
        super(PhaseVsFrequencyDetailChart, self).__init__(
                context, result, xaxis='freq', yaxis='phase', intent=intent, 
                **plot_args)


class AmpVsUVDetailChart(SpwAntDetailChart):
    """
    Create an amplitude vs UV distance plot for each spw and antenna
    """
    def __init__(self, context, result, intent='', ydatacolumn='corrected'):
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'avgtime'     : '',
                     'avgscan'     : False,
                     'avgbaseline' : False,
                     'plotrange'   : [0, 0, 0, 0],
                     'coloraxis'   : 'corr',
                     'overwrite'   : True}
        
        super(AmpVsUVDetailChart, self).__init__(
                context, result, xaxis='uvdist', yaxis='amp', intent=intent, 
                **plot_args)


class PhaseVsUVDetailChart(SpwAntDetailChart):
    """
    Create an amplitude vs UV distance plot for each spw and antenna
    """
    def __init__(self, context, result, intent='', ydatacolumn='corrected'):
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'avgtime'     : '',
                     'avgscan'     : False,
                     'avgbaseline' : False,
                     'coloraxis'   : 'corr',
                     'plotrange'   : [0, 0, -180, 180],
                     'overwrite'   : True}
        
        super(PhaseVsUVDetailChart, self).__init__(
                context, result, xaxis='uvdist', yaxis='phase', intent=intent, 
                **plot_args)


class AmpVsTimeDetailChart(FieldSpwAntDetailChart):
    """
    Create an amplitude vs time plot for each field, spw and antenna, 
    overplotting by correlation.
    """
    def __init__(self, context, result, intent='', ydatacolumn='corrected'):
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'avgtime'     : '',
                     'avgscan'     : False,
                     'avgantenna'  : False,
                     'plotrange'   : [0, 0, 0, 0],
                     'coloraxis'   : 'corr',
                     'overwrite'   : True}
        
        super(AmpVsTimeDetailChart, self).__init__(
                context, result, xaxis='time', yaxis='amp', intent=intent, 
                **plot_args)


class PhaseVsTimeDetailChart(FieldSpwAntDetailChart):
    """
    Create an phase vs time plot for each field, spw and antenna, 
    overplotting by correlation.
    """
    def __init__(self, context, result, intent='', ydatacolumn='corrected'):
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'avgtime'     : '',
                     'avgscan'     : False,
                     'avgantenna'  : False,
                     'plotrange'   : [0, 0, -180, 180],
                     'coloraxis'   : 'corr',
                     'overwrite'   : True}
        
        super(PhaseVsTimeDetailChart, self).__init__(
                context, result, xaxis='time', yaxis='phase', intent=intent, 
                **plot_args)
