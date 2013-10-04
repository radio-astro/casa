##########################################################################
# imfit_test.py
#
# Copyright (C) 2008, 2009
# Associated Universities, Inc. Washington DC, USA.
#
# This script is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be adressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#                        Charlottesville, VA 22903-2475 USA
#
# <author>
# Dave Mehringer
# </author>
#
# <summary>
# Test suite for the CASA tool method po.rotationmeasure
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the po.rotationmeasure() tool method
# </etymology>
#
# <synopsis>
# Test the po.rotationmeasure() tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/gcwrap/python/scripts/regressions/admin/runUnitTest.py test_po_rotationmeasure[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.tofits() tool method to ensure
# coding changes do not break the associated bits 
# </motivation>
#

###########################################################################
import shutil
import casac
from tasks import *
from taskinit import *
from __main__ import *
import unittest
import scipy.constants

datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/po_tool/'
eq_beams = datapath + "pol_eq_beams.fits"
neq_beams = datapath + "pol_neq_beams.fits"

class po_rotationmeasure_test(unittest.TestCase):
    
    def setUp(self):
        self.mypo = potool()
    
    def tearDown(self):
        self.mypo.done()
        self.assertTrue(len(tb.showcache()) == 0)
    
    def test_multibeam(self):
        """Test multibeam images for correct behavior"""
        mypo = self.mypo
        print eq_beams
        mypo.open(eq_beams)
        self.assertTrue(mypo.rotationmeasure("g"))
        mypo.done()
        mypo.open(neq_beams)
        self.assertRaises(Exception, mypo.rotationmeasure, "hh")
        mypo.done()
        
    def test_rmfit_basics(self):
        """Sanity tests for task rmfit"""
        myia = iatool()
        outfile = "xx.im"
        myia.fromshape(outfile, [20, 20, 4, 20])
        myia.addnoise()
        myia.done()
        myrm = "rm1.im"
        self.assertTrue(rmfit(imagename=outfile, rm=myrm))
        myia.open(myrm)
        self.assertTrue((myia.shape() == [20, 20]).all())
        got1 = myia.statistics(list=True, verbose=True)['sumsq']
        myia.done()
        
        # test concatenation of images
        outfile = "yy.im"
        myia.fromshape(outfile, [20, 20, 4, 20])
        myia.addnoise()
        csys = myia.coordsys()
        refval = csys.referencevalue()['numeric']
        refval[3] = 1.5e9
        csys.setreferencevalue(refval)
        myia.setcoordsys(csys.torecord())
        myia.done()
        images = ["xx.im", "yy.im"]
        myrm = "rm2.im"
        self.assertTrue(rmfit(imagename=images, rm=myrm))
        myia.open(myrm)
        self.assertTrue((myia.shape() == [20, 20]).all())
        got2 = myia.statistics(list=True, verbose=True)['sumsq']
        myia.done()
        self.assertTrue(abs(got1 - got2) > 0.1)

    def test_algorithm(self):
        """Test rotation measure computation algorithm"""
        myia = iatool()
        imagename = "rm_input.im"
        myia.fromshape(imagename, [20, 20, 4, 20])
        csys = myia.coordsys()
        incr = csys.increment()['numeric']
        incr[3] = 1000*incr[3]
        csys.setincrement(incr)
        myia.setcoordsys(csys.torecord())
        pixvals = myia.getchunk()
        # U values all 1
        U = 1
        pixvals[:,:,2,:] = U
        c = scipy.constants.c
        RM = 9.6
        pa0deg = 22.5
        pa0 = pa0deg/180*pi
        for chan in range(myia.shape()[3]):
            freq = myia.toworld([0,0,0,chan])['numeric'][3]
            lam = c/freq
            Q = U/tan(2*(pa0 + RM*lam*lam))
            pixvals[:,:,1,chan] = Q
        myia.putchunk(pixvals)
        myia.done()
        mypo = self.mypo
        rmim = "rm.im"
        pa0im = "pa0.im"
        sigma = 10e-8
        for i in [0, 1]:
            if i == 0:
                mypo.open(imagename)
                mypo.rotationmeasure(rm=rmim, pa0=pa0im, sigma=sigma)
                mypo.done()
            else:
                rmfit(imagename=imagename, rm=rmim, pa0=pa0im, sigma=sigma)
            myia.open(rmim)
            stats = myia.statistics(list=True, verbose=True)
            self.assertTrue((abs(stats['min'][0] - RM)) < 1e-4)
            self.assertTrue((abs(stats['max'][0] - RM)) < 1e-4)
            myia.done(remove=True)
            myia.open(pa0im)
            stats = myia.statistics(list=True, verbose=True)
            self.assertTrue((abs(stats['min'][0] - pa0deg)) < 1e-4)
            self.assertTrue((abs(stats['max'][0] - pa0deg)) < 1e-4)
            myia.done(remove=True)

def suite():
    return [po_rotationmeasure_test]
