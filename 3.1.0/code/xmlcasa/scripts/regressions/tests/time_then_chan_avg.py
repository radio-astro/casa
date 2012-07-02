import numpy
import pprint
import os, re, shutil
from tasks import *
from taskinit import *

def description():
    return "Tests time and channel averaging with split."

# The different spw selection strings are designed to test different paths
# through split, not MSSelection.
# Typically channel averaging does not change the # of rows.
expected = {
    # Input MS
    'labelled_by_time+ichan.ms': {  # Has weight spectrum (some rows altered)
       # Time averaging width in s.
       'tav': 20,
       # Channel averaging width in channels.
       'cav': 2,
        # The rest of the keys at this level are chanselstrs.
        '': {
            # Test type
            'nrows_aft_tavg': 1259,
            'nrows_aft_cavg': 1259,
            'datshp':         (1, 4, 1259),
            'cells': {
                #T or C  (col, row)  val, or (val, tolerance)
                'tav': {('FLAG', 0): [False for i in xrange(8)],
                        ('WEIGHT', 0): (2304.0, 0.1),
                        ('EXPOSURE', 0): 18.874367952346802,
                        ('DATA', 1): (numpy.array([[0.0+0.j, 4.5+1.j,
                                                    4.5+2.j, 4.5+3.j,
                                                    4.5+4.j, 4.5+5.j,
                                                    0.0+0.j, 0.0+0.j]]), 0.005),
                        ('WEIGHT_SPECTRUM', 1): (numpy.array([[0.0, 10.,
                                                               20., 30.,
                                                               40., 50.,
                                                               0.,  0.]]), 0.005),
                        ('TIME', 1): (4715114710.4857559, 0.01),
                        ('EXPOSURE', 1): 20.971519947052002,
                        ('FLAG', 1): [False for i in xrange(8)],
                        ('WEIGHT', 1): (150.0, 0.1),
                        ('SIGMA', 1): 0.081649661,
                        ('WEIGHT', 2): (2332.0, 0.05),
                        ('WEIGHT_SPECTRUM', 2): (numpy.array([[288., 289.,
                                                               290., 291.,
                                                               292., 293.,
                                                               294., 295.]]), 0.01),
                        ('FLAG', 3): [False for i in xrange(8)],
                        ('WEIGHT_SPECTRUM', 4): (numpy.array([[64., 32.,
                                                               32., 32.,
                                                               32., 32.,
                                                               32., 32.]]), 0.005),
                        ('ANTENNA2', 5): 7,  # Baseline 0-6 is dropped
                        },
                'cav': {('FLAG', 0): [False for i in xrange(4)],
                        ('WEIGHT', 0): (2304.0, 0.1),
                        ('EXPOSURE', 0): 18.874367952346802,
                        ('FEED1', 0): 0,
                        ('FEED2', 9): 0,
                        ('DATA', 1): (numpy.array([[4.5+1.j, 4.5+2.6j,
                                                    4.5+4.5555556j, 0.0+0.j]]),
                                      0.005),
                        ('WEIGHT_SPECTRUM', 1): (numpy.array([[10., 50., 90., 0.]]),
                                                 0.005),
                        ('TIME', 1): (4715114710.4857559, 0.01),
                        ('EXPOSURE', 1): 20.971519947052002,
                        ('ARRAY_ID', 1): 0,
                        ('FLAG', 1): [False, False, False, True],
                        ('WEIGHT', 1): (150.0, 0.1),
                        ('SIGMA', 1): 0.081649661,
                        ('WEIGHT', 2): (2332.0, 0.05),
                        ('WEIGHT_SPECTRUM', 2): (numpy.array([[577., 581.,
                                                               585., 589.]]), 0.01),
                        ('FLAG', 3): [False for i in xrange(4)],
                        ('WEIGHT_SPECTRUM', 4): (numpy.array([[96., 64.,
                                                               64., 64.]]), 0.005),
                        ('ANTENNA2', 5): 7  # Baseline 0-6 is dropped
                        }}},
        '0': {
            'nrows_aft_tavg': 314,
            'nrows_aft_cavg': 314,
            'datshp':         (1, 4, 314),
            'cells': {
                #T or C  (col, row)  val, or (val, tolerance)
                'tav': {('FLAG', 0): [False for i in xrange(8)],
                        ('WEIGHT', 0): (2304.0, 1.0),
                        ('EXPOSURE', 0): 18.874367952346802,
                        ('DATA', 1): (numpy.array([[0.0+0.j, 4.5+1.j,
                                                    4.5+2.j, 4.5+3.j,
                                                    4.5+4.j, 4.5+5.j,
                                                    0.0+0.j, 0.0+0.j]]), 0.005),
                        ('WEIGHT_SPECTRUM', 1): (numpy.array([[0.0, 10.0,
                                                               20.0, 30.0,
                                                               40.0, 50.0,
                                                               0.,  0.]]), 0.005),
                        ('TIME', 1): (4715114710.4857559, 0.01),
                        ('EXPOSURE', 1): 20.971519947052002,
                        ('FLAG', 1): [False for i in xrange(8)],
                        ('WEIGHT', 1): (150.0, 0.1),
                        ('SIGMA', 1): 0.081649661,
                        ('WEIGHT', 2): (2332.0, 0.05),
                        ('WEIGHT_SPECTRUM', 2): (numpy.array([[288.0, 289.,
                                                               290.,  291.,
                                                               292.,  293.,
                                                               294.,  295.]]),
                                                 0.01),
                        ('FLAG', 3): [False for i in xrange(8)],
                        ('WEIGHT_SPECTRUM', 4): (numpy.array([[64., 32.,
                                                               32., 32.,
                                                               32., 32.,
                                                               32., 32.]]), 0.005),
                        ('ANTENNA2', 5): 7,  # Baseline 0-6 is dropped
                        },
                'cav': {('FLAG', 0): [False for i in xrange(4)],
                        ('WEIGHT', 0): (2304.0, 0.1),
                        ('EXPOSURE', 0): 18.874367952346802,
                        ('FEED1', 0): 0,
                        ('FEED2', 9): 0,
                        ('DATA', 1): (numpy.array([[4.5+1.j, 4.5+2.6j,
                                                    4.5+4.5555556j, 0.0+0.j]]),
                                      0.005),
                        ('WEIGHT_SPECTRUM', 1): (numpy.array([[10., 50., 90., 0.]]),
                                                 0.005),
                        ('TIME', 1): (4715114710.4857559, 0.01),
                        ('EXPOSURE', 1): 20.971519947052002,
                        ('ARRAY_ID', 1): 0,
                        ('FLAG', 1): [False, False, False, True],
                        ('WEIGHT', 1): (150.0, 0.1),
                        ('SIGMA', 1): 0.081649661,
                        ('WEIGHT', 2): (2332.0, 0.05),
                        ('WEIGHT_SPECTRUM', 2): (numpy.array([[577., 581.,
                                                               585., 589.]]), 0.01),
                        ('FLAG', 3): [False for i in xrange(4)],
                        ('WEIGHT_SPECTRUM', 4): (numpy.array([[96., 64.,
                                                               64., 64.]]), 0.005),
                        ('ANTENNA2', 5): 7,  # Baseline 0-6 is dropped
                        }}},
        '2': {  # spw 2 didn't get the manual flagging that spw 0 did.
            'nrows_aft_tavg': 315,
            'nrows_aft_cavg': 315,
            'datshp':         (1, 4, 315),
            'cells': {
                #T or C  (col, row)  val, or (val, tolerance)
                'tav': {('FLAG', 0): [False for i in xrange(8)],
                        ('WEIGHT', 0): (2304.0, 0.1),
                        ('EXPOSURE', 0): 18.874367952346802,
                        ('DATA', 1): (numpy.array([[4.5+0.j, 4.5+1.j,
                                                    4.5+2.j, 4.5+3.j,
                                                    4.5+4.j, 4.5+5.j,
                                                    4.5+6.j, 4.5+7.j]]), 0.005),
                        ('WEIGHT_SPECTRUM', 1): (numpy.array([[320.0 for i in xrange(8)]]), 0.005),
                        ('TIME', 1): (4715114710.4857559, 0.01),
                        ('EXPOSURE', 1): 20.971519947052002,
                        ('FLAG', 1): [False for i in xrange(8)],
                        ('WEIGHT', 1): (2560.0, 0.1),
                        ('SIGMA', 1): 0.019764,
                        ('WEIGHT', 2): (2560.0, 0.05),
                        ('ANTENNA2', 5): 6  # Baseline 0-6 is OK in spw 2.
                        },
                'cav': {('FLAG', 0): [False for i in xrange(4)],
                        ('WEIGHT', 0): (2304.0, 1.0),
                        ('EXPOSURE', 0): 18.874367952346802,
                        ('FEED1', 0): 0,
                        ('FEED2', 9): 0,
                        ('DATA', 1): (numpy.array([[4.5+0.5j, 4.5+2.5j,
                                                    4.5+4.5j, 4.5+6.5j]]), 0.005),
                        ('WEIGHT_SPECTRUM', 1): (numpy.array([[640.0, 640.0,
                                                               640.0, 640.0]]), 0.005),
                        ('TIME', 1): (4715114710.4857559, 0.01),
                        ('EXPOSURE', 1): 20.971519947052002,
                        ('ARRAY_ID', 1): 0,
                        ('WEIGHT', 1): (2560.0, 0.1),
                        ('SIGMA', 1): 0.019764,
                        ('WEIGHT', 2): (2560.0, 0.05)
                        }}},
        '0~2:0~3': {
            'nrows_aft_tavg': 944,
            'nrows_aft_cavg': 944,
            'datshp':         (1, 2, 944),
            'cells': {
                #T or C  (col, row)  val, or (val, tolerance)
                'tav': {('FLAG', 0): [False for i in xrange(4)],
                        ('WEIGHT', 0): (1152.0, 0.1),
                        ('EXPOSURE', 0): 18.8744,
                        ('DATA', 1): (numpy.array([[0.0+0.j, 4.5+1.j,
                                                    4.5+2.j, 4.5+3.j]]), 0.005),
                        ('TIME', 1): (4715114710.4857559, 0.01),
                        ('EXPOSURE', 1): 20.971519947052002,
                        ('WEIGHT', 1): (60.0, 0.1),
                        ('SIGMA', 1): 0.1290994,
                        ('WEIGHT_SPECTRUM', 1): (numpy.array([0.0, 10.0, 20.0, 30.0]),
                                                 0.01),
                        ('WEIGHT', 2): (1158.0, 0.05),
                        ('WEIGHT_SPECTRUM', 4): (numpy.array([[64., 32.,
                                                               32., 32.]]), 0.005),
                        ('ANTENNA2', 5): 7,  # Baseline 0-6 is dropped
                        },
                'cav': {('FLAG', 0): [False, False],
                        ('WEIGHT', 0): (1152.0, 1.0),
                        ('EXPOSURE', 0): 18.8744,
                        ('FEED1', 0): 0,
                        ('FEED2', 9): 0,
                        ('DATA', 1): (numpy.array([[4.5+1.j, 4.5+2.6j]]), 0.005),
                        ('WEIGHT_SPECTRUM', 1): (numpy.array([[10., 50.]]), 0.005),
                        ('TIME', 1): (4715114710.4857559, 0.01),
                        ('EXPOSURE', 1): 20.971519947052002,
                        ('ARRAY_ID', 1): 0,
                        ('FLAG', 1): [False, False],
                        ('WEIGHT', 1): (60.0, 0.1),
                        ('SIGMA', 1): 0.1290994,
                        ('WEIGHT', 2): (1158.0, 0.05),
                        ('WEIGHT_SPECTRUM', 2): (numpy.array([[577.0, 581.0]]),
                                                 0.01),
                        ('FLAG', 3): [False, False], # wt invalid, but wtsp valid
                        ('WEIGHT_SPECTRUM', 4): (numpy.array([[96., 64.]]), 0.005),
                        ('ANTENNA2', 5): 7,  # Baseline 0-6 is dropped
                        }}},
        '0,2:0~3': {
            'nrows_aft_tavg': 629,
            'nrows_aft_cavg': 629,
            'datshp': set([(1, 4, 1), (1, 2, 1)]),
            'cells': {
                #T or C  (col, row)  val, or (val, tolerance)
                'tav': {('FLAG', 0): [False for i in xrange(8)],
                        ('WEIGHT', 0): (2304.0, 0.1),
                        ('EXPOSURE', 0): 18.874367952346802,
                        ('DATA', 1): (numpy.array([[0.0+0.j, 4.5+1.j, 4.5+2.j, 4.5+3.j,
                                                    4.5+4.j, 4.5+5.j, 0.0+0.j, 0+0.j]]),
                                      0.005),
                        ('WEIGHT_SPECTRUM', 1): (numpy.array([[0.0, 10.0, 20.0, 30.0,
                                                               40., 50.,  0., 0.]]),
                                                 0.005),
                        ('TIME', 1): (4715114710.4857559, 0.01),
                        ('EXPOSURE', 1): 20.971519947052002,
                        ('WEIGHT', 1): (150.0, 0.1),
                        ('SIGMA', 1): 0.081649661,
                        ('WEIGHT', 2): (2332.0, 0.05),
                        ('WEIGHT_SPECTRUM', 2): (numpy.array([[288., 289., 290., 291.,
                                                             292., 293., 294., 295.]]),
                                                 0.01),
                        ('FLAG', 3): [False for i in xrange(8)], # wt invalid, but wtsp valid
                        ('WEIGHT_SPECTRUM', 4): (numpy.array([[64., 32.,
                                                               32., 32.,
                                                               32., 32.,
                                                               32., 32.]]), 0.005),
                        ('ANTENNA2', 5): 7,  # Baseline 0-6 is dropped
                        ('DATA_DESC_ID', 44): 1
                        },
                'cav': {('FLAG', 0): [False for i in xrange(4)],
                        ('WEIGHT', 0): (2304.0, 0.1),
                        ('EXPOSURE', 0): 18.874368,
                        ('FEED1', 0): 0,
                        ('FEED2', 9): 0,
                        ('DATA', 1): (numpy.array([[4.5+1.0j, 4.5+2.6j,
                                                    4.5+4.5555553j, 0.0+0.j]]), 0.005),
                        ('WEIGHT_SPECTRUM', 1): (numpy.array([[10., 50., 90., 0.]]),
                                                 0.005),
                        ('TIME', 1): (4715114710.4857559, 0.01),
                        ('EXPOSURE', 1): 20.971519947,
                        ('ARRAY_ID', 1): 0,
                        ('WEIGHT', 1): (150.0, 0.1),
                        ('SIGMA', 1): 0.081649661,
                        ('WEIGHT', 2): (2332.0, 0.05),
                        ('WEIGHT_SPECTRUM', 2): (numpy.array([[577., 581.,
                                                               585., 589.]]), 0.01),
                        ('FLAG', 3): [False for i in xrange(4)], # wt invalid, but wtsp valid
                        ('WEIGHT_SPECTRUM', 4): (numpy.array([[96., 64., 64., 64.]]),
                                                 0.005),
                        ('ANTENNA2', 5): 7,  # Baseline 0-6 is dropped
                        ('DATA_DESC_ID', 134): 1,
                        ('DATA', 134): (numpy.array([[14.462+0.5j, 14.462+2.5j]]), 0.005)
                        }
                }
            }
        }
    }

