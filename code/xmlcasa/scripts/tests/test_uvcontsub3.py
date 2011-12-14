import os
import numpy
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from tests.test_split import check_eq, SplitChecker
import unittest

'''
Unit tests for task uvcontsub3.

Features tested:
  1. uvcontsub3 will use DATA if CORRECTED_DATA is absent.
  2. It gets the right answer for a known line + 0th order continuum.
  3. It gets the right answer for a known line + 4th order continuum.
  4. It gets the right answer for a known line + 0th order continuum,
     even when fitorder = 4.
'''

uvcdatadir = 'unittest/uvcontsub/'  # in SplitChecker.datapath/

class UVContChecker(SplitChecker):
    """
    Base class for uvcontsub3 unit testing.
    """
    need_to_initialize = True
    records = {}

    def do_split(self, corrsel):
        """
        This is only called do_split because it comes from SplitChecker.
        run_task (uvcontsub3 in this case) would have been a better name.
        """
        record = {}
        try:
            print "\nRunning uvcontsub3"
            uvran = uvcontsub3(self.inpms, fitspw='0:0~5;18~23',
                               fitorder=corrsel, async=False)
        except Exception, e:
            print "Error running uvcontsub3"
            raise e
        specms = self.inpms + '.contsub'
        tb.open(specms)
        record['contsub'] = tb.getcell('DATA', 52)
        tb.close()
        shutil.rmtree(specms)
        self.__class__.records[corrsel] = record
        return uvran

class zeroth(UVContChecker):
    inpms = uvcdatadir + 'known0.ms' 
    corrsels = [0]                    # fitorder, not corr selection.

    def test_line_cont(self):
        """Continuum-subtracted data in line-free region"""
        check_eq(self.records[0]['contsub'][:,21],   # RR, LL
                 numpy.array([ 0.+0.j,  0.+0.j]), 0.0001)
    def test_line_line(self):
        """Continuum-subtracted data in line region"""
        check_eq(self.records[0]['contsub'][:,9],   # RR, LL
                 numpy.array([87.+26.j, 31.+20.j]), 0.0001)
        
class fourth(UVContChecker):
    inpms = uvcdatadir + 'known4.ms'
    corrsels = [4]                    # fitorder, not corr selection.

    def test_line(self):
        """Continuum-subtracted data"""
        check_eq(self.records[4]['contsub'],   # [[RR], [LL]]
                 numpy.array([[0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                              30.86956-0.10000j,   34.34782-0.10000j,
                              37.82608-0.10000j,   41.30434-0.10000j,
                              44.78260-0.10000j,   48.26086-0.10000j,
                              48.26086-0.10000j,   44.78260-0.10000j,
                              41.30434-0.10000j,   37.82608-0.10000j,
                              34.34782-0.10000j,   30.86956-0.10000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j],
                              [0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                             -30.86956+0.10000j,  -34.34782+0.10000j,
                             -37.82608+0.10000j,  -41.30434+0.10000j,
                             -44.78260+0.10000j,  -48.26086+0.10000j,
                             -48.26086+0.10000j,  -44.78260+0.10000j,
                             -41.30434+0.10000j,  -37.82608+0.10000j,
                             -34.34782+0.10000j,  -30.86956+0.10000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j,
                               0.00000+0.00000j,    0.00000+0.00000j]]),
                 0.0001)

class combspw(UVContChecker):
    inpms = uvcdatadir + 'combspw.ms'
    corrsels = [0, 1]                    # fitorder, not corr selection.

    def do_split(self, fitord):
        """
        This is only called do_split because it comes from SplitChecker.
        run_task (uvcontsub3 in this case) would have been a better name.
        """
        record = {}
        try:
            print "\nRunning uvcontsub3"
            uvran = uvcontsub3(self.inpms, fitspw='1~10:5~122,15~22:5~122',
                               spw='6~14', combine='spw',
                               fitorder=fitord, async=False)
        except Exception, e:
            print "Error running uvcontsub3"
            raise e
        specms = self.inpms + '.contsub'
        tb.open(specms)
        
        # Same data, different row number as compared to uvcontsub or the input MS.
        record['contsub'] = tb.getcell('DATA', 31)[0][73]
        
        tb.close()
        shutil.rmtree(specms)
        self.__class__.records[fitord] = record
        return uvran

    def test_comb0(self):
        """combspw fitorder=0 line estimate"""
        check_eq(self.records[0]['contsub'], -6.2324+17.9865j, 0.001)

    def test_comb1(self):
        """combspw fitorder=1 line estimate"""
        check_eq(self.records[1]['contsub'], -6.2533+17.6584j, 0.001)
        
def suite():
    return [zeroth, fourth, combspw]
