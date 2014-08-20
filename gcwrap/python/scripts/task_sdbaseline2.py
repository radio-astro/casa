import os
import numpy
import sdutil

from taskinit import casalog

import asap as sd
from asap.scantable import is_scantable

@sdutil.sdtask_decorator
def sdbaseline2(infile, antenna, row, field, spw, restfreq, frame, doppler, timerange, scan, pol, blmode, blparam, bltable, outfile, overwrite, keeprows):
    with sdutil.sdtask_manager(sdbaseline2_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()


class sdbaseline2_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdbaseline2_worker,self).__init__(**kwargs)
        self.suffix = '_bs'

    def initialize_scan(self):
        sorg = sd.scantable(self.infile, average=False, antenna=self.antenna)

        if len(self.row.strip()) > 0:
            self.rowlist = sorg.parse_idx_selection('row', self.row)
        
        sel = self.get_selector(sorg)
        sorg.set_selection(sel)
        del sel

        # Copy scantable when using disk storage not to modify
        # the original table.
        if is_scantable(self.infile) and self.is_disk_storage:
            self.scan = sorg.copy()
        else:
            self.scan = sorg
        del sorg

    def parameter_check(self):
        if self.blmode.lower().strip() not in ['subtract', 'apply']:
            msg = "The parameter blmode must be 'subtract' or 'apply'."
            raise Exception(msg)
        if not self.overwrite and self.blmode.lower()=='subtract' and os.path.exists(self.bltable):
            msg = "Output baseline table '" + self.bltable + "' exists."
            raise Exception(msg)
    
    def execute(self):
        engine = sdbaseline2_engine(self)
        engine.initialize()

        # check if the data contains spectra
        if (self.scan.nchan()==1):
           s = "Cannot process the input data. It contains only single channel data."
           raise Exception, s
   
        # set various attributes to self.scan
        self.set_to_scan()
        
        scanns = self.scan.getscannos()
        sn=list(scanns)
        casalog.post( "Number of scans to be processed: %d" % (len(sn)) )
        
        engine.execute()
        engine.finalize()

        if self.keeprows:
            self.scan.set_selection()

    def save(self):
        sdutil.save(self.scan, self.project, 'ASAP', self.overwrite)

class sdbaseline2_engine(sdutil.sdtask_engine):
    def __init__(self, worker):
        super(sdbaseline2_engine,self).__init__(worker)

        self.params = {}
        self.funcname = None

    def __del__(self):
        del self.params

    def initialize(self):
        pass
    
    def execute(self):
        scan = self.worker.scan
        maskdict = scan.parse_spw_selection(self.spw)

        # call target function with appropriate parameter set
        self.funcname = 'sub_baseline' if self.blmode == 'subtract' else 'apply_bltable'
        main_func = getattr(scan, self.funcname)

        self.params['insitu'] = True
        self.params['retfitres'] = False
        self.params['overwrite'] = self.overwrite

        if self.blmode == 'subtract':
            self.params['bltable'] = self.bltable
            self.params['blinfo'] = self.blparam
            if isinstance(self.params['blinfo'], list):
                for i in xrange(len(self.params['blinfo'])):
                    blinfo = self.params['blinfo'][i]
                    ifno = scan.getif(blinfo['row'])
                    if maskdict.has_key(ifno):
                        in_masklist = maskdict[ifno]
                        if blinfo.has_key('masklist'):
                            blinfo['masklist'] = sdutil.combine_masklist(in_masklist, blinfo['masklist'])
                        else:
                            blinfo['masklist'] = in_masklist
        else:
            self.params['inbltable'] = self.bltable
            self.params['outbltable'] = ''

        main_func(**self.params)
        #print 'Execute '+self.funcname+' with params '+str(self.params)

    def finalize(self):
        pass
