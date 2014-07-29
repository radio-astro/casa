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
# Test suite for the CASA method rg.selectectedchannels()
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# </prerequisite>
#
# <etymology>
# Test for the rg.selectectedchannels() method
# </etymology>
#
# <synopsis>
# Test the rg.selectectedchannels() method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_rg_selectectedchannels[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the rg.selectectedchannels() method to ensure
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

class rg_selectedchannels_test(unittest.TestCase):
    
    def setUp(self):
        pass
    
    def tearDown(self):
        self.assertTrue(len(tb.showcache()) == 0)

    def test_no_spectral_axis(self):
        """ Test no spectral axis throws exception"""
        myia = iatool()
        myia.fromshape("",[4,4,4])
        rg.setcoordinates(myia.coordsys().torecord())
        shape = myia.shape()
        self.assertRaises(
            Exception, rg.selectedchannels,
            "range=[1415MHz, 1415.1MHz]", shape
        )
        myia.done()
        
    def test_no_overlap(self):
        """ Test selection outside of image"""
        myia = iatool()
        myia.fromshape("",[4,4,20])
        rg.setcoordinates(myia.coordsys().torecord())
        shape = myia.shape()
        self.assertRaises(
            Exception, rg.selectedchannels,
            "range=[1315MHz, 1315.1MHz]", shape
        )
        myia.done()
        
    def test_range(self):
        """ Test range"""
        myia = iatool()
        myia.fromshape("",[4,4,20])
        rg.setcoordinates(myia.coordsys().torecord())
        shape = myia.shape()
        myia.done()

        chans = rg.selectedchannels("range=[1415MHz,1415.002MHz]", shape)
        self.assertTrue((chans == [10, 11, 12]).all())
        chans = rg.selectedchannels("range=[1415.002MHz, 1415MHz]", shape)
        self.assertTrue((chans == [10, 11, 12]).all())
        
        chans = rg.selectedchannels("range=[1414MHz,1415.002MHz]", shape)
        self.assertTrue((chans == [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12]).all())
        
        chans = rg.selectedchannels("range=[1415MHz,1415.2MHz]", shape)
        self.assertTrue((chans == [10, 11, 12, 13, 14, 15, 16, 17, 18, 19]).all())
         
         
def suite():
    return [rg_selectedchannels_test]
