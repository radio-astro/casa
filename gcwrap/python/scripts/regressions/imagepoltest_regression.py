##############################################################################
#                                                                            #
# Test Name:                                                                 #
#    imagepoltest_regression.py                                              #
# Rationale for Inclusion:                                                   #
#    This is a Python translation of the Glish assay test                    #
#    imagepolservertest.g  It tests many imagepol tool methods.              #
#    The first test is a general test of everything at a basic level.        #
#    Succeeding tests work on individual areas of the imagepol tool.         #
#    Some of the tests included forced errors.  As long as imagepoltest      #
#    finally returns True it has succeeded (regardless of what error         #
#    messages you might see).                                                #
# Features Tested:                                                           #
#    open, done, summary, stokesi, stokesq, stokesu, stokesv, stokes,        #
#    linpolint, linpolposang, totpolint, fraclinpol, fractotpol, pol,        #
#    sigmalinpolint, sigmalinpolposang, sigmatotpolint, stokes               #
#    sigmastokesi, sigmastokesq, sigmastokesu, sigmastokesv, sigmastokes     #
#    sigmafraclinpol, sigmafractotpol, imagepoltestimage, rotationmeasure    #
#    fourierrotationmeasure, complexlinpol, complexfraclinpol, makecomplex,  #
#    depolratio, sigmadepolratio                                             #
# Success/failure criteria:                                                  #
#    Internally tests each method for pass/fail.  Throws an uncaught         #
#    exception ending test when an unexpected failure occurs.                #
#    All tests pass if script runs to completion.                            #
#                                                                            #
##############################################################################
#                                                                            #
# Converted by RRusk 2007-11-16 from imagepolservertest.g                    #
#                                                                            #
##############################################################################

import os
import shutil
import time
import math
import random
from regression_utility import note

po = casac.imagepol()

