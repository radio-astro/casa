##############################################################################
#                                                                            #
# Test Name:                                                                 #
#    imagetest_regression.py                                                 #
# Rationale for Inclusion:                                                   #
#    This is a Python translation of the Glish assay test imageservertest.g  #
#    It tests many image tool methods                                        #
# Features Tested:                                                           #
#    fromshape, fromarray, fromimage, fromfits, imageconcat, imagecalc       #
#    done, close, open, isopen, type, lock, unlock, haslock,                 #
#    coordsys, setcoordsys, shape, name, rename, delte, persistent           #
#    getchunk,  putchunk, pixelvalue, getregion, putregion, set              #
#    replacemaskedpixels, FITS conversion, boundingbox, {set}restoringbeam   #
#    coordmeasures, topixel, toworld,                                        #
#    summary, maskhandler, subimage, insert, hanning, convolve, sepconvole,  #
#    LEL, statistics, histograms, moments, modify, fft, regrid,      #
#    convolve2d, deconvolvecomponentlist, findsources, maxfit, adddegaxes,   #
#    addnoise, {set}miscinfo, {set}history, {set}brightnessunit              #
#    {set}restoringbeam, convertflux
#    fromascii, toascii, fitpolynomial, twopointcorrelation, continuumsub,   #
#    rebin, fitprofile                                                       #
# Success/failure criteria:                                                  #
#    Internally tests each method for pass/fail.  Throws an uncaught         #
#    exception ending test when an unexpected failure occurs.                #
#    All tests pass if script runs to completion.                            #
#                                                                            #
##############################################################################
#                                                                            #
# Converted by RRusk 2007-11-08 from imagetest.py                            #
# Updated      RRusk 2007-11-16 fixed some typos, cleaned up temp output     #
#                                                                            #
##############################################################################

import os
import shutil
import time
import math
from numpy import array

#
# imagetest.py: test casapy image tool
#

#
# Utils
#
def note(message, priority="NORMAL", origin="imagetest", ntime=None, postcli='F'):
    #if not ntime:  #if (ntime==None):
    #    ntime=time.asctime()
    #print ntime, priority, origin, message
    print message
    casalog.postLocally(message, priority, origin)

def info(message):
    #note(message,origin='imagetest')
    print message
    casalog.postLocally(message, priority="NORMAL", origin='imagetest')

def fail(message=""):
    casalog.postLocally(message, priority="SEVERE", origin='imagetest')
    raise RuntimeError, message

    #Glish's all() returns T if every element of x is either T (if x's
    #type is boolean) or non-zero (otherwise). It returns F if any
    #element of x is either F or zero. For example,
    #
    #all(y > 3)
    #
    #returns T if-and-only-if every element of y is greater than 3.
    #If x is not numeric an error is generated and an undefined value
    #is returned.
def all(x,y):
    if len(x) != len(y):
        return false
    for i in range(len(x)):
        if not (x[i]==y[i]):
            print "x["+str(i)+"]=", x[i]
            print "y["+str(i)+"]=", y[i]
            return false
    return true

def allbooleq(x,y):
    if len(x) != len(y):
        return false
    if len(x.shape)==1:
        for i in range(x.shape[0]):
            if not (x[i]==y[i]):
                return false
    if len(x.shape)==2:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                if not (x[i][j]==x[i][j]):
                    return false
    if len(x.shape)==3:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    if not (x[i][j][k]==y[i][j][k]):
                        return false
    if len(x.shape)==4:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    for l in range(x.shape[3]):
                        if not (x[i][j][k][l]==y[i][j][k][l]):
                            return false
    if len(x.shape)>4:
        stop('unhandled array shape in all')
    return true

def alltrue(x):
    if len(x.shape)==1:
        for i in range(x.shape[0]):
            if not (x[i]):
                return false
    if len(x.shape)==2:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                if not (x[i][j]):
                    return false
    if len(x.shape)==3:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    if not (x[i][j][k]):
                        return false
    if len(x.shape)==4:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    for l in range(x.shape[3]):
                        if not (x[i][j][k][l]):
                            return false
    if len(x.shape)>4:
        stop('unhandled array shape in alltrue')
    return true

def alleq(x,y,tolerance=0):
    if x.size != y.size:
        print "x.size=", x.size
        print "y.size=", y.size
        return false
    if len(x.shape)==1:
        for i in range(x.shape[0]):
            if not (abs(x[i]-y[i]) < tolerance):
                print "x[",i,"]=", x[i]
                print "y[",i,"]=", y[i]
                return false
    if len(x.shape)==2:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                if not (abs(x[i][j]-y[i][j]) < tolerance):
                    print "x[",i,"][",j,"]=", x[i][j]
                    print "y[",i,"][",j,"]=", y[i][j]
                    return false
    if len(x.shape)==3:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    if not (abs(x[i][j][k]-y[i][j][k]) < tolerance):
                        print "x[",i,"][",j,"][",k,"]=", x[i][j][k]
                        print "y[",i,"][",j,"][",k,"]=", y[i][j][k]
                        return false
    if len(x.shape)==4:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    for l in range(x.shape[3]):
                        if not (abs(x[i][j][k][l]-y[i][j][k][l]) < tolerance):
                            print "x[",i,"][",j,"][",k,"][",l,"]=", x[i][j][k][l]
                            print "y[",i,"][",j,"][",k,"][",l,"]=", y[i][j][k][l]
                            return false
    if len(x.shape)>4:
        stop('unhandled array shape in alleq')
    return true

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

def datamax(x):
    if len(x.shape)==1:
        dmax = x[0]
        for i in range(x.shape[0]):
            if dmax < x[i]: dmax = x[i]
        return dmax
    if len(x.shape)==2:
        dmax = x[0][0]
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                if dmax < x[i][j]: dmax = x[i][j]
        return dmax
    if len(x.shape)==3:
        dmax = x[0][0][0]
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    if dmax < x[i][j][k]: dmax = x[i][j][k]
        return dmax
    if len(x.shape)==4:
        dmax = x[0][0][0][0]
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    for l in range(x.shape[3]):
                        if dmax < x[i][j][k][l]: dmax = x[i][j][k][l]
        return dmax
    if len(x.shape)>4:
        stop('unhandled array shape in max')

def imagedemo():
    #
    # Cleanup
    #
    def cleanup(dir):
        if (os.path.isdir(dir)):
            info("Cleaning up directory "+dir)
            def errFunc(raiser, problemPath, excInfo):
                note(raiser.__name__+'failed on'+problemPath,"SEVERE")
                raise RuntimeError, "Cleanup of " + dir + " fails!"
            shutil.rmtree(dir,0,errFunc)
        return true

    demodir = "./demoimage_temp"
    if not cleanup(demodir): fail()

    #
    # Make directory
    #
    try:
        os.mkdir(demodir,0770)
    except IOError, e:
        note(e, "SEVERE")
        raise RuntimeError, "Failed to make directory " + demodir
    #
    # Manufacture some data   
    #
    nx = 10; ny = 20; nz = 5
    data = ia.makearray(0, [nx,ny,nz])
    file = demodir+"/demoimage.image"
    ia.fromarray(file,data)
    note('Created image='+ ia.name(), origin="imagedemo")
    for k in range(0,nz):
        slice = ia.getchunk(blc=[0,0,k], trc=[nx-1,ny-1,k], list=F)
        for i in range(0,nx):
            slice[i]=list(slice[i])
            for j in range(0,ny):
                slice[i][j] = k+1
            slice[i]=tuple(slice[i])
        ok = ia.putchunk(pixels=slice, blc=[0,0,k], list=F)
        if not ok: fail('putchunk failed')
        note('Set plane ' + str(k) + ' to ' + str(k), origin='imagedemo()')
    stats = ia.statistics(axes=[0,1],async=F)
    #
    file = demodir + '/DEMOIMAGE.FITS'
    ok = ia.tofits(file)
    if not ok: fail('tofits failed')
    ia.close()
    note('Created fits file=' + file, origin='imagedemo')
    return cleanup(demodir)

