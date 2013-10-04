from __future__ import with_statement
from contextlib import contextmanager
from glob import glob
import os
import shutil
import tempfile

"""
Version 2 (2009/12/7), prompted by Ger van Diepen's suggestion of using TaQL
instead of tb.getcol().  Requires python >= 2.5.
"""

def list_autocorr(mspat="*[-_.][Mm][Ss]"):
    """
    List MSes that include autocorrelations, and the flagging status of those
    autocorrelations.

    Status codes:
    N: The MS has unflagged autocorrelations.
    R: The MS has autocorrelations, but FLAG_ROW is True for all of them.
    F: The MS has non-rowflagged autocorrelations, but they are still flagged.
    """
    mses = glob(mspat)

    if mses:
        msdict = find_autocorr(mses)
        if msdict:
            print "F MS"
            print "- --"
            mses = msdict.keys()
            mses.sort()
            for m in mses:
                print msdict[m], m
        else:
            print "No autocorrelations found."
    else:
        print "Nothing matching", mspat, "was found."


@contextmanager
def taql(query, tbinst=None, cols=None):
    if not tbinst:
        tbinst = tb
    tmpnam = tempfile.mkdtemp(suffix="_tb", dir='.')
    if cols:
        cols = cols.rstrip() + ' '
        restab = tbinst.query(query, tmpnam, columns=cols)
    else:
        restab = tbinst.query(query, tmpnam)
    try:
        yield restab
    finally:
        restab.close()
        shutil.rmtree(tmpnam)


def find_autocorr(mses):
    """
    Check the list of MSes in mses for ones that include autocorrelations, and
    return the flagging status of those autocorrelations in a dictionary keyed
    by MS name.

    Status codes:
    N: The MS has unflagged autocorrelations.
    R: The MS has autocorrelations, but FLAG_ROW is True for all of them.
    F: The MS has non-rowflagged autocorrelations, but they are still flagged.
    """
    retval = {}
    for currms in mses:
        tb.open(currms)
        with taql('ANTENNA1==ANTENNA2', cols="FLAG_ROW, FLAG") as t1:
            if t1.nrows() > 0:
                retval[currms] = 'N'
                with taql('!FLAG_ROW', t1, cols="FLAG") as t2:
                    if t2.nrows() == 0:
                        retval[currms] = 'R'
                    else:
                        with taql('!all(FLAG)', t2) as t3:
                            if t3.nrows() == 0:
                                retval[currms] = 'F'
        tb.close()
    return retval
