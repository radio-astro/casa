"""This module provides functions to set up resource parameters (rc).
These can be set in a file .asaprc or using functions.
"""
__all__ = ["rc", "list_rcparameters", "rcParams", "rcParamsDefault"]

import os

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
    ## for older Matplotlib version
    #'plotter.axesformatting' : ['mpl', str],
    'plotter.axesformatting' : ['asap', str],

    # scantable
    'scantable.save'      : ['ASAP', str],
    'scantable.autoaverage'      : [True, _validate_bool],
    'scantable.freqframe' : ['LSRK', str],  #default frequency frame
    'scantable.verbosesummary'   : [False, _validate_bool],
    'scantable.storage'   : ['memory', str],
    'scantable.history'   : [True, _validate_bool],
    'scantable.reference'      : ['.*(e|w|_R)$', str],
    'scantable.parallactify'   : [False, _validate_bool]
    # fitter
    }

def list_rcparameters():

    print """
# general
# only valid in asap standard mode not in scripts or casapy
# It will disable exceptions and just print the messages
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

# push panels together, to share axis labels
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
plotter.axesformatting    : 'mpl' (default) or 'asap' (for old versions of matplotlib)

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
scantable.reference        : .*(e|w|_R)$

# Indicate whether the data was parallactified (total phase offest == 0.0)
scantable.parallactify     : False

# Fitter
"""

def rc_params():
    'Return the default params updated from the values in the rc file'
    fname = _asap_fname()

    if fname is None or not os.path.exists(fname):
        ret =  dict([ (key, tup[0]) for key, tup in defaultParams.items()])
        #print message
        #message = 'could not find rc file; returning defaults'
        return ret

    cnt = 0
    for line in file(fname):
        cnt +=1
        line = line.strip()
        if not len(line): continue
        if line.startswith('#'): continue
        tup = line.split(':',1)
        if len(tup) !=2:
            print ('Illegal line #%d\n\t%s\n\tin file "%s"' % (cnt, line, fname))
            #asaplog.push('Illegal line #%d\n\t%s\n\tin file "%s"' % (cnt, line, fname))
            #asaplog.post('WARN')
            continue

        key, val = tup
        key = key.strip()
        if not defaultParams.has_key(key):
            print ('Bad key "%s" on line %d in %s' % (key, cnt, fname))
            #asaplog.push('Bad key "%s" on line %d in %s' % (key, cnt, fname))
            #asaplog.post('WARN')
            continue

        default, converter =  defaultParams[key]

        ind = val.find('#')
        if ind>=0: val = val[:ind]   # ignore trailing comments
        val = val.strip()
        try: cval = converter(val)   # try to convert to proper type or raise
        except ValueError, msg:
            print ('Bad val "%s" on line #%d\n\t"%s"\n\tin file "%s"\n\t%s' % (val, cnt, line, fname, msg))
            #asaplog.push('Bad val "%s" on line #%d\n\t"%s"\n\tin file "%s"\n\t%s' % (val, cnt, line, fname, str(msg)))
            #asaplog.post('WARN')
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
