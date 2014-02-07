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
# Test suite for the CASA tool method ia.fft
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
# Test for the ia.fft tool method
# </etymology>
#
# <synopsis>
# Test the ia.fft() method.
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_fft[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.fft tool method to ensure
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

class ia_fft_test(unittest.TestCase):
    
    def setUp(self):
        pass
    
    def tearDown(self):
        self.assertTrue(len(tb.showcache()) == 0)
    
    def test_stretch(self):
        """ ia.fft(): Test stretch parameter"""
        yy = iatool()
        mymask = "maskim"
        yy.fromshape(mymask, [20, 20, 1, 1])
        yy.addnoise()
        yy.done()
        shape = [20,20,1,5]
        yy.fromshape("", shape)
        #yy.addnoise()
        self.assertRaises(
            Exception,
            yy.fft, real="real1.im", mask=mymask + ">0",
            stretch=False
        )
        zz = yy.fft(
            real="real2.im", mask=mymask + ">0", stretch=True
        )
        self.assertTrue(type(zz) == type(false))
        yy.done()
        
    def test_delta(self):
        """Test fft of delta function"""
        myia = iatool()
        for t in ['f', 'c']:
            myia.fromshape("", [100, 100], type=t)
            bb = myia.getchunk()
            bb[50, 50] = 1
            myia.putchunk(bb)
            real = "real.im" + t
            imag = "imag.im" + t
            amp = "amp.im" + t
            phase = "phase.im" + t
            complex = "complex.im" + t
            myia.fft(
                real=real, imag=imag, amp=amp,
                phase=phase, complex=complex
            )
            for im in [real, imag, amp, phase, complex]:
                expec = 1
                if im == imag or im == phase:
                    expec = 0
                elif im == complex:
                    expec = 1 + 0j
                myia.open(im)
                got = myia.getchunk()
                myia.done()
                self.assertTrue((got == expec).all())
                
                
    def test_regression(self):
        """Was regression test in imagetest"""

        # Open test image (has sky coordinates)
        data = os.environ.get("CASAPATH").split()[0]
        testname = data + '/data/regression/unittest/ia_fft/test_image'
        myia = iatool()
        testim = myia.newimage(testname)
        self.assertTrue(testim)
        testshape = testim.shape()
        self.assertTrue(len(testshape) == 3)
         
        rname = 'real_reg'
        iname = 'imag_reg'
        aname = 'amp_reg'
        pname = 'phase_reg'
        self.assertTrue(
            testim.fft(
                real=rname, imag=iname, phase=pname, amp=aname
            )
        )
        im1 = myia.newimage(rname)
        self.assertTrue(im1)
        im2 = ia.newimage(iname)
        self.assertTrue(im2)
        im3 = ia.newimage(aname)
        self.assertTrue(im3)
        im4 = ia.newimage(pname)
        self.assertTrue(im4)
        trc = testim.shape()
        trc[2] = 0
        a1 = im1.getchunk(trc=trc)
        a2 = im2.getchunk(trc=trc)
        a3 = im3.getchunk(trc=trc)
        a4 = im4.getchunk(trc=trc)

        from numpy.fft import fft2
        p = testim.getchunk(trc=trc)
        c = fft2(p)
        b1 = c.real
        b2 = c.imag
        b3 = abs(c)  # sqrt( real(x)^2 + imag(x)^2 )
        
        ok =im1.remove(T) and im2.remove(T) and im3.remove(T) and im4.remove(T)
        self.assertTrue(ok)
        #
        # FFT whole image
        #
        ndim = len(testim.shape())
        axes = range(ndim)
        ok = testim.fft(real=rname, imag=iname, phase=pname, amp=aname, axes=axes)
        self.assertTrue(ok)
        im1 = ia.newimage(rname)
        self.assertTrue(im1)
        im2 = ia.newimage(iname)
        self.assertTrue(im2)
        im3 = ia.newimage(aname)
        self.assertTrue(im3)
        im4 = ia.newimage(pname)
        self.assertTrue(im4)
        a1 = im1.getchunk()
        a2 = im2.getchunk()
        a3 = im3.getchunk()
        a4 = im4.getchunk()
    
        p = testim.getchunk()
        c = fft2(p)
        b1 = c.real
        b2 = c.imag
        b3 = abs(c)
       
        ok = testim.done() and im1.done() and im2.done() and im3.done() and im4.done()
        self.assertTrue(ok)
        
def suite():
    return [ia_fft_test]
