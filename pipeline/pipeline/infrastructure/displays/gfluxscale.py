from __future__ import absolute_import
import collections
import os
import string
import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.filenamer as filenamer
import pipeline.infrastructure.renderer.logger as logger
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.casatools as casatools


from pipeline.infrastructure import casa_tasks

from . import common

LOG = infrastructure.get_logger(__name__)




class PlotmsLeaf(object):
    """
    Class to execute plotms and return a plot wrapper. It passes the spw and
    ant arguments through to plotms without further manipulation, creating
    exactly one plot. 
    """
    def __init__(self, context, result, xaxis, yaxis,  
                 spw='', ant='', field='', scan='', intent='', **plot_args):
        self._context = context
        self._result = result

        self._ms = context.observing_run.get_ms(result.inputs['vis'])
        self._vis = result.inputs['vis']

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

        # use field name rather than ID if possible
        if field != '':
            domain_fields = self._ms.get_fields(field)
            idents = [f.name if f.name else f.id for f in domain_fields]
            field = ','.join(idents)
        self._field = field

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
            'field'    : '' if self._field == '' else '-%s' % filenamer.sanitize(self._field.replace(',','_')),
            'intent'   : '' if self._intent == '' else '%s-' % self._intent.replace(',','_')
        }
        
      

        if self._spw == '':
            fileparts['spw'] = ''
        else:
            # format spws for filename sorting
            spws = ['%0.2d' % int(spw) for spw in string.split(str(self._spw), ',')]
            fileparts['spw'] = 'spw%s-' % '_'.join(spws)

        # some filesystems have limits on the length of the filenames. Mosaics
        # can exceed this limit due to including the names of all the field.
        # Truncate over-long field components while keeping them unique by
        # replacing them with the hash of the component  
        if len(fileparts['field']) > 19:
            fileparts['field'] = '-%s' % str(hash(fileparts['field']))
        
        if self._baseband:
            fileparts['spw'] = 'bb%s-' % self._baseband

        png = '{vis}{field}-{spw}{ant}{intent}{y}_vs_{x}.png'.format(**fileparts)


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
            parameters['field'] = self._field

        if self._baseband != '':
            parameters['baseband'] = self._baseband
        
        parameters['ant'] = self._ant

        for attr in ['spw', 'intent', 'scan']:
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






class BasebandComposite(common.LeafComposite):
    """
    Create a PlotLeaf for each baseband in the applied calibration.
    """
    # reference to the PlotLeaf class to call
    leaf_class = None

    def __init__(self, context, result, xaxis, yaxis, ant='', field='',
                 intent='', **kwargs):
        ms = context.observing_run.get_ms(result.inputs['vis'])
        
        basebands = collections.defaultdict(list)
        

        
        spws=[]
        for fieldname in result.measurements.keys():
            for fluxm in result.measurements[fieldname]:
                spws.append(fluxm.spw_id)
        
        spwlist = ','.join([str(i) for i in spws])
        
        for spw in ms.get_spectral_windows(spwlist):
            if intent != '':
                wanted = set(intent.split(','))
                if spw.intents.isdisjoint(wanted):
                    continue
                
            basebands[spw.baseband].append(spw.id)

        children = []
        for baseband_id, spw_ids in basebands.items():
            spws = ','.join([str(i) for i in spw_ids])
            leaf_obj = self.leaf_class(context, result, xaxis, yaxis,
                                       spw=spws, ant=ant, field=field, 
                                       intent=intent, baseband=str(baseband_id),
                                       **kwargs)
            children.append(leaf_obj)

        super(BasebandComposite, self).__init__(children)






class PlotmsBasebandComposite(BasebandComposite):
    leaf_class = PlotmsLeaf




class BasebandSummaryChart(PlotmsBasebandComposite):
    """
    Base class for executing plotms per baseband
    """
    def __init__(self, context, result, xaxis, yaxis, intent, ant, **kwargs):
        LOG.info('%s vs %s plot' % (yaxis, xaxis))

 

        # request plots per spw, overlaying all antennas
        super(BasebandSummaryChart, self).__init__(
                context, result, xaxis, yaxis, intent=intent, ant=ant, **kwargs)
                


class GFluxscaleSummaryChart(BasebandSummaryChart):
    """
    Create an amplitude vs UV distance plot for baseband.
    """
    def __init__(self, context, result, intent='', ydatacolumn='model', ant=''):
    
        #Reference fields
        fields = result.inputs['reference']
    
        #Get uvdist min/max  for horizontal axis
        uvrange = {}
        
        try:
            with casatools.MSReader(result.vis) as msfile:
                uvrange = msfile.range(["uvdist"])
        except:
            LOG.warn("Unable to obtain plotting ranges for gfluxscale uvdist.")
            uvrange["uvdist"] = numpy.array([0,0])
        
        #Get amp min/max   for vertical axis
        try:
            visstat_args = {'vis'          : result.vis,
                            'axis'         : 'amp',
                            'datacolumn'   : ydatacolumn,
                            'useflags'     : False,
                            'field'        : fields}
                        
            task = casa_tasks.visstat(**visstat_args)
            ampstats = task.execute(dry_run=False)
            ampmax = ampstats['MODEL']['max']
            ampmin = ampstats['MODEL']['min']
            amprange = ampmax - ampmin
            pltmin = ampmin - 0.05 * amprange
            pltmax = ampmax + 0.05 * amprange
        except:
            LOG.warn("Unable to obtain plotting ranges for gfluxscale amps.")
            pltmin = 0
            pltmax = 0
    

    
        plot_args = {'ydatacolumn' : ydatacolumn,
                     'field'       : fields,
                     'avgtime'     : '',
                     'avgscan'     : False,
                     'avgbaseline' : False,
                     'antenna'     : ant,
                     'plotrange'   : [uvrange['uvdist'][0], uvrange['uvdist'][1],pltmin,pltmax],
                     'coloraxis'   : 'spw',
                     'overwrite'   : True}
        
        super(GFluxscaleSummaryChart, self).__init__(
                context, result, xaxis='uvdist', yaxis='amp', intent=intent, ant=ant,
                **plot_args)

