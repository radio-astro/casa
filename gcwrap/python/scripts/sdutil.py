import os
import numpy as np
import traceback
import string
import functools
import re
import abc
import datetime

from casac import casac
from taskinit import casalog, gentools, qatool
import asap as sd
from asap import _to_list
from asap.scantable import is_scantable, is_ms, scantable

def sdtask_decorator(func):
    """
    This is a decorator function for sd tasks. 
    Currently the decorator does:

       1) set origin to the logger 
       2) handle exception

    So, you don't need to set origin in the task any more. 
    Also, you don't need to write anything about error 
    handling in the task. If you have something to do 
    at the end of the task execution, those should be 
    written in the destructor of worker class, not in 
    the 'finally' block.
    """
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        # set origin
        casalog.origin(func.__name__)

        retval = None
        # Any errors are handled outside the task.
        # however, the implementation below is effectively 
        # equivalent to handling it inside the task.
        try:
            # execute task 
            retval = func(*args, **kwargs)
        except Exception, e:
            traceback_info = format_trace(traceback.format_exc())
            casalog.post(traceback_info,'SEVERE')
            casalog.post(str(e),'ERROR')
            raise Exception, e
        return retval
    return wrapper

def format_trace(s):
    wexists=True
    regex='.*sdutil\.py.*in wrapper.*'
    retval = s
    while wexists:
        ss = retval.split('\n')
        wexists = False
        for i in xrange(len(ss)):
            if re.match(regex,ss[i]):
                ss = ss[:i] + ss[i+2:]
                wexists = True
                break
        retval = string.join(ss,'\n')
    return retval

class sdtask_manager(object):
    def __init__(self, cls, args):
        self.cls = cls
        self.args = args

    def __enter__(self):
        self.obj = self.cls(**self.args)
        return self.obj

    def __exit__(self, exc_type, exc_value, traceback):
        # explicitly call destructure to make sure it is called here
        self.obj.__del__()
        del self.obj
        if exc_type:
            return False
        else:
            return True

class sdtask_interface(object):
    """
    The sdtask_interface defines a common interface for sdtask_worker
    class. All worker classes can be used as follows:

       worker = sdtask_worker(**locals())
       worker.initialize()
       worker.execute()
       worker.finalize()
       del worker

    Derived classes must implement the above three methods: initialize(),
    execute(), and finalize().
    """
    __metaclass__ = abc.ABCMeta
    
    def __init__(self, **kwargs):
        for (k,v) in kwargs.items():
            setattr(self, k, v)
        # special treatment for selection parameters
        select_params = ['scan', 'pol','beam']
        for param in select_params:
            if hasattr(self, param):
                setattr(self, param+'no', getattr(self, param))
                #print("renaming self.%s -> self.%sno='%s'" % (param, param, getattr(self, param)))
                delattr(self, param)

    def __del__(self):
        pass

    @abc.abstractmethod
    def initialize(self):
        raise NotImplementedError('initialize is abstract method')

    @abc.abstractmethod
    def execute(self):
        raise NotImplementedError('execute is abstract method')

    @abc.abstractmethod
    def finalize(self):
        raise NotImplementedError('finalize is abstract method')

class sdtask_template(sdtask_interface):
    """
    The sdtask_template is a template class for standard worker
    class of non-imaging sdtasks. It partially implement initialize()
    and finalize() using internal methods that must be implemented
    in the derived classes. For initialize(), derived classes
    must implement initialize_scan(), which initializes scantable
    object and set it to self.scan. You can implement paramter_check()
    to do any task specific parameter check in initialize().
    For finalize(), derived classes can implement save() and cleanup().
    """
    __metaclass__ = abc.ABCMeta

    def __init__(self, **kwargs):
        super(sdtask_template,self).__init__(**kwargs)
        if not hasattr(self, 'outform'):
            self.outform = 'undefined'
        ### WORKAROUND to support old telescopeparm parameter
        if hasattr(self, 'telescopeparm'):
            self.telescopeparam = self.telescopeparm
        self.is_disk_storage = (sd.rcParams['scantable.storage'] == 'disk')
        # attribute for tasks that return any result
        self.result = None

    def __del__(self, base=sdtask_interface):
        self.cleanup()
        super(sdtask_template,self).__del__()

    def initialize(self):
        if hasattr(self, 'infile'):
            assert_infile_exists(self.infile)
        elif hasattr(self, 'infiles'):
            if isinstance(self.infiles,str):
                assert_infile_exists(self.infiles)
            else:
                for f in self.infiles:
                    assert_infile_exists(f)
        if hasattr(self, 'suffix'):
            if hasattr(self, 'infile'):
                self.project = get_default_outfile_name(self.infile,self.outfile,self.suffix)
            elif hasattr(self, 'infiles'):
                self.project = get_default_outfile_name(self.infiles[0],self.outfile,self.suffix)
        elif hasattr(self, 'outfile') and len(self.outfile) > 0:
            self.project = self.outfile