def imagepoltest(which=None):
    #
    dowait = true
    #
    def info(message):
        note(message, origin="imagepoltest")
    def stop(message):
        note(message, priority="SEVERE", origin="imagepoltest")
        raise RuntimeError, message
    def fail(message=""):
        stop(message)
    def cleanup(dir):
        if (os.path.isdir(dir)):
            info("Cleaning up directory "+dir)
            def errFunc(raiser, problemPath, excInfo):
                note(raiser.__name__+'failed on'+problemPath,"SEVERE")
                raise RuntimeError, "Cleanup of " + dir + " fails!"
            shutil.rmtree(dir,0,errFunc)
        return True

    def addnoise(data, sigma):
        if len(data.shape)==3:
            for i in range(data.shape[0]):
                for j in range(data.shape[1]):
                    for k in range(data.shape[2]):
                        data[i][j][k] += random.normalvariate(0.0, sigma)
        else:
            stop('unhandled array shape '+str(data.shape)+' in addnoise')
        return True

    # 3D only
    def make_data(imshape, stokes):
        if imshape[2]>4: fail()
        if imshape[2]!=len(stokes): fail()
        data = ia.makearray(0, [imshape[0], imshape[1], imshape[2]])
        for k in range(imshape[2]):
            for i in range(imshape[0]):
                for j in range(imshape[1]):
                    data[i][j][k] = stokes[k]
        return data

    def alleqnum(x,num,tolerance=0):
        if len(x.shape)==3:
            for i in range(x.shape[0]):
                for j in range(x.shape[1]):
                    for k in range(x.shape[2]):
                        if not (abs(x[i][j][k]-num) < tolerance):
                            print "x[",i,"][",j,"][",k,"]=", x[i][j][k]
                            return false
        else:
            stop('unhandled array shape in alleqnum')
        return true
        
    def mean(x):
        result=0
        if len(x.shape)==2:
            for i in range(x.shape[0]):
                for j in range(x.shape[1]):
                    result += x[i][j]
            result = result/float(x.shape[0]*x.shape[1])
        else:
            if len(x.shape)==3:
                for i in range(x.shape[0]):
                    for j in range(x.shape[1]):
                        for k in range(x.shape[2]):
                            result += x[i][j][k]
                result = result /float(x.shape[0]*x.shape[1]*x.shape[2])
            else:
                stop('unhandled array shape '+str(x.shape)+' in mean')
        return result

    def max_with_location(x):
        max=-1e16
        loc=[-1,-1,-1,-1]
        if len(x.shape)==4:
            for i in range(x.shape[0]):
                for j in range(x.shape[1]):
                    for k in range(x.shape[2]):
                        for l in range(x.shape[3]):
                            if x[i][j][k][l] > max:
                                max = x[i][j][k][l]
                                loc = [i,j,k,l]
        else:
            stop('unhandled array shape '+str(x.shape)+' in max_with_location')
        return max, loc
    
    def test1():
        info('')
        info('')
        info('')
        info('Test 1 - open, done, and summary')
        testdir = 'imagepoltest_temp'
        if not cleanup(testdir): return False
        # Make the directory
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        #
        # Make RA/DEC only
        #
        imname = testdir+'/imagefromshape.image'
        myim = ia.newimagefromshape(imname, [10,10])
        if not myim: fail()
        try:
            note("Expect SEVERE error and failure here")
            po.open(imname)
            isfail = False
        except Exception, e:
            note("Caught expected Exception")
            isfail = True
        if not isfail:
            stop('open 1 unexpectedly did not fail')
        if not myim.remove(True): fail()
        #
        # RA/DEC/I
        #
        myim = ia.newimagefromshape(imname, [10,10,1])
        if not myim: fail()
        try:
            note("Expect SEVERE error and failure here")
            po.open(imname)
            isfail = False
        except Exception, e:
            note("Caught expected Exception")
            isfail = True
        if not isfail:
            stop('open 2 unexpectedly did not fail')
        if not myim.remove(True): fail()
        #
        # RA/DEC/IQ
        #
        myim = ia.newimagefromshape(imname, [10,10,2])
        if not myim: fail()
        try:
            note("Expect SEVERE error and failure here")
            po.open(imname)
            isfail = False
        except Exception, e:
            note("Caught expected Exception")
            isfail = True
        if not isfail:
            stop('open 3 unexpectedly did not fail')
        if not myim.remove(True): fail()
        #
        # RA/DEC/IQU
        #
        myim = ia.newimagefromshape(imname, [10,10,3])
        if not myim: fail()
        try:
            po.open(imname)
            isfail = False
        except Exception, e:
            isfail = True
        if isfail: stop('open 4 failed')
        if not po.done(): fail()
        if not myim.remove(True): fail()
        #
        # RA/DEC/IQUV
        #
        myim = ia.newimagefromshape(imname, [10,10,4])
        if not myim: fail()
        try:
            po.open(imname)
            isfail = False
        except Exception, e:
            isfail = True
        if isfail: stop('open 5 failed')
        if not po.done(): fail()
        #
        # Test tool constructor
        #
        if not(po.open(myim.torecord())):
            return stop('open 6 failed')
        if not po.done(): fail()
        if not(po.open(myim.name())):
            return stop('open 7 failed')

        if not myim.done(): fail()
        #
        # Utility functions
        #
        ok = po.summary()
        if not ok: fail()
        if not po.done(): fail()
        #
        return cleanup(testdir)


    def test2():
        info('')
        info('')
        info('')
        info('Test 2 - stokesi, stokesq, stokesu, stokesv, stokes')
        testdir = 'imagepoltest_temp'
        if not cleanup(testdir): return False
        # Make the directory
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        #
        # Make some data
        #
        shape = [10,10,4]
        stokes = [14,2,3,4]
        data = make_data(shape, stokes)
        if len(data)==0: fail()

        #
        # Make image - RA/DEC/IQUV
        #
        imname = testdir+'/imagefromarray.image'
        myim = ia.newimagefromarray(imname, data)
        if not myim: fail()
        if not myim.done(): fail()

        #
        # Attach image to polarimetry tool
        #
        if not po.open(imname): fail()

        #
        # Get Stokes images
        #
        s = po.stokesi()
        if not s: fail()
        pixels = s.getchunk()
        if not alleqnum(pixels, stokes[0], 0.0001):
            stop('Stokes I values are wrong')
        if not s.done(): fail()
        #
        s = po.stokesq()
        if not s: fail()
        pixels = s.getchunk()
        if not alleqnum(pixels, stokes[1], 0.0001):
            stop('Stokes Q values are wrong')
        if not s.done(): fail()
        #
        s = po.stokesu()
        if not s: fail()
        pixels = s.getchunk()
        if not alleqnum(pixels, stokes[2], 0.0001):
            stop('Stokes U values are wrong')
        if not s.done(): fail()
        #
        s = po.stokesv();    
        if not s: fail()
        pixels = s.getchunk()
        if not alleqnum(pixels, stokes[3], 0.0001):
            stop('Stokes V values are wrong')
        if not s.done(): fail()

        ss = ["i","q","u","v"]
        for i in range(len(ss)):
            s = po.stokes(ss[i])
            if not s: fail()
            pixels = s.getchunk()
            if not alleqnum(pixels, stokes[i], 0.0001):
                stop('Stokes '+ss[i]+' values are wrong')
            if not s.done(): fail()

        #
        try:
            note('Expect SEVERE error and Exception here')
            s = po.stokes('fish')
            isfail = False
        except Exception, e:
            note('Cuaght expected Exception')
            isfail = True
        if not isfail:
            stop('Function stokes unexpectedly did not fail')

        #
        if not po.done(): fail()
        #
        return cleanup(testdir)


    def test3():
        info('')
        info('')
        info('')
        info('Test 3 - linpolint, linpolposang, totpolint')
        #
        testdir = 'imagepoltest_temp'
        if not cleanup(testdir): return False
        # Make the directory
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        #
        # Make some data
        #
        shape = [256,256,4]
        stokes = [14,2,3,4]
        data = make_data(shape, stokes)
        sigma = 0.01 * stokes[1]
        ok = addnoise(data, sigma)
        if not ok: fail()

        #
        # Make image - RA/DEC/IQUV
        #
        imname = testdir+'/imagefromarray.image'
        myim = ia.newimagefromarray(imname, data)
        if not myim: fail()
        if not myim.done(): fail()

        #
        # Open image with polarimetry tool
        #
        if not po.open(imname): fail()

        #
        # Linearly polarized intensity
        #
        pp = math.sqrt(stokes[1]*stokes[1] + stokes[2]*stokes[2] + sigma*sigma)
        s = po.linpolint(debias=F)
        if not s: fail()
        pixels = s.getchunk()
        d = abs(mean(pixels)-pp) / float(pp)
        if d > 0.01:
            stop('Linearly polarized intensity values are wrong')
        if not s.done(): fail()
        #
        pp = math.sqrt(stokes[1]*stokes[1] + stokes[2]*stokes[2] - sigma*sigma)
        s = po.linpolint(debias=T, clip=10.0)
        if not s: fail()
        pixels = s.getchunk()
        d = abs(mean(pixels)-pp) / float(pp)
        if d > 0.01:
            stop('Debiased linearly polarized intensity values (1) are wrong')
        if not s.done(): fail()
        #
        s = po.linpolint(debias=T, clip=10.0, sigma=sigma)
        if not s: fail()
        pixels = s.getchunk()
        d = abs(mean(pixels)-pp) / float(pp)
        if d > 0.01:
            stop('Debiased linearly polarized intensity values (2) are wrong')
        if not s.done(): fail()

        #
        # Linearly polarized position angle
        pp = (180.0/(2.0*math.pi)) * math.atan2(stokes[2], stokes[1])
        s = po.linpolposang()
        if not s: fail()
        pixels = s.getchunk()
        d = abs(mean(pixels)-pp) / float(pp)
        if d > 0.01:
            stop('Linearly polarized position angles are wrong')
        if not s.done(): fail()

        #
        #
        # Total polarized intensity
        #
        pp = math.sqrt(stokes[1]*stokes[1] + stokes[2]*stokes[2] +
                       stokes[3]*stokes[3] + sigma*sigma)
        s = po.totpolint(debias=F)
        if not s: fail()
        pixels = s.getchunk()
        d = abs(mean(pixels)-pp) / float(pp)
        if d>0.1:
            stop('Total polarized intensity values are wrong')
        if not s.done(): fail()
        #
        pp = math.sqrt(stokes[1]*stokes[1] + stokes[2]*stokes[2] +
                       stokes[3]*stokes[3] - sigma*sigma)
        s = po.totpolint(debias=T, clip=10.0)
        if not s: fail()
        pixels = s.getchunk()
        d = abs(mean(pixels)-pp) / float(pp)
        if d>0.1:
            stop('Debiased total polarized intensity values (1) are wrong')
        if not s.done(): fail()
        #
        s = po.totpolint(debias=T, clip=10.0, sigma=sigma)
        if not s: fail()
        pixels = s.getchunk()
        d = abs(mean(pixels)-pp) / float(pp)
        if d>0.1:
            stop('Debiased total polarized intensity values (2) are wrong')
        if not s.done(): fail()
        #
        if not po.done(): fail()
        #
        return cleanup(testdir)

    def test4():
        info('')
        info('')
        info('')
        info('Test 4 - fraclinpol, fractotpol')
        #
        testdir = 'imagepoltest_temp'
        if not cleanup(testdir): return False
        # Make the directory
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        #
        # Make some data
        #
        shape = [256,256,4]
        stokes = [14,2,3,4]
        data = make_data(shape, stokes)
        sigma = 0.01 * stokes[1]
        ok = addnoise(data, sigma)
        if not ok: fail()

        #
        # Make image - RA/DEC/IQUV
        #
        imname = testdir+'/imagefromarray.image'
        myim = ia.newimagefromarray(imname, data)
        if not myim: fail()
        if not myim.done(): fail()

        #
        # Open image with polarimetry tool
        #
        if not po.open(imname): fail()

        #
        # Fractional linear polarization
        #
        pp = math.sqrt(stokes[1]*stokes[1] + stokes[2]*stokes[2] +
                       sigma*sigma) / float(stokes[0])
        s = po.fraclinpol(debias=F)
        if not s: fail()
        pixels = s.getchunk()
        d = abs(mean(pixels)-pp) / float(pp)
        if d>0.1:
            stop('Fractional linear polarization values are wrong')
        if not s.done(): fail()
        #
        pp = math.sqrt(stokes[1]*stokes[1] + stokes[2]*stokes[2] -
                       sigma*sigma) / float(stokes[0])
        s = po.fraclinpol(debias=T, clip=10.0)
        if not s: fail()
        pixels = s.getchunk()
        d = abs(mean(pixels)-pp) / float(pp)
        if d>0.1:
            stop('Debiased fractional linear polarization values (1) are wrong')
        if not s.done(): fail()
        #
        s = po.fraclinpol(debias=T, clip=10.0, sigma=sigma)
        if not s: fail()
        pixels = s.getchunk()
        d = abs(mean(pixels)-pp) / float(pp)
        if d>0.1:
            stop('Debiased fractional linear polarization values (2) are wrong')
        if not s.done(): fail()

        #
        # Fractional total polarization
        #
        pp = math.sqrt(stokes[1]*stokes[1] + stokes[2]*stokes[2] +
                       stokes[3]*stokes[3] + sigma*sigma) / float(stokes[0])
        s = po.fractotpol(debias=F)
        if not s: fail()
        pixels = s.getchunk()
        d = abs(mean(pixels)-pp) / float(pp)
        if d>0.1:
            stop('Fractional total polarization values are wrong')
        if not s.done(): fail()
        #
        pp = math.sqrt(stokes[1]*stokes[1] + stokes[2]*stokes[2] +
                       stokes[3]*stokes[3] - sigma*sigma) / float(stokes[0])
        s = po.fractotpol(debias=T, clip=10.0)
        if not s: fail()
        pixels = s.getchunk()
        d = abs(mean(pixels)-pp) / float(pp)
        if d>0.1:
            stop('Debiased fractional total polarization values (1) are wrong')
        if not s.done(): fail()
        #
        s = po.fractotpol(debias=T, clip=10.0, sigma=sigma)
        if not s: fail()
        pixels = s.getchunk()
        d = abs(mean(pixels)-pp) / float(pp)
        if d>0.1:
            stop('Debiased fractional total polarization values (2) are wrong')
        if not s.done(): fail()
        #
        if not po.done(): fail()
        #
        return cleanup(testdir)

    def test5():
        info('')
        info('')
        info('')
        info('Test 5 - pol')
        #
        testdir = 'imagepoltest_temp'
        if not cleanup(testdir): return False
        # Make the directory
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        #
        # Make some data
        #
        shape = [256,256,4]
        stokes = [14,2,3,4]
        data = make_data(shape, stokes)
        sigma = 0.01 * stokes[1]
        ok = addnoise(data, sigma)
        if not ok: fail()

        #
        # Make image - RA/DEC/IQUV
        #
        imname = testdir+'/imagefromarray.image'
        myim = ia.newimagefromarray(imname, data)
        if not myim: fail()
        if not myim.done(): fail()

        #
        # Open image with polarimetry tool
        #
        if not po.open(imname): fail()

        #
        # We just test that each function runs, not its results, as this
        # just packages previously tested functions
        #
        which = ["lpi","tpi","lppa","flp","ftp"]
        for i in which:
            s = po.pol(i, debias=F)
            if not s: fail()
            if not s.done(): fail()
            #
            s = po.pol(i, debias=T, clip=10.0)
            if not s: fail()
            if not s.done(): fail()
            #
            s = po.pol(i, debias=T, clip=10.0, sigma=sigma)
            if not s: fail()
            if not s.done(): fail()
        try:
            note("Expect SEVERE error and Exception here")
            s = po.pol('fish')
            isfail = False
        except Exception, e:
            note("Caught expected Exception")
            isfail = True
        if not isfail:
            stop('Function pol unexpectedly did not fail')

        #
        if not po.done(): fail()
        return cleanup(testdir)

    def test6():
        info('')
        info('')
        info('')
        info('Test 6 - sigmalinpolint, sigmalinpolposang, sigmatotpolint')
        #
        testdir = 'imagepoltest_temp'
        if not cleanup(testdir): return False
        # Make the directory
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        #
        # Make some data
        #
        shape = [256,256,4]
        stokes = [14,2,3,4]
        data = make_data(shape, stokes)
        sigma = 0.01 * stokes[1]
        ok = addnoise(data, sigma)
        if not ok: fail()

        #
        # Make image - RA/DEC/IQUV
        #
        imname = testdir+'/imagefromarray.image'
        myim = ia.newimagefromarray(imname, data)
        if not myim: fail()
        if not myim.done(): fail()

        #
        # Open image with polarimetry tool
        #
        if not po.open(imname): fail()

        #
        # Error in linearly polarized intensity
        #
        s = po.sigmalinpolint(clip=10.0)
        if not s: fail()
        d = abs(s-sigma)/float(sigma)
        if d>0.01:
            stop('Sigma for linearly polarized intensity (1) is wrong')
        #
        s = po.sigmalinpolint(clip=10.0, sigma=sigma)
        if not s: fail()
        d = abs(s-sigma)/float(sigma)
        if d>0.1:
            stop('Sigma for linearly polarized intensity (2) is wrong')

        #
        # Error in linearly polarized position angle
        #
        s = po.sigmalinpolposang(clip=10.0)
        if not s: fail()
        data = s.getchunk()
        if not s.done(): fail()
        lpi = math.sqrt(stokes[1]*stokes[1] + stokes[2]*stokes[2])
        s2 = 180.0 * sigma / float(lpi) / 2.0 / math.pi
        if not abs((mean(data)-s2)/float(s2)) < 0.01:
            stop('Sigma for linearly polarized position angle (1) is wrong')
        #
        s = po.sigmalinpolposang(clip=10.0, sigma=sigma)
        if not s: fail()
        data = s.getchunk()
        if not s.done(): fail()
        lpi = math.sqrt(stokes[1]*stokes[1] + stokes[2]*stokes[2])
        s2 = 180.0 * sigma / float(lpi) / 2.0 / math.pi
        d = abs(mean(data)-s2)/float(s2)
        if d>0.1:
            stop('Sigma for linearly polarized position angle (2) is wrong')

        #
        # Error in total linearly polarized intensity
        #
        s = po.sigmatotpolint(clip=10.0)
        if not s: fail()
        d = abs(s-sigma)/float(sigma)
        if d>0.1:
            stop('Sigma for total polarized intensity (1) is wrong')
        #
        s = po.sigmatotpolint(clip=10.0, sigma=sigma)
        if not s: fail()
        d = abs(s-sigma)/float(sigma)
        if d>0.1:
            stop('Sigma for total polarized intensity (2) is wrong')

        #
        # Cleanup
        #
        if not po.done(): fail()
        return cleanup(testdir)

    def test7():
        info('')
        info('')
        info('')
        info('Test 7 - sigma, sigmastokesi, sigmastokesq, sigmastokesu, sigmastokesv, sigmastokes')
        #
        testdir = 'imagepoltest_temp'
        if not cleanup(testdir): return False
        # Make the directory
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        #
        # Make some data
        #
        shape = [256,256,4]
        stokes = [14,2,3,4]
        data = make_data(shape, stokes)
        sigma = 0.01 * stokes[1]
        ok = addnoise(data, sigma)
        if not ok: fail()

        #
        # Make image - RA/DEC/IQUV
        #
        imname = testdir+'/imagefromarray.image'
        myim = ia.newimagefromarray(imname, data)
        if not myim: fail()
        if not myim.done(): fail()

        #
        # Open image with polarimetry tool
        #
        if not po.open(imname): fail()

        #
        # Best guess at thermal noise
        #
        s = po.sigma(clip=100.0)
        if not s: fail()
        d = abs(s-sigma)/float(sigma)
        if d>0.1:
            stop('Sigma is wrong')

        #
        # Error in stokes I, Q, U, V
        #
        s = po.sigmastokesi(clip=100.0)
        if not s: fail()
        d = abs(s-sigma)/float(sigma)
        if d>0.1:
            stop('Sigma for Stokes I is wrong')
        #
        s = po.sigmastokesq(clip=100.0)
        if not s: fail()
        if not abs(s-sigma) < 0.001:
            stop('Sigma for Stokes Q is wrong')
        #
        s = po.sigmastokesu(clip=100.0)
        if not s: fail()
        d = abs(s-sigma)/float(sigma)
        if d>0.1:
            stop('Sigma for Stokes U is wrong')
        #
        s = po.sigmastokesv(clip=100.0)
        if not s: fail()
        d = abs(s-sigma)/float(sigma)
        if d>0.1:
            stop('Sigma for Stokes V is wrong')
        #
        which = ["I","Q","U","V"]
        for i in which:
            s = po.sigmastokes(which=i, clip=100.0)
            if not s: fail()
        try:
            note("Expect SEVERE error and Exception here")
            s = po.sigmastokes(which='fish')
            isfail = false
        except Exception, e:
            note("Caught expected exception")
            isfail = true
        if not isfail:
            stop('Function sigmastokes unexpectedly did not fail')

        #
        # Cleanup
        #
        if not po.done(): fail()
        return cleanup(testdir)

    def test8():
        info('')
        info('')
        info('')
        info('Test 8 - sigmafraclinpol, sigmafractotpol')
        #
        testdir = 'imagepoltest_temp'
        if not cleanup(testdir): return False
        # Make the directory
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        #
        # Make some data
        #
        shape = [256,256,4]
        stokes = [14,2,3,4]
        data = make_data(shape, stokes)
        sigma = 0.01 * stokes[1]
        ok = addnoise(data, sigma)
        if not ok: fail()

        #
        # Make image - RA/DEC/IQUV
        #
        imname = testdir+'/imagefromarray.image'
        myim = ia.newimagefromarray(imname, data)
        if not myim: fail()
        if not myim.done(): fail()

        #
        # Open image with polarimetry tool
        #
        if not po.open(imname): fail()

        #
        # Error in fractional linearly polarized intensity
        #
        s = po.sigmafraclinpol(clip=10.0)
        if not s: fail()
        data = s.getchunk()
        if not s.done(): fail()
        pi = math.sqrt(stokes[1]*stokes[1] + stokes[2]*stokes[2])
        m = pi / float(stokes[0])
        s2 = m * math.sqrt( (sigma/float(pi))*(sigma/float(pi)) +
                            (sigma/float(stokes[0]))*(sigma/float(stokes[0])))
        d = abs(mean(data)-s2)/float(s2)
        if d>0.1:
            stop('Sigma for fractional linear polarization (1) is wrong')
        #
        s = po.sigmafraclinpol(clip=10.0, sigma=sigma)
        if not s: fail()
        data = s.getchunk()
        if not s.done(): fail()
        pi = math.sqrt(stokes[1]*stokes[1] + stokes[2]*stokes[2])
        m = pi / float(stokes[0])
        s2 = m * math.sqrt( (sigma/float(pi))*(sigma/float(pi)) +
                            (sigma/float(stokes[0]))*(sigma/float(stokes[0])))
        d = abs(mean(data)-s2)/float(s2)
        if d>0.1:
            stop('Sigma for fractional linear polarization (2) is wrong')

        #
        # Error in fractional total polarized intensity
        #
        s = po.sigmafractotpol(clip=10.0)
        if not s: fail()
        data = s.getchunk()
        if not s.done(): fail()
        pi = math.sqrt(stokes[1]*stokes[1] + stokes[2]*stokes[2] +
                       stokes[3]*stokes[3])
        m = pi / float(stokes[0])
        s2 = m * math.sqrt( (sigma/float(pi))*(sigma/float(pi)) +
                            (sigma/float(stokes[0]))*(sigma/float(stokes[0])))
        d = abs(mean(data)-s2)/float(s2)
        if d>0.1:
            stop('Sigma for fractional total polarization (1) is wrong')
        #
        s = po.sigmafractotpol(clip=10.0, sigma=sigma)
        if not s: fail()
        data = s.getchunk()
        if not s.done(): fail()
        pi = math.sqrt(stokes[1]*stokes[1] + stokes[2]*stokes[2] +
                       stokes[3]*stokes[3])
        m = pi / float(stokes[0])
        s2 = m * math.sqrt( (sigma/float(pi))*(sigma/float(pi)) +
                            (sigma/float(stokes[0]))*(sigma/float(stokes[0])))
        d = abs(mean(data)-s2)/float(s2)
        if d>0.1:
            stop('Sigma for fractional total polarization (2) is wrong')

        #
        # Cleanup
        #
        if not po.done(): fail()
        return cleanup(testdir)

    def test9():
        info('')
        info('')
        info('')
        info('Test 9 - imagepoltestimage, rotationmeasure')
        #
        testdir = 'imagepoltest_temp'
        if not cleanup(testdir): return False
        # Make the directory
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        #
        # Make imagepoltestimage
        #
        outfile = testdir+'/imagepoltestimage.image'
        rm = 200.0
        pa0 = 10.0
        sigma = 0.001
        nf = 32
        f0 = 1.4e9
        bw  = 128.0e6
        if not po.imagepoltestimage (outfile=outfile, rm=rm, pa0=pa0,
                                     sigma=sigma, nx=32, ny=32, nf=nf,
                                     f0=f0, bw=bw): fail()
        #
        # Rotation Measure
        #
        rmmax = rm + 100.0
        maxpaerr = 100000.0
        rmfg = 0.0
        rmname = testdir+'/rm.image'
        rmename = testdir+'/rme.image'
        pa0name = testdir+'/pa0.image'
        pa0ename = testdir+'/pa0e.image'
        ok = po.rotationmeasure(rm=rmname, pa0=pa0name,
                                rmerr=rmename, pa0err=pa0ename,
                                sigma=sigma, rmfg=rmfg,
                                rmmax=rmmax, maxpaerr=maxpaerr)
        if not ok: fail()

        #
        # Check results
        #
        rmim = ia.newimagefromfile(rmname)
        rmeim = ia.newimagefromfile(rmename)
        pa0im = ia.newimagefromfile(pa0name)
        pa0eim = ia.newimagefromfile(pa0ename)
        #
        err = mean(rmeim.getchunk())
        diff = mean(rmim.getchunk()) - rm
        if abs(diff) > 3*err:
            stop('Recovered RM is wrong')
        #
        err = mean(pa0eim.getchunk())
        diff = mean(pa0im.getchunk()) - pa0
        if abs(diff) > 3*err:
            stop('Recovered Position Angle at zero frequency is wrong')

        #
        # Cleanup
        #
        if not po.done(): fail()
        if not rmim.done(): fail()
        if not rmeim.done(): fail()
        if not pa0im.done(): fail()
        if not pa0eim.done(): fail()
        #
        return cleanup(testdir)

    def test10():
        info('')
        info('')
        info('')
        info('Test 10 - fourierrotationmeasure')
        #
        testdir = 'imagepoltest_temp'
        if not cleanup(testdir): return False
        # Make the directory
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        #
        # Make imagepoltestimage
        #
        outfile = testdir+'/imagepoltestimage.image'
        rm = 1.0e6
        pa0 = 0.0
        sigma = 0.0001
        nf = 512
        f0 = 1.4e9
        bw = 15.625e3 * nf
        if not po.imagepoltestimage (outfile=outfile, rm=rm, pa0=pa0,
                                     sigma=sigma, nx=1, ny=1, nf=nf,
                                     f0=f0, bw=bw): fail()
        #
        # Fourier Rotation Measure
        #
        ampname = testdir+'/amp.image'
        ok = po.fourierrotationmeasure(amp=ampname)
        if not ok: fail()

        #
        # Check results
        #
        ampim = ia.newimagefromfile(ampname)
        srec = ampim.summary(list=F)
        rminc = srec['incr'][3]
        rmrefpix = srec['refpix'][3]
        idx = int((rm+rminc/2.0)/float(rminc) + rmrefpix)
        y = ampim.getchunk()
        max,loc = max_with_location(y)
        if idx != loc[3]:
            stop('Peak of RM spectrum is in wrong channel')

        #
        # Cleanup
        #
        if not po.done(): fail()
        if not ampim.done(): fail()
        #
        return cleanup(testdir)

    def test11():
        info('')
        info('')
        info('')
        info('Test 11 - complexlinpol, complexfraclinpol, makecomplex')
        #
        testdir = 'imagepoltest_temp'
        if not cleanup(testdir): return False
        # Make the directory
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        #
        # Make some data
        #
        shape = [2,2,4]
        stokes = [14,2,3,4]
        data = make_data(shape, stokes)

        #
        # Make image - RA/DEC/IQUV
        #
        imname = testdir+'/imagefromarray.image'
        myim = ia.newimagefromarray(imname, data)
        if not myim: fail()
        if not myim.done(): fail()

        #
        # Open image with polarimetry tool
        #
        if not po.open(imname): fail()

        #
        # Complex linear polarization
        #
        s = testdir+'/complexpol.image'
        ok = po.complexlinpol(s)
        if not ok: fail()
        #
        expr = 'real("'+s+'")'
        rim = ia.imagecalc(pixels=expr)
        if not rim: fail()
        qpixels = rim.getchunk()
        if len(qpixels)==0: fail()
        if not rim.done(): fail()
        d = abs(mean(qpixels)-stokes[1])
        if d > 0.001:
            stop('Complex linear polarization (1) values (Q) are wrong')
        #
        expr = 'imag("'+s+'")'
        iim = ia.imagecalc(pixels=expr)
        if not iim: fail()
        upixels = iim.getchunk()
        if len(upixels)==0: fail()
        if not iim.done(): fail()
        d = abs(mean(upixels)-stokes[2])
        if d > 0.001:
            stop('Complex linear polarization (1) values (U) are wrong')

        #
        # Complex fractional polarization
        #
        s = testdir+'/complexfracpol.image'
        ok = po.complexfraclinpol(s)
        if not ok: fail()
        #
        expr = 'real("'+s+'")'
        rim = ia.imagecalc(pixels=expr)
        if not rim: fail()
        qpixels = rim.getchunk()
        if len(qpixels)==0: fail()
        if not rim.done(): fail()
        d = abs(mean(qpixels)-(float(stokes[1])/float(stokes[0])))
        if d > 0.001:
            stop('Complex fractional polarization (1) values (Q) are wrong')
        #
        expr = 'imag("'+s+'")'
        iim = ia.imagecalc(pixels=expr)
        if not iim: fail()
        upixels = iim.getchunk()
        if len(upixels) ==0: fail()
        if not iim.done(): fail()
        d = abs(mean(upixels)-(stokes[2]/float(stokes[0])))
        if d > 0.001:
            stop('Complex fractional polarization (1) values (U) are wrong')

        #
        # Makecomplex
        #
        q = po.stokesq()
        qs = testdir+'/q.image'
        q2 = q.subimage(qs)
        if not q.done(): fail()
        if not q2.done(): fail()
        #
        u = po.stokesu()
        us = testdir+'/u.image'
        u2 = u.subimage(us)
        if not u.done(): fail()
        if not u2.done(): fail()
        #
        lpi = po.linpolint()
        lpis = testdir+'/lpi.image'
        lpi2 = lpi.subimage(lpis)
        if not lpi.done(): fail()
        if not lpi2.done(): fail()
        #
        lppa = po.linpolposang()
        lppas = testdir+'/lppa.image'
        lppa2 = lppa.subimage(lppas)
        if not lppa.done(): fail()
        if not lppa2.done(): fail()
        #
        s = testdir+'/cplx1.image'
        po.makecomplex(s, real=qs, imag=us)
        #
        expr = 'real("'+s+'")'
        rim = ia.imagecalc(pixels=expr)
        if not rim: fail()
        rpixels = rim.getchunk()
        if len(rpixels)==0: fail()
        if not rim.done(): fail()
        d = abs(mean(rpixels)-(stokes[1]))
        if d > 0.001:
            stop('Complex linear polarization (2) values (Q) are wrong')
        #
        expr = 'imag("'+s+'")'
        iim = ia.imagecalc(pixels=expr)
        if not iim: fail()
        ipixels = iim.getchunk()
        if len(ipixels)==0: fail()
        if not iim.done(): fail()
        d = abs(mean(ipixels)-(stokes[2]))
        if d > 0.001:
            stop('Complex linear polarization (2) values (U) are wrong')
        #
        s = testdir+'/cplx2.image'
        po.makecomplex(s, amp=lpis, phase=lppas)
        #
        expr = 'real("'+s+'")'
        rim = ia.imagecalc(pixels=expr)
        if not rim: fail()
        rpixels = rim.getchunk()
        if len(rpixels)==0: fail()
        if not rim.done(): fail()
        d = abs(mean(rpixels)-(stokes[1]))
        if d > 0.001:
            stop('Complex linear polarization (3) values (Q) are wrong')
        #
        expr = 'imag("'+s+'")'
        iim = ia.imagecalc(pixels=expr)
        if not iim: fail()
        ipixels = iim.getchunk()
        if len(ipixels)==0: fail()
        if not iim.done(): fail()
        d = abs(mean(ipixels)-(stokes[2]))
        if d > 0.001:
            stop('Complex linear polarization (3) values (U) are wrong')
        #
        if not po.done(): fail()
        return cleanup(testdir)

    def test12():
        info('')
        info('')
        info('')
        info('Test 12 - depolratio, sigmadepolratio')
        #
        testdir = 'imagepoltest_temp'
        if not cleanup(testdir): return False
        # Make the directory
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        #
        # Make some data
        #
        shape = [256,256,4]
        #
        stokes1 = [14,2,3,0.2]
        data1 = make_data(shape, stokes1)
        sigma1 = 0.01 * stokes1[1]
        if not addnoise(data1, sigma1): fail()
        #
        stokes2 = [13,1.5,2.8,0.1]
        data2 = make_data(shape, stokes2)
        sigma2 = 0.01 * stokes2[1]
        if not addnoise(data2, sigma2): fail()

        #
        # Make images - RA/DEC/IQUV
        #
        imname1 = testdir+'/imagefromarray.image1'
        myim1 = ia.newimagefromarray(imname1, data1)
        if not myim1: fail()
        if not myim1.done(): fail()
        #
        imname2 = testdir+'/imagefromarray.image2'
        myim2 = ia.newimagefromarray(imname2, data2)
        if not myim2: fail()
        if not myim2.done(): fail()
        #
        # Make polarimetry server
        #
        if not po.open(imname1): fail()

        #
        # Depolarization ratio 
        #
        i1 = stokes1[0]
        i2 = stokes1[0]
        ei1 = sigma1
        ei2 = sigma2
        #
        p1 = math.sqrt(stokes1[1]*stokes1[1] + stokes1[2]*stokes1[2])
        p2 = math.sqrt(stokes2[1]*stokes2[1] + stokes2[2]*stokes2[2])
        ep1 = sigma1
        ep2 = sigma2
        #
        m1 = p1 / float(stokes1[0])
        m2 = p2 / float(stokes2[0])
        em1 = m1 * math.sqrt( (ep1*ep1/float(p1*p1)) + (ei1*ei1/float(i1*i1)))
        em2 = m2 * math.sqrt( (ep2*ep2/float(p2*p2)) + (ei2*ei2/float(i2*i2)))
        #
        dd = m1 / float(m2)
        edd = dd * math.sqrt( (em1*em1/float(m1*m1)) + (em2*em2/float(m2*m2)) )
        #
        depol = po.depolratio(infile=imname2, debias=F);       # Use file name
        if not depol: fail()
        pixels = depol.getchunk()
        diff = abs(mean(pixels)-dd) / float(dd)
        if diff > 0.01:
            stop('Depolarization ratio values are wrong')
        if not depol.done(): fail()
        #
        myim2 = ia.newimagefromfile(imname2)
        if not myim2: fail()
        depol = po.depolratio(infile=myim2.name(), debias=F);  # Use Image tool
        if not depol: fail()
        pixels = depol.getchunk()
        diff = abs(mean(pixels)-dd) / float(dd)
        if diff > 0.01:
            stop('Depolarization ratio values are wrong')
        if not myim2.done(): fail()
        if not depol.done(): fail()
        #
        # Error in depolarization ratio
        #
        edepol = po.sigmadepolratio(infile=imname2, debias=F); # Use file name
        if not edepol: fail()
        pixels = edepol.getchunk()
        diff = abs(mean(pixels)-edd) / float(edd)
        if diff > 0.01:
            stop('Depolarization ratio error values are wrong')
        if not edepol.done(): fail()

        #
        if not po.done(): fail()
        return cleanup(testdir)

    #run tests
    test1()
    test2()
    test3()
    test4()
    test5()
    test6()
    test7()
    test8()
    test9()
    test10()
    test11()
    test12()
    print ''
    print 'Regression PASSED'
    print ''
    
    

#####################################################################
#
# Get on with it
#
note ('', priority='WARN', origin='imagepoltest_regression.py')
note ('These tests include intentional errors.  You should expect to see error messages in the log.',
      priority='WARN', origin='imagepoltest.py')
note ('If the test finally returns True, then it has succeeded\n\n',
      priority='WARN', origin='imagepoltest.py')
note ('', priority='WARN', origin='imagepoltest.py')
#

Benchmarking = True
if Benchmarking:
    startTime = time.time()
    regstate = False
    for i in range(1):
        imagepoltest()
    endTime = time.time()
    regstate = True
else:
    imagepoltest()

#exit()
