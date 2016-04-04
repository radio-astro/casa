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
# Test suite for the CASA tool method ia.rotate()
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
# Test for the ia.rotate() tool method
# </etymology>
#
# <synopsis>
# Test the ia.rotate() tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_rotate[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.rotate() tool method to ensure
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
import numpy

datapath = os.environ.get('CASAPATH').split()[0]+ '/data/regression/unittest/ia_rotate/'

class ia_rotate_test(unittest.TestCase):
    
    def setUp(self):
        pass
    
    def tearDown(self):
        pass
    
    def test_stretch(self):
        """ ia.rotate(): Test stretch parameter"""
        yy = iatool()
        mymask = "maskim"
        yy.fromshape(mymask, [200, 200, 1, 1])
        yy.addnoise()
        yy.done()
        shape = [200,200,1,20]
        yy.fromshape("", shape)
        yy.addnoise()
        self.assertRaises(
            Exception,
            yy.rotate,
            mask=mymask + ">0", stretch=False
        )
        zz = yy.rotate(
            mask=mymask + ">0", stretch=True
        )
        self.assertTrue(zz and type(zz) == type(yy))
        yy.done()
        zz.done()
        
    def test_basic(self):
        """verify basic rotation works"""
        myia = iatool()
        myia.open(datapath + "prerot.im")
        rot = myia.rotate(pa="45deg")
        got = rot.getchunk();
        rot.done()
        myia.open(datapath + "postrot.im")
        expec = myia.getchunk()
        myia.done()
        self.assertTrue(numpy.abs(got - expec).max() < 10e-22)
        
    def test_history(self):
        """Verify history is written"""
        myia = iatool()
        myia.open(datapath + "prerot.im")
        rot = myia.rotate(pa="45deg")
        myia.done()
        msgs = rot.history()
        print "msgs ", msgs
        rot.done()
        self.assertTrue("ia.rotate" in msgs[-2])
        self.assertTrue("ia.rotate" in msgs[-1])
    
def suite():
    return [ia_rotate_test]
