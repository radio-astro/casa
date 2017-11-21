from __future__ import absolute_import

import types

import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.vdp as vdp
from pipeline.h.heuristics import caltable as caltable_heuristic
from pipeline.hifv.heuristics import find_EVLA_band
from pipeline.infrastructure import casa_tasks
from . import resultobjects

LOG = infrastructure.get_logger(__name__)


def _find_spw(vis, bands, context):
    """Identify spw information
    """

    with casatools.TableReader(vis+'/SPECTRAL_WINDOW') as table:
        channels = table.getcol('NUM_CHAN')
        originalBBClist = table.getcol('BBC_NO')
        spw_bandwidths = table.getcol('TOTAL_BANDWIDTH')
        reference_frequencies = table.getcol('REF_FREQUENCY')
    
    center_frequencies = map(lambda rf, spwbw: rf + spwbw/2, reference_frequencies, spw_bandwidths)
    
    if bands == []:
        bands = map(find_EVLA_band, center_frequencies)
    
    unique_bands = list(numpy.unique(bands))
    
    numSpws = len(channels)

    unique_bands_string = ','.join(unique_bands)
    
    all_spws_list = range(numSpws)
    
    all_spws = ','.join(map(str,all_spws_list))
    
    return all_spws, center_frequencies


class OpcalInputs(vdp.StandardInputs):
    spw = vdp.VisDependentProperty(default='')

    @vdp.VisDependentProperty
    def caltable(self):
        namer = caltable_heuristic.OpCaltable()
        casa_args = self._get_task_args(ignore=('caltable',))
        return namer.calculate(output_dir=self.output_dir, stage=self.context.stage, **casa_args)

    @vdp.VisDependentProperty
    def parameter(self):
        return []

    def __init__(self, context, output_dir=None, vis=None, caltable=None, parameter=None, spw=None):
        self.context = context
        self.output_dir = output_dir
        self.vis = vis
        self.spw = spw
        self.parameter = parameter
        self.caltable = caltable

    def to_casa_args(self):
        args = super(OpcalInputs, self).to_casa_args()
        args['caltype'] = 'opac'
        return args


class Opcal(basetask.StandardTaskTemplate):
    Inputs = OpcalInputs    

    def prepare(self):
        inputs = self.inputs
        
        context = self.inputs.context
        
        m = self.inputs.context.observing_run.get_ms(self.inputs.vis)
        # spw2band = context.evla['msinfo'][m.name].spw2band
        spw2band = m.get_vla_spw2band()
        bands = spw2band.values()
        
        # with casatools.MSReader(inputs.vis) as ms:
        #     ms_summary = ms.summary()
        
        # startdate = ms_summary['BeginTime']

        seasonal_weight = 1.0

        try:
            with casatools.TableReader(self.inputs.vis + '/WEATHER') as table:
                numRows = table.nrows()
                if numRows == 0:
                    LOG.warn("Weather station broken during this period, using 100% seasonal model for calculating the zenith opacity")
                    seasonal_weight = 1.0
                else:
                    LOG.info("Using seasonal_weight of 0.5")  # Standard value to use
                    seasonal_weight = 0.5
        except:
            LOG.warn("Unable to open MS weather table.  Using 100% seasonal model for calculating the zenith opacity")

        '''
        if (((startdate >= 55918.80) and (startdate <= 55938.98)) or ((startdate >= 56253.6) and (startdate <= 56271.6))):
            LOG.warn("Weather station broken during this period, using 100% seasonal model for calculating the zenith opacity")
            seasonal_weight=1.0            
        else:
            LOG.info("Using seasonal_weight of 0.5")
            seasonal_weight=0.5
        '''

        plotweather_args = {'vis' : inputs.vis, 'seasonal_weight': seasonal_weight, 'doPlot' : True}
        plotweather_job = casa_tasks.plotweather(**plotweather_args)
        opacities = self._executor.execute(plotweather_job)
 
        inputs.parameter = opacities
        
        inputs.spw, center_frequencies = _find_spw(inputs.vis, bands, context)

        gencal_args = inputs.to_casa_args()
        gencal_job = casa_tasks.gencal(**gencal_args)
        self._executor.execute(gencal_job)

        callist = []
        calto = callibrary.CalTo(vis=inputs.vis)
        calfrom = callibrary.CalFrom(gencal_args['caltable'], caltype='opac', interp='', calwt=False)
        calapp = callibrary.CalApplication(calto, calfrom)
        callist.append(calapp)

        return resultobjects.OpcalResults(pool=callist, opacities=opacities, spw=inputs.spw,
                                          center_frequencies=center_frequencies, seasonal_weight=seasonal_weight)

    def analyse(self, result):
        # With no best caltable to find, our task is simply to set the one
        # caltable as the best result

        # double-check that the caltable was actually generated
        on_disk = [ca for ca in result.pool
                   if ca.exists() or self._executor._dry_run]
        result.final[:] = on_disk

        missing = [ca for ca in result.pool
                   if ca not in on_disk and not self._executor._dry_run]
        result.error.clear()
        result.error.update(missing)

        return result
