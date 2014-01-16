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
# Test suite for the CASA tool method ia.adddegaxes()
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
# Test for the ia.adddegaxes() tool method
# </etymology>
#
# <synopsis>
# Test for the ia.adddegaxes tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_adddegaxes[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.adddegaxes() tool method to ensure
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

class ia_adddegaxes_test(unittest.TestCase):
    
    def setUp(self):
        self._myia = iatool()
    
    def tearDown(self):
        self._myia.done()
    
    def test_general(self):
        """general tests"""
        myia = self._myia
        # Make RA/DEC image
        imname = 'ia.fromshape.image'
        imshape = [10,10]
        myim = myia.newimagefromshape(imname, imshape)
        self.assertTrue(myim)
        self.assertRaises(Exception, myim.adddegaxes, direction=T)
        myim2 = myim.adddegaxes(spectral=T)
        self.assertTrue(myim2)
        s = myim2.shape()
        s2 = [imshape[0],imshape[1],1]
        self.assertTrue((s == s2).all())
        mycs = myim2.coordsys()
        types = mycs.axiscoordinatetypes()
        self.assertTrue(types[2] == 'Spectral')
        self.assertTrue(mycs.done())
        self.assertTrue(myim2.done())
        myim2 = myim.adddegaxes(stokes='i')
        self.assertTrue(myim2)
        s = myim2.shape()
        s2 = [imshape[0],imshape[1],1]
        self.assertTrue((s == s2).all())
        mycs = myim2.coordsys()
        types = mycs.axiscoordinatetypes()
        self.assertTrue(types[2] == 'Stokes')
        self.assertTrue(mycs.done())
        self.assertTrue(myim2.done())
        #
        myim2 = myim.adddegaxes(linear=T)
        self.assertTrue(myim2)
        s = myim2.shape()
        s2 = [imshape[0],imshape[1],1]
        self.assertTrue((s == s2).all())
        mycs = myim2.coordsys()
        types = mycs.axiscoordinatetypes()
        self.assertTrue(types[2] == 'Linear')
        self.assertTrue(mycs.done())
        self.assertTrue(myim2.done())
        
        myim2 = myim.adddegaxes(tabular=T)
        self.assertTrue(myim2)
        s = myim2.shape()
        s2 = [imshape[0],imshape[1],1]
        self.assertTrue((s == s2).all())
        mycs = myim2.coordsys()
        types = mycs.axiscoordinatetypes()
        self.assertTrue(types[2] == 'Tabular')
        self.assertTrue(mycs.done())
        self.assertTrue(myim2.done())
        self.assertTrue(myim.done())
        #
        # Make Spectral image
        #
        mycs = cs.newcoordsys(spectral=T)
        self.assertTrue(mycs)
        imname = 'ia.fromshape2.image'
        imshape = [10]
        myim = myia.newimagefromshape(imname, imshape, csys=mycs.torecord())
        self.assertTrue(myim)
        myim2 = myim.adddegaxes(direction=T)
        self.assertTrue(myim2)
        s = myim2.shape()
        s2 = [imshape[0],1,1]
        self.assertTrue((s == s2).all())
        mycs2 = myim2.coordsys()
        types = mycs2.axiscoordinatetypes()
        self.assertTrue(types[1] == 'Direction' and types[2] == 'Direction')
        self.assertTrue(mycs2.done())
        self.assertTrue(myim2.done())
        self.assertTrue(mycs.done())
        self.assertTrue(myim.done())
        
    def test_beams(self):
        """test hyperbeams get accounted for correctly"""
        myia = self._myia
        myia.fromshape(shape=[10, 10, 10])
        major = "4arcsec"
        minor = "3arcsec"
        pa = "4deg"
        nminor = "2arcsec"
        myia.setrestoringbeam(major=major, minor=minor, pa=pa, channel=1)
        myia.setrestoringbeam(major=major, minor=nminor, pa=pa, channel=3)
        deg = myia.adddegaxes(stokes="I")
        self.assertTrue((deg.shape() == [10, 10, 10, 1]).all())
        beam = deg.restoringbeam(channel=1)
        self.assertTrue(beam["major"] == qa.quantity(major))
        self.assertTrue(beam["minor"] == qa.quantity(minor))
        self.assertTrue(beam["positionangle"] == qa.quantity(pa))
        beam = deg.restoringbeam(channel=3)
        self.assertTrue(beam["major"] == qa.quantity(major))
        self.assertTrue(beam["minor"] == qa.quantity(nminor))
        self.assertTrue(beam["positionangle"] == qa.quantity(pa))

        
        deg.done()
        

def suite():
    return [ia_adddegaxes_test]
