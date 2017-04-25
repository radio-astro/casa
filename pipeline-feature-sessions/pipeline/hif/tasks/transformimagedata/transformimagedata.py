from __future__ import absolute_import
import os


import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
from pipeline.h.tasks.mstransform import mssplit

LOG = infrastructure.get_logger(__name__)


class TransformimagedataResults(mssplit.MsSplitResults):
    def __init__(self, vis, outputvis):
        self.vis = vis
        self.outputvis = outputvis
        self.ms = None
        self.qa = None
        self.pool = None


class TransformimagedataInputs(mssplit.MsSplitInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, output_dir=None, vis=None,
                 outputvis=None, field=None, intent=None, spw=None,
                 datacolumn=None, chanbin=None, timebin=None, replace=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

    replace = basetask.property_with_default('replace', False)
    datacolumn = basetask.property_with_default('datacolumn', 'data')


class Transformimagedata(mssplit.MsSplit):
    Inputs = TransformimagedataInputs

    def prepare(self):

        inputs = self.inputs

        # Test whether or not a split has been requested
        """
        if inputs.field == '' and inputs.spw == '' and inputs.intent == '' and \
            inputs.chanbin == 1 and inputs.timebin == '0s':
            result = TransformimagedataResults(vis=inputs.vis, outputvis=inputs.outputvis)
            LOG.warning('Output MS equals input MS %s' % (os.path.basename(inputs.vis)))
            return
        """

        # Split is required so create the results structure
        result = mssplit.MsSplitResults(vis=inputs.vis, outputvis=inputs.outputvis)

        # Run CASA task
        #    Does this need a try / except block

        visfields = []
        for imageparam in inputs.context.clean_list_pending:
            visfields.extend(imageparam['field'].split(','))
        visfields = set(visfields)
        visfields = list(visfields)
        visfields = ','.join(visfields)

        mstransform_args = inputs.to_casa_args()
        mstransform_args['field'] = visfields
        mstransform_args['reindex'] = False
        mstransform_job = casa_tasks.mstransform(**mstransform_args)

        self._executor.execute(mstransform_job)

        return result