##         if hasattr(self, 'outfile') and len(self.outfile) > 0:
##             assert_outfile_canoverwrite_or_nonexistent(self.outfile,self.outform,self.overwrite)
        if hasattr(self, 'project'):
            assert_outfile_canoverwrite_or_nonexistent(self.project,self.outform,self.overwrite)

        # task specific parameter check
        self.parameter_check()
        
        # set self.scan
        self.initialize_scan()

    def finalize(self):
        # Save result on disk if necessary
        self.save()

    @abc.abstractmethod
    def initialize_scan(self):
        # initialize scantable object to work with
        raise NotImplementedError('initialize_scan is abstract method')

    def parameter_check(self):
        # do nothing by default
        pass

    def save(self):
        # do nothing by default
        pass
        
    def cleanup(self):
        # do nothing by default
        pass
        
    def get_selector_by_list(self):
        attributes = ['scanlist','iflist','pollist','beamlist',
                      'rowlist','field']
        for a in attributes:
            if not hasattr(self,a): setattr(self,a,None)
        selector = get_selector_by_list(in_scans=self.scanlist,
                                        in_ifs=self.iflist,
                                        in_pols=self.pollist,
                                        in_beams=self.beamlist,
                                        in_rows=self.rowlist,
                                        in_field=self.field)

        # CAS-5496 selection by timerange
        if hasattr(self, 'timerange') and len(self.timerange) > 0:
            # base scantable
            if hasattr(self, 'infile'):
                base_table = self.infile
            elif hasattr(self, 'infiles'):
                base_table = self.infiles[0]
            else:
                base_table = None
            taql_for_timerange = select_by_timerange(base_table, self.timerange)
            query_org = selector.get_query()
            if len(query_org) > 0:
                selector.set_query(' && '.join([query_org, taql_for_timerange]))
            else:
                selector.set_query(taql_for_timerange)
            casalog.post('taql: \'%s\''%(selector.get_query()), priority='DEBUG')

        return selector
                

    def get_selector(self, scantb=None):
        if not scantb:
            if hasattr(self,'scan') and isinstance(self.scan, scantable):
                scantb = self.scan
            else:
                raise Exception, "Internal Error. No valid scantable."
        
        attributes = ['scanno','spw','polno','beamno','field']
        for a in attributes:
            if not hasattr(self,a): setattr(self,a,"")

        
        self.scanlist = scantb.parse_idx_selection("SCAN",self.scanno)
        self.iflist = []
        if self.spw != "":
            ifno_str = str(',').join(scantb.parse_maskexpr(self.spw).keys())
            self.iflist = scantb.parse_idx_selection("IF",ifno_str)
        self.pollist = scantb.parse_idx_selection("POL",self.polno)
        self.beamlist = scantb.parse_idx_selection("BEAM",self.beamno)
        
        attributes = ['scanlist','iflist','pollist','beamlist',
                      'rowlist','field']
        for a in attributes:
            if not hasattr(self,a): setattr(self,a,None)
        selector = get_selector(in_scans=self.scanlist,
                                in_ifs=self.iflist,
                                in_pols=self.pollist,
                                in_beams=self.beamlist,
                                in_rows=self.rowlist,
                                in_field=self.field)

        # CAS-5496 selection by timerange
        if hasattr(self, 'timerange') and len(self.timerange) > 0:
            # base scantable
            if hasattr(self, 'infile'):
                base_table = self.infile
            elif hasattr(self, 'infiles'):
                base_table = self.infiles[0]
            else:
                base_table = None
            taql_for_timerange = select_by_timerange(base_table, self.timerange)
            query_org = selector.get_query()
            if len(query_org) > 0:
                selector.set_query(' && '.join([query_org, taql_for_timerange]))
            else:
                selector.set_query(taql_for_timerange)
            casalog.post('taql: \'%s\''%(selector.get_query()), priority='DEBUG')

        return selector
                
        
    def set_to_scan(self):
        if hasattr(self,'fluxunit'):
            set_fluxunit(self.scan, self.fluxunit, self.telescopeparam)
        if hasattr(self,'frame'):
            set_freqframe(self.scan, self.frame)
        if hasattr(self,'doppler'):
            set_doppler(self.scan,self.doppler)
        if hasattr(self,'specunit'):
            set_spectral_unit(self.scan,self.specunit)
            if hasattr(self,'restfreq'):
                rfset = self.restfreq not in ['',[]]
                if self.specunit == 'km/s':
                    if len(self.scan.get_restfreqs().values()[0]) == 0 and not rfset:
                        raise Exception('Restfreq must be given')
                    if rfset:
                        fval = normalise_restfreq(self.restfreq)
                        casalog.post( 'Set rest frequency to %s Hz' % str(fval) )
                        self.scan.set_restfreqs(freqs=fval)

