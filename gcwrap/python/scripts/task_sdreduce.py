from taskinit import casalog

import asap as sd
from asap.scantable import is_scantable
import sdutil

import task_sdcal as task_sdcal
import task_sdaverage as task_sdaverage
import task_sdbaseline as task_sdbaseline

@sdutil.sdtask_decorator
def sdreduce(infile, antenna, fluxunit, telescopeparam, field, spw, restfreq, frame, doppler, timerange, scan, pol, calmode, fraction, noff, width, elongated, markonly, plotpointings, tau, average, timeaverage, tweight, scanaverage, averageall, polaverage, pweight, kernel, kwidth, chanwidth, maskmode, thresh, avg_limit, edge, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, verifycal, verifysm, verifybl, verbosebl, bloutput, blformat, showprogress, minnrow, outfile, outform, overwrite, plotlevel):
    with sdutil.sdtask_manager(sdreduce_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()


class sdreduce_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdreduce_worker,self).__init__(**kwargs)
        self.suffix = '_cal'

    def initialize_scan(self):
        # instantiate scantable
        sorg = sd.scantable(self.infile, average=False, antenna=self.antenna)
        
#         # restorer
#         self.restorer = sdutil.scantable_restore_factory(self.scan,
#                                                          self.infile,
#                                                          self.fluxunit,
#                                                          '', # specunit=''
#                                                          self.frame,
#                                                          self.doppler,
#                                                          self.restfreq)
        
        # Apply selection
        sorg.set_selection(self.get_selector(sorg))
        # Copy scantable when usign disk storage not to modify
        # the original table.
        if is_scantable(self.infile) and self.is_disk_storage:
            self.scan = sorg.copy()
        else:
            self.scan = sorg
        del sorg

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

        ## channel splitting
        #sdutil.dochannelrange(self.scan, self.channelrange)

        #WORKAROUND for new tasks (in future this should be done in sdutil)
        if not self.timeaverage: self.scanaverage = False
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
        if self.kernel.lower() not in ['none', '']:
            self.verify = self.verifysm
            engine = task_sdaverage.sdsmooth_engine(self)
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

#     def cleanup(self):
#         # restore
#         if hasattr(self,'restorer') and self.restorer:
#             self.restorer.restore()
