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
# Test suite for the CASA tool method ia.coordmeasures()
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
# Test for the ia.coordmeasures() tool method
# </etymology>
#
# <synopsis>
# Test for the ia.coordmeasures() tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casa | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_coordmeasures[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.coordmeasures() tool method to ensure
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

class ia_coordmeasures_test(unittest.TestCase):
    
    def setUp(self):
        self._myia = iatool()
    
    def tearDown(self):
        self._myia.done()
        self.assertTrue(len(tb.showcache()) == 0)
        
    def test_frame(self):
        """CAS-7927: Test returned frame is correct"""
        myia = self._myia
        myia.fromshape("",[20,20,20])
        cm = myia.coordmeasures()
        self.assertTrue(cm, "Unable to get coordmeasures")
        self.assertTrue(cm['measure']['spectral']['frequency']['m0']['value'] == 1.415e9, "wrong frequency")
        self.assertTrue(cm['measure']['spectral']['frequency']['refer'] == "LSRK", "wrong frequency reference frame")
        self.assertTrue(cm["measure"]["direction"]['m0']['value'] == 0, "wrong RA")
        self.assertTrue(cm["measure"]["direction"]['m1']['value'] == 0, "wrong Dec")
        self.assertTrue(cm["measure"]["direction"]['refer'] == "J2000", "wrong direction reference frame")
        csys = myia.coordsys()
        csys.setconversiontype(direction='B1950', spectral='CMB')
        myia.setcoordsys(csys.torecord())
        
        for i in xrange(4):
            if i == 0:
                cm = myia.coordmeasures()
            elif i == 1:
                cm = myia.coordmeasures(dframe="cl", sframe="cl")
            elif i == 2:
                cm = myia.coordmeasures(dframe="cl")
            elif i == 3:
                cm = myia.coordmeasures(sframe="cl")
            self.assertTrue(cm, "Unable to get coordmeasures")
            self.assertTrue(abs(cm['measure']['spectral']['frequency']['m0']['value'] - 1416700650.52) < 0.1, "wrong frequency")
            self.assertTrue(cm['measure']['spectral']['frequency']['refer'] == 'CMB', "wrong frequency reference frame")
            self.assertTrue(abs(cm["measure"]["direction"]['m0']['value'] - -0.0111827672206) < 1e-6, "wrong RA")
            self.assertTrue(abs(cm["measure"]["direction"]['m1']['value'] - -0.00485811473549) < 1e-6, "wrong Dec")
            self.assertTrue(cm["measure"]["direction"]['refer'] == "B1950", "wrong direction reference frame")

        cm = myia.coordmeasures(dframe="native", sframe="native")
        self.assertTrue(cm, "Unable to get coordmeasures")
        self.assertTrue(cm['measure']['spectral']['frequency']['m0']['value'] == 1.415e9, "wrong frequency")
        self.assertTrue(cm['measure']['spectral']['frequency']['refer'] == "LSRK", "wrong frequency reference frame")
        self.assertTrue(cm["measure"]["direction"]['m0']['value'] == 0, "wrong RA")
        self.assertTrue(cm["measure"]["direction"]['m1']['value'] == 0, "wrong Dec")
        self.assertTrue(cm["measure"]["direction"]['refer'] == "J2000", "wrong direction reference frame")
        
        cm = myia.coordmeasures(dframe="cl", sframe="native")
        self.assertTrue(cm, "Unable to get coordmeasures")
        self.assertTrue(cm['measure']['spectral']['frequency']['m0']['value'] == 1.415e9, "wrong frequency")
        self.assertTrue(
            abs(cm["measure"]["direction"]['m0']['value'] - -0.0111827672206) < 1e-6,
            "wrong RA"
        )
        self.assertTrue(
            abs(cm["measure"]["direction"]['m1']['value'] - -0.00485811473549) < 1e-6,
            "wrong Dec"
        )
        self.assertTrue(
            cm["measure"]["direction"]['refer'] == "B1950",
            "wrong direction reference frame"
        )
        
        cm = myia.coordmeasures(dframe="native", sframe="cl")
        self.assertTrue(cm, "Unable to get coordmeasures")
        self.assertTrue(abs(cm['measure']['spectral']['frequency']['m0']['value'] - 1416700650.52) < 0.1, "wrong frequency")
        self.assertTrue(cm['measure']['spectral']['frequency']['refer'] == 'CMB', "wrong frequency reference frame")
        self.assertTrue(cm["measure"]["direction"]['m0']['value'] == 0, "wrong RA")
        self.assertTrue(cm["measure"]["direction"]['m1']['value'] == 0, "wrong Dec")
        self.assertTrue(cm["measure"]["direction"]['refer'] == "J2000", "wrong direction reference frame")
        
        cm = myia.coordmeasures(dframe="GALACTIC", sframe="cl")
        self.assertTrue(cm, "Unable to get coordmeasures")
        self.assertTrue(
            abs(cm['measure']['spectral']['frequency']['m0']['value'] - 1416700650.52) < 0.1,
            "wrong frequency"
        )
        self.assertTrue(
            cm['measure']['spectral']['frequency']['refer'] == 'CMB',
            "wrong frequency reference frame"
        )
        self.assertTrue(
            abs(cm["measure"]["direction"]['m0']['value'] - 1.68140724) < 1e-6 ,
            "wrong RA"
        )
        self.assertTrue(
            abs(cm["measure"]["direction"]['m1']['value'] - -1.05048941) < 1e-6,
            "wrong Dec"
        )
        self.assertTrue(
            cm["measure"]["direction"]['refer'] == "GALACTIC",
            "wrong direction reference frame"
        )
        
        cm = myia.coordmeasures(dframe="cl", sframe="LGROUP")
        self.assertTrue(cm, "Unable to get coordmeasures")
        self.assertTrue(
            abs(cm['measure']['spectral']['frequency']['m0']['value'] - 1414142155.34) < 0.1,
            "wrong frequency"
        )
        self.assertTrue(
            cm['measure']['spectral']['frequency']['refer'] == 'LGROUP',
            "wrong frequency reference frame"
        )
        self.assertTrue(
            abs(cm["measure"]["direction"]['m0']['value'] - -0.0111827672206) < 1e-6,
            "wrong RA"
        )
        self.assertTrue(
            abs(cm["measure"]["direction"]['m1']['value'] - -0.00485811473549) < 1e-6,
            "wrong Dec"
        )
        self.assertTrue(
            cm["measure"]["direction"]['refer'] == "B1950",
            "wrong direction reference frame"
        )
        
        cm = myia.coordmeasures(dframe="GALACTIC", sframe="LGROUP")
        self.assertTrue(cm, "Unable to get coordmeasures")
        self.assertTrue(
            abs(cm['measure']['spectral']['frequency']['m0']['value'] - 1414142155.34) < 0.1,
            "wrong frequency"
        )
        self.assertTrue(
            cm['measure']['spectral']['frequency']['refer'] == 'LGROUP',
            "wrong frequency reference frame"
        )
        self.assertTrue(
            abs(cm["measure"]["direction"]['m0']['value'] - 1.68140724) < 1e-6 ,
            "wrong RA"
        )
        self.assertTrue(
            abs(cm["measure"]["direction"]['m1']['value'] - -1.05048941) < 1e-6,
            "wrong Dec"
        )
        self.assertTrue(
            cm["measure"]["direction"]['refer'] == "GALACTIC",
            "wrong direction reference frame"
        )
        
        self.assertRaises(Exception, myia.coordmeasures, dframe="CL", sframe="BOGUS")
        self.assertRaises(Exception, myia.coordmeasures, dframe="BOGUS", sframe="CL")

        myia.done()
        
def suite():
    return [ia_coordmeasures_test]
