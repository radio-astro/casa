from __future__ import absolute_import

import types
import os
import shutil

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.tablereader as tablereader
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.vdp as vdp

LOG = infrastructure.get_logger(__name__)

__all__ = [
    'MsSplitInputs',
    'MsSplit',
    'MsSplitResults'
]


# Define the minimum set of parameters required to split out
# the requested data (defined by field, spw, intent) from the
# original MS and optionally average in time and channel.  
#
# If replace is True replace the parameter MS with the transformed
# one on disk and in the context

class MsSplitInputs(vdp.StandardInputs):

    # Revisit this naming convention later
    #    For the time being use a simple default naming convention
    #    Borrow the output file name list handling technique from
    #    the hif_flagdata interface

    @vdp.VisDependentProperty
    def outputvis(self):
        vis_root = os.path.splitext(self.vis)[0]
        return vis_root + '_split.ms'

    @outputvis.convert
    def outputvis(self, value):
        if isinstance(value, str):
            return list(value.replace('[', '').replace(']', '').replace("'", "").split(','))
        else:
            return value

    @vdp.VisDependentProperty
    def field(self):
        return ''

    @field.convert
    def field(self, value):
        return value

    @vdp.VisDependentProperty
    def intent(self):
        return ''

    @intent.convert
    def intent(self, value):
        return value

    @vdp.VisDependentProperty
    def spw(self):
        return ''

    @spw.convert
    def spw(self, value):
        return value

    datacolumn = vdp.VisDependentProperty(default='data')
    chanbin = vdp.VisDependentProperty(default=1)
    timebin = vdp.VisDependentProperty(default='0s')
    replace = vdp.VisDependentProperty(default=True)

    def __init__(self, context, vis=None, output_dir=None, outputvis=None,
        field=None, intent=None, spw=None, datacolumn=None, chanbin=None,
        timebin=None, replace=None):

        super(MsSplitInputs, self).__init__()

        self.context = context
        self.vis = vis
        self.output_dir = output_dir
        self.outputvis = outputvis
        self.field = field
        self.intent = intent
        self.spw = spw
        self.datacolumn = datacolumn
        self.chanbin = chanbin
        self.timebin = timebin
        self.replace = replace

    def to_casa_args(self):
        d = super(MsSplitInputs, self).to_casa_args()

        # Filter out unwanted parameters
        #     Note that the trailing , is required
        #     Leave the commented out code in place
        #     as an example.
        for ignore in ('replace', ):
            if ignore in d:
                del d[ignore]

        if d['chanbin'] > 1:
            d['chanaverage'] = True
        if d['timebin'] != '0s':
            d['timeaverage'] = True

        return d


class MsSplit(basetask.StandardTaskTemplate):
    Inputs = MsSplitInputs

    def prepare(self):

        inputs = self.inputs

        # Test whether or not a split has been requested
        if inputs.field == '' and inputs.spw == '' and inputs.intent == '' and \
            inputs.chanbin == 1 and inputs.timebin == '0s':
            result = MsSplitResults(vis=inputs.vis, outputvis=inputs.outputvis)
            LOG.warning('Output MS equals input MS %s' % (os.path.basename(inputs.vis)))
            return

        # Split is required so create the results structure
        result = MsSplitResults(vis=inputs.vis, outputvis=inputs.outputvis)

        # Run CASA task
        #    Does this need a try / except block 

        mstransform_args = inputs.to_casa_args()
        mstransform_job = casa_tasks.mstransform(**mstransform_args)
        self._executor.execute(mstransform_job)

        return result

    def analyse(self, result):
        # Check for existence of the output vis. 
        if not os.path.exists(result.outputvis):
            return result

        inputs = self.inputs

        # There seems to be a rerendering issue with replace. Fir now just
        # remove the old file.
        if inputs.replace:
            shutil.rmtree(result.vis)

        # Import the new MS
        to_import = os.path.abspath(result.outputvis)
        observing_run = tablereader.ObservingRunReader.get_observing_run(to_import)

        # Adopt same session as source measurement set
        for ms in observing_run.measurement_sets:
            LOG.debug('Setting session to %s for %s', self.inputs.ms.session, ms.basename)
            ms.session = self.inputs.ms.session

        # Note there will be only 1 MS in the temporary observing run structure
        result.ms = observing_run.measurement_sets[0]

        return result


class MsSplitResults(basetask.Results):
    def __init__(self, vis, outputvis):
        super(MsSplitResults, self).__init__()
        self.vis = vis
        self.outputvis = outputvis
        self.ms = None

    def merge_with_context(self, context):
        # Check for an output vis
        if not self.ms:
            LOG.error('No h_mssplit results to merge')
            return

        target = context.observing_run
        parentms = None
        # The parent MS has been removed.
        if not os.path.exists(self.vis):
            for index, ms in enumerate(target.get_measurement_sets()):
                if ms.name == self.vis:
                    parentms = index
                    break

        if self.ms:
            if parentms is not None:
                LOG.info('Replace {} in context'.format(self.ms.name))
                del target.measurement_sets[parentms]
                target.add_measurement_set(self.ms)

            else:
                LOG.info('Adding {} to context'.format(self.ms.name))
                target.add_measurement_set(self.ms)

    def __str__(self):
        # Format the MsSplit results.
        s = 'MsSplitResults:\n'
        s += '\tOriginal MS {vis} transformed to {outputvis}\n'.format(
            vis=os.path.basename(self.vis),
            outputvis=os.path.basename(self.outputvis))

        return s

    def __repr__(self):
        return 'MsSplitResults({}, {})'.format(os.path.basename(self.vis), os.path.basename(self.outputvis))
