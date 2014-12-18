import numpy
from taskinit import *

def tsdbaseline(infile, datacolumn, antenna, field, spw, timerange, scan, pol, maskmode, thresh, avg_limit, edge, blmode, dosubtract, blparam, blformat, bloutput, bltable, blfunc, order, npiece, applyfft, fftmethod, fftthresh, addwn, rejwn, clipthresh, clipniter, verify, verbose, showprogress, minnrow, outfile, overwrite):
    print 'tsdbaseline...'
