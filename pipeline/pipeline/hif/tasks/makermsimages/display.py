from __future__ import absolute_import
import collections
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools

from pipeline.infrastructure import displays as displays

LOG = infrastructure.get_logger(__name__)

# class used to transfer image statistics through to plotting routines
ImageStats = collections.namedtuple('ImageStats', 'rms max')


class RmsimagesSummary(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        # self.image_stats = image_stats

    def plot(self):
        stage_dir = os.path.join(self.context.report_dir,
                                 'stage%d' % self.result.stage_number)
        if not os.path.exists(stage_dir):
            os.mkdir(stage_dir)

        LOG.info("Making PNG RMS images for weblog")
        plot_wrappers = []

        for rmsimagename in self.result.rmsimagenames:
            plot_wrappers.append(displays.SkyDisplay().plot(self.context, rmsimagename,
                                                            reportdir=stage_dir, intent='',
                                                            collapseFunction='mean'))

            with casatools.ImageReader(rmsimagename) as image:
                stats = image.statistics(robust=True)
                self.result.max = stats.get('max')[0]
                self.result.min = stats.get('min')[0]
                self.result.mean = stats.get('mean')[0]
                self.result.median = stats.get('median')[0]
                self.result.sigma = stats.get('sigma')[0]
                self.result.MADrms = stats.get('medabsdevmed')[0] * 1.4826  # see CAS-9631

        return [p for p in plot_wrappers if p is not None]