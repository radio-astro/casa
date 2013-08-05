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
from pipeline.vla.heuristics import getCalFlaggedSoln, getBCalStatistics
from pipeline.vla.tasks.setmodel.setmodel import find_standards, standard_sources
from . import setjy


from pipeline.vla.tasks.vlautils import VLAUtils




LOG = infrastructure.get_logger(__name__)

class FluxgainsInputs(basetask.StandardInputs):
    def __init__(self, context, output_dir=None, vis=None, reference=None, 
                 refintent=None, transfer=None, scalebychan=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())
        
    @property
    def reference(self):
        if not callable(self._reference):
            return self._reference

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('reference')

        # this will give something like '0542+3243,0343+242'
        reference_fields = self._reference(self.ms, self.refintent)

        # run the answer through a set, just in case there are duplicates
        fields = set()
        fields.update(utils.safe_split(reference_fields))
        
        return ','.join(fields)

    @reference.setter
    def reference(self, value):
        if value is None:
            value = fieldnames.IntentFieldnames()
        self._reference = value

    @property
    def refintent(self):
        if self._refintent is None:
            return 'AMPLITUDE'
        return self._refintent
    
    @refintent.setter
    def refintent(self, value):
        self._refintent = value

    @property
    def transfer(self):
        if not callable(self._transfer):
            return self._transfer

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('transfer')

        # a transfer fields is any field that's not a reference field
        all_fields = set([field.name for field in self.ms.fields])    
        reference_fields = set([i for i in utils.safe_split(self.reference)])                    
        transfer_fields = all_fields.difference(reference_fields)

        return ','.join(set(transfer_fields))

    @transfer.setter
    def transfer(self, value):
        if value is None:
            value = fieldnames.IntentFieldnames()
        self._transfer = value

    @property
    def scalebychan(self):
        if self._scalebychan is None:
            return True
        return self._scalebychan

    @scalebychan.setter
    def scalebychan(self, value):
        if value is None:
            value = True
        self._scalebychan = value
        
    def to_casa_args(self):
        raise NotImplementedError


class Fluxgains(basetask.StandardTaskTemplate):
    Inputs = FluxgainsInputs

    def prepare(self):

        calMs = 'calibrators.ms'

        standard_source_names, standard_source_fields = standard_sources(calMs)

        context = self.inputs.context
        m = context.observing_run.measurement_sets[0]
        field_spws = context.evla['msinfo'][m.name].field_spws

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
                    model_image = standard_source_names[i] + '_' + EVLA_band + '.im'

                    #Double check, but the fluxdensity=-1 should not matter since
                    #  the model image take precedence
                    try:
                        setjy_result = self._do_setjy(calMs, str(myfield), str(myspw), model_image, -1)
                        #result.measurements.update(setjy_result.measurements)
                    except Exception, e:
                        # something has gone wrong, return an empty result
                        LOG.error('Unable to complete flux scaling operation')
                        LOG.exception(e)
                        return setjy_result
       
        gaincal_result = _do_gaincal(context, calMs, 'fluxphaseshortgaincal.g', 'p', [''], solint=new_gain_solint1, minsnr=3.0)
        
        gaincal_result = _do_gaincal(context, calMs, 'fluxgaincal.g', 'ap', ['fluxphaseshortgaincal.g'], solint=gain_solint2, minsnr=5.0)

    def analyse(self, result):
        return result
    
    def _do_setjy(self, calMs, field, spw, modimage, fluxdensity):
        
        task_args = {'vis'            : calMs,
                     'field'          : field,
                     'spw'            : spw,
                     'selectdata'     : False,
                     'modimage'       : model_image,
                     'listmodeimages' : False,
                     'scalebychan'    : True,
                     'fluxdensity'    : -1,
                     'standard'       : 'Perley-Butler 2010',
                     'usescratch'     : False,
                     'async'          : False}
        
        job = casa_tasks.setjy(**task_args)
            
        return self._executor.execute(job)
    
    
    def _do_gaincal(self, context, calMs, caltable, calmode, gaintablelist, solint='int', minsnr=3.0):
        
        m = context.observing_run.measurement_sets[0]
        minBL_for_cal = context.evla['msinfo'][m.name].minBL_for_cal
        
        #Do this to get the reference antenna string
        temp_inputs = gaincal.GTypeGaincal.Inputs(context)
        refant = temp_inputs.refant.lower()
        
        task_args = {'vis'            : calMs,
                     'caltable'       : caltable,
                     'field'          : '',
                     'spw'            : '',
                     'intent'         : '',
                     'selectdata'     : False,
                     'solint'         : solint,
                     'combine'        : 'scan',
                     'preavg'         : -1.0,
                     'refant'         : refant,
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
                     'gaincurve'      : False,
                     'opacity'        : [],
                     'parang'         : False,
                     'async'          : False}
        
        job = casa_tasks.setjy(**task_args)
            
        return self._executor.execute(job)
        
        