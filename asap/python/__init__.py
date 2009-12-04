"""
This is the ATNF Single Dish Analysis package.

"""
import os,sys,shutil, platform

# Set up CASAPATH and first time use of asap i.e. ~/.asap/*
plf = None
if sys.platform == "linux2":
    if platform.architecture()[0] == '64bit':
        plf = 'linux_64b'
    else:
        plf = 'linux_gnu'
elif sys.platform == 'darwin':
    plf = 'darwin'
else:
    # Shouldn't happen - default to linux
    plf = 'linux'
asapdata = __path__[-1]
# Allow user defined data location
if os.environ.has_key("ASAPDATA"):
    if os.path.exists(os.environ["ASAPDATA"]):
        asapdata = os.environ["ASAPDATA"]
# use CASAPATH if defined and "data" dir present
if not os.environ.has_key("CASAPATH") or \
        not os.path.exists(os.environ["CASAPATH"].split()[0]+"/data"):
    os.environ["CASAPATH"] = "%s %s somwhere" % ( asapdata, plf)
# set up user space
userdir = os.environ["HOME"]+"/.asap"
if not os.path.exists(userdir):
    print 'First time ASAP use. Setting up ~/.asap'
    os.mkdir(userdir)
    #shutil.copyfile(asapdata+"/data/ipythonrc-asap", userdir+"/ipythonrc-asap")
    # commented out by TT on 2009.06.23 for casapy use
    ##shutil.copyfile(asapdata+"/data/ipy_user_conf.py", 
    ##                userdir+"/ipy_user_conf.py")
    f = file(userdir+"/asapuserfuncs.py", "w")
    f.close()
    f = file(userdir+"/ipythonrc", "w")
    f.close()
# commented out by TT on 2009.06.23 for casapy use
##else:
    # upgrade to support later ipython versions
    ##if not os.path.exists(userdir+"/ipy_user_conf.py"):
    ##    shutil.copyfile(asapdata+"/data/ipy_user_conf.py", 
    ##                    userdir+"/ipy_user_conf.py")

# remove from namespace
del asapdata, userdir, shutil, platform

def _validate_bool(b):
    'Convert b to a boolean or raise'
    bl = b.lower()
    if bl in ('f', 'no', 'false', '0', 0): return False
    elif bl in ('t', 'yes', 'true', '1', 1): return True
    else:
        raise ValueError('Could not convert "%s" to boolean' % b)

def _validate_int(s):
    'convert s to int or raise'
    try: return int(s)
    except ValueError:
        raise ValueError('Could not convert "%s" to int' % s)

def _asap_fname():
    """
    Return the path to the rc file

    Search order:

     * current working dir
     * environ var ASAPRC
     * HOME/.asaprc

    """

    fname = os.path.join( os.getcwd(), '.asaprc')
    if os.path.exists(fname): return fname

    if os.environ.has_key('ASAPRC'):
        path =  os.environ['ASAPRC']
        if os.path.exists(path):
            fname = os.path.join(path, '.asaprc')
            if os.path.exists(fname):
                return fname

    if os.environ.has_key('HOME'):
        home =  os.environ['HOME']
        fname = os.path.join(home, '.asaprc')
        if os.path.exists(fname):
            return fname
    return None


defaultParams = {
    # general
    'verbose'             : [True, _validate_bool],
    'useplotter'          : [True, _validate_bool],
    'insitu'              : [True, _validate_bool],

    # plotting
    'plotter.gui'         : [True, _validate_bool],
    'plotter.stacking'    : ['p', str],
    'plotter.panelling'   : ['s', str],
    'plotter.colours'     : ['', str],
    'plotter.linestyles'  : ['', str],
    'plotter.decimate'    : [False, _validate_bool],
    'plotter.ganged'      : [True, _validate_bool],
    'plotter.histogram'  : [False, _validate_bool],
    'plotter.papertype'  : ['A4', str],
    'plotter.xaxisformatting' : ['asap', str],

    # scantable
    'scantable.save'      : ['ASAP', str],
    'scantable.autoaverage'      : [True, _validate_bool],
    'scantable.freqframe' : ['LSRK', str],  #default frequency frame
    'scantable.verbosesummary'   : [False, _validate_bool],
    'scantable.storage'   : ['memory', str],
    'scantable.history'   : [True, _validate_bool],
    'scantable.reference'      : ['.*(e|w|_R)$', str]
    # fitter
    }