def isquantized(val):
    """
    Returns whether or not val has a type with quantized values, i.e.
    would 1.1 * val's type differ from val's?
    """
    # try: 1.1 * val ... except: might be more general, but could be expensive.
    retval = True
    if isinstance(val, (float, complex)):
        retval = False
    elif hasattr(val, 'issubdtype'):
        if numpy.issubdtype(val.dtype, float) or numpy.issubdtype(val.dtype, complex):
            retval = False
    return retval


def run():
    input_mses = data()
    badcells = {}
    for input_ms in input_mses:
        tbin = str(expected[input_ms]['tav']) + 's'
        cbin = expected[input_ms]['cav']

        spwsels = expected[input_ms].keys()
        spwsels.remove('tav')
        spwsels.remove('cav')
        spwsels.sort()                      # Prob. not needed.
        for spwsel in spwsels:
            tavms = re.sub(r'.ms$', '', input_ms) + '_%s_timebin%s.ms' % (spwsel, tbin)
            #print "run: tavms =", tavms
            outputms = re.sub(r'.ms$', '', tavms) + '_width%d.ms' % cbin
            #print "run: outputms =", outputms

            for oms in (tavms, outputms):   # Get rid of past problems.
                if os.path.isdir(oms):
                    shutil.rmtree(oms)

            # The actual run.
            try:
                (nrows_aft_tavg,
                 nrows_aft_cavg,
                 shp_aft_cavg) = time_then_chan_avg(input_ms, tbin, cbin,
                                                    outms=outputms,
                                                    chanselstr=spwsel,
                                                    zaptemp=False)
            except Exception, e:
                raise Exception, "Error (%s) running time_then_chan_avg()." % e

            ###### Test # of rows.
            if nrows_aft_tavg != expected[input_ms][spwsel]['nrows_aft_tavg']:
                raise Exception, """
                The number of rows after time averaging (%d) does not match
                the expected number (%d) for input MS %s with selection string "%s"
                """ % (nrows_aft_tavg, expected[input_ms][spwsel]['nrows_aft_tavg'],
                       input_ms, spwsel)

            if nrows_aft_cavg != expected[input_ms][spwsel]['nrows_aft_cavg']:
                raise Exception, """
                The number of rows after channel averaging (%d) does not match
                the expected number (%d) for input MS %s with selection string "%s"
                """ % (nrows_aft_cavg, expected[input_ms][spwsel]['nrows_aft_cavg'],
                       input_ms, spwsel)

            if shp_aft_cavg != expected[input_ms][spwsel]['datshp']:
                raise Exception, """
                The data shape (%s) after channel averaging does not match
                the expectation (%s) for input MS %s with selection string "%s"
                """ % (shp_aft_cavg, expected[input_ms][spwsel]['datshp'],
                       input_ms, spwsel)

            ###### Compare averaged data with expectation, and rm the averaged
            ###### data if it passes.
            outputmses = {'tav': tavms, 'cav': outputms}
            for avtype, avms in outputmses.iteritems():
                expcells = expected[input_ms][spwsel]['cells'][avtype]
                try:
                    tb.open(avms)
                except Exception, e:
                    raise Exception, "Error (%s) opening %s." % (e, avms)
                try:
                    for ((col, row), expent) in expcells.iteritems():
                        usetol = True
                        if isinstance(expent, tuple):
                            tol = expent[1]
                            expent = expent[0]
                        elif not isquantized(expent):
                            tol = 0.005 * expent
                        else:
                            usetol = False
                            
                        gotval = tb.getcell(col, row)
                        if hasattr(gotval, 'flatten') and gotval.size == 1:
                            gotval = gotval.flatten()[0]
                        if usetol:
                            closeenough = (abs(gotval - expent) < tol)
                        else:
                            closeenough = (gotval == expent)
                        if hasattr(closeenough, 'all'):
                            closeenough = closeenough.all()
                        if not closeenough:
                            if not badcells.has_key(avms):
                                badcells[avms] = {}
                            badcells[avms][(col, row)] = (gotval, expent)
                except Exception, e:
                    raise Exception, "Error (%s) checking %s's cell %s." % (e, avms,
                                                                            (col, row))
                finally:
                    tb.close(avms)

                if not badcells.has_key(avms):
                    shutil.rmtree(avms)
    if badcells:
        pp = pprint.PrettyPrinter(indent=0, width=80)
        pp.pprint(badcells)
        raise Exception, "There were unexpected values in the averaged MSes.  Check the log."
    return []


