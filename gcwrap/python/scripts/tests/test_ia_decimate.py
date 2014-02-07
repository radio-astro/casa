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
# Test suite for the CASA tool method ia.decimate()
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
# Test for the ia.decimate() tool method
# </etymology>
#
# <synopsis>
# Test the ia.decimate() method.
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_decimate[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.decimate() tool method to ensure
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

class ia_decimate_test(unittest.TestCase):
    
    def setUp(self):
        pass
    
    def tearDown(self):
        self.assertTrue(len(tb.showcache()) == 0)
    
    def test_stretch(self):
        """ ia.fft(): Test stretch parameter"""
        yy = iatool()
        mymask = "maskim"
        yy.fromshape(mymask, [20, 20, 1, 1])
        yy.addnoise()
        yy.done()
        shape = [20,20,1,5]
        yy.fromshape("", shape)
        #yy.addnoise()
        self.assertRaises(
            Exception,
            yy.decimate, outfile="xx.im", mask=mymask + ">0",
            stretch=False
        )
        zz = yy.decimate(
            outfile="xx2.im", mask=mymask + ">0", stretch=True
        )
        self.assertTrue(type(zz) == type(yy))
        yy.done()
        zz.done()
        
    def test_methods(self):
        """Test straight aggregation methods"""
        myia = iatool()
        for m in [0, 1]:
            imagename = "myim_" + str(m)  + ".im"
            shape = numpy.array([10, 20, 41, 1])
            myia.fromshape(imagename, shape)
            myia.addnoise()
            myia.calcmask(imagename + " < 0")
            factor = 4
            if m == 0:
                method = "none"
            if m == 1:
                method = "mean"
            zz = myia.decimate("", axis=2, factor=factor, method=method)
            expec = shape
            expec[2] /= factor
            if m == 0:
                expec[2] += 1
            self.assertTrue((zz.shape() == expec).all())
            
            inc = myia.coordsys()
            outc = zz.coordsys()
            expec = inc.increment()['numeric']
            expec[2] *= factor
            got = outc.increment()['numeric']
            self.assertTrue((expec == got).all())
            
            expec = inc.referencepixel()['numeric']
            expec[2] /= factor
            got = outc.referencepixel()['numeric']
            self.assertTrue((expec == got).all())
            
            expec = inc.referencevalue()['numeric']
            got = outc.referencevalue()['numeric']
            self.assertTrue((expec == got).all())
            
            for i in range(10):
                blc = [0, 0, i*factor, 0]
                trc = shape - 1
                if m == 0:
                    trc[2] = i*factor
                    expdata = myia.getchunk(blc, trc)
                    expmask = myia.getchunk(blc, trc, getmask=T)
                elif m == 1:
                    trc[2] = (i+1)*factor - 1
                    reg = rg.box(blc, trc)
                    cc = myia.collapse("mean", 2, region=reg)
                    expdata = cc.getchunk()
                    expmask = cc.getchunk(getmask=T)
                blc = [0, 0, i, 0]
                trc = shape - 1
                trc[2] = i
                got = zz.getchunk(blc, trc)
                self.assertTrue((expdata == got).all())
                got = zz.getchunk(blc, trc, getmask=True)
                self.assertTrue((expmask == got).all())
            myia.done()
            zz.done()
        
        
def suite():
    return [ia_decimate_test]