def list_rcparameters():

    print """
# general
# print verbose output
verbose                    : True

# preload a default plotter
useplotter                 : True

# apply operations on the input scantable or return new one
insitu                     : True

# plotting

# do we want a GUI or plot to a file
plotter.gui                : True

# default mode for colour stacking
plotter.stacking           : Pol

# default mode for panelling
plotter.panelling          : scan

# push panels together, to share axislabels
plotter.ganged             : True

# decimate the number of points plotted by a factor of
# nchan/1024
plotter.decimate           : False

# default colours/linestyles
plotter.colours            :
plotter.linestyles         :

# enable/disable histogram plotting
plotter.histogram          : False

# ps paper type
plotter.papertype          : A4

# The formatting style of the xaxis
plotter.xaxisformatting    : 'asap' or 'mpl'

# scantable

# default storage of scantable ('memory'/'disk')
scantable.storage          : memory

# write history of each call to scantable
scantable.history          : True

# default ouput format when saving
scantable.save             : ASAP

# auto averaging on read
scantable.autoaverage      : True

# default frequency frame to set when function
# scantable.set_freqframe is called
scantable.freqframe        : LSRK

# Control the level of information printed by summary
scantable.verbosesummary   : False

# Control the identification of reference (off) scans
# This is has to be a regular expression
scantable.reference         : .*(e|w|_R)$
# Fitter
"""

def rc_params():
    'Return the default params updated from the values in the rc file'

    fname = _asap_fname()

    if fname is None or not os.path.exists(fname):
        message = 'could not find rc file; returning defaults'
        ret =  dict([ (key, tup[0]) for key, tup in defaultParams.items()])
        #print message
        return ret

    cnt = 0
    for line in file(fname):
        cnt +=1
        line = line.strip()
        if not len(line): continue
        if line.startswith('#'): continue
        tup = line.split(':',1)
        if len(tup) !=2:
            #print ('Illegal line #%d\n\t%s\n\tin file "%s"' % (cnt, line, fname))
            asaplog.push('Illegal line #%d\n\t%s\n\tin file "%s"' % (cnt, line, fname))
            print_log('WARN')
            continue

        key, val = tup
        key = key.strip()
        if not defaultParams.has_key(key):
            #print ('Bad key "%s" on line %d in %s' % (key, cnt, fname))
            asaplog.push('Bad key "%s" on line %d in %s' % (key, cnt, fname))
            print_log('WARN')
            continue

        default, converter =  defaultParams[key]

        ind = val.find('#')
        if ind>=0: val = val[:ind]   # ignore trailing comments
        val = val.strip()
        try: cval = converter(val)   # try to convert to proper type or raise
        except ValueError, msg:
            #print ('Bad val "%s" on line #%d\n\t"%s"\n\tin file "%s"\n\t%s' % (val, cnt, line, fname, msg))
            asaplog.push('Bad val "%s" on line #%d\n\t"%s"\n\tin file "%s"\n\t%s' % (val, cnt, line, fname, msg.message))
            print_log('WARN')
            continue
        else:
            # Alles Klar, update dict
            defaultParams[key][0] = cval

    # strip the conveter funcs and return
    ret =  dict([ (key, tup[0]) for key, tup in defaultParams.items()])
    print ('loaded rc file %s'%fname)

    return ret


# this is the instance used by the asap classes
rcParams = rc_params()

rcParamsDefault = dict(rcParams.items()) # a copy

def rc(group, **kwargs):
    """
    Set the current rc params.  Group is the grouping for the rc, eg
    for scantable.save the group is 'scantable', for plotter.stacking, the
    group is 'plotter', and so on.  kwargs is a list of attribute
    name/value pairs, eg

      rc('scantable', save='SDFITS')

    sets the current rc params and is equivalent to

      rcParams['scantable.save'] = 'SDFITS'

    Use rcdefaults to restore the default rc params after changes.
    """

    aliases = {}

    for k,v in kwargs.items():
        name = aliases.get(k) or k
        if len(group):
            key = '%s.%s' % (group, name)
        else:
            key = name
        if not rcParams.has_key(key):
            raise KeyError('Unrecognized key "%s" for group "%s" and name "%s"' % (key, group, name))

        rcParams[key] = v


def rcdefaults():
    """
    Restore the default rc params - the ones that were created at
    asap load time
    """
    rcParams.update(rcParamsDefault)

def _n_bools(n, val):
    return [ val for i in xrange(n) ] 

def _is_sequence_or_number(param, ptype=int):
    if isinstance(param,tuple) or isinstance(param,list):
        if len(param) == 0: return True # empty list
        out = True
        for p in param:
            out &= isinstance(p,ptype)
        return out
    elif isinstance(param, ptype):
        return True
    return False

