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
# Test suite for the CASA task specfit and tool method ia.fitprofile
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="task_specfit.py:description">imcollapse</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the specfit task and ia.fitprofile() tool method.
# </etymology>
#
# <synopsis>
# Test the specfit task and the ia.fitprofile() method upon which it is built.
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_coordsys[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the specfit task to ensure
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

myim = "center_0.fits"

datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/coordsys/'

class coordsys_test(unittest.TestCase):
    
    def setUp(self):
        shutil.copy(datapath + myim, myim)
        
    def tearDown(self):
        os.remove(myim)
        self.assertTrue(len(tb.showcache()) == 0)
        
    def test_CAS_2724(self):
        myia = iatool()
        myia.open(myim)
        mycsys = myia.coordsys()
        self.assertRaises(Exception, mycsys.toworldmany, [1])

    def test_constructor(self):
        """Test constructors"""
        x = coordsystool()
        x = cstool()
        
    def test_findaxisbyname(self):
        myia = iatool()
        myia.fromshape("", [4, 4, 4, 4])
        csys = myia.coordsys()
        myia.done()

        self.assertTrue(csys.findaxisbyname("ri") == 0)
        self.assertRaises(Exception, csys.findaxisbyname, "ra", False)
        self.assertTrue(csys.findaxisbyname("ra", True) == 0)
        
    def test_transpose(self):
        """Test the transpose() method"""
        myia = iatool()
        myia.fromshape("", [4, 4, 4, 4])
        csys = myia.coordsys()
        myia.done()
        
        orig = csys.names()
        self.assertTrue(csys.transpose([3,2,1,0]))
        new = csys.names()
        self.assertTrue(orig[0] == new[3])
        self.assertTrue(orig[1] == new[2])
        self.assertTrue(orig[2] == new[1])
        self.assertTrue(orig[3] == new[0])
        
    def test_findaxis(self):
        """Test the findaxis() method"""
        myia = iatool()
        myia.fromshape("", [20, 20, 4, 20])
        csys = myia.coordsys()
        self.assertRaises(Exception, csys.findaxis, True, -1)
        self.assertRaises(Exception, csys.findaxis, True, 4)
        got = csys.findaxis(True, 0)
        self.assertTrue(got == {'axisincoordinate': 0, 'coordinate': 0})
        got = csys.findaxis(True, 1)
        self.assertTrue(got == {'axisincoordinate': 1, 'coordinate': 0})
        got = csys.findaxis(True, 2)
        self.assertTrue(got == {'axisincoordinate': 0, 'coordinate': 1})
        got = csys.findaxis(True, 3)
        self.assertTrue(got == {'axisincoordinate': 0, 'coordinate': 2})
        myia.done()
        
    def test_findcoordinate(self):
        """Test the findcoordinate() method"""
        myia = iatool()
        myia.fromshape("", [20, 20, 4, 20])
        csys = myia.coordsys()
        myia.done()
        self.assertFalse(csys.findcoordinate('linear')['return'])
        self.assertFalse(
            csys.findcoordinate('direction', 2)['return']
        )
        got = csys.findcoordinate("direction")
        self.assertTrue(
            got['return'] and (got['pixel'] == [0,1]).all()
            and (got['world'] == [0,1]).all()
        )
        got = csys.findcoordinate("spectral")
        self.assertTrue(
            got['return'] and (got['pixel'] == [3]).all()
            and (got['world'] == [3]).all()
        )
        got = csys.findcoordinate("stokes")
        self.assertTrue(
            got['return'] and (got['pixel'] == [2]).all()
            and (got['world'] == [2]).all()
        )
        
def suite():
    return [coordsys_test]