def data():
    """As I understand it, this must return the filenames of needed input data."""
    
    # Just in case order matters.
    inplist = expected.keys()
    inplist.sort()
    
    #return ['split/' + inpms for inpms in inplist]
    return inplist

def time_then_chan_avg(inms, tbin, chanbin, outms="", zaptemp=True,
                       zaporig=False, chanselstr="", datacolstr='corrected'):
    """
    Bins time and frequency in 1.x passes.  First inms is time averaged to
    in_timebin<tbin>.ms by split with timebin = tbin.  Then in_timebin<tbin>.ms
    (much smaller than inms!) is channel averaged to outms by split with width
    = chanbin.

    outms: Name to use for the output MS.  Defaults to inms - ".ms" +
    "_" + chanselstr + "_timebin" + tbin + "_width" + chanbin + ".ms".

    zaptemp: whether or not to delete the intermediate ms.
    zaporig: whether or not to delete the original ms.

    chanselstr: An spw:chan;step channel selection string corresponding to
                split's spw.  Note that it is an error to select fewer
                than chanbin channels in any of the selected input spws.
                Defaulting to averaging all the channels would be reasonable in
                that case, but it would make it hard to detect more serious
                errors.  Not that you should rely on casapy to detect your bad
                selection strings...
    """
    casalog.origin('time_then_chan_avg')
    inms_root = inms.rpartition(".ms")[0]
    if not inms_root:
        inms_root = inms
    inms_root = os.path.basename(inms_root) # Keep outms in local directory.
    troot = inms_root + '_' + chanselstr + "_timebin" + tbin
    tms = troot + ".ms"

    funnyshapes = False
    
    try:
        # Do time averaging.
        datacolstr = 'corrected'
        if (chanselstr.find(':') > -1) and ((chanselstr.find(',') > -1) or
                                            (chanselstr.find(';') > -1)):
            funnyshapes = True

        ## print 'tbin =', tbin
        ## print 'tavspw =', chanselstr
        ## print 'tavdc =', datacolstr
        ## print 'inms =', inms
        casalog.post("Time averaging to " + tms)
        split(vis=inms, outputvis=tms, timebin=tbin, spw=chanselstr,
              datacolumn=datacolstr)
        ms.open(tms)
        nrows_aft_tavg = ms.nrow()
        ms.close()
    except Exception, e:
        casalog.post(
"""Error from split(vis='%s', outputvis='%s',
                 timebin='%s', spw='%s', datacolumn='%s').""" % (inms, tms,
                                                                tbin,
                                                                chanselstr,
                                                                datacolstr),
                     'SEVERE')
        if os.path.isdir(tms):
           casalog.post("\t%s has been left on disk for your inspection." % tms,
                        'SEVERE')
        raise e

    if outms == "":
        outms = "%s_width%d.ms" % (troot, chanbin)
    try:
        # Do channel averaging.
        casalog.post("Channel averaging to " + outms)
        if datacolstr.lower() not in ['all', 'float_data', 'lag_data']:
            datacolstr = 'data'
        split(vis=tms, outputvis=outms, datacolumn=datacolstr, width=chanbin)
        tb.open(outms)
        if funnyshapes:
            dat = tb.getvarcol('DATA')
            nrows_aft_cavg = len(dat)
            shape_aft_cavg = set([d.shape for d in dat.itervalues()])
        else:
            dat = tb.getcol('DATA')
            shape_aft_cavg = dat.shape
            nrows_aft_cavg = shape_aft_cavg[-1]
        tb.close()
    except Exception, e:
        casalog.post(
"""Error from split(vis='%s', outputvis='%s',
                 datacolumn='data', width=%d, spw='%s').""" % (tms, outms,
                                                               chanbin,
                                                               chanselstr),
                     'SEVERE')
        raise e
    if zaptemp:
        shutil.rmtree(tms)
    if zaporig:
        casalog.post("Following your instruction to shutil.rmtree(" + inms + ")", 'WARN')
        shutil.rmtree(inms)
    return nrows_aft_tavg, nrows_aft_cavg, shape_aft_cavg


def disk_usage(pat):
    """
    Returns a dictionary, keyed by path, of the disk usage in kilobytes for
    each path matching the shell filename pattern pat.
    """
    retval = {}
    dudu = os.popen('du -sk ' + pat)
    for outline in dudu:
        try:
            size, path = outline.split() # newline dumped.
            size = int(size)
            retval[path] = size
        except Exception, e:
            print "Error (%s) getting size and path from %s." % (e, outline)
    return retval
