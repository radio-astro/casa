from __future__ import absolute_import

import types
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks

LOG = infrastructure.get_logger(__name__)

# Define the minimum set of parameters required to split out
# the TARGET data from the complete and fully calibrated
# original MS. Other parameters will be added as more
# capabilites are added to hif_mstransform.

class MstransformInputs(basetask.StandardInputs):

    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None,
        outputvis=None, field=None, intent=None, spw=None):

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    # Revisit this later
    #    For now use a default name convention
    #    Borrow the output file name list handling technique from
    #    the hif_flagdata interface
    @property
    def outputvis(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('outputvis')

        if not self._outputvis:
            vis_root = os.path.splitext(self.vis)[0]
            return vis_root + '_target.ms'

        if type(self._outputvis) is types.ListType:
            idx = self._my_vislist.index(self.vis)
            return self._outputvis[idx]

        return self._outputvis

    @outputvis.setter
    def outputvis(self, value):
        if value in (None, ''):
            value = []
        elif type(value) is types.StringType:
            value = list(value.replace('[','').replace(']','').replace("'","").split(','))
        self._outputvis = value


    # Find all the fields with TARGET intent
    #     Put in the code for proper field selection but don't pass this
    #     selection to CASA mstransform until the no redindexing capability
    #     is available at the ms transform user level.
    @property
    def field(self):
        # If field was explicitly set, return that value
        if self._field is not None:
            return self._field

        # If invoked with multiple mses, return a list of fields
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('field')

        # Otherwise return each field in the current ms that has been observed
        # with the desired intent
        fields = self.ms.get_fields(intent=self.intent)
        unique_field_names = set([f.name for f in fields])
        field_ids = set([f.id for f in fields])

        # Fields with different intents may have the same name. Check for this
        # and return the IDs if necessary
        if len(unique_field_names) is len(field_ids):
            return ','.join(unique_field_names)
        else:
            return ','.join([str(i) for i in field_ids])

    @field.setter
    def field(self, value):
        self._field = value

    # Select TARGET data by default
    @property
    def intent(self):
        if self._intent is not None:
            return self._intent
        return None

    @intent.setter
    def intent(self, value):
        if value is None:
            value = 'TARGET'
        self._intent = value

    # Find all the the spws TARGET intent. These may be a subset of the
    # science windows which included calibration spws.
    #     Put in the code for proper spw selection but don't pass this
    #     selection to CASA mstransform until the no redindexing capability
    #     is available at the ms transform user level.
    @property
    def spw(self):
        if self._spw is not None:
            return self._spw

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('spw')

        science_target_intents = set (self.intent.split(','))
        science_target_spws = []

        science_spws = [spw for spw in self.ms.get_spectral_windows(self._spw)]
        for spw in science_spws:
            if spw.intents.intersection(science_target_intents):
                science_target_spws.append(spw)
        return ','.join([str(spw.id) for spw in science_target_spws])

    @spw.setter
    def spw(self, value):
        self._spw = value

    def to_casa_args(self):
        # Override the field and spw selection for the
        # time being.
        d = super(MstransformInputs, self).to_casa_args()

        # Filter out field and spw for now and use defaults for
        # now. Note that the trailing , is required
        for ignore in ('field', 'spw', ):
            if ignore in d:
                del d[ignore]

        return d

class Mstransform(basetask.StandardTaskTemplate):
    Inputs = MstransformInputs

    def prepare(self):
        inputs = self.inputs

        # Create results object
        result = MstransformResults (vis=inputs.vis,
            outputvis=inputs.outputvis)

        # Check whether or not there is any target data
        #    If not don't execute

        #  Run task
        mstransform_args = inputs.to_casa_args()
        mstransform_job = casa_tasks.mstransform(**mstransform_args)
        self._executor.execute(mstransform_job)

        return result

    def analyse (self, result):

        # Check for existence of the output vis
        # and set it to '' if it does not exist
        # on disk.
        if not os.path.exists(result.outputvis):
            result.outputvis = ''
        return result

class MstransformResults(basetask.Results):
    def __init__(self, vis, outputvis=''):
        super(MstransformResults, self).__init__()

        self.vis = vis
        self.outputvis = outputvis

    def merge_with_context(self, context):

        # Check for an output vis
        if not self.outputvis:
            LOG.error('No results to merge')
            return

        # TBD
        # Import the transformed target MS into
        # the pipeline context using TBD light
        # weight import data


    def __repr__(self):
        # Format the Mstransform results.
        s = 'MstransformResults:\n'
        s += '\tOriginal MS {vis} transformed to {outputvis}\n'.format(
            vis=os.path.basename(self.vis),
            outputvis=os.path.basename(self.outputvis))

        return s



