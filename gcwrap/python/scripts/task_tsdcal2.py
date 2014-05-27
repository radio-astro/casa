import os
import re
import numpy

from taskinit import casalog, gentools

import asap as sd
from asap.scantable import is_scantable
import sdutil

@sdutil.sdtask_decorator
def tsdcal2(infile, calmode, fraction, noff, width, elongated, tsysavg, tsysspw, applytable, interp, ifmap, field, spw, scan, pol, outfile, overwrite):
    casalog.post(str(locals()))
    with sdutil.sdtask_manager(sdcal2_worker, locals()) as worker: 
        worker.initialize()
        worker.execute()
        worker.finalize()

class sdcal2_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdcal2_worker,self).__init__(**kwargs)
        self.suffix = ''
        self.interp_time = ''
        self.interp_freq = ''
        self.doapply = False
        self.dosky = False
        self.dotsys = False
        self.insitu = False
        self.skymode = ''
        self.manager = sd._asap._calmanager()

    def initialize(self):
        # override initialize method
        self.parameter_check()
        self.initialize_scan()
        
    def parameter_check(self):
        self.check_infile()
        sep = ','
        num_separator = self.calmode.count(sep)
        if num_separator == 0:
            # single calibration process
            if self.calmode == 'apply':
                # apply sky (and Tsys) tables
                self.check_applytable()
                self.check_outfile()
                self.check_interp()
                #self.check_ifmap2()
                self.check_update()
                self.doapply = True
            elif self.calmode == 'tsys':
                # generate Tsys table
                self.check_outfile()
                self.check_tsysspw()
                self.dotsys = True
            else:
                # generate sky table
                self.check_outfile()
                self.dosky = True
                self.skymode = self.calmode
        elif num_separator == 1:
            # generate sky table and apply it on-the-fly
            #self.check_ifmap2()
            self.check_interp()
            self.check_update()
            self.dosky = True
            self.doapply = True
            self.skymode = self.calmode.split(sep)[0]
        else:
            # generate sky and Tsys table and apply them on-the-fly
            self.check_tsysspw()
            #self.check_ifmap()
            self.check_interp()
            self.check_update()
            self.dosky = True
            self.dotsys = True
            self.doapply = True
            self.skymode = self.calmode.split(sep)[0]

        if len(self.skymode) > 0:
            casalog.post('sky calibration mode: \'%s\''%(self.skymode), 'INFO')

    def check_infile(self):
        if not is_scantable(self.infile):
            raise Exception('infile must be in scantable format.')

    def check_outfile(self):
        if len(self.outfile) > 0:
            # outfile is specified
            if os.path.exists(self.outfile):
                if not self.overwrite:
                    raise Exception('Output file \'%s\' exists.'%(self.outfile))
                else:
                    casalog.post('Overwrite %s ...'%(self.outfile), 'INFO')
                    os.system('rm -rf %s'%(self.outfile))
    def check_applytable(self):
        # length should be > 0 either applytable is string or string list
        if len(self.applytable) == 0:
            raise Exception('Name of the apply table must be given.')

        if type(self.applytable) == str:
            # string 
            if not os.path.exists(self.applytable):
                raise Exception('Apply table \'%s\' does not exist.'%(self.applytable))
        else:
            # string list
            for tab in self.applytable:
                if len(tab) == 0:
                    raise Exception('Name of the apply table must be given.')
                elif not os.path.exists(tab):
                    raise Exception('Apply table \'%s\' does not exist.'%(tab))

    def check_interp(self):
        if len(self.interp) == 0:
            # default
            self.interp_time = ''
            self.interp_freq = ''
        else:
            valid_types = '^(nearest|linear|cspline|[0-9]+)$'
            interp_types = self.interp.split(',')
            if len(interp_types) > 2:
                raise Exception('Invalid format of the parameter interp: \'%s\''%(self.interp))
            self.interp_time = interp_types[0].lower()
            self.interp_freq = interp_types[-1].lower()
            if not re.match(valid_types,self.interp_time):
                raise Exception('Interpolation type \'%s\' is invalid or not supported yet.'%(self.interp_time))
            if not re.match(valid_types,self.interp_freq):
                raise Exception('Interpolation type \'%s\' is invalid or not supported yet.'%(self.interp_freq))
            
    def check_ifmap(self):
        if not isinstance(self.ifmap, dict) or len(self.ifmap) == 0:
            raise Exception('ifmap must be non-empty dictionary.')

    def check_ifmap2(self):
        self.check_ifmap()
            
    def check_tsysspw(self):
        pass

    def check_update(self):
        if len(self.outfile) == 0:
            if not self.overwrite:
                raise Exception('You should set overwrite to True if you want to update infile.')
            else:
                casalog.post('%s will be overwritten by the calibrated spectra.'%(self.infile), 'INFO')
                self.insitu = True

    def initialize_scan(self):
        if self.insitu:
            # update infile 
            storage = sd.rcParams['scantable.storage']
            sd.rcParams['scantable.storage'] = 'disk'
            self.scan = sd.scantable(self.infile,average=False)
            sd.rcParams['scantable.storage'] = storage
        else:
            self.scan = sd.scantable(self.infile,average=False)

        sel = self.get_selector(self.scan)
        self.scan.set_selection(sel)
        self.assert_no_channel_selection_in_spw('warn')
    
    def execute(self):
        self.manager.set_data(self.scan)
        if self.dosky:
            self.set_caloption()
            self.manager.set_calmode(self.skymode)
            self.manager.calibrate()
        if self.dotsys:
            self.__detect_tsysspw()
            self.manager.set_calmode('tsys')
            #self.manager.set_tsys_spw(self.tsysspw)
            if len(self.tsysspw) > 0:
                casalog.post('Testing new tsysspw specification: %s'%(self.tsysspw))
                tsysspw_dict = {}
                for (k,v) in self.scan.parse_spw_selection(self.tsysspw).items():
                    tsysspw_dict[str(k)] = numpy.array(v).flatten().tolist()
                casalog.post('tsysspw_dict=%s'%(tsysspw_dict))
                self.manager.set_tsys_spw_withrange(tsysspw_dict, self.tsysavg)
            self.manager.calibrate()
        if self.doapply:
            if isinstance(self.applytable,str):
                if len(self.applytable) > 0:
                    self.manager.add_applytable(self.applytable)
            else:
                for tab in self.applytable:
                    self.manager.add_applytable(tab)
            if len(self.interp_time) > 0:
                self.manager.set_time_interpolation(self.interp_time)
            if len(self.interp_freq) > 0:
                self.manager.set_freq_interpolation(self.interp_freq)
            for (k,v) in self.ifmap.items():
                self.manager.set_tsys_transfer(int(k),list(v))
            self.manager.apply(self.insitu, True)

    def save(self):
        if self.doapply:
            if not self.insitu:
                self.manager.split(self.outfile)
        elif self.dosky:
            outfile = sdutil.get_default_outfile_name(self.infile, self.outfile, '_sky')
            self.manager.save_caltable(outfile)
        elif self.dotsys:
            outfile = sdutil.get_default_outfile_name(self.infile, self.outfile, '_tsys')
            self.manager.save_caltable(outfile)
            
    def cleanup(self):
        self.manager.reset()

    def set_caloption(self):
        if self.skymode == 'otf':
            opt = {'fraction': self.fraction,
                   'width': self.width,
                   'elongated': self.elongated}
            self.manager.set_calibration_options(opt)
        elif self.skymode == 'otfraster':
            opt = {'fraction': self.fraction,
                   'npts': self.noff}
            self.manager.set_calibration_options(opt)

    def __detect_tsysspw(self):
        if len(self.tsysspw) == 0:
            if is_scantable(self.infile):
                sel_org = self.scan.get_selection()
                query_org = sel_org.get_query()
                casalog.post('original query: %s'%(query_org),priority='DEBUG')
                query_new = 'SRCTYPE == 10 && NELEMENTS(TSYS) > 1'
                if len(query_org.strip()) > 0:
                    query_new = '(%s) && (%s)'%(query_org, query_new)
                casalog.post('new query: %s'%(query_new),priority='DEBUG')
                sel = sd.selector(sel_org)
                sel.set_query(query_new)
                try:
                    self.scan.set_selection(sel)
                    self.tsysspw = ','.join(map(str,self.scan.getifnos()))
                except Exception, e:
                    casalog.post('Exception: %s'%(e))
                    self.tsysspw = ''
                finally:
                    self.scan.set_selection(sel_org)
            else:
                # should be MS
                tb = gentools(['tb'])[0]
                try:
                    tb.open(os.path.join(self.infile,'SYSCAL'))
                except:
                    return
                self.tsysspw = ','.join(map(str,numpy.unique(tb.getcol('SPECTRAL_WINDOW_ID'))))
                tb.close()
        casalog.post('tsysspw is set to %s (type %s)'%(self.tsysspw,type(self.tsysspw)), 'INFO')
