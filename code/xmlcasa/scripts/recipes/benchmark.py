"""
Using tools with timeit seems to be a real pain, so this is a simpler
alternative, although likely a little less accurate.
"""

import time

def benchmark(func, args, kwargs, nreps=5, nperrep=1):
    """
    Run func(*args, **kwargs) nreps times and report how much CPU time it took.
    
    args: a tuple of the positional parameters to pass to func.  Remember that
          (single_item) doesn't cut it - use (single_item,) as in
          benchmark(listvis, (vis,), 7).  (When benchmarking interactive cases
          like that, just hit the keys when required.)

    kwargs: a dictionary of keyword arguments to pass to func.

    nreps: should be a small integer > 1 to combat random error from other
           processes on the system (you probably want the minimum time).

    nperrep: integer >= 1 used to improve the timing precision on fast
             funcs, i.e. nperrep times will be added and then divided by nperrep.
    """
    meantime = 0.0
    maxtime =  0.0
    print "Run",
    for i in xrange(nreps):
        print i + 1,
        sys.stdout.flush()
        t0 = time.time()
        for j in xrange(nperrep):
            dummy = func(*args, **kwargs)
        wallclocktime = (time.time() - t0) / float(nperrep)
        meantime += (wallclocktime - meantime) / (i + 1.0)
        if i < 1:
            mintime = wallclocktime
        else:
            mintime = min(mintime, wallclocktime)
        maxtime = max(maxtime, wallclocktime)
    if nreps > 1:
        print "\nMin wall clock time:  %.3gs" % mintime
        print "Mean wall clock time: %.3gs" % meantime
        print "Max wall clock time:  %.3gs" % maxtime
    else:
        print "\nWall clock time: %.3gs" % mintime
