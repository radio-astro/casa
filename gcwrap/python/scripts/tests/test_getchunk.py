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
import shutil
import numpy

image = "gauss_no_pol.fits"


datapath = os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/getchunk/'


myia = iatool()

class getchunk_test(unittest.TestCase):
    
    def setUp(self):
        os.system('cp -r ' +datapath + image + ' ' + image)

    def tearDown(self):
        os.system('rm -rf ' + image)
        myia.done()
        self.assertTrue(len(tb.showcache()) == 0)

    def test_CAS_2355(self):
        '''getchunk: test 32/64 bit resolution'''
        myia.open(image)
        bb = myia.boundingbox()
        trc = bb['trc']
        blc = bb['blc']
        chunk = myia.getchunk(blc=blc, trc=trc)
        # if the last statement didn't throw an exception, then that's proof enough
        # the fix worked, but just for kicks...
        self.assertTrue(chunk[0][0][0] > 1973 and chunk[0][0][0] < 1974)
        
    def test_getprofile(self):
        """Tests for ia.getprofile()"""
        imagename = "prof.im"
        shutil.copytree(datapath + imagename, imagename)
        myia.open(imagename)
        bb = myia.getchunk()
        axis = 2
        expecworld = numpy.zeros([30])
        for i in range(30):
            expecworld[i] = myia.toworld([0, 0, i])['numeric'][axis]
        csys = myia.coordsys()
        expecblank = csys.frequencytovelocity(expecworld)

        expecradio = csys.frequencytovelocity(expecworld, doppler="radio")
        expecbeta = csys.frequencytovelocity(expecworld, doppler="beta")
        expecoptical = csys.frequencytovelocity(expecworld, doppler="optical")
        c = 2.99792458e+08
        expecm = c/expecworld
        
        vacwavemicrons = expecm*1e6
        vacwave2 = vacwavemicrons*vacwavemicrons
        idx_refrac = 1. + 1e-6 * (287.6155 + 1.62887/vacwave2 + 0.01360/vacwave2/vacwave2)
        expecairwavem = vacwavemicrons/idx_refrac/1e6
        
        csys = csys.torecord()
        shape = myia.shape()
        r1 = ""
        r2 = rg.box([0,0,2], [3,3,10])
        # -1 and not 0 pix because of unexpected WCPolygon behavior, see CAS-6520
        box = "box[[-1pix, -1pix], [3pix, 3pix]]"
        reg1 = rg.fromtext(
            box + " range=[3pix,5pix]", csys=csys, shape=shape
        )
        reg2 = rg.fromtext(
            box + " range=[10pix,15pix]", csys=csys, shape=shape
        )              
        regs = {'reg1': reg1, 'reg2': reg2}
        r3 = rg.makeunion(regs)
        veltypes = ["ra", "o", "re", "b"]
        wavetypes = ["w", "a"]
        spectypes = [""]
        spectypes.extend(veltypes)
        spectypes.extend(wavetypes)
        for function in ['mean', 'sum']:
            for region in [r1, r2, r3]:
                for unit in ["pixel", "", "MHz", "km/s", "cm/s", "m", "cm"]:
                    for spectype in spectypes:
                        if (
                            (
                                (
                                    veltypes.count(spectype) > 0
                                    and ["km/s", "cm/s"].count(unit) == 0
                                )
                                or (
                                    wavetypes.count(spectype) > 0
                                    and ["cm", "m"].count(unit) == 0
                                )
                            )
                        ):
                            continue
                        res = myia.getprofile(
                            function=function, axis=axis,
                            region=region, unit=unit, spectype=spectype
                        )
                        if region == r1:
                            n = 30
                            off = 0
                            self.assertTrue(res['mask'].all())
                        elif region == r2:
                            n = 9
                            off = 2
                            self.assertTrue(res['mask'].all())
                        elif region == r3:
                            n = 13
                            off = 3
                            self.assertTrue(res['mask'][0:2].all())
                            self.assertFalse(res['mask'][3:6].all())
                            self.assertTrue(res['mask'][7:12].all())
                        self.assertTrue(len(res['values'] == n))
                        coords = res['coords']
                        if ["km/s", "cm/s", "m", "cm"].count(unit) == 1:
                            if (spectype == "" and ["km/s", "cm/s"].count(unit) == 1) or spectype == "re" or spectype == "b":
                                expec = numpy.copy(expecbeta)
                            elif spectype == "ra":
                                expec = numpy.copy(expecradio)
                            elif spectype == "o":
                                expec = numpy.copy(expecoptical)
                            elif (spectype == "" and ["m", "cm"].count(unit) == 1) or spectype == "w":
                                expec = numpy.copy(expecm)
                            elif spectype == "a":
                                expec = numpy.copy(expecairwavem)
                            vfac = 1
                            if unit == "cm/s":
                                vfac = 1e5
                            if unit == "cm":
                                vfac = 100
                            expec *= vfac
                        for k in range(n):
                            if region == r3 and k > 2 and k < 7:
                                self.assertTrue(res['values'][k] == 0)
                            else:
                                if function == 'mean':
                                    self.assertTrue(res['values'][k] == numpy.mean(bb[:,:,k + off]))
                                elif function == 'sum':
                                    self.assertTrue(res['values'][k] == numpy.sum(bb[:,:,k + off]))
                            if ["km/s", "cm/s", "m", "cm"].count(unit) == 1:
                                self.assertTrue(abs(coords[k]/expec[k + off] - 1) < 1e-12)   
                            elif unit == "pixel":
                                self.assertTrue(coords[k] == k + off)
                            else:
                                f = 1
                                if unit == 'MHz':
                                    f = 1e6
                                self.assertTrue(coords[k] == expecworld[k + off]/f)                                
                        if unit == "":
                            self.assertTrue(res['xUnit'] == 'Hz')
                        else:
                            self.assertTrue(res['xUnit'] == unit)
        myia.done()
        
        myimd = imdtool()
        myimd.open(imagename)
        rfreq = myimd.get("restfreq")
        myimd.set("restfreq", "1e12THz")
        self.assertTrue(myimd.get("restfreq") != rfreq)
        myimd.done()
        myia.open(imagename)
        res = myia.getprofile(
            function="mean", axis=2, unit="km/s",
            spectype="radio", restfreq=rfreq
        )
        self.assertTrue((res['coords'] == expecradio).all())
       
        res = myia.getprofile(
            function="mean", axis=2,
            restfreq=rfreq, frame="cmb"
        )
        nchan = myia.shape()[2]
        expeccmb = numpy.zeros([nchan])
        csys = myia.coordsys()
        csys.setconversiontype(spectral="cmb")
        for i in range(nchan):
            expeccmb[i] = csys.toworld([0, 0, i])['numeric'][2]
        self.assertTrue((res['coords'] == expeccmb).all())
        myia.done()
        
    def test_flux(self):
        """Test the function='flux' option in ia.getprofile()"""
        myia = iatool()
        myia.fromshape("", [20, 20, 20, 1])
        myia.setrestoringbeam(major="3arcmin", minor="3arcmin", pa="0deg")
        myia.setbrightnessunit("Jy/beam")
        bb = myia.getchunk()
        for i in range(20):
            bb[:, :, i, :] = i
        myia.putchunk(bb)
        res = myia.getprofile(axis=2, function='flux')
        myia.done()
        fac = pi/4/log(2)*9
        for i in range(20):
            got = res['values'][i]
            expec = 400*i/fac
            self.assertTrue(abs(got - expec) < 1e-4)

    def test_flux_multibeam(self):
        """Test getprofile() for flux on a multibeam image"""
        myia = iatool()
        myia.fromshape("",[10,10,4,5])
        myia.setbrightnessunit("Jy/beam")
        for c in range(5):
            for p in range(4):
                myia.setrestoringbeam(
                    major=str(4 + c + p)+"arcsec",
                    minor=str(3 + c + p)+"arcsec",
                    pa="20deg",channel=c, polarization=p
                )
        self.assertRaises(Exception, myia.getprofile, function='flux', axis=3)
        self.assertTrue(
            myia.getprofile(
                function='flux', axis=3,
                region=rg.box([0, 0, 0, 0], [9, 9, 0, 4])
            )
        )
        
    def test_CAS7553(self):
        """verify CAS7553, error no longer occurs when getprofile axis is degenerate"""
        myia = iatool()
        myia.fromshape("", [20, 20, 1, 1])
        myia.addnoise()
        bb = myia.getchunk()
        res = myia.getprofile(2)
        myia.done()
        exp = numpy.mean(bb)
        got = res['values'][0]
        self.assertTrue(abs((got - exp)/exp) < 1e-5)

def suite():
    return [getchunk_test]
