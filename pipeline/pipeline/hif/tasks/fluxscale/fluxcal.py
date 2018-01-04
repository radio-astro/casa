from __future__ import absolute_import
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils
import pipeline.infrastructure.vdp as vdp

from . import fluxscale
from pipeline.hif.tasks.setmodel import setmodel
from pipeline.hif.tasks.setmodel import setjy
from pipeline.h.heuristics import fieldnames as fieldnames

LOG = infrastructure.get_logger(__name__)

class FluxcalInputs(vdp.StandardInputs):

    @vdp.VisDependentProperty
    def reference(self):
        # this will give something like '0542+3243,0343+242'
        field_fn = fieldnames.IntentFieldnames()
        reference_fields = field_fn.calculate(self.ms, self.refintent)
        # run the answer through a set, just in case there are duplicates
        fields = {s for s in utils.safe_split(reference_fields)}

        return ','.join(fields)

    refintent = vdp.VisDependentProperty(default='AMPLITUDE')

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

    transintent = vdp.VisDependentProperty(default='PHASE,BANDPASS,CHECK')

    def __init__(self, context, vis=None, output_dir=None, reference=None,
                 transfer=None, refintent=None, transintent=None):
         self.context = context
         self.vis = vis
         self.output_dir = output_dir
         self.reference = reference
         self.transfer = transfer
         self.refintent = refintent
         self.transintent = transintent

    def to_casa_args(self):
        raise NotImplementedError

class Fluxcal(basetask.StandardTaskTemplate):
    Inputs = FluxcalInputs

    def prepare(self, **parameters):
        results1 = self._do_setmodel()
        results2 = self._do_fluxscale()
        LOG.info('TODO: average flux across measurement sets?')
        self._do_setjy()

        results1.merge_result(results2)
        return results1

    def analyse(self, result):
        return result

    def _do_setmodel(self):
        inputs = setmodel.SetModel.Inputs(self.inputs.context,
            vis=self.inputs.vis,
            reference=self.inputs.reference,
            transfer=self.inputs.transfer)
        task = setmodel.SetModel(inputs)
        return self._executor.execute(task, True)

    def _do_fluxscale(self):
        inputs = fluxscale.Fluxscale.Inputs(self.inputs.context,
            vis=self.inputs.vis,
            reference=self.inputs.reference,
            transfer=self.inputs.transfer)
        task = fluxscale.Fluxscale(inputs)
        return self._executor.execute(task, True)

    def _do_setjy(self):
        inputs = setjy.Setjy.Inputs(self.inputs.context,
            vis=self.inputs.vis,
            intent=self.inputs.transintent)
        task = setjy.Setjy(inputs)
        return self._executor.execute(task, True)
