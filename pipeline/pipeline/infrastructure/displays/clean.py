from __future__ import absolute_import
import os

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.displays as displays

LOG = infrastructure.get_logger(__name__)


class CleanSummary(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result

    def plot(self):
        stage_dir = os.path.join(self.context.report_dir, 
                                 'stage%d' % self.result.stage_number)
        if not os.path.exists(stage_dir):
            os.mkdir(stage_dir)

        LOG.info('Plotting')
        plot_wrappers = []

        # this class can handle a list of results from hif_cleanlist, or a single 
        # result from hif_clean if we make the single result from the latter
        # the member of a list
        if hasattr(self.result, 'results'):
            results = self.result.results
        else:
            results = [self.result]

        for r in results:
            if r.empty():
                continue

            # psf map
            plot_wrappers.append(displays.SkyDisplay().plot(self.context, r.psf,
              reportdir=stage_dir, intent=r.intent))

            # flux map
            plot_wrappers.append(displays.SkyDisplay().plot(self.context,
              r.flux, reportdir=stage_dir, intent=r.intent))

            # image iterations
            iterations = r.iterations.keys()
            iterations.sort()
            for i in iterations:
                # image for this iteration
                plot_wrappers.append(displays.SkyDisplay().plot(self.context,
                  r.iterations[i]['image'], reportdir=stage_dir, intent=r.intent))

                # residual for this iteration
                plot_wrappers.append(displays.SkyDisplay().plot(self.context,
                  r.iterations[i]['residual'], reportdir=stage_dir, intent=r.intent))

                # model for this iteration (currently only last but
                # allow for others in future)
                if r.iterations[i].has_key('model') and \
                  os.path.exists(r.iterations[i]['model']):
                    plot_wrappers.append(displays.SkyDisplay().plot(
                      self.context, r.iterations[i]['model'],
                      reportdir=stage_dir, intent=r.intent))

                # cleanmask for this iteration - not for iter 0
                if i > 0:
                    plot_wrappers.append(displays.SkyDisplay().plot(
                      self.context, r.iterations[i]['cleanmask'],
                      reportdir=stage_dir, intent=r.intent))

        return [p for p in plot_wrappers if p is not None]
