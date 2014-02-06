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

from ...heuristics import fieldnames
import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.utils as utils
from . import setjy

LOG = infrastructure.get_logger(__name__)



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
            return False
        return self._scalebychan

    @scalebychan.setter
    def scalebychan(self, value):
        if value is None:
            value = False
        self._scalebychan = value
        
    def to_casa_args(self):
        raise NotImplementedError


class SetModel(basetask.StandardTaskTemplate):
    Inputs = SetModelInputs

    def prepare(self, **parameters):
        reference_fields = self.inputs.reference
        transfer_fields = self.inputs.transfer

        # Resetting the model flux to 1 acts directly on the data and does
        # not give a mergeable result, hence there's no point capturing it. 
        self._do_setjy(transfer_fields, 1)

        # On the other hand, looking up the reference field flux *does* give a
        # mergeable result, so this we can return.
        return self._do_setjy(reference_fields, -1)

    def analyse(self, result):
        return result

    def _do_setjy(self, field, fluxdensity=None):
        inputs = setjy.Setjy.Inputs(self.inputs.context,
            vis=self.inputs.vis,
            fluxdensity=fluxdensity,
            field=field,
	    intent='')
        task = setjy.Setjy(inputs)
        return self._executor.execute(task, True)
