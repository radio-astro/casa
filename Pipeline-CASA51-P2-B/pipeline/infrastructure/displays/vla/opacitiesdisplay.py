from __future__ import absolute_import
import collections
import json
import os

import numpy

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.casatools as casatools
import pipeline.infrastructure.renderer.logger as logger
from pipeline.infrastructure import casa_tasks
import casa
import numpy as np
import math
import shutil

LOG = infrastructure.get_logger(__name__)


class opacitiesSummaryChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        #self.caltable = result.final[0].gaintable

    def plot(self):
        plots = []
        context = self.context
        result = self.result
    
        ms_active=self.ms.basename
        
        #create phase time plot for all calibrators
        figfile = self.get_figfile()
                            
        plot = logger.Plot(figfile, x_axis='freq', y_axis='opacity',
                          parameters={'vis'      : self.ms.basename,
                                      'type'     : 'opacities',
                                      'spw'      : ''})

        if not os.path.exists(figfile):
            LOG.trace('Plotting opacities vs. frequency. Copying plot.')
            try:
                origfile = self.ms.basename+'.plotweather.png'
                shutil.copy2(origfile, figfile)
            except Exception as ex:
                LOG.error('Could not copy plotweather plot.')
                LOG.exception(ex)
                plot = None
        
        plots.append(plot)

        return [p for p in plots if p is not None]



    def get_figfile(self):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            self.ms.basename+'.plotweather.png')