def _to_list(param, ptype=int):
    if isinstance(param, ptype):
        if ptype is str: return param.split()
        else: return [param]
    if _is_sequence_or_number(param, ptype):
        return param
    return None

def unique(x):
    """
    Return the unique values in a list
    Parameters:
        x:      the list to reduce
    Examples:
        x = [1,2,3,3,4]
        print unique(x)
        [1,2,3,4]
    """
    return dict([ (val, 1) for val in x]).keys()

def list_files(path=".",suffix="rpf"):
    """
    Return a list files readable by asap, such as rpf, sdfits, mbf, asap
    Parameters:
        path:     The directory to list (default '.')
        suffix:   The file extension (default rpf)
    Example:
        files = list_files("data/","sdfits")
        print files
        ['data/2001-09-01_0332_P363.sdfits',
        'data/2003-04-04_131152_t0002.sdfits',
        'data/Sgr_86p262_best_SPC.sdfits']
    """
    if not os.path.isdir(path):
        return None
    valid = "rpf rpf.1 rpf.2 sdf sdfits mbf asap".split()
    if not suffix in valid:
        return None
    files = [os.path.expanduser(os.path.expandvars(path+"/"+f)) for f in os.listdir(path)]
    return filter(lambda x: x.endswith(suffix),files)

# workaround for ipython, which redirects this if banner=0 in ipythonrc
sys.stdout = sys.__stdout__
sys.stderr = sys.__stderr__

# Logging
from asap._asap import Log as _asaplog
global asaplog
asaplog=_asaplog()
if rcParams['verbose']:
    asaplog.enable()
else:
    asaplog.disable()

def print_log(level='INFO'):
    from taskinit import casalog
    log = asaplog.pop()
    #if len(log) and rcParams['verbose']: print log
    if len(log) and rcParams['verbose']: casalog.post( log, priority=level )
    return

def mask_and(a, b):
    assert(len(a)==len(b))
    return [ a[i] & b[i] for i in xrange(len(a)) ]

def mask_or(a, b):
    assert(len(a)==len(b))
    return [ a[i] | b[i] for i in xrange(len(a)) ]

def mask_not(a):
    return [ not i for i in a ]

from asapfitter import fitter
from asapreader import reader
from selector import selector

from asapmath import *
from scantable import scantable
from asaplinefind import linefinder
from linecatalog import linecatalog
from interactivemask import interactivemask

if rcParams['useplotter']:
    try:
        from asapplotter import asapplotter
        gui = os.environ.has_key('DISPLAY') and rcParams['plotter.gui']
        if gui:
            import matplotlib
            matplotlib.use("TkAgg")
        import pylab
        xyplotter = pylab
        plotter = asapplotter(gui)
        del gui
    except ImportError:
        #print "Matplotlib not installed. No plotting available"
        asaplog.post( "Matplotlib not installed. No plotting available")
        print_log('WARN')

__date__ = '$Date: 2009-11-26 21:54:30 -0700 (Thu, 26 Nov 2009) $'.split()[1]
__version__  = '2.3.1 alma'
# nrao casapy specific, get revision number
#__revision__ = ' unknown '
casapath=os.environ["CASAPATH"].split()
#svninfo.txt path 
if os.path.isdir(casapath[0]+'/'+casapath[1]+'/python/2.5/asap'):
    # for casa developer environment (linux or darwin)
    revinfo=casapath[0]+'/'+casapath[1]+'/python/2.5/asap/svninfo.txt'
else:
    # for end-user environments
    if casapath[1]=='darwin':
        revinfo=casapath[0]+'/Resources/python/asap/svninfo.txt'
    else:
        revinfo=casapath[0]+'/lib/python2.5/asap/svninfo.txt'
if os.path.isfile(revinfo):
    f = file(revinfo)
    f.readline()
    revsionno=f.readline()
    f.close()
    del f
    __revision__ = revsionno.rstrip()
else:
    __revision__ = ' unknown '

def is_ipython():
    return '__IP' in dir(sys.modules["__main__"])
