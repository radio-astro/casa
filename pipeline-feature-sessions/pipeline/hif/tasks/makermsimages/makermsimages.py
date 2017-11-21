from __future__ import absolute_import
import os
import glob

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.imagelibrary as imagelibrary

LOG = infrastructure.get_logger(__name__)


class MakermsimagesResults(basetask.Results):
    def __init__(self, final=None, pool=None, preceding=None, rmsimagelist=None, rmsimagenames=None):
        super(MakermsimagesResults, self).__init__()

        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []
        if rmsimagelist is None:
            rmsimagelist = []
        if rmsimagenames is None:
            rmsimagenames = []

        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.rmsimagelist = rmsimagelist[:]
        self.rmsimagenames = rmsimagenames[:]

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """

        # if not self.rmsimagenames:
        #     LOG.warn('No makermsimages results')
        #     return

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
                    print('ADDED IMAGE ITEM')
                    context.rmsimlist.add_item(imageitem)
            except:
                pass

    def __repr__(self):
        return 'MakermsimagesResults:'


class MakermsimagesInputs(basetask.StandardInputs):
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

        # tt0 images only
        imagenames = [im for im in imagenames if 'pbcor.tt0' in im]

        imagenames = [im for im in imagenames if '.rms' not in im]
        rmsimagenames = []

        for imagename in imagenames:
            if not os.path.exists(imagename + '.rms') and 'residual' not in imagename:
                rmsimagename = imagename + '.rms'
                LOG.info("Imagename: " + rmsimagename)
                _ = self._do_imdev(imagename)
                rmsimagenames.append(rmsimagename)

        LOG.info("RMS IMAGE NAMES:" + ','.join(rmsimagenames))

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
