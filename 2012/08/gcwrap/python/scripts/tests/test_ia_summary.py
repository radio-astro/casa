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
# Test suite for the CASA tool method ia.summary()
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
# Test for the ia.summary() tool method
# </etymology>
#
# <synopsis>
# Test for the ia.summary tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `which casapy` --nologger --log2term  -c `echo $CASAPATH | awk '{print $1}'`/linux_64b/python/2.6/runUnitTest.py test_ia_summary[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.summary() tool method to ensure
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

class ia_summary_test(unittest.TestCase):
    
    def setUp(self):
        self._myia = iatool()
    
    def tearDown(self):
        self._myia.done()
        
    def test_beams(self):
        """test per plane beams get accounted for correctly"""
        myia = self._myia
        shape = [10, 10, 10, 4]
        myia.fromshape("", shape)
        bmaj = qa.quantity("4arcsec")
        bmin = qa.quantity("2arcsec")
        bpa = qa.quantity("40deg")
        myia.setrestoringbeam(major=bmaj, minor=bmin, pa=bpa, channel=0, polarization=0)
        cmaj = qa.quantity("7arcsec")
        cmin = qa.quantity("5arcsec")
        cpa = qa.quantity("80deg")
        myia.setrestoringbeam(major=cmaj, minor=cmin, pa=cpa, channel=6, polarization=3)
        summary = myia.summary()
        self.assertTrue(summary.has_key("perplanebeams"))
        beams = summary["perplanebeams"]["beams"]
        for c in range(shape[2]):
            for p in range(shape[3]):
                beam = beams["*" + str(c)]["*" + str(p)]
                majax = beam["major"]
                minax = beam["minor"]
                pa = beam["positionangle"]
                if c == 6 and p == 3:
                    self.assertTrue(majax == cmaj)
                    self.assertTrue(minax == cmin)
                    self.assertTrue(pa == cpa)
                else:
                    self.assertTrue(majax == bmaj)
                    self.assertTrue(minax == bmin)
                    self.assertTrue(pa == bpa)

def suite():
    return [ia_summary_test]
