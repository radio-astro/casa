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
# Test suite for the CASA tool method ia.subimage
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="ia.subimage:description">ia.subimage</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the ia subimage tool method
# </etymology>
#
# <synopsis>
# Test the ia.subimage tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_subimage[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.subimage tool method to ensure
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


class ia_subimage_test(unittest.TestCase):
    
    def setUp(self):
        pass
    
    def tearDown(self):
        pass


    def test_stretch(self):
        """Test the stretch parameter"""
        myia = iatool.create()
        myia.fromshape("mask1.im", [20, 30, 4, 10])
        myia.fromshape("mask2.im", [20, 30, 4, 1])
        myia.fromshape("mask3.im", [20, 30, 4, 2])

        myia.fromshape("", [20,30,4,10])

        mm = myia.subimage("", mask="mask1.im > 10")
        self.assertTrue(mm)
        self.assertRaises(Exception, myia.subimage, "", mask="mask2.im > 10", stretch=False)
        self.assertTrue(myia.subimage("", mask="mask2.im > 10", stretch=True))
        self.assertRaises(Exception, myia.subimage, "", mask="mask3.im > 10", stretch=True)

        

def suite():
    return [ia_subimage_test]
