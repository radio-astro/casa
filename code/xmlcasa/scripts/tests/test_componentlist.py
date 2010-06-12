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
# Test suite for the CASA imfit Task
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="imfit.py:description">imfit</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# imfit_test stands for imfit test
# </etymology>
#
# <synopsis>
# imfit_test.py is a Python script that tests the correctness
# of the ia.fitcomponents tool method and the imfit task in CASA.
# </synopsis> 
#
# <example>
# # This test was designed to run in the automated CASA test system.
# # This example shows who to run it manually from within casapy.
# casapy -c runUnitTest test_imcontsub
#
# or
#
# # This example shows who to run it manually from with casapy.
# sys.path.append( os.environ["CASAPATH"].split()[0]+'/code/xmlcasa/scripts/regressions/admin' )
# import runUnitTest
# runUnitTest.main(['imcontsub_test'])
#
# </example>
#
# <motivation>
# To provide a test standard to the imfit task to ensure
# coding changes do not break the associated bits 
# </motivation>
#

###########################################################################
import casac
from tasks import *
from taskinit import *
from __main__ import *
import unittest

class componentlist_test(unittest.TestCase):
    
    def setUp(self):
        return

    def tearDown(self):
        return
 
    def test_summarize(self):
        """Test the cl.summarize() method"""
        # make me a list
        cl.addcomponent(
            [1,0,0,0],'Jy','Stokes',['J2000', '10:30:00.00', '-20.00.00.0'],
            'gaussian','4arcsec','2arcsec','30deg'
        )
        self.assertTrue(cl.summarize(0))
        self.assertRaises(Exception, cl.summarize, 1)


 

def suite():
    return [componentlist_test]