class sdtask_template_imaging(sdtask_interface):
    """
    The sdtask_template_imaging is a template class for worker
    class of imaging related sdtasks. It partially implement initialize()
    and finalize() using internal methods that must be implemented
    in the derived classes. For initialize(), derived classes
    must implement compile(), which sets up imaging parameters.
    You can implement paramter_check() to do any task specific parameter
    check in initialize().
    For finalize(), derived classes can implement cleanup().
    """
    def __init__(self, **kwargs):
        super(sdtask_template_imaging,self).__init__(**kwargs)
        self.is_table_opened = False
        self.is_imager_opened = False
        self.table, self.imager = gentools(['tb','im'])
        # workaround for sdtpimaging
        if not hasattr(self, 'infiles') and hasattr(self, 'infile'):
            self.infiles = [self.infile]

        self.__set_infiles()
        self.__set_subtable_name()

    def __del__(self, base=sdtask_interface):
        # table and imager must be closed when the instance
        # is deleted
        self.close_table()
        self.close_imager()
        self.cleanup()
        super(sdtask_template_imaging,self).__del__()

    def open_table(self, name, nomodify=True):
        if self.is_table_opened:
            casalog.post('Close table before re-open', priority='WARN')
            return
        self.table.open(name, nomodify=nomodify)
        self.is_table_opened = True

    def close_table(self):
        if self.is_table_opened:
            self.table.close()
        self.is_table_opened = False

    def open_imager(self, name=''):
        if self.is_imager_opened:
            casalog.post('Close imager before re-open', priority='WARN')
            return
        self.imager.open(name)
        self.is_imager_opened = True

    def close_imager(self):
        if self.is_imager_opened:
            self.imager.close()
        self.is_imager_opened = False

    def initialize(self):
        # infiles must be MS
        for idx in range(len(self.infiles)):
            if not is_ms(self.infiles[idx]):
                msg='input data sets must be in MS format'
                raise Exception, msg
        
        self.parameter_check()
        self.compile()

    def finalize(self):
        pass

    def parameter_check(self):
        pass

    def compile(self):
        pass

    def cleanup(self):
        pass
        
    def __set_subtable_name(self):
        self.open_table(self.infiles[0])
        keys = self.table.getkeywords()
        self.close_table()
        self.field_table = get_subtable_name(keys['FIELD'])
        self.spw_table = get_subtable_name(keys['SPECTRAL_WINDOW'])
        self.source_table = get_subtable_name(keys['SOURCE'])
        self.antenna_table = get_subtable_name(keys['ANTENNA'])
        self.polarization_table = get_subtable_name(keys['POLARIZATION'])
        self.observation_table = get_subtable_name(keys['OBSERVATION'])
        self.pointing_table = get_subtable_name(keys['POINTING'])
        self.data_desc_table = get_subtable_name(keys['DATA_DESCRIPTION'])
        self.pointing_table = get_subtable_name(keys['POINTING'])

    def __set_infiles(self):
        if type(self.infiles) == str:
            self.infiles = [self.infiles]


class sdtask_engine(sdtask_interface):
    def __init__(self, worker):
        # set worker instance to work with
        self.worker = worker

        # copy worker attributes except scan
        # use worker.scan to access scantable
        for (k,v) in self.worker.__dict__.items():
            if k != 'scan':
                setattr(self, k, v)
        #super(sdtask_engine,self).__init__(**self.worker.__dict__)
        #if hasattr(self,'scan'): del self.scan
    
def get_abspath(filename):
    return os.path.abspath(expand_path(filename))

def expand_path(filename):
    return os.path.expanduser(os.path.expandvars(filename))

def assert_infile_exists(infile=None):
    if (infile == ""):
        raise Exception, "infile is undefined"

    filename = get_abspath(infile)
    if not os.path.exists(filename):
        mesg = "File '%s' not found." % (infile)
        raise Exception, mesg


def get_default_outfile_name(infile=None, outfile=None, suffix=None):
    if (outfile == ""):
        res = infile.rstrip("/") + suffix
    else:
        res = outfile.rstrip("/")
    return res


def assert_outfile_canoverwrite_or_nonexistent(outfile=None, outform=None, overwrite=None):
    if not overwrite and (outform.upper() != "ASCII"):
        filename = get_abspath(outfile)
        if os.path.exists(filename):
            mesg = "Output file '%s' exists." % (outfile)
            raise Exception, mesg


def get_listvalue(value):
    return _to_list(value, int) or []

def get_selector_by_list(in_scans=None, in_ifs=None, in_pols=None, \
                         in_field=None, in_beams=None, in_rows=None,
                         in_timerange=None):
    scans = get_listvalue(in_scans)
    ifs   = get_listvalue(in_ifs)
    pols  = get_listvalue(in_pols)
    beams = get_listvalue(in_beams)
    rows = get_listvalue(in_rows)
    selector = sd.selector(scans=scans, ifs=ifs, pols=pols, beams=beams,
                           rows=rows)

    if (in_field != ""):
        # NOTE: currently can only select one
        # set of names this way, will probably
        # need to do a set_query eventually
        selector.set_name(in_field)

    return selector


def get_selector(in_scans=None, in_ifs=None, in_pols=None, \
                 in_field=None, in_beams=None, in_rows=None,
                 in_timerange=None):
    scans = get_listvalue(in_scans)
    ifs   = get_listvalue(in_ifs)
    pols  = get_listvalue(in_pols)
    beams = get_listvalue(in_beams)
    rows = get_listvalue(in_rows)
    selector = sd.selector(scans=scans, ifs=ifs, pols=pols, beams=beams,
                           rows=rows)

    if (in_field != ""):
        selector.set_msselection_field(in_field)

    return selector


