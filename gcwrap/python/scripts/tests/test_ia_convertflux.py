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
# Test suite for the CASA tool method ia.convertflux()
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
# Test for the ia.convertflux() tool method
# </etymology>
#
# <synopsis>
# Test for the ia.convertflux tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_convertflux[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.convertflux() tool method to ensure
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

class ia_convertflux_test(unittest.TestCase):
    
    def setUp(self):
        self._myia = iatool()
    
    def tearDown(self):
        self._myia.done()
        
    def test_beams(self):
        """test per plane beams get accounted for correctly"""
        myia = self._myia
        myia.fromshape("", [2, 2, 2])
        myia.setbrightnessunit("Jy/beam")
        myia.setrestoringbeam(
            "4arcsec", "3arcsec", "40deg", channel=-1,
            polarization=0
        )
        myia.setrestoringbeam(
            "5arcsec", "4arcsec", "40deg", channel=-1,
            polarization=1
        )
        got = myia.convertflux("1Jy","1arcsec","1arcsec",topeak=T, polarization=0)
        exp = qa.quantity("12Jy/beam")
        self.assertTrue(got["unit"] == exp["unit"])
        self.assertTrue(abs(got["value"] - exp["value"])/exp["value"] < 1e-7)

        got = myia.convertflux("3Jy/beam", "4arcsec", "2arcsec",topeak=F, polarization=0)
        exp = qa.quantity("2Jy")
        self.assertTrue(got["unit"] == exp["unit"])
        self.assertTrue(abs(got["value"] - exp["value"])/exp["value"] < 1e-7)
        
        got = myia.convertflux("1Jy","1arcsec","1arcsec",topeak=T, polarization=1)
        exp = qa.quantity("20Jy/beam")
        self.assertTrue(got["unit"] == exp["unit"])
        self.assertTrue(abs(got["value"] - exp["value"])/exp["value"] < 1e-7)

        got = myia.convertflux("3Jy/beam", "4arcsec", "2arcsec",topeak=F, polarization=1)
        exp = qa.quantity("1.2Jy")
        self.assertTrue(got["unit"] == exp["unit"])
        self.assertTrue(abs(got["value"] - exp["value"])/exp["value"] < 1e-7)

        

def suite():
    return [ia_convertflux_test]
