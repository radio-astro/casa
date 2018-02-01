from __future__ import absolute_import

import ast
import glob
import math
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.basetask as basetask
import pipeline.infrastructure.imagelibrary as imagelibrary
import pipeline.infrastructure.vdp as vdp
from pipeline.infrastructure import casa_tasks
from pipeline.infrastructure import task_registry

LOG = infrastructure.get_logger(__name__)


class MakecutoutimagesResults(basetask.Results):
    def __init__(self, final=None, pool=None, preceding=None,
                 subimagelist=None, subimagenames=None, image_size=None):
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
        self.image_size = image_size

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
                if 'TARGET' in subitem['sourcetype']:
                    print('ADDED IMAGE ITEM')
                    context.subimlist.add_item(imageitem)
            except:
                pass

    def __repr__(self):
        return 'MakecutoutimagesResults:'


class MakecutoutimagesInputs(vdp.StandardInputs):
    @vdp.VisDependentProperty
    def offsetblc(self):
        return []   # Units of arcseconds

    @vdp.VisDependentProperty
    def offsettrc(self):
        return []   # Units of arcseconds

    def __init__(self, context, vis=None, offsetblc=None, offsettrc=None):
        super(MakecutoutimagesInputs, self).__init__()
        self.context = context
        self.vis = vis
        self.offsetblc = offsetblc
        self.offsettrc = offsettrc


@task_registry.set_equivalent_casa_task('hif_makecutoutimages')
class Makecutoutimages(basetask.StandardTaskTemplate):
    Inputs = MakecutoutimagesInputs

    def prepare(self):

        imlist = self.inputs.context.sciimlist.get_imlist()
        imagenames = []
        # Per VLASS Tech Specs page 22
        for imageitem in imlist:
            imagenames.extend(glob.glob(imageitem['imagename'].replace('.image', '.image.pbcor') + '*'))
            imagenames.extend(glob.glob(imageitem['imagename'].replace('.image', '.psf') + '*'))
            imagenames.extend(glob.glob(imageitem['imagename'].replace('.image', '.image.residual.pbcor') + '*'))
            imagenames.extend(glob.glob(imageitem['imagename'].replace('.image', '.pb') + '*'))

        # tt0 images only
        imagenames = [im for im in imagenames if 'tt0' in im]

        imagenames = [im for im in imagenames if '.subim' not in im]
        subimagenames = []

        subimage_size = None
        for imagename in imagenames:
            if not os.path.exists(imagename + '.subim'):
                subimagename = imagename + '.subim'
                LOG.info("SubImagename: " + subimagename)
                _, subimage_size = self._do_subim(imagename)
                subimagenames.append(subimagename)

        imlist.sort()
        subimagenames.sort()

        return MakecutoutimagesResults(subimagelist=imlist, subimagenames=subimagenames, image_size=subimage_size)

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
        # imstat_dict = self._do_imstat(imagename)

        # Get image header
        imhead_dict = self._do_imhead(imagename)

        # Read in image size from header
        imsizex = math.fabs(imhead_dict['refpix'][0]*imhead_dict['incr'][0]*(180.0/math.pi)*2)  # degrees
        imsizey = math.fabs(imhead_dict['refpix'][1]*imhead_dict['incr'][1]*(180.0/math.pi)*2)  # degrees

        image_size_x = 1.0  # degrees:  size of cutout
        image_size_y = 1.0  # degrees:  size of cutout

        # If less than or equal to 1 deg + 2 arcminute buffer, use the image size and no buffer
        buffer_deg = 2.0 / 60.0   # Units of degrees
        if imsizex <= (1.0 + buffer_deg):
            image_size_x = imsizex
            image_size_y = imsizey
            buffer_deg = 0.0

        imsize = [imhead_dict['shape'][0], imhead_dict['shape'][1]]  # pixels

        xcellsize = 3600.0 * (180.0 / math.pi) * math.fabs(imhead_dict['incr'][0])
        ycellsize = 3600.0 * (180.0 / math.pi) * math.fabs(imhead_dict['incr'][1])

        fld_subim_size_x = int(3600.0 * (image_size_x + buffer_deg) / xcellsize)   # Cutout size with buffer in pixels
        fld_subim_size_y = int(3600.0 * (image_size_y + buffer_deg) / ycellsize)   # Cutout size with buffer in pixels

        # equivalent blc,trc for extracting requested field, in pixels:
        blcx = imsize[0] / 2 - (fld_subim_size_x / 2)
        blcy = imsize[1] / 2 - (fld_subim_size_y / 2)
        trcx = imsize[0] / 2 + (fld_subim_size_x / 2) + 1
        trcy = imsize[1] / 2 + (fld_subim_size_y / 2) + 1

        if blcx < 0.0:
            blcx = 0
        if blcy < 0.0:
            blcy = 0
        if trcx > imsize[0]:
            trcx = imsize[0] - 1
        if trcy > imsize[1]:
            trcy = imsize[1] - 1

        if inputs.offsetblc and inputs.offsettrc:
            offsetblc = inputs.offsetblc
            offsettrc = inputs.offsettrc
            buffer_deg = 0.0

            if type(offsetblc) is str:
                offsetblc = ast.literal_eval(offsetblc)
            if type(offsettrc) is str:
                offsettrc = ast.literal_eval(offsettrc)

            fld_subim_size_x_blc = int(3600.0 * (offsetblc[0] / 3600.0 + buffer_deg / 2.0) / xcellsize)
            fld_subim_size_y_blc = int(3600.0 * (offsetblc[1] / 3600.0 + buffer_deg / 2.0) / ycellsize)
            fld_subim_size_x_trc = int(3600.0 * (offsettrc[0] / 3600.0 + buffer_deg / 2.0) / xcellsize)
            fld_subim_size_y_trc = int(3600.0 * (offsettrc[1] / 3600.0 + buffer_deg / 2.0) / ycellsize)

            blcx = imsize[0] / 2 - fld_subim_size_x_blc
            blcy = imsize[1] / 2 - fld_subim_size_y_blc
            trcx = imsize[0] / 2 + fld_subim_size_x_trc + 1
            trcy = imsize[1] / 2 + fld_subim_size_y_trc + 1

            if blcx < 0.0:
                blcx = 0
            if blcy < 0.0:
                blcy = 0
            if trcx > imsize[0]:
                trcx = imsize[0] - 1
            if trcy > imsize[1]:
                trcy = imsize[1] - 1

            LOG.info("Using user defined offsets in arcseconds of: blc:({!s}), trc:({!s})".format(
                ','.join([str(i) for i in offsetblc]), ','.join([str(i) for i in offsettrc])))

        fld_subim = str(blcx) + ',' + str(blcy) + ',' + str(trcx) + ',' + str(trcy)
        LOG.info('Using field subimage blc,trc of {!s},{!s}, {!s},{!s}, which includes a buffer '
                 'of {!s} arcminutes.'.format(blcx, blcy, trcx, trcy, buffer_deg*60))

        # imsubimage(imagename=clnpbcor, outfile=clnpbcor + '.subim', box=fld_subim)

        # Quicklook parameters
        imsubimageparams = {'imagename': imagename,
                            'outfile': imagename + '.subim',
                            'box': fld_subim}

        task = casa_tasks.imsubimage(**imsubimageparams)
        subimage_size = {'pixels_x': trcx - blcx,
                         'pixels_y': trcy - blcy,
                         'arcsec_x': (trcx - blcx) * xcellsize,
                         'arcsec_y': (trcy - blcy) * ycellsize}

        return self._executor.execute(task), subimage_size