def get_restfreq_in_Hz(s_restfreq):
    qatl = casac.quanta()
    if not qatl.isquantity(s_restfreq):
        mesg = "Input value is not a quantity: %s" % (str(s_restfreq))
        raise Exception, mesg
    if qatl.compare(s_restfreq,'Hz'):
        return qatl.convert(s_restfreq, 'Hz')['value']
    elif qatl.quantity(s_restfreq)['unit'] == '':
        return float(s_restfreq)
    else:
        mesg = "wrong unit of restfreq."
        raise Exception, mesg
###############################################################
# def get_restfreq_in_Hz(s_restfreq):
#     value = 0.0
#     unit = ""
#     s = s_restfreq.replace(" ","")
# 
#     for i in range(len(s))[::-1]:
#         if s[i].isalpha():
#             unit = s[i] + unit
#         else:
#             value = float(s[0:i+1])
#             break
# 
#     if (unit == "") or (unit.lower() == "hz"):
#         return value
#     elif (len(unit) == 3) and (unit[1:3].lower() == "hz"):
#         unitprefix = unit[0]
#         factor = 1.0
# 
#         if (unitprefix == "a"):
#             factor = 1.0e-18
#         elif (unitprefix == "f"):
#             factor = 1.0e-15
#         elif (unitprefix == "p"):
#             factor = 1.0e-12
#         elif (unitprefix == "n"):
#             factor = 1.0e-9
#         elif (unitprefix == "u"):
#             factor = 1.0e-6
#         elif (unitprefix == "m"):
#             factor = 1.0e-3
#         elif (unitprefix == "k"):
#             factor = 1.0e+3
#         elif (unitprefix == "M"):
#             factor = 1.0e+6
#         elif (unitprefix == "G"):
#             factor = 1.0e+9
#         elif (unitprefix == "T"):
#             factor = 1.0e+12
#         elif (unitprefix == "P"):
#             factor = 1.0e+15
#         elif (unitprefix == "E"):
#             factor = 1.0e+18
#         
#         return value*factor
#     else:
#         mesg = "wrong unit of restfreq."
#         raise Exception, mesg
###############################################################

def normalise_restfreq(in_restfreq):
    if isinstance(in_restfreq, float):
        return in_restfreq
    elif isinstance(in_restfreq, int) or isinstance(in_restfreq, long):
        return float(in_restfreq)
    elif isinstance(in_restfreq, str):
        return get_restfreq_in_Hz(in_restfreq)
    elif isinstance(in_restfreq, list) or isinstance(in_restfreq, np.ndarray):
        if isinstance(in_restfreq, np.ndarray):
            if len(in_restfreq.shape) > 1:
                mesg = "given in numpy.ndarray, in_restfreq must be 1-D."
                raise Exception, mesg
        
        res = []
        for i in xrange(len(in_restfreq)):
            elem = in_restfreq[i]
            if isinstance(elem, float):
                res.append(elem)
            elif isinstance(elem, int) or isinstance(elem, long):
                res.append(float(elem))
            elif isinstance(elem, str):
                res.append(get_restfreq_in_Hz(elem))
            elif isinstance(elem, dict):
                if isinstance(elem["value"], float):
                    res.append(elem)
                elif isinstance(elem["value"], int):
                    dictelem = {}
                    dictelem["name"]  = elem["name"]
                    dictelem["value"] = float(elem["value"])
                    res.append(dictelem)
                elif isinstance(elem["value"], str):
                    dictelem = {}
                    dictelem["name"]  = elem["name"]
                    dictelem["value"] = get_restfreq_in_Hz(elem["value"])
                    res.append(dictelem)
            else:
                mesg = "restfreq elements must be float, int, or string."
                raise Exception, mesg
        return res
    else:
        mesg = "wrong type of restfreq given."
        raise Exception, mesg

def set_restfreq(s, restfreq):
    rfset = (restfreq != '') and (restfreq != [])
    if rfset:
        s.set_restfreqs(normalise_restfreq(restfreq))

def set_spectral_unit(s, specunit):
    if (specunit != ''):
        s.set_unit(specunit)

def set_doppler(s, doppler):
    if (doppler != ''):
        if (doppler in ['radio', 'optical', 'z']):
            ddoppler = doppler.upper()
        else:
            ddoppler = doppler
        s.set_doppler(ddoppler)
    else:
        casalog.post('Using current doppler conversion')

def set_freqframe(s, frame):
    if (frame != ''):
        s.set_freqframe(frame)
    else:
        casalog.post('Using current frequency frame')

