import casac, os, shutil
from tasks import *
from taskinit import *

ms_root = 'phoenix_test'
input_ms = ms_root + '.ms'

import inspect, string, sys

def description():
    return "Tests time and channel averaging with split using %s." % input_ms

def data():
    return [input_ms]

def time_then_chan_avg(inms, tbin, chanbin, outms="", zaptemp=True, zaporig=False):
    """
    Bins time and frequency in 1.x passes.  First inms is time averaged to
    in_timebin<tbin>.ms by split with timebin = tbin.  Then in_timebin<tbin>.ms
    (much smaller than inms!) is channel averaged to outms by split with width
    = chanbin.

    outms: Name to use for the output MS.  Defaults to inms - ".ms" +
    "_timebin" + tbin + "_width" + chanbin + ".ms".

    zaptemp: whether or not to delete the intermediate ms.
    zaporig: whether or not to delete the original ms.
    """
    inms_root = inms.rpartition(".ms")[0]
    if not inms_root:
        inms_root = inms
    troot = inms_root + "_timebin" + tbin
    tms = troot + ".ms"
    try:
        split(vis=inms, outputvis=tms, timebin=tbin)
    except Exception, e:
        print "Error", e, "from split(vis=%s, outputvis=%s, timebin=%s)." % (inms, tms, tbin)
        if os.path.isdir(tms):
            print "\t", tms, "has been left on disk for your inspection."
        raise e

    if outms == "":
        outms = "%s_width%d.ms" % (troot, chanbin)
    try:
        split(vis=tms, outputvis=outms, datacolumn='data', width=chanbin)
    except Exception, e:
        print "Error", e, "from split(vis=%s, outputvis=%s, datacolumn='data', width=%s)." % (tms, outms, chanbin)
        raise e
    if zaptemp:
        shutil.rmtree(tms)
    if zaporig:
        print "Warning!  Following your instruction to rm -rf", inms
        shutil.rmtree(inms)

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
        time_then_chan_avg(input_ms, '20s', 4) # The actual run.

        ###### Test disk usage.
        ###### In 32 bit Linux, phoenix_test.ms shrinks from 4880 kB to 1192kB.
        ########## 7/8/2009:                                10640!
        dudict = disk_usage(ms_root + '*.ms')
        print "Disk usage:", dudict

        try:
            squash_factor = dudict[input_ms] / dudict[ms_root + '_timebin20s_width4.ms']
            if squash_factor not in range(3, 16):
                errmsg = "The compression factor, %d, is far outside the expected range.\n" % squash_factor
                errmsg += "%s: %d\n" % (input_ms, dudict[input_ms])
                errmsg += "%s: %d\n" % (ms_root + '_timebin20s_width4.ms', dudict[ms_root + '_timebin20s_width4.ms'])
                raise ValueError, errmsg
        except Exception, e:
            raise Exception, "Error (%s) in checking the before and after disk usage." % e 

        ###### TODO: compare with the internals of a reference output.

    return []

