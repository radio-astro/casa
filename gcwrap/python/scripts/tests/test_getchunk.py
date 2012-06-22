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
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_imfit[test1,test2,...]
# </example>
#
# <motivation>
# To provide a test standard to the imfit task to ensure
# coding changes do not break the associated bits 
# </motivation>
#

###########################################################################
import os
import casac
from tasks import *
from taskinit import *
import sha
from __main__ import *
import unittest

image = "gauss_no_pol.fits"
myia = iatool()

class getchunk_test(unittest.TestCase):
    
    def setUp(self):
        datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/imfit/'
        os.system('cp -r ' +datapath + image + ' ' + image)

    def tearDown(self):
        os.system('rm -rf ' + image)
        myia.done()

    def test_CAS_2355(self):
        '''getchunk: test 32/64 bit resolution'''
        myia.open(image)
        bb = myia.boundingbox( myia.setboxregion() )
        trc = bb['trc']
        blc = bb['blc']
        chunk = myia.getchunk(blc=blc, trc=trc)
        # if the last statement didn't throw an exception, then that's proof enough
        # the fix worked, but just for kicks...
        self.assertTrue(chunk[0][0][0] > 1973 and chunk[0][0][0] < 1974) 

def suite():
    return [getchunk_test]
