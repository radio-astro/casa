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
            if not os.path.exists(figfile):
                LOG.trace('psf plot %s not found. Creating new '
                  'plot.' % os.path.basename(figfile))
                plot = displays.SkyDisplay().plot(self.context, r.psf,
                  reportdir=stage_dir)
                wrapper = logger.Plot(figfile,
                                      x_axis='Right Ascension',
                                      y_axis='Declination',
                                      parameters={'spw'   : r.spw,
                                                  'intent': r.intent,
                                                  'type'  : 'psf'})
                plot_wrappers.append(wrapper)

            # flux map
            figfile = displays.sky.plotfilename(r.flux, stage_dir) 
            if not os.path.exists(figfile):
                LOG.trace('flux plot %s not found. Creating new '
                  'plot.' % os.path.basename(figfile))
                plot = displays.SkyDisplay().plot(self.context, r.flux,
                  reportdir=stage_dir)

                wrapper = logger.Plot(figfile,
                                      x_axis='Right Ascension',
                                      y_axis='Declination',
                                      parameters={'spw'   : r.spw,
                                                  'intent': r.intent,
                                                  'type'  : 'flux'})
                plot_wrappers.append(wrapper)

            # image iterations
            iterations = r.iterations.keys()
            iterations.sort()
            for iter in iterations:
                # image for this iteration
                figfile = displays.sky.plotfilename(
                  r.iterations[iter]['image'], stage_dir) 
                if not os.path.exists(figfile):
                    LOG.trace('image plot %s not found. Creating new '
                      'plot.' % os.path.basename(figfile))
                    plot = displays.SkyDisplay().plot(self.context,
                      r.iterations[iter]['image'], reportdir=stage_dir)

                    wrapper = logger.Plot(figfile,
                                          x_axis='Right Ascension',
                                          y_axis='Declination',
                                          parameters={'spw'   : r.spw,
                                                      'intent': r.intent,
                                                      'iter'  : iter,
                                                      'type'  : 'image'})
                plot_wrappers.append(wrapper)

        return [p for p in plot_wrappers if p is not None]