if is_ipython():
    def version(): print  "ASAP %s(%s)"% (__version__, __date__)
    def list_scans(t = scantable):
        import types
        globs = sys.modules['__main__'].__dict__.iteritems()
        print "The user created scantables are:"
        sts = map(lambda x: x[0], filter(lambda x: isinstance(x[1], t), globs))
        print filter(lambda x: not x.startswith('_'), sts)
        return

    def commands():
        x = """
    [The scan container]
        scantable           - a container for integrations/scans
                              (can open asap/rpfits/sdfits and ms files)
            copy            - returns a copy of a scan
            get_scan        - gets a specific scan out of a scantable
                              (by name or number)
            drop_scan       - drops a specific scan out of a scantable
                              (by number)
            set_selection   - set a new subselection of the data
            get_selection   - get the current selection object
            summary         - print info about the scantable contents
            stats           - get specified statistic of the spectra in
                              the scantable
            stddev          - get the standard deviation of the spectra
                              in the scantable
            get_tsys        - get the TSys
            get_time        - get the timestamps of the integrations
            get_inttime     - get the integration time
            get_sourcename  - get the source names of the scans
            get_azimuth     - get the azimuth of the scans
            get_elevation   - get the elevation of the scans
            get_parangle    - get the parallactic angle of the scans
            get_unit        - get the current unit
            set_unit        - set the abcissa unit to be used from this
                              point on
            get_abcissa     - get the abcissa values and name for a given
                              row (time)
            get_column_names - get the names of the columns in the scantable
                               for use with selector.set_query
            set_freqframe   - set the frame info for the Spectral Axis
                              (e.g. 'LSRK')
            set_doppler     - set the doppler to be used from this point on
            set_dirframe    - set the frame for the direction on the sky
            set_instrument  - set the instrument name
            set_feedtype    - set the feed type
            get_fluxunit    - get the brightness flux unit
            set_fluxunit    - set the brightness flux unit
            set_sourcetype  - set the type of the source - source or reference
            create_mask     - return an mask in the current unit
                              for the given region. The specified regions
                              are NOT masked
            get_restfreqs   - get the current list of rest frequencies
            set_restfreqs   - set a list of rest frequencies
            shift_refpix    - shift the reference pixel of the IFs
            set_spectrum    - overwrite the spectrum for a given row
            get_spectrum    - retrieve the spectrum for a given
            get_mask        - retrieve the mask for a given
            flag            - flag selected channels in the data
            lag_flag        - flag specified frequency in the data
            save            - save the scantable to disk as either 'ASAP',
                              'SDFITS' or 'ASCII'
            nbeam,nif,nchan,npol - the number of beams/IFs/Pols/Chans
            nscan           - the number of scans in the scantable
            nrow            - the number of spectra in the scantable
            history         - print the history of the scantable
            get_fit         - get a fit which has been stored witnh the data
            average_time    - return the (weighted) time average of a scan
                              or a list of scans
            average_pol     - average the polarisations together.
            average_beam    - average the beams together.
            convert_pol     - convert to a different polarisation type
            auto_quotient   - return the on/off quotient with
                              automatic detection of the on/off scans (closest
                              in time off is selected)
            mx_quotient     - Form a quotient using MX data (off beams)
            scale, *, /     - return a scan scaled by a given factor
            add, +, -       - return a scan with given value added
            bin             - return a scan with binned channels
            resample        - return a scan with resampled channels
            smooth          - return the spectrally smoothed scan
            poly_baseline   - fit a polynomial baseline to all Beams/IFs/Pols
            auto_poly_baseline - automatically fit a polynomial baseline
            recalc_azel     - recalculate azimuth and elevation based on
                              the pointing
            gain_el         - apply gain-elevation correction
            opacity         - apply opacity correction
            convert_flux    - convert to and from Jy and Kelvin brightness
                              units
            freq_align      - align spectra in frequency frame
            invert_phase    - Invert the phase of the cross-correlation
            swap_linears    - Swap XX and YY (or RR LL)
            rotate_xyphase  - rotate XY phase of cross correlation
            rotate_linpolphase - rotate the phase of the complex
                                 polarization O=Q+iU correlation
            freq_switch     - perform frequency switching on the data
            stats           - Determine the specified statistic, e.g. 'min'
                              'max', 'rms' etc.
            stddev          - Determine the standard deviation of the current
                              beam/if/pol
     [Selection]
         selector              - a selection object to set a subset of a scantable
            set_scans          - set (a list of) scans by index
            set_cycles         - set (a list of) cycles by index
            set_beams          - set (a list of) beamss by index
            set_ifs            - set (a list of) ifs by index
            set_polarisations  - set (a list of) polarisations by name
                                 or by index
            set_names          - set a selection by name (wildcards allowed)
            set_tsys           - set a selection by tsys thresholds
            set_query          - set a selection by SQL-like query, e.g. BEAMNO==1
            ( also  get_ functions for all these )
            reset              - unset all selections
            +                  - merge two selections

     [Math] Mainly functions which operate on more than one scantable

            average_time    - return the (weighted) time average
                              of a list of scans
            quotient        - return the on/off quotient
            simple_math     - simple mathematical operations on two scantables,
                              'add', 'sub', 'mul', 'div'
            quotient        - build quotient of the given on and off scans
                              (matched pairs and 1 off - n on are valid)
            merge           - merge a list of scantables

     [Line Catalog]
        linecatalog              - a linecatalog wrapper, taking an ASCII or
                                   internal format table
            summary              - print a summary of the current selection
            set_name             - select a subset by name pattern, e.g. '*OH*'
            set_strength_limits  - select a subset by line strength limits
            set_frequency_limits - select a subset by frequency limits
            reset                - unset all selections
            save                 - save the current subset to a table (internal
                                   format)
            get_row              - get the name and frequency from a specific
                                   row in the table
     [Fitting]
        fitter
            auto_fit        - return a scan where the function is
                              applied to all Beams/IFs/Pols.
            commit          - return a new scan where the fits have been
                              commited.
            fit             - execute the actual fitting process
            store_fit       - store the fit parameters in the data (scantable)
            get_chi2        - get the Chi^2
            set_scan        - set the scantable to be fit
            set_function    - set the fitting function
            set_parameters  - set the parameters for the function(s), and
                              set if they should be held fixed during fitting
            set_gauss_parameters - same as above but specialised for individual
                                   gaussian components
            get_parameters  - get the fitted parameters
            plot            - plot the resulting fit and/or components and
                              residual
    [Plotter]
        asapplotter         - a plotter for asap, default plotter is
                              called 'plotter'
            plot            - plot a scantable
            plot_lines      - plot a linecatalog overlay
            save            - save the plot to a file ('png' ,'ps' or 'eps')
            set_mode        - set the state of the plotter, i.e.
                              what is to be plotted 'colour stacked'
                              and what 'panelled'
            set_selection   - only plot a selected part of the data
            set_range       - set a 'zoom' window [xmin,xmax,ymin,ymax]
            set_legend      - specify user labels for the legend indeces
            set_title       - specify user labels for the panel indeces
            set_abcissa     - specify a user label for the abcissa
            set_ordinate    - specify a user label for the ordinate
            set_layout      - specify the multi-panel layout (rows,cols)
            set_colors      - specify a set of colours to use
            set_linestyles  - specify a set of linestyles to use if only
                              using one color
            set_font        - set general font properties, e.g. 'family'
            set_histogram   - plot in historam style
            set_mask        - set a plotting mask for a specific polarization
            text            - draw text annotations either in data or relative
                              coordinates
            arrow           - draw arrow annotations either in data or relative
                              coordinates
            axhline,axvline - draw horizontal/vertical lines
            axhspan,axvspan - draw horizontal/vertical regions

        xyplotter           - matplotlib/pylab plotting functions

    [Reading files]
        reader              - access rpfits/sdfits files
            open            - attach reader to a file
            close           - detach reader from file
            read            - read in integrations
            summary         - list info about all integrations

    [General]
        commands            - this command
        print               - print details about a variable
        list_scans          - list all scantables created bt the user
        list_files          - list all files readable by asap (default rpf)
        del                 - delete the given variable from memory
        range               - create a list of values, e.g.
                              range(3) = [0,1,2], range(2,5) = [2,3,4]
        help                - print help for one of the listed functions
        execfile            - execute an asap script, e.g. execfile('myscript')
        list_rcparameters   - print out a list of possible values to be
                              put into $HOME/.asaprc
        rc                  - set rc parameters from within asap
        mask_and,mask_or,
        mask_not            - boolean operations on masks created with
                              scantable.create_mask

    Note:
        How to use this with help:
                                         # function 'summary'
        [xxx] is just a category
        Every 'sub-level' in this list should be replaces by a '.' Period when
        using help
        Example:
            ASAP> help scantable # to get info on ths scantable
            ASAP> help scantable.summary # to get help on the scantable's
            ASAP> help average_time

            """
        if rcParams['verbose']:
            try:
                from IPython.genutils import page as pager
            except ImportError:
                from pydoc import pager
            pager(x)
        else:
            print x
        return

def welcome():
    return """Welcome to ASAP v%s (%s) - the ATNF Spectral Analysis Package

Please report any bugs via:
http://svn.atnf.csiro.au/trac/asap/simpleticket

[IMPORTANT: ASAP is 0-based]
Type commands() to get a list of all available ASAP commands.""" % (__version__, __date__)
