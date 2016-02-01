import os
import sys
import shutil
import inspect
import re
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import sha
import time
import numpy

from importasap import importasap

myms = gentools(['ms'])[0]


class importasap_test(unittest.TestCase):
    """
       test_overwrite -- File existence check
       test_invaliddata -- Invalid data check
       test_normal -- Normal data import
    """
    # Input and output names
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/importasap/'
    infile='uid___A002_X85c183_X36f.test.asap'
    prefix='importasap_test'
    outfile=prefix+'.ms'

    def setUp(self):
        self.res=None
        if (not os.path.exists(self.infile)):
            shutil.copytree(self.datapath+self.infile, self.infile)

        default(importasap)

    def tearDown(self):
        if (os.path.exists(self.infile)):
            shutil.rmtree(self.infile)
        os.system( 'rm -rf '+self.prefix+'*' )

    def test_overwrite(self):
        """test_overwrite: File existence check"""
        shutil.copytree(self.infile, self.outfile)
        with self.assertRaisesRegexp(RuntimeError, '.* exists\.$') as cm:
            importasap(infile=self.infile, outputvis=self.outfile, overwrite=False)
    
    def test_invaliddata(self):
        """test_invaliddata: Invalid data check"""
        os.remove(os.path.join(self.infile, 'table.info'))
        with self.assertRaisesRegexp(RuntimeError, '.* is not a valid Scantable\.$') as cm:
            importasap(infile=self.infile, outputvis=self.outfile, overwrite=False)
    
    def test_normal(self):
        """test_normal: Normal data import"""
        ret = importasap(infile=self.infile, outputvis=self.outfile, overwrite=True)
        self.assertTrue(os.path.exists(self.outfile))
        try:
            myms.open(self.outfile)
            myms.close()
        except:
            self.fail('outputvis is not a valid ms')


def suite():
    return [importasap_test]
