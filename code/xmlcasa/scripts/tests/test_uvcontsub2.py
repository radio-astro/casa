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
'''

class try_zeroth(SplitChecker):
    need_to_initialize = True
    inpms = 'unittest/uvcontsub2/known0.ms' # in SplitChecker.datapath/
    corrsels = ['']
    records = {}

    def initialize(self):
        self.locms = self.inpms.split('/')[-1]
        os.symlink(self.inpms, self.locms)
        SplitChecker.initialize(self)

    def do_split(self, corrsel):
        """
        This is only called do_split because it works with SplitChecker.
        run_task (uvcontsub2 in this case) would have been a better name.
        """
        record = {}
        try:
            print "\nRunning uvcontsub2"
            uvran = uvcontsub2(self.locms, fitspw='0:0~5;18~23',
                               want_cont=True, async=False)
        except Exception, e:
            print "Error running uvcontsub2"
            raise e
        for spec in ('cont', 'contsub'):
            specms = self.locms + '.' + spec
            tb.open(specms)
            record[spec] = tb.getcell('DATA', 52)
            tb.close()
            shutil.rmtree(specms)
        os.unlink(self.locms)
        self.__class__.records[corrsel] = record
        return uvran

    def test_cont_cont(self):
        """Continuum estimate in line-free region"""
        check_eq(self.records['']['cont'][:,3],   # RR, LL
                 numpy.array([ 2.+3.j,  4.+5.j]), 0.0001)
    def test_cont_line(self):
        """Continuum estimate in line region"""
        check_eq(self.records['']['cont'][:,13],
                 numpy.array([ 2.+3.j,  4.+5.j]), 0.0001)
    def test_line_cont(self):
        """Continuum-subtracted data in line-free region"""
        check_eq(self.records['']['contsub'][:,21],   # RR, LL
                 numpy.array([ 0.+0.j,  0.+0.j]), 0.0001)
    def test_line_line(self):
        """Continuum-subtracted data in line region"""
        check_eq(self.records['']['contsub'][:,9],   # RR, LL
                 numpy.array([87.+26.j, 31.+20.j]), 0.0001)
        
def suite():
    return [try_zeroth]
