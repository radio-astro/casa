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
# Test suite for the CASA tool method ia.hanning()
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
# Test for the ia.hanning() tool method
# </etymology>
#
# <synopsis>
# Test the ia.hanning() tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_hanning[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.hanning() tool method to ensure
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

class ia_hanning_test(unittest.TestCase):
    
    def setUp(self):
        pass
    
    def tearDown(self):
        self.assertTrue(len(tb.showcache()) == 0)
    
    def test_stretch(self):
        """ ia.hanning(): Test stretch parameter"""
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
            yy.hanning, mask=mymask + ">0", stretch=False
        )
        zz = yy.hanning(
            mask=mymask + ">0", stretch=True
        )
        self.assertTrue(type(zz) == type(yy))
        yy.done()
        zz.done()
    
    def test_regression(self):
        """Tests moved from imagetest regression"""
        # Make image
        imname = 'ia.fromshape.image'
        imshape = [10,20]
        myim = ia.newimagefromshape(outfile=imname, shape=imshape)
        self.assertTrue(myim)
        pixels = myim.getchunk()
        self.assertTrue(len(pixels) > 0)
        for i in range(pixels.shape[0]):
            for j in range(pixels.shape[1]):
                if pixels[i][j]>-10000:
                    pixels[i][j]=1
        self.assertTrue(myim.putchunk(pixels))
        self.assertRaises(Exception, myim.hanning, axis=19)
        hanname = 'hanning.image'
        myim2 = myim.hanning(outfile=hanname, axis=0, drop=F)
        self.assertTrue(myim2)
        pixels2 = myim2.getchunk()
        self.assertFalse(len(pixels2)==0)
        self.assertTrue((pixels2 == 1).all())
        self.assertTrue(myim2.remove(done=T))
        myim2 = myim.hanning(outfile=hanname, axis=0, drop=T)
        self.assertTrue(myim2)
        shape2 = [myim.shape()[0]/2-1,myim.shape()[1]]
        self.assertTrue((myim2.shape() == shape2).all())
        pixels2 = myim2.getchunk()
        self.assertFalse(len(pixels2)==0)
        self.assertTrue((pixels2 == 1).all())
        self.assertTrue(myim2.remove(done=T))
        pixels = myim.getregion()
        mask = myim.getregion(getmask=true)
        mask[0,0] = F
        mask[1,0] = F
        mask[2,0] = F
        mask[3,0] = F
        self.assertTrue(myim.putregion(pixelmask=mask))
        myim2 = myim.hanning(outfile=hanname, axis=0, drop=F)
        self.assertTrue(myim2)
        pixels2 = myim2.getregion()
        mask2 = myim2.getregion(getmask=true)
        self.assertTrue(mask2[0,0]==F and mask2[1,0]==F)
        self.assertFalse(mask2[2,0])
        self.assertFalse(mask2[3,0])
        self.assertTrue(pixels2[0,0]==0 and pixels2[1,0]==0)
        self.assertTrue(pixels2[2,0]==0)
        self.assertTrue(pixels2[3,0]==0.25)
        
        self.assertTrue(myim2.done())
        
        self.assertTrue(myim.done())

    def test_general(self):
        """Test general behavior"""
        myia = iatool()
        length = 6
        myia.fromshape("", [1, 1, length])
        bb = myia.getchunk()
        for i in range(length):
            bb[0, 0, i] = i*i + 1
        myia.putchunk(bb)
        for i in range(length):
            reg = rg.box([0, 0, 0], [0, 0, i])
            if (i < 2):
                self.assertRaises(Exception, myia.hanning, region=reg, axis=2)
            else:
                for drop in (False, True):
                    if drop:
                        for dropmethod in ("n", "m"):
                            if i==2 or i==3:
                                if dropmethod=="n":
                                    expec = [2.5]
                                else:
                                    if i == 2:
                                        expec = [3.0]
                                    elif i == 3:
                                        expec = [4.0]
                            elif i==4 or i==5:
                                if dropmethod=="n":
                                    expec = [2.5, 10.5]
                                else:
                                    if i == 4:
                                        expec = [4.0, 12.0]
                                    if i == 5:
                                        expec = [4.0, 14.0]
                            han = myia.hanning(region=reg, axis=2, drop=drop, dropmethod=dropmethod)
                            got = han.getchunk().ravel()
                            self.assertTrue((got == expec).all())
                    else:
                        dropmethod="n"
                        if i == 2:
                            expec = [1.5, 2.5, 3.5]
                        elif i == 3:
                            expec = [1.5, 2.5, 5.5, 7.5]
                        elif i == 4:
                            expec = [1.5, 2.5, 5.5, 10.5, 13.5]
                        elif i == 5:
                            expec = [1.5, 2.5, 5.5, 10.5, 17.5, 21.5]
                        han = myia.hanning(region=reg, axis=2, drop=drop, dropmethod=dropmethod)
                        got = han.getchunk().ravel()
                        self.assertTrue((got == expec).all())
                    
        myia.done()
            
        
        
    
def suite():
    return [ia_hanning_test]
