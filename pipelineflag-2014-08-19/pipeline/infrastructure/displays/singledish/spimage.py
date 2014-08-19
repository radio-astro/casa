from __future__ import absolute_import

import os
import time
import abc
import pylab as pl

import pipeline.infrastructure as infrastructure
import pipeline.infrastructure.renderer.logger as logger
from . import spectralmap
from . import sparsemap
from . import channelmap
from . import rmsmap
from . import tpimage
from .common import SDImageDisplay, ShowPlot

LOG = infrastructure.get_logger(__name__)
        
class SDSpectralImageDisplay(SDImageDisplay):
    MATPLOTLIB_FIGURE_ID = 8910

    def plot(self):
        if ShowPlot: pl.ion()
        else: pl.ioff()
        pl.figure(self.MATPLOTLIB_FIGURE_ID)
        if ShowPlot: pl.ioff()
        
        self.init()
        
        plot_list = []
        t0 = time.time()
        worker = sparsemap.SDSparseMapDisplay(self.inputs)
        plot_list.extend(worker.plot())
        t1 = time.time()
        worker = channelmap.SDChannelMapDisplay(self.inputs)
        plot_list.extend(worker.plot())
        t2 = time.time()
        worker = spectralmap.SDSpectralMapDisplay(self.inputs)
        plot_list.extend(worker.plot())
        t3 = time.time()
        worker = rmsmap.SDRmsMapDisplay(self.inputs)
        plot_list.extend(worker.plot())
        t4 = time.time()
        worker = tpimage.SDIntegratedImageDisplay(self.inputs)
        plot_list.extend(worker.plot())
        t5 = time.time()
        LOG.debug('sparse_map: elapsed time %s sec'%(t1-t0))
        LOG.debug('channel_map: elapsed time %s sec'%(t2-t1))
        LOG.debug('spectral_map: elapsed time %s sec'%(t3-t2))
        LOG.debug('rms_map: elapsed time %s sec'%(t4-t3))
        LOG.debug('integrated_map: elapsed time %s sec'%(t5-t4))
        return plot_list

