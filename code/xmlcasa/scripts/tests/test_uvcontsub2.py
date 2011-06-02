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
Unit tests for task uvcontsub2.

Features tested:
  1. uvcontsub2 will use DATA if CORRECTED_DATA is absent.
  2. It gets the right answer for a known line + 0th order continuum.
  3. It gets the right answer for a known line + 4th order continuum.
  4. It gets the right answer for a known line + 0th order continuum,
     even when fitorder = 4.
'''

uvcdatadir = 'unittest/uvcontsub2/'  # in SplitChecker.datapath/

class UVContChecker(SplitChecker):
    """
    Base class for uvcontsub2 unit testing.
    """
    need_to_initialize = True
    records = {}

    def do_split(self, corrsel):
        """
        This is only called do_split because it comes from SplitChecker.
        run_task (uvcontsub2 in this case) would have been a better name.
        """
        record = {}
        try:
            print "\nRunning uvcontsub2"
            uvran = uvcontsub2(self.inpms, fitspw='0:0~5;18~23',
                               fitorder=corrsel, want_cont=True,
                               async=False)
        except Exception, e:
            print "Error running uvcontsub2"
            raise e
        for spec in ('cont', 'contsub'):
            specms = self.inpms + '.' + spec
            tb.open(specms)
            record[spec] = tb.getcell('DATA', 52)
            tb.close()
            shutil.rmtree(specms)
        self.__class__.records[corrsel] = record
        return uvran

class zeroth(UVContChecker):
    inpms = uvcdatadir + 'known0.ms' 
    corrsels = [0]                    # fitorder, not corr selection.

    def test_cont_cont(self):
        """Continuum estimate in line-free region"""
        check_eq(self.records[0]['cont'][:,3],   # RR, LL
                 numpy.array([ 2.+3.j,  4.+5.j]), 0.0001)
    def test_cont_line(self):
        """Continuum estimate in line region"""
        check_eq(self.records[0]['cont'][:,13],
                 numpy.array([ 2.+3.j,  4.+5.j]), 0.0001)
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

    def test_cont(self):
        """Continuum estimate"""
        check_eq(self.records[4]['cont'],   # [[RR], [LL]]
                 numpy.array([[20.00000-10.j,      12.50660-10.00000j,
                                7.10324-10.00000j,  3.35941-10.j,
                                0.89944-10.j,      -0.59741-10.j,
                               -1.39700-10.j,      -1.71032-10.j,
                               -1.69345-10.j,      -1.44760-10.j,
                               -1.01907-10.j,      -0.39929-10.j,
                                0.47521-10.j,       1.72278-10.j,
                                3.51665-10.j,       6.08496-10.j,
                                9.71073-10.j,      14.73187-10.j,
                               21.54116-10.j,      30.58629-10.j,
                               42.36984-10.j,      57.44926-10.00000j,
                               76.43690-10.00000j,100.00000-10.j],
                             [-10.00000+5.j,       -6.25330+5.j,
                               -3.55162+5.00000j,  -1.67970+5.00000j,
                               -0.44972+5.00000j,   0.29870+5.j,
                                0.69850+5.j,        0.85516+5.j,
                                0.84673+5.j,        0.72380+5.j,
                                0.50953+5.j,        0.19964+5.j,
                               -0.23760+5.j,       -0.86139+5.j,
                               -1.75832+5.j,       -3.04248+5.j,
                               -4.85537+5.j,       -7.36593+5.j,
                              -10.77058+5.j,      -15.29315+5.00000j,
                              -21.18492+5.00000j, -28.72463+5.00000j,
                              -38.21845+5.j,      -50.00000+5.j]]),
                 0.0001)

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

    def do_split(self, corrsel):
        """
        This is only called do_split because it comes from SplitChecker.
        run_task (uvcontsub2 in this case) would have been a better name.
        """
        record = {}
        try:
            print "\nRunning uvcontsub2"
            uvran = uvcontsub2(self.inpms, fitspw='1~10:5~122,15~22:5~122',
                               spw='6~14', combine='spw',
                               fitorder=corrsel, want_cont=False,
                               async=False)
        except Exception, e:
            print "Error running uvcontsub2"
            raise e
        specms = self.inpms + '.contsub'
        tb.open(specms)
        record['contsub'] = tb.getcell('DATA', 52)[0][73]
        tb.close()
        shutil.rmtree(specms)
        self.__class__.records[corrsel] = record
        return uvran

    def test_0ran(self):
        """combspw fitorder=0 line estimate"""
        check_eq(self.records[0]['contsub'], -6.99146+17.53703j,
                 0.001)

    def test_1ran(self):
        """combspw fitorder=1 line estimate"""
        check_eq(self.records[1]['contsub'], -7.0302+17.4318428j,
                 0.001)
        
def suite():
    return [zeroth, fourth, combspw]
