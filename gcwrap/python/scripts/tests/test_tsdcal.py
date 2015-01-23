import os
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest
#
import listing

from tsdcal import tsdcal 

class tsdcal_test(unittest.TestCase):
    
    """
    Unit test for task tsdcal.

    The list of tests:
    test00   --- default parameters (raises an error)
    """

    # Data path of input
    datapath=os.environ.get('CASAPATH').split()[0]+ '/data/regression/unittest/tsdcal/'

    # Input 
    infile1 = 'uid___A002_X6218fb_X264.ms'
    infiles = [infile1]

    def setUp(self):
        for infile in self.infiles:
            if os.path.exists(infile):
                shutil.rmtree(infile)
            shutil.copytree(self.datapath+infile, infile)

        default(tsdcal)


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
	"""Test1:Check the shapes of TSYS_SPECTRuM and FPARAM"""
	
	tid = "00"
	infile = self.infile1
        tsdcal(infile=infile, calmode='tsys', outfile='out.cal')
        compfile1=infile+'/SYSCAL'
        compfile2='out.cal'
	
	tb.open(compfile1)
	subt1=tb.query('', sortlist='ANTENNA_ID, TIME, SPECTRAL_WINDOW_ID', columns='TSYS_SPECTRUM')
	tsys1=subt1.getcol('TSYS_SPECTRUM')
	tb.close()
	subt1.close()

	tb.open(compfile2)
	subt2=tb.query('', sortlist='ANTENNA1, TIME, SPECTRAL_WINDOW_ID', columns='FPARAM, FLAG')
	tsys2=subt2.getcol('FPARAM')
	flag=subt2.getcol('FLAG')
	
	tb.close()
	subt2.close()

	if tsys1.all() == tsys2.all():
		print ''
		print 'The shape of the MS/SYSCAL/TSYS_SPECTRUM', tsys1.shape
		print 'The shape of the FPARAM extracted with tsdcal', tsys2.shape  
		print 'Both tables are identical. '
	else:
		print 'The shape of the MS/SYSCAL/TSYS_SPECTRUM', tsys1.shape
        	print 'The shape of the FPARAM of the extraction with tsdcal', tsys2.shape
        	print 'Both tables are not identical.'
       
        if flag.all()==0:
		print 'ALL FLAGs are set to zero: OK'


	


def suite():
    return [tsdcal_test]


