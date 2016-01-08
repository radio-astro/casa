from __future__ import absolute_import
import os
import types
import copy

from ...heuristics import fieldnames
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils
from ..common import commonfluxresults
from . import setjy

LOG = infrastructure.get_logger(__name__)


class SetModelsInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None, reference=None, 
                 refintent=None, transfer=None, transintent=None,
                 reffile=None, normfluxes=None, scalebychan=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

    @property
    def reference(self):
        if not callable(self._reference):
            return self._reference

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('reference')

        # this will give something like '0542+3243,0343+242'
        if self.refintent in (None, ''):
            return ''
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
        return self._refintent

    @refintent.setter
    def refintent(self, value):
        if value is None:
            value = 'AMPLITUDE'
        self._refintent = value

    @property
    def transfer(self):
        if not callable(self._transfer):
            return self._transfer

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('transfer')

        # call the heuristic to get the transfer fields as a string
        if self.transintent in (None, ''):
            return ''
        transfer_fields = self._transfer(self.ms, self.transintent)

        # Remove the reference field should it also have been observed with
        # the transfer intent
        transfers = set(self.ms.get_fields(task_arg=transfer_fields))
        references = set(self.ms.get_fields(task_arg=self.reference))
        diff = transfers.difference(references)

        transfer_names = set([f.name for f in diff])
        fields_with_name = self.ms.get_fields(name=transfer_names)
        if len(fields_with_name) is not len(diff) or len(diff) is not len(transfer_names):
            return ','.join([str(f.id) for f in diff])
        else:
            return ','.join(transfer_names)

    @transfer.setter
    def transfer(self, value):
        if value is None:
            value = fieldnames.IntentFieldnames()
        self._transfer = value

    @property
    def transintent(self):
        return self._transintent

    @transintent.setter
    def transintent(self, value):
        if value is None:
            value = 'PHASE,BANDPASS,CHECK'
        self._transintent = value

    @property
    def reffile(self):
        return self._reffile

    @reffile.setter
    def reffile(self, value=None):
        if value in (None, ''):
            value = os.path.join(self.context.output_dir, 'flux.csv')
        self._reffile = value

    @property
    def normfluxes(self):
        return self._normfluxes

    @normfluxes.setter
    def normfluxes(self, value=None):
        if value is None:
            value = True
        self._normfluxes = value

    @property
    def scalebychan(self):
        return self._scalebychan

    @scalebychan.setter
    def scalebychan(self, value):
        if value is None:
            value = True
        self._scalebychan = value
        

class SetModels(basetask.StandardTaskTemplate):
    Inputs = SetModelsInputs

    def prepare(self, **parameters):

         # Initialize the result.
         result = commonfluxresults.FluxCalibrationResults(vis=self.inputs.vis)

         # Set reference calibrator models.
         #    These models will always be assigned the lookup reference frequency,
         #    Stokes parameters, and spix if they are available. If not the 
         #    Setjy defaults (spw center frequency, [1.0, 0.0, 0.0, 0.0], 0.0)
         #    will be assigned.
         reference_fields = self.inputs.reference
         reference_intents = self.inputs.refintent
         if reference_fields not in (None, ''):
            refresults = self._do_setjy(reference_fields, reference_intents, reffile=self.inputs.reffile,
                normfluxes=False, scalebychan=self.inputs.scalebychan)
         print "REFRESULTS"
         print "ref"

         # Set transfer calibrator models.
         #    These models will always be assigned the lookup reference frequency,
         #    Stokes parameters, and spix if they are available. If not the 
         #    Setjy defaults (spw center frequency, [1.0, 0.0, 0.0, 0.0], 0.0)
         #    will be assigned. If normfluxes is True then the stokes parameters
         #    will be normalized to a value of 1
         transfer_fields = self.inputs.transfer
         transfer_intents = self.inputs.transintent
         if transfer_fields not in (None, ''):
             transresults = self._do_setjy(transfer_fields, transfer_intents, reffile=self.inputs.reffile,
                 normfluxes=self.inputs.normfluxes, scalebychan=self.inputs.scalebychan)

         # Construct the results object
         measurements = copy.deepcopy(refresults.measurements)
         measurements.update(copy.deepcopy(transresults.measurements))
         result.measurements = measurements

         return result

    def analyse(self, result):
        return result

    # Call the Setjy task
    #    Note that intent has already been used to compute the appropriate fields
    #    so it is set to ''. Reffile, normfluxes, scalebychan default to the current
    #    Setjy defaults. Do not accept these results into the context.
    def _do_setjy(self, field, intent, reffile=None, normfluxes=None, scalebychan=None):

        task_args = {
            'output_dir'    : self.inputs.output_dir,
            'vis'           : self.inputs.vis,
            'field'         : field,
            'intent'        : intent,
            'fluxdensity'   : -1,
            'reffile'       : reffile,
            'normfluxes'    : normfluxes,
            'scalebychan'   : scalebychan }

        task_inputs = setjy.Setjy.Inputs(self.inputs.context, **task_args)
        task = setjy.Setjy(task_inputs)
        return self._executor.execute(task, merge=False)
