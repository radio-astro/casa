import casac, os, re, shutil
import inspect, string, sys
from tasks import *
from taskinit import *

ms_root = 'phoenix_test'
input_ms = ms_root + '.ms'

# The different spw selection strings are designed to test different paths
# through split, not MSSelection.
# Typically channel averaging does not change the # of rows.
expected = {'phoenix_test.ms': {'': {'nrows_aft_tavg': 1260,
                                     'nrows_aft_cavg': 1260,
                                     'datshp':         (1, 2, 1260)},
                                '0': {'nrows_aft_tavg': 315,
                                      'nrows_aft_cavg': 315,
                                     'datshp':         (1, 2, 315)},
                                '2': {'nrows_aft_tavg': 315,
                                      'nrows_aft_cavg': 315,
                                      'datshp':         (1, 2, 315)},
                                '0~2:0~3': {'nrows_aft_tavg': 945,
                                            'nrows_aft_cavg': 945,
                                            'datshp':         (1, 1, 945)},
                                '0,2:0~3': {'nrows_aft_tavg': 630,
                                            'nrows_aft_cavg': 630,
                                            'datshp': set([(1, 2, 1), (1, 1, 1)])}
                                }
            }

def description():
    return "Tests time and channel averaging with split using %s." % input_ms

def data():
    return [input_ms]

def time_then_chan_avg(inms, tbin, chanbin, outms="", zaptemp=True, zaporig=False,
                       chanselstr=""):
    """
    Bins time and frequency in 1.x passes.  First inms is time averaged to
    in_timebin<tbin>.ms by split with timebin = tbin.  Then in_timebin<tbin>.ms
    (much smaller than inms!) is channel averaged to outms by split with width
    = chanbin.

    outms: Name to use for the output MS.  Defaults to inms - ".ms" +
    "_timebin" + tbin + "_width" + chanbin + ".ms".

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
    troot = inms_root + "_timebin" + tbin
    tms = troot + ".ms"

    # One annoying thing about having 2 passes of split is that the channel
    # selection string may need to be updated for the 2nd pass.
    timechansel = chanselstr
    cs_simplifications = []

    funnyshapes = False
    
    try:
        # split currently can't do time averaging with differently shaped
        # output spws.  Since this is time averaging followed by channel
        # averaging, simplify the chanselstr for time averaging, even though it
        # means averaging some data which will be rejected in the end.

        datacolstr = 'corrected'
        if (timechansel.find(':') > -1) and ((timechansel.find(',') > -1) or
                                             (timechansel.find(';') > -1)):
            funnyshapes = True
            chanpat = r':\d+(~\d+)?(;\d+)?'
            timechansel = re.sub(chanpat, '', timechansel)
            datacolstr = 'all'
            tb.open(inms)
            cns = tb.colnames()
            tb.close()
            if 'IMAGING_WEIGHT' not in cns:           # So much for
                clearcal(vis=inms, spw=timechansel)   # read-only input!
        
        split(vis=inms, outputvis=tms, timebin=tbin, spw=timechansel,
              datacolumn=datacolstr)
        ms.open(tms)
        nrows_aft_tavg = ms.nrow()
        ms.close()
    except Exception, e:
        casalog.post(
"""Error from split(vis='%s', outputvis='%s',
                 timebin='%s', spw='%s', datacolumn='%s').""" % (inms, tms,
                                                                tbin,
                                                                timechansel,
                                                                datacolstr),
                     'SEVERE')
        if os.path.isdir(tms):
           casalog.post("\t%s has been left on disk for your inspection." % tms,
                        'SEVERE')
        raise e

    if outms == "":
        outms = "%s_width%d.ms" % (troot, chanbin)
    try:
        # Adjust chanselstr to account for the selection already made in the
        # 1st run.
        chanselstr = update_chanselstr(chanselstr, timechansel)
        
        split(vis=tms, outputvis=outms, datacolumn='data', width=chanbin,
              spw=chanselstr)
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
                 datacolumn='data', width=%d, spw='%s').""" % (tms, outms, chanbin,
                                                               chanselstr),
                     'SEVERE')
        raise e
    if zaptemp:
        shutil.rmtree(tms)
    if zaporig:
        casalog.post("Following your instruction to rm -rf %s" % inms, 'WARN')
        shutil.rmtree(inms)
    return nrows_aft_tavg, nrows_aft_cavg, shape_aft_cavg


def update_chanselstr(totchansel, firstchansel):
    """
    When doing a two-step split, the channel selection string (spw) may need to
    be updated for the 2nd run, since the 1st run can remap the spw and channel
    numbers.  For example, if the first run uses "15~18:0~9;3", the output of
    that run no longer has spws called 15 to 18, or all of channels 0 to 9 - it
    has spws 0 to 3 and channels 0 to 3.

    This likely cannot handle all possible spw:chan;step strings, but the same
    goes for split.  Consider yourself warned, and keep it simple.

    totchansel: The spw string that would be used in a single channel selection
                step, i.e. what the user supplies to time_then_chan_avg().

    firstchansel: The spw string that was used by the first run.
    """
    retval = ''
    if totchansel != firstchansel:
        # Need to find all spws in firstchansel, and monotonically map them to
        # range(noutputspws).
        chanpat = r'(:\d+~?\d*;?\d*)' # do not compile
        spwstrs = re.split(r'[,;]', re.sub(chanpat, '', firstchansel))
        outspw = 0
        for spwstr in spwstrs:
            retval += str(outspw)
            spwrange = map(int, spwstr.split('~'))
            if len(spwrange) > 1:
                outspw += spwrange[1] - spwrange[0]
                retval += '~' + str(outspw)
            outspw += 1
            
            # Only insert the channel range part if it was not already handled
            # by firstchansel.
            chanmatch = re.search(spwstr + chanpat, totchansel)
            if chanmatch and not re.search(spwstr + chanpat, firstchansel):
                retval += chanmatch.group(1)
                
            retval += ','
        retval = retval.rstrip(',')
    return retval


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


def run():
    input_mses = data()
    for input_ms in input_mses:
        outputms = input_ms.rstrip('.ms') + '_timebin20s_width4.ms'
        for spwsel in expected[input_ms]:
            # The actual run.
            try:
                (nrows_aft_tavg,
                 nrows_aft_cavg,
                 shp_aft_cavg) = time_then_chan_avg(input_ms, '20s', 4,
                                                    outms=outputms,
                                                    chanselstr=spwsel)
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

            ###### TODO: compare averaged data with expectation.

            shutil.rmtree(outputms)
    return []

