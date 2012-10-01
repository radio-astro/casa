import os
import shutil
import numpy
from __main__ import default
from tasks import *
from taskinit import *
import unittest

IMAGE = 'image.im'

total = 0
fail  = 0
current_test =""
stars = "*************"

datapath = os.environ.get('CASAPATH').split()[0]+'/data/regression/unittest/imregrid/'

def alleqnum(x,num,tolerance=0):
    if len(x.shape)==1:
        for i in range(x.shape[0]):
            if not (abs(x[i]-num) < tolerance):
                print "x[",i,"]=", x[i]
                return False
    if len(x.shape)==2:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                if not (abs(x[i][j]-num) < tolerance):
                    print "x[",i,"][",j,"]=", x[i][j]
                    return False
    if len(x.shape)==3:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    if not (abs(x[i][j][k]-num) < tolerance):
                        print "x[",i,"][",j,"][",k,"]=", x[i][j][k]
                        return False
    if len(x.shape)==4:
        for i in range(x.shape[0]):
            for j in range(x.shape[1]):
                for k in range(x.shape[2]):
                    for l in range(x.shape[3]):
                        if not (abs(x[i][j][k][l]-num) < tolerance):
                            print "x[",i,"][",j,"][",k,"][",l,"]=", x[i][j][k]
                            return False
    if len(x.shape)>4:
        stop('unhandled array shape in alleq')
    return True

def test_start(msg):
    global total, current_test
    total += 1
    print
    print stars + " Test " + msg + " start " + stars
    current_test = msg
    
def test_end(condition, error_msg):
    global total, fail
    status = "OK"
    if not condition:
        print >> sys.stderr, error_msg
        fail += 1
        status = "FAIL"
    print stars + " Test " + current_test + " " + status + " " + stars
        
out1 = 'regridded'
out2 = 'bigger_image'
out3 = 'shifted_image'
out4 = 'back_to_image'
out5 = 'template'

