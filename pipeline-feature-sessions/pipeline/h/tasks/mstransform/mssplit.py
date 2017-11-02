from __future__ import absolute_import

import types
import os
import shutil

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.callibrary as callibrary
import pipeline.infrastructure.tablereader as tablereader
from pipeline.infrastructure import casa_tasks

LOG = infrastructure.get_logger(__name__)

# Define the minimum set of parameters required to split out
# the requested data (defined by field, spw, intent) from the
# original MS and optionaly average in time and channel.  
# If replace us True replace the parameter MS with the transformed
# one on disk and in the context

class MsSplitInputs(basetask.StandardInputs):

    def __init__(self, context, output_dir=None, vis=None,
        outputvis=None, field=None, intent=None, spw=None,
        datacolumn=None, chanbin=None, timebin=None, replace=None):

        # set the properties to the values given as input arguments
        self._init_properties(vars())

    # Revisit this later
    #    For now use a simple default naming convention
    #    Borrow the output file name list handling technique from
    #    the hif_flagdata interface
    @property
    def outputvis(self):
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('outputvis')

        if not self._outputvis:
            vis_root = os.path.splitext(self.vis)[0]
            return vis_root + '_split.ms'

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

    # Select all fields by default
    @property
    def field(self):
        # If field was explicitly set, return that value
        if self._field is not None:
            return self._field

        # If invoked with multiple mses, return a list of fields
        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('field')

        return ''

    @field.setter
    def field(self, value):
        self._field = value

    # Select all intents by default
    #    Pipeline intents must be stored here 
    @property
    def intent(self):
        return self._intent

    @intent.setter
    def intent(self, value):
        if value is None:
            value = ''
        self._intent = value

    # Select all spws by default
    @property
    def spw(self):
        if self._spw is not None:
            return self._spw

        if type(self.vis) is types.ListType:
            return self._handle_multiple_vis('spw')

        return ''

    @spw.setter
    def spw(self, value):
        self._spw = value

    # Select the data column by default 
    @property
    def datacolumn(self):
        return self._datacolumn

    @datacolumn.setter
    def datacolumn(self, value):
        if value is None:
            value = 'data'
        self._datacolumn = value

    # Select the spectral binning factor
    #    This factor is passed directly to CASA
    @property
    def chanbin(self):
        return self._chanbin

    @chanbin.setter
    def chanbin(self, value):
        if value is None:
            value = 1
        self._chanbin = value

    # Select the time averaging factor
    #    This factor is passed directly to CASA
    @property
    def timebin(self):
        return self._timebin

    @timebin.setter
    def timebin(self, value):
        if value is None:
            value = '0s'
        self._timebin = value

    # Define the replace parameter
    @property
    def replace (self):
        return self._replace

    @replace.setter
    def replace(self, value):
        if value is None:
            value = True
        self._replace = value

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

        # Force the the reindex parameter to False. 
        # This option may no be needed here.
        #d['reindex'] = False

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
            #shutil.move (result.outputvis, result.vis)
            #result.outputvis = result.vis

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
        #if self.vis == self.outputvis:
        # The parent MS has been removed.
        if not os.path.exists(self.vis):
            for index, ms in enumerate(target.get_measurement_sets()):
                #if ms.name == self.outputvis:
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
