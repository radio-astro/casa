from __future__ import absolute_import
import os
import glob
import math

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
from pipeline.infrastructure import casa_tasks
import pipeline.infrastructure.imagelibrary as imagelibrary

LOG = infrastructure.get_logger(__name__)


class MakecutoutimagesResults(basetask.Results):
    def __init__(self, final=None, pool=None, preceding=None, subimagelist=None, subimagenames=None):
        super(MakecutoutimagesResults, self).__init__()

        if final is None:
            final = []
        if pool is None:
            pool = []
        if preceding is None:
            preceding = []
        if subimagelist is None:
            subimagelist = []
        if subimagenames is None:
            subimagenames = []


        self.pool = pool[:]
        self.final = final[:]
        self.preceding = preceding[:]
        self.error = set()
        self.subimagelist = subimagelist[:]
        self.subimagenames = subimagenames[:]

    def merge_with_context(self, context):
        """
        See :method:`~pipeline.infrastructure.api.Results.merge_with_context`
        """

        # subimagelist is a list of dictionaries
        # Use the same format and information from sciimlist, save for the image name and image plot

        for subitem in self.subimagelist:
            try:
                imageitem = imagelibrary.ImageItem(
                  imagename=subitem['imagename'] + '.sub', sourcename=subitem['sourcename'],
                  spwlist=subitem['spwlist'], specmode=subitem['specmode'],
                  sourcetype=subitem['sourcetype'],
                  multiterm=subitem['multiterm'],
                  imageplot=subitem['imageplot'])
                context.subimlist.add_item(imageitem)
                if 'TARGET' in subitem['sourcetype']:
                    print 'ADDED IMAGE ITEM'
                    context.subimlist.add_item(imageitem)
            except:
                pass

    def __repr__(self):
        #return 'MakesubimagesResults:\n\t{0}'.format(
        #    '\n\t'.join([ms.name for ms in self.mses]))
        return 'MakecutoutimagesResults:'


class MakecutoutimagesInputs(basetask.StandardInputs):
    @basetask.log_equivalent_CASA_call
    def __init__(self, context, vis=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())


class Makecutoutimages(basetask.StandardTaskTemplate):
    Inputs = MakecutoutimagesInputs

    def prepare(self):

        imlist = self.inputs.context.sciimlist.get_imlist()

        imagenames = []
        for imageitem in imlist:
            imagenames.extend(glob.glob(imageitem['imagename'] + '*'))

        # tt0 images only
        imagenames = [im for im in imagenames if 'tt0' in im]

        imagenames = [im for im in imagenames if '.subim' not in im]
        subimagenames = []

        for imagename in imagenames:
            if not os.path.exists(imagename + '.subim'):
                subimagename = imagename + '.subim'
                LOG.info("SubImagename: " + subimagename)
                taskresult = self._do_subim(imagename)
                subimagenames.append(subimagename)

        imlist.sort()
        subimagenames.sort()

        return MakecutoutimagesResults(subimagelist=imlist, subimagenames=subimagenames)

    def analyse(self, results):
        return results

    def _do_imstat(self, imagename):

        task = casa_tasks.imstat(imagename=imagename)

        return self._executor.execute(task)

    def _do_imhead(self, imagename):

        task = casa_tasks.imhead(imagename=imagename)

        return self._executor.execute(task)

    def _do_subim(self, imagename):

        # Get image parameters
        imstat_dict = self._do_imstat(imagename)

        # Get image header
        imhead_dict = self._do_imhead(imagename)

        imsizex = math.fabs(imhead_dict['refpix'][0]*imhead_dict['incr'][0]*(180.0/math.pi)*2)
        imsizey = math.fabs(imhead_dict['refpix'][1]*imhead_dict['incr'][1]*(180.0/math.pi)*2)
        imageSizeX = 1.0  # degrees
        imageSizeY = 1.0  # degrees
        imsize = [3600.0*imsizex, 3600.0*imsizey]

        xcellsize = 3600.0 * (180.0 / math.pi) * math.fabs(imhead_dict['incr'][0])
        ycellsize = 3600.0 * (180.0 / math.pi) * math.fabs(imhead_dict['incr'][1])

        fld_subim_sizeX = int(3600.0 * (imageSizeX + 2.0 / 60.0) / xcellsize)
        fld_subim_sizeY = int(3600.0 * (imageSizeY + 2.0 / 60.0) / ycellsize)

        # equivalent blc,trc for extracting requested field, in pixels:
        blcx = imsize[0] / 2 - (fld_subim_sizeX / 2)
        blcy = imsize[1] / 2 - (fld_subim_sizeY / 2)
        trcx = imsize[0] / 2 + (fld_subim_sizeX / 2) + 1
        trcy = imsize[1] / 2 + (fld_subim_sizeY / 2) + 1
        fld_subim = str(blcx) + ',' + str(blcy) + ',' + str(trcx) + ',' + str(trcy)
        LOG.info('Using field subimage blc,trc of {!s},{!s}, {!s},{!s}'.format(blcx,blcy,trcx,trcy))

        # imsubimage(imagename=clnpbcor, outfile=clnpbcor + '.subim', box=fld_subim)

        # Quicklook parameters
        imsubimageparams = {'imagename' : imagename,
                            'outfile'   : imagename + '.subim',
                            'box'       : fld_subim
                           }

        task = casa_tasks.imsubimage(**imsubimageparams)

        return self._executor.execute(task)
