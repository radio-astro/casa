from taskinit import casalog

import asap as sd
import sdutil

@sdutil.sdtask_decorator
def sdcoadd(infiles, antenna, freqtol, outfile, outform, overwrite):
    with sdutil.sdtask_manager(sdcoadd_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()


class sdcoadd_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdcoadd_worker,self).__init__(**kwargs)
        self.suffix = '_coadd'

    def initialize(self):
        self.nfile = len(self.infiles)
        if self.nfile < 2:
            raise Exception, 'Need at least two data file names'
        super(sdcoadd_worker,self).initialize()
            
    def initialize_scan(self):
        self.scanlist = [sd.scantable(f,average=False,antenna=self.antenna)
                         for f in self.infiles]

    def execute(self):
        self.set_to_scan()

        self.merge()

    def set_to_scan(self):
        pass

    def merge(self):
        self.scan = sd.merge(self.scanlist, freq_tol=self.freqtol)
        casalog.post( "Coadded %s" % self.infiles )

        # total row
        totalrow = 0
        for scan in self.scanlist:
            totalrow += scan.nrow()
        if totalrow > self.scan.nrow():
            casalog.post( "Actual number of rows is less than the number of rows expected in merged data.", priority = 'WARN' )
            casalog.post( "Possibly, there are conformance error among the input data.", priority = 'WARN' )

    def save(self):
        sdutil.save(self.scan, self.project, self.outform, self.overwrite)
        
    def cleanup(self):
        pass
