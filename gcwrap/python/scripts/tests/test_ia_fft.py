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
        myia.fromshape("", [100, 100])
        bb = myia.getchunk()
        bb[50, 50] = 1
        myia.putchunk(bb)
        real = "real.im"
        imag = "imag.im"
        amp = "amp.im"
        phase = "phase.im"
        myia.fft(real=real, imag=imag, amp=amp, phase=phase)
        for im in [real, imag, amp, phase]:
            expec = 1
            if im == imag or im == phase:
                expec = 0
            myia.open(im)
            self.assertTrue((myia.getchunk() == expec).all())
        myia.done()
        
def suite():
    return [ia_fft_test]