class imregrid_test(unittest.TestCase):

    def setUp(self):
        self._myia = iatool()
    
    def tearDown(self):
        self._myia.done()

        os.system('rm -rf ' +IMAGE)
        os.system('rm -rf ' +out1)
        os.system('rm -rf ' +out2)
        os.system('rm -rf ' +out3)
        os.system('rm -rf ' +out4)
        os.system('rm -rf ' +out5)

        
    def test1(self):    
        myia = self._myia  
        myia.maketestimage(outfile = IMAGE)
        default('imregrid')
        
        outim=imregrid(
            imagename = IMAGE,
            template = IMAGE,
            output = out1
        )
        im1 = myia.newimage(IMAGE)
        im2 = myia.newimage(out1)
        
        im1.statistics()
        im2.statistics()
        
        rec1 = im1.torecord()
        print '*************'
        print rec1['shape']
        print '*************'
        shape = im1.shape()
        print shape
        checked = 0
        for x in range(shape[0]):
            for y in range(shape[1]):
                p1 = im1.pixelvalue([x, y])
                p2 = im2.pixelvalue([x, y])
                if p1['mask'] != p2['mask']: raise Exception, p1['mask'] + ' != ' + p2['mask']
                if p1['value']['value'] != p2['value']['value']: raise Exception, p1['value']['value'] + ' != ' + p2['value']['value']
                if p1['value']['unit'] != p2['value']['unit']: raise Exception, p1['value']['unit'] + ' != ' + p2['value']['unit']
                checked += 3
        
        im2.done()
        
        print str(checked) + ' values checked'
        
        # rescale by factors 3 x 2
        rec1 = im1.torecord()
        print '*************'
        print "shape before " + str(rec1['shape'])
        print '*************'
        rec1['shape'] = numpy.array([3*rec1['shape'][0], 2*rec1['shape'][1]], numpy.int32)
        print "shape after " + str(rec1['shape'])

        rec1['coordsys']['coordsys']['direction0']['cdelt'] = [
            rec1['coordsys']['coordsys']['direction0']['cdelt'][0]/3.0,
            rec1['coordsys']['coordsys']['direction0']['cdelt'][1]/2.0]
        rec1['coordsys']['coordsys']['direction0']['crpix'] = [
            rec1['coordsys']['coordsys']['direction0']['crpix'][0]*3.0,
            rec1['coordsys']['coordsys']['direction0']['crpix'][1]*2.0]
        print rec1
        
        myia.fromrecord(rec1, out2)
        
        # First we need to remove the output file.
        if (  os.path.exists(out1) ):
              shutil.rmtree( out1 )
        outim=imregrid(
            imagename=IMAGE, template=out2,
            output=out1, shape=rec1["shape"]
        )
        s1 = imstat(IMAGE)
        s2 = imstat(out1)
        ia.open(out1)
        print "out shape " + str(ia.shape())
        print "S1: ", s1
        print " "
        print " "
        print "S2: ", s2
        
        if s1['maxpos'][0]*3 != s2['maxpos'][0]:
            raise Exception, str(s1['maxpos'][0]*3) + ' != ' + str(s2['maxpos'][0])
        if s1['maxpos'][1]*2 != s2['maxpos'][1]:
            raise Exception, str(s1['maxpos'][1]*2) + ' != ' + str(s2['maxpos'][1])
        
        
        
        # shift by -13, 1 pixels

        rec1 = im1.torecord()
        rec1['coordsys']['coordsys']['direction0']['crpix'] = [
            rec1['coordsys']['coordsys']['direction0']['crpix'][0]-13,
            rec1['coordsys']['coordsys']['direction0']['crpix'][1]+1]
        
        myia.fromrecord(rec1, out3)
        myia.close()
        # First we need to remove the output file.
        if (  os.path.exists(out1 ) ):
              shutil.rmtree( out1)
        outim = imregrid(imagename = IMAGE,
                 template = out3,
                 output = out1)
        s1 = imstat(IMAGE)
        s2 = imstat(out1)
        if s1['maxpos'][0]-13 != s2['maxpos'][0]:
            raise Exception, str(s1['maxpos'][0]-13) + ' != ' + str(s2['maxpos'][0])
        if s1['maxpos'][1]+1 != s2['maxpos'][1]:
            raise Exception, str(s1['maxpos'][1]+1) + ' != ' + str(s2['maxpos'][1])
        
        
        # Shift back to original
        rec1['coordsys']['coordsys']['direction0']['crpix'] = [
            rec1['coordsys']['coordsys']['direction0']['crpix'][0]+13,
            rec1['coordsys']['coordsys']['direction0']['crpix'][1]-1]
        if (  os.path.exists(out3 ) ):
            shutil.rmtree( out3)
        myia.fromrecord(rec1, out3)
        myia.close()
        outim = imregrid(imagename = IMAGE,
                 template = out3,
                 output = out4)
        s1 = imstat(IMAGE)
        s2 = imstat(out4)
        print s1
        print s2
        for stat in ['rms', 'medabsdevmed', 'minpos',
                     'min', 'max', 'sum', 'minposf',
                     'median', 'flux', 'sumsq', 'maxposf',
                     'trcf', 'quartile', 'npts', 'maxpos',
                     'mean', 'sigma', 'trc', 'blc', 'blcf']:
            if type(s1[stat]) == type('a string'):
                print "Checking string", stat, s1[stat]
                if s1[stat] != s2[stat]:
                    raise Exception
            else:
                for i in range(len(s1[stat])):
                    print "Checking", stat, "[", i, "]", s1[stat][i]
                    if s1[stat][i] != s2[stat][i]:
                        # Note:  == comparison of floating point values,
                        # it works right now on this computer but might need to get fixed...
                        raise Exception
        
        
        # Exercise various reference codes (no check on output)
        codes = cs.newcoordsys(direction=True).referencecode('dir', True)
        rec1 = im1.torecord()
        im1.done()
        for ref in codes:
            print "Regrid to", ref
            if ref not in ['JMEAN', 'JTRUE', 'APP',
                           'BMEAN', 'BTRUE', 'HADEC',
                           'AZEL', 'AZELSW', 'AZELNE',
                           'AZELGEO',
                           'AZELSWGEO', 'AZELNEGEO',
                           'JNAT',
                           'MECLIPTIC', 'TECLIPTIC',
                           'ITRF', 'TOPO']:
                rec1['coordsys']['coordsys']['direction0']['conversionSystem'] = ref
                
                myia.fromrecord(rec1, out5)
                myia.close()
                if (  os.path.exists(out1 ) ):
                    shutil.rmtree( out1 )
                outim=imregrid(imagename = IMAGE,
                         template = out5,
                         output = out1)
        self.assertTrue(len(tb.showcache()) == 0)

    def test_asvelocity(self):
        """ Test regrid by velocity """
        image = "byvel.im"
        expected = "expected.im"
        shutil.copytree(datapath + image, image)
        shutil.copytree(datapath + expected, expected)
        myia = self._myia
        myia.open(expected)
        csys = myia.coordsys().torecord()
        myia.done()
        myia.open(image)
        ff = myia.regrid("",csys=csys,asvelocity=True)
        myia.done()
        myia.open(expected)
        res = (ff.getchunk() == myia.getchunk()).all()
        self.assertTrue(res)
        res = (ff.getchunk(getmask=True) == myia.getchunk(getmask=True)).all()
        self.assertTrue(res)
        ff.done()
        outfile = "junk"
        outim = myia.regrid(outfile=outfile, csys=csys, asvelocity=True)
        outim.done()
        ff.open(outfile)
        res = (ff.getchunk() == myia.getchunk()).all()
        self.assertTrue(res)
        res = (ff.getchunk(getmask=True) == myia.getchunk(getmask=True)).all()
        ff.done()
        myia.done()
        self.assertTrue(res)  
        shutil.rmtree(outfile)
        shutil.rmtree(image)
        shutil.rmtree(expected)      
        self.assertTrue(len(tb.showcache()) == 0)        

    def test_stretch(self):
        """ ia.regrid(): Test stretch parameter"""
        yy = self._myia
        mymask = "maskim"
        yy.fromshape(mymask, [200, 200, 1, 1])
        yy.addnoise()
        yy.done()
        shape = [200,200,1,20]
        yy.fromshape("", shape)
        yy.addnoise()
        mycsys = yy.coordsys()
        mycsys.setreferencepixel([2.5], "spectral")
        for i in [0,1]:
            byvel = i == 0
            self.assertRaises(
                Exception,
                yy.regrid, outfile="", asvelocity=byvel,
                csys=mycsys.torecord(),
                mask=mymask + ">0", stretch=False
            )
            zz = yy.regrid(
                outfile="", asvelocity=byvel,
                csys=mycsys.torecord(),
                mask=mymask + ">0", stretch=True
            )
            self.assertTrue(type(zz) == type(yy))
            zz.done()
            
        yy.done()
        self.assertTrue(len(tb.showcache()) == 0)
        
    def test_axes(self):
        imagename = "test_axes.im"
        templatename = "test_axes.tmp"
        output = "test_axes.out"
        myia = self._myia
        myia.fromshape(imagename, [10, 10, 10])
        exp = myia.coordsys().increment()["numeric"]
        myia.fromshape(templatename, [10, 10, 10])
        mycsys = myia.coordsys()
        mycsys.setincrement(mycsys.increment()["numeric"]/2)
        myia.setcoordsys(mycsys.torecord())
        exp[2] = mycsys.increment()["numeric"][2]
        zz = imregrid(imagename, template=templatename, output=output, axes=2)
        myia.open(output)
        got = myia.coordsys().increment()["numeric"]
        self.assertTrue((got == exp).all())
        myia.done()
        self.assertTrue(len(tb.showcache()) == 0)
        
    def test_general(self):
        """ imregrid general tests """
        # moved from iamgetest_regression
        
        # Make RA/DEC/Spectral image
        
        imname = 'ia.fromshape.image1'
        imshape = [32,32,32]
        myia = self._myia
        myim = myia.newimagefromshape(imname, imshape)
        self.assertTrue(myim)
        self.assertTrue(myim.set(1.0))
        # Forced failures
        self.assertRaises(Exception, myim.regrid, axes=[20])
        self.assertRaises(Exception, myim.regrid, shape=[10,20,30,40])       
        self.assertRaises(Exception, myim.regrid, csys='fish')
        self.assertRaises(Exception, myim.regrid, method='doggies')

        # Regrid it to itself (all axes        #
        iDone = 1
        #      for method in ["near","linear","cubic"]:
        for method in ["cubic"]:
            myim2 = myim.regrid(method=method)
            self.assertTrue(myim2)
            p = myim2.getchunk([3,3],[imshape[0]-3,imshape[1]-3,imshape[2]-3])
            self.assertTrue(alleqnum(p,1,tolerance=1e-3))
            self.assertTrue(myim2.done())
            iDone = iDone + 1
            
        #      for method in ["cubic","linear","near"]:
        for method in ["cubic"]:
            myim2 = myim.regrid(method=method, axes=[0,1])
            self.assertTrue(myim2)
            p = myim2.getchunk([3,3],[imshape[0]-3,imshape[1]-3,imshape[2]-3])
            self.assertTrue(alleqnum(p,1,tolerance=1e-3))
            self.assertTrue(myim2.done())
            iDone = iDone + 1

        #      for method in ["near","linear","cubic"]:
        for method in ["cubic"]:
            myim2 = myim.regrid(method=method, axes=[2])
            self.assertTrue(myim2)
            p = myim2.getchunk([3,3],[imshape[0]-3,imshape[1]-3,imshape[2]-3])
            self.assertTrue(alleqnum(p,1,tolerance=1e-3))
            self.assertTrue(myim2.done())
            iDone = iDone + 1
        #
        self.assertTrue(myim.done())
        self.assertTrue(len(tb.showcache()) == 0)        

    def test_multibeam(self):
        """imregrid, test multibeam image"""
        myia = self._myia
        myia.fromshape("", [10, 10, 10])
        csys = myia.coordsys()
        refpix = csys.increment()["numeric"][2]
        refpix = refpix * 0.9
        csys.setincrement(refpix, "spectral")
        
        myia.setrestoringbeam(major="4arcsec", minor="2arcsec", pa="0deg", channel=0, polarization=-1)
        regridded = myia.regrid(axes=[0, 1], csys=csys.torecord())
        regridded.done()
        self.assertRaises(Exception, myia.regrid, axes=[0,1,2], csys=csys.torecord())
        self.assertTrue(len(tb.showcache()) == 0)
        
    def test_CAS_4315(self):
        """ test ia.regrid does not leave image open after tool is closed"""
        myia = self._myia
        myia.fromshape("",[100,100,1,1])
        myib = myia.regrid(
            outfile='moulou1', csys=myia.coordsys().torecord(), axes=[0,1],
            overwrite=True, shape=[100, 100, 1, 1]
        )
        myia.done()
        myib.done()
        self.assertTrue(len(tb.showcache()) == 0)
        
    def test_CAS_4262(self):
        """ Test degenerate axes are not relabeled to template"""
        myia = self._myia
        myia.fromshape("", [10, 10, 1, 10])
        csys = myia.coordsys()
        refvals = csys.referencevalue()["numeric"]
        refvals[3] *= 10
        csys.setreferencevalue(refvals)
        regridded = myia.regrid("", myia.shape(), csys.torecord())
        self.assertTrue((regridded.getchunk(getmask=True) == False).all())
        self.assertTrue(
            (
             regridded.coordsys().referencevalue()["numeric"] == refvals
            ).all()
        )
        # test degenerate spectral exis is not regridded nor relabeled in output
        myia.fromshape("", [10, 10, 1, 1])
        csys = myia.coordsys()
        refvals = csys.referencevalue()["numeric"]
        refvals[3] *= 10
        csys.setreferencevalue(refvals)
        regridded = myia.regrid("", myia.shape(), csys.torecord())
        self.assertTrue(regridded.getchunk(getmask=True).all())
        self.assertTrue(
            (
             regridded.coordsys().referencevalue()["numeric"]
             == myia.coordsys().referencevalue()["numeric"]
            ).all()
        )
        
        
            
def suite():
    return [imregrid_test]
    
    
