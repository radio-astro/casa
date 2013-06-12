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
# Test suite for the CASA task spxfit and tool method ia.fitprofile
# </summary>
#
# <reviewed reviwer="" date="" tests="" demos="">
# </reviewed
#
# <prerequisite>
# <ul>
#   <li> <linkto class="task_specfit.py:description">spxfit</linkto> 
# </ul>
# </prerequisite>
#
# <etymology>
# Test for the spxfit task and ia.fitprofile() tool method.
# </etymology>
#
# <synopsis>
# Test the spxfit task and the ia.fitprofile() method upon which it is built.
# </synopsis> 
#
# <example>
#
# This test runs as part of the CASA python unit test suite and can be run from
# the command line via eg
# 
# `echo $CASAPATH/bin/casapy | sed -e 's$ $/$'` --nologger --log2term -c `echo $CASAPATH | awk '{print $1}'`/code/xmlcasa/scripts/regressions/admin/runUnitTest.py test_spxfit[test1,test2,...]
#
# </example>
#
# <motivation>
# To provide a test standard for the spxfit task to ensure
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


datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/specfit/'
myia = iatool()
myfn = fntool()

def run_fitprofile (
    imagename, box, region, chans, stokes,
    axis, mask, ngauss, poly, multifit, model="",
    residual="", amp="", amperr="", center="", centererr="",
    fwhm="", fwhmerr="", integral="", integralerr="",
    estimates="", logresults=True, pampest="", pcenterest="", pfwhmest="",
    pfix="", gmncomps=0, gmampcon="", gmcentercon="",
    gmfwhmcon="", gmampest=[0], gmcenterest=[0],
    gmfwhmest=[0], gmfix="", logfile="", pfunc="",
    goodamprange=[0.0], goodcenterrange=[0.0], goodfwhmrange=[0.0],
    sigma="", outsigma=""
):
    myia = iatool()
    myia.open(imagename)
    if (not myia.isopen()):
        myia.done()
        raise Exception
    res = myia.fitprofile(
        box=box, region=region, chans=chans,
        stokes=stokes, axis=axis, mask=mask,
        ngauss=ngauss, poly=poly, estimates=estimates,
        multifit=multifit,
        model=model, residual=residual, amp=amp,
        amperr=amperr, center=center, centererr=centererr,
        fwhm=fwhm, fwhmerr=fwhmerr, integral=integral,
        integralerr=integralerr, logresults=logresults, pampest=pampest,
        pcenterest=pcenterest, pfwhmest=pfwhmest, pfix=pfix,
        gmncomps=gmncomps, gmampcon=gmampcon,
        gmcentercon=gmcentercon, gmfwhmcon=gmfwhmcon,
        gmampest=gmampest, gmcenterest=gmcenterest,
        gmfwhmest=gmfwhmest, gmfix=gmfix, logfile=logfile,
        pfunc=pfunc, goodamprange=goodamprange,
        goodcenterrange=goodcenterrange,
        goodfwhmrange=goodfwhmrange, sigma=sigma, outsigma=outsigma    )
    myia.close()
    myia.done()
    return res

def run_specfit(
    imagename, box, region, chans, stokes,
    axis, mask, ngauss, poly, multifit, model="",
    residual="", amp="", amperr="", center="", centererr="",
    fwhm="", fwhmerr="", integral="", integralerr="",
    wantreturn=True, estimates="", logresults=None,
    pampest="", pcenterest="", pfwhmest="", pfix="",
    gmncomps=0, gmampcon="", gmcentercon="",
    gmfwhmcon="", gmampest=[0], gmcenterest=[0],
    gmfwhmest=[0], gmfix="", logfile="", pfunc="",
    goodamprange=[0.0], goodcenterrange=[0.0], goodfwhmrange=[0.0],
    sigma="", outsigma=""
):
    return specfit(
        imagename=imagename, box=box, region=region,
        chans=chans, stokes=stokes, axis=axis, mask=mask,
        ngauss=ngauss, poly=poly, estimates=estimates,
        multifit=multifit,
        model=model, residual=residual, amp=amp,
        amperr=amperr, center=center, centererr=centererr,
        fwhm=fwhm, fwhmerr=fwhmerr, integral=integral,
        integralerr=integralerr,
        wantreturn=wantreturn, logresults=logresults, pampest=pampest,
        pcenterest=pcenterest, pfwhmest=pfwhmest, pfix=pfix,
        gmncomps=gmncomps, gmampcon=gmampcon,
        gmcentercon=gmcentercon, gmfwhmcon=gmfwhmcon,
        gmampest=gmampest, gmcenterest=gmcenterest,
        gmfwhmest=gmfwhmest, gmfix=gmfix, logfile=logfile,
        pfunc=pfunc,
        goodamprange=goodamprange,
        goodcenterrange=goodcenterrange,
        goodfwhmrange=goodfwhmrange, sigma=sigma, outsigma=outsigma
    )


