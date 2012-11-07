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

def sdreduce(infile, antenna, fluxunit, telescopeparm, specunit, restfreq, frame, doppler, calmode, fraction, noff, width, elongated, markonly, plotpointings, scanlist, field, iflist, pollist, channelrange, average, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, kernel, kwidth, chanwidth, masklist, maskmode, thresh, avg_limit, edge, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, verifycal, verifysm, verifybl, verbosebl, bloutput, blformat, showprogress, minnrow, outfile, outform, overwrite, plotlevel):

    casalog.origin('sdreduce')
    
    ###
    ### Now the actual task code
    ###
    
    try:
        worker = sdreduce_worker(**locals())
        worker.initialize()
        worker.execute()
        worker.finalize()

    except Exception, instance:
        sdutil.process_exception(instance)
        raise Exception, instance

class sdreduce_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdreduce_worker,self).__init__(**kwargs)
        self.suffix = '_cal'

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
        task_sdcal.prior_plot(self.scan, self.plotlevel)
        self.scan = task_sdcal.docalibration(self.scan, self.calmode,
                                             self.fraction,
                                             self.noff, self.width,
                                             self.elongated,
                                             self.markonly, self.plotpointings,
                                             self.verifycal)

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
        task_sdcal.posterior_plot(self.scan, self.project, self.plotlevel)

        # smoothing stage
        casalog.post( "" )
        casalog.post( "*** sdsmooth stage ***" )
        if self.kernel != 'none':
            task_sdsmooth.prior_plot(self.scan, self.project, self.plotlevel)
            task_sdsmooth.dosmooth(self.scan, self.kernel, self.kwidth,
                                   self.chanwidth, self.verifysm)
            task_sdsmooth.posterior_plot(self.scan, self.project, self.plotlevel)
        else:
            casalog.post( "No smoothing was applied..." )

            
        # baseline stage
        casalog.post( "" )
        casalog.post( "*** sdbaseline stage ***")
        if self.blfunc != 'none':
            task_sdbaseline.prior_plot(self.scan, self.plotlevel)
            blfile = task_sdbaseline.init_blfile(self.scan, self.infile, self.project, self.masklist, self.maskmode, self.thresh, self.avg_limit, self.edge, self.blfunc, self.order, self.npiece, self.applyfft, self.fftmethod, self.fftthresh, self.addwn, self.rejwn, self.clipthresh, self.clipniter, self.bloutput, self.blformat)
            task_sdbaseline.dobaseline(self.scan, blfile, self.masklist, self.maskmode, self.thresh, self.avg_limit, self.edge, self.blfunc, self.order, self.npiece, self.applyfft, self.fftmethod, self.fftthresh, self.addwn, self.rejwn, self.clipthresh, self.clipniter, self.verifybl, self.verbosebl, self.blformat, self.showprogress, self.minnrow)
            task_sdbaseline.posterior_plot(self.scan, self.project, self.plotlevel)
        else:
            casalog.post( "No baseline subtraction was applied..." )

    def save(self):
        # write result on disk
        sdutil.save(self.scan, self.project, self.outform, self.overwrite)

    def cleanup(self):
        # restore
        if self.restorer:
            self.restorer.restore()
