import os
from taskinit import *
import sys
import string
import inspect
from odict import odict

import sdutil
import asap as sd
from asap._asap import Scantable
import pylab as pl
from sdcal import sdcal
from sdsmooth import sdsmooth
from sdbaseline import sdbaseline
import task_sdcal
import task_sdsmooth
import task_sdbaseline

@sdutil.sdtask_decorator
def sdreduce(infile, antenna, fluxunit, telescopeparm, specunit, restfreq, frame, doppler, calmode, fraction, noff, width, elongated, markonly, plotpointings, scanlist, field, iflist, pollist, channelrange, average, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, kernel, kwidth, chanwidth, masklist, maskmode, thresh, avg_limit, edge, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, verifycal, verifysm, verifybl, verbosebl, bloutput, blformat, showprogress, minnrow, outfile, outform, overwrite, plotlevel):
    worker = sdreduce_worker(**locals())
    worker.initialize()
    worker.execute()
    worker.finalize()


class sdreduce_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdreduce_worker,self).__init__(**kwargs)
        self.suffix = '_cal'

    def __del__(self):
        # restore scantable when the instance is deleted
        self.cleanup()

    def initialize_scan(self):
        # instantiate scantable
        self.scan = sd.scantable(self.infile, average=False, antenna=self.antenna)
        
        # restorer
        self.restorer = sdutil.scantable_restore_factory(self.scan,
                                                         self.infile,
                                                         self.fluxunit,
                                                         self.specunit,
                                                         self.frame,
                                                         self.doppler,
                                                         self.restfreq)
        
        # Apply selection
        self.scan.set_selection(self.get_selector())

    def execute(self):
        # calibration stage
        casalog.post( "*** sdcal stage ***" )
        self.verify = self.verifycal
        engine = task_sdcal.sdcal_engine(self)
        engine.initialize()
        engine.execute()
##         self.scan = engine.get_result()
##         task_sdcal.prior_plot(self.scan, self.plotlevel)
##         self.scan = task_sdcal.docalibration(self.scan, self.calmode,
##                                              self.fraction,
##                                              self.noff, self.width,
##                                              self.elongated,
##                                              self.markonly, self.plotpointings,
##                                              self.verifycal)

        # apply input parameters 
        self.set_to_scan()

        # opacity correction
        sdutil.doopacity(self.scan, self.tau)

        # channel splitting
        sdutil.dochannelrange(self.scan, self.channelrange)
        
        # averaging stage
        if self.average:
            self.scan = sdutil.doaverage(self.scan, self.scanaverage,
                                         self.timeaverage, self.tweight,
                                         self.polaverage, self.pweight,
                                         self.averageall)
        else:
            casalog.post( "No averaging was applied..." )
##         task_sdcal.posterior_plot(self.scan, self.project, self.plotlevel)
        engine.finalize()
        del engine
        
        # smoothing stage
        casalog.post( "" )
        casalog.post( "*** sdsmooth stage ***" )
        if self.kernel != 'none':
            self.verify = self.verifysm
            engine = task_sdsmooth.sdsmooth_engine(self)
            engine.initialize()
            engine.execute()
##             self.scan = engine.get_result()
            engine.finalize()
            del engine
        else:
            casalog.post( "No smoothing was applied..." )

            
        # baseline stage
        casalog.post( "" )
        casalog.post( "*** sdbaseline stage ***")
        if self.blfunc != 'none':
            self.verify = self.verifybl
            self.verbose = self.verbosebl
            engine = task_sdbaseline.sdbaseline_engine(self)
            engine.initialize()
            engine.execute()
##             self.scan = engine.get_result()
            engine.finalize()
            del engine
        else:
            casalog.post( "No baseline subtraction was applied..." )

    def save(self):
        # write result on disk
        sdutil.save(self.scan, self.project, self.outform, self.overwrite)

    def cleanup(self):
        # restore
        if self.restorer:
            self.restorer.restore()
