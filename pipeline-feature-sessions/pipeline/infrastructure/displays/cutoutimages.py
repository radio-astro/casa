from __future__ import absolute_import
import collections
import os

import numpy as np

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.displays as displays
import pipeline.infrastructure.casatools as casatools

LOG = infrastructure.get_logger(__name__)

# class used to transfer image statistics through to plotting routines
ImageStats = collections.namedtuple('ImageStats', 'rms max')


class CutoutimagesSummary(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        # self.image_stats = image_stats

    def plot(self):
        stage_dir = os.path.join(self.context.report_dir,
                                 'stage%d' % self.result.stage_number)
        if not os.path.exists(stage_dir):
            os.mkdir(stage_dir)

        LOG.info("Making PNG cutout images for weblog")
        plot_wrappers = []

        for subimagename in self.result.subimagenames:
            if 'rms.subim' in subimagename:
                with casatools.ImageReader(subimagename) as image:
                    stats = image.statistics(robust=True)
                    self.result.RMSmax = stats.get('max')[0]
                    self.result.RMSmedian = stats.get('median')[0]
                    arr = image.getchunk()
                    # get fraction of pixels <= 120 micro Jy VLASS technical goal.  ignore 0 (masked) values.
                    self.result.RMSfraction120 = (np.count_nonzero((arr != 0) & (arr <= 120e-6)) / float(arr.size)) * 100
                    # get fraction of pixels <= 168 micro Jy VLASS SE goal.  ignore 0 (masked) values.
                    self.result.RMSfraction168 = (np.count_nonzero((arr != 0) & (arr <= 168e-6)) / float(arr.size)) * 100
                    # get fraction of pixels <= 200 micro Jy VLASS technical requirement.  ignore 0 (masked) values.
                    self.result.RMSfraction200 = (np.count_nonzero((arr != 0) & (arr <= 200e-6)) / float(arr.size)) * 100

        for subimagename in self.result.subimagenames:
            if '.psf.tt' in subimagename:
                plot_wrappers.append(displays.SkyDisplay().plot(self.context, subimagename,
                                                                reportdir=stage_dir, intent='',
                                                                collapseFunction='mean',
                                                                vmin=-0.1, vmax=0.3))
            elif 'image.pbcor.tt0.subim' in subimagename:
                plot_wrappers.append(displays.SkyDisplay().plot(self.context, subimagename,
                                                                reportdir=stage_dir, intent='',
                                                                collapseFunction='mean',
                                                                vmin=-5 * self.result.RMSmedian,
                                                                vmax=20 * self.result.RMSmedian))
            else:
                plot_wrappers.append(displays.SkyDisplay().plot(self.context, subimagename,
                                                                reportdir=stage_dir, intent='',
                                                                collapseFunction='mean'))

        return [p for p in plot_wrappers if p is not None]