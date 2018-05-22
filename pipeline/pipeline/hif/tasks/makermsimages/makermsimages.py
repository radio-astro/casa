from __future__ import absolute_import

import glob
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.vdp as vdp
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.imagelibrary as imagelibrary
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry

LOG = infrastructure.get_logger(__name__)


class MakermsimagesResults(basetask.Results):
    def __init__(self, rmsimagelist=None, rmsimagenames=None):
        super(MakermsimagesResults, self).__init__()

        if rmsimagelist is None:
            rmsimagelist = []
        if rmsimagenames is None:
            rmsimagenames = []

        self.rmsimagelist = rmsimagelist[:]
        self.rmsimagenames = rmsimagenames[:]

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """

        # rmsimagelist is a list of dictionaries
        # Use the same format and information from sciimlist, save for the image name and image plot
        for rmsitem in self.rmsimagelist:
            try:
                imageitem = imagelibrary.ImageItem(
                    imagename=rmsitem['imagename'] + '.rms', sourcename=rmsitem['sourcename'],
                    spwlist=rmsitem['spwlist'], specmode=rmsitem['specmode'],
                    sourcetype=rmsitem['sourcetype'],
                    multiterm=rmsitem['multiterm'],
                    imageplot=rmsitem['imageplot'])
                if 'TARGET' in rmsitem['sourcetype']:
                    context.rmsimlist.add_item(imageitem)
            except:
                pass

    def __repr__(self):
        return 'MakermsimagesResults:'


class MakermsimagesInputs(vdp.StandardInputs):
    def __init__(self, context, vis=None):
        super(MakermsimagesInputs, self).__init__()
        # set the properties to the values given as input arguments
        self.context = context
        self.vis = vis


@task_registry.set_equivalent_casa_task('hif_makermsimages')
class Makermsimages(basetask.StandardTaskTemplate):
    Inputs = MakermsimagesInputs

    def prepare(self):

        imlist = self.inputs.context.sciimlist.get_imlist()

        imagenames = []
        for imageitem in imlist:
            if imageitem['multiterm']:
                imagenames.extend(glob.glob(imageitem['imagename'] + '.pbcor.tt0'))
            else:
                imagenames.extend(glob.glob(imageitem['imagename'] + '.pbcor'))

        rmsimagenames = []
        for imagename in imagenames:
            if not os.path.exists(imagename + '.rms') and 'residual' not in imagename:
                rmsimagename = imagename + '.rms'
                LOG.info("Imagename: " + rmsimagename)
                _ = self._do_imdev(imagename)
                rmsimagenames.append(rmsimagename)

        LOG.info("RMS image names:" + ','.join(rmsimagenames))

        return MakermsimagesResults(rmsimagelist=imlist, rmsimagenames=rmsimagenames)

    def analyse(self, results):
        return results

    def _do_imdev(self, imagename):

        # Quicklook parameters
        imdevparams = {'imagename': imagename,
                       'outfile': imagename + '.rms',
                       'region': "",
                       'box': "",
                       'chans': "",
                       'stokes': "",
                       'mask': "",
                       'overwrite': True,
                       'stretch': False,
                       'grid': [10, 10],
                       'anchor': "ref",
                       'xlength': "60arcsec",
                       'ylength': "60arcsec",
                       'interp': "cubic",
                       'stattype': "xmadm",
                       'statalg': "chauvenet",
                       'zscore': -1,
                       'maxiter': -1
                       }

        task = casa_tasks.imdev(**imdevparams)

        return self._executor.execute(task)
