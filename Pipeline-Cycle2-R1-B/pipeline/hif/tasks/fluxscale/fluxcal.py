from __future__ import absolute_import
import types

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils
from . import fluxscale
from pipeline.hif.tasks.setmodel import setmodel
from pipeline.hif.tasks.setmodel import setjy

from pipeline.hif.heuristics import fieldnames as fieldnames

LOG = infrastructure.get_logger(__name__)


class FluxcalInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None, output_dir=None, reference=None, 
                 transfer=None, refintent=None, transintent=None):
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

        # call the heuristic to get the transfer fields as a string
        transfer_fields = self._transfer(self.ms, self.transintent)

        # remove the reference field should it also have been observed with
        # the transfer intent
        transfers = set([i for i in utils.safe_split(transfer_fields)])
        references = set([i for i in utils.safe_split(self.reference)])
        return ','.join(transfers.difference(references))

    @transfer.setter
    def transfer(self, value):
        if value is None:
            value = fieldnames.IntentFieldnames()
        self._transfer = value

    @property
    def transintent(self):
        if self._transintent is None:
            return 'PHASE'
        return self._transintent
    
    @transintent.setter
    def transintent(self, value):
        self._transintent = value

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
