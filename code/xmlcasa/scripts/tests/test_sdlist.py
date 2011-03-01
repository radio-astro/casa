import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
from asap_init import * 
import unittest
#
import listing

asap_init()
from sdlist import sdlist

class sdlist_test(unittest.TestCase):
    """
    Unit test for task sdlist.

    The list of tests:
    test00   --- default parameters (raises an error)
    test01   --- valid input filename and save output as a text file
    test02   --- test scanaverage=True
    test03   --- test overwrite=True
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdlist/'
    # Input and output names
    sdfile1 = 'OrionS_rawACSmod_cal2123.asap'
    sdfile2 = 'OrionS_rawACSmod'
    sdfiles = [sdfile1, sdfile2]
    outroot = 'sdlist_test'
    refroot = "refsdlist"

    def setUp(self):
        for sdfile in self.sdfiles:
            if os.path.exists(sdfile):
                shutil.rmtree(sdfile)
            shutil.copytree(self.datapath+sdfile, sdfile)

        default(sdlist)


    def tearDown(self):
        for sdfile in self.sdfiles:
            if (os.path.exists(sdfile)):
                shutil.rmtree(sdfile)

    def _compareOutFile(self,out,reference):
        self.assertTrue(os.path.exists(out))
        self.assertTrue(os.path.exists(reference),
                        msg="Reference file doesn't exist: "+reference)
        self.assertTrue(listing.compare(out,reference),
                        'New and reference files are different. %s != %s. '
                        %(out,reference))


    def test00(self):
        """Test 0: Default parameters"""
        result=sdlist()
        self.assertFalse(result)

    def test01(self):
        """Test 1: Save output to an ascii file"""
        tid = "01"
        sdfile = self.sdfile1
        listfile = self.outroot+tid+".out"
        result = sdlist(sdfile=sdfile,listfile=listfile)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareOutFile(listfile,self.datapath+self.refroot+tid+".txt")
        
    def test02(self):
        """Test 2: Test scanaverage=True"""
        tid = "02"
        sdfile = self.sdfile1
        listfile = self.outroot+tid+".out"
        result = sdlist(sdfile=sdfile,scanaverage=True,listfile=listfile)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareOutFile(listfile,self.datapath+self.refroot+tid+".txt")

    def test03(self):
        """Test 3: Test overwrite=True"""
        tid = "03"
        sdfile = self.sdfile1
        listfile = self.outroot+tid+".out"
        result = sdlist(sdfile=sdfile,listfile=listfile)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(listfile),
                        msg="Output file doesn't exist after the 1st run.")
        # overwrite 'listfile'
        sdfile = self.sdfile2
        listfile = self.outroot+tid+".out"
        result = sdlist(sdfile=sdfile,listfile=listfile,overwrite=True)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareOutFile(listfile,self.datapath+self.refroot+tid+".txt")
        
        
def suite():
    return [sdlist_test]