def set_fluxunit(s, fluxunit, telescopeparam, insitu=True):
    ret = None
    
    # check current fluxunit
    # for GBT if not set, set assumed fluxunit, Kelvin
    antennaname = s.get_antennaname()
    fluxunit_now = s.get_fluxunit()
    if (antennaname == 'GBT'):
        if (fluxunit_now == ''):
            casalog.post('No fluxunit in the data. Set to Kelvin.')
            s.set_fluxunit('K')
            fluxunit_now = s.get_fluxunit()
    casalog.post('Current fluxunit = %s'%(fluxunit_now))

    # convert flux
    # set flux unit string (be more permissive than ASAP)
    if (fluxunit == 'k'):
        fluxunit_local = 'K'
    elif (fluxunit.upper() == 'JY'):
        fluxunit_local = 'Jy'
    else:
        fluxunit_local = fluxunit

        
    # fix the fluxunit if necessary
    if ( telescopeparam == 'FIX' or telescopeparam == 'fix' ):
        if ( fluxunit_local != '' ):
            if ( fluxunit_local == fluxunit_now ):
                #print "No need to change default fluxunits"
                casalog.post( "No need to change default fluxunits" )
            else:
                s.set_fluxunit(fluxunit_local)
                #print "Reset default fluxunit to "+fluxunit
                casalog.post( "Reset default fluxunit to "+fluxunit_local )
                fluxunit_now = s.get_fluxunit()
        else:
            #print "Warning - no fluxunit for set_fluxunit"
            casalog.post( "no fluxunit for set_fluxunit", priority = 'WARN' )


    elif ( fluxunit_local=='' or fluxunit_local==fluxunit_now ):
        if ( fluxunit_local==fluxunit_now ):
            #print "No need to convert fluxunits"
            casalog.post( "No need to convert fluxunits" )

    elif ( type(telescopeparam) == list ):
        # User input telescope params
        if ( len(telescopeparam) > 1 ):
            D = telescopeparam[0]
            eta = telescopeparam[1]
            #print "Use phys.diam D = %5.1f m" % (D)
            #print "Use ap.eff. eta = %5.3f " % (eta)
            casalog.post( "Use phys.diam D = %5.1f m" % (D) )
            casalog.post( "Use ap.eff. eta = %5.3f " % (eta) )
            ret = s.convert_flux(eta=eta,d=D,insitu=insitu)
        elif ( len(telescopeparam) > 0 ):
            jypk = telescopeparam[0]
            #print "Use gain = %6.4f Jy/K " % (jypk)
            casalog.post( "Use gain = %6.4f Jy/K " % (jypk) )
            ret = s.convert_flux(jyperk=jypk,insitu=insitu)
        else:
            #print "Empty telescope list"
            casalog.post( "Empty telescope list" )

    elif ( telescopeparam=='' ):
        if ( antennaname == 'GBT'):
            # needs eventually to be in ASAP source code
            #print "Convert fluxunit to "+fluxunit
            casalog.post( "Convert fluxunit to "+fluxunit_local )
            # THIS IS THE CHEESY PART
            # Calculate ap.eff eta at rest freq
            # Use Ruze law
            #   eta=eta_0*exp(-(4pi*eps/lambda)**2)
            # with
            #print "Using GBT parameters"
            casalog.post( "Using GBT parameters" )
            eps = 0.390  # mm
            eta_0 = 0.71 # at infinite wavelength
            # Ideally would use a freq in center of
            # band, but rest freq is what I have
            rf = s.get_restfreqs()[0][0]*1.0e-9 # GHz
            eta = eta_0*np.exp(-0.001757*(eps*rf)**2)
            #print "Calculated ap.eff. eta = %5.3f " % (eta)
            #print "At rest frequency %5.3f GHz" % (rf)
            casalog.post( "Calculated ap.eff. eta = %5.3f " % (eta) )
            casalog.post( "At rest frequency %5.3f GHz" % (rf) )
            D = 104.9 # 100m x 110m
            #print "Assume phys.diam D = %5.1f m" % (D)
            casalog.post( "Assume phys.diam D = %5.1f m" % (D) )
            ret = s.convert_flux(eta=eta,d=D,insitu=insitu)
            
            #print "Successfully converted fluxunit to "+fluxunit
            casalog.post( "Successfully converted fluxunit to "+fluxunit_local )
        elif ( antennaname in ['AT','ATPKSMB', 'ATPKSHOH', 'ATMOPRA', 'DSS-43', 'CEDUNA', 'HOBART']):
            ret = s.convert_flux(insitu=insitu)
            
        else:
            # Unknown telescope type
            #print "Unknown telescope - cannot convert"
            casalog.post( "Unknown telescope - cannot convert", priority = 'WARN' )

    return ret
    
def save(s, outfile, outform, overwrite):
    assert_outfile_canoverwrite_or_nonexistent(outfile,
                                               outform,
                                               overwrite)
    outform_local = outform.upper()
    if outform_local == 'MS': outform_local = 'MS2'
    if outform_local not in ['ASAP','ASCII','MS2','SDFITS']:
        outform_local = 'ASAP'

    outfilename = get_abspath(outfile)
    if overwrite and os.path.exists(outfilename):
        os.system('rm -rf %s' % outfilename)

    s.save(outfile, outform_local, overwrite)

    if outform_local!='ASCII':
        casalog.post('Wrote output %s file %s'%(outform_local,outfile))

def doopacity(s, tau):
    antennaname = s.get_antennaname()
    if (tau > 0.0):
        if (antennaname != 'GBT'):
            s.recalc_azel()
        s.opacity(tau, insitu=True)

def dochannelrange(s, channelrange):
    # channel splitting
    if ( channelrange != [] ):
        if ( len(channelrange) == 1 ):
            #print "Split spectrum in the range [%d, %d]" % (0, channelrange[0])
            casalog.post( "Split spectrum in the range [%d, %d]" % (0, channelrange[0]) )
            s._reshape( 0, int(channelrange[0]) )
        else:
            #print "Split spectrum in the range [%d, %d]" % (channelrange[0], channelrange[1])
            casalog.post( "Split spectrum in the range [%d, %d]" % (channelrange[0], channelrange[1]) )
            s._reshape( int(channelrange[0]), int(channelrange[1]) )


