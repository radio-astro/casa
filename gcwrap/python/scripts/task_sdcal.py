import os
from taskinit import *

import asap as sd
from asap._asap import Scantable
import pylab as pl
from asap import _to_list
from asap.scantable import is_scantable
import sdutil

def sdcal(infile, antenna, fluxunit, telescopeparm, specunit, frame, doppler, calmode, fraction, noff, width, elongated, markonly, plotpointings, scanlist, field, iflist, pollist, channelrange, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, tau, verify, outfile, outform, overwrite, plotlevel):
    
    casalog.origin('sdcal')
    
    ###
    ### Now the actual task code
    ###
    
    restorer = None
    
    try:
        worker = sdcal_worker(**locals())
        worker.initialize()
        worker.execute()
        worker.finalize()
        
    except Exception, instance:
        sdutil.process_exception(instance)
        raise Exception, instance

class sdcal_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdcal_worker,self).__init__(**kwargs)
        self.suffix = '_cal'

    def initialize_scan(self):
        sorg=sd.scantable(self.infile,average=False,antenna=self.antenna)

        if not isinstance(sorg,Scantable):
            raise Exception, 'Scantable data %s, is not found'

        # A scantable selection
        sel = self.get_selector()
        sorg.set_selection(sel)
        
        # Copy scantable when usign disk storage not to modify
        # the original table.
        if is_scantable(self.infile) and \
               sd.rcParams['scantable.storage'] == 'disk':
            self.scan = sorg.copy()
        else:
            self.scan = sorg
        del sorg

    def execute(self):
        prior_plot(self.scan, self.plotlevel)
                
        # set various attributes to self.scan
        self.set_to_scan()

##         # do opacity (atmospheric optical depth) correction
##         sdutil.doopacity(self.scan, self.tau)

##         # channel splitting
##         sdutil.dochannelrange(self.scan, self.channelrange)

        # Actual implementation is defined outside the class
        # since those are used in task_sdreduce.
        self.scan = docalibration(self.scan, self.calmode, self.fraction,
                                  self.noff, self.width, self.elongated,
                                  self.markonly, self.plotpointings,
                                  self.verify)
        
        # do opacity (atmospheric optical depth) correction
        sdutil.doopacity(self.scan, self.tau)

        # channel splitting
        sdutil.dochannelrange(self.scan, self.channelrange)

        # Average data if necessary
        self.scan = sdutil.doaverage(self.scan, self.scanaverage, self.timeaverage, self.tweight, self.polaverage, self.pweight, self.averageall)

        posterior_plot(self.scan, self.project, self.plotlevel)

    def save(self):
        sdutil.save(self.scan, self.project, self.outform, self.overwrite)
        

def docalibration(s, calmode, fraction, noff, width, elongated,
                  markonly, plotpointings, verify=False):
    scanns = s.getscannos()
    sn=list(scanns)
    casalog.post( "Number of scans to be processed: %d" % (len(sn)) )
    if calmode == 'otf' or calmode=='otfraster':
        s2 = _mark( s,
                    (calmode=='otfraster'),
                    fraction=fraction,
                    npts=noff,
                    width=width,
                    elongated=elongated,
                    plot=plotpointings )
        if markonly:
            scal = s2
        else:
            scal = sd.asapmath.calibrate( s2,
                                          scannos=sn,
                                          calmode='ps',
                                          verify=verify )
    else:
        scal = sd.asapmath.calibrate( s,
                                      scannos=sn,
                                      calmode=calmode,
                                      verify=verify )
    return scal

def prior_plot(s, plotlevel):
    if ( abs(plotlevel) > 1 ):
        # print summary of input data
        #print "Initial Raw Scantable:"
        #print s
        casalog.post( "Initial Raw Scantable:" )
        #casalog.post( s._summary() )
        s._summary()

def posterior_plot(s, project, plotlevel):
    if ( abs(plotlevel) > 1 ):
        # print summary of calibrated data
        #print "Final Calibrated Scantable:"
        #print spave
        casalog.post( "Final Calibrated Scantable:" )
        #casalog.post( spave._summary() )
        s._summary()

    # Plot final spectrum
    if ( abs(plotlevel) > 0 ):
        pltfile = project + '_calspec.eps'
        sdutil.plot_scantable(s, pltfile, plotlevel)

def _mark(s, israster, *args, **kwargs):
        marker = sd.edgemarker( israster=israster )
        marker.setdata( s )
        marker.setoption( *args, **kwargs )
        marker.mark()
        if kwargs.has_key('plot') and kwargs['plot']:
                marker.plot()
        return marker.getresult()
