from taskinit import casalog

import asap as sd
import sdutil

@sdutil.sdtask_decorator
def sdcoadd(infiles, antenna, fluxunit, telescopeparm, specunit, frame, doppler, scanaverage, timeaverage, tweight, polaverage, pweight, outfile, outform, overwrite):
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
        self.scanlist = [None] * self.nfile
        self.restorer = [None] * self.nfile

        for i in xrange(self.nfile):
            sorg = sd.scantable(self.infiles[i],average=self.scanaverage,antenna=self.antenna)
            
            # prepare restorer object
            restorer = sdutil.scantable_restore_factory(sorg,
                                                        self.infiles[i],
                                                        self.fluxunit,
                                                        self.specunit,
                                                        self.frame,
                                                        self.doppler)
            # convert flux
            stmp = sdutil.set_fluxunit(sorg, self.fluxunit, self.telescopeparm, False)
            if stmp:
                # Restore header in original table before deleting
                if restorer is not None:
                    restorer.restore()
                self.scanlist[i] = stmp
            else:
                self.scanlist[i] = sorg
                self.restorer[i] = restorer

        # this is bit tricky
        # set fluxunit here instead of self.set_to_scan
        # and remove fluxunit attribute to disable additional
        # call of set_fluxunit in self.set_to_scan
        self.fluxunit_saved = self.fluxunit
        del self.fluxunit

    def execute(self):
        self.set_to_scan()

        self.merge()

        # Average in time if desired
        self.scan = sdutil.doaverage(self.scan, self.scanaverage, self.timeaverage,
                                     self.tweight, self.polaverage, self.pweight)

    def set_to_scan(self):
        for scan in self.scanlist:
            self.scan = scan
            super(sdcoadd_worker,self).set_to_scan()
        del self.scan

    def merge(self):
        self.scan = sd.merge(self.scanlist)
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
        if hasattr(self,'restorer'):
            for r in self.restorer:
                if r:
                    r.restore()

