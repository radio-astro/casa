from taskinit import casalog

import asap as sd
from asap.scantable import is_scantable, is_ms
import sdutil

@sdutil.sdtask_decorator
def sdsave(infile, splitant, antenna, getpt, rowlist, scanlist, field, iflist, pollist, scanaverage, timeaverage, tweight, polaverage, pweight, restfreq, outfile, outform, overwrite):
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
        if self.splitant:
            if not is_ms(self.infile):
                msg = 'input data must be in MS format'
                raise Exception, msg
            
            import datetime
            dt = datetime.datetime.now()
            self.temp_prefix = "temp-sdsave" + dt.strftime("%Y%m%d%H%M%S")
            self.split_infiles = sd.splitant(filename=self.infile,
                                             outprefix=self.temp_prefix,
                                             overwrite=self.overwrite,
                                             getpt=self.getpt)
            self.scans = []
            self.antenna_names = []
            for split_infile in self.split_infiles:
                work_scan = sd.scantable(split_infile,
                                         average=self.scanaverage)
                # scantable selection
                work_scan.set_selection(self.get_selector_by_list())
                self.scans.append(work_scan)

                #retrieve antenna names
                self.antenna_names.append(split_infile.split('.')[1])
            
        else:
            self.scan = sd.scantable(self.infile,
                                     average=self.scanaverage,
                                     antenna=self.antenna,
                                     getpt=self.getpt)

            # scantable selection
            #self.scan.set_selection(self.get_selector())
            self.scan.set_selection(self.get_selector_by_list())

    def execute(self):
        if self.splitant:
            for work_scan in self.scans:
                #Apply averaging
                self.original_scan = work_scan
                work_scan = sdutil.doaverage(self.original_scan, self.scanaverage,
                                             self.timeaverage, self.tweight,
                                             self.polaverage, self.pweight)
                # set rest frequency
                casalog.post('restore=%s'%(self.restore))
                if self.rfset:
                    work_scan.set_restfreqs(sdutil.normalise_restfreq(self.restfreq))
        
        else:
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
        if self.splitant:
            outfile_ext = ''
            elem_outfilename = self.outfile.split('.')
            len_elem = len(elem_outfilename)
            if (len_elem > 1):
                outfile_ext = elem_outfilename.pop().lower()
                if (outfile_ext == 'asap'):
                    outfile_ext = '.asap'
                else:
                    elem_outfilename.append(outfile_ext)
                    outfile_ext = ''

            outfile_prefix = '.'.join(elem_outfilename) + '_'

            i = 0
            for work_scan in self.scans:
                split_outfile = outfile_prefix + self.antenna_names[i] + outfile_ext
                sdutil.save(work_scan, split_outfile, self.outform, self.overwrite)
                i += 1

            for tempfile in self.split_infiles:
                import os
                os.system('rm -rf %s' % tempfile)
            
        else:
            # save
            sdutil.save(self.scan, self.outfile, self.outform, self.overwrite)
        
    def cleanup(self):
        if hasattr(self,'restore') and self.restore:
            casalog.post( "Restoring MOLECULE_ID column in %s "%self.infile )
            self.original_scan._setmolidcol_list(self.molids)