class spxfit_test(unittest.TestCase):
    
    def setUp(self):
        pass

    def tearDown(self):
        myia.done()
        self.assertTrue(len(tb.showcache()) == 0)

    def checkImage(self, gotImage, expectedName):
        expected = iatool()                                
        expected.open(expectedName)
        got = iatool()
        if type(gotImage) == str:
            got.open(gotImage)
        else:
            got = gotImage
        self.assertTrue((got.shape() == expected.shape()).all())
        gotchunk = got.getchunk()
        expchunk = expected.getchunk()
        if (numpy.isnan(gotchunk).any()):
            gotchunk = gotchunk.ravel()
            for i in range(len(gotchunk)):
                if isnan(gotchunk[i]):
                    gotchunk[i] = nanvalue
        if (numpy.isnan(expchunk).any()):
            expchunk = expchunk.ravel()
            for i in range(len(expchunk)):
                if isnan(expchunk[i]):
                    expchunk[i] = nanvalue
        diffData = gotchunk - expchunk
        self.assertTrue(abs(diffData).max() < 2e-11)
        self.assertTrue(
            (
                got.getchunk(getmask=T) == expected.getchunk(getmask=T)
            ).all()
        )
        gotCsys = got.coordsys()
        expectedCsys = expected.coordsys()
        diffPixels = gotCsys.referencepixel()['numeric'] - expectedCsys.referencepixel()['numeric']
        self.assertTrue(abs(diffPixels).max() == 0)
        
        diffRef = gotCsys.referencevalue()['numeric'] - expectedCsys.referencevalue()['numeric']
        # fracDiffRef = (diffRef)/expectedCsys.referencevalue()['numeric'];
        self.assertTrue(abs(diffRef).max() == 0)
        got.close()
        got.done()
        expected.close()
        expected.done()

    def test_exceptions(self):
        """spxfit: Test various exception cases"""
        myia.fromshape("", [1,1,10])
        self.assertRaises(Exception, myia.fitprofile, poly=2, plpest=[1,2])
        
    def test_plpfit(self):
        """ Test fitting a power logarithmic polynomial"""
        imagename = "spxfit.im"
        myia.fromshape(imagename,[2, 2, 100])
        csys = myia.coordsys()
        inc = csys.increment()['numeric']
        inc[2] = 1e7
        csys.setincrement(inc)
        myia.setcoordsys(csys.torecord())
        zz = myia.getchunk()
        plpest = [0.5, 2]
        myfn = fn.powerlogpoly(plpest)
        for i in range(zz.shape[2]):
            world = myia.toworld([0,0,i])['numeric'][2]
            zz[:,:,i] = myfn.f(world/1e9)
        myia.putchunk(zz)
        
        for i in [0,1]:
            if i == 0:
                rec = myia.fitprofile(ngauss=0, spxtype="plp", spxest=plpest)
            if i == 1:
                rec = spxfit(imagename=imagename, spxtype="plp", spxest=plpest)
            sols = rec['plp']['solution'].ravel()
            self.assertTrue((abs(1 - sols/plpest) < 0.1e-7).all())
            if i == 1:
                rec = myia.fitprofile(ngauss=0, spxtype="plp", spxest=[0.4, 3])
            if i == 2:
                rec = spxfit(imagename=imagename, spxtype="plp", spxest=[0.4, 3])
            sols = rec['plp']['solution'].ravel()
            self.assertTrue((abs(1 - sols/plpest) < 0.1e-7).all())
            
            
        myia.addnoise(pars=[0, 0.001])
        for i in [0, 1]:
            plpestoff = [0.4, 3]
            if i == 0:
                rec = myia.fitprofile(ngauss=0, spxtype="plp", spxest=plpestoff)
            if i == 1:
                rec = spxfit(imagename=imagename, spxtype="plp", spxest=plpestoff)
            sols = rec['plp']['solution'].ravel()
            print "*** i " + str(i)
            self.assertTrue((abs(1 - sols/plpest) < 0.1e-1).all())
            plpsol = "plpsol.im"
            plperr = "plperr.im"
            plpestoff = [0.4, 2.2]
            plpfix = [False, True]
            if i == 0:
                rec = myia.fitprofile(
                        ngauss=0, spxtype="plp", spxest=plpestoff, spxfix=plpfix,
                        multifit=True, spxsol=plpsol, spxerr=plperr
                )
            if i == 1:
                rec = spxfit(
                    imagename=imagename, spxtype="plp", spxest=plpestoff, spxfix=plpfix,
                    multifit=True, spxsol=plpsol, spxerr=plperr
                )
                myia.done(remove=True)
            sols = rec['plp']['solution']
            self.assertTrue((sols[:,:,:,1] == 2.2).all())
            myia.open(plpsol)
            self.assertTrue(
                (
                    abs(myia.getchunk()/sols - 1) < 1e-7
                ).all()
            )
            myia.done(remove=True)

            myia.open(plperr)
            self.assertTrue(
                (
                    abs(myia.getchunk() - rec['plp']['error']) < 1e-8
                ).all()
            )
            myia.done(remove=True)
            
    def test_ltpfit(self):
        """ Test fitting a logarithmic transformed polynomial"""
        imagename = "ltpfit.im"
        myia.fromshape(imagename,[2, 2, 100])
        csys = myia.coordsys()
        inc = csys.increment()['numeric']
        inc[2] = 1e7
        csys.setincrement(inc)
        myia.setcoordsys(csys.torecord())
        zz = myia.getchunk()
        plpest = [0.5, 2]
        myfn = fn.powerlogpoly(plpest)
        for i in range(zz.shape[2]):
            world = myia.toworld([0,0,i])['numeric'][2]
            zz[:,:,i] = myfn.f(world/1e9)
        myia.putchunk(zz)
        ltpest = plpest
        ltpest[:] = plpest
        ltpest[0] = log(plpest[0])
        for i in [0,1]:
            if i == 0:
                rec = myia.fitprofile(ngauss=0, spxtype="ltp", spxest=ltpest)
            if i == 1:
                rec = spxfit(imagename=imagename, spxtype="ltp", spxest=ltpest)
            print str(rec)
            sols = rec['ltp']['solution'].ravel()
            self.assertTrue((abs(1 - sols/ltpest) < 0.1e-7).all())
            if i == 1:
                rec = myia.fitprofile(ngauss=0, spxtype="ltp", spxest=[0.4, 3])
            if i == 2:
                rec = spxfit(imagename=imagename, spxtype="ltp", spxest=[0.4, 3])
            sols = rec['ltp']['solution'].ravel()
            self.assertTrue((abs(1 - sols/ltpest) < 0.1e-7).all())
        
        myia.addnoise(pars=[0, 0.001])
        for i in [0, 1]:
            ltpestoff = [0.4, 3]
            if i == 0:
                rec = myia.fitprofile(ngauss=0, spxtype="ltp", spxest=ltpestoff)
            if i == 1:
                rec = spxfit(imagename=imagename, spxtype="ltp", spxest=ltpestoff)
            sols = rec['ltp']['solution'].ravel()
            print "*** i " + str(i)
            self.assertTrue((abs(1 - sols/ltpest) < 0.1e-1).all())
            spxsol = "ltpsol.im"
            spxerr = "ltperr.im"
            ltpestoff = [0.4, 2.2]
            ltpfix = [False, True]
            if i == 0:
                rec = myia.fitprofile(
                        ngauss=0,  spxtype="ltp", spxest=ltpestoff, spxfix=ltpfix,
                        multifit=True, spxsol=spxsol, spxerr=spxerr
                )
            if i == 1:
                rec = spxfit(
                    imagename=imagename, spxtype="ltp", spxest=ltpestoff, spxfix=ltpfix,
                    multifit=True, spxsol=spxsol, spxerr=spxerr
                )
                myia.done(remove=True)
            sols = rec['ltp']['solution']
            self.assertTrue((sols[:,:,:,1] == 2.2).all())
            myia.open(spxsol)
            self.assertTrue(
                (
                    abs(myia.getchunk()/sols - 1) < 1e-7
                ).all()
            )
            myia.done(remove=True)

            myia.open(spxerr)
            self.assertTrue(
                (
                    abs(myia.getchunk() - rec['ltp']['error']) < 1e-8
                ).all()
            )
            myia.done(remove=True)
            
            
    def test_multi_image(self):
        """Test multi image support"""
        imagename1 = "concat1.im"
        global myia
        myia.fromshape(imagename1,[2, 2, 100])
        csys = myia.coordsys()
        inc = csys.increment()['numeric']
        inc[2] = 1e7
        csys.setincrement(inc)
        myia.setcoordsys(csys.torecord())
        
        imagename2 = "concat2.im"
        myia.fromshape(imagename2,[2, 2, 100])
        csys = myia.coordsys()
        inc = csys.increment()['numeric']
        inc[2] = 1e7
        csys.setincrement(inc)
        inc[2] = 1e7
        refval = csys.referencevalue()['numeric']
        refval[2] = 3e9
        csys.setreferencevalue(refval)
        myia.setcoordsys(csys.torecord())
        plpest = [0.5, 2]
        for imagename in [imagename1, imagename2]:
            myia.open(imagename)
            zz = myia.getchunk()
            myfn = fn.powerlogpoly(plpest)
            for i in range(zz.shape[2]):
                world = myia.toworld([0,0,i])['numeric'][2]
                zz[:,:,i] = myfn.f(world/1e9)
                myia.putchunk(zz)
        rec = spxfit(imagename=[imagename1, imagename2], spxtype="plp", spxest=plpest, model="model.im")
        sols = rec['plp']['solution'].ravel()
        self.assertTrue((abs(1 - sols/plpest) < 1e-9).all())
        rec = spxfit(imagename=[imagename1, imagename2], spxtype="plp", spxest=[0.4, 3])
        sols = rec['plp']['solution'].ravel()
        self.assertTrue((abs(1 - sols/plpest) < 1e-9).all())
        
        myia.open(imagename1)
        myia.addnoise(pars=[0, 0.001])
        myia.open(imagename2)
        myia.addnoise(pars=[0, 0.001])
        
        plpestoff = [0.4, 3]
        rec = spxfit(imagename=[imagename1, imagename2], spxtype="plp", spxest=plpestoff)
        sols = rec['plp']['solution'].ravel()
        self.assertTrue((abs(1 - sols/plpest) < 1e-2).all())
        
        plpsol = "plpsol.im"
        plperr = "plperr.im"
        plpestoff = [0.4, 2.2]
        plpfix = [False, True]
        rec = spxfit(
            imagename=[imagename1, imagename2], spxtype="plp", spxest=plpestoff,
            spxfix=plpfix, multifit=True, spxsol=plpsol, spxerr=plperr
        )
        myia.done(remove=True)
        sols = rec['plp']['solution']
        self.assertTrue((sols[:,:,:,1] == 2.2).all())
        myia.open(plpsol)
        self.assertTrue(
            (
                abs(myia.getchunk()/sols - 1) < 1e-7
            ).all()
        )
        myia.done(remove=True)

        myia.open(plperr)
        self.assertTrue(
            (
                abs(myia.getchunk() - rec['plp']['error']) < 1e-8
            ).all()
        )
        
def suite():
    return [spxfit_test]
