from __future__ import absolute_import

import copy
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.vdp as vdp
from pipeline.h.heuristics import fieldnames as fieldnames
from pipeline.h.tasks.common import commonfluxresults
from pipeline.infrastructure import task_registry
from . import setjy

LOG = infrastructure.get_logger(__name__)


class SetModelsInputs(vdp.StandardInputs):

    @vdp.VisDependentProperty
    def reference(self):
        # this will give something like '0542+3243,0343+242'
        field_fn = fieldnames.IntentFieldnames()
        reference_fields = field_fn.calculate(self.ms, self.refintent)
        # run the answer through a set, just in case there are duplicates
        fields = {s for s in utils.safe_split(reference_fields)}
        return ','.join(fields)

    refintent = vdp.VisDependentProperty(default = 'AMPLITUDE')

    @vdp.VisDependentProperty
    def transfer(self):
        transfer_fn = fieldnames.IntentFieldnames()
        # call the heuristic to get the transfer fields as a string
        transfer_fields = transfer_fn.calculate(self.ms, self.transintent)

        # remove the reference field should it also have been observed with
        # the transfer intent
        transfers = set(self.ms.get_fields(task_arg=transfer_fields))
        references = set(self.ms.get_fields(task_arg=self.reference))
        diff = transfers.difference(references)

        transfer_names = {f.name for f in diff}
        fields_with_name = self.ms.get_fields(name=transfer_names)
        if len(fields_with_name) is not len(diff) or len(diff) is not len(transfer_names):
            return ','.join([str(f.id) for f in diff])
        else:
            return ','.join(transfer_names)

    transintent = vdp.VisDependentProperty(default = 'BANDPASS')

    @vdp.VisDependentProperty
    def reffile(self):
        value = os.path.join(self.context.output_dir, 'flux.csv')
        return value

    normfluxes = vdp.VisDependentProperty(default = True)
    scalebycan = vdp.VisDependentProperty(default = True)

    def __init__(self, context, output_dir=None, vis=None, reference=None,
                 refintent=None, transfer=None, transintent=None,
                 reffile=None, normfluxes=None, scalebychan=None):

        super(SetModelsInputs, self).__init__()
        self.context = context
        self.vis = vis
        self.output_dir = output_dir
        self.reference = reference
        self.refintent = refintent
        self.transfer = transfer
        self.transintent = transintent
        self.reffile = reffile
        self.normfluxes = normfluxes
        self.scalebychan = scalebychan


@task_registry.set_equivalent_casa_task('hif_setmodels')
class SetModels(basetask.StandardTaskTemplate):
    Inputs = SetModelsInputs

    def prepare(self, **parameters):

        # Initialize the result.
        result = commonfluxresults.FluxCalibrationResults(vis=self.inputs.vis)

        # Set reference calibrator models.
        #    These models will be assigned the lookup reference frequency,
        #    Stokes parameters, and spix if they are available. If they are not the
        #    Setjy defaults (spw center frequency, [1.0, 0.0, 0.0, 0.0], 0.0)
        #    will be used
        reference_fields = self.inputs.reference
        reference_intents = self.inputs.refintent
        if reference_fields not in (None, ''):
            refresults = self._do_setjy(
                reference_fields, reference_intents, reffile=self.inputs.reffile,
                normfluxes=False, scalebychan=self.inputs.scalebychan)
            # Add measurements to the results object
            result.measurements.update(copy.deepcopy(refresults.measurements))

        # Set transfer calibrator models.
        #    These models will  be assigned the lookup reference frequency,
        #    Stokes parameters, and spix if they are available. If they are not the
        #    Setjy defaults (spw center frequency, [1.0, 0.0, 0.0, 0.0], 0.0)
        #    will be used . If normfluxes is True then the stokes parameters
        #    will be normalized to a value of 1
        transfer_fields = self.inputs.transfer
        transfer_intents = self.inputs.transintent
        if transfer_fields not in (None, ''):
            transresults = self._do_setjy(
                transfer_fields, transfer_intents, reffile=self.inputs.reffile,
                normfluxes=self.inputs.normfluxes, scalebychan=self.inputs.scalebychan)
            # Add measurements to the results object
            result.measurements.update(copy.deepcopy(transresults.measurements))

        return result

    def analyse(self, result):
        return result

    # Call the Setjy task
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
