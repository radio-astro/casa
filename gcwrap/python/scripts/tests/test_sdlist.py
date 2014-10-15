import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
#
import listing

from sdlist import sdlist

class sdlist_test(unittest.TestCase):
    """
    Unit test for task sdlist.

    The list of tests:
    test00   --- default parameters (raises an error)
    test01   --- valid input filename and save output as a text file
    test02   --- test overwrite=True
    test03   --- test overwrite=False
    test04   --- verify correct handling of flag information 
    """
    # Data path of input/output
    datapath=os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/sdlist/'
    # Input and output names
    infile1 = 'OrionS_rawACSmod_cal2123.asap'
    infile2 = 'OrionS_rawACSmod'
    infile_flag = 'sdlist_flagtest.asap'
    infiles = [infile1, infile2, infile_flag]
    outroot = 'sdlist_test'
    reffile = "refsdlist01.txt"

    def setUp(self):
        for infile in self.infiles:
            if os.path.exists(infile):
                shutil.rmtree(infile)
            shutil.copytree(self.datapath+infile, infile)

        default(sdlist)


    def tearDown(self):
        for infile in self.infiles:
            if (os.path.exists(infile)):
                shutil.rmtree(infile)

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
        infile = self.infile1
        outfile = self.outroot+tid+".out"
        result = sdlist(infile=infile,outfile=outfile)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareOutFile(outfile,self.datapath+self.reffile)
        
    def test02(self):
        """Test 2: Test overwrite=True"""
        tid = "02"
        infile = self.infile2
        outfile = self.outroot+tid+".out"
        result = sdlist(infile=infile,outfile=outfile)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                        msg="Output file doesn't exist after the 1st run.")
        # overwrite 'outfile'
        infile = self.infile1
        outfile = self.outroot+tid+".out"
        result = sdlist(infile=infile,outfile=outfile,overwrite=True)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self._compareOutFile(outfile,self.datapath+self.reffile)
        
    def test03(self):
        """Test 3: Test overwrite=False"""
        tid = "03"
        infile = self.infile2
        outfile = self.outroot+tid+".out"
        result = sdlist(infile=infile,outfile=outfile)
        self.assertEqual(result,None,
                         msg="The task returned '"+str(result)+"' instead of None")
        self.assertTrue(os.path.exists(outfile),
                        msg="Output file doesn't exist after the 1st run.")
        # overwrite 'outfile'
        infile = self.infile1
        outfile = self.outroot+tid+".out"
        try:
            result = sdlist(infile=infile,outfile=outfile,overwrite=False)
            self.assertTrue(False,
                            msg='The task must throw exception')
        except Exception, e:
            pos=str(e).find('Output file \'%s\' exists.'%(outfile))
            self.assertNotEqual(pos,-1,
                                msg='Unexpected exception was thrown: %s'%(str(e)))

    def test04(self):
        """Test 4: Verify correct handling of flag information"""
        tid = "04"
        infile = self.infile_flag
        outfile = self.outroot+tid+".out"
        result = sdlist(infile=infile, outfile=outfile)
        rowinfo = []
        read_rowinfo = False
        with open(outfile, 'r') as f:
            for line in f:
                sline = line.strip()
                if sline.startswith('Data Records:'):
                    sline_elem = sline.split(' ')
                    self.assertEqual(int(sline_elem[len(sline_elem)-2]), 4)
                elif sline.startswith('FREQUENCIES'):
                    read_rowinfo = True
                elif sline.startswith('-----'):
                    if read_rowinfo: read_rowinfo = False

                if read_rowinfo:
                    rowinfo.append(sline)
        self.assertTrue(len(rowinfo) == 5)
        del rowinfo[0:2]
        import re
        import string
        found_flaggedrow = [False, False]
        for i in xrange(len(rowinfo)):
            rowinfo_tmp = re.sub(r' +', ' ', rowinfo[i])
            rowinfo_elem = re.sub(r', +', ',', rowinfo_tmp).split(' ')
            rowinfo_if = rowinfo_elem[1]
            if rowinfo_if == '23':
                rowinfo_pollist = rowinfo_elem[7].translate(string.maketrans('[]','  ')).strip().split(',')
                try:
                    rowinfo_pollist.index('0')
                    found_flaggedrow[0] = True
                except:
                    pass
            if rowinfo_if == '25': found_flaggedrow[1] = True
        self.assertTrue(found_flaggedrow[0] and found_flaggedrow[1])

def suite():
    return [sdlist_test]
