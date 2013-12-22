from taskinit import casalog, qatool

import asap as sd
from asap._asap import Scantable
from asap.scantable import is_scantable
import sdutil

@sdutil.sdtask_decorator
def tsdaverage(infile, antenna, field, spw, scan, pol, scanaverage, timeaverage, tweight, averageall, polaverage, pweight, kernel, kwidth, chanwidth, verify, outfile, outform, overwrite, plotlevel):
    with sdutil.sdtask_manager(sdaverage_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()
        

class sdaverage_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdaverage_worker,self).__init__(**kwargs)
        self.suffix = '_ave'

    def initialize_scan(self):
        sorg=sd.scantable(self.infile,average=False,antenna=self.antenna)
        if not (isinstance(sorg,Scantable)):
            raise Exception, 'infile=%s is not found' % self.infile

        # A scantable selection
        #sel = self.get_selector()
        sel = self.get_selector(sorg)
        sorg.set_selection(sel)

        # Copy scantable when usign disk storage not to modify
        # the original table.
        if is_scantable(self.infile) and self.is_disk_storage:
            self.scan = sorg.copy()
        else:
            self.scan = sorg
        del sorg

    def parameter_check(self):
        doave = (self.scanaverage or self.timeaverage or self.polaverage)
        if not doave and (self.kernel=='' or self.kernel=='none'):
            errstr = "Neither averaging nor smoothing parameter is set. No operation to be done."
            #errstr = "kernel need to be specified"
            raise Exception, errstr
        elif self.kernel!='hanning' and self.kwidth<=0:
            errstr = "kernel should be > 0"
            raise Exception, errstr
    
    def execute(self):
        # set various attributes to self.scan
        self.set_to_scan()

        #Average within each scan
        self.scan = sdutil.doaverage(self.scan, self.scanaverage, self.timeaverage, self.tweight, self.polaverage, self.pweight, self.averageall)

        engine = sdsmooth_engine(self)

        # Smoothing
        if self.kernel not in ['', 'none']:
            engine.initialize()

            # Actual implementation is defined outside the class
            # since those are used in task_sdreduce.
            engine.execute()

            engine.finalize()

    def save(self):
        sdutil.save(self.scan, self.project, self.outform, self.overwrite)


class sdsmooth_engine(sdutil.sdtask_engine):
    def __init__(self, worker):
        super(sdsmooth_engine,self).__init__(worker)

    def initialize(self):
        if ( abs(self.plotlevel) > 1 ):
            # print summary of input data
            casalog.post( "Initial Scantable:" )
            #casalog.post( s._summary() )
            self.worker.scan._summary()

        if ( abs(self.plotlevel) > 0 ):
            pltfile=self.project+'_rawspec.eps'
            sdutil.plot_scantable(self.worker.scan, pltfile, self.plotlevel, 'Raw spectra')

    def execute(self):
        # CAS-5410 Use private tools inside task scripts
        qa = qatool()

        scan = self.worker.scan
        if self.kernel == 'regrid':
            if not qa.isquantity(self.chanwidth):
                errstr = "Invalid quantity chanwidth "+self.chanwidth
                raise Exception, errstr
            qchw = qa.quantity(self.chanwidth)
            oldUnit = scan.get_unit()
            if qchw['unit'] in ("", "channel", "pixel"):
                scan.set_unit("channel")
            elif qa.compare(self.chanwidth,"1Hz") or \
                     qa.compare(self.chanwidth,"1m/s"):
                scan.set_unit(qchw['unit'])
            else:
                errstr = "Invalid dimension of quantity chanwidth "+self.chanwidth
                raise Exception, errstr
            casalog.post( "Regridding spectra in width "+self.chanwidth )
            scan.regrid_channel(width=qchw['value'],plot=self.verify,insitu=True)
            scan.set_unit(oldUnit)
        else:
            casalog.post( "Smoothing spectra with kernel "+self.kernel )
            scan.smooth(kernel=self.kernel,width=self.kwidth,plot=self.verify,insitu=True)

    def finalize(self):
        if ( abs(self.plotlevel) > 0 ):
            pltfile=self.project+'_smspec.eps'
            sdutil.plot_scantable(self.worker.scan, pltfile, self.plotlevel, 'Smoothed spectra')
        
