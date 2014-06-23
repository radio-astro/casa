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
# Test suite for the CASA tool method ia.continuumsub()
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
# Test for the ia.continuumsub() tool method
# </etymology>
#
# <synopsis>
# Test for the ia.continuumsub tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_continuumsub[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.continuumsub() tool method to ensure
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

class ia_continuumsub_test(unittest.TestCase):
    
    def setUp(self):
        self._myia = iatool()
    
    def tearDown(self):
        self._myia.done()
        
    def test_beams(self):
        """test per plane beams get accounted for correctly"""
        myia = self._myia
        myia.fromshape("",[4,4,20, 4])
        chunk = myia.getchunk()
        for i in range(20):
            for j in range(4):
                print "i " + str(i) + " j " + str(j)
                chunk[2, 2, i, j] = i
        myia.putchunk(chunk)
        myia.setrestoringbeam("3arcsec","2arcsec", "4deg", channel=10)
        for i in range(20):
            for j in range(4):
                major=qa.quantity(i+4*j+4,"arcsec")
                myia.setrestoringbeam(major,"2arcsec","4deg",channel=i, polarization=j)
        reg = rg.box(blc=[2, 2, 3, 0], trc=[2, 2, 17, 3])
        outcont = "line.im"
        resid = myia.continuumsub(outcont=outcont, fitorder=1, region=reg, pol="U")
        for i in range(resid.shape()[2]):
            exp = qa.quantity(i+15, "arcsec")
            got = resid.restoringbeam(channel=i)["major"]
            self.assertTrue(got == exp)
        myia.open(outcont)
        self.assertTrue(myia.restoringbeam() == resid.restoringbeam())
        resid.done()

def suite():
    return [ia_continuumsub_test]
