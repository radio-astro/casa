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
# Test suite for the CASA tool method ia.rebin()
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
# Test for the ia.rebin() tool method
# </etymology>
#
# <synopsis>
# Test the ia.rebin() tool method
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_ia_rebin[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the ia.rebin() tool method to ensure
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

def alleqnum(x,num,tolerance=0):
    if len(x.shape)==1:
        for i in range(x.shape[0]):
            if not (abs(x[i]-num) < tolerance):
                print "x[",i,"]=", x[i]
                return false
    if len(x.shape)==2:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                if not (abs(x[i][j]-num) < tolerance):
                    print "x[",i,"][",j,"]=", x[i][j]
                    return false
    if len(x.shape)==3:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    if not (abs(x[i][j][k]-num) < tolerance):
                        print "x[",i,"][",j,"][",k,"]=", x[i][j][k]
                        return false
    if len(x.shape)==4:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    for l in range(x.shape[3]):
                        if not (abs(x[i][j][k][l]-num) < tolerance):
                            print "x[",i,"][",j,"][",k,"][",l,"]=", x[i][j][k]
                            return false
    if len(x.shape)>4:
        stop('unhandled array shape in alleq')
    return true


class ia_rebin_test(unittest.TestCase):
    
    def setUp(self):
        self._myia = iatool()
    
    def tearDown(self):
        self._myia.done()
        self.assertTrue(len(tb.showcache()) == 0)
    
    def test_stretch(self):
        """ ia.rebin(): Test stretch parameter"""
        yy = iatool()
        mymask = "maskim"
        yy.fromshape(mymask, [200, 200, 1, 1])
        yy.addnoise()
        yy.done()
        shape = [200,200,1,10]
        imagename = "aa.im"
        yy.fromshape(imagename, shape)
        yy.addnoise()
        self.assertRaises(
            Exception,
            yy.rebin, outfile="", bin=[2,2,1,1],
            mask=mymask + ">0", stretch=False
        )
        
        zz = yy.rebin(
            outfile="", bin=[2,2,1,1],
            mask=mymask + ">0", stretch=True
        )
        self.assertTrue(type(zz) == type(yy))
        yy.done()
        zz.done()
        outfile = "ab.im"
        self.assertFalse(
            imrebin(
                imagename=imagename, outfile=outfile, factor=[2,2,1,1],
                mask=mymask + ">0", stretch=False, overwrite=True
            )
        )
        self.assertTrue(
            imrebin(
                imagename=imagename, outfile=outfile, factor=[2,2,1,1],
                mask=mymask + ">0", stretch=True, overwrite=True
            )
        )
        yy.open(outfile)
        print "shape " + str(yy.shape())
        self.assertTrue((yy.shape() == [100, 100, 1, 10]).all())
        yy.done()
        
        
    def test_general(self):
        """ ia.rebin(): General tests"""
        # tests moved from imagetest_regression.py and modified
        
        myia = self._myia
        shp2 = [20,40]
        d2 = myia.makearray(1.0, [shp2[0], shp2[1]])
        #
        imagename = "st.im"
        myim2 = myia.newimagefromarray(outfile=imagename, pixels=d2)
        self.assertTrue(myim2)
        
        outfile = "gk.im"
        try:
            myim2b = true
            myim2b = myim2.rebin("", bin=[-100,2], overwrite=True)
        except Exception, e:
            myim2b = false
        self.assertFalse(myim2b)
        
        self.assertFalse(
            imrebin(
                imagename=imagename, outfile=outfile,
                factor=[-100,2], overwrite=True
            )
        )
        
        myim2b = myim2.rebin("", bin=[2,2])
        self.assertTrue(myim2b)
        p = myim2b.getchunk()
        self.assertTrue(alleqnum(p,1.0,tolerance=0.0001))
    
        self.assertTrue(myim2.done() and myim2b.done())
        
        self.assertTrue(
            imrebin(
                imagename=imagename, outfile=outfile, overwrite=True,
                factor=[2,2]
            )
        )
        myim2b.open(outfile)
        p = myim2b.getchunk()
        self.assertTrue(alleqnum(p,1.0,tolerance=0.0001))
        myim2b.done()

    def test_multibeam(self):
        """Test multiple beams"""
        myia = self._myia
        imagename = "gd.im"
        myia.fromshape(imagename, [10, 10, 10])
        myia.setrestoringbeam(
            major="4arcsec", minor="2arcsec", pa="0deg",
            channel=0, polarization=0
        )
        rebin = myia.rebin("", [2, 2, 1])
        self.assertTrue(rebin)
        outfile = "dx.im"
        self.assertTrue(
            imrebin(
                imagename=imagename, outfile=outfile,
                factor=[2,2,1]
            )
        )
        self.assertRaises(
            Exception, myia.rebin, "", [2,2,2]
        )
        rebin.done()
        myia.done()
        self.assertFalse(
            imrebin(
                imagename=imagename, outfile=outfile,
                factor=[2,2,2]
            )
        )
        
    def test_crop(self):
        """Test crop parameter"""
        myia = self._myia
        imagename = "xxyy.im"
        myia.fromshape(imagename, [20, 20, 20])
        factor = [3,3,3]
        zz = myia.rebin("", bin=factor, crop=True)
        self.assertTrue((zz.shape() == [6,6,6]).all())
        zz = myia.rebin("", bin=factor, crop=False)
        self.assertTrue((zz.shape() == [7,7,7]).all())
        myia.done()
        zz.done()
        outfile = "outxdkd.im"
        imrebin(imagename=imagename, outfile=outfile, factor=factor, crop=True)
        zz.open(outfile)
        self.assertTrue((zz.shape() == [6,6,6]).all())
        zz.done()
        imrebin(imagename=imagename, outfile=outfile, factor=factor, crop=False, overwrite=True)
        zz.open(outfile)
        self.assertTrue((zz.shape() == [7,7,7]).all())
        zz.done()

    def test_dropdeg(self):
        """Test dropdeg parameter"""
        myia = self._myia
        imagename = "kjfasd.im"
        myia.fromshape(imagename, [20, 20, 1])
        factor = [5,5]
        zz = myia.rebin("", bin=factor, dropdeg=True)
        myia.done()
        self.assertTrue((zz.shape() == [4,4]).all())
        zz.done()
        outfile = "dkfajfas.im"
        imrebin(imagename=imagename, outfile=outfile, factor=factor, dropdeg=True)
        myia.open(outfile)
        self.assertTrue((myia.shape() == [4,4]).all())
        myia.done()
   
    def test_box(self):
        """Test use of box"""
        myia = self._myia
        imagename = "erzvd.im"
        myia.fromshape(imagename, [30, 30, 1])
        factor = [5,5]
        zz = myia.rebin("", bin=factor, region=rg.box([5,5,0],[25,25,0]),crop=True)
        myia.done()
        print "*** shape ",zz.shape()
        self.assertTrue((zz.shape() == [4,4,1]).all())
        zz.done()
        outfile = "vcsfea.im"
        imrebin(imagename=imagename, outfile=outfile, factor=factor, box="5,5,25,25",crop=True)
        myia.open(outfile)
        self.assertTrue((myia.shape() == [4,4,1]).all())
        myia.done()
 
def suite():
    return [ia_rebin_test]
