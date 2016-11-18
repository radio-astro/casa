##########################################################################
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
# Test suite for the CASA tool method ia.pad()
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
# Test for the ia.pad() tool method
# </etymology>
#
# <synopsis>
# Test the ia.pad() tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casa | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_pad[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.pad() tool method to ensure
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

class ia_pad_test(unittest.TestCase):
    
    def setUp(self):
        self.ia = iatool()
    
    def tearDown(self):
        pass
    
    def test_pad(self):
        """ ia.pad(): Test pad()"""
        myia = self.ia
        shape = [10, 10, 10]
        myia.fromshape(shape=shape)
        chunk = myia.getchunk()
        chunk[:,:,:] = 1
        myia.putchunk(chunk)
        for np in [5, 7]:
            pad = myia.pad(npixels=np)
            got = pad.shape()
            # make a copy, not a reference
            expshape = shape[:]
            expshape[0] += 2*np
            expshape[1] += 2*np
            # did we get the correct shape
            self.assertTrue((got == expshape).all())
            # did we actually copy the source pixel values
            exp = myia.statistics()['sum']
            got = pad.statistics()['sum']
            self.assertTrue((got == exp).all())
            # test that padding pixels are masked
            got = pad.statistics()['npts']
            exp = myia.statistics()['npts']
            self.assertTrue((got == exp).all())
            # coordinate system consistency checks
            exp = myia.coordsys().referencepixel()['numeric'] + [np, np, 0]
            got = pad.coordsys().referencepixel()['numeric']
            self.assertTrue((abs(got - exp) < 1e-8).all())
            exp = myia.toworld([0, 0, 0])['numeric']
            got = pad.toworld([np, np, 0])['numeric']

            self.assertTrue((abs(got - exp) < 1e-8).all())
            
            # checks for not masking pixels
            pad = myia.pad(npixels=np, padmask=True)
            got = pad.shape()
            self.assertTrue((got == expshape).all())
            # test that padding pixels are not masked
            got = pad.statistics()['npts']
            exp = numpy.prod(pad.shape())
            self.assertTrue(got[0] == exp)
            # coordinate system consistency checks
            exp = myia.coordsys().referencepixel()['numeric'] + [np, np, 0]
            got = pad.coordsys().referencepixel()['numeric']
            self.assertTrue((abs(got - exp) < 1e-8).all())
            exp = myia.toworld([0, 0, 0])['numeric']
            got = pad.toworld([np, np, 0])['numeric']
            
            # checks for not masking pixels and setting to value
            pad = myia.pad(npixels=np, padmask=True, value=1)
            got = pad.shape()
            self.assertTrue((got == expshape).all())
            # test that padding pixels are not masked
            got = pad.statistics()['npts']
            exp = numpy.prod(pad.shape())
            self.assertTrue(got[0] == exp)
            # test that padding pixels are set to value
            got = pad.statistics()['sum'][0]
            exp = myia.statistics()['sum'] + (numpy.prod(pad.shape()) - numpy.prod(shape) )
            # coordinate system consistency checks
            exp = myia.coordsys().referencepixel()['numeric'] + [np, np, 0]
            got = pad.coordsys().referencepixel()['numeric']
            self.assertTrue((abs(got - exp) < 1e-8).all())
            exp = myia.toworld([0, 0, 0])['numeric']
            got = pad.toworld([np, np, 0])['numeric']
            
    def test_stretch(self):
        """ ia.pad(): Test stretch parameter"""
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
            yy.pad, npixels=1,
            mask=mymask + ">0", stretch=False
        )
        zz = yy.pad(
            npixels=1, mask=mymask + ">0", stretch=True
        )
        self.assertTrue(zz and type(zz) == type(yy))
        yy.done()
        zz.done()

    def test_mask(self):
        """Test that mask is preserved"""
        
        def _check():
            data = pad.getchunk()
            self.assertTrue((data[0, :] == 0).all())
            self.assertTrue((data[:, 0] == 0).all())
            self.assertTrue((data[padsize-1, :] == 0).all())
            self.assertTrue((data[:, padsize-1] == 0).all())
            self.assertTrue((data[1:padsize-1, 1:padsize-1] == myia.getchunk()).all()) 
            mask = pad.getchunk(getmask=True)
            self.assertTrue((mask[0, :] == False).all())
            self.assertTrue((mask[:, 0] == False).all())
            self.assertTrue((mask[padsize-1, :] == False).all())
            self.assertTrue((mask[:, padsize-1] == False).all())
            self.assertTrue((mask[1:padsize-1, 1:padsize-1] == expec).all()) 
        
        
        myia = iatool()
        imagename = "xyz.im"
        n = 20
        myia.fromshape(imagename, [n, n])
        myia.addnoise()
        np = 1
        padsize = n + 2*np
        pad = myia.pad(npixels=np)
        expec = myia.getchunk(getmask=True)
        _check()

        pad = myia.pad(npixels=np, mask=imagename + ">0")
        expec = myia.getchunk() > 0
        _check()

        # give the image a pixel mask
        myia.calcmask(imagename + "<0")
        pad = myia.pad(npixels=np)
        expec = myia.getchunk(getmask=True)
        _check()

        # pixel mask + region defined by using an OTF mask
        pad = myia.pad(npixels=np, mask=imagename + "<0.5")
        expec = numpy.logical_and(myia.getchunk(getmask=True), myia.getchunk() < 0.5)
        _check()

        pad.done()
        myia.done()
    
    def test_history(self):
        """Verify history writing"""
        myia = iatool()
        myia.fromshape("",[20,20])
        bb = myia.pad()
        myia.done()
        msgs = bb.history()
        bb.done()
        self.assertTrue("ia.pad" in msgs[-4])
        self.assertTrue("ia.pad" in msgs[-3])

def suite():
    return [ia_pad_test]
