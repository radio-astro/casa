"""
Using tools with timeit seems to be a real pain, so this is a simpler
alternative, although likely a little less accurate.
"""

import time

def benchmark(func, args, nreps=5, nperrep=1):
    """
    Run func(*args) nreps times and report how much CPU time it took.  args is
    a tuple of the parameters to pass to func.  Remember that (single_item)
    doesn't cut it - use (single_item,) as in benchmark(listvis, (vis,), 7).
    In interactive cases like that, just hit the keys as normal.

    nreps should be a small integer > 1 to combat random error (you probably
    want the minimum time).

    nperrep is an integer >= 1 used to improve the timing precision on fast
    funcs.
    """
    meantime = 0.0
    maxtime =  0.0
    print "Run",
    for i in xrange(nreps):
        print i + 1,
        sys.stdout.flush()
        t0 = time.clock()
        for j in xrange(nperrep):
            dummy = func(*args)
        cputime = (time.clock() - t0) / float(nperrep)
        meantime += (cputime - meantime) / (i + 1.0)
        if i < 1:
            mintime = cputime
        else:
            mintime = min(mintime, cputime)
        maxtime = max(maxtime, cputime)
    if nreps > 1:
        print "\nMin CPU time:  %.2gs" % mintime
        print "Mean CPU time: %.2gs" % meantime
        print "Max CPU time:  %.2gs" % maxtime
    else:
        print "\nCPU time: %.2gs" % mintime
