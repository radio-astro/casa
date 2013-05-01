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
# Test suite for the CASA method ia.deconvolvecomponentlist()
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
#
# <etymology>
# Test for the method ia.deconvolvecomponentlist()
# </etymology>
#
# <synopsis>
# Test the method ia.deconvolvecomponentlist().
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_deconvolvecomponentlist[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the method ia.deconvolvecomponentlist() to ensure
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

def _near(got, exp, tol):
    qgot = qa.quantity(got)
    qexp = qa.quantity(exp)
    return qa.abs(qa.div(qa.sub(qgot, qexp), qexp))["value"] < tol

class ia_deconvolvecomponentlist_test(unittest.TestCase):
    
    def setUp(self):
        self.ia = iatool()
        self.cl = cltool()
    
    def tearDown(self):
        self.ia.done()
        self.cl.done()
    
    def test_multibeams(self):
        """ ia.deconvolvecomponentlist(): Test multi beams"""
        myia = self.ia
        mycl = self.cl
        mycl.addcomponent(
            flux=1, dir=["J2000", "2h0m0s", "40d0m0s"], shape="gauss",
            majoraxis="4arcsec", minoraxis="3arcsec", positionangle="20deg"
        )
        
        myia.fromshape("", [2, 2, 2])
        mycs = myia.coordsys()
        mycs.setunits(["deg","deg",""])
        mycs.setdirection(
            refcode="J2000", refval=[30,40],
            incr=[-1.0/36000,1.0/36000]
        )
        myia.setcoordsys(mycs.torecord())
        myia.setrestoringbeam(
            major="2arcsec", minor="1arcsec", pa="20deg",
            polarization=0
        )
        myia.setrestoringbeam(
            major="3arcsec", minor="2arcsec", pa="50deg",
            polarization=1
        )
        bb = cltool()
        emaj = [
            qa.quantity({'unit': 'arcsec', 'value': 3.4641016151377548}),
            qa.quantity({'unit': 'arcsec', 'value': 3.0203474964295665})
        ]
        emin = [
            qa.quantity({'unit': 'arcsec', 'value': 2.8284271247461894}),
            qa.quantity({'unit': 'arcsec', 'value': 1.6963198403637358})
        ]
        epa = [
            qa.quantity({'unit': 'deg', 'value': 20}),
            qa.quantity({'unit': 'deg', 'value': -1.948943124031587 + 180})
        ]
        tol = 1e-10
        for i in [0, 1]:
            res = myia.deconvolvecomponentlist(mycl.torecord(), polarization=i)
            bb.fromrecord(res)
            shape = bb.getshape(0)
            bb.done()
            self.assertTrue(_near(shape["majoraxis"], emaj[i], tol))
            self.assertTrue(_near(shape["minoraxis"], emin[i], tol))
            print "*** pa " + str(shape["positionangle"])
            self.assertTrue(_near(shape["positionangle"], epa[i], tol))
        
def suite():
    return [ia_deconvolvecomponentlist_test]
