"""
B. Kent, NRAO, June 2013

Example task class session:

import pipeline

files=['vla_m81.avg.raw']
dry_run=False

context = pipeline.Pipeline().context
inputs = pipeline.vla.tasks.VLAImportData.Inputs(context, files=files)
task = pipeline.vla.tasks.VLAImportData(inputs)
results = task.execute(dry_run=dry_run)
results.accept(context)

inputs = pipeline.vla.tasks.SetModel.Inputs(context)

task = pipeline.tasks.SetModel(inputs)
results = task.execute(dry_run=dry_run)
"""
from __future__ import absolute_import
import types
import math
import pickle

import pipeline.infrastructure.casatools as casatools
import numpy

import itertools

from pipeline.hif.heuristics import fieldnames
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils
from pipeline.hif.tasks.common import commonfluxresults
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.casatools as casatools
from pipeline.hifv.tasks.setmodel import vlasetjy

from pipeline.hifv.tasks.vlautils import VLAUtils

import pipeline.domain.measures as measures
import pipeline.extern.asizeof as asizeof


LOG = infrastructure.get_logger(__name__)

def find_standards(positions):
    """Function for finding standards from the original scripted EVLA pipeline
    """
    # set the max separation as ~1'
    MAX_SEPARATION = 60*2.0e-5
    position_3C48 = casatools.measures.direction('j2000', '1h37m41.299', '33d9m35.133')
    fields_3C48 = []
    position_3C138 = casatools.measures.direction('j2000', '5h21m9.886', '16d38m22.051')
    fields_3C138 = []
    position_3C147 = casatools.measures.direction('j2000', '5h42m36.138', '49d51m7.234')
    fields_3C147 = []
    position_3C286 = casatools.measures.direction('j2000', '13h31m8.288', '30d30m23.959')
    fields_3C286 = []

    for ii in range(0,len(positions)):
        position = casatools.measures.direction('j2000', str(positions[ii][0])+'rad', str(positions[ii][1])+'rad')
        separation = casatools.measures.separation(position,position_3C48)['value'] * math.pi/180.0
        if (separation < MAX_SEPARATION):
            fields_3C48.append(ii)
        else:
            separation = casatools.measures.separation(position,position_3C138)['value'] * math.pi/180.0
            if (separation < MAX_SEPARATION):
                fields_3C138.append(ii)
            else:
                separation = casatools.measures.separation(position,position_3C147)['value'] * math.pi/180.0
                if (separation < MAX_SEPARATION):
                    fields_3C147.append(ii)
                else:
                    separation = casatools.measures.separation(position,position_3C286)['value'] * math.pi/180.0
                    if (separation < MAX_SEPARATION):
                        fields_3C286.append(ii)

    fields = [ fields_3C48, fields_3C138, fields_3C147, fields_3C286 ]
    
    return fields


def standard_sources(vis):
    """Get standard source names, fields, and positions - convenience function from
       the original EVLA scripted pipeline
    """

    with casatools.TableReader(vis+'/FIELD') as table:
        field_positions = table.getcol('PHASE_DIR')
        
    positions = []

    for ii in range(0,len(field_positions[0][0])):
        positions.append([field_positions[0][0][ii], field_positions[1][0][ii]])

    standard_source_names = [ '3C48', '3C138', '3C147', '3C286' ]
    standard_source_fields = find_standards(positions)
    
    standard_source_found = False
    for standard_source_field in standard_source_fields:
        if standard_source_field:
            standard_source_found = True
    if not standard_source_found:
        standard_source_found = False
        LOG.error("ERROR: No standard flux density calibrator observed, flux density scale will be arbitrary")
        QA2_calprep='Fail'

    return standard_source_names, standard_source_fields



class SetModelInputs(basetask.StandardInputs):
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


class SetModel(basetask.StandardTaskTemplate):
    Inputs = SetModelInputs

    def prepare(self, **parameters):
        standard_source_names, standard_source_fields = standard_sources(self.inputs.vis)

        result = commonfluxresults.FluxCalibrationResults(self.inputs.vis)
        
        context = self.inputs.context
        m = context.observing_run.measurement_sets[0]
        field_spws = context.evla['msinfo'][m.name].field_spws
        spw2band = context.evla['msinfo'][m.name].spw2band
        bands = spw2band.values()

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
                #reference_frequency = center_frequencies[myspw]
                
                
                for myspw in spws:
                    reference_frequency = center_frequencies[myspw]
                    try:
                        EVLA_band = spw2band[myspw]
                    except:
                        LOG.info('Unable to get band from spw id - using reference frequency instead')
                        EVLA_band = vlautils.find_EVLA_band(reference_frequency)
                    
                    LOG.info("Center freq for spw "+str(myspw)+" = "+str(reference_frequency)+", observing band = "+EVLA_band)
                    model_image = standard_source_names[i] + '_' + EVLA_band + '.im'

                    #Double check, but the fluxdensity=-1 should not matter since
                    #  the model image take precedence
                    
                    LOG.info("Setting model for field "+str(myfield)+" spw "+str(myspw)+" using "+model_image)
                    
                    try:
                        LOG.info('Context size: %s' % str(measures.FileSize(asizeof.asizeof(context), measures.FileSizeUnits.BYTES)))
                        setjy_result = self._do_setjy(str(myfield), str(myspw), model_image, -1, '')
                        
                    except Exception, e:
                        # something has gone wrong, return an empty result
                        LOG.error('Unable to complete flux scaling operation')
                        LOG.exception(e)
                        #return result
                            
                    try:
                        #Need to add this line back in when calling vla.tasks.setjy
                        #result.measurements[setjy_result[0].measurements.keys()[0]]=setjy_result[0].measurements
                        result.measurements.update(setjy_result[0].measurements)
                        #print setjy_result.__dict__
                    except Exception, e:
                        LOG.error('Unable to add setjy measurements to the result')
                        LOG.exception(e)
                               
        return result

    def analyse(self, result):
        return result

    def _do_setjy(self, field, spw, model, fluxdensity, intent):
        
        '''
        task_args = {'vis'            : self.inputs.vis,
                     'field'          : field,
                     'spw'            : spw,
                     'selectdata'     : False,
                     'modimage'       : modimage,
                     'listmodels'     : False,
                     'scalebychan'    : True,
                     'fluxdensity'    : fluxdensity,
                     'spix'           : 0,
                     'standard'       : 'Perley-Butler 2010',
                     'usescratch'     : False,
                     'async'          : False}
        
        job = casa_tasks.setjy(**task_args)
            
        return self._executor.execute(job)
        '''
        
        setjyinputs = vlasetjy.VLASetjy.Inputs(self.inputs.context,
             field=field, spw=spw, model=model, intent=intent, fluxdensity=fluxdensity)
        setjytask = vlasetjy.VLASetjy(setjyinputs)
        setjy_result = self._executor.execute(setjytask, True)
        
        #pickle.dump(setjy_result, open("setjy_result.p", "wb"))
        
        return setjy_result
