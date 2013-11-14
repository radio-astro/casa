from __future__ import absolute_import
import types
import math

import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
import pipeline.domain.measures as measures
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.utils as utils
from pipeline.hif.tasks.common import commonfluxresults
from pipeline.hif.heuristics import fieldnames

import itertools
import numpy

from pipeline.hif.tasks import gaincal
from pipeline.hif.tasks import bandpass
from pipeline.hif.tasks import applycal
from pipeline.hifv.heuristics import getCalFlaggedSoln, getBCalStatistics
from pipeline.hifv.tasks.setmodel.setmodel import find_standards, standard_sources
from . import vlasetjy
import pipeline.hif.heuristics.findrefant as findrefant

from pipeline.hifv.tasks.vlautils import VLAUtils




LOG = infrastructure.get_logger(__name__)

class FluxgainsInputs(basetask.StandardInputs):
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())


class FluxgainsResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(FluxgainsResults, self).__init__()

        self.vis = None
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

        
    def merge_with_context(self, context):    
        m = context.observing_run.measurement_sets[0]


class Fluxgains(basetask.StandardTaskTemplate):
    Inputs = FluxgainsInputs

    def prepare(self):

        calMs = 'calibrators.ms'

        LOG.info("Setting models for standard primary calibrators")

        standard_source_names, standard_source_fields = standard_sources(calMs)

        context = self.inputs.context
        m = context.observing_run.measurement_sets[0]
        field_spws = context.evla['msinfo'][m.name].field_spws
        new_gain_solint1 = context.evla['msinfo'][m.name].new_gain_solint1
        gain_solint2 = context.evla['msinfo'][m.name].gain_solint2

        #Look in spectral window domain object as this information already exists!
        with casatools.TableReader(self.inputs.vis+'/SPECTRAL_WINDOW') as table:
            channels = table.getcol('NUM_CHAN')
            originalBBClist = table.getcol('BBC_NO')
            spw_bandwidths = table.getcol('TOTAL_BANDWIDTH')
            reference_frequencies = table.getcol('REF_FREQUENCY')
    
        center_frequencies = map(lambda rf, spwbw: rf + spwbw/2, reference_frequencies, spw_bandwidths)

        vlainputs = VLAUtils.Inputs(context)
        vlautils = VLAUtils(vlainputs)
        
        for i, fields in enumerate(standard_source_fields):
            for myfield in fields:
                spws = field_spws[myfield]
                #spws = [1,2,3]
                for myspw in spws:
                    reference_frequency = center_frequencies[myspw]
                    EVLA_band = vlautils.find_EVLA_band(reference_frequency)
                    LOG.info("Center freq for spw "+str(myspw)+" = "+str(reference_frequency)+", observing band = "+EVLA_band)
                    
                    model_image = standard_source_names[i] + '_' + EVLA_band + '.im'

                    LOG.info("Setting model for field "+str(myfield)+" spw "+str(myspw)+" using "+model_image)

                    #Double check, but the fluxdensity=-1 should not matter since
                    #  the model image take precedence
                    try:
                        setjy_result = self._do_setjy(calMs, str(myfield), str(myspw), model_image, -1)
                        #result.measurements.update(setjy_result.measurements)
                    except Exception, e:
                        # something has gone wrong, return an empty result
                        LOG.error('Unable to complete flux scaling operation for field '+str(myfield)+', spw '+str(myspw))
                        LOG.exception(e)
        
        LOG.info("Making gain tables for flux density bootstrapping")
        LOG.info("Short solint = " + new_gain_solint1)
        LOG.info("Long solint = " + gain_solint2)
        
        refantfield = context.evla['msinfo'][m.name].calibrator_field_select_string
        refantobj = findrefant.RefAntHeuristics(vis='calibrators.ms',field=refantfield,geometry=True,flagging=True, intent='', spw='')
        
        RefAntOutput=refantobj.calculate()
        
        refAnt=str(RefAntOutput[0])+','+str(RefAntOutput[1])+','+str(RefAntOutput[2])+','+str(RefAntOutput[3])
                        
        LOG.info("The pipeline will use antenna(s) "+refAnt+" as the reference")
       
        gaincal_result = self._do_gaincal(context, calMs, 'fluxphaseshortgaincal.g', 'p', [''], solint=new_gain_solint1, minsnr=3.0, refAnt=refAnt)
        
        gaincal_result = self._do_gaincal(context, calMs, 'fluxgaincal.g', 'ap', ['fluxphaseshortgaincal.g'], solint=gain_solint2, minsnr=5.0, refAnt=refAnt)
        
        LOG.info("Gain table fluxgaincal.g is ready for flagging")
        
        return FluxgainsResults()

    def analyse(self, result):
        return result
    
    def _do_setjy(self, calMs, field, spw, modimage, fluxdensity):
        
        task_args = {'vis'            : calMs,
                     'field'          : field,
                     'spw'            : spw,
                     'selectdata'     : False,
                     'modimage'       : modimage,
                     'listmodels'     : False,
                     'scalebychan'    : True,
                     'fluxdensity'    : -1,
                     'standard'       : 'Perley-Butler 2010',
                     'usescratch'     : False,
                     'async'          : False}
        
        job = casa_tasks.setjy(**task_args)
            
        return self._executor.execute(job)
    
    
    def _do_gaincal(self, context, calMs, caltable, calmode, gaintablelist, solint='int', minsnr=3.0, refAnt=None):
        
        m = context.observing_run.measurement_sets[0]
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal
        
        #Do this to get the reference antenna string
        #temp_inputs = gaincal.GTypeGaincal.Inputs(context)
        #refant = temp_inputs.refant.lower()
        
        task_args = {'vis'            : calMs,
                     'caltable'       : caltable,
                     'field'          : '',
                     'spw'            : '',
                     'intent'         : '',
                     'selectdata'     : False,
                     'solint'         : solint,
                     'combine'        : 'scan',
                     'preavg'         : -1.0,
                     'refant'         : refAnt.lower(),
                     'minblperant'    : minBL_for_cal,
                     'minsnr'         : minsnr,
                     'solnorm'        : False,
                     'gaintype'       : 'G',
                     'smodel'         : [],
                     'calmode'        : calmode,
                     'append'         : False,
                     'gaintable'      : gaintablelist,
                     'gainfield'      : [''],
                     'interp'         : [''],
                     'spwmap'         : [],
                     'parang'         : False,
                     'async'          : False}
        
        job = casa_tasks.gaincal(**task_args)
            
        return self._executor.execute(job)
        
        
