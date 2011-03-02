import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from tests.test_split import check_eq, datapath, SplitChecker
import unittest

'''
Unit tests for task uvcontsub2.

Features tested:
  1. uvcontsub2 will use DATA if CORRECTED_DATA is absent.
'''

class try_datacol(SplitChecker):
    need_to_initialize = True
    inpms = datapath + 'unittest/uvcontsub2/ngc5921corr.ms'
    corrsels = ['']
    records = {}

    def initialize(self):
        self.locms = self.inpms.split('/')[-1]
        os.symlink(self.inpms, self.locms)
        SplitChecker.initialize(self)

    def do_split(self, corrsel):
        """
        The preparatory split has already been run, so just run uvcontsub2.
        """
        record = {}
        try:
            print "\nRunning uvcontsub2"
            uvran = uvcontsub2(self.locms, field='N5921*', fitspw='0:4~6;50~59',
                               combine='', solint='int', fitorder=0, spw='0',
                               want_cont=True, async=False)
        except Exception, e:
            print "Error running uvcontsub2"
            raise e
        for spec in ('cont', 'contsub'):
            specms = self.locms + '.' + spec
            tb.open(specms)
            record[spec] = tb.getcell('DATA', 88)
            tb.close()
            shutil.rmtree(specms)
        os.unlink(self.locms)
        self.__class__.records[corrsel] = record
        return uvran

    def test_cont(self):
        """Continuum data"""
        check_eq(self.records['']['cont'][1][5],
                 -0.14124476909637451+0.23102830350399017j, 0.0001)
    def test_cont(self):
        """Continuum-subtracted data"""
        check_eq(self.records['']['contsub'][0][34],
                 0.08608352392911911-0.35485363006591797j, 0.0001)
        
def suite():
    return [try_datacol]
