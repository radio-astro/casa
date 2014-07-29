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
# Test suite for the CASA tool method ia.crop()
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
# Test for the ia.crop() tool method
# </etymology>
#
# <synopsis>
# Test for the ia.crop tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_crop[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.crop() tool method to ensure
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

class ia_crop_test(unittest.TestCase):
    
    def setUp(self):
        self._myia = iatool()
    
    def tearDown(self):
        self._myia.done()
        self.assertTrue(len(tb.showcache()) == 0)
        
    def test_crop(self):
        """Test general cropping functionality"""
        myia = self._myia
        myia.fromshape("",[20,20,20])
        myrg = rgtool()
        myrg.setcoordinates(myia.coordsys().torecord())
        # mask 4, 6, and 8 pixels at image edges for axes 0, 1, 2
        reg = myrg.complement(
            myrg.wbox(
                ["8.00002381e+00arcmin","-7.00000484e+00arcmin","1.41499400e+09Hz"],
                ["2.15930000e+04arcmin", "6.00000305e+00arcmin","1.41500500e+09Hz"]
            )
        )
        myia.set(pixelmask=F,region=reg)
        crop = myia.crop()
        self.assertTrue((crop.shape() == [16, 14, 12]).all())
        crop = myia.crop(axes=[0])
        self.assertTrue((crop.shape() == [16, 20, 20]).all())
        crop = myia.crop(axes=[1])
        self.assertTrue((crop.shape() == [20, 14, 20]).all())
        crop = myia.crop(axes=[2])
        self.assertTrue((crop.shape() == [20, 20, 12]).all())
        crop = myia.crop(axes=[0, 1])
        self.assertTrue((crop.shape() == [16, 14, 20]).all())
        crop = myia.crop(axes=[0, 2])
        self.assertTrue((crop.shape() == [16, 20, 12]).all())
        crop = myia.crop(axes=[1, 2])
        self.assertTrue((crop.shape() == [20, 14, 12]).all())
        crop = myia.crop(axes=[0, 1, 2])
        self.assertTrue((crop.shape() == [16, 14, 12]).all())
        crop.done()
        

        
def suite():
    return [ia_crop_test]
