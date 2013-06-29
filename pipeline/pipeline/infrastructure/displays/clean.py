from __future__ import absolute_import
import collections
import json
import os

import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.displays as displays
import pipeline.infrastructure.renderer.logger as logger
from pipeline.infrastructure import casa_tasks

LOG = infrastructure.get_logger(__name__)


class CleanSummary(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result

    def plot(self):
        stage_dir = os.path.join(self.context.report_dir, 'stage%d' % (
         self.result[0].stage_number))
        if (not os.path.exists(stage_dir)):
            os.mkdir(stage_dir)

        LOG.info('Plotting')
        plot_wrappers = []
        for r in self.result:
            if r.empty():
                continue

            # psf map
            figfile = displays.sky.plotfilename(r.psf, stage_dir) 
            plot_wrappers.append(displays.SkyDisplay().plot(self.context, r.psf,
              reportdir=stage_dir))

            # flux map
            figfile = displays.sky.plotfilename(r.flux, stage_dir) 
            plot_wrappers.append(displays.SkyDisplay().plot(self.context,
              r.flux, reportdir=stage_dir))

            # image iterations
            iterations = r.iterations.keys()
            iterations.sort()
            for iter in iterations:
                # image for this iteration
                figfile = displays.sky.plotfilename(
                  r.iterations[iter]['image'], stage_dir) 
                plot_wrappers.append(displays.SkyDisplay().plot(self.context,
                  r.iterations[iter]['image'], reportdir=stage_dir))

        return [p for p in plot_wrappers if p is not None]