def doaverage(s, scanaverage, timeaverage, tweight, polaverage, pweight,
              averageall=False, docopy=False):
    # Average in time if desired
    sret = None
    if ( timeaverage ):
        if tweight=='none':
            errmsg = "Please specify weight type of time averaging"
            raise Exception,errmsg
        stave=sd.average_time(s,weight=tweight,scanav=scanaverage,compel=averageall)
        # Now average over polarizations;
        if ( polaverage ):
            if pweight=='none':
                errmsg = "Please specify weight type of polarization averaging"
                raise Exception,errmsg
            np = len(stave.getpolnos())
            if ( np > 1 ):
                sret=stave.average_pol(weight=pweight)
            else:
                # only single polarization
                #print "Single polarization data - no need to average"
                casalog.post( "Single polarization data - no need to average" )
                sret = stave
        else:
            sret = stave
        #    spave=stave.copy()
        
    else:
        #if ( scanaverage ):
        #        # scan average if the input is a scantable
        #        spave=sd.average_time(scal,weight=pweight,scanav=True)
        #        scal=spave.copy()
        if ( polaverage ):
            if pweight=='none':
                errmsg = "Please specify weight type of polarization averaging"
                raise Exception,errmsg
            np = s.npol()
            if ( np > 1 ):
                if not scanaverage:
                    sret = sd.average_time(s,weight=pweight)
                else:
                    sret = s
                sret=sret.average_pol(weight=pweight)
            else:
                # only single polarization
                #print "Single polarization data - no need to average"
                casalog.post( "Single polarization data - no need to average" )
                #spave=scal.copy()
                sret = s
        else:
            if scanaverage:
                sret=sd.average_time(s,scanav=True)
            else:
                #spave=scal.copy()
                sret = s
    if docopy and (sret == s):
        sret = s.copy()
    return sret

def plot_scantable(s, pltfile, plotlevel, comment=None):
    # reset plotter
    if sd.plotter._plotter:
        sd.plotter._plotter.quit()
    visible = sd.plotter._visible
    sd.plotter.__init__(visible=visible)
    # each IF is separate panel, pols stacked
    sd.plotter.set_mode(stacking='p',panelling='i',refresh=False)
    sd.plotter.set_histogram(hist=True,linewidth=1,refresh=False)
    sd.plotter.plot(s)
    if comment is not None:
        # somehow I need to put text() twice in order to the second
        # text() actually displays on the plot...
        sd.plotter.text(0.0, 1.0,'',coords='relative')
        #sd.plotter.text(0.0, 1.0,'Raw spectra', coords='relative')
        sd.plotter.text(0.0, 1.0,comment, coords='relative')
    #sd.plotter.axhline(color='r',linewidth=2)
    if ( plotlevel < 0 ):
        # Hardcopy - currently no way w/o screen display first
        #pltfile=project+'_'+suffix+'.eps'
        sd.plotter.save(pltfile)

def scantable_restore_factory(s, infile, fluxunit, specunit, frame, doppler, restfreq=''):
    storage = sd.rcParams['scantable.storage']
    isscantable = is_scantable(infile)
    if storage == 'memory' or isscantable == False:
        return scantable_restore_null(s, fluxunit, specunit, frame, doppler, restfreq)
    else:
        return scantable_restore_impl(s, fluxunit, specunit, frame, doppler, restfreq)

class scantable_restore_interface(object):
    def __init__(self, s=None, fluxunit=None, specunit=None, frame=None, doppler=None, restfreq=None):
        pass

    def restore(self):
        raise NotImplementedError('scantable_restore.restore() is not implemented')

class scantable_restore_null(scantable_restore_interface):
    def __init__(self, s, fluxunit, specunit, frame, doppler, restfreq=''):
        super(scantable_restore_null,self).__init__()

    def restore(self):
        pass
    
        
class scantable_restore_impl(scantable_restore_interface):
    def __init__(self, s, fluxunit, specunit, frame, doppler, restfreq=''):
        super(scantable_restore_impl,self).__init__()
        self.scntab = s
        self.fluxunit = s.get_fluxunit()
        self.specunit = s.get_unit()
        self.coord = s._getcoordinfo()
        self.frame = self.coord[1]
        self.doppler = self.coord[2]
        self.molids = s._getmolidcol_list()
        self.rfset = ((restfreq != '') and (restfreq != []))
        self.frameset = frame != '' or frame != self.frame
        self.dopplerset = doppler != '' or doppler != self.doppler
        self.fluxset = self.fluxunit != '' and \
                       (fluxunit != '' or fluxunit != self.fluxunit)
        self.specset = specunit != '' or specunit != self.specunit
        self.restore_not_done = True

    def __del__(self):
        # do restore when the instance is deleted
        self.restore()

    def restore(self):
        if self.restore_not_done:
            self.scntab.set_selection()
        
            casalog.post('Restoreing header information in input scantable')
            self._restore()

        self.restore_not_done = False
                         
    def _restore(self):
        if self.fluxset:
            self.scntab.set_fluxunit(self.fluxunit)
        if self.specset:
            self.scntab.set_unit(self.specunit)
        if self.dopplerset:
            self.scntab.set_doppler(self.doppler)
        if self.frameset:
            self.scntab.set_freqframe(self.frame)
        if self.rfset:
            self.scntab._setmolidcol_list(self.molids)

