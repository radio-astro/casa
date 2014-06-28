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

LOG = infrastructure.get_logger(__name__)


class fluxbootSummaryChart(object):
    def __init__(self, context, result):
        self.context = context
        self.result = result
        self.ms = context.observing_run.get_ms(result.inputs['vis'])
        #self.caltable = result.final[0].gaintable

    def plot(self):
        ##science_spws = self.ms.get_spectral_windows(science_windows_only=True)
        plots = [self.get_plot_wrapper()]
        return [p for p in plots if p is not None]

    def create_plot(self):
        figfile = self.get_figfile()
        
        context = self.context
        result = self.result
        m = context.observing_run.measurement_sets[0]
        corrstring = context.evla['msinfo'][m.name].corrstring
        calibrator_scan_select_string = context.evla['msinfo'][m.name].calibrator_scan_select_string
        ms_active = m.name

        casa.plotms(vis=ms_active, xaxis='freq', yaxis='amp', ydatacolumn='model', selectdata=True,   scan=calibrator_scan_select_string, correlation=corrstring, averagedata=True, avgtime='1e8s', avgscan=True, transform=False,    extendflag=False, iteraxis='', coloraxis='field', plotrange=[], title='', xlabel='', ylabel='',  showmajorgrid=False, showminorgrid=False,    plotfile=figfile, overwrite=True, clearplots=True)


    def get_figfile(self):
        return os.path.join(self.context.report_dir, 
                            'stage%s' % self.result.stage_number, 
                            'bootstrappedFluxDensities-%s-summary.png' % self.ms.basename)

    def get_plot_wrapper(self):
        figfile = self.get_figfile()
        
        context = self.context
        m = context.observing_run.measurement_sets[0]
       
        wrapper = logger.Plot(figfile,
                          x_axis='freq',
                          y_axis='amp',
                          parameters={'vis'      : self.ms.basename,
                                      'type'     : 'fluxboot',
                                      'spw'      : ''})

        if not os.path.exists(figfile):
            LOG.trace('Plotting model calibrator flux densities. Creating new '
                      'plot.')
            try:
                self.create_plot()
            except Exception as ex:
                LOG.error('Could not create fluxboot plot.')
                LOG.exception(ex)
                return None
            
        return wrapper

