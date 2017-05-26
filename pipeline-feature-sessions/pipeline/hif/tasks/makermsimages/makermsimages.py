from __future__ import absolute_import
import os
import glob


import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks


LOG = infrastructure.get_logger(__name__)


class MakermsimagesResults(basetask.Results):
    def __init__(self, final=[], pool=[], preceding=[]):
        super(MakermsimagesResults, self).__init__()
        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """

        if not self.final:
            LOG.warn('No makermsimages results')
            return

    def __repr__(self):
        #return 'MakermsimagesResults:\n\t{0}'.format(
        #    '\n\t'.join([ms.name for ms in self.mses]))
        return 'MakermsimagesResults:'


class MakermsimagesInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())


class Makermsimages(basetask.StandardTaskTemplate):
    Inputs = MakermsimagesInputs

    def prepare(self):

        imlist = self.inputs.context.sciimlist.get_imlist()

        imagenames = []
        for imageitem in imlist:
            imagenames.extend(glob.glob(imageitem['imagename'] + '*'))

        imagenames = [im for im in imagenames if '.rms' not in im]

        for imagename in imagenames:
            if not os.path.exists(imagename + '.rms'):
                LOG.info("Imagename: " + imagename + '.rms')
                result = self._do_imdev(imagename)

        return MakermsimagesResults()

    def analyse(self, results):
        return results

    def _do_imdev(self, imagename):

        # Quicklook parameters
        imdevparams = {'imagename' : imagename,
                       'outfile'   : imagename + '.rms',
                       'grid'      : [20, 20],
                       'xlength'   : '60pix',
                       'ylength'   : '60pix',
                       'stattype'  : 'rms'}

        task = casa_tasks.imdev(**imdevparams)

        return self._executor.execute(task)
