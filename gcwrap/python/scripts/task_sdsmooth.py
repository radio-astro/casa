import os
from taskinit import *

import asap as sd
from asap._asap import Scantable
from asap import _to_list
from asap.scantable import is_scantable
import sdutil

def sdsmooth(infile, antenna, scanaverage, scanlist, field, iflist, pollist, kernel, kwidth, chanwidth, verify, outfile, outform, overwrite, plotlevel):

    casalog.origin('sdsmooth')
    
    try:
        worker = sdsmooth_worker(**locals())
        worker.initialize()
        worker.execute()
        worker.finalize()
        
    except Exception, instance:
        sdutil.process_exception(instance)
        raise Exception, instance

class sdsmooth_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdsmooth_worker,self).__init__(**kwargs)
        self.suffix = '_sm'

    def initialize_scan(self):
        sorg=sd.scantable(self.infile,average=False,antenna=self.antenna)
        if not (isinstance(sorg,Scantable)):
            raise Exception, 'infile=%s is not found' % self.infile

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

    def parameter_check(self):
        if self.kernel=='' or self.kernel=='none':
            errstr = "kernel need to be specified"
            raise Exception, errstr
        elif self.kernel!='hanning' and self.kwidth<=0:
            errstr = "kernel should be > 0"
            raise Exception, errstr
            
    def execute(self):
        prior_plot(self.scan, self.project, self.plotlevel)
            
        # set various attributes to self.scan
        self.set_to_scan()

        #Average within each scan
        if self.scanaverage:
            self.scan = sdutil.doaverage(self.scan,
                                         self.scanaverage,
                                         True,
                                         'tint',
                                         false,
                                         'none')

        # Actual implementation is defined outside the class
        # since those are used in task_sdreduce.
        dosmooth(self.scan, self.kernel, self.kwidth,
                 self.chanwidth, self.verify)

        posterior_plot(self.scan, self.project, self.plotlevel)

    def save(self):
        sdutil.save(self.scan, self.project, self.outform, self.overwrite)


def dosmooth(s, kernel, kwidth, chanwidth, verify):
    if kernel == 'regrid':
        if not qa.isquantity(chanwidth):
            errstr = "Invalid quantity chanwidth "+chanwidth
            raise Exception, errstr
        qchw = qa.quantity(chanwidth)
        oldUnit = s.get_unit()
        if qchw['unit'] in ("", "channel", "pixel"):
            s.set_unit("channel")
        elif qa.compare(chanwidth,"1Hz") or \
                 qa.compare(chanwidth,"1m/s"):
            s.set_unit(qchw['unit'])
        else:
            errstr = "Invalid dimension of quantity chanwidth "+chanwidth
            raise Exception, errstr
        casalog.post( "Regridding spectra in width "+chanwidth )
        s.regrid_channel(width=qchw['value'],plot=verify,insitu=True)
        s.set_unit(oldUnit)
    else:
        casalog.post( "Smoothing spectra with kernel "+kernel )
        s.smooth(kernel=kernel,width=kwidth,plot=verify,insitu=True)

def prior_plot(s, project, plotlevel):
    if ( abs(plotlevel) > 1 ):
        # print summary of input data
        casalog.post( "Initial Scantable:" )
        #casalog.post( s._summary() )
        s._summary()
        
    if ( abs(plotlevel) > 0 ):
        pltfile=project+'_rawspec.eps'
        sdutil.plot_scantable(s, pltfile, plotlevel, 'Raw spectra')

def posterior_plot(s, project, plotlevel):
    if ( abs(plotlevel) > 0 ):
        pltfile=project+'_smspec.eps'
        sdutil.plot_scantable(s, pltfile, plotlevel, 'Smoothed spectra')
