from tasks import *
from taskinit import *
import os

epsilon = 0.0001

def description():
    return "Test of task visstat"

def run():
    vis='ngc5921.ms'
    importuvfits(os.environ.get('CASAPATH').split()[0] + \
             '/data/regression/ngc5921/ngc5921.fits', \
             vis)

    tb.open(vis)
    cols = tb.colnames()
    tb.close()


    #visstat(vis=vis, column=

    for col in cols:
        visstat(vis=vis, column=col)

    for a in range(1, 5):
        s = visstat(vis=vis, column='ANTENNA1', antenna=str(a)+'&26')
        print "antenna =", a, "; mean = ", s['mean']

        # Note there's a counting from 0 or 1 issue here
        # with the antenna numbering
        if abs((s['mean']-1) - a > epsilon):
            raise Exception("Error!")

    for scan in range(1, 8):
        s = visstat(vis=vis, column='SCAN_NUMBER', scan=str(scan))

        print "scan =", scan, "; mean = ", s['mean']
        if abs(s['mean'] - scan > epsilon):
            raise Exception("Error!")

    return []

def data():
    return []