def interactive_mask(s, masklist, invert=False, purpose=None):
    new_mask = init_interactive_mask(s, masklist, invert)
    msk = get_interactive_mask(new_mask, purpose)
    finalize_interactive_mask(new_mask)
    del new_mask
    return msk

def init_interactive_mask(s, masklist, invert=False):
    new_mask = sd.interactivemask(scan=s)
    if (len(masklist) > 0):
        new_mask.set_basemask(masklist=masklist,invert=invert)
    new_mask.select_mask(once=False,showmask=True)
    return new_mask

def get_interactive_mask(obj, purpose=None):
    # Wait for user to finish mask selection
    finish=raw_input("Press return %s.\n"%(purpose))
    return obj.get_mask()

def finalize_interactive_mask(obj):
    obj.finish_selection()
    
def get_plotter(plotlevel=0):
    from matplotlib import rc as rcp
    rcp('lines', linewidth=1)
    from asap.asapplotter import new_asaplot
    visible = sd.rcParams['plotter.gui']
    if plotlevel > 0 and (not visible):
        casalog.post("GUI plot not available", priority = "ERROR")
##     visible = (plotlevel > 0) if plotlevel else sd.rcParams['plotter.gui']
    plotter = new_asaplot(visible=visible)
    return plotter

def get_nx_ny(n):
    nl = _to_list(n, int)
    if len(nl) == 1:
        nx = ny = nl[0]
    else:
        nx = nl[0]
        ny = nl[1]
    return (nx,ny)

def get_cellx_celly(c,unit='arcsec'):
    if isinstance(c, str):
        cellx = celly = c
    #elif isinstance(c, list) or isinstance(c, np.ndarray):
    elif type(c) in (list, tuple, np.ndarray):
        if len(c) == 1:
            cellx = celly = __to_quantity_string(c[0],unit)
        elif len(c) > 1:
            cellx = __to_quantity_string(c[0],unit)
            celly = __to_quantity_string(c[1],unit)
        else:
            cellx = celly = ''
    else:
        cellx = celly = __to_quantity_string(c,unit)
    return (cellx, celly)
                
def get_map_center(c,frame='J2000',unit='rad'):
    map_center = ''
    if isinstance(c, str):
        if len(c) > 0:
            s = c.split()
            if len(s) == 2:
                map_center = 'J2000 '+c
            elif len(s) == 3:
                if s[0].upper() != 'J2000':
                    raise ValueError, 'Currently only J2000 is supported'
                map_center = c
            else:
                raise ValueError, 'Invalid map center: %s'%(c)
    else:
        l = [frame]
        for i in xrange(2):
            if isinstance(c[i], str):
                l.append(c[i])
            else:
                l.append('%s%s'%(c[i],unit))
        map_center = string.join(l)
    return map_center

def __to_quantity_string(v,unit='arcsec'):
    if isinstance(v, str):
        return v
    else:
        return '%s%s'%(v,unit)

def get_subtable_name(v):
    return v.replace('Table:','').strip()

def read_factor_file(filename):
    factor_list = []
    with open(filename, 'r') as f:
        for line in f:
            split_line = line.split()
            nelem = len(split_line)
            factor = [0] * nelem
            for j in xrange(nelem):
                factor[j] = float(split_line[j])
            factor_list.append(factor)
    return factor_list


#
# The following functions are for timerange selection (CAS-5496)
#
def split_timerange(timerange, separator):
    return [s.strip() for s in timerange.split(separator)
            if not (s.isspace() or len(s) == 0)]

def split_date_string(date_string, full=True):
    split_by_slash = date_string.split('/')
    split_by_colon = split_by_slash[-1].split(':')
    if full:
        split_by_comma = split_by_colon[-1].split('.')
    else:
        split_by_comma = [split_by_colon[-1]]
    elements_list = [element for element in
                     split_by_slash[:-1] + split_by_colon[:-1] + split_by_comma
                     if len(element) > 0]
    if full and len(elements_list) > 1 and date_string.find('.') == -1:
        elements_list += ['0']
    return elements_list
        
def get_full_description(date_string, year='YYYY', month='MM', day='DD', hour='hh', minute='mm', second='ss', subsecond='ff', default=None):
    number_of_slashes = date_string.count('/')
    number_of_colons = date_string.count(':')

    elements_list = split_date_string(date_string)

    template = string.Template('$year/$month/$day/$hour:$min:$sec.$subsec')
    keys = ['year', 'month', 'day', 'hour', 'min', 'sec', 'subsec']

    if default is None:
        default_values = [year, month, day, hour, minute, second, subsecond]
    else:
        default_values = split_date_string(default)
        if len(default_values) < 7:
            default_values = defalut_values + ['00']
            
    values = default_values[:7-len(elements_list)] + elements_list

    return template.safe_substitute(**dict(zip(keys, values)))