def imagetest(which=None, size=[32,32,8]):
    #
    dowait = true

    def stop(message):
        note(message, priority='SEVERE',origin='imagetest()')
        raise RuntimeError, message

    def cleanup(dir):
        if (os.path.isdir(dir)):
            info("Cleaning up directory "+dir)
            def errFunc(raiser, problemPath, excInfo):
                note(raiser.__name__+'failed on'+problemPath,"SEVERE")
                raise RuntimeError, "Cleanup of " + dir + " fails!"
            shutil.rmtree(dir,0,errFunc)
        return true

    def compareComponentList(cl0, cl1, tol=0.005, dotype=T):
        n0 = cl0.length()
        n1 = cl1.length()
        errmsg = 'compareComponentList: '
        if (n0 != n1):
            errmsg += 'Number of components differ'
            print cl0.torecord()
            print cl1.torecord()
            info(errmsg)
            return False
        #
        for i in range(0,n0):
            f0 = cl0.getfluxvalue(i)
            f1 = cl1.getfluxvalue(i)
            t = tol * f0[0]
            if not alleq(array(f1),array(f0),tolerance=t):
                errmsg +='Component fluxes differ'
                info(errmsg)
                return False
            #
            shp0 = cl0.getshape(i)
            shp1 = cl1.getshape(i)
            type0 = cl0.shapetype(i)
            type1 = cl1.shapetype(i)
            if (dotype and type0!=type1):
                errmsg+='Component types differ'
                info(errmsg)
                return False
            #
            dir0 = cl0.getrefdir(i)
            dir1 = cl1.getrefdir(i)
            #
            v0 = me.getvalue(dir0)
            v1 = me.getvalue(dir1)
            #
            d = abs(qa.convert(v1['m0'],v0['m0']['unit'])['value'] - v0['m0']['value'])
            t = tol * abs(v0['m0']['value'])
            if (d > t):
                errmsg+='Longitudes differ'
                info(errmsg)
                return False
            #
            d = abs(qa.convert(v1['m1'],v0['m1']['unit'])['value'] - v0['m1']['value'])
            t = tol * abs(v0['m1']['value'])
            if (d > t):
                errmsg+='Latitudes differ'
                info(errmsg)
                return False
            #
            if dotype and (type0=='Gaussian' or type1=='Disk'):
                q0 = shp0['majoraxis']
                q1 = shp1['majoraxis']
                d = abs(qa.convert(q1,q0['unit'])['value']  - q0['value'])
                t = tol * q0['value']
                if (d > t):
                    errmsg+='Major axes differ'
                    info(errmsg)
                    return False
                #
                q0 = shp0['minoraxis']
                q1 = shp1['minoraxis']
                d = abs(qa.convert(q1,q0['unit'])['value']  - q0['value'])
                t = tol * q0['value'];
                if (d > t):
                    errmsg+='Minor axes differ'
                    info(errmsg)
                    return False
                #
                q0 = shp0['positionangle']
                q1 = shp1['positionangle']
                d = abs(qa.convert(q1,q0['unit'])['value']  - q0['value'])
                t = tol * q0['value']
                if (d > t):
                    errmsg+='Position angles differ'
                    info(errmsg)
                    return False
            #
        return True

    def deconvolveTest(myim, majIn, minIn, paIn, iTest):
        smaj = str(majIn) + 'arcsec'
        smin = str(minIn) + 'arcsec'
        spa  = str(paIn) + 'deg'
        #
        ok = myim.setrestoringbeam(major=smaj, minor=smin, pa=spa, log=F)
        if not ok: fail('failed setrestoringbeam in deconvolveTest')
        beam = myim.restoringbeam()
        if not beam: fail('failed restoringbeam in deconvolveTest')
        #
        # Very simple test. Align major axis of source with beam.
        #
        newcl = cltool()
        newcl.simulate(1)
        #
        major = qa.quantity(1.5*majIn, 'arcsec')
        minor = qa.quantity(1.5*minIn, 'arcsec')
        pa = qa.quantity(spa)
        newcl.setshape(which=0, type='Gaussian', majoraxis=major,
                    minoraxis=minor, positionangle=pa,
                    majoraxiserror='0arcsec', minoraxiserror='0arcsec',
                    positionangleerror='0deg', log=F)
        #
        mycs = myim.coordsys()
        refval = mycs.referencevalue(type='dir', format='q')
        newcl.setrefdir(which=0, ra=refval['quantity']['*1'],
                     dec=refval['quantity']['*2'],
                     log=F)
        #
        cl2 = myim.deconvolvecomponentlist(newcl.torecord())
        if not cl2:
            stop ('deconvolvecomponentlist 1 failed')
        cl2tool = cltool()
        cl2tool.fromrecord(cl2)
        shape = cl2tool.getshape(0)
        #
        majin = major['value']
        minin = minor['value']
        pain = qa.convert(pa,'deg')['value']
        majout = shape['majoraxis']['value']
        minout = shape['minoraxis']['value']
        paout = qa.convert(shape['positionangle'],'deg')['value']
        bmaj = beam['major']['value']
        bmin = beam['minor']['value']
        #
        e1 = math.sqrt(majin*majin - bmaj*bmaj)
        d1 = abs(e1 - majout)
        e2 = math.sqrt(minin*minin - bmin*bmin)
        d2 = abs(e2 - minout)
        #
        t1 = zeroToPi (paout)
        t2 = zeroToPi (pain)
        d3 = abs(t1-t2)
        if (d1>1e-5 or d2>1e-5 or d3>1e-5):
            msg = 'deconvolvecomponentlist ' + str(iTest) + ' gave wrong results'
            fail(msg)
        #
        if not newcl.done(): fail('failed done')
        if not cl2tool.done(): fail('failed done')
        return True

    def gaussian(flux, major, minor, pa, dir=None):
        newcl = cltool()
        newcl.simulate(1,log=F);
        #newcl.setspectrum(0)
        newcl.setshape(which=0, type='Gaussian',
                    majoraxis=major, minoraxis=minor, positionangle=pa,
                    majoraxiserror = '0arcsec', minoraxiserror = '0arcsec',
                    positionangleerror = '0deg', log=F);
        flux2 = [flux, 0, 0, 0];
        newcl.setflux(which=0, value=flux2, unit='Jy',
                   polarization='Stokes', log=F);
        if dir==None:
            dir = me.direction('J2000', '0rad', '0rad')
        values = me.getvalue(dir);
        newcl.setrefdir(which=0, ra=values['m0'], dec=values['m1'], log=F);
        return newcl;

    def gaussianarray(nx, ny, height, major, minor, pa):
        pa = 0
        x = ia.makearray(0.0,[nx,ny])
        xc = ((nx - 1)/2)
        yc = ((ny - 1)/2)
        centre = [xc, yc]
        fwhm = [major, minor]
        #
        for j in range(ny):
            for i in range(nx):
                x[i,j] = gaussianfunctional(i, j, 1.0, centre, fwhm, pa)
        return x

    def gaussianfunctional(x, y, height, center, fwhm, pa):
        x -= center[0]
        y -= center[1]
        #
        pa -= math.pi/2
        cpa = math.cos(pa)
        spa = math.sin(pa)
        if (cpa != 1):
            tmp = x
            x =  cpa*tmp + spa*y
            y = -spa*tmp + cpa*y
        #
        width=[0,0]
        width[0] = fwhm[0] / math.sqrt(math.log(16))
        width[1] = fwhm[1] / math.sqrt(math.log(16))
        if (width[0] != 1): x = x/width[0]
        if (width[1] != 1): y = y/width[1]
        #
        x *= x
        y *= y
        #
        return height * math.exp(-(x+y))

    def doneAllImageTypes(rec):
        names = rec.keys()
        for mytype in names:
            ok = rec[mytype]['tool'].done()
            if not ok: fail()
        return True

    def makeAllImageTypes(imshape=[10,10], root="im", data=None,
                          includereadonly=true):
        rec = {}
        # PagedImage
        imname=root+"1"
        rec["pi"] = {}
        if (data==None):
            #rec["pi"]["tool"] = ia.newimagefromshape(imname, shape=imshape)
            ok = ia.fromshape(imname, shape=imshape)
        else:
            #rec["pi"]["tool"] = ia.newimagefromarray(imname, pixels=data)
            ok = ia.fromarray(imname, pixels=data)
        #if not rec["pi"]["tool"]: fail('failed to make PagedImage')
        if not ok: fail('failed to make PagedImage')
        ia.close()
        #rec["pi"]["tool"] = ia.newimage(imname)
        rec["pi"]["tool"] = ia.newimagefromfile(imname)
        rec["pi"]["type"] = "PagedImage"

        #FITSImage
        if includereadonly:
            fitsname = imname +'.fits'
            ok = rec["pi"]["tool"].tofits(fitsname)
            if not ok: fail('failed in tofits')
            rec["fi"] = {}
            rec["fi"]["tool"] = ia.newimagefromfile(fitsname)
            if not rec["fi"]["tool"]: fail('failed to open '+fitsname)
            rec["fi"]["type"] = "FITSImage"

        # Virtual: SubImage (make it from another PagedImage so there
        # are no locking problems)
        if includereadonly:
            imname = root+'2'
            if not data:
                ok = ia.fromshape(imname, shape=imshape)
            else:
                ok = ia.fromarray(imname, pixels=data)
            if not ok: fail('failed to make virtual subimage')
            ok = ia.close()
            if not ok: fail('failed to close image while making virtual subimage')
            ok = ia.fromimage(infile=imname)
            if not ok: fail('failed in fromimage from '+imname)
            #since fromimage no longer returns an image tool use subimage
            rec["si"] = {}
            rec["si"]["tool"] = ia.subimage()
            if not rec["si"]["tool"]: fail('failed to get subimage')
            rec["si"]["type"] = "SubImage"  # equivalent to old subimage?

        # Virtual: TempImage
        rec["ti"] = {}
        if data==None:
            rec["ti"]["tool"] = ia.newimagefromshape(shape=imshape)
        else:
            rec["ti"]["tool"] = ia.newimagefromarray(pixels=data)
        if not rec["ti"]["tool"]: fail('failed in Virtual: TempImage')
        #rec["ti"]["tool"] = ia.subimage()
        #if not rec["ti"]["tool"]: fail('failed to get temporary image')
        rec["ti"]["type"] = "TempImage"

        # Virtual: ImageExpr (make it from another PagedImage so there
        # are no locking problems)
        if includereadonly:
            imname = root+'3'
            if not data: #(data==None):
                ok = ia.fromshape(imname, shape=imshape)
            else:
                ok = ia.fromarray(imname, pixels=data)
            if not ok: fail('failed in Virtual: ImageExpr')
            if not ia.close(): fail('failed in Virtual: ImageExpr closing file')
            expr = '"'+imname+'"'
            rec["ie"] = {}
            rec["ie"]["tool"] = ia.imagecalc(pixels=expr)
            if not rec["ie"]["tool"]: fail('failed in imagecalc')
            rec["ie"]["type"] = "ImageExpr"

        # Virtual: ImageConcat (make it from another PagedImage so there
        # are no locking problems)
        if includereadonly:
            imname1 = root +'4'
            if not data:
                ok = ia.fromshape(imname1, shape=imshape)
            else:
                ok = ia.fromarray(imname1, pixels=data)
            if not ok: fail('failed in Virtual: ImageConcat making PagedImage')
            if not ia.close(): fail('failed in Virtual: ImageConcat closing PagedImage')
            #
            imname2 = root + '5'
            if not data: #(data==None):
                ok = ia.fromshape(imname2, shape=imshape)
            else:
                ok = ia.fromarray(imname2, pixels=data)
            if not ok: fail('failed to make ' + imname2)
            if not  ia.close(): fail('failed to close ' + imname2)
            #
            files = [imname1, imname2]
            rec["ic"] = {}
            rec["ic"]["tool"] = ia.imageconcat(infiles=files, axis=0, relax=T, tempclose=F)
            if not rec["ic"]["tool"]: fail('failed in imageconcat')
            rec["ic"]["type"] = "ImageConcat"

        return rec

    # 3D only
    def make_data(imshape):
        data = ia.makearray(0, imshape)
        for i in range(imshape[0]):
            data[i] = list(data[i])
            for j in range(imshape[1]):
                data[i][j] = list(data[i][j])
                for k in range(imshape[2]):
                    data[i][j][k] = (k+1) + (j+1)*imshape[0] + (i+1)
                data[i][j] = tuple(data[i][j])
            data[i] = tuple(data[i])
        return data


    # 3D only
    def pick(imshape, data, inc):
        idxx = range(0,imshape[0],inc[0])  #range doesn't include maxval
        idxy = range(0,imshape[1],inc[1])
        idxz = range(0,imshape[2],inc[2])
        data2 = ia.makearray(0,[len(idxx),len(idxy),len(idxz)])
        kk = 0
        for k in idxz:
            jj = 0
            for j in idxy:
                ii = 0
                for i in idxx:
                    data2[ii,jj,kk] = data[i, j, k]
                    ii += 1
                jj += 1
            kk += 1
        return data2

    def fitsreflect(imagefile, fitsfile, do16=false):
        #
        # imagefile can be file name or image object
        #
        myim = imagefile;
        opened = F
        try:
            if (myim.type()=='image'):
                pass
        except Exception, e:
            myim = ia.newimagefromfile(imagefile);
            if not myim:
                stop('fitsreflect: image constructor failed');
            opened = T
        mi = myim.miscinfo()
        mi['hello'] = 'hello'
        mi['world'] = 'world';
        ok = myim.setmiscinfo(mi)
        if not ok:
            raise RuntimeError, 'setmiscinfo failed in fitsreflect'
        myim.sethistory(history=["A","B","C","D"])
        history = myim.history(F,F)
        #
        p1 = myim.getregion()
        m1 = myim.getregion(getmask=T)
        if len(p1)==0 or len(m1)==0:
            raise RuntimeError, 'getregion 1 failed in fitsreflect'
        imshape = myim.shape()
        m0=m1
        m1[0,0,0] = F
        m1[imshape[0]-1,imshape[1]-1,imshape[2]-1] = F
        ok = myim.putregion(pixelmask=m1)
        if not ok:
            raise RuntimeError, 'putregion 1 failed in fitsreflect'
        #
        bitpix = -32
        if do16: bitpix = 16
        ok = myim.tofits(outfile=fitsfile, bitpix=bitpix)
        if not ok:
            raise RuntimeError, 'tofits failed in fitsreflect'
        #
        testdir = os.path.dirname(fitsfile)
        imname2 = testdir +'/fitsreflect.image'
        myim2 = ia.newimagefromfits(outfile=imname2, infile=fitsfile)
        if not ok:
            raise RuntimeError, 'ia.newimagefromfits failed in fitsreflect'
        p1 = myim.getregion()
        m1 = myim.getregion(getmask=T)
        if len(p1)==0 or len(m1)==0:
            return throw('getregion 2 failed', origin='fitsreflect')
        p2 = myim2.getregion()
        m2 = myim2.getregion(getmask=T)
        if len(p2)==0 or len(m2)==0:
            stop('fitsreflect: getregion 3 failed')
        if not allbooleq(m2,m1):
            raise RuntimeError, 'Some mask values have changed in FITS reflection'
        #
        d = p2-p1
        sh = d.shape
        for i in range(sh[0]):
            for j in range(sh[1]):
                for k in range(sh[2]):
                    if m1[i][j][k]==true:
                        if not (abs(d[i][j][k]) < 0.0001):
                            raise RuntimeError, 'Some values have changed in FITS reflection'
        ok = myim.putregion(pixelmask=m0);
        if not ok:
            raise RuntimeError, 'putregion 2 failed in fitsreflect'
        #
        mi = myim2.miscinfo();
        if not (mi['hello'] == 'hello') or not (mi['world'] == 'world'):
            raise RuntimeError, 'miscinfo changed after fits in fitsreflect'
        #
        history2 = myim2.history(F,F);
        #Behaviour of history logged in FITS changed (2007/10/02)
        #Grab just the messages (indices 2, 4, 6, ...)
	#
	# funny things are happening here with move to casacore
	#
        history3=[]
        for i in range(len(history2)/2 - 1):
            history3.append(history2[2*(i+1)])
        if not all(history,history2):
            print "history written=", history
            print "history retrieved=", history2
            raise RuntimeError, 'history changed after fits in fitsreflect'
        #
        ok = myim2.done();
        if not ok:
            raise RuntimeError, 'done 1 failed in fitsreflect'
        if opened:
            ok = myim.done();
            if not ok:
                raise RuntimeError, 'done 2 failed in fitsreflect'
        #
        os.remove(fitsfile)
        ok = cleanup(imname2)
        if not ok:
            raise RuntimeError, 'image file deletion failed in fitsreflect'
        return true

    def coordcheck(im1, axes, testdir):
        #im1 = ia.newimage(im1name)
        ok = im1.summary(list=F)
        if not ok:
            fail('summary 1 failed in coordcheck')
        rec1 = ok
        imname = testdir + "/" + "coordcheck.image"
        cs2 = im1.coordsys(axes)
        if not cs2:
            fail('coordsys 1 failed in coordcheck')
        shape2=[]
	tmp = im1.shape()
	print tmp
        for i in axes:
            shape2.append(tmp[i])
	    print tmp[i]
	print 'shape2 is ', type(shape2), shape2
        ok = ia.fromshape(imname, shape2, cs2.torecord())
        if not ok:
            fail('ia.fromshape 1 failed in coordcheck');
        ia.close()  # close coordcheck.image
        im2 = ia.newimage(imname)
        ok = im2.summary(list=F)
        if not ok:
            fail('summary 2 failed in coordcheck')
        rec2 = ok
        #
        if rec1.has_key('axisnames') and rec2.has_key('axisnames'):
            rec1axes=[]
            for i in axes:
                rec1axes.append(rec1['axisnames'][i])
            if not all(rec1axes,rec2['axisnames']):
                note('axisnames mismatch')
                return false
        else:
            note('axisnames key missing')
            return false
        #
        if rec1.has_key('refpix') and rec2.has_key('refpix'):
            rec1axes=[]
            for i in axes:
                rec1axes.append(rec1['refpix'][i])
            if not all(rec1axes,rec2['refpix']):
                note('refpix mismatch')
                return false
        else:
            note('refpix key missing')
            return false
        #
        if rec1.has_key('refval') and rec2.has_key('refval'):
            rec1axes=[]
            for i in axes:
                rec1axes.append(rec1['refval'][i])
            if not all(rec1axes,rec2['refval']):
                note('refval mismatch')
                return false
        else:
            note('refval key missing')
            return false
        #
        if rec1.has_key('incr') and rec2.has_key('incr'):
            rec1axes=[]
            for i in axes:
                rec1axes.append(rec1['incr'][i])
            if not all(rec1axes,rec2['incr']):
                note('incr mismatch')
                return false
        else:
            note('incr key missing')
            return false
        #
        if rec1.has_key('axisunits') and rec2.has_key('axisunits'):
            rec1axes=axes
            for i in range(len(axes)):
                rec1axes[i]=rec1['axisunits'][i]
            if not all(rec1axes,rec2['axisunits']):
                note('axisunits mismatch')
                return false
        else:
            note('axisunits key missing')
            return false
        if not im2.remove(done=T):
            fail('failed image delete in coordcheck')
        if not cs2.done():
            fail('failed cs2 done in coordcheck')
        return true


    def zeroToPi(x):
        n = int(x / 180.0)
        rem = x - n*180.0
        #
        if (rem < 0): rem += 180.0
        return rem


    def test1():
        #
        # Test everything a bit
        #
        info('')
        info('')
        info('')
        info('Test 1 - general')
        #
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            note("Cleanup failed", "SEVERE")
            return false
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        #
        # Make some data
        #
        imshape = [32,32,8]
        info("Manufacturing data cube shaped " + str(imshape))
        data = make_data(imshape)

        info('z = 1:'+ str(imshape[2])+' + y * '+str(imshape[0])+' + x')
        manname = testdir+'/'+'manufactured.im'
        info('Turning array into image named ' + manname)
        man = ia.newimagefromarray(manname, data)
        if not man:
            stop('ia.fromarray constructor 1 fails')
        #
        info('Trying close')
        ok = man.close()
        if not ok:
            stop('Close failed')
        info('')
        #
        info('Trying open')
        ok = man.open(manname)
        if not ok:
            stop('Open failed')
        info('')
        #
        info('Trying rename')
        manname2 = testdir+'/'+'manufactured.im_rename'
        ok = man.rename(manname2)
        if not ok:
            stop('Rename of', manname, ' to ', manname2, 'fails')
        info('')
        #
        info('Trying remove')
        ok = man.remove(done=T)
        if not ok:
            stop('Remove of ', mannam2, 'fails')
        info('')
        #
        info('Remake the image')
        man = ia.newimagefromarray(manname, data)
        if not man:
            stop('ia.fromarray constructor fails')
        ok = man.done()
        if not ok:
            stop('Done 1 fails')
        #
        info('Trying image(...)')
        man = ia.newimagefromfile(manname)
        if not man:
            stop('image constructor 1 failed')
        info('')

        # verify the shape
        info('Verifying shape')
        if not all(man.shape(),imshape):
            stop('Shape of image is wrong:'+str(man.shape()))
        info('')

        # verify the name
        info('Verifying name');
        fullname = os.path.abspath(manname)
        if (man.name(strippath=F) != fullname):
            stop('The name is wrong: ', man.name())
        info('')

        # verify bounding box of default region
        info('Verifying default bounding box');
        bb = man.boundingbox(rg.box())
        actualStart = man.shape()
        actualEnd = man.shape()
        for i in range(len(actualStart)):
            actualStart[i] = 0
            actualEnd[i] = actualEnd[i]-1
        if not all(bb['blc'],actualStart) or not all(bb['trc'],actualEnd):
            msg = 'The default bounding box '+str(bb['blc'])+str(bb['trc'])+' is wrong'
            stop(msg)
        info('')

        # Summarise the image
        info('Summarize image');
        header = man.summary()
        info('')

        # Do statistics
        info('Find statistics')
        stuff = man.statistics()
        if not stuff: fail()
        info('')

        # Do histograms
        info('Find histograms')
        ok = man.histograms()
	print ok
        if not ok: fail()
        stuff = ok['counts']
        info('')

        # Find coordinates
        info('Verify coordinates')
        refPix = header['refpix']
        world = man.toworld(list(refPix), 'n')
        pixel = man.topixel(world)['numeric']
        if not alleq(refPix,pixel,tolerance=.0001):
            stop('Coordinate reflection failed')
        info('')

        # Fish out coordinates as measures
        info('Get coordinates as measures')
        rec = man.coordmeasures(list(pixel))
        info('')

        # Fish out CoordinateSYStem
        info('Get CoordinateSystem')
        csys = man.coordsys()
        if not csys:
            stop('Coordinate recovery failed')
        ok = csys.done()
        if not ok: fail()

        # Get/put region (mask is T at present)
        info('Verify get/putregion')
        trc = [imshape[0]-1,imshape[1]-1,0]
        r = rg.box([0,0,0], trc)
        pixels = man.getregion(region=r, list=F, dropdeg=F)
        mask = man.getregion(getmask=true, region=r, list=F, dropdeg=F)
        if len(pixels)==0 or len(mask)==0:
            stop('getregion 1 failed')
        pixels2 = ia.makearray(0,list(pixels.shape))  # don't set pixel2=pixels
        mask2 = ia.makearray(1,list(mask.shape))      # don't set mask2=mask
        for i in range(pixels2.shape[0]):
            for j in range(pixels2.shape[1]):
                for k in range(pixels2.shape[2]):
                    pixels2[i][j][k]=pixels[i][j][k]
                    mask2[i][j][k]=mask[i][j][k]
        pixels2[0,0,0] = -10
        pixels2[imshape[0]-1,imshape[1]-1,0] = -10
        ok = man.putregion(pixels=pixels2, pixelmask=mask2, region=r)
        if not ok:
            stop('putregion 1 failed')
        pixels3 = man.getregion(region=r, dropdeg=F)
        mask3 = man.getregion(getmask=true, region=r, dropdeg=F)
        if not ok:
            stop('getregion 2 failed')
        if not alleq(pixels3,pixels2,tolerance=.00001) or not allbooleq(mask3,mask2):
            stop ('get/putregion reflection failed')
        ok = man.putregion(pixels=pixels, pixelmask=mask, region=r)
        if not ok:
            stop('putregion 2 failed')
        info('')

        # Subimage
        info('Subimage')
        trc = [imshape[0]-1,imshape[1]-1,imshape[2]-1]
        trc[2] = 0
        r = rg.box([0,0,0], trc)
        pixels = man.getregion(region=r, dropdeg=F)
        mask = man.getregion(getmask=true, region=r, dropdeg=F)
        if len(pixels)==0 or len(mask)==0:
            stop('getregion 3 failed')
        if not man: fail()
        man3 = man.subimage(region=r)
        if not man3:
            stop('subimage 1 failed')
        pixels2 = man3.getregion(region=rg.box(), dropdeg=F)
        mask2 = man3.getregion(getmask=true, region=rg.box(), dropdeg=F)
        if len(pixels2)==0 or len(mask2)==0:
            stop('getregion 4 failed')
        if not alleq(pixels,pixels2,tolerance=.00001) or not allbooleq(mask,mask2):
            stop('SubImage got wrong results')
        ok = man3.done()
        if not ok:
            stop('Done 2 fails')

        # Do moments
        info('Find moments')
        ok = man.moments(axis=2)
        if not ok: fail()
        ok.done()
        info('')

        # Do Hanning
        info('Hanning smooth')
        man3 = man.hanning(axis=2, drop=F)
        if not man3:
            stop('Hanning fails')
        ok = man3.done()
        if not ok:
            stop('Done 3 fails')
        info('')

        # Do insert
        info('Insert')
        padshape = [imshape[0]+2,imshape[1]+2,imshape[2]+2]
        man3 = ia.newimagefromshape(shape=padshape)
        stats1 = man3.statistics()
        ok = man3.insert(infile=man.name(F), locate=[2,2,2]) 
        if not ok:
            stop('insert 1 fails')
        stats2 = man3.statistics()
        if stats1['sigma'][0] == stats2['sigma'][0]: fail()
        ok = man3.done()
        if not ok:
            stop('Done 4 fails')
        info('')

        # Read the data in various directions
        info('Starting read/compare tests....')
        info('XY plane by XY plane')
        t = time.time()
        for i in range(imshape[2]):
            data2 = man.getchunk(blc=[0,0,i],
                                 trc=[imshape[0]-1,imshape[1]-1,i],
                                 list=F)
            len_data2 = 1
            for idx in data2.shape: len_data2 *= idx
            if (len_data2 != imshape[0]*imshape[1]):
                stop('Not enough data read')
            for j in range(imshape[0]):
                for k in range(imshape[1]):
                    data2[j][k][0] = abs(data2[j][k][0]-data[j][k][i])

        rate = imshape[0]*imshape[1]*imshape[2]/(time.time() - t)/1.0e+6
        #rate::print.precision = 3
        info('            OK ('+ str(rate)+ 'M pix/s)')

        info('XZ plane by XZ plane')
        t = time.time()
        for i in range(imshape[1]):
            data2 = man.getchunk(blc=[0,i,0],
                                 trc=[imshape[0]-1,i,imshape[2]-1],
                                 list=F)
            len_data2 = 1
            for idx in data2.shape: len_data2 *= idx
            if (len_data2 != imshape[0]*imshape[2]):
                stop('Not enough data read')
            for j in range(imshape[0]):
                for k in range(imshape[2]):
                    data2[j][0][k] = abs(data2[j][0][k]-data[j][i][k])
            if not alleqnum(data2,0.0,tolerance=0.0001):
                dmax = datamax(data2)
                stop('Some values have changed, max deviation='+ str(max))
        rate = imshape[0]*imshape[1]*imshape[2]/(time.time() - t)/1.0e+6
        #rate::print.precision = 3
        info('            OK ('+ str(rate)+ 'M pix/s)')

        info('X row by X row')
        t = time.time()
        for j in range(imshape[2]):
            for i in range(imshape[1]):
                data2 = man.getchunk(blc=[0,i,j],
                                     trc=[imshape[0]-1,i,j],
                                     list=F)
                len_data2 = 1
                for idx in data2.shape: len_data2 *= idx
                if (len_data2 != imshape[0]):
                    stop('Not enough data read')
                for k in range(imshape[0]):
                    data2[k][0][0] = abs(data2[k][0][0] - data[k][i][j])
                if not alleqnum(data2,0.0,tolerance=0.0001):
                    dmax = datamax(data2)
                    stop('Some values have changed, max deviation='+str(dmax))
        rate = imshape[0]*imshape[1]*imshape[2]/(time.time() - t)/1.0e+6
        #rate::print.precision = 3
        info('            OK (' + str(rate) + 'M pix/s)')

        info('Z row by Z row')
        t = time.time()
        for j in range(imshape[1]):
            for i in range(imshape[0]):
                data2 = man.getchunk(blc=[i,j,0],
                                     trc=[i,j,imshape[2]],
                                     list=F)
                len_data2 = 1
                for idx in data2.shape: len_data2 *= idx
                if (len_data2 != imshape[2]):
                    stop('Not enough data read')

                for k in range(imshape[2]):
                    data2[0][0][k] = abs(data2[0][0][k] - data[i][j][k])
                if not alleqnum(data2,0.0,tolerance=0.0001):
                    dmax = datamax(data2)
                    stop('Some values have changed, max deviation='+str(dmax))
        rate = imshape[0]*imshape[1]*imshape[2]/(time.time() - t)/1.0e+6
        #rate::print.precision = 3
        info('            OK (' + str(rate) + 'M pix/s)')

        info('Check get/set miscinfo')
        mi = {'hello':('world'), 'foo':('bar'), 'answer':(42)}
        ok = man.setmiscinfo(mi)
        if not ok: stop('Error in setmiscinfo')
        mi = man.miscinfo()
        if (len(mi) != 3 or mi['hello'] != 'world' or mi['foo'] != 'bar' or
            mi['answer'] != 42):
            stop('Error in miscinfo ' + str(mi))

        info('Reflect file through fits and look for changes in data values and miscinfo')
        mannamefits = manname + '.fits'
        ok = fitsreflect (man, mannamefits)
        if not ok: fail()

        # close
        ok = man.done();
        if not ok:
            stop('Done 5 fails')
        #
        return cleanup(testdir)

        
    def test2():
        #
        # Test constructors
        #
        info('')
        info('')
        info('')
        info('Test 2 - ia.fromshape constructor')
        #
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            note("Cleanup failed", "SEVERE")
            return false
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        #
        imname = testdir+'/'+'ia.fromshape.image'
        try:
            myim = true
            note('Expect SEVERE error and Exception here')
            myim = ia.newimagefromshape(shape='fish')
        except Exception, e:
            note('Caught expected Exception:'+str(e))
            myim = false
        if myim:
            stop('ia.fromshape constructor 1 unexpectedly did not fail')

        try:
            myim = true
            note('Expect SEVERE error and Exception here')
            myim = ia.newimagefromshape(outfile=[10,20], shape=[10,20])
        except Exception, e:
            note('Caught expected Exception')
            myim = false
        if myim:
            stop('ia.fromshape constructor 2 unexpectedly did not fail')

        myim = ia.newimagefromshape(outfile=imname, shape=[10,20])
        if not myim:
            stop('ia.fromshape constructor 3 failed')
        pixels = myim.getregion()
        mask = myim.getregion(getmask=true)
        if not alleqnum(pixels,0.0,tolerance=0.00001):
            stop('ia.fromshape constructor 3 pixels are not all zero')
        if not alltrue(mask):
            stop('ia.fromshape constructor 3 mask is not all T')
        csys = myim.coordsys()
        if not csys:
            stop('coordsys 1 failed')
        ok = myim.remove(done=T)
        if not ok:
            stop('Delete 1 of', imname, ' failed')

        #
	try:
           note('Expect SEVERE error here')
           myim = ia.newimagefromshape(shape=[10,20], csys='xyz')
           if myim and myim.name()!="none":
               stop('ia.fromshape constructor 4 unexpectedly did not fail')
           else:
               note('Expected SEVERE error occurred')
	except Exception, e:
            note('Caught expected Exception')
            myim = false
        myim = ia.newimagefromshape(shape=[10,20], csys=csys.torecord())
        if not myim:
            stop('ia.fromshape constructor 5 failed')
        csysRec = csys.torecord()
        csys2 = myim.coordsys()
        if not csys2:
            stop('coordsys 2 failed')
        csys2Rec = csys2.torecord()
        if not all(csysRec.keys(),csys2Rec.keys()):
            stop('coordinates from ia.fromshape 5 are wrong')
        ok = myim.done()
        if not ok: fail()
        if not csys.done(): fail()
        if not csys2.done(): fail()
        #
        # Try a few different shapes to test out the standard coordinate
        # system making of CoordinateUtil
        #
        myim = ia.newimagefromshape(shape=[10])
        if not myim:
            stop('ia.fromshape constructor 6 failed')
        ok = myim.done()
        if not ok: fail()
        myim = ia.newimagefromshape(shape=[10,20])
        if not myim:
            stop('ia.fromshape constructor 7 failed')
        ok = myim.done()
        if not ok: fail()
        myim = ia.newimagefromshape(shape=[10,20,4])
        if not myim:
            stop('ia.fromshape constructor 8 failed')
        ok = myim.done()
        if not ok: fail()
        myim = ia.newimagefromshape(shape=[10,20,4,16])
        if not myim:
            stop('ia.fromshape constructor 9 failed')
        ok = myim.done()
        if not ok: fail()
        myim = ia.newimagefromshape(shape=[10,20,16,4])
        if not myim:
            stop('ia.fromshape constructor 10 failed')
        ok = myim.done()
        if not ok: fail()
        myim = ia.newimagefromshape(shape=[10,20,16])
        if not myim:
            stop('ia.fromshape constructor 11 failed')
        ok = myim.done()
        if not ok: fail()
        myim = ia.newimagefromshape(shape=[10,20,16,4,5,6])
        if not myim:
            stop('ia.fromshape constructor 12 failed')
        ok = myim.done()
        if not ok: fail()

        ###
        return cleanup(testdir)
        
    def test3():
        info('')
        info('')
        info('')
        info('Test 3 - ia.fromarray constructor')

        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            note("Cleanup failed", "SEVERE")
            return false
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        #
        # Make some data
        #
        imshape = [10,20,30]
        data = make_data(imshape)
        #
        imname = testdir+'/'+'ia.fromarray.image'
        try:
            myim = true
            note('Expect SEVERE error and Exception here')
            myim = ia.newimagefromarray(outfile=[10,20], pixels=data)
        except Exception, e:
            note('Caught expected Exception')
            myim = false
        if myim:
            stop('ia.fromarray constructor 1 unexpectedly did not fail')
        myim = ia.newimagefromarray(outfile=imname, pixels=data)
        if not myim:
            stop('ia.fromarray constructor 2 failed')
        pixels = myim.getregion()
        mask = myim.getregion(getmask=true)
        #data2 = pixels - data
        if not alleq(pixels,data,tolerance=0.0001):
            stop('ia.fromarray 2 pixels have the wrong value')
        if not alltrue(mask):
            stop('ia.fromarray 2 mask is not all T')
        csys = myim.coordsys()
        if not csys:
            stop('coordinates 1 failed')
        ok = myim.remove(done=T)
        if not ok:
            stop('Delete 1 of '+imname+' failed')
        #
        note('Expect SEVERE error here')
	try :
           myim = ia.newimagefromarray(outfile=imname, pixels=data, csys='xyz')
           if myim.isopen():
               stop('ia.fromarray constructor 3 unexpectedly did not fail')
	except Exception, e :
            note('Caught expected Exception')

        myim = ia.newimagefromarray(pixels=data, csys=csys.torecord())
        if not myim:
            stop('ia.fromarray constructor 4 failed')
        csysRec = csys.torecord()
        csys2 = myim.coordsys()
        if not csys2:
            stop('coordinates 2 failed')
        csys2Rec = csys2.torecord()
        if not all(csysRec.keys(),csys2Rec.keys()):
            stop('coordinates from ia.fromarray 4 are wrong')
        ok = myim.done()
        if not ok: fail()
        if not csys.done(): fail()
        if not csys2.done(): fail()
        ###
        return cleanup(testdir)

        
    def test4():
        info('')
        info('')
        info('')
        info('Test 4 - image constructor')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            note("Cleanup failed", "SEVERE")
            return false
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        # Test native aips++ image
        imname = testdir+'/'+'image.image'
        note('Expect SEVERE error here')
        myim = ia.newimage(infile='_doggies')
        if myim.isopen():
            stop('image constructor 1 unexpectedly did not fail')
        myim = ia.newimagefromshape(outfile=imname, shape=[10,20])
        if not myim:
            stop('ia.fromshape constructor 1 failed')
        myim.done()
        myim = ia.newimage(infile=imname)
        if not myim:
            stop('image constructor 2 failed')
        pixels = myim.getregion()
        mask = myim.getregion(getmask=true)
        if not alleqnum(pixels,0.0,tolerance=0.00001):
            stop('pixels (1) are not all zero')
        if not alltrue(mask):
            stop('mask (1) is not all T')
        #
        fitsname = testdir+'/'+'image.fits'
        ok = myim.tofits(fitsname)
        if not ok: fail()
        #
        ok = myim.remove(done=T)
        if not ok:
            stop('Failed to remove ', imname)

        # Test FITS image
        myim = ia.newimage(fitsname)
        if not myim:
            stop('image constructor 3 failed')
        #
        pixels = myim.getregion()
        mask = myim.getregion(getmask=true)
        if len(pixels)==0 or len(mask)==0: fail()
        if not alleqnum(pixels,0.0,tolerance=0.00001):
            stop('pixels (2) are not all zero')
        if not alltrue(mask):
            stop('mask (2) is not all T')
        if not myim.done(): fail()

        ###
        return cleanup(testdir)

        
    def test5():
        info('')
        info('')
        info('')
        info('Test 5 - ia.fromimage constructor')

        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            note("Cleanup failed", "SEVERE")
            return false
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        #
        # Test ia.fromimage constructor
        #
        info('')
        imname = testdir+'/'+'ia.fromshape.image'
        imname2 = testdir+'/'+'ia.fromimage.image'
        #
        #myim = ia.fromimage(outfile=imname2, infile='_doggies')
        try:
            myim = true
            note('Expect SEVERE error and Exception here')
            myim = ia.newimagefromimage(outfile=imname2, infile='_doggies')
        except Exception, e:
            note('Caught expected Exception')
            myim = false
        if myim:
            stop('ia.fromimage constructor 1 unexpectedly did not fail')
        #myim = ia.fromimage(outfile=imname2, infile='_doggies',
        #                    region='doggies')
        try:
            myim = true
            note('Expect SEVERE error and Exception here')
            myim = ia.newimagefromimage(outfile=imname2, infile='_doggies',region='doggies')
        except Exception, e:
            note('Caught expected Exception')
            myim = false
        if myim:
            stop('ia.fromimage constructor 2 unexpectedly did not fail')
        myim = ia.newimagefromshape(outfile=imname, shape=[20,40])
        if not myim:
            stop('ia.fromshape constructor 1 failed')
        #region1 = rg.quarter()
        region1 = rg.box(blc=[.25,.25],trc=[.75,.75],frac=true)
        pixels = myim.getregion(region=region1)
        mask = myim.getregion(getmask=true, region=region1)
        if len(pixels)==0 or len(mask)==0:
            stop('getregion 1 failed')
        ok = myim.done()
        if not ok:
            stop('done 1 failed')
        #
        #myim = ia.fromimage(outfile=imname2, infile=imname)
        myim = ia.newimagefromimage(outfile=imname2, infile=imname)
        if not myim:
            stop('ia.fromimage constructor 3 failed')
        if not myim.remove(done=T): fail()
        #myim = ia.fromimage(infile=imname, region=region1)
        myim = ia.newimagefromimage(infile=imname, region=region1)
        if not myim:
            stop('ia.fromimage constructor 4 failed')
        bb = myim.boundingbox()
        shape = myim.shape()
        shape2 = bb['trc'] - bb['blc'] + 1
        if not all(shape,shape2):
            stop ('Output image has wrong shape');
        pixels2 = myim.getregion()
        mask2 = myim.getregion(getmask=true)
        if not alleq(pixels,pixels2,tolerance=0.00001):
            stop('The data values are wrong in the ia.fromimage')
        if not allbooleq(mask,mask2):
            stop('The mask values are wrong in the ia.fromimage')
        if not myim.done(): fail()
        ###
        return cleanup(testdir)
        
    def test6():
        info('')
        info('')
        info('')
        info('Test 6 - ia.fromfits constructor')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            note("Cleanup failed", "SEVERE")
            return false
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        #
        info('Testing ia.fromfits constructor')
        imname = testdir+'/'+'ia.fromshape.image'
        imname2 = testdir+'/'+'image.fits'
        imname3 = testdir+'/'+'ia.fromfimage'
        #
        # ia.fromfits
        #
        try:
            myim = true
            note('Expect SEVERE error and Exception here')
            myim = ia.newimagefromfits(outfile=imname3, infile='_doggies')
        except Exception, e:
            note('Caught expected Exception')
            myim = false
        if myim:
            stop('ia.fromfits constructor 1 unexpectedly did not fail')
        myim = ia.newimagefromshape(outfile=imname, shape=[10,20])
        if not myim:
            stop('ia.fromshape constructor 1 failed')
        ok = myim.tofits(outfile=imname2)
        if not ok:
            stop('tofits failed')
        ok = myim.done()
        if not ok: fail()

        try:
            myim = true
            note('Expect SEVERE error and Exception here')
            myim = ia.newimagefromfits(outfile=imname3, infile=imname2,
                                       whichhdu=22)
        except Exception, e:
            note('Caught expected Exception')
            myim = false
        if myim:
            stop('ia.fromfits constructor 2 unexpectedly did not fail')
        myim = ia.newimagefromfits(outfile=imname3, infile=imname2, whichhdu=0)
        if not myim:
            stop('ia.fromfits constructor 3 failed')
        pixels = myim.getregion()
        mask = myim.getregion(getmask=true)
        if len(pixels)==0 or len(mask)==0: fail()
        if not alleqnum(pixels,0.0,tolerance=0.00001):
            stop('ia.fromfits constructor 3 pixels are not all zero')
        if not alltrue(mask):
            stop('ia.fromfits constructor 3 mask is not all T')
        if not myim.remove(done=T): fail('*****FAILED TO REMOVE IMAGE*****')
        #
        myim = ia.newimagefromfits(infile=imname2, whichhdu=0)
        if not myim:
            stop('ia.fromfits constructor 4 failed')
        pixels = myim.getregion()
        mask = myim.getregion(getmask=true)
        if len(pixels)==0 or len(mask)==0: fail()
        if not alleqnum(pixels,0.0,tolerance=0.00001):
            stop('ia.fromfits constructor 3 pixels are not all zero')
        if not alltrue(mask):
            stop('ia.fromfits constructor 3 mask is not all T')
        if not myim.done(): fail()

        ###
        return cleanup(testdir)
        
    def test7():
        #
        # Test  constructors
        #
        info('')
        info('')
        info('')
        info('Test 7 - imageconcat constructor')

        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            note("Cleanup failed", "SEVERE")
            return false
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        #
        imname = testdir+'/'+'ia.fromshapeconcat.image'
        imname2 = testdir+'/'+'imageconcat.image'
        #
        try:
            myim = true
            note('Expect SEVERE error and Exception here')
            myim = ia.imageconcat(outfile=imname2, infiles='_doggies')
        except Exception, e:
            note('Caught expected Exception')
            myim = false
        if myim:
            stop('imageconcat constructor 1 unexpectedly did not fail')
        myim = ia.newimagefromshape(outfile=imname, shape=[10,20])
        if not myim:
            stop('ia.fromshape constructor 1 failed')
        shape = myim.shape()
        myim.done()
        #
        infiles = [imname,imname,imname]
        shapex = [3*shape[0], shape[1]]
        shapey = [shape[0], 3*shape[1]]
        try:
            myim = true
            note('Expect SEVERE error and Exception here')
            myim = ia.imageconcat(outfile=imname2, infiles=infiles,
                                  axis=29, relax=T)
        except Exception, e:
            note('Caught expected Exception')
            myim = false
        if myim:
            stop('imageconcat constructor 2 unexpectedly did not fail')
        try:
            myim = true
            note('Expect SEVERE error and Exception here')
            myim = ia.imageconcat(outfile=imname2, infiles=infiles, axis=1,
                               relax=F)
        except Exception, e:
            note('Caught expected Exception:'+str(e))
            myim = false
        if myim:
            stop('imageconcat constructor 3 unexpectedly did not fail')
        myim = ia.imageconcat(outfile=imname2,infiles=infiles,axis=1,relax=T)
        if not myim:
            stop('imageconcat constructor 4 failed')
        shape = myim.shape()
        if not all(shape,shapey):
            stop('imageconcat image has wrong shape')
        ia.close() # needed to we can done myim
        ok = myim.remove(done=T)
        if not ok: fail()
        myim = ia.imageconcat(infiles=infiles, axis=0, relax=T)
        if not myim:
            stop('imageconcat constructor 5 failed')
        shape = myim.shape()
        if not all(shape,shapex):
            stop('imageconcat image has wrong shape')
        if not myim.done(): fail()

         ###
        return cleanup(testdir)

    def test10():
        # Test methods
        #   is_image, imagetools, imagedones
        #   open, close, done, type, id
        info('')
        info('')
        info('')
        #info('Test 10 - is_image, imagetools, imagedones, done, close, open')
        #info('Test 10 - isopen, type, id, lock, unlock, haslock')
        info('Test 10 - done, close, open')
        info('Test 10 - isopen, type, lock, unlock, haslock')

        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            note("Cleanup failed", "SEVERE")
            return false
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        # Make two images
        imname1 = testdir+'/'+'ia.fromshape.image'
        shape1 = [10,20]
        myim1_test10_ = ia.newimagefromshape(outfile=imname1, shape=shape1)
        if not myim1_test10_:
            stop('ia.fromshape constructor 1 failed')
        imname2 = testdir+'/'+'ia.fromshape.image2'
        shape2 = [10,20]
        myim2_test10_ = ia.newimagefromshape(outfile=imname2, shape=shape2)
        if not myim2_test10_:
            stop('ia.fromshape constructor 2 failed')

        #
        #info('')
        #info('Testing is_image')
        #if not is_image(myim1_test10_) or not is_image(myim2_test10_):
        #    stop('Global function is_image failed')

        #
        #info('')
        #info('Testing imagetools')
        #itools = ia.imagetools()
        #
        # There might be other global image tools out there
        #
        #nimages = len(itools)
        #if nimages>1:
        #    ok = T
        #    found1 = F
        #    found2 = F
        #    for i in range(nimages):
        #        if (itools[i]=='myim1_test10_'):
        #            if (found1): ok = F
        #            else: found1 = T
        #        else:
        #            if (itools[i]=='myim2_test10_'):
        #                if (found2): ok = F
        #                else: found2 = T
        #    if (!ok or !found1 or !found2):
        #        stop('Global function imagetools failed')
        #else:
        #    stop('Global function imagetools failed')
        #
        #
        #info('')
        #info('Testing done/imagedones')
        #ok = imagedones("myim1_test10_ myim2_test10_")
        #if not ok:
        #    stop('imagedones failed')
        #if (myim1_test10_!=F or myim2_test10_!=F):
        #    stop('imagedones did not completely destroy image tools')
        myim1_test10_.done()
        myim2_test10_.done()

        # Test shape/close/open
        info('')
        info('Testing close')
        myim = ia.newimage(imname1)
        shape = myim.shape()
        if not all(shape,shape1):
            stop('image has wrong shape')
        ok = myim.close()
        if not ok:
            stop('Close fails')
        if myim.isopen():
            stop('isopen 1 fails')
        note('Expect SEVERE error here')
        ok = myim.shape()
        if ok:
            stop('Closed image is unexpectedly viable')
        else:
            note('Expected SEVERE error occurred')

        #
        info('')
        info('Testing open')
        ok = myim.open(imname2)
        if not ok:
            stop('Open fails')
        if not myim.isopen():
            stop('isopen 2 fails')
        shape = myim.shape()
        if not all(shape,shape2):
            stop('image has wrong shape')

        #
        info('')
        info('Testing type')
        if (myim.type()!='image'):
            stop('image has wrong type')

        #
        #info('')
        #info('Testing id')
        #id = myim.id()
        #if not id:
        #    stop('id failed')
        #ok = is_record(id) and id.has_key('sequence')
        #ok = ok and id.has_key('pid')
        #ok = ok and id.has_key('time')
        #ok = ok and id.has_key('host')
        #ok = ok and id.has_key('agentid')
        #if not ok:
        #    stop('id record has wrong fields')

        #
        ok = myim.done()
        if not ok:
            stop('Done 2 failed')

        #
        myim = ia.newimage(imname1)
        ok = myim.open(imname2)
        if not ok:
            stop('Open on already open image failed')

        #
        # We cant test locking properly without two processes
        # trying to access the same image.  So all we can do is
        # see that the functions run and test that haslock
        # gives the right answers
        #
        info('')
        info('Testing locking')
        ok = myim.lock(T, nattempts=0)
        if not ok:
            stop('Lock failed (1)')
        ok = myim.haslock()
        if not ok[0]:
            stop('haslock failed (1)')
        if (ok[0]!=T or ok[1]!=T):
            stop('haslock returns wrong values (1)')
        #
        ok = myim.unlock()
        if not ok:
            stop('Unlock failed (1)')
	ok = myim.haslock()
        if not len(ok):
            stop('haslock failed (2)')
        if (ok[0]!=F or ok[1]!=F):
            stop('haslock returns wrong values (2)')
        #
        ok = myim.lock(F, nattempts=0)
        if not ok:
            stop('Lock failed (2)')
        ok = myim.haslock()
        if not ok[0]:
            stop('haslock failed (3)')
        if (ok[0]!=T or ok[1]!=F):
            stop('haslock returns wrong values (3)')
        #
        ok = myim.done()
        if not ok:
            stop('Done 3 failed')
        ###
        return cleanup(testdir)


    def test11():
        # Test methods
        #   coordsys, setcoordsys, shape, name, rename, delete, persistent
        info('-------')
        info('Test 11 - coordsys, setcoordsys,')
        info('          shape, name, rename,  ')
        info('          delete, persistent    ')
        info('-------')
        
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            note("Cleanup failed", "SEVERE")
            return false
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        # Make two images
        imname1 = testdir+'/'+'ia.fromshape.image1'
        shape1 = [10,20]
        if not ia.fromshape(outfile=imname1, shape=shape1):
            stop('ia.fromshape constructor 1 failed')
        #
        imname2 = testdir+'/'+'ia.fromshape.image2'
        shape2 = [5,10]
        myim = ia.newimagefromshape(outfile=imname2, shape=shape2)
        if not myim:
            stop('ia.newimagefromshape constructor 2 failed')
        if not myim.done():
            stop('Done 2 fails')

        # coordsys
        info('Testing coordsys')
        myim = ia.newimage(imname1)
        try:
            mycs = myim.coordsys()
        except:
            stop('coordsys 1 failed')
        #if not is_coordsys(mycs):
        #    stop('coordinates are not valid (1)')
        ok = coordcheck(myim, [0,1], testdir)
        if not ok:
            stop('coordinates subset selection 0,1 failed')
        ok = coordcheck(myim, [0], testdir)
        if not ok:
            stop('coordinates subset selection 0 failed')
        ok = coordcheck(myim, [1], testdir)
        if not ok:
            stop('coordinates subset selection 1 failed')
        if not mycs.done():
            stop('mycs.done fails')

        # setcoordsys
        info('')
        info('Testing setcoordsys')
        try:
            mycs = myim.coordsys()
        except:
            stop('coordsys 2 failed')
        #
        try:
            incr = mycs.increment(format='n')
            incr = list(incr['numeric'])
            for i in range(len(incr)):
                incr[i] *= 2.0
        except:
            fail('failed in mycs.increment')
        try:
            refval = mycs.referencevalue(format='n')
            refval = list(refval['numeric'])
            for i in range(len(refval)):
                refval[i] += 0.01
        except:
            fail('failed in mycs.referencevalue')
        try:
            refpix = list(mycs.referencepixel()['numeric'])
            for i in range(len(refpix)):
                refpix[i] += 10.0
        except:
            fail('failed in mycs.referencepixel')
        ok = mycs.setincrement (value=incr)
        if not ok:
            fail('mycs.setincrement failed')
        ok = mycs.setreferencevalue(value=refval)
        if not ok:
            fail('mycs.setreferencevalue failed')
        ok = mycs.setreferencepixel(value=refpix);
        if not ok:
            fail('mycs.setreferencepixel failed')
        #
        ok = myim.setcoordsys (mycs.torecord())
        if not ok:
            stop('setcoordsys 1 failed')
        if not mycs.done():
            fail('mycs.done failed')
        cs2 = myim.coordsys()
        incr2 = list(cs2.increment(format='n')['numeric'])
        if not all(incr,incr2):
            stop('coordsys/setcoordsys  reflection fails increment test')
        refval2 = list(cs2.referencevalue(format='n')['numeric'])
        if not all(refval, refval2):
            stop('coordsys/setcoordsys  reflection fails ref val test')
        if not all(refpix,list(cs2.referencepixel()['numeric'])):
            stop('coordsys/setcoordsys  reflection fails ref pix test')
        if not cs2.done():
            fail('failed cs2.done')
        #
        try:
            mycs2 = cs.newcoordsys(direction=F, spectral=F, stokes="", linear=0)
        except:
            fail('failed creating mycs2')
        try:
            note("Expect a SEVERE error and exception here.")
            ok = myim.setcoordsys(mycs2.torecord())
        except Exception, e:
            note("Exception: "+ str(e),"SEVERE")
            note("Exception occurred as expected.")
            ok = false
        if ok:
            stop('setcoordsys 3 unexpectedly did not fail')
        if not mycs2.done():
            fail('failed mycs2.done')
        badcs = 'doggies'
        try:
            note("Expect a SEVERE error and exception here.")
            ok = myim.setcoordsys(badcs)
        except Exception, e:
            note("Exception occurred as expected.")
        if ok:
            stop('setcoordsys 4 unexpectedly did not fail')

        # shape
        info('')
        info('Testing shape')
        if not all(myim.shape(),shape1):
            stop('Shape fails')

        # Name
        info('')
        info('Testing name')
        absoluteName = os.path.abspath(imname1)
        if not absoluteName:
            fail('failed to get absoluteName')
        if (myim.name(strippath=F) != absoluteName):
            stop('The absolute name is wrong')
        baseName = 'ia.fromshape.image1'
        if (myim.name(strippath=T) != baseName):
            stop('The base name is wrong')

        # Rename
        info('')
        info('Testing rename')
        imname4 = testdir + '/' + 'ia.fromshape.image4'
        ok = myim.rename(imname4, overwrite=F)
        if not ok:
            stop('Rename 1 fails')
        absoluteName = os.path.abspath(imname4)
        if not absoluteName:
            fail('failed to get absoluteName for rename')
        if (myim.name(strippath=F) != absoluteName):
            stop('The name has not been renamed correctly')
        #
        ok = myim.rename(imname2, overwrite=F)
        if ok:
            stop('Rename unexpectedly did not fail')
        ok = myim.rename(imname2, overwrite=T)
        if not ok:
            stop('Rename 2 fails')

        # Remove/Delete
        info('')
        info('Testing remove')
        ok = myim.remove(done=T)
        if not ok:
            stop('Done 3 fails')
        if (myim!=F):
            note('Done did not completely destroy image tool')
        #
        #myim = ia.fromshape(imname1, shape1)
        ok = ia.fromshape(imname1, shape1)
        if not ok:
            stop('failed to create image from shape in remove')
        ia.close()  # remove lock on imname1
        myim = ia.newimage(imname1)
        if not myim:
            stop('ia.fromshape constructor 3 fails')
        ok = myim.remove(done=F)
        if not ok:
            stop('Remove fails')
        #if (is_boolean(myim) && myim==F) {
        if not myim:
            stop('Remove erroneously destroyed the image tool')
        ok = myim.done()
        if not ok:
            stop('Done 4 fails')

        # persistent
        info('')
        info('Testing ispersistent')
        imname4 = testdir + '/' + 'ia.fromshape.image4'
        shape4 = [10,10]
        #myim4 = ia.fromshape(outfile=imname4, shape=shape4)
        ok = ia.fromshape(outfile=imname4, shape=shape4)
        if not ok:
            stop('failed to create image from shape4 in persistent')
        ia.close()  # unlock imname4
        myim4 = ia.newimage(imname4)
        if not myim4:
            stop('ia.fromshape constructor 4 failed')
        #
        if not myim4.ispersistent():
            stop('Persistent test 1 fails')
        ok = myim4.done()
        if not ok:
            stop('Done 5 fails')
        ex = '"' + imname4 + '"' + '+' + '"' + imname4 + '"'
        myim4 = ia.imagecalc(pixels=ex)
        if not myim4:
            stop('imagecalc constructor fails')
        if myim4.ispersistent():
            stop('Persistent test 2 fails')
        ok = myim4.done()
        if not ok:
            stop('Done 6 fails')

        #
        return cleanup(testdir)

    def test12():
        # Test methods
        #   getchunk, putchunk
        #   pixelvalue
        info('')
        info('')
        info('')
        info('Test 12 - getchunk, putchunk, pixelvalue')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            note("Cleanup failed", "SEVERE")
            return false
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        #
        # Make images of all the wondrous flavours that we have
        #
        root = testdir + '/' + 'testimage'
        imshape = [12,24,20]
        data = make_data(imshape)
        images = makeAllImageTypes(imshape, root, data, F)
        if not images:
            fail('failed to make all image types')
        #
        # Loop over all image types
        #
        types = images.keys()
        for mytype in types:
            info('')
            info('Testing Image type ' + images[mytype]["type"])
            myim = images[mytype]["tool"]
            #
            # Get some chunks !
            #
            info('')
            info('Testing getchunk')
            pixels = myim.getchunk(list=T)
            if len(pixels)==0:
                stop('getchunk 1 failed')
            if list(pixels.shape) != imshape:
                stop('getchunk 1 recovers wrong array shape')
            #if (!all((pixels-data < 0.0001)))
            if not alleq(pixels, data, tolerance=0.0001):
                stop('getchunk 1 recovers wrong pixel values')
            #
            inc = [1,2,5]
            pixels = myim.getchunk(inc=inc)
            if len(pixels)==0:
                stop('getchunk 2 failed')
            timshape=[]
            timshape.append(imshape[0]/1)
            timshape.append(imshape[1]/2)
            timshape.append(imshape[2]/5)
            if list(pixels.shape) != timshape:
                stop('getchunk 2 recovers wrong array shape')
            data2 = pick(imshape, data, inc)
            if not alleq(pixels,data2,tolerance= 0.0001):
                return stop('getchunk 2 recovers wrong pixel values')
            #
            blc = [2,3,4]
            trc = [5,6,7]
            shape2 = [trc[0]-blc[0]+1, trc[1]-blc[1]+1, trc[2]-blc[2]+1]
            pixels = myim.getchunk(blc=blc, trc=trc, inc=[1,1,1])
            if len(pixels)==0:
                stop('getchunk 3 failed')
            if list(pixels.shape) != shape2:
                stop('getchunk 3 recovers wrong array shape')
            data2 = data[blc[0]:(trc[0]+1), blc[1]:(trc[1]+1), blc[2]:(trc[2]+1)]
            if not alleq(pixels,data2,tolerance=0.0001):
                stop('getchunk 3 recovers wrong pixel values')
            #
            blc = [-10,-10,-10];
            pixels = myim.getchunk(blc=blc)
            if len(pixels)==0:
                stop('getchunk 4 failed')
            if list(pixels.shape) != imshape:
                stop('getchunk 4 recovers wrong array shape')
            #
            trc = [10000,10000,10000]
            pixels = myim.getchunk(trc=trc)
            if len(pixels)==0:
                stop('getchunk 5 failed')
            if list(pixels.shape) != imshape:
                stop('getchunk 5 recovers wrong array shape')
            #
            blc = [5,6,7]
            trc = [1,2,3]
            pixels = myim.getchunk(blc=blc,trc=trc)
            if len(pixels)==0:
                stop('getchunk 6 failed')
            if list(pixels.shape) != imshape:
                stop('getchunk 6 recovers wrong array shape')
            #
            inc = [100,100,100]
            pixels = myim.getchunk(inc=100)
            if len(pixels)==0:
                stop('getchunk 7 failed')
            if list(pixels.shape) != imshape:
                stop('getchunk 7 recovers wrong array shape')
            #
            pixels = myim.getchunk (axes=[0,1], dropdeg=T)
            if len(pixels) != imshape[2]:
                stop('getchunk 8 recovers wrong array shape')
            #
            pixels = myim.getchunk (axes=[1,2], dropdeg=T)
            if len(pixels) != imshape[0]:
                stop('getchunk 9 recovers wrong array shape')
            #
            pixels = myim.getchunk (axes=[0], dropdeg=T)
            if not all(list(pixels.shape),imshape[1:3]):
                stop('getchunk 10 recovers wrong array shape')
            #
            # Now some putchunking
            #
            info('')
            info('Testing putchunk')
            pixels = myim.getchunk()
            if len(pixels)==0:
                stop('getchunk 8 failed')
            data2=ia.makearray(100,[data.shape[0],data.shape[1],data.shape[2]])
            for i in range(data.shape[0]):
                for j in range(data.shape[1]):
                    for k in range(data.shape[2]):
                        if data[i][j][k]<-10000:
                            data2[i][j][k] = data[i][j][k]
            ok = myim.putchunk(pixels=data2, list=T)
            if not ok:
                stop('putchunk 1 failed')
            pixels = myim.getchunk()
            if len(pixels)==0:
                stop('getchunk 9 failed')
            if not alleq(pixels,data2, tolerance=0.0001):
                stop('getchunk 10 recovers wrong pixel values=')
            #
            inc = [2,1,5]
            data2 = ia.makearray(0,[imshape[0]/inc[0], imshape[1]/inc[1], imshape[2]/inc[2]])
            ok = myim.putchunk(pixels=data2, inc=inc)
            if not ok:
                stop('putchunk 2 failed')
            pixels = myim.getchunk(inc=inc)
            if len(pixels)==0:
                stop('getchunk 9 failed')
            if not alleq(pixels,data2, tolerance=0.0001):
                stop('getchunk 10 recovers wrong pixel values=')
            pixels2 = myim.getchunk()
            if len(pixels2)==0:
                stop('getchunk 11 failed')
            data2 = pick(imshape, pixels2, inc)
            if not alleq(data2,pixels, tolerance=0.0001):
                stop('getchunk 11 recovers wrong pixel values=')
            pixels = F; pixels2 = F; data2 = F
            #
            pixels = myim.getchunk()
            if len(pixels)==0:
                stop('getchunk 12 failed')
            try:
                note('Expect SEVERE error and Exception here')
                ok = myim.putchunk(pixels=pixels, blc=[3,4,5])
            except Exception, e:
                note('Expected exception occurred.')
                ok = False
            if ok:
                stop('putchunk 3 unexpectedly did not fail')
            try:
                note('Expect SEVERE error and Exception here')
                ok = myim.putchunk(pixels=pixels, inc=[3,4,5])
            except Exception, e:
                note('Expected exception occurred.')
                ok = False
            if ok:
                stop('putchunk 4 unexpectedly did not fail')
            #
            pixels_shape=pixels.shape
            for i in range(pixels_shape[0]):
                for j in range(pixels_shape[1]):
                    for k in range(pixels_shape[2]):
                        if pixels[i][j][k] >= -10000:
                            pixels[i]=100
            ok = myim.putchunk(pixels=pixels)
            if not ok:
                stop('putchunk 5 failed')
            pixels = ia.makearray(0,[imshape[0], imshape[1]])
            ok = myim.putchunk(pixels=pixels)
            if not ok:
                stop('putchunk 6 failed')
            pixels2 = myim.getchunk()
            pixels2shape=pixels2.shape
            for i in range(pixels2shape[0]):
                for j in range(pixels2shape[1]):
                    if not pixels2[i,j,0]==0:
                        stop('getchunk 13 recovered wrong pixel values')
                    if not pixels2[i,j,1]==100:
                        stop('getchunk 14 recovered wrong pixel values')
            #
            # Test replication in putchunk
            #
            ok = myim.set(pixels=0.0)
            if not ok:
                stop('set 1 fails')
            #
            p = ia.makearray(10.0, [imshape[0],imshape[1]]); # Adds degenerate axis
            ok = myim.putchunk(p, replicate=T)
            if not ok:
                stop('putchunk 8 fails')
            p2 = myim.getchunk()
            if len(p2)==0:
                stop('getchunk 16 fails')
            p2shape=p2.shape
            for i in range(p2shape[0]):
                for j in range(p2shape[1]):
                    for k in range(p2shape[2]):
                        if not (p2[i,j,k]==10.0):
                            stop('putchunk 8 put wrong values')
            #
            p = ia.makearray(10.0, [imshape[0],imshape[1], 1])
            ok = myim.putchunk(p, replicate=T)
            if not ok:
                stop('putchunk 9 fails')
            p2 = myim.getchunk()
            if len(p2)==0:
                stop('getchunk 17 fails')
            p2shape=p2.shape
            for i in range(p2shape[0]):
                for j in range(p2shape[1]):
                    for k in range(p2shape[2]):
                        if not (p2[i,j,k]==10):
                            stop('putchunk 9 put wrong values')
            #
            # Now pixelvalue
            #
            info('')
            info('Testing pixelvalue')
            #pixels2[:,:,:] = 0.0
            pixels2shape=pixels2.shape
            for i in range(pixels2shape[0]):
                for j in range(pixels2shape[1]):
                    for k in range(pixels2shape[2]):
                        pixels2[i,j,k] = 0.0
            pixels2[0,0,0] = 1.0
            ok = myim.putchunk(pixels=pixels2)
            if not ok:
                stop('putchunk 7 failed')
            ok = myim.setbrightnessunit('Jy/beam')
            if not ok:
                stop('setbrightnessunit 1 failed')
            #
            r = myim.pixelvalue([0,0,0])
            ok = (r['value']['value']==1.0) and (r['value']['unit']=='Jy/beam')
            ok = ok and (r['mask']==T)
            ok = ok and (r['pixel'][0]==0)
            ok = ok and (r['pixel'][1]==0)
            ok = ok and (r['pixel'][2]==0)
            if not ok:
                stop('pixelvalue 1 recovered wrong values')
            #
            r = myim.pixelvalue([-1,-1,-1])
            if r != {}:
                stop('pixelvalue 2 recovered wrong values')
            r = myim.pixelvalue(myim.shape())
            if r != {}:
                stop('pixelvalue 3 recovered wrong values')
            #
            r = myim.pixelvalue([2,2,2,100])
            ok = (r['value']['value']==0.0) and (r['value']['unit']=='Jy/beam')
            ok = ok and (r['mask']==T)
            ok = ok and (r['pixel'][0]==2)
            ok = ok and (r['pixel'][1]==2)
            ok = ok and (r['pixel'][2]==2)
            if not ok:
                stop('pixelvalue 4 recovered wrong values')
            #
            ok = myim.done();
            if not ok:
                stop('Done 1 fails')
        return cleanup(testdir)
    
    def test13():
        # Test methods
        #   getregion, putregion, set, replacemaskedpixels
        info('')
        info('')
        info('')
        info('Test 13 - getregion, putregion, set, replacemaskedpixels')

        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            note("Cleanup failed", "SEVERE")
            return false
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        # Make images of all the wondrous flavours that we have
        root = testdir + '/' + 'testimage'
        imshape = [10,20,30]
        images = makeAllImageTypes(imshape, root, includereadonly=T)
        if not images:
            fail('failed to make all image types')

        # Loop over all image types
        types = images.keys()
        for mytype in types:
            info('')
            info('Testing Image type ' + images[mytype]["type"])
            myim = images[mytype]["tool"]
            # Reassign as one of the image types is ImageConcat
            imshape = myim.shape();
            #
            # Get some regions !
            #
            info('')
            info('Testing getregion')
            #local pixels, mask
            r1 = rg.box()
            pixels = myim.getregion(region=r1, list=T)
            mask = myim.getregion(region=r1, list=T, getmask=True)
            if len(pixels)==0 or len(mask)==0:
                stop('getregion 1 failed')
            bb = myim.boundingbox(r1)
            pixels_shape=list(pixels.shape)
            mask_shape=list(mask.shape)
            if not all(pixels_shape, mask_shape):
                stop('getregion 1 pixels and mask have different shapes')
            if not all(pixels_shape, bb['regionShape']):
                stop('getregion 1 recovers wrong array shape')
            for i in range(pixels_shape[0]):
                for j in range(pixels_shape[1]):
                    for k in range(pixels_shape[2]):
                        if not (pixels[i][j][k] < 0.0001):
                            stop('getregion 1 recovers wrong pixel values=')
                        if not (mask[i][j][k] == T):
                            stop('getregion 1 recovers wrong mask values=')
            #
            csys = myim.coordsys()
            ok = rg.setcoordinates(csys.torecord())
            if not ok:
                stop('Failed to set coordinate system in regionmanager 1')
            if not csys.done():
                fail('Failed in csys.done()')
            #blc = qa.quantity([2,4,6], 'pix')
            blc = ['2pix','4pix','6pix']
            if not blc:
                fail('Failed to create blc')
            #trc = qa.quantity([8,10,12], 'pix')
            trc = ['8pix','10pix','12pix']
            if not trc:
                fail('Failed to create trc')
            r1 = rg.wbox(blc=blc, trc=trc)
            if not r1:
                stop('Failed to make region r1')
            r2 = rg.wpolygon(x=['5pix','6pix','7pix','8pix'],
                             y=['5pix','5pix','10pix','7.5pix'],
                             pixelaxes=[0,1])
            if not r2:
                stop('Failed to make region r2')
            d={"reg1":r1, "reg2":r2}
            r3 = rg.makeunion(d)
            if not r3:
                stop('Failed to make region r3')
            pixels = myim.getregion(r3)
            mask = myim.getregion(r3, getmask=true)
            if len(pixels)==0 or len(mask)==0:
                stop('getregion 2 failed')
            bb = myim.boundingbox(r3)
            pixels_shape = pixels.shape
            for i in range(len(pixels_shape)):
                if pixels_shape[i] != bb['regionShape'][i]:
                    stop('getregion 2 recovers wrong array shape')
            for i in range(pixels_shape[0]):
                for j in range(pixels_shape[1]):
                    for k in range(pixels_shape[2]):
                        if not (pixels[i][j][k] < 0.0001):
                            stop('getregion 2 recovers wrong pixel values=')
            #
            pixels = myim.getregion (axes=[0,1], dropdeg=T)
            mask = myim.getregion (axes=[0,1], dropdeg=T, getmask=true)
            if len(pixels)==0:
                fail('getregion 3 failed to retrieval pixels')
            if ((pixels.shape)[0]!=imshape[2]) or ((mask.shape)[0]!=imshape[2]):
                stop('getregion 3 recovers wrong array shape')
            #
            pixels = myim.getregion (axes=[1,2], dropdeg=T)
            mask = myim.getregion (axes=[1,2], dropdeg=T, getmask=true)
            if (len(pixels)==0) or (len(mask)==0):
                fail('getgregion could not retrieve pixels/mask')
            if (pixels.shape[0] != imshape[0]) or (mask.shape[0] != imshape[0]):
                stop('getregion 4 recovers wrong array shape')
            #
            pixels = myim.getregion (axes=[0], dropdeg=T)
            mask = myim.getregion (axes=[0], dropdeg=T, getmask=true)
            if (len(pixels)==0) or (len(mask)==0):
                if (not all(list(shape(pixels)),imshape[1:3])) or (not all(list(shape(mask)),imshape[1:3])):
                    stop('getregion 5 recovers wrong array shape')
            #
            pixels = myim.getregion()
            if len(pixels)==0:
                fail('failed to retrieve pixels with getregion')
            #
            mask = myim.getregion (getmask=true)
            if len(mask)==0:
                fail('failed to retrieve pixelmask with getregion')
            #
            if not myim.done():
                fail('failed in myim.done()')
            #
        ok = cleanup(testdir)
        if not ok:
            fail('failed to cleanup')

        #
        # Putregions
        #
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        imshape = [10,20,30]
        images = makeAllImageTypes(imshape, root, includereadonly=F)
        if not images:
            fail('failed to make all image types')

        # Loop over all image types
        types = images.keys()
        for thetype in types:
            info('')
            info('Testing Image type ' + images[thetype]["type"])
            myim = images[thetype]["tool"]
            imshape = myim.shape(); # Reassign as one of the image types is ImageConcat
            #
            info('');
            info('Testing putregion')
            pixels = myim.getregion(r3)
            mask = myim.getregion(r3, getmask=true)
            if (len(pixels)==0 or len(mask)==0):
                fail('geregion failed to get r3')
            ok = myim.putregion(pixelmask=mask, region=r3)
            if not ok:
                stop('putregion 1 failed')
            pixels2 = myim.getregion(r3)
            mask2 = myim.getregion(r3, getmask=true)
            if (len(pixels)==0 or len(mask)==0):
                stop('getregion 6 failed')
            if not allbooleq(mask2,mask):
                stop('getregion 6 recovered wrong mask')
            #
            r1 = rg.box(trc=[1000])
            pixels = myim.getregion(r1)
            mask = myim.getregion(r1)
            if (len(pixels)==0 or len(mask)==0):
                stop('getregion 7 failed')
            #
            r1 = rg.box(trc=[1000])
            ok = myim.putregion(pixels, mask, r1)
            if not ok:
                stop('putregion 2 failed')
            #
            try:
                note('Expect SEVERE error and Exception here')
                ok = myim.putregion()
            except Exception, e:
                note('Expected exception occurred.')
                ok = False
            if ok:
                stop('putregion 3 unexpectedly did not fail')
            pixels = ia.makearray(0,[2,3,4,5])
            try:
                note('Expect SEVERE error and Exception here')
                ok = myim.putregion(pixels=pixels)
            except Exception, e:
                note('Expected exception occurred')
                ok = False
            if ok:
                stop('putregion 4 unexpectedly did not fail')
            mask = ia.makearray(F,[2,3,4,5])
            try:
                note('Expect SEVERE error and Exception here')
                ok = myim.putregion(pixelmask=mask)
            except Exception, e:
                note('Expected exception occurred')
                ok = False
            if ok:
                stop('putregion 5 unexpectedly did not fail')
            pixels = ia.makearray(0,[100,100,100])
            try:
                note('Expect SEVERE error and Exception here')
                ok = myim.putregion(pixels=pixels)
            except Exception, e:
                note('Expected exception occurred')
                ok = False
            if ok:
                stop('putregion 6 unexpectedly did not fail')
            #
            pixels = myim.getregion()
            mask = myim.getregion(getmask=true)
            if (len(pixels)==0 or len(mask)==0):
                stop('getregion 8 failed')
            sh = myim.shape()
            pixels[0:sh[0], 0:sh[1], 0:sh[2]] = 1.0
            mask[0:sh[0], 0:sh[1], 0:sh[2]] = T
            mask[0,0,0] = F
            ok = myim.putregion(pixels=pixels, pixelmask=mask, usemask=F)
            if not ok:
                stop('putregion 7 failed')
            mask = myim.getregion(getmask=true)
            pixels = myim.getregion()
            if (len(pixels)==0 or len(mask)==0):
                stop('getregion 9 failed')
            for i in range(sh[0]):
                for j in range(sh[1]):
                    for k in range(sh[2]):
                        if not (pixels[i][j][k] - 1.0 < 0.0001):
                            stop('getregion 9 recovers wrong pixel values')
            if (mask[0,0,0]==T):
                stop('getregion 9 recovers wrong mask values (1)')
            tmp = mask[1:sh[0], 1:sh[1], 1:sh[2]]
            for i in range(sh[0]-1):
                for j in range(sh[1]-1):
                    for k in range(sh[2]-1):
                        if not (tmp[i][j][k] == T):
                            stop('getregion 9 recovers wrong mask values (2)')
            #
            pixels[0:sh[0], 0:sh[1], 0:sh[2]] = 10.0
            ok = myim.putregion(pixels=pixels, usemask=T)
            if not ok:
                stop('putregion 8 failed')
            pixels = myim.getregion()
            mask = myim.getregion(getmask=true)
            if len(pixels)==0 or len(mask)==0:
                stop('getregion 10 failed')
            if ((pixels[0,0,0]-1.0)>0.0001):
                stop('getregion 10 recovers wrong pixel values (1)')
            tmp = pixels[1:sh[0], 1:sh[1], 1:sh[2]] - 1.0
            for i in range(sh[0]-1):
                for j in range(sh[1]-1):
                    for k in range(sh[2]-1):
                        if not (tmp[i][j][k] > 0.0001):
                            stop('getregion 10 recovers wrong pixel values (2)')
            #
            pixels[0:sh[0], 0:sh[1], 0:sh[2]] = 10.0
            mask[0:sh[0], 0:sh[1], 0:sh[2]] = T
            ok = myim.putregion(pixels=pixels, pixelmask=mask, usemask=F)
            if not ok:
                stop('putregion 9 failed')
            pixels = ia.makearray(0.0, [sh[0], sh[1]])
            # Pad with degenerate axes
	    print sh
	    print "shape", sh[0], sh[1]
	    print type(sh[0])
	    print type(sh[1])
	    print "pixels", pixels
            ok = myim.putregion(pixels=pixels, usemask=F)
            if not ok:
                stop('putregion 10 failed')
            pixels = myim.getregion()
            mask = myim.getregion(getmask=true)
            if len(pixels)==0 or len(mask)==0:
                stop('getregion 11 failed')
            tmp = pixels[0:sh[0], 0:sh[1], 0]
            for i in range(sh[0]):
                for j in range(sh[1]):
                    if not (tmp[i][j] < 0.0001):
                        stop('getregion 11 recovers wrong pixel values (1)')
            tmp = pixels[0:sh[0], 0:sh[1], 0:sh[2]] - 10.0
            for i in range(sh[0]):
                for j in range(sh[1]):
                    for k in range(sh[2]):
                        if not (tmp[i][j][k] < 0.0001):
                            stop('getregion 11 recovers wrong pixel values (2)')
            #
            pixels = ia.makearray(0.0, [sh[0], sh[1], sh[2]])
            mask = ia.makearray(T, [sh[0], sh[1], sh[2]])
            mask[0,0,0] = F
            ok = myim.putregion(pixels=pixels, pixelmask=mask, usemask=F)
            if not ok:
                stop('putregion 11 failed')
            pixels = ia.makearray(0.0, [sh[0], sh[1]])
            # Pad with degenerate axes
            ok = myim.putregion(pixels=pixels, usemask=T);
            if not ok:
                stop('putregion 12 failed')
            pixels = myim.getregion()
            mask = myim.getregion(getmask=true)
            if not ok:
                stop('getregion 12 failed')
            if ((pixels[0,0,0]-10.0)>0.0001):
                stop('getregion 12 recovers wrong pixel values (1)')
            tmp = pixels[1:sh[0], 1:sh[1], 0]
            for i in range(sh[0]-1):
                for j in range(sh[1]-1):
                    if not (tmp[i][j] < 0.0001):
                        stop('getregion 12 recovers wrong pixel values (1)')
            tmp = pixels[0:sh[0], 0:sh[1], 1:sh[2]] - 10.0
            for i in range(sh[0]):
                for j in range(sh[1]):
                    for k in range(sh[2]-1):
                        if not (tmp[i][j][k] < 0.0001):
                            stop('getregion 12 recovers wrong pixel values (2)')
            #
            # Test replication
            #
            ok = myim.set(pixels=0.0, pixelmask=F)
            if not ok:
                stop('set 1 fails')
            #
            # Adds degenerate axis
            p = ia.makearray(10.0, [imshape[0], imshape[1]]);
            # Adds degenerate axis
            m = ia.makearray(T, [imshape[0], imshape[1]]);
            ok = myim.putregion(pixels=p, pixelmask=m, usemask=F, replicate=T)
            if not ok:
                stop('putregion 13 fails')
            p2 = myim.getregion()
            m2 = myim.getregion(getmask=true)
            if len(p2)==0 or len(m2)==0:
                stop('getregion 13 fails')
            sh = p2.shape
            for i in range(sh[0]):
                for j in range(sh[1]):
                    for k in range(sh[2]):
                        if not p2[i][j][k]==10.0:
                            stop('putregion 13 put wrong values')
            #
            ok = myim.set(pixels=0.0, pixelmask=F)
            if not ok:
                stop('set 1 fails')
            #
            p = ia.makearray(10.0, [imshape[0], imshape[1], 1])
            m = ia.makearray(T, [imshape[0], imshape[1], 1]);
            ok = myim.putregion(pixels=p, pixelmask=m, usemask=F, replicate=T)
            if not ok:
                stop('putregion 14 fails')
            p2 = myim.getregion()
            m2 = myim.getregion(getmask=true)
            if len(p2)==0 or len(m2)==0:
                stop('getregion 14 fails')
            sh = p2.shape
            for i in range(sh[0]):
                for j in range(sh[1]):
                    for k in range(sh[2]):
                        if not p2[i][j][k]==10.0:
                            stop('putregion 14 put wrong values')
            #
            # set
            #
            info('');
            info('Testing set')
            #
            try:
                note('Expect SEVERE error and Exception here')
                ok = myim.set(pixels='doggies')
            except Exception, e:
                note('Expected exception occurred')
                ok = False
            if ok:
                stop('set 1 unexpectedly did not fail')
            try:
                note('Expect SEVERE error and Exception here')
                ok = myim.set(pixelmask='doggies')
            except Exception, e:
                note('Expected exception occurred: '+str(e))
                ok = False
            if ok:
                stop('set 2 unexpectedly did not fail')
            note('Expect Warning message here')
            ok = myim.set()
            if ok:
                stop('set 3 unexpectedly did not fail')
	    try :
               note('Expect Warning message here')
               ok = myim.set(region='doggies')
	    except Exception, e:
	       ok = False
            if ok:
                stop('set 4 unexpectedly did not fail')
            try:
                note('Expect SEVERE error and Exception here')
                ok = myim.set(pixels='imname')
            except Exception, e:
                note('Expected exception occurred')
                ok = False
            if ok:
                stop('set 5 unexpectedly did not fail')
            #
            ok = myim.set(pixels=1.0)
            if not ok:
                stop('set 6 failed')
            pixels = myim.getregion()
            mask = myim.getregion(getmask=true)
            if len(pixels)==0 or len(mask)==0:
                stop('getregion 6 failed')
            sh = pixels.shape
            for i in range(sh[0]):
                for j in range(sh[1]):
                    for k in range(sh[2]):
                        if not ((pixels[i][j][k] -1) < 0.0001):
                            stop('getregion 6 recovers wrong pixel values')
            #
            ok = myim.set(pixels='1.0')
            if not ok:
                stop('set 7 failed')
            pixels = myim.getregion()
            mask = myim.getregion(getmask=true)
            if len(pixels)==0 or len(mask)==0:
                stop('getregion 7 failed')
            sh = pixels.shape
            for i in range(sh[0]):
                for j in range(sh[1]):
                    for k in range(sh[2]):
                        if not ((pixels[i][j][k] -1) < 0.0001):
                            stop('getregion 7 recovers wrong pixel values')
            #
            pixels[0,0,0] = -100
            ok = myim.putregion(pixels=pixels)
            if not ok:
                stop('putregion 7 failed')
            imname = testdir + '/subimage.test'
            myim2 = myim.subimage(imname)
            if not myim2:
                fail('failed to get subimage myim2')
            expr = 'min("' + imname + '")'
            ok = myim.set(pixels=expr)
            if not ok:
                stop('set 8 failed')
            pixels = myim.getregion()
            mask = myim.getregion(getmask = true)
            if len(pixels)==0 or len(mask)==0:
                stop('getregion 8 failed')
            sh = pixels.shape
            for i in range(sh[0]):
                for j in range(sh[1]):
                    for k in range(sh[2]):
                        if not ((pixels[i][j][k] + 100) < 0.0001):
                            stop('getregion 8 recovers wrong pixel values')
            if not myim2.remove(T):
                fail('failed to remove myim2')
            #
            ok = myim.set(pixelmask=F)
            if not ok:
                stop('set 9 failed')
            pixels = myim.getregion()
            mask = myim.getregion(getmask=true)
            if len(pixels)==0 or len(mask)==0:
                stop('getregion 9 failed')
            if mask.any():  #mask==F
                stop('getregion 9 recovers wrong mask values')
            #
            ok = myim.set(pixels=1.0)
            if not ok:
                stop('set 10 failed')
            blc = [0,0,4]
            trc = [2,3,9]
            r1 = rg.box(blc, trc)
            ok = myim.set(pixels=0.0, pixelmask=T, region=r1)
            if not ok:
                stop('set 11 failed')
            pixels = myim.getregion()
            mask = myim.getregion(getmask=true)
            if len(pixels)==0 or len(pixels)==0:
                stop('getregion 10 failed')
            blc2 = [3,4,10] # blc2 = trc+ 1
            tmp = pixels[blc[0]:trc[0],blc[1]:trc[1],blc[2]:trc[2]]
            for i in range(tmp.shape[0]):
                for j in range(tmp.shape[1]):
                    for k in range(tmp.shape[2]):
                        if not (tmp[i][j][k] < 0.0001):
                            stop('getregion 10 recovers wrong pixel values')
            tmp = pixels[blc2[0]:imshape[0],blc2[1]:imshape[1],blc2[2]:imshape[2]]
            for i in range(tmp.shape[0]):
                for j in range(tmp.shape[1]):
                    for k in range(tmp.shape[2]):
                        if not ((tmp[i][j][k] - 1) < 0.0001):
                            stop('getregion 10 recovers wrong pixel values')
            ok1 = (mask[blc[0]:trc[0],blc[1]:trc[1],blc[2]:trc[2]]).all()
            ok2 = (mask[blc2[0]:imshape[0],blc2[1]:imshape[1],blc2[2]:imshape[2]]).any()  # (not all false if ok2 is true)
            if not ok1 or ok2:
                stop('getregion 10 recovers wrong mask values')
            #
            # Can not see a way to do this in casapy
            #global __global_setimage = ref myim
            #ok = myim.set(pixels='min('+myim+')')
            #if not ok:
            #    stop('set 11 failed')
            #
            # replacemaskedpixels
            #
            info('')
            info('Testing replacemaskedpixels')
            ok = myim.set(pixels=0.0, pixelmask=T)
            if not ok:
                stop('set 12 failed')
            pixels = myim.getregion()
            mask = myim.getregion(getmask=true)
            if len(pixels)==0 or len(mask)==0:
                stop('getregion 11 failed')
            ys = imshape[1] - 3
            ye = imshape[1]
            mask[0:2,ys:(ye+1),] = F
            ok = myim.putregion(pixelmask=mask)
            if not ok:
                stop('putregion 8 failed')
            #
            try:
                note('Expect SEVERE error and Exception here')
                ok = myim.replacemaskedpixels(pixels=T)
            except Exception, e:
                note('Expected exception occurred: '+str(e))
                ok = False
            if ok:
                stop('replacemaskedpixels 1 unexpectedly did not fail')
            try:
                note('Expect SEVERE error and Exception here')
                ok = myim.replacemaskedpixels(mask ='makemaskgood doggies')
            except Exception, e:
                note('Expected exception occurred: '+str(e))
                ok = False
            if ok:
                stop('replacemaskedpixels 2 unexpectedly did not fail')
            #
            value = -1.0
            ok = myim.replacemaskedpixels(pixels=value)
            if not ok:
                stop('replacemaskedpixels 3 failed')
            pixels = myim.getregion()
            mask2 = myim.getregion(getmask=true)
            if not ok:
                stop('getregion 12 failed')
            tmp = pixels[0:2,ys:ye,] - value
            for i in range(tmp.shape[0]):
                for j in range(tmp.shape[1]):
                    for k in range(tmp.shape[2]):
                        if not (tmp[i][j][k] < 0.0001):
                            stop('getregion 12 recovered wrong pixel values')
            tmp = pixels[2:imshape[0], 0:ys,]
            for i in range(tmp.shape[0]):
                for j in range(tmp.shape[1]):
                    for k in range(tmp.shape[2]):
                        if not (tmp[i][j][k] < 0.0001):
                            stop('getregion 12 recovered wrong pixel values')
            if not allbooleq(mask,mask2):
                stop('getregion 12 recovered wrong mask')
            #
            #global __global_replaceimage = ref myim
            value = 10.0
            #ex1 = 'max($__global_replaceimage)+10'
            #ok = myim.replacemaskedpixels(pixels=ex1)
            #if not ok:
            #    stop('replacemaskedpixels 4 failed')
            pixels = myim.getregion()
            mask2 = myim.getregion(getmask=true)
            if len(pixels)==0 or len(mask2)==0:
                stop('getregion 13 failed')
            tmp = pixels[0:2,ys-1:ye,] - value
            for i in range(tmp.shape[0]):
                for j in range(tmp.shape[1]):
                    for k in range(tmp.shape[2]):
                        if not (tmp[i][j][k] < 0.0001):
                            stop('getregion 13 recovered wrong pixel values')
            tmp = pixels[2:imshape[0], 0:(ys-2),]
            for i in range(tmp.shape[0]):
                for j in range(tmp.shape[1]):
                    for k in range(tmp.shape[2]):
                        if not (tmp[i][j][k] < 0.0001):
                            stop('getregion 13 recovered wrong pixel values')
            if not allbooleq(mask,mask2):
                stop('getregion 13 recovered wrong mask')
            #
            ok = myim.set(pixels=1.0, pixelmask=T)
            if not ok:
                stop('set 12 failed')
            pixels = myim.getregion()
            mask = myim.getregion(getmask=true)
            if len(pixels)==0 or len(mask)==0:
                stop('getregion 15 failed')
            mask[0,0,0] = F;
            mask[imshape[0]-1,imshape[1]-1,imshape[2]-1] = F
            ok = myim.putregion(pixelmask=mask)
            if not ok:
                stop('putregion 9 failed')
            #
            imname2 = testdir + '/' + 'ia.fromshape.image3'
            myim2 = ia.newimagefromshape(outfile=imname2, shape=imshape)
            if not myim2:
                stop('ia.newimagefromshape constructor 2 failed')
            ok = myim2.set(pixels=2.0)
            if not ok:
                stop('set 13 failed')
            ex1 = '"' + imname2 +'"'
            ok = myim.replacemaskedpixels(pixels=ex1)
            if not ok:
                stop('replacemaskedpixels 6 failed')
            pixels = myim.getregion()
            mask2 = myim.getregion(getmask=true)
            if len(pixels)==0 or len(mask2)==0:
                stop('getregion 16 failed')
            if pixels[0,0,0]!=2 or pixels[imshape[0]-1,imshape[1]-1,imshape[2]-1]!=2:
                stop('getregion 16a recovered wrong pixel values')
            tmp = pixels[1:(imshape[0]-2), 1:(imshape[1]-2),1:(imshape[2]-2)]
            for i in range(tmp.shape[0]):
                for j in range(tmp.shape[1]):
                    for k in range(tmp.shape[2]):
                        if not (tmp[i][j][k] == 1.0):
                            stop('getregion 16b recovered wrong pixel values')
            #
            ok = myim.done();
            if not ok:
                stop('Done 2 fails')
            ok = myim2.remove(T);
            if not ok:
                stop('Done 3 fails')

        ###
        return cleanup(testdir)

    def test14():
        #
        # Test methods
        #   tofits
        #
        info('')
        info('')
        info('')
        info('Test 14 - FITS conversion')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            note("Cleanup failed", "SEVERE")
            return false
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        # Make image
        imshape = [12,24,20]
        myim = ia.newimagefromshape(shape=imshape)
        if not myim:
            stop('ia.fromshape constructor 1 failed')
        #
        fitsname1 = testdir + '/' + 'fimage1'
        ok = fitsreflect (myim, fitsname1)
        if not ok:
            fail('failed in fitsreflect')
        ok = fitsreflect (myim, fitsname1, do16=T)
        if not ok:
            fail('failed in second fitsreflect')
        #
        ok = myim.tofits(outfile=fitsname1, region=rg.box())
        if not ok:
            stop('tofits 1 failed')
        #
        fitsname2 = testdir + '/' + 'fimage2'
        r1 = rg.box(trc=[10000])
        ok = myim.tofits(outfile=fitsname2, region=r1)
        if not ok:
            stop('tofits 2 failed')
        #
        # Not useful because there is no spectral axis and I can't make one !
        #
        fitsname3 = testdir + '/' + 'fimage3'
        ok = myim.tofits(outfile=fitsname3, optical=F, velocity=F)
        if not ok:
            stop('tofits 3 failed')
        ok = myim.done();
        if not ok:
            stop('Done 1 fails')
        ###
        return cleanup(testdir)

    def test15():
        # Test methods
        #   boundingbox, {set}restoringbeam, coordmeasures, topixel, toworld
        info('-------')
        info('Test 15 - boundingbox,       ')
        info('          {set}restoringbeam ')
        info('          coordmeasures,     ')
        info('          topixel, toworld   ')
        info('-------')

        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            note("Cleanup failed", "SEVERE")
            return false
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        # Make images of all the wondrous flavours that we have
        root = testdir + '/' + 'testimage'
        imshape = [12,24,20]
        images = makeAllImageTypes(imshape, root, includereadonly=T)
        if not images:
            fail('failed to make all image types')

        # Loop over all image types
        types = images.keys()
        for mytype in types:
            info('')
            info('Testing Image type ' + images[mytype]["type"])
            myim = images[mytype]["tool"]
            imshape = myim.shape()
            #
            info('')
            info('Testing boundingbox')
            info('')
            bb = myim.boundingbox()
            if not bb:
                stop('boundingbox 1 failed')
            ok = bb.has_key('blc') and bb.has_key('trc')
            ok = ok and bb.has_key('inc')
            ok = ok and bb.has_key('bbShape')
            ok = ok and bb.has_key('regionShape')
            ok = ok and bb.has_key('imageShape')
            if not ok:
                stop('boundingbox record 1 has invalid fields')
            stat = not all(bb['blc'],[0,0,0])
            theTRC = []
            for value in imshape:
                theTRC.append(value-1)
            stat = stat or (not all(bb['trc'], theTRC))
            stat = stat or (not all(bb['inc'], [1,1,1]))
            stat = stat or (not all(bb['bbShape'], imshape))
            stat = stat or (not all(bb['regionShape'], imshape))
            stat = stat or (not all(bb['imageShape'], imshape))
            if stat:
                stop('boundingbox record 1 values are invalid')
            #
            blc = [2,3,4]
            trc = [5,8,15]
            inc = [1,2,3]
            #
            trc2 = [5,7,13];    # Account for stride
            rShape = [4,3,4]
            r1 = rg.box(blc=blc, trc=trc, inc=inc)
            bb = myim.boundingbox(region=r1)
            if not bb:
                stop('boundingbox 2 failed')
            stat = not all(bb['blc'],blc)
            stat = stat or (not all(bb['trc'], trc2))
            stat = stat or (not all(bb['inc'], inc))
            bbshp = []
            for i in range(len(trc2)):
                bbshp.append(trc2[i]-blc[i]+1)
            stat = stat or (not all(bb['bbShape'], bbshp))
            stat = stat or (not all(bb['regionShape'], rShape))
            stat = stat or (not all(bb['imageShape'], imshape))
            if stat:
                stop('boundingbox record 2 values are invalid')
            #
            trc = [100,100,100]
            r1 = rg.box(trc=trc)
            bb = myim.boundingbox(region=r1)
            if not bb:
                stop('boundingbox 4 failed')
            stat = not all(bb['blc'], [0,0,0])
            theTRC = []
            for value in imshape:
                theTRC.append(value-1)
            stat = stat or (not all(bb['trc'], theTRC))
            stat = stat or (not all(bb['inc'], [1,1,1]))
            stat = stat or (not all(bb['bbShape'], imshape))
            stat = stat or (not all(bb['regionShape'], imshape))
            stat = stat or (not all(bb['imageShape'], imshape))
            if stat:
                stop('boundingbox record 4 values are invalid')
            #
            trc = [10,20,30,40,50,60]
            r1 = rg.box(trc=trc)
            try:
                note("Expect SEVERE Error and Exception here")
                bb = myim.boundingbox(region=r1)
            except Exception, e:
                #print "Expected Exception: ", e
                note("Excepted occurred as expected.")
                bb = {}
            myim.done()
            if bb:
                stop('boundingbox 5 unexpectedly did not fail')

        ok = cleanup(testdir)
        if not ok:
            fail('cleanup failed')
        #
        # Make image with all coordinate types
        #
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        imname =  testdir + '/' + 'ia.fromshape.image2'
        imshape = [10,10,4,10,10]
        ok = ia.fromshape(outfile=imname, shape=imshape)
        if not ok:
            stop('ia.fromshape 2 failed')
        ia.close()
        myim = ia.newimage(imname)
        #
        # Coordmeasures. A wrapper for coordsys where testing more thorough
        #
        info('')
        info('Testing coordmeasures')
        info('')
        mycs = myim.coordsys()
        if not mycs:
            fail('failed to get mycs from image')
        rp = mycs.referencepixel()
        if not rp:
            fail('failed to get referencepixel')
        if not mycs.done():
            fail('failed to close mycs')
        w = myim.coordmeasures(list(rp['numeric']))
        if not w:
            stop('coordmeasures failed')
        w = w['measure']
        ok = w.has_key('direction') and w.has_key('spectral')
        ok = ok and w.has_key('stokes')
        ok = ok and w.has_key('linear')
        if not ok:
            stop('coordmeasures record has wrong fields')
        #
        # topixel/toworld are just wrappers; coordsys tests more thoroughly
        #
        info('')
        info('Testing topixel/toworld')
        info('')
        #w = myim.toworld(list(rp['numeric']), 'nqms')
        w = myim.toworld(rp, 'nqms')
        if not w:
            fail('failed toworld')
        p = myim.topixel(w)
        if not p:
            fail('failed topixel')
        #
        ok = myim.done()
        if not ok:
            stop('Done 2 fails')
            
        ###
        return cleanup(testdir)

    def test16():
        #
        # Test methods
        #   summary, maskhandler
        #
        info('')
        info('')
        info('')
        info('Test 16 - summary, maskhandler')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            note("Cleanup failed", "SEVERE")
            return false
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"

        # Make images of all the wondrous flavours that we have
        root = testdir + '/' + 'testimage'
        imshape = [12,24,20]
        images = makeAllImageTypes(imshape, root, includereadonly=T)
        if not images:
            fail('failed to make all image types')

        # Loop over all image types
        info('')
        info('Testing summary')
        types = images.keys()
        for mytype in types:
            info('')
            info('Testing Image type ' + images[mytype]["type"])
            myim = images[mytype]["tool"]
            imshape = myim.shape()
            #
            # Summary
            #
            header = myim.summary(list=F)
            if not header:
                fail('unable to retrieve summary')
            ok = header.has_key('ndim') and header.has_key('shape')
            ok = ok and header.has_key('tileshape')
            ok = ok and header.has_key('axisnames')
            ok = ok and header.has_key('refpix')
            ok = ok and header.has_key('refval')
            ok = ok and header.has_key('incr')
            ok = ok and header.has_key('axisunits')
            ok = ok and header.has_key('unit')
            ok = ok and header.has_key('imagetype')
            ok = ok and header.has_key('hasmask')
            ok = ok and header.has_key('defaultmask')
            ok = ok and header.has_key('masks')
            if not ok:
                stop('summary record is invalid')
            if not myim.done():
                fail('failed in myim.done')
        ok = cleanup(testdir)
        if not ok:
            fail('could not cleanup testdir')
        #
        # Masks
        #
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        imshape = [12,24,20]
        images = makeAllImageTypes(imshape, root, includereadonly=F)
        if not images:
            fail('failed to make all image types')
        #
        # Loop over all image types
        #
        info('')
        info('Testing maskhandler')
        types = images.keys()
        for mytype in types:
            info('')
            info('Testing Image type ' + images[mytype]["type"])
            myim = images[mytype]["tool"]
            imshape = myim.shape()
            #
            myim2 = ia.newimagefromshape(shape=imshape)
            if not myim2:
                stop('ia.fromshape constructor 2 failed')
            #
            pixels = myim.getregion()
            mask = myim.getregion(getmask=true)
            if len(pixels)==0 or len(mask)==0:
                stop('getregion 1 failed')
            #
            ok = myim.putregion(pixels, mask)
            if not ok:
                stop('putregion 1a failed')
            ok = myim2.putregion(pixels, mask)
            if not ok:
                stop('putregion 1b failed')
            #
            names = myim.maskhandler('get')
            if not names:
                stop('maskhandler 1a failed')
            if (names[0]!="mask0"):      #(len(names)!=1) doesn't work
                stop('maskhandler 1a unexpectedly recovered more than 1 mask')
            names = myim2.maskhandler('get')
            if not names:
                stop('maskhandler 1b failed')
            if (names[0]!="mask0"):      #(len(names)!=1) doesn't work
                stop('maskhandler 1b unexpectedly recovered more than 1 mask')
            #
            ok = myim.maskhandler('set', name=names)
            if not ok[0]=="T":
                stop('maskhandler 2a failed')
            ok = myim2.maskhandler('set', name=names)
            if not ok[0]=="T":
                stop('maskhandler 2b failed')
            #
            defname = myim.maskhandler('default')
            if not defname:
                stop('maskhandler 3a failed')
            if (names!=defname):
                stop('maskhandler 3a did not recover the default mask name')
            defname = myim2.maskhandler('default')
            if not defname:
                stop('maskhandler 3b failed')
            if (names!=defname):
                stop('maskhandler 3b did not recover the default mask name')
            #
            names.append('fish')
            ok = myim.maskhandler('rename', names)
            if not ok[0]=="T":
                stop('maskhandler 4a failed')
            ok = myim2.maskhandler('rename', names)
            if not ok[0]=="T":
                stop('maskhandler 4b failed')
            #
            names = myim.maskhandler('get')
            if not names:
                stop('maskhandler 5a failed')
            if names[0]!='fish':
                stop('maskhandler 5a did not recover the correct mask name')
            names = myim2.maskhandler('get')
            if not names:
                stop('maskhandler 5b failed')
            if (names[0]!='fish'):
                stop('maskhandler 5b did not recover the correct mask name')
            #
            names = ['fish', 'mask1']
            ok = myim.maskhandler('copy', names)
            if not ok:
                stop('maskhandler 6a failed')
            ok = myim2.maskhandler('copy', names)
            if not ok:
                stop('maskhandler 6b failed')
            #
            names = myim.maskhandler('get')
            if not names:
                stop('maskhandler 7a failed')
            if (len(names)!=2):
                stop('maskhandler 7a unexpectedly recovered more than 2 mask')
            if (names[0] !='fish' or names[1]!='mask1'):
                stop('maskhandler 7a did not recover the correct mask names')
            names = myim2.maskhandler('get')
            if not names:
                stop('maskhandler 7b failed')
            if (len(names)!=2):
                stop('maskhandler 7b unexpectedly recovered more than 2 mask')
            if (names[0] !='fish' or names[1]!='mask1'):
                stop('maskhandler 7b did not recover the correct mask names')
            #
            ok = myim.maskhandler('set', 'mask1')
            if not ok:
                stop('maskhandler 8b failed')
            ok = myim2.maskhandler('set', 'mask1')
            if not ok:
                stop('maskhandler 8b failed')
            #
            defname = myim.maskhandler('default')
            if not defname:
                stop('maskhandler 9a failed')
            if (defname[0] !='mask1'):
                stop('maskhandler 9a did not recover the correct default mask name')
            defname = myim2.maskhandler('default')
            if not defname:
                stop('maskhandler 9b failed')
            if (defname[0] !='mask1'):
                stop('maskhandler 9b did not recover the correct default mask name')
            #
            names = myim.maskhandler('get')
            if not names:
                stop('maskhandler 10a failed')
            names = myim2.maskhandler('get')
            if not names:
                stop('maskhandler 10b failed')
            #
            ok = myim.maskhandler('delete', names)
            if not ok:
                stop('maskhandler 11a failed')
            ok = myim2.maskhandler('delete', names)
            if not ok:
                stop('maskhandler 11b failed')
            #
            names = myim.maskhandler('get')
            if not names:
                stop('maskhandler 12a failed')
            if (names[0]!="T"):
                return('maskhandler 12a failed to delete the masks')
            names = myim2.maskhandler('get')
            if not names:
                stop('maskhandler 12b failed')
            if (names[0]!="T"):
                return('maskhandler 12b failed to delete the masks')
            if not (myim2.done()):
                fail('failed done in test16')
            #
            ok = myim.done()
            ###
        return cleanup(testdir)
        #end test16

    def test17():
        #
        # Test methods
        #   subimage, insert
        #
        info('')
        info('')
        info('')
        info('Test 17 - subimage, insert')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            return False
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        # Make image
        imname = testdir +'/'+'ia.fromshape.image1'
        imshape = [10,20,6]
        myim = ia.newimagefromshape(outfile=imname, shape=imshape)
        if not myim:
            stop('ia.fromshape constructor 1 failed')
        #
        # Subimage
        #
        info('')
        info('Testing subimage function')
        imname2 = testdir+'/'+'subimage.image'
        pixels = myim.getregion()
        mask = myim.getregion(getmask=true)
        if len(pixels)==0 or len(mask)==0:
            stop('getregion 1 failed')
        mask[imshape[0]/2,imshape[1]/2,imshape[2]/2] = F
        ok = myim.putregion(pixelmask=mask)
        if not ok:
            stop('putregion 1 failed')
        #
        dowait = F
        try:
            note('Expect SEVERE error and Exception here')
            myim2 = myim.subimage(outfile=imname2, region='doggies')
        except Exception, e:
            note('Caught expected Exception')
            myim2 = false
        if myim2:
            stop('subimage 1 unexpectedly did not fail')
        #
        #r1 = rg.quarter()
        #r1 = myim.setboxregion([0.25,0.25],[0.75,0.75],frac=true)
        r1 = rg.box([0.25,0.25],[0.75,0.75],frac=true)
        myim2 = myim.subimage(outfile=imname2, region=r1)
        if not myim2:
            stop('subimage 2 failed')
        #
        bb = myim.boundingbox(region=r1)
        shape = bb['regionShape']
        shape2 = myim2.shape()
        if not all(shape,shape2):
            stop ('Output subimage has wrong shape');
        #
        pixels = myim.getregion(region=r1)
        mask = myim.getregion(getmask=true, region=r1)
        if len(pixels)==0 or len(mask)==0:
            stop('getregion 2 failed')
        pixels2 = myim2.getregion()
        mask2 = myim2.getregion(getmask=true)
        if not ok:
            stop('getregion 3 failed')
        if not alleq(pixels,pixels2,tolerance=0.0001):
            stop('The data values are wrong in the subimage')
        if not allbooleq(mask,mask2):
            stop('The mask values are wrong in the subimage')
        ok = myim2.remove(done=T)
        if not ok:
            stop('Failed to remove' + imname2)
        ok = myim.done()
        if not ok:
            stop('Done 1 failed')
        #
        # Insert
        #
        info('')
        info('Testing insert function')
        imname = testdir+'/'+'ia.fromshape.image'
        a = ia.makearray(1,[10,20])
        myim = ia.newimagefromarray(outfile=imname, pixels=a)
        if not myim:
            stop('ia.fromarray constructor 2 failed')
        #
        try:
            note("Expect SEVERE error and Exception here")
            ok = myim.insert(region='fish')
        except Exception, e:
            note("Caught expected exception.")
            ok = false
        if ok:
            stop('insert 1 unexpectedly did not fail')
        #
        pixels = myim.getchunk()
        if len(pixels)==0:
            stop('getchunk 1 failed')
        #
        padshape = [myim.shape()[0]+2,myim.shape()[1]+2]
        padname = testdir+'/'+'insert.image'
        myim2 = ia.newimagefromshape(shape=padshape)
        if not myim2:
            fail('failed to create myim2 from padshape')
        #
        ok = myim2.insert(infile=myim.name(F), locate=[0,0,0])
        if not ok:
            fail('failed to insert myim')
        pixels2 = myim2.getchunk()
        if len(pixels2)==0:
            stop('getchunk 2 failed')
        pixels3 = pixels2[0:(padshape[0]-2), 0:(padshape[1]-2)]
        if not alleqnum(pixels3,1.0,tolerance=0.0001):
            stop('inserted image pixels have wrong value (1)')
        #
        myim2.set(0.0)
        ok = myim2.insert(infile=myim.name(F));  # Placed symmetrically
        if not ok:
            fail('failed insert')
        pixels2 = myim2.getchunk()
        if len(pixels2)==0:
            fail('failed to getchunk')
        if (pixels2[0,0]!=0.0):
            stop('inserted image pixels have wrong value (3)')
        if not alleqnum(pixels2[1:(padshape[0]-1),1:(padshape[1]-1)],1.0,tolerance=0.0001):
            stop('inserted image pixels have wrong value (2)')
        if not myim2.done():
            fail('failed myim2.done')
        #
        ok = myim.done()
        if not ok:
            stop('Done 3 failed')
        ###
        return cleanup(testdir)

    def test18():
        #
        # Test methods
        #   hanning
        #
        info('');
        info('');
        info('');
        info('Test 18 - hanning');
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            return False
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        # Make image
        imname = testdir+'/'+'ia.fromshape.image'
        imshape = [10,20]
        myim = ia.newimagefromshape(outfile=imname, shape=imshape)
        if not myim:
            stop('ia.fromshape constructor 1 failed')
        pixels = myim.getchunk()
        if len(pixels)==0:
            stop('getchunk 1 failed')
        for i in range(pixels.shape[0]):
            for j in range(pixels.shape[1]):
                if pixels[i][j]>-10000:
                    pixels[i][j]=1
        ok = myim.putchunk(pixels)
        if not ok:
            stop('putchunk 1 failed')
        #
        try:
            note('Expect SEVERE error and Exception here')
            myim2 = myim.hanning(region='fish');
        except Exception, e:
            note('Caught expected Exception')
            myim2 = false
        if myim2:
            stop('hanning 1 unexpectedly did not fail')
        try:
            note('Expect SEVERE error and Exception here')
            myim2 = myim.hanning(axis=19)
        except Exception, e:
            note('Caught expected Exception')
            myim2 = false
        if myim2:
            stop('hanning 2 unexpectedly did not fail')
        try:
            note('Expect SEVERE error and Exception here')
            myim2 = myim.hanning(drop='fish')
        except Exception, e:
            note('Caught expected Exception:'+str(e))
            myim2 = false
        if myim2:
            stop('hanning 3 unexpectedly did not fail')
        try:
            note('Expect SEVERE error and Exception here')
            myim2 = myim.hanning(outfile=[1,2,3])
        except Exception, e:
            note('Caught expected Exception:'+str(e))
            myim2 = false
        if myim2:
            stop('hanning 4 unexpectedly did not fail')
        #
        hanname = testdir+'/'+'hanning.image'
        myim2 = myim.hanning(outfile=hanname, axis=0, drop=F)
        if not myim2:
            stop('hanning 5 failed')
        if not all(myim2.shape(),myim.shape()):
            stop('Output image has wrong shape (1)')
        pixels2 = myim2.getchunk()
        if len(pixels2)==0:
            stop('getchunk 2 failed')
        if not alleqnum(pixels2,1,tolerance=0.0001):
            stop('hanning image pixels have wrong value (1)')
        ok = myim2.remove(done=T)
        if not ok:
            stop('Failed to remove'+hanname)
        #
        myim2 = myim.hanning(outfile=hanname, axis=0, drop=T)
        if not myim2:
            stop('hanning 6 failed')
        shape2 = [myim.shape()[0]/2-1,myim.shape()[1]]
        if not all(myim2.shape(),shape2):
            stop('Output image has wrong shape (2)')
        pixels2 = myim2.getchunk()
        if len(pixels2)==0:
            stop('getchunk 3 failed')
        if not alleqnum(pixels2,1,tolerance=0.0001):
            stop('Hanning image pixels have wrong value (2)')
        ok = myim2.remove(done=T)
        if not ok:
            stop('Failed to remove'+hanname)
        #
        pixels = myim.getregion()
        mask = myim.getregion(getmask=true)
        if len(pixels)==0 or len(mask)==0:
            stop('getregion 1 failed')
        mask[0,0] = F
        mask[1,0] = F
        mask[2,0] = F
        mask[3,0] = F
        ok = myim.putregion(pixelmask=mask)
        if not ok:
            stop('putregion 1 failed')
        myim2 = myim.hanning(outfile=hanname, axis=0, drop=F)
        if not myim2:
            stop('hanning 7 failed')
        pixels2 = myim2.getregion()
        mask2 = myim2.getregion(getmask=true)
        if not ok:
            stop('getregion 2 failed')
        ok = (mask2[0,0]==F and mask2[1,0]==F)
        ok = ok and mask2[2,0]==F
        ok = ok and mask2[3,0]==F
        if not ok:
            stop('Hanning image mask is wrong (1)')
        ok = pixels2[0,0]==0 and pixels2[1,0]==0
        ok = ok and pixels2[2,0]==0
        ok = ok and pixels2[3,0]==0.25
        if not ok:
            stop('Hanning image pixels have wrong value (3)')
        ok = myim2.done()
        if not ok:
            stop('Done 1 failed')
        #
        ok = myim.done()
        if not ok:
            stop('Done 2 failed')
        ###
        return cleanup(testdir)

    def test19():
        #
        # Test methods
        #   convolve
        #
        info('')
        info('')
        info('')
        info('Test 19 - convolve')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            return False
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        # Make image
        imname = testdir+'/'+'ia.fromshape.image'
        imshape = [10,10]
        myim = ia.newimagefromshape(outfile=imname, shape=imshape)
        if not myim:
            stop('ia.fromshape constructor 1 failed')
        pixels = myim.getchunk()
        if len(pixels)==0:
            stop('getchunk 1 failed')
        for i in range(pixels.shape[0]):
            for j in range(pixels.shape[1]):
                if pixels[i][j]>-10000:
                    pixels[i][j]=1
        ok = myim.putchunk(pixels)
        if not ok:
            stop('putchunk 1 failed')
        # 
        try:
            note('Expect SEVERE error and Exception here - 1')
            kernel = ia.makearray(0.0,[3,3])
            myim2 = myim.convolve(kernel=kernel, region='fish');
        except Exception, e:
            note('Caught expected Exception:'+str(e))
            myim2 = false
        if myim2:
            stop('convolve 1 unexpectedly did not fail')
        try:
            note('Expect SEVERE error and Exception here - 2')
            #kernel = array(0,2,4,6,8)
            kernel = ia.makearray(0,[2,4,6,8])
            myim2 = myim.convolve(kernel=kernel)
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            myim2 = false
        if myim2:
            stop('convolve 3 unexpectedly did not fail')
        try:
            note('Expect SEVERE error and Exception here - 3')
            myim2 = myim.convolve(outfile=[1,2,3])
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            myim2 = false
        if myim2:
            stop('convolve 4 unexpectedly did not fail')
        #
        outname = testdir+'/'+'convolve.image'
        kernel = ia.makearray(0.0,[3,3])
        kernel[1,1] = 1;                   # Delta function
        #
        myim2 = myim.convolve(outfile=outname, kernel=kernel)
        if not myim2:
            stop('convolve 5 failed')
        if not all(myim2.shape(),myim.shape()):
            stop('Output image has wrong shape (1)')
        pixels2 = myim2.getchunk()
        if len(pixels2)==0:
            stop('getchunk 2 failed')
        if not alleq(pixels2, pixels, 0.0001):
            stop('convolve image pixels have wrong value (1)')
        ok = myim2.remove(done=T)
        if not ok:
            stop('Failed to remove'+outname)
        #
        kernelname = testdir+'/'+'convolve.kernel'
        kernelimage = ia.newimagefromarray(outfile=kernelname, pixels=kernel)
        kernelimage.done()
        myim2 = myim.convolve(outfile=outname, kernel=kernelname)
        if not myim2:
            stop('convolve 5b failed')
        if not all(myim2.shape(),myim.shape()):
            stop('Output image has wrong shape (1)')
        pixels2 = myim2.getchunk()
        if len(pixels2)==0:
            stop('getchunk 2b failed')
        if not alleq(pixels2,pixels,0.0001):
            stop('convolve image pixels have wrong value (1b)')
        ok = myim2.remove(done=T)
        if not ok:
            stop('Failed to remove' + outname)
        r1 = rg.box([0,0], [8,8])
        myim2 = myim.convolve(outfile=outname, kernel=kernel, region=r1)
        if not myim2:
            stop('convolve 6 failed')
        if myim2.shape()[0] != 9 or myim2.shape()[1] !=9:
            stop('Output image has wrong shape (2)')
        pixels2 = myim2.getchunk()
        if len(pixels2)==0:
            stop('getchunk 3 failed')
        pixels = myim.getregion(region=r1)
        mask = myim.getregion(getmask=true, region=r1)
        if len(pixels)==0 or len(mask)==0:
            stop('getregion 3 failed')
        if not alleq(pixels2,pixels,0.0001):
            stop('convolve image pixels have wrong value (2)')
        ok = myim2.remove(done=T)
        if not ok:
            stop('Failed to remove' + outname)
        #
        pixels = myim.getregion()
        mask = myim.getregion(getmask=true)
        if len(pixels)==0 or len(mask)==0:
            stop('getregion 1 failed')
        mask[0,0] = F
        mask[1,0] = F
        mask[0,1] = F
        mask[1,1] = F
        ok = myim.putregion(pixelmask=mask)
        if not ok:
            stop('putregion 1 failed')
        myim2 = myim.convolve(outfile=outname, kernel=kernel)
        if not myim2:
            stop('convolve 7 failed')
        pixels2 = myim2.getregion()
        mask2 = myim2.getregion(getmask=true)
        if len(pixels2)==0 or len(mask2)==0:
            stop('getregion 2 failed')
        ok = mask2[0,0]==F and mask2[1,0]==F
        ok = ok and mask2[0,1]==F
        ok = ok and mask2[1,1]==F
        if not ok:
            stop('convolved image mask is wrong (1)')
        ok = abs(pixels2[0,0])<0.0001 and abs(pixels2[1,0])<0.0001
        ok = ok and abs(pixels2[0,1])<0.0001
        ok = ok and abs(pixels2[1,1])<0.0001
        ok = ok and abs(pixels2[2,2]-1.0)<0.0001
        if not ok:
            stop('convolved image pixels have wrong value (3)')
        ok = myim2.done()
        if not ok:
            stop('Done 1 failed')
        #
        ok = myim.done()
        if not ok:
            stop('Done 2 failed')
        ###
        return cleanup(testdir)

    def test20():
        #
        # Test methods
        #   sepconvolve
        #
        info('')
        info('')
        info('')
        info('Test 20 - sepconvolve')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            return False
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        # Make image
        imname = testdir+'/'+'ia.fromshape.image'
        imshape = [128,128]
        centre = [imshape[0]/2,imshape[1]/2]
        myim = ia.newimagefromshape(outfile=imname, shape=imshape)
        if not myim:
            stop('ia.fromshape constructor 1 failed')
        pixels = myim.getchunk()
        if len(pixels)==0:
            stop('getchunk 1 failed')
        pixels[centre] = 1
        pixels[centre[0]-1, centre[1]-1] = 1
        pixels[centre[0]-1, centre[1]+1] = 1
        pixels[centre[0]+1, centre[1]+1] = 1
        pixels[centre[0]+1, centre[1]+1] = 1
        #
        ok = myim.putchunk(pixels)
        if not ok:
            stop('putchunk 1 failed')
        #
        try:
            note('Expect SEVERE error and Exception here')
            myim2 = myim.sepconvolve(region='fish');
        except Exception, e:
            note('Caught expected Exception')
            myim2 = false
        if myim2:
            stop('sepconvolve 1 unexpectedly did not fail')
        try:
            note('Expect SEVERE error and Exception here')
            myim2 = myim.sepconvolve(types=["doggies"], widths=[1], axes=[0])
        except Exception, e:
            note('Caught expected Exception')
            myim2 = false
        if myim2:
            stop('sepconvolve 2 unexpectedly did not fail')
        try:
            note('Expect SEVERE error and Exception here')
            myim2 = myim.sepconvolve(types=["gauss","gauss","gauss","gauss"],
                                     widths=[5,5,5,5], axes=[0,1,2,3])
        except Exception, e:
            note('Caught expected Exception')
            myim2 = false
        if myim2:
            stop('sepconvolve 3 unexpectedly did not fail')
        try:
            note('Expect SEVERE error and Exception here')
            myim2 = myim.sepconvolve(types=["gauss","gauss"], widths=[1], axes=[0,1])
        except Exception, e:
            note('Caught expected Exception')
            myim2 = false
        if myim2:
            stop('sepconvolve 4 unexpectedly did not fail')
        try:
            note('Expect SEVERE error and Exception here')
            myim2 = myim.sepconvolve(outfile=[1,2,3])
        except Exception, e:
            note('Caught expected Exception')
            myim2 = false
        if myim2:
            stop('sepconvolve 5 unexpectedly did not fail')
        #
        outname2 = testdir+'/'+'sepconvolve.image'
        myim2 = myim.sepconvolve(outfile=outname2, axes=[0,1],
                                 types=["gauss","box"], widths=[3,3])
        if not myim2:
            stop('sepconvolve 6 failed')
        ok = myim2.remove(done=T)
        if not ok:
            stop('Failed to remove' + outname2)
        #
        outname2 = testdir+'/'+'sepconvolve.image'
        myim2 = myim.sepconvolve(outfile=outname2, axes=[0,1],
                                 types=["hann","gauss"], widths=[3,10])
        if not myim2:
            fail('Failed in sepconvolve')
        ok = myim2.remove(done=T)
        if not ok:
            stop('Failed to remove' + outname2)
        #
        myim2 = myim.sepconvolve(outfile=outname2, axes=[0,1],
                               types=["gauss","gauss"], widths=[5,5])
        if not myim2:
            fail('failed in sepconvole')
        stats1 = myim.statistics(list=F);
        if not stats1:
            fail('failed to get statistics')
        stats2 = myim2.statistics(list=F)
        if not stats2:
            fail('failed to get stat2')
        #if not (abs((stats1['sum'])-(stats2['sum']))<0.0001):
        #    stop('Convolution did not preserve flux (1)')
        ok = myim2.remove(done=T)
        if not ok:
            stop('Failed to remove' + outname2)
        #
        cen = [imshape[0]/2-1,imshape[1]/2-1]
        blc = [cen[0]-10,cen[1]-10]
        trc = [cen[0]+10,cen[1]+10]
        r1 = rg.box(blc, trc)
        myim2 = myim.sepconvolve(outfile=outname2, axes=[0],
                                 types="hann", widths=[3], region=r1)
        if not myim2:
            fail('failed in sepconvolve')
        if not all(myim2.shape(),[trc[0]-blc[0]+1,trc[1]-blc[1]+1]):
            stop('Output image has wrong shape (2)')
        pixels2 = myim2.getchunk()
        if len(pixels2)==0:
            stop('getchunk 3 failed')
        #     if (!(abs(sum(pixels)-sum(pixels2))<0.0001)) {
        #        stop('Convolution did not preserve flux (2)')
        ok = myim2.remove(done=T)
        if not ok:
            stop('Failed to remove', outname2)
        #
        pixels = myim.getregion()
        mask = myim.getregion(getmask=true)
        if len(pixels)==0 or len(mask)==0:
            fail('failed getregion in sepconvolve')
        mask[0,0] = F
        mask[1,0] = F
        mask[0,1] = F
        mask[1,1] = F
        ok = myim.putregion(pixels=pixels, pixelmask=mask)
        if not ok:
            stop('putregion 1 failed')
        myim2 = myim.sepconvolve(outfile=outname2, types="gauss",
                                 widths=[10], axes=[0])
        if not myim2:
            stop('sepconvolve 8 failed')
        pixels2 = myim2.getregion()
        mask2 = myim2.getregion(getmask=true)
        if not ok:
            stop('getregion 2 failed')
        ok = mask2[0,0]==F and mask2[1,0]==F
        ok = ok and mask2[0,1]==F
        ok = ok and mask2[1,1]==F
        if not ok:
            stop('convolved image mask is wrong (1)')
        ok = myim2.done()
        if not ok:
            stop('Done 1 failed')
        #
        # Some more tests just on the widths interface.
        #
        myim2 = myim.sepconvolve(widths=[10,10], axes=[0,1])
        if not myim2: stop ('sepconvolve 9 failed')
        if not myim2.done(): fail('failed myim2.done')
        #
        #myim2 = myim.sepconvolve(widths="10 10", axes=[0,1])
        myim2 = myim.sepconvolve(widths="10pix 10pix", axes=[0,1])
        if not myim2: stop ('sepconvolve 10 failed')
        if not myim2.done(): fail('failed myim2.done')
        #
        myim2 = myim.sepconvolve(widths="0.01rad 10pix", axes=[0,1])
        if not myim2: stop ('sepconvolve 11 failed')
        if not myim2.done(): fail('failed myim2.done')
        #
        #myim2 = myim.sepconvolve(widths="20 10pix", axes=[0,1])
        myim2 = myim.sepconvolve(widths="20pix 10pix", axes=[0,1])
        if not myim2: stop ('sepconvolve 12 failed')
        if not myim2.done(): fail('failed myim2.done')
        #
        #myim2 = myim.sepconvolve(widths='10 10', axes=[0,1])
        myim2 = myim.sepconvolve(widths='10pix 10pix', axes=[0,1])
        if not myim2: stop ('sepconvolve 13 failed')
        if not myim2.done(): fail('failed myim2.done')
        #
        #widths = qa.quantity("0.01rad 0.02rad")
        widths = "0.01rad 0.02rad"
        myim2 = myim.sepconvolve(widths=widths, axes=[0,1])
        if not myim2: stop ('sepconvolve 14 failed')
        if not myim2.done(): fail('failed myim2.done')
        #
        ok = myim.done()
        if not ok:
            stop('Done 2 failed')
        ###

        return cleanup(testdir)
    def test22():
        #
        # Test methods
        #   statistics
        #
        info('')
        info('')
        info('')
        info('Test 22 - statistics')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            return False
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        # Make image
        pixels = ia.makearray(0.0, [10, 20])
        pixels[0,0] = -100
        pixels[9,19] = 100
        imname = testdir+'/'+'ia.fromarray.image'
        myim = ia.newimagefromarray(outfile=imname, pixels=pixels)
        if not myim:
            stop('ia.fromarray constructor 1 failed')
        #
        try:
            note('Expect SEVERE error and Exception here')
            ok = myim.statistics(axes=[10,20])
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok:
            stop('Statistics unexpectedly did not fail (1)')
        try:
            note('Expect SEVERE error and Exception here')
            ok = myim.statistics(includepix=[-100,100], excludepix=[-100,100])
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok:
            stop('Statistics unexpectedly did not fail (2)')
        #
        stats = myim.statistics(list=F)
        if not stats:
            stop('Statistics failed (1)')
        ok = stats.has_key('npts') and stats.has_key('sum')
        ok = ok and stats.has_key('sumsq')
        ok = ok and stats.has_key('min')
        ok = ok and stats.has_key('max')
        ok = ok and stats.has_key('mean')
        ok = ok and stats.has_key('sigma')
        ok = ok and stats.has_key('rms')
        if not ok:
            stop('Statistics record does not have the correct fields')
        prod = 1
        for i in myim.shape(): prod *= i
        ok = stats['npts']==prod and stats['sum']==0.0
        ok = ok and stats['sumsq']==2e4
        ok = ok and stats['min']==-100.0
        ok = ok and stats['max']==100.0
        ok = ok and stats['mean']==0.0
        if not ok:
            stop('Statistics values are wrong (1)')
        ##
        blc = [0,0]; trc = [4,4]
        r1 = rg.box(blc=blc, trc=trc)
        stats = myim.statistics(list=F, region=r1)
        if not stats:
            stop('Statistics failed (2)')
        ok = stats['npts']==((trc[0]-blc[0]+1)*(trc[1]-blc[1]+1))
        ok = ok and stats['sum']==-100.0
        ok = ok and stats['sumsq']==10000.0
        ok = ok and stats['min']==-100.0
        ok = ok and stats['max']==0.0
        ok = ok and stats['mean']==(-100/stats['npts'])
        if not ok:
            stop('Statistics values are wrong (2)')
        ##
        stats = myim.statistics(list=F, axes=[0])
        if not ok:
            stop('Statistics failed (3)')
        imshape = myim.shape()
        ok = len(stats['npts'])==imshape[1]
        ok = ok and len(stats['sum'])==imshape[1]
        ok = ok and len(stats['sumsq'])==imshape[1]
        ok = ok and len(stats['min'])==imshape[1]
        ok = ok and len(stats['max'])==imshape[1]
        ok = ok and len(stats['mean'])==imshape[1]
        if not ok:
            stop('Statistics record fields are wrong length (1)')
        ok = alleqnum(stats['npts'],10,tolerance=0.0001)
        ok = ok and stats['sum'][0]==-100
        ok = ok and stats['sum'][imshape[1]-1]==100
        ok = ok and alleqnum(stats['sum'][1:imshape[1]-1],0,tolerance=0.0001)
        ok = ok and stats['sumsq'][0]==10000
        ok = ok and stats['sumsq'][imshape[1]-1]==10000
        ok = ok and alleqnum(stats['sumsq'][1:(imshape[1]-1)],0,tolerance=0.0001)
        ok = ok and stats['min'][0]==-100
        ok = ok and alleqnum(stats['min'][1:imshape[1]],0,tolerance=0.0001)
        ok = ok and stats['max'][imshape[1]-1]==100
        ok = ok and alleqnum(stats['max'][0:(imshape[1]-1)],0,tolerance=0.0001)
        ok = ok and stats['mean'][0]==-10
        ok = ok and stats['mean'][imshape[1]-1]==10
        ok = ok and alleqnum(stats['mean'][1:(imshape[1]-1)],0,tolerance=0.0001)
        if not ok:
            stop('Statistics values are wrong (3)')
        ##
        stats = myim.statistics(list=F, includepix=[-5,5])
        if not stats:
            stop('Statistics failed (4)')
        prod = 1
        for i in imshape: prod *= i
        ok = stats['npts']==prod-2
        ok = ok and stats['sum']==0.0
        ok = ok and stats['sumsq']==0.0
        ok = ok and stats['min']==0.0
        ok = ok and stats['max']==0.0
        ok = ok and stats['mean']==0.0
        if not ok:
            stop('Statistics values are wrong (4)')
        #
        stats = myim.statistics(list=F, excludepix=[-5,5])
        if not stats:
            stop('Statistics failed (4)')
        ok = stats['npts']==2 and stats['sum']==0.0
        ok = ok and stats['sumsq']==20000.0
        ok = ok and stats['min']==-100.0
        ok = ok and stats['max']==100.0
        ok = ok and stats['mean']==0.0
        if not ok:
            stop('Statistics values are wrong (5)')
        ##
        ok = myim.statistics(list=F, disk=T, force=T)
        if not ok:
            stop('Statistics failed (5)')
        ok = myim.statistics(list=F, disk=F, force=T)
        if not ok:
            stop('Statistics failed (6)')
        #
        ok = myim.done()
        if not ok:
            stop('Done failed (1)')

        return cleanup(testdir)

    def test24():
        #
        # Test methods
        #   moments
        info('')
        info('')
        info('')
        info('Test 24 - moments')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            return False
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        # Make image
        imshape = [50,100]
        pixels = ia.makearray(0.0, imshape)
        imname = testdir+'/'+'ia.fromarray.image'
        myim = ia.newimagefromarray(outfile=imname, pixels=pixels)
        if not myim:
            fail('failed to create test image in test24')
        #
        try:
            note('Expect SEVERE error and Exception here')
            ok = myim.moments(axis=0, moments=[22])
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok:
            stop('moments unexpectedly did not fail (1)')
        try:
            note('Expect SEVERE error and Exception here')
            ok = myim.moments(axis=99)
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok:
            stop('moments unexpectedly did not fail (2)')
        try:
            note('Expect SEVERE error and Exception here')
            ok = myim.moments(method='doggies')
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok:
            stop('moments unexpectedly did not fail (3)')
        try:
            note('Expect SEVERE error and Exception here')
            ok = myim.moments(smoothaxes=[9,19])
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok:
            stop('moments unexpectedly did not fail (4)')
        try:
            note('Expect SEVERE error and Exception here')
            ok = myim.moments(smoothaxes=[9,19], smoothtypes="gauss gauss", smoothwidths=[10,10])
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok:
            stop('moments unexpectedly did not fail (5)')
        try:
            note('Expect SEVERE error and Exception here')
            ok = myim.moments(smoothaxes=[0,1], smoothtypes="fish gauss", smoothwidths=[10,10])
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok:
            stop('moments unexpectedly did not fail (6)')
        try:
            note('Expect SEVERE error and Exception here')
            ok = myim.moments(smoothaxes=[0,1], smoothtypes="gauss gauss", smoothwidths=[-100,10])
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok:
            stop('moments unexpectedly did not fail (7)')
        try:
            note('Expect SEVERE error and Exception here')
            ok = myim.moments(includepix=[-100,100], excludepix=[-100,100])
        except Exception, e:
            note('Caught expected Exception')
            ok = false
        if ok:
            stop('moments unexpectedly did not fail (8)')
        #
        base1 = testdir+'/'+'base1'
        base2 = testdir+'/'+'base2'
        im2 = myim.moments(outfile=base1, axis=0)
        if not im2:
            fail('failed to get moments')
        ok = im2.done()
        if not ok: fail('failed to done image')
        im2 = myim.moments(outfile=base2, axis=1)
        if not im2:
            fail('failed to get moments (2)')
        ok = im2.done()
        if not ok: fail('failed to done image')
        #
        base3 = testdir+'/'+'base3'
        #im2 = myim.moments(outfile=base3, axis=0, moments=[-1,0,1,2,3,5,6,7,8,9,10,11])
        im2 = myim.moments(outfile=base3, axis=0,
                           moments=range(-1,4)+range(5,12))
        if not im2:
            fail('failed to get moments (3)')
        ok = im2.done()
        if not ok: fail('failed to done image')
        #
        base4 = testdir+'/'+'base4'
        pixels = myim.getchunk()
        pixels[0,0] = 10
        ok = myim.putchunk(pixels)
        if not ok: fail('failed to putchunk')
        # Average
        im2 = myim.moments(outfile=base4, axis=0, moments=[-1],
                           smoothaxes=[0,1], smoothtypes="gauss box",
                           smoothwidths=[5,10], includepix=[-100,100])
        if not im2: fail('failed to get moments (4)')
        pixels2 = im2.getchunk()
        v = 10.0 / imshape[0]
        if abs(pixels2[0]-v)>0.00001:
            stop('Moment pixel values are wrong')
        #
        ok = myim.done()
        if not ok: fail('failed done')
        ok = im2.done()
        if not ok: fail('failed done')
        return cleanup(testdir)

    def test25():
        #
        # Test methods
        #   modify and fitcomponents
        info('')
        info('')
        info('')
        info('Test 25 - modify, fitcomponents')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            return False
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        #
        # Make image
        #
        imname = testdir+'/'+'ia.fromshape.image'
        imshape = [128,128,1]
        myim = ia.newimagefromshape(imname, imshape)
        if not myim:
            stop('ia.fromshape constructor 1 failed')
        #
        # Add units and restoring beam
        #
        ok = myim.setbrightnessunit('Jy/beam')
        if not ok: fail('failed in setbrightnessunit')
        ok = myim.setrestoringbeam(major='5arcmin', minor='2.5arcmin',
                                   pa='60deg', log=F)
        if not ok: fail('failed in setrestoringbeam')
        #
        # Pretty hard to test properly.  Add model
        #
        qmaj = '10arcmin'  #qa.quantity(10, 'arcmin')
        qmin = '5arcmin'   #qa.quantity(5, 'arcmin')
        qpa = '45.0deg'    #qa.quantity(45.0,'deg')
        flux = 100.0
        cl0 = gaussian(flux, qmaj, qmin, qpa)
        if not (myim.modify(cl0.torecord(), subtract=F)):
            fail('failed in modify')
        stats = myim.statistics(list=F)
        if not stats:
            fail('failed to get statistics')
        diff = abs(stats['flux']-flux)/flux
        if (diff > 0.001):
            stop('model image 1 has wrong values');
        #
        # Subtract it again
        #
        if not myim.modify(cl0.torecord(), subtract=T): fail()
        stats = myim.statistics(list=F)
        if not stats: fail()
        p = myim.getchunk()
        if not alleqnum(p,0.0,tolerance=1e-6):
            stop('model image 2 has wrong values');
        #
        # Now add the model for fitting
        #
        if not myim.modify(cl0.torecord(), subtract=F): fail()
        #

        cl1 = myim.fitcomponents(
            region=rg.box(blc=[32, 32, 0],
            trc=[96, 96, 0])
        )
        myim.done()
        if not cl1:
            stop('fitcomponents 1 failed')
        if not cl1['converged']:
            stop('fitcomponents 1 did not converge')
        cl1tool=cltool()
        cl1tool.fromrecord(cl1['results'])
        if not compareComponentList(cl0,cl1tool):
            stop('failed fitcomponents 1')
        return cleanup(testdir)

    def test28():
        #
        # Test methods
        #   convolve2d
        info('')
        info('')
        info('')
        info('Test 28 - convolve2d')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            return False
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        # Make sky image
        nx = 128
        ny = 128
        imshape = [nx,ny]
        centre = [imshape[0]/2,imshape[1]/2]
        myim = ia.newimagefromshape(shape=imshape)
        if not myim: stop('ia.fromshape constructor 1 failed')
        mycs = myim.coordsys()
        #mycs.setincrement(value="-1arcsec 1arcsec")
        mycs.setincrement(value=["-1arcsec","1arcsec"])
        ok = myim.setcoordsys(mycs.torecord())
        if not ok: stop ('Failed to set coordsys 1')
        mycs.done()
        #
        # These tests don't test the pixel values, just units and interface
        # Whack a gaussian in and set restoring beam to its shape
        #
        gausspixels = gaussianarray (nx, ny, 1.0, 20.0, 10.0, 0.0)
        ok = myim.putchunk(gausspixels)
        if not ok: stop('putchunk 1 failed')
        ok = myim.setrestoringbeam(major='20arcsec', minor='10arcsec',
                                   pa='0.0deg', log=F)
        if not ok: stop('setrestoringbeam 1 failed')
        ok = myim.setbrightnessunit('Jy/beam')
        if not ok: stop('setbrightnessunits 1 failed')
        #
        # First test a Jy/beam convolution
        #
        #r = rg.quarter()
        r = rg.box(blc=[.25,.25],trc=[.75,.75],frac=true)
        myim2 = myim.convolve2d(major='20arcsec', minor='10arcsec', pa='0deg')
        if not myim2: stop('convolve2d 1 failed')
        bUnit = myim2.brightnessunit()
        if (bUnit!='Jy/beam'): stop ('convolve2d 1 set wrong brightness unit')
        major = math.sqrt(400 + 400)
        minor = math.sqrt(100 + 100)
        rb = myim2.restoringbeam()
        d1 = abs(rb['major']['value'] - major)
        d2 = abs(rb['minor']['value'] - minor)
        d3 = abs(rb['positionangle']['value'] - 0.0)
        if (d3 > 1e-5): d3 = abs(rb['positionangle']['value'] - 180.0)
        if (d1 >1e-5 or d2>1e-5 or d3>1e-5):
            stop ('convolve2d 1 set wrong restoring beam')
        if not myim2.done(): stop ('done 1 failed')
        #
        # Now set values in pixels (increment=1arcsec)
        #
        ok = myim.putchunk(gausspixels)
        if not ok: stop('putchunk 2 failed')
        ok = myim.setrestoringbeam(major='20arcsec', minor='10arcsec',
                                   pa='0.0deg', log=F)
        if not ok: stop('setrestoringbeam 2 failed')
        ok = myim.setbrightnessunit('Jy/beam')
        if not ok: stop('setbrightnessunits 2 failed')
        #
        #myim2 = myim.convolve2d (major=20, minor=10, pa=0, region=r)
        myim2 = myim.convolve2d (major='20arcsec', minor='10arcsec', pa='0deg',
                                 region=r)
        if not myim2: stop('convolve2d 2 failed')
        bUnit = myim2.brightnessunit()
        if (bUnit!='Jy/beam'): stop ('convolve2d 2 set wrong brightness unit')
        major = math.sqrt(20*20 + 20*20)
        minor = math.sqrt(10*10 + 10*10)
        rb = myim2.restoringbeam()
        d1 = abs(rb['major']['value'] - major)
        d2 = abs(rb['minor']['value'] - minor)
        d3 = abs(rb['positionangle']['value'] - 0.0)
        if (d3 > 1e-5): d3 = abs(rb['positionangle']['value'] - 180.0)
        if (d1 >1e-5 or d2>1e-5 or d3>1e-5):
            stop ('convolve2d 2 set wrong restoring beam')
        if not myim2.done(): stop ('done 2 failed')
        #
        # Now test a Jy/pixel convolution
        #
        ok = myim.set(0.0)
        if not ok: stop ('set 1 failed')
        pixels = myim.getchunk()
        if len(pixels)==0: stop ('getchunk 1 failed')
        pixels[nx/2,ny/2] = 1.0
        ok = myim.putchunk(pixels)
        if not ok: stop ('putchunk 3 failed')
        ok = myim.setrestoringbeam(remove=T, log=F)
        if not ok: stop('setrestoringbeam 3 failed')
        ok = myim.setbrightnessunit('Jy/pixel')
        if not ok: stop('setbrightnessunits 3 failed')
        #
        myim2 = myim.convolve2d (major='20arcsec', minor='10arcsec',
                                 pa='20deg', region=r)
        if not myim2: stop('convolve2d 3 failed')
        bUnit = myim2.brightnessunit()
        if (bUnit!='Jy/beam'): stop ('convolve2d 3 set wrong brightness unit')
        major = 20
        minor = 10
        rb = myim2.restoringbeam()
        d1 = abs(rb['major']['value'] - major)
        d2 = abs(rb['minor']['value'] - minor)
        q = qa.convert(rb['positionangle'],'deg')
        if not q: fail('failed convert')
        d3 = abs(qa.getvalue(q) - 20.0)
        if (d1 >1e-5 or d2>1e-5 or d3>1e-5):
            stop ('convolve2d 3 set wrong restoring beam')
        if not myim2.done(): stop ('done 3 failed')
        #
        # Now test axes other than the sky
        #
        mycs = cs.newcoordsys(linear=2)
        ok = mycs.setunits(value=["km","km"], overwrite=T)
        if not ok: fail('failed setunits')
        ok = myim.setcoordsys(mycs.torecord())
        if not ok: stop ('Failed to set coordsys 2')
        mycs.done()
        ok = myim.set(0.0)
        if not ok: stop ('set 2 failed')
        pixels = myim.getchunk()
        if len(pixels)==0: stop ('getchunk 2 failed')
        pixels[nx/2,ny/2] = 1.0
        ok = myim.putchunk(pixels)
        if not ok: stop ('putchunk 4 failed')
        ok = myim.setrestoringbeam(remove=T, log=F)
        if not ok: stop('setrestoringbeam 3 failed')
        ok = myim.setbrightnessunit('kg')
        if not ok: stop('setbrightnessunits 4 failed')
        #
        myim2 = myim.convolve2d(major='20km', minor='10km',
                                pa='20deg', region=r)
        if not myim2: stop('convolve2d 4 failed')
        if not myim2.done(): stop ('done 4 failed')
        if not myim.done(): stop('done 5 failed')
        """ 
        I have no idea why anyone would want to do this, this is nonsensical and
        even though convolve2d() used to complete with these parameters, the resulting
        image had no meaning. convolve2d() does not work correctly for non-square pixels,
        and certainly does not give a proper result when the axes represent different
        domains, since there is no proper result in that case for a gaussian kernel (the only
        kernel supported at this time). I've added exception throwing for those cases, so ia.convolve2d()
        now fails. Tests should reflect real
        world use cases, not picking parameters randomly from parameter space just to show
        that a method completes for that nonsensical case. That's a bug, not a feature, and
        the method should fail.
        #
        # Now try a mixed axis convolution
        #
        mycs = cs.newcoordsys(direction=T, linear=1)
        nz = 32
        imshape = [nx,ny,nz]
        print "*** ea"
        centre = [imshape[0]/2,imshape[1]/2,imshape[2]/2]
        myim = ia.newimagefromshape(shape=imshape, csys=mycs.torecord())
        if not myim: stop('ia.fromshape constructor 2 failed')
        print "*** fa"
        if not mycs.done(): stop ('done 6 failed')
        #
        #myim2 = myim.convolve2d (major=20, minor=10, axes=[1,3])
        note('EXPECT WARNING MESSAGE HERE')
        print "*** fb"
        myim2 = myim.convolve2d (major='20pix', minor='10pix', axes=[0,2])
        print "*** fc"
        print "*** ba"
        if not myim2: stop('convolve2d 5 failed')
        if not myim2.done(): stop ('done 7 failed')
        if not myim.done(): stop('done 8 failed')
        """
        #
        # Now do some non autoscaling
        #
        imshape = [nx,ny]
        centre = [imshape[0]/2,imshape[1]/2]
        myim = ia.newimagefromshape(shape=imshape)
        if not myim: stop('ia.fromshape constructor 3 failed')
        mycs = myim.coordsys()
        #mycs.setincrement(value="-1arcsec 1arcsec")
        mycs.setincrement(value=["-1arcsec","1arcsec"])
        ok = myim.setcoordsys(mycs.torecord())
        if not ok: stop ('Failed to set coordsys 3')
        if not mycs.done(): stop('done 10 failed')
        #
        pixels = myim.getchunk()
        if len(pixels)==0: stop ('getchunk 3 failed')
        pixels[nx/2,ny/2] = 1.0
        ok = myim.putchunk(pixels)
        if not ok: stop ('putchunk 5 failed')
        ok = myim.setbrightnessunit('Jy/pixel')
        if not ok: stop('setbrightnessunits 5 failed')
        #
        # Convolution kernel has peak 1.0*scale
        #
        myim2 = myim.convolve2d (scale=2.0, major='20arcsec', minor='10arcsec', pa="0deg")
        if not myim2: stop('convolve2d 6 failed')
        stats = myim2.statistics(list=F)
        if not stats: fail('failed to get statistics')
        maxVal = stats['max']
        d1 = abs(maxVal - 2.0)
        if (d1>1e-5): stop ('convolve2d 6 got scaling wrong')
        if not myim2.done(): stop ('done 11 failed')
        if not myim.done(): stop('done 12 failed')
        #
        # Now some forced errors
        #
        nz = 32
        imshape = [nx,ny,nz]
        centre = [imshape[0]/2,imshape[1]/2,imshape[2]/2]
        myim = ia.newimagefromshape(shape=imshape)
        if not myim: stop('ia.fromshape constructor 4 failed')
        #
        try:
            note('Expect SEVERE error and Exception here')
            myim2 = myim.convolve2d (major='1km', minor='20arcsec', axes=[0,1])
        except Exception, e:
            note('Caught expected Exception')
            myim2 = false
        if myim2: stop ('Forced failure 1 did not occur')
        try:
            note('Expect SEVERE error and Exception here')
            myim2 = myim.convolve2d (major='10arcsec', minor='10Hz', axes=[0,2])
        except Exception, e:
            note('Caught expected Exception')
            myim2 = false
        if myim2: stop ('Forced failure 2 did not occur')
        try:
            note('Expect SEVERE error and Exception here')
            myim2 = myim.convolve2d (major='10pix', minor='10arcsec', axes=[0,1])
        except Exception, e:
            note('Caught expected Exception')
            myim2 = false
        if myim2: stop ('Forced failure 3 did not occur')
        if not myim.done(): stop('done 14 failed')
        ###
        return cleanup(testdir)

    def test29():
        #
        # Test methods
        #   deconvolvecomponentlist
        #
        info('')
        info('')
        info('')
        info('Test 29 - deconvolvecomponentlist')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            return False
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        # Make sky image
        nx = 128
        ny = 128
        imshape = [nx,ny]
        centre = [imshape[0]/2,imshape[1]/2]
        myim = ia.newimagefromshape(shape=imshape)
        if not myim: stop('ia.fromshape constructor 1 failed')
        ok = myim.summary()
        #
        ok = deconvolveTest (myim, 20.0, 10.0, 0.0, 1)
        if not ok: fail('failed deconvolveTest')
        #
        ok = deconvolveTest (myim, 20.0, 10.0, 45.0, 2)
        if not ok: fail('failed deconvolveTest 2')
        #
        ok = deconvolveTest (myim, 20.0, 10.0, -20.0, 3)
        if not ok: fail('failed deconvolveTest 3')
        #
        if not myim.done(): stop ('done 1 failed')
        ###
        return cleanup(testdir)

    def test30():
        #
        # Test methods
        #   findsources, maxfit
        #
        info('')
        info('')
        info('')
        info('Test 30 - findsources, maxfit')
        #
        # Make image
        #
        imshape = [128,128,1]
        myim = ia.newimagefromshape(shape=imshape)
        if not myim:
            stop('ia.fromshape constructor 1 failed')
        #
        # Add units and restoring beam
        #
        ok = myim.setbrightnessunit('Jy/beam')
        if not ok: fail()
        #
        qmaj = qa.quantity('5.00000000001 arcmin')
        qmin = qa.quantity('5.0 arcmin')
        qpa = qa.quantity('0.0 deg')
        ok = myim.setrestoringbeam(major=qmaj, minor=qmin, pa=qpa, log=F)
        if not ok: fail()
        #
        # Add four gaussians
        #
        mycs = myim.coordsys()
        if not mycs: fail()
        #
        rp = mycs.referencepixel()['numeric']
        p1 = [-30.0,-30.0,0.0] + rp
        d1 = mycs.toworld(p1, 'm')['measure']['direction']
        if not d1: fail()
        #
        p2 = [-30.0, 30.0,0.0] + rp
        d2 = mycs.toworld(p2, 'm')['measure']['direction']
        if not d2: fail()
        #
        p3 = [ 30.0, 30.0,0.0] + rp
        d3 = mycs.toworld(p3, 'm')['measure']['direction']
        if not d3: fail()
        #
        p4 = [ 30.0, -30.0,0.0] + rp
        d4 = mycs.toworld(p4, 'm')['measure']['direction']
        if not d4: fail()
        #
        f1 = 100.0
        cl1 = gaussian(f1, qmaj, qmin, qpa, dir=d1)
        if not cl1: fail()
	cl1Point = cltool()
        cl1Point.simulate(1)
        ok = cl1Point.setflux(0, cl1.getfluxvalue(0))
        if not ok: fail()
        ok = cl1Point.setshape(0, 'point', log=F)
        if not ok: fail()
        rd = cl1.getrefdir(0)
        ok = cl1Point.setrefdir(0, me.getvalue(rd)['m0'],
                                me.getvalue(rd)['m1'], log=F)
        if not ok: fail()
        #
        f2 = 80.0
        cl2 = gaussian(f2, qmaj, qmin, qpa, dir=d2)
        if not cl2: fail()
        #
        f3 = 60.0
        cl3 = gaussian(f3, qmaj, qmin, qpa, dir=d3)
        if not cl3: fail()
        #
        f4 = 40.0
        cl4 = gaussian(f4, qmaj, qmin, qpa, dir=d4)
        if not cl4: fail()
        #
        clIn = cltool()
        clIn.concatenate(cl1.torecord(), log=F)
        clIn.concatenate(cl2.torecord(), log=F)
        clIn.concatenate(cl3.torecord(), log=F)
        clIn.concatenate(cl4.torecord(), log=F)
        cl1.done()
        cl2.done()
        cl3.done()
        cl4.done()
        #
        if not myim.modify(clIn.torecord(), subtract=F): fail()

        # Now find them
        clOut = myim.findsources(10, cutoff=0.3)
        if not clOut: fail()
        clOuttool = cltool()
        clOuttool.fromrecord(clOut);
        if not compareComponentList(clIn,clOuttool,dotype=F):
            stop('failed findsources 1')
        if not clOuttool.done(): fail()
        #
        # Now try and find just first 3 sources
        #
        clOut = myim.findsources(10, cutoff=0.5)
        if not clOut: fail()
        clOuttool = cltool()
        clOuttool.fromrecord(clOut)
        clIn2 = cltool()
        clIn2.concatenate (clIn.torecord(), [0,1,2], log=F)
        if not compareComponentList(clIn2,clOuttool,dotype=F):
            stop('failed findsources 2')
        #
        if not clIn.done(): fail()
        if not clIn2.done(): fail()
        if not clOuttool.done(): fail()

        # Maxfit
        clOut = myim.maxfit()
        if not clOut: fail()
        clOuttool = cltool()
        clOuttool.fromrecord(clOut)
        if not compareComponentList(cl1Point,clOuttool,dotype=F):
            stop('failed maxfit')
        if not cl1Point.done(): fail()
        if not clOuttool.done(): fail()
        #
        ok = myim.done()
        if not ok:
            stop('Done failed (1)')
        ok = cs.done()
        if not ok:
            stop('Done failed (2)')
        #
        return True

    def test32():
        #
        # Test methods
        #   addnoise
        #
        info('')
        info('')
        info('')
        info('Test 32 - addnoise')
        #
        # Make tempimage
        #
        imshape = [512,512]
        myim = ia.newimagefromshape(shape=imshape)
        if not myim:
            stop('ia.fromshape constructor 1 failed')
        n = imshape[0]*imshape[1]
        # Add noisesesesese
        n1 = "binomial discreteuniform erlang geometric hypergeometric "
        n2 = "normal lognormal negativeexponential poisson uniform weibull"
        noises = (n1 + n2).split()
        #
        rec = {}
        rec['binomial'] = {}
        rec['binomial']['pars'] = [1, 0.5]
        #
        rec['discreteuniform'] = {}
        rec['discreteuniform']['pars'] = [-100, 100]
        rec['discreteuniform']['mean'] = 0.5 * (rec['discreteuniform']['pars'][1] + rec['discreteuniform']['pars'][0])
        #
        rec['erlang'] = {}
        rec['erlang']['pars'] = [1,1]
        rec['erlang']['mean'] = rec['erlang']['pars'][0]
        rec['erlang']['var'] = rec['erlang']['pars'][1]
        #
        rec['geometric'] = {}
        rec['geometric']['pars'] = [0.5]
        rec['geometric']['mean'] = rec['geometric']['pars'][0]
        #
        rec['hypergeometric'] = {}
        rec['hypergeometric']['pars'] = [0.5, 0.5]
        rec['hypergeometric']['mean'] = rec['hypergeometric']['pars'][0]
        rec['hypergeometric']['var'] = rec['hypergeometric']['pars'][1]
        #
        rec['normal'] = {}
        rec['normal']['pars'] = [0, 1]
        rec['normal']['mean'] = rec['normal']['pars'][0]
        rec['normal']['var'] = rec['normal']['pars'][1]
        #
        rec['lognormal'] = {}
        rec['lognormal']['pars'] = [1, 1]
        rec['lognormal']['mean'] = rec['lognormal']['pars'][0]
        rec['lognormal']['var'] = rec['lognormal']['pars'][1]
        #
        rec['negativeexponential'] = {}
        rec['negativeexponential']['pars'] = [1]
        rec['negativeexponential']['mean'] = rec['negativeexponential']['pars'][0]
        #
        rec['poisson'] = {}
        rec['poisson']['pars'] = [1]
        rec['poisson']['mean'] = rec['poisson']['pars'][0]
        #
        rec['uniform'] = {}
        rec['uniform']['pars'] = [-1, 1]
        rec['uniform']['mean'] = 0.5 * (rec['uniform']['pars'][1] + rec['uniform']['pars'][0])
        #
        rec['weibull'] = {}
        rec['weibull']['pars'] = [0.5, 1]
        #
        for n in noises:
            ok = myim.addnoise(zero=T, type=n, pars=rec[n]['pars'])
            if not ok: fail()
            #
            stats = myim.statistics (list=F)
            if not stats: fail()
            errMean = stats['sigma'] / math.sqrt(stats['npts'])
            sig = stats['sigma']
            #
            if rec[n].has_key('mean'):
                d = abs(stats['mean'] - rec[n]['mean'])
                if (d > errMean):
                    #fail('Mean wrong for distribution ' + n)
                    #note('MEAN WRONG FOR DISTRIBUTION ' + n)
                    #print "d=", d, "errMean=", errMean
                    pass
            #
            if rec[n].has_key('var'):
                d = abs(sig*sig - rec[n]['var'])
                #if (d > errMean):    # What is the error in the variance ???
                #    #fail('Variance wrong for distribution '+ n)
                #    note('VARIANCE WRONG FOR DISTRIBUTION '+ n)
                #    print "d=", d, "errMean=", errMean
                pass
            #
        if not myim.done(): fail()
        return True

    def test33():
        info('')
        info('')
        info('')
        info('Test 33 - {set}miscinfo, {set}history, {set}brightnessunit')
        info('        - {set}restoringbeam, convertflux')
        info('           with many Image types')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            return False
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        #
        # Make images of all the wondrous flavours that we have
        #
        root = testdir+'/'+'testimage'
        imshape = [10,10]
        rec = makeAllImageTypes(imshape, root)
        #
        mii = {}
        mii['x'] = 'x'
        mii['y'] = 'y'
        #
        bui = 'Jy/beam'
        hii = "I like doggies"
        #
        rbi = {}
        rbi['major'] = qa.quantity('10arcsec')
        rbi['minor'] = qa.quantity('5arcsec')
        rbi['positionangle'] = qa.quantity('30deg')
        names = rec.keys()
        for mytype in names:
            info('Testing Image type '+rec[mytype]['type'])
            #
            myim = rec[mytype]['tool']
            ok = myim.sethistory(history=hii)
            if not ok: fail()
            hio = myim.history(list=F, browse=F)
            if not hio: fail()
            if (len(hii)!=len(hio[0])):
                fail('History length does not reflect')
            for i in range(len(hii)):
                if (hii[i]!=hio[0][i]):
                    fail('History fields do not reflect')
            #
            ok = myim.setmiscinfo(mii)
            if not ok: fail()
            mio = myim.miscinfo()
            if not mio: fail()
            for f in mii.keys():
                if mio.has_key(f):
                    if (mii[f] != mio[f]):
                        fail('miscinfo field values do not reflect')
                else:
                    fail('miscinfo fields  do not reflect')
            #
            ok = myim.setrestoringbeam(beam=rbi, log=F)
            if not ok: fail()
            rbo = myim.restoringbeam();
            if not rbo: fail()
            for f in rbi.keys():
                if rbo.has_key(f):
                    if (qa.getvalue(rbi[f]) != qa.getvalue(rbo[f])):
                        fail('restoring beam values do not reflect')
                    if (qa.getunit(rbi[f]) != qa.getunit(rbo[f])):
                        fail('restoring beam units do not reflect')
                else:
                    fail('restoring beam fields do not reflect')
            #
            ok = myim.setbrightnessunit(bui)
            if not ok: fail()
            buo = myim.brightnessunit();
            if not buo: fail()
            if (bui != buo):
                fail('brightness units do not reflect')
            # Test convert flux.
            ok = myim.setrestoringbeam(beam=rbi, log=F)
            if not ok: fail()
            ok = myim.setbrightnessunit('Jy/beam')
            if not ok: fail()
            # First a point source
            for mytype in ["gauss","disk"]:
                peakFlux = qa.quantity('1.0 mJy/beam')
                major = rbi['major']
                minor = rbi['minor']
                integralFlux = myim.convertflux(value=peakFlux, major=major,
                                                minor=minor, topeak=F,
                                                type=mytype)
                if not integralFlux: fail()
                peakFlux2 = myim.convertflux(value=integralFlux, major=major,
                                             minor=minor, topeak=T, type=mytype)
                if not peakFlux2: fail()
                #
                d = abs(qa.getvalue(peakFlux)) - abs(1000.0*qa.getvalue(integralFlux))
                if (d > 1e-5):
                    fail('Point source flux conversion reflection 1 failed')
                d = abs(qa.getvalue(peakFlux)) - abs(1000.0*qa.getvalue(peakFlux2))
                if (d > 1e-5):
                    fail('Point source flux conversion reflection 2 failed')
                # Now an extended source
                peakFlux = qa.quantity('1.0 mJy/beam')
                major = qa.quantity("30arcsec")
                minor = qa.quantity("20arcsec")
                integralFlux = myim.convertflux(value=peakFlux, major=major,
                                                minor=minor, topeak=F, type=mytype)
                if not integralFlux: fail()
                peakFlux2 = myim.convertflux(value=integralFlux, major=major,
                                             minor=minor, topeak=T)
                if not peakFlux2: fail()
                #
                d = abs(qa.getvalue(peakFlux)) - abs(1000.0*qa.getvalue(integralFlux))
                if (d > 1e-5):
                    fail('Extended source flux conversion reflection 1 failed')
                d = abs(qa.getvalue(peakFlux)) - abs(1000.0*qa.getvalue(peakFlux2))
                if (d > 1e-5):
                    fail('Extended source flux conversion reflection 2 failed')
            #
        ok = doneAllImageTypes(rec)
        if not ok: fail()
        #
        return cleanup(testdir)
           
    def test34():
        info('')
        info('')
        info('')
        info('Test 34 - ia.fromascii constructor and toascii')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            return False
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        #
        info('Testing toascii')
        info('')
        shape = [2,4,6]
        pixels = ia.makearray(0,shape)
        num = 0
        for i in range(shape[0]):
            for j in range(shape[1]):
                for k in range(shape[2]):
                    num = num+1
                    pixels[i][j][k]=num
        #
        imname = testdir+'/'+'ia.fromarray.image'
        myim = ia.newimagefromarray(pixels=pixels)
        if not myim: fail()
        #
        filename = testdir+'/'+'ia.fromarray.ascii'
        ok = myim.toASCII(outfile=filename)
        if not ok: fail()
        #
        info('Testing ia.fromascii')
        info('')
        #myim2 = ia.fromascii(infile=filename, shape=shape)
        #if not myim2: fail()
        ok = ia.fromascii(infile=filename, shape=shape)
        if not ok: fail()
        #pixels2 = myim2.getchunk()
        pixels2 = ia.getchunk()
        if len(pixels2)==0: fail()
        #
        if not alleq(pixels,pixels2,tolerance=0.0001):
            stop('ia.fromascii reflection failed')
        #
        if not myim.done(): fail()
        #if not myim2.done(): fail()
        if not ia.close(): fail()
        ###
        return cleanup(testdir)

    def test36():
        #
        # Test methods
        #   twopointcorrelation
        #
        # Not very extensive
        #
        info('')
        info('')
        info('')
        info('Test 36 - twopointcorrelation')
        # Make RA/DEC/Spectral image
        imshape = [5,10,20]
        myim = ia.newimagefromshape(shape=imshape)
        if not myim:
            stop('ia.fromshape constructor 1 failed')
        if not myim.set(1.0): fail()
        #
        # Forced failures
        #
        try:
            myim2 = true
            note('Expect SEVERE error and Exception here')
            myim2 = myim.twopointcorrelation(axes=[20])
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            myim2 = false
        if myim2: stop('twopointcorrelation 1 unexpectedly did not fail')
        try:
            myim2 = true
            note('Expect SEVERE error and Exception here')
            myim2 = myim.twopointcorrelation(method='fish')
        except Exception, e:
            note('Caught expected Exception:'+str(e))
            myim2 = false
        if myim2: stop('twopointcorrelation 2 unexpectedly did not fail')
        try:
            myim2 = true
            note('Expect SEVERE error and Exception here')
            myim2 = myim.twopointcorrelation(region='fish')
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            myim2 = false
        if myim2: stop('twopointcorrelation 3 unexpectedly did not fail')
        try:
            myim2 = true
            note('Expect SEVERE error and Exception here')
            myim2 = myim.twopointcorrelation(mask='fish')
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            myim2 = false
        if myim2: stop('twopointcorrelation 4 unexpectedly did not fail')
        #
        # Some simple tests.  Doing it in Glish is way too slow, so
        # just run tests, no value validation
        #
        myim2 = myim.twopointcorrelation()
        if not myim2:
            stop('twopointcorrelation 5 failed')
        # twopointcorrelation now returns boolean rather than new image
        #ok = myim2.done()
        #if not ok: fail()
        #
        myim2 = myim.twopointcorrelation(axes=[0,1])
        if not myim2:
            stop('twopointcorrelation 6 failed')
        #ok = myim2.done()
        #if not ok: fail()
        #
        ok = myim.done()
        if not ok: fail()
        # Make another image only with Linear coordinates
        imshape = [5,10,20]
        myim = ia.newimagefromshape(shape=imshape, linear=T)
        if not myim:
            stop('ia.fromshape constructor 1 failed')
        #
        myim2 = myim.twopointcorrelation(axes=[0,2])
        if not myim2:
            stop('twopointcorrelation 7 failed')
        #ok = myim2.done()
        #if not ok: fail()
        #
        myim2 = myim.twopointcorrelation(axes=[1,2])
        if not myim2:
            stop('twopointcorrelation 8 failed')
        #
        #ok = myim2.done()
        #if not ok: fail()
        #
        ok = myim.done()
        if not ok: fail()
        #
        return True

    def test37():
        #
        # Test methods
        #   continuumsub
        #
        # Not very extensive.  Remove this when this function goes elsewhere.
        #
        info('')
        info('')
        info('')
        info('Test 37 - continuumsub')
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            return False
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        lineOut = testdir + '/line.im'
        contOut = testdir + '/cont.im'

        # Make image with Stokes and Spectral axis

        mycs = cs.newcoordsys(spectral=T, stokes='I')
        if not mycs: fail()
        #
        d = ia.makearray(0,[1,100])
        for i in range(d.size): d[0][i]=i+1
        myim = ia.newimagefromarray(pixels=d, csys=mycs.torecord())
        if not myim:
            stop('ia.fromarray constructor 1 failed')
        if not mycs.done(): fail()
        #
        # Forced failures
        #
        try:
            myim2 = true
            note('Expect SEVERE error and Exception here')
            myim2 = myim.continuumsub(lineOut, contOut, region='fish',
                                      overwrite=T)
        except Exception, e:
            note('Caught expected Exception: '+str(e) )
            myim2 = false
        if myim2:
            stop('continuumsub 1 unexpectedly did not fail')
        try:
            myim2 = true
            note('Expect SEVERE error and Exception here')
            #myim2 = myim.continuumsub(lineOut, contOut, channels='rats',
            #                          overwrite=T)
            myim2 = myim.continuumsub(lineOut, contOut, channels=[-99],
                                      overwrite=T)
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            myim2 = false
        if myim2:
            stop('continuumsub 2 unexpectedly did not fail')
        try:
            myim2 = true
            note('Expect SEVERE error and Exception here')
            #myim2 = myim.continuumsub(lineOut, contOut, pol='DOGGIES',
            #                          overwrite=T)
            myim2 = myim.continuumsub(lineOut, contOut, pol='??',
                                      overwrite=T)
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            myim2 = false
        if myim2:
            stop('continuumsub 3 unexpectedly did not fail')
        try:
            myim2 = true
            note('Expect SEVERE error and Exception here')
            myim2 = myim.continuumsub(lineOut, contOut, fitorder=-2,
                                      overwrite=T)
        except Exception, e:
            note('Caught expected Exception: '+str(e))
            myim2 = false
        if myim2:
            stop('continuumsub 4 unexpectedly did not fail')
        #
        # Some simple run tests.
        #
        myim2 = myim.continuumsub (lineOut, contOut, overwrite=T)

        if not myim2:
            stop('continuumsub 5 failed')
        ok = myim2.done()
        if not ok: fail()
        #
        return cleanup(testdir)

    def test39():
        #
        # Test methods
        #   fitprofile
        #
        info('')
        info('')
        info('')
        info('Test 39 - fitprofile')
        y = [ 
            8.30588e-19,1.16698e-17,1.50353e-16,1.77636e-15,1.92451e-14,
            1.91198e-13,1.74187e-12,1.45519e-11,1.1148e-10,7.83146e-10,
            5.045e-09,2.98023e-08,1.6144e-07,8.01941e-07,3.65297e-06,
            1.52588e-05,5.84475e-05,0.000205297,0.000661258,0.00195312,
            0.00529006,0.013139,0.0299251,0.0625,0.1197,0.210224,0.338564,
            0.5,0.677128,0.840896,0.957603,1,0.957603,0.840896,0.677128,0.5,
            0.338564,0.210224,0.1197,0.0625,0.0299251,0.013139,0.00529006,
            0.00195312,0.000661258,0.000205297,5.84475e-05,1.52588e-05,
            3.65297e-06,8.01941e-07,1.6144e-07,2.98023e-08,5.045e-09,
            7.83146e-10,1.1148e-10,1.45519e-11,1.74187e-12,1.91198e-13,
            1.92451e-14,1.77636e-15,1.50353e-16,1.16698e-17,8.30588e-19,
            5.42101e-20
        ]
        # Make image
        myim = ia.newimagefromarray(pixels=y)
        if not myim:
            stop('ia.fromarray constructor 1 failed')
        testdir = 'imagetest_temp'
        os.mkdir(testdir)
        model = testdir +'/xmodel.im'
        fit = myim.fitprofile(axis=0, model=model)
        myim.close()
        if not fit:
            stop('fitprofile 2 failed')
        #
        tol = 1e-4
        myim.open(model)
        values = myim.getchunk().flatten()
        for i in range(len(y)):
            if not abs(y[i]-values[i]) < tol:
                stop('fitprofile gives wrong values')
        #
        ok = myim.done()
        if not ok: fail()
        return True

    def test40():
        #
        # Test methods
        #   momentsgui, sepconvolvegui, maskhandlergui, view
        #
        info('')
        info('')
        info('')
        info('Test 40 - GUIS: momentsgui, sepconvolvegui, maskhandlergui, view')
        if true:
            info ('No GUI available, cannot test GUI methods')
            #return True
        # Make the directory
        testdir = 'imagetest_temp'
        if not cleanup(testdir):
            return False
        try:
            os.mkdir(testdir)
        except IOError, e:
            note(e, "SEVERE")
            raise RuntimeError, "mkdir " + testdir + " fails!"
        # Make image
        imshape = [100,100]
        pixels = ia.makearray(0.0, imshape)
        for j in range(39,60):
            for i in range(39,60):
                if (i>=44 and j>=44 and i<=54 and j<=54):
                    pixels[i,j] = 10
                else:
                    pixels[i,j] = 5
        imname = testdir+'/'+'ia.fromarray1.image'
        myim = ia.newimagefromarray(outfile=imname, pixels=pixels)
        if not myim:
            stop('ia.fromarray constructor 1 failed')
        #
        ok = true
        info('')
        #info('Testing function view')
        #ok = myim.view(raster=T, contour=T)
        if not ok:
            stop('view failed')
        #
        info('')
        #info('Testing maskhandlerguiview')
        #ok = myim.maskhandlergui()
        if not ok:
            stop('maskhandlergui failed')
        #
        info('')
        #info('Testing momentsgui')
        #ok = myim.momentsgui()
        if not ok:
            stop('momentsgui failed')
        #
        info('')
        #info('Testing sepconvolvegui')
        #ok = myim.sepconvolvegui()
        if not ok:
            stop('sepconvolvegui failed')
        #
        ok = myim.done()
        if not ok:
            stop('Done failed (1)')
        #
        return cleanup(testdir)


    test1()
    test2()
    test3()
    test4()
    test5()
    test6()
    test7()
    test10()
    #test11()
    test12()
    test13()  # segmentation fault on multiple runs
    test14()
    test15()
    test16()
    test17()
    test18()
    test19()
    test20()
    test22()
    test24()
    test25()
    test28()
    test29()
    test30()  # are abs/rel/world/pixel output values correct?
    test32()  # original has commented out bits. Why?
    test33()
    test34()
    test36()
    test37()
    test39()  # update once functionals is available
    test40()  # doesn't do much without gui
    print ''
    print 'Regression PASSED'
    print ''
    
imagedemo()

Benchmarking = True
if Benchmarking:
    startTime = time.time()
    regstate = False
    for i in range(1):  #get seg fault when reran
        imagetest()
    endTime = time.time()
    regstate = True
else:
    imagetest()

print "End of imagetest.py"
#exit()
