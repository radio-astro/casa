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
                  imagename=subitem['imagename'] + '.subim', sourcename=subitem['sourcename'],
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
    def __init__(self, context, vis=None, offsetblc=None, offsettrc=None):
        # set the properties to the values given as input arguments
        self._init_properties(vars())

    offsetblc = basetask.property_with_default('offsetblc', [])  #Units of arcseconds
    offsettrc = basetask.property_with_default('offsettrc', [])  #Units of arcseconds

class Makecutoutimages(basetask.StandardTaskTemplate):
    Inputs = MakecutoutimagesInputs

    def prepare(self):

        imlist = self.inputs.context.sciimlist.get_imlist()
        imagenames = []
        # Per VLASS Tech Specs page 22
        for imageitem in imlist:
            imagenames.extend(glob.glob(imageitem['imagename'].replace('.image', '.image.pbcor') + '*'))
            imagenames.extend(glob.glob(imageitem['imagename'].replace('.image','.psf') + '*'))
            imagenames.extend(glob.glob(imageitem['imagename'].replace('.image','.residual') + '*'))
            imagenames.extend(glob.glob(imageitem['imagename'].replace('.image', '.pb') + '*'))

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

        inputs = self.inputs

        # Get image parameters
        imstat_dict = self._do_imstat(imagename)

        # Get image header
        imhead_dict = self._do_imhead(imagename)

        imsizex = math.fabs(imhead_dict['refpix'][0]*imhead_dict['incr'][0]*(180.0/math.pi)*2)  # degrees
        imsizey = math.fabs(imhead_dict['refpix'][1]*imhead_dict['incr'][1]*(180.0/math.pi)*2)  # degrees
        imageSizeX = 1.0  # degrees:  size of cutout
        imageSizeY = 1.0  # degrees:  size of cutout

        # For testing of small images, if less than 2 deg, make a half image size cutout
        if imsizex < 2.0:
            imageSizeX = 0.5 * imsizex
            imageSizeY = 0.5 * imsizey

        imsize = [3600.0*imsizex, 3600.0*imsizey]

        xcellsize = 3600.0 * (180.0 / math.pi) * math.fabs(imhead_dict['incr'][0])
        ycellsize = 3600.0 * (180.0 / math.pi) * math.fabs(imhead_dict['incr'][1])

        buffer = 2.0 / 60.0

        if imsizex < 2.0:
            buffer = 10.0 / 3600.0

        fld_subim_sizeX = int(3600.0 * (imageSizeX + buffer) / xcellsize)   # Cutout size with buffer in pixels
        fld_subim_sizeY = int(3600.0 * (imageSizeY + buffer) / ycellsize)   # Cutout size with buffer in pixels

        # equivalent blc,trc for extracting requested field, in pixels:
        blcx = imsize[0] / 2 - (fld_subim_sizeX / 2)
        blcy = imsize[1] / 2 - (fld_subim_sizeY / 2)
        trcx = imsize[0] / 2 + (fld_subim_sizeX / 2) + 1
        trcy = imsize[1] / 2 + (fld_subim_sizeY / 2) + 1

        if (inputs.offsetblc and inputs.offsettrc):
            fld_subim_sizeXblc = int(3600.0 * (inputs.offsetblc[0] / 3600.0 + buffer) / xcellsize)
            fld_subim_sizeYblc = int(3600.0 * (inputs.offsetblc[1] / 3600.0 + buffer) / ycellsize)
            fld_subim_sizeXtrc = int(3600.0 * (inputs.offsettrc[0] / 3600.0 + buffer) / xcellsize)
            fld_subim_sizeYtrc = int(3600.0 * (inputs.offsettrc[1] / 3600.0 + buffer) / ycellsize)

            blcx = imsize[0] / 2 - (fld_subim_sizeXblc)
            blcy = imsize[1] / 2 - (fld_subim_sizeYblc)
            trcx = imsize[0] / 2 + (fld_subim_sizeXtrc) + 1
            trcy = imsize[1] / 2 + (fld_subim_sizeYtrc) + 1

            LOG.info("Using offsets: blc:({!s}), trc:({!s})".format(
                ','.join([str(i) for i in inputs.offsetblc]),','.join([str(i) for i in inputs.offsettrc])))

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