def to_datetime(date):
    date_elements_list = split_date_string(date, full=False)
    date_list = map(int, date_elements_list[:-1]) + [float(date_elements_list[-1])]
    t = datetime.datetime(date_list[0], date_list[1], date_list[2],
                          date_list[3], date_list[4], int(date_list[5]),
                          int((date_list[5]-int(date_list[5]))*1e6))
    return t

def to_timedelta(delta):
    delta_elements_list = split_date_string(delta, full=False)
    delta_list = map(int, delta_elements_list[:-1]) + [float(delta_elements_list[-1])]
    dummy1 = datetime.datetime(1999, 1, 1)
    dummy2 = datetime.datetime(1999, 1, 1,
                               delta_list[0], delta_list[1], int(delta_list[2]),
                               int((delta_list[2]-int(delta_list[2]))*1e6))
    dt = dummy2 - dummy1
    return dt

def add_time(date, delta):
    t = to_datetime(date)
    dt = to_timedelta(delta)
    return t+dt    

def sub_time(date, delta):
    t = to_datetime(date)
    dt = to_timedelta(delta)
    return t-dt

def select_by_timerange(data, timerange):
    tb = gentools(['tb'])[0]
    qa = qatool()

    # first get default time and interval
    if data is not None:
        tb.open(data)
        irow = 0
        while (irow < tb.nrows() and
               (tb.getcell('FLAGROW', irow) != 0
               or all(tb.getcell('FLAGTRA', irow) != 0))):
            irow = irow + 1
        irow %= tb.nrows()
        default_mjd = tb.getcell('TIME', irow)
        default_interval = tb.getcell('INTERVAL', irow)
        tb.close()
    else:
        default_mjd = 0.0
        defalut_interval = 0.0

    qdate = qa.quantity(default_mjd, 'd')
    date_dict = qa.splitdate(qdate)
    parameters = {'year': str(date_dict['year']),
                  'month': str(date_dict['month']),
                  'day': str(date_dict['monthday']),
                  'hour': str(date_dict['hour']),
                  'minute': str(date_dict['min']),
                  'second': str(date_dict['sec']),
                  'subsecond': str(date_dict['usec'])}    
    
    if re.match('.+~.+', timerange):
        # This is case 1: 'T0~T1'
        dates_list = split_timerange(timerange, '~')
        first_date = get_full_description(dates_list[0], **parameters)
        second_date = get_full_description(dates_list[1], default=first_date)
        taql = 'TIME >= MJD(DATETIME("%s")) && TIME <= MJD(DATETIME("%s"))'%(first_date, second_date)
    elif re.match('.+\+.+', timerange):
        # This is case 3: 'T0+dT'
        dates_list = split_timerange(timerange, '+')
        first_date = get_full_description(dates_list[0], **parameters)
        delta_time = dates_list[1]
        second_date_datetime = add_time(first_date, delta_time)
        second_date = second_date_datetime.strftime('%Y/%m/%d/%H:%M:%S')
        microsec = '%s'%(second_date_datetime.microsecond/1e6)
        second_date += microsec.lstrip('0')
        taql = 'TIME >= MJD(DATETIME("%s")) && TIME <= MJD(DATETIME("%s"))'%(first_date, second_date)
    elif re.match('^ *>.+', timerange):
        # This is case 4: '>T0'
        dates_list = split_timerange(timerange, '>')
        first_date = get_full_description(dates_list[0], **parameters)
        taql = 'TIME > MJD(DATETIME("%s"))'%(first_date)
    elif re.match('^ *<.+', timerange):
        # This is case 5: '<T0'
        dates_list = split_timerange(timerange, '<')
        first_date = get_full_description(dates_list[0], **parameters)
        taql = 'TIME < MJD(DATETIME("%s"))'%(first_date)
    elif re.match('^[0-9/:.]+$', timerange):
        # This is case 2: 'T0'
        middle_date = get_full_description(timerange, **parameters)
        hours = int(0.5 * default_interval / 3600.0)
        minutes = int((0.5 * default_interval - hours * 3600.0) / 60.0)
        seconds = (0.5 * default_interval) % 60.0
        delta_time = '%d:%d:%s'%(hours, minutes, seconds)
        first_date_datetime = sub_time(middle_date, delta_time)
        second_date_datetime = add_time(middle_date, delta_time)
        first_date = first_date_datetime.strftime('%Y/%m/%d/%H:%M:%S')
        microsec = '%s'%(first_date_datetime.microsecond/1e6)
        first_date += microsec.lstrip('0')
        second_date = second_date_datetime.strftime('%Y/%m/%d/%H:%M:%S')
        microsec = '%s'%(second_date_datetime.microsecond/1e6)
        second_date += microsec.lstrip('0')
        taql = 'TIME >= MJD(DATETIME("%s")) && TIME <= MJD(DATETIME("%s"))'%(first_date, second_date)
    else:
        # invalid format
        casalog.post('WARNING: timerange="%s" is invalid'%(timerange), priority='WARN')
        taql = ''

    casalog.post('taql for timerange: \'%s\''%(taql), priority='DEBUG')

    return taql
