"""
Example task class session:

sys.path.insert(0, '/home/sjw/local/eclipse/plugins/org.python.pydev.debug_2.5.0.2012040618/pysrc')
sys.path.insert(0, '/home/sjw/alma/cvs/PipelineRefactoring-2012-01-B/PIPELINE/Heuristics/src')
execfile('/home/sjw/alma/cvs/PipelineRefactoring-2012-01-B/PIPELINE/Heuristics/src/pipeline/cli/mytasks.py')
__rethrow_casa_exceptions=False

import pipeline

files=['vla_m81.avg.raw']
dry_run=False

context = pipeline.Pipeline().context
inputs = pipeline.tasks.ImportData.Inputs(context, files=files)
task = pipeline.tasks.ImportData(inputs)
results = task.execute(dry_run=dry_run)
results.accept(context)

inputs = pipeline.tasks.SetModel.Inputs(context)
# inputs.refintent='BANDPASS'
task = pipeline.tasks.SetModel(inputs)
results = task.execute(dry_run=dry_run)
"""
from __future__ import absolute_import
import types
import math

import pipeline.infrastructure.casatools as casatools
import numpy


from pipeline.hif.heuristics import fieldnames
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils
from . import setjy

from pipeline.vla.tasks.vlautils import VLAUtils

LOG = infrastructure.get_logger(__name__)

def _find_standards(positions):
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


def _standard_sources(vis):
    """Get standard source names, fields, and positions - convenience function from
       the original EVLA scripted pipeline
    """

    with casatools.TableReader(vis+'/FIELD') as table:
        field_positions = table.getcol('PHASE_DIR')
        
    positions = []

    for ii in range(0,len(field_positions[0][0])):
        positions.append([field_positions[0][0][ii], field_positions[1][0][ii]])

    standard_source_names = [ '3C48', '3C138', '3C147', '3C286' ]
    standard_source_fields = _find_standards(positions)

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
        standard_source_names, standard_source_fields = _standard_sources(self.inputs.vis)

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
                for myspw in spws:
                    reference_frequency = center_frequencies[myspw]
                    EVLA_band = vlautils.find_EVLA_band(reference_frequency)
                    model_image = standard_source_names[i] + '_' + EVLA_band + '.im'

                    #Double check, but the fluxdensity=-1 should not matter since
                    #  the model image take precedence
                    self._do_setjy(str(myfield), str(myspw), model_image, -1)
                    
       
        return None

    def analyse(self, result):
        return result

    def _do_setjy(self, field, spw, modimage, fluxdensity):
        
        setjyinputs = setjy.Setjy.Inputs(self.inputs.context,
             field=field, spw=spw, modimage=modimage, fluxdensity=fluxdensity)
        setjytask = setjy.Setjy(setjyinputs)
        return self._executor.execute(setjytask, True)
