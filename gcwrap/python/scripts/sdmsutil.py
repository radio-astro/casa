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
        #select_params = ['scan', 'pol','beam']
        #for param in select_params:
        #    if hasattr(self, param):
        #        setattr(self, param+'no', getattr(self, param))
        #        #print("renaming self.%s -> self.%sno='%s'" % (param, param, getattr(self, param)))
        #        delattr(self, param)

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
        # attribute for tasks that return any result
        self.result = None
        # 

    def __del__(self, base=sdtask_interface):
        self.cleanup()
        super(sdtask_template,self).__del__()

    def initialize(self):
        # make sure infile(s) exist
        if hasattr(self, 'infile'):
            assert_infile_exists(self.infile)
        elif hasattr(self, 'infiles'):
            if isinstance(self.infiles,str):
                assert_infile_exists(self.infiles)
            else:
                for f in self.infiles:
                    assert_infile_exists(f)
        # check for output table.
        if hasattr(self, 'outfile'):
            self.inplace = False
            if len(self.outfile) > 0:
                self.project = self.outfile
            elif hasattr(self, 'suffix'):
                if hasattr(self, 'infile'):
                    self.project = get_default_outfile_name(self.infile,self.outfile,self.suffix)
                elif hasattr(self, 'infiles'):
                    self.project = get_default_outfile_name(self.infiles[0],self.outfile,self.suffix)
        else: # output to self
            self.inplace = True
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
        
    def format_selection(self, auto_only=True, spwid_only=True):
        """
        Format selection string and define self.selection dictionary. 

        Parameter
            auto_only : set baseline to select only auto-correlation
            spwid_only : set spw selection so that it select spw idices
                         instead of spw idxs and channel ranges.
        """
        attributes = ['antenna', 'scan','spw','pol','beam','field','timerange']
        for a in attributes:
            if not hasattr(self,a): setattr(self,a,"")

        if self.spw.startswith(":"): self.spw = "*"+self.spw

        # spw selection
        if spwid_only:
            myms = gentools(['ms'])[0]
            ms_name = self.infile if hasattr(self, 'infile') else self.infiles[0]
            sel_idx = myms.msseltoindex(vis=ms_name,spw=self.spw,field=self.field,
                                        baseline=self.antenna,time=self.timerange,
                                        scan=self.scan,polarization=self.pol)
            if (len(sel_idx['channel']) > 0):
                spwid = [ chanarr[0] for chanarr in sel_idx['channel'] ]
                spw_sel = str(",").join(spwid)
            else: #no spw selection
                spw_sel = ""
        else: spw_sel = self.spw
        # baseline selection
        if auto_only:
            if self.antenna.strip() in ["", "*", "-1"]:
                baseline = "*&&&"
            else:
                antlist = self.antenna.splt(",")
                autolist = [ aname.strip()+"&&&" for aname in antlist ]
                baseline = str(",").join(autolist)
        else:
            baseline = self.antenna
        # define selection dictionary
        #NOTE: beam selection is not available yet!!!
        self.selection = dict(scan=self.scan, polarization=self.pol,
                              spw=spw_sel, field=self.field, time=self.timerange,
                              baseline=baseline, beam=self.beam)

    def assert_no_channel_selection_in_spw(self, mode='warn'):
        """
        Assert 'spw' does not have channel selection
        Returns True if spw string does not have channel selecton
        Returns False or raises an error if spw has channel selection

        Available modes are
            'result' : just returns the result (true or false)
            'warn'   : warn user if channel selection is set
            'error'  : raise an error if channel seledtion is set
        """
        if not hasattr(self, 'spw'): return True
        # find pattern spw = 'spwID:channelRange'
        has_chan = (self.spw.find(':') > -1)
        ## TODO: also need to do something with "###Hz" and "###km/s"?
        #quantap = re.compile('[a-z]', re.IGNORECASE)
        #has_chan = has_chan or len(quantap.findall(self.spw))
        if has_chan:
            if mode.upper().startswith('E'):
                raise ValueError, "spw parameter should not contain channel selection."
            elif mode.upper().startswith('W'):
                casalog.post("Channel selection found in spw parameter. It would be ignored", priority='WARN')
        
        return has_chan
        
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


def get_listvalue(value):
    return _to_list(value, int) or []

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
