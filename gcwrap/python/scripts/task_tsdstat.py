import os
import string
from get_user import get_user

from taskinit import casalog, qatool

import asap as sd
from asap.scantable import is_scantable
import sdutil

@sdutil.sdtask_decorator
def tsdstat(infile, antenna, fluxunit, telescopeparm, specunit, restfreq, frame, doppler, scanlist, field, iflist, pollist, scanaverage, timeaverage, tweight, polaverage, pweight, masklist, invertmask, interactive, outfile, format, overwrite):
    with sdutil.sdtask_manager(sdstat_worker, locals()) as worker:
        worker.initialize()
        worker.execute()
        worker.finalize()
        
        return worker.result
        

class sdstat_worker(sdutil.sdtask_template):
    def __init__(self, **kwargs):
        super(sdstat_worker,self).__init__(**kwargs)

    def parameter_check(self):
        # If outfile is set, sd.rcParams['verbose'] must be True
        self.verbose_saved = sd.rcParams['verbose']
        if ( len(self.outfile) > 0 ):
            if ( not os.path.exists(sdutil.get_abspath(self.outfile)) \
                 or self.overwrite ):
                sd.rcParams['verbose']=True


    def initialize_scan(self):
        #load the data without averaging
        sorg = sd.scantable(self.infile,average=self.scanaverage,antenna=self.antenna)

        # collect data to restore
        self.restorer = sdutil.scantable_restore_factory(sorg,
                                                         self.infile,
                                                         self.fluxunit,
                                                         self.specunit,
                                                         self.frame,
                                                         self.doppler,
                                                         self.restfreq)
        
        # scantable selection
        #sorg.set_selection(self.get_selector())
        sorg.set_selection(self.get_selector_by_list())


        # this is bit tricky
        # set fluxunit here instead of self.set_to_scan
        # and remove fluxunit attribute to disable additional
        # call of set_fluxunit in self.set_to_scan
        self.scan = sdutil.set_fluxunit(sorg, self.fluxunit, self.telescopeparm, False)
        self.fluxunit_saved = self.fluxunit
        del self.fluxunit

        if self.scan:
            # Restore flux unit in original table before deleting
            self.restorer.restore()
            del self.restorer
            self.restorer = None
        else:
            self.scan = sorg

    def execute(self):
        self.set_to_scan()

        # Average data if necessary
        self.scan = sdutil.doaverage(self.scan, self.scanaverage, self.timeaverage, self.tweight, self.polaverage, self.pweight)

        # restore self.fluxunit
        self.fluxunit = self.fluxunit_saved
        del self.fluxunit_saved

        self.calc_statistics()

    def calc_statistics(self):
        # CAS-5410 Use private tools inside task scripts
        qa = qatool()

        # Warning for multi-IF data
        if len(self.scan.getifnos()) > 1:
            casalog.post( 'The scantable contains multiple IF data.', priority='WARN' )
            casalog.post( 'Note the same mask(s) are applied to all IFs based on CHANNELS.', priority='WARN' )
            casalog.post( 'Baseline ranges may be incorrect for all but IF=%d.\n' % (self.scan.getif(0)), priority='WARN' )

        # set mask
        self.__set_mask()

        # set formatter
        self.__set_formatter()

        # set unit labels
        self.__set_unit_labels()

        # calculate statistics
        #statsdict = get_stats(s, msk, formstr)
        self.__calc_stats()

        # reshape statsdict for return
        for k in ['min','max']:
            self.result['%s_abscissa'%(k)] = qa.quantity(self.result.pop('%s_abc'%(k)),self.xunit)
        for (k,u) in [('eqw',self.xunit),('totint',self.intunit)]:
            self.result[k] = qa.quantity(self.result[k],u)

    def save(self):
        if ( len(self.outfile) > 0 ):
            if ( not os.path.exists(sdutil.get_abspath(self.outfile)) \
                 or self.overwrite ):
                f=open(self.outfile,'w')
                f.write(self.resultstats)
                f.close()
            else:
                casalog.post( 'File '+self.outfile+' already exists.\nStatistics results are not written into the file.', priority = 'WARN' )

    def cleanup(self):
        # restore sd.rcParams
        sd.rcParams['verbose'] = self.verbose_saved

        # restore original scantable
        if hasattr(self,'restorer') and self.restorer is not None:
            self.restorer.restore()

    def __calc_stats(self):
        usr = get_user()
        tmpfile = '/tmp/tmp_'+usr+'_casapy_asap_scantable_stats'
        self.resultstats = ''
        self.result = {}

        # calculate regular statistics
        statsname = ['max', 'min', 'max_abc', 'min_abc',
                     'sum', 'mean', 'median', 'rms',
                     'stddev']
        for name in statsname:
            v = self.scan.stats(name,self.msk,self.format_string)
            self.result[name] = list(v) if len(v) > 1 else v[0]
            if sd.rcParams['verbose']:
                self.resultstats += get_text_from_file(tmpfile)

        # calculate additional statistics (eqw and integrated intensity)
        self.__calc_eqw_and_integf()

        if sd.rcParams['verbose']:
            # Print equivalent width
            out = self.__get_statstext('eqw', self.abclbl, 'eqw')
            self.resultstats += out

            # Print integrated flux
            outp = self.__get_statstext('Integrated intensity', self.intlbl, 'totint')
            self.resultstats += outp

            # to logger
            casalog.post(out[:-2]+outp)

    def __calc_eqw_and_integf(self):
        eqw = None
        integratef = None
        if isinstance(self.result['max'],list):
            # User selected multiple scans,ifs
            ns = len(self.result['max'])
            eqw=[]
            integratef=[]
            for i in range(ns):
                #Get bin width
                abcissa, lbl = self.scan.get_abcissa(rowno=i)
                dabc=abs(abcissa[-1] - abcissa[0])/float(len(abcissa)-1)
                # Construct equivalent width (=sum/max)
                eqw = eqw + [get_eqw(self.result['max'][i],
                                     self.result['min'][i],
                                     self.result['sum'][i],
                                     dabc)]
                # Construct integrated flux
                integratef = integratef + [get_integf(self.result['sum'][i], dabc)]
        else:
            # Single scantable only
            abcissa, lbl = self.scan.get_abcissa(rowno=0)
            dabc=abs(abcissa[-1] - abcissa[0])/float(len(abcissa)-1)
        
            # Construct equivalent width (=sum/max)
            eqw = get_eqw(self.result['max'],
                          self.result['min'],
                          self.result['sum'],
                          dabc)

            # Construct integrated flux
            integratef = get_integf(self.result['sum'], dabc)
        self.result['eqw'] = eqw
        self.result['totint'] = integratef

    def __set_mask(self):
        self.msk = None
        if self.interactive:
            # Interactive masking
            self.msk = sdutil.interactive_mask(self.scan,
                                               self.masklist,
                                               False,
                                               purpose='to calculate statistics')
            msks = self.scan.get_masklist(self.msk)
            if len(msks) < 1:
                raise Exception, 'No channel is selected. Exit without calculation.'
            lbl=self.scan.get_unit()
            casalog.post( 'final mask list ('+lbl+') = '+str(msks) )

            del msks

        # set the mask region
        elif ( len(self.masklist) > 0):
            self.msk=self.scan.create_mask(self.masklist,invert=self.invertmask)
            msks=self.scan.get_masklist(self.msk)
            if len(msks) < 1:
                del self.msk, msks
                raise Exception, 'Selected mask lists are out of range. Exit without calculation.'
            del msks
        else:
            # Full region
            casalog.post( 'Using full region' )
        
    def __set_formatter(self):
        self.format_string=self.format.replace(' ','')
        if len(self.format_string)==0:
            casalog.post("Invalid format string. Using the default 3.3f.")
            self.format_string='3.3f'

    def __set_unit_labels(self):
        # CAS-5410 Use private tools inside task scripts
        qa = qatool()

        if self.specunit != '': self.abclbl = self.specunit
        else: self.abclbl = self.scan.get_unit()
        if self.fluxunit != '': ordlbl = self.fluxunit
        else: ordlbl = self.scan.get_fluxunit()
        self.intlbl = ordlbl+' * '+self.abclbl

        # Check units
        if self.abclbl == 'channel' and not qa.check(self.abclbl):
            qa.define('channel','1 _')

        self.xunit = check_unit(self.abclbl,self.abclbl,'_')
        self.intunit = check_unit(ordlbl,ordlbl+'.'+self.abclbl,'_.'+self.abclbl)

    def __get_statstext(self, title, label, key):
        sep = "--------------------------------------------------"
        head = string.join([sep,string.join([" ","%s ["%(title),label,"]"]," "),sep],'\n')
        tail = ''
        out = head + '\n'
        val = self.result[key]
        if isinstance(val,list):
            ns = len(val)
            for i in xrange(ns):
                out += self.__get_statstr(i, val[i], sep)
        else:
            out += self.__get_statstr(0, val, sep)
        out += '\n%s'%(tail)
        return out

    def __get_statstr(self, irow, val, separator):
        out = ''
        out += 'Scan[%d] (%s) ' % (self.scan.getscan(irow), self.scan._getsourcename(irow))
        out += 'Time[%s]:\n' % (self.scan._gettime(irow))
        if self.scan.nbeam(-1) > 1: out +=  ' Beam[%d] ' % (self.scan.getbeam(irow))
        if self.scan.nif(-1) > 1: out +=  ' IF[%d] ' % (self.scan.getif(irow))
        if self.scan.npol(-1) > 1: out +=  ' Pol[%d] ' % (self.scan.getpol(irow))
        out += ('= %'+self.format_string) % (val) + '\n'
        out +=  "%s\n "%(separator)
        return out 

def check_unit(unit_in,valid_unit=None,default_unit=None):
    # CAS-5410 Use private tools inside task scripts
    qa = qatool()

    if qa.check(unit_in):
        return valid_unit
    else:
        casalog.post('Undefined unit: \'%s\'...ignored'%(unit_in), priority='WARN')
        return default_unit

def get_eqw(maxl, minl, suml, dabc):
    eqw = 0.0
    if ( maxl != 0.0 or minl != 0.0 ):
        if ( abs(maxl) >= abs(minl) ):
            eqw = suml/maxl*dabc
        else:
            eqw = suml/minl*dabc
    return eqw
    
def get_integf(suml, dabc):
    return suml * dabc

def get_text_from_file(filename):
    text = ''
    with open(filename, 'r') as f:
        for line in f:
            text += line
    return text

