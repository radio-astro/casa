import os
import numpy
import sys
import shutil
from __main__ import default
from tasks import *
from taskinit import *
import unittest

import numpy as np

'''
Unit tests for UVFITS I/O tasks.

Features tested:
  0. Can multiple spws with the same # of channels be exported to UVFITS
     using padwithflags?
  1. When that UVFITS file is read back in, is its data still correct?
'''

datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/uvfits/'
datapath2 = os.environ.get('CASAPATH').split()[0] + '/data/regression/'

def check_eq(val, expval, tol=None):
    """Checks that val matches expval within tol."""
    try:
        if tol:
            are_eq = abs(val - expval) < tol
        else:
            are_eq = val == expval
        if hasattr(are_eq, 'all'):
            are_eq = are_eq.all()
        if not are_eq:
            raise ValueError, '!='
    except ValueError:
        raise ValueError, "%r != %r" % (val, expval)


class uvfits_test(unittest.TestCase):
    # 06/13/2010: This seemed to be the only MS in the regression repo
    # that is a good test of padwithflag.
    inpms = 'cvel/input/ANTEN_sort_hann_for_cvel_reg.ms'

    origms = 'start.ms'               # Just a copy of inpms
    fitsfile = 'hanningsmoothed.UVF'
    msfromfits = 'end.ms'
    
    records = {}
    need_to_initialize = True    # Do once, at start.
    do_teardown        = False   # Do once, after initializing and filling records.
                                 # Its value here should not really matter.
    
    def setUp(self):
        pass
        #if self.need_to_initialize:
        #    self.initialize()

    #def initialize(self):
        # The realization that need_to_initialize needs to be
        # a class variable more or less came from
        # http://www.gossamer-threads.com/lists/python/dev/776699
    #    self.__class__.need_to_initialize = False

    #    if not os.path.exists(self.origms):
            # Copying is technically unnecessary for split,
            # but self.self.origms is shared by other tests, so making
            # it readonly might break them.
    #        shutil.copytree(datapath + self.inpms, self.origms)

    #    if os.path.exists(self.fitsfile):
    #        os.remove(self.fitsfile)

     #   try:
     #       exportuvfits(self.origms, self.fitsfile, padwithflags=True)
     #       self.records['exported'] = os.path.exists(self.fitsfile)

      #      if self.records['exported']:
      #          importuvfits(self.fitsfile, self.msfromfits)
     #   except Exception, e:
     #       print "Error exporting or importing uv data"
     #       raise e


    def tearDown(self):
        if self.do_teardown:
            shutil.rmtree(self.origms)
            shutil.rmtree(self.msfromfits)
            os.remove(self.fitsfile)
            self.do_teardown = False

    #def test_sts(self):
    #    """Subtables, time avg. without correlation selection"""
    #    self.check_subtables('', [(4, 1)])
        
    #def test_data(self):
    #    """DATA[2],   time avg. without correlation selection"""
    #    check_eq(self.records['']['data'],
    #             numpy.array([[ 0.14428490-0.03145669j],
    #                          [-0.00379944+0.00710297j],
    #                          [-0.00381106-0.00066403j],
    #                          [ 0.14404297-0.04763794j]]),
    #             0.0001)
        
    #def test_wt(self):
    #    """WEIGHT[5], time avg. without correlation selection"""
    #    check_eq(self.records['']['weight'],
    #             numpy.array([143596.34375, 410221.34375,
    #                          122627.1640625, 349320.625]),
    #             1.0)
    
    def test_stokes(self):
        """Verify fix to CAS_4283, uvfits files containing actual Stokes parameters will not be imported"""
        myms = mstool()
        msname = "my.ms"
        fitsname = datapath + "1331+305_I.UVFITS"
        self.assertRaises(Exception, myms.fromfits, msname, fitsname)

    def test_receptor_angle(self):
        """CAS-7081: Test receptor angle is preserved"""
        myms = mstool()
        msname = datapath + "uvfits_test.ms"
        self.assertTrue(myms.open(msname), "Input dataset not found")
        uvfits = "xyz.uvfits"
        self.assertTrue(myms.tofits(uvfits), "Failed to write uvfits")
        myms.done()
        feed = "/FEED"
        tb.open(msname + feed)
        rec_ang = "RECEPTOR_ANGLE"
        expec = tb.getcol(rec_ang)
        tb.done()
        for i in [0, 1]:
            if i == 0:
                importname = "ke.ms"
                self.assertTrue(myms.fromfits(importname, uvfits), "Failed uvfits import")
                myms.done()
            else:
                importname = "kf.ms"
                importuvfits(fitsfile=uvfits, vis=importname)
            tb.open(importname + feed)
            got = tb.getcol(rec_ang)
            tb.done()
            self.assertTrue(np.max(np.abs(got-expec)) < 1e-7, "Receptor angles not preserved")

    def test_diameters(self):
        """CAS-5818: Verify bogus dish diameters in AN table are not used but normal algorithm is used instead"""
        myms = mstool()
        fitsname = datapath + "CTR_CHI_TR2.RWYCP.2"
        msname = "CAS-5818.ms"
        self.assertTrue(myms.fromfits(msname, fitsname), "Failed to import uvfits file")
        myms.done()
        mymd = msmdtool()
        mymd.open(msname)
        diam = mymd.antennadiameter(-1)
        mymd.done()
        expec = "25m"
        for i in diam.keys():
            self.assertTrue(qa.eq(diam[i], expec), "Unexpected diameter for antenna " + i)

    def test_filename_extensions(self):
        """CAS-7696: Verify we turn off fits filename extension support when necessary"""
        myms = mstool()
        fitsname = datapath + "name+000"
        msname = "CAS-7696.ms"
        self.assertTrue(myms.fromfits(msname, fitsname), "Failed to import uvfits file")
        myms.done()

    def test_export_overwrite(self):
        """CAS-5492: test the overwrite parameter when exporting MSes to uvfits"""
        myms = mstool()
        msname = datapath + "uvfits_test.ms"
        myms.open(msname)
        fitsname = "CAS-5492.uvfits"
        self.assertTrue(myms.tofits(fitsname))
        # fail because overwrite=False
        self.assertRaises(myms.tofits, fitsfile=fitsname, overwrite=False)
        # succeed because overwrite=True
        self.assertTrue(myms.tofits(fitsname, overwrite=True))
        myms.done()
        self.assertFalse(exportuvfits(msname, fitsname, overwrite=False))
        self.assertTrue(exportuvfits(msname, fitsname, overwrite=True))
            
    def test_badscan(self):
        """CAS-10054: Tests intermittent incorrect scan number in last row of single-scan dataset"""
        myms = mstool()
        fitsname = datapath2 + "ngc4826/fitsfiles/3c273.fits7"
        msname = "ngc4826.tutorial.3c273.7.ms"
        self.assertTrue(myms.fromfits(msname, fitsname), "Failed to import uvfits file")
        myms.done()

        mytb = tbtool()
        mytb.open(msname);
        scans=mytb.getcol('SCAN_NUMBER')
        mytb.close()

        nrows=len(scans)

        print 'Last row has scan='+str(scans[nrows-1])+' ; (should be 1).'
        self.assertFalse(scans[nrows-1]==2, "Last row has wrong scan number: "+str(scans[nrows-1]) )
        # the following verifies that _all_ scan numbers are correct (and lists unique values)
        self.assertTrue(sum(scans==1)==nrows, "Unexpected scan number found: "+str(np.unique(scans)) )



def suite():
    return [uvfits_test]        
        
    
