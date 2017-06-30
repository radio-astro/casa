from __future__ import absolute_import
import collections
import os

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
            plot_wrappers.append(displays.SkyDisplay().plot(self.context, subimagename,
                                                            reportdir=stage_dir, intent='',
                                                            collapseFunction='mean'))

        return [p for p in plot_wrappers if p is not None]