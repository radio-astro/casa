import os
import numpy
import contextlib

from taskinit import casalog, gentools

import asap as sd
from asap.scantable import is_scantable, is_ms
import sdutil

@sdutil.sdtask_decorator
def sdsave(infile, splitant, antenna, getpt, field, spw, timerange, scan, pol, beam, restfreq, outfile, outform, fillweight, overwrite):
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
                work_scan = sd.scantable(split_infile, average=False)
                # scantable selection
                #work_scan.set_selection(self.get_selector_by_list())
                work_scan.set_selection(self.get_selector(work_scan))
                self.scans.append(work_scan)

                # retrieve antenna names
                self.antenna_names.append(split_infile.split('.')[1])
            
        else:
            scan = sd.scantable(self.infile,
                                average=False,
                                antenna=self.antenna,
                                getpt=self.getpt)

            # scantable selection
            scan.set_selection(self.get_selector(scan))
            #self.scan.set_selection(self.get_selector_by_list())
            self.scan = scan

    def execute(self):
        if self.splitant:
            for work_scan in self.scans:
                #Apply averaging
#                self.original_scan = work_scan
#                work_scan = sdutil.doaverage(self.original_scan, self.scanaverage,
                 #                            self.timeaverage, self.tweight,
                 #                            self.polaverage, self.pweight)

                # channel range selection
                self.__dochannelrange(work_scan)
                
                # set rest frequency
                casalog.post('restore=%s'%(self.restore))
                if self.rfset:
                    work_scan.set_restfreqs(sdutil.normalise_restfreq(self.restfreq))
        
        else:
            # Apply averaging
 #           self.original_scan = self.scan
 #           self.scan = sdutil.doaverage(self.original_scan, self.scanaverage,
 #                                        self.timeaverage, self.tweight,
 #                                        self.polaverage, self.pweight)
            self.original_scan = self.scan

            if is_scantable(self.infile) and self.is_disk_storage:
                self.scan = self.original_scan.copy()

            if self.original_scan == self.scan and self.rfset \
                   and is_scantable(self.infile) \
                   and self.is_disk_storage:
                self.molids = self.original_scan._getmolidcol_list()
                self.restore = True

            # channel range selection
            self.__dochannelrange(self.scan)
                    
            # set rest frequency
            casalog.post('restore=%s'%(self.restore))
            if self.rfset:
                self.scan.set_restfreqs(sdutil.normalise_restfreq(self.restfreq))

    def save(self):
        if self.splitant:
            outfile_ext = ''
            elem_outfilename = self.project.split('.')
            len_elem = len(elem_outfilename)
            if (len_elem > 1):
                outfile_ext = elem_outfilename.pop().lower()
                if (outfile_ext == 'asap'):
                    outfile_ext = '.asap'
                else:
                    elem_outfilename.append(outfile_ext)
                    outfile_ext = ''

            outfile_prefix = '.'.join(elem_outfilename) + '.'

            i = 0
            for work_scan in self.scans:
                split_outfile = outfile_prefix + self.antenna_names[i] + outfile_ext
                sdutil.save(work_scan, split_outfile, self.outform, self.overwrite)
                i += 1

        else:
            # save
            sdutil.save(self.scan, self.project, self.outform, self.overwrite)

            if self.outform == 'MS2' and self.fillweight:
                _fillweight(self.project)
            
    def cleanup(self):
        if hasattr(self,'restore') and self.restore:
            casalog.post( "Restoring MOLECULE_ID column in %s "%self.infile )
            self.original_scan._setmolidcol_list(self.molids)
        if hasattr(self,'temp_prefix') and self.splitant:
            os.system('rm -rf %s*' % self.temp_prefix)

    def __dochannelrange(self, scantab):
        if len(self.spw) > 0:
            sel_org = scantab.get_selection()
            channelrange_dic = scantab.parse_spw_selection(self.spw)
            valid_spw_list = []
            for (k,v) in channelrange_dic.items():
                casalog.post('k=%s, v=%s'%(k,v), priority='DEBUG')
                unique_list = []
                for item in map(list, v):
                    if not item in unique_list:
                        unique_list.append(item)
                casalog.post('unique_list=%s'%(unique_list), priority='DEBUG')
                if len(unique_list) > 1:
                    raise SyntaxError('sdsave doesn\'t support multiple channel range selection for spw.')
                elif len(unique_list) == 0:
                    raise SyntaxError('Invalid channel range specification')
                elif numpy.any(numpy.array(map(len, unique_list)) == 0):
                    # empty channel range
                    continue
                nchan = scantab.nchan(k)
                full_range = [0.0, float(nchan-1)]
                if unique_list[0] != full_range:
                    sel = sd.selector()
                    sel.set_ifs(k)
                    scantab.set_selection(sel)
                    sdutil.dochannelrange(scantab, unique_list[0])
                    scantab.set_selection()
                valid_spw_list.append(k)
            sel = sd.selector(sel_org)
            ifs_org = sel.get_ifs()
            ifs_new = list(set(ifs_org) & set(valid_spw_list))
            casalog.post('ifs_new = %s'%(ifs_new))
            sel.set_ifs(ifs_new)
            scantab.set_selection(sel)

@contextlib.contextmanager
def toolmanager(vis, tooltype, *args, **kwargs):
    tool = gentools([tooltype])[0]
    tool.open(vis, *args, **kwargs)
    yield tool
    tool.close()

def _fillweight(vis):
    if not os.path.exists(vis):
        return

    casalog.post('fill WEIGHT and SIGMA columns for %s ...'%(vis))

    # work with private cb tool
    with toolmanager(vis, 'cb', compress=False, addcorr=False, addmodel=False) as cb:
        status = cb.initweights()

    if status:
        # cb.initweights() succeeded so try to apply Tsys to
        # weight column
        # procedure:
        # 1. generate temporary Tsys caltable
        # 2. apply temporary Tsys caltable to vis
        # 3. remove temporary Tsys caltable
        import time
        from gencal import gencal
        from applycal import applycal
        caltable = 'temporary_caltable%s.tsys'%(str(time.time()).replace('.',''))
        casalog.post('tempolary caltable name: %s'%(caltable))
        try:
            gencal(vis=vis, caltable=caltable, caltype='tsys')
            applycal(vis=vis, docallib=False, gaintable=[caltable], applymode='calonly')
        except Exception, e:
            # Tsys application failed so that reset WEIGHT and SIGMA to 1.0
            _resetweight(vis)
            raise e
        finally:
            if os.path.exists(caltable):
                casalog.post('remove %s...'%(caltable))
                os.system('rm -rf %s'%(caltable))
            # remove CORRECTED_DATA column
            casalog.post('remove CORRECTED_DATA from %s...'%(vis))
            with toolmanager(vis, 'tb', nomodify=False) as tb:
                if 'CORRECTED_DATA' in tb.colnames():
                    tb.removecols('CORRECTED_DATA')
        
    else:
        # initweights failed so reset WEIGHT and SIGMA to 1.0
        _resetweight(vis)

def _resetweight(vis):
    # work with private tb tool
    casalog.post('fill WEIGHT and SIGMA failed. reset all WEIGHT and SIGMA to 1.0...', priority='WARN')
    with toolmanager(vis, 'tb', nomodify=False) as tb:
        for column in ['WEIGHT', 'SIGMA']:
            values = tb.getvarcol(column)
            for v in values.values():
                v[:] = 1.0
            tb.putvarcol(column, values)

