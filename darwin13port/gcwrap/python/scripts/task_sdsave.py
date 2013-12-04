from taskinit import casalog

import asap as sd
from asap.scantable import is_scantable
import sdutil

@sdutil.sdtask_decorator
def sdsave(infile, antenna, getpt, rowlist, scanlist, field, iflist, pollist, scanaverage, timeaverage, tweight, polaverage, pweight, restfreq, outfile, outform, overwrite):
    with sdutil.sdtask_manager(sdsave_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()
        

class sdsave_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdsave_worker,self).__init__(**kwargs)
        self.suffix = '_saved'

    def parameter_check(self):
        # for restore information
        self.restore = False
        self.molids = None
        self.rfset = (self.restfreq != '') and (self.restfreq != [])
        
    def initialize_scan(self):
        self.scan = sd.scantable(self.infile,
                                 average=self.scanaverage,
                                 antenna=self.antenna,
                                 getpt=self.getpt)

        # scantable selection
        #self.scan.set_selection(self.get_selector())
        self.scan.set_selection(self.get_selector_by_list())

    def execute(self):
        # Apply averaging
        self.original_scan = self.scan
        self.scan = sdutil.doaverage(self.original_scan, self.scanaverage,
                                     self.timeaverage, self.tweight,
                                     self.polaverage, self.pweight)

        if self.original_scan == self.scan and self.rfset \
               and is_scantable(self.infile) \
               and self.is_disk_storage:
            self.molids = self.original_scan._getmolidcol_list()
            self.restore = True
        
        # set rest frequency
        casalog.post('restore=%s'%(self.restore))
        if self.rfset:
            self.scan.set_restfreqs(sdutil.normalise_restfreq(self.restfreq))

    def save(self):
        # save
        sdutil.save(self.scan, self.outfile, self.outform, self.overwrite)
        
    def cleanup(self):
        if hasattr(self,'restore') and self.restore:
            casalog.post( "Restoreing MOLECULE_ID column in %s "%self.infile )
            self.original_scan._setmolidcol_list(self.molids)
            
