import os
import shutil
import numpy
from __main__ import default
from tasks import *
from taskinit import *
import unittest

IMAGE = 'image.im'
gim = "gaussian_source.im"

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
out6 = 'gal_coords.im'

class imregrid_test(unittest.TestCase):

    def setUp(self):
        self._myia = iatool()
    
    def tearDown(self):
        self._myia.done()
        
        for i in (IMAGE, out1, out2, out3, out4, out5, out6):
            if (os.path.exists(i)):
                os.system('rm -rf ' + i)
        
        self.assertTrue(len(tb.showcache()) == 0)
        
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
        # test degenerate spectral axis is not regridded nor relabeled in output
        myia = self._myia
        myia.fromshape("", [10, 10, 1, 1])
        csys = myia.coordsys()
        refvals = csys.referencevalue()["numeric"]
        refvals[3] *= 10
        csys.setreferencevalue(refvals)
        regridded = myia.regrid("", myia.shape(), csys.torecord(), asvelocity=False)
        self.assertTrue(regridded.getchunk(getmask=True).all())
        self.assertTrue(
            (
             regridded.coordsys().referencevalue()["numeric"]
             == myia.coordsys().referencevalue()["numeric"]
            ).all()
        )
        
    def test_ref_code(self):
        """Test regridding to a new reference frame"""
        imregrid(imagename=datapath + "myim.im", template="GALACTIC", output=out6)
        myia = self._myia
        myia.open(out6)
        got = myia.getchunk()
        myia.open(datapath + "mygal.im")
        expec = myia.getchunk()
        self.assertTrue((got == expec).all())
        
    def test_ref_code_preserves_position(self):
        """Test that regridding to new refcode preserves source positions"""
        gal = "mygalactic.im"
        imregrid(datapath+gim,template="GALACTIC", output=gal)
        orig = iatool()
        orig.open(datapath+gim)
        ofit = orig.fitcomponents(box="850,150,950,250")
        orig.done()
        ocl = cltool()
        ocl.add(ofit['results']['component0'])
        orefdir = ocl.getrefdir(0)
        galtool = iatool()
        galtool.open(gal)
        gfit = galtool.fitcomponents(box="1120,520,1170,570")
        galtool.done()
        gcl = cltool()
        gcl.add(gfit['results']['component0'])
        grefdir = gcl.getrefdir(0)
        myme = metool()
        self.assertTrue(qa.getvalue(qa.convert(myme.separation(orefdir, grefdir), "arcsec")) < 0.003)
        rev = "back_to_J2000.im"
        imregrid(gal,template="J2000", output=rev)
        revtool = iatool()
        revtool.open(rev)
        rfit = revtool.fitcomponents(box="850,150,950,250")
        revtool.done()
        rcl = cltool()
        rcl.add(rfit['results']['component0'])
        rrefdir = rcl.getrefdir(0)
        self.assertTrue(qa.getvalue(qa.convert(myme.separation(orefdir, rrefdir), "arcsec")) < 2e-4)
        
    def test_get(self):
        """Test using template='get' works"""
        tempfile = "xyz.im"
        myia = self._myia 
        myia.fromshape(tempfile,[20,20,20])
        dicttemp = imregrid(tempfile, template="get")
        dicttemp['csys']['direction0']['crpix'] = [2.5, 2.5]
        output = "out.im"
        imregrid (tempfile, template=dicttemp, output=output)
        
    def test_interpolate(self):
        """Test interpolation parameter is recognized"""
        imagename = "zzx.im"
        myia = self._myia
        myia.fromshape(imagename, [30, 30])
        csys = myia.coordsys()
        incr = csys.increment()['numeric']
        incr[0] = incr[0]*0.9
        incr[1] = incr[1]*0.9
        csys.setincrement(incr)
        template = {}
        template['csys'] = csys.torecord()
        template['shap'] = myia.shape()
        myia.done()
        self.assertFalse(
            imregrid(
                imagename=imagename, template=template,
                output="blah", interpolation="x"
            )
        )
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output="blah3", interpolation="cubic"
            )
        )
        
    def test_default_shape(self):
        """ Verify default shape is what users have requested, CAS-4959"""
        myia = self._myia
        imagename = "myim.im"
        myia.fromshape(imagename,[20,20,20])
        template = "mytemp.im"
        myia.fromshape(template,[10,10,10])
        csys = myia.coordsys()
        csys.setreferencepixel([5,5,5])
        myia.setcoordsys(csys.torecord())
        myia.done()
        output = "cas_4959_0"
        imregrid(
            imagename=imagename, template=template,
            output=output
        )
        myia.open(output)
        self.assertTrue((myia.shape() == [10, 10, 10]).all())
        output = "CAS_4959_1"
        imregrid(
            imagename=imagename, template=template,
            output=output, axes=[0,1]
        )
        myia.open(output)
        self.assertTrue((myia.shape() == [10, 10, 20]).all())
        output = "CAS_4959_2"
        imregrid(
            imagename=imagename, template=template,
            output=output, axes=[2]
        )
        myia.open(output)
        self.assertTrue((myia.shape() == [20, 20, 10]).all())
        
    def test_axis_recognition(self):
        """Test that imregrid recognizes axis by type, not position"""
        myia = self._myia
        target = "target.im"
        myia.fromshape(target, [4,4,2,30])
        template = "template.im"
        myia.fromshape(template, [6, 6, 36, 2])
        outfile = "myout.im"
        self.assertTrue(imregrid(imagename=target, template=template, output=outfile))
        myia.open(outfile)
        self.assertTrue((myia.shape() == [6, 6, 2, 36]).all())
        myia.done()
        outfile = "myout1.im"
        self.assertTrue(imregrid(imagename=target, template=template, output=outfile, axes=[0, 1]))
        myia.open(outfile)
        self.assertTrue((myia.shape() == [6, 6, 2, 30]).all())
        myia.done()

    def test_overlap(self):
        """Test for notification if no overlap between input and output images"""
        myia = self._myia
        myia.fromshape("", [20, 20, 20, 4])
        csys = myia.coordsys()
        csys.setreferencevalue([1800, 0], 'direction')
        myia.setcoordsys(csys.torecord())

        ccopy = csys.copy()
        xx = myia.regrid(outfile="first",csys=ccopy.torecord())
        self.assertTrue(xx)
        xx.done()

        ccopy.setreferencevalue([1890, 0], 'direction')
        self.assertRaises(Exception, myia.regrid, "second",csys=ccopy.torecord())
        xx = myia.regrid("forth",csys=ccopy.torecord(), axes=2)
        self.assertTrue(xx)
        xx.done()
        myia.fromshape("", [200, 200, 20, 4], csys=csys.torecord())
        xx = myia.regrid(outfile="third",csys=ccopy.torecord())
        self.assertTrue(xx)
        xx.done()
        ccopy.setreferencevalue(1.416e9, 'spectral')
        self.assertRaises(Exception, myia.regrid, "fifth",csys=ccopy.torecord())
        myia.fromshape("", [20, 20, 1001, 4], csys=csys.torecord())
        xx = myia.regrid(outfile="sixth",csys=ccopy.torecord(), axes=2)
        self.assertTrue(xx)
        xx.done()
        self.assertRaises(
            Exception,myia.regrid, outfile="seventh",csys=ccopy.torecord(),
            axes=2, region=rg.box([0,0,0,0],[19,19,998,3])
        )
        
    def test_no_output_stokes(self):
        """Test rule that if input image has no stokes and template image has stokes, output image has no stokes"""
        myia = self._myia
        imagename = "aa.im"
        myia.fromshape(imagename, [20, 20, 20])
        template = "aa_temp.im"
        myia.fromshape(template, [20, 20, 2, 20])
        csys = myia.coordsys()
        csys.setincrement([-0.9, 0.9, 1, 1500])
        myia.setcoordsys(csys.torecord())
        myia.done()
        output = "aa.out.im"
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5
            )
        )
        myia.open(output)
        self.assertFalse(myia.coordsys().findcoordinate("stokes")[0])
        myia.done()
        
    def test_no_template_stokes(self):
        """Test rule that if input image has stokes and template image does not have stokes, output image has stokes"""
        myia = self._myia
        imagename = "ab.im"
        myia.fromshape(imagename, [20, 20, 2, 20])
        template = "ab_temp.im"
        myia.fromshape(template, [20, 20, 20])
        csys = myia.coordsys()
        csys.setincrement([-0.9, 0.9, 1, 1500])
        myia.setcoordsys(csys.torecord())
        myia.done()
        output = "ab.out.im"
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5
            )
        )
        myia.open(output)
        stokes_info = myia.coordsys().findcoordinate("stokes")
        self.assertTrue(stokes_info[0])
        exp_axis = 2
        self.assertTrue(stokes_info[1][0] == exp_axis)
        self.assertTrue(myia.shape()[exp_axis] == 2)
        self.assertTrue(myia.coordsys().stokes() == ['I','Q'])
        myia.done()
        # specifying an output stokes length other than the input stokes length
        # is not allowed
        self.assertFalse(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True,
                shape=[20, 20, 1, 20]
            )
        )
        self.assertFalse(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True,
                shape=[20, 20, 3, 20]
            )
        )
        # specifying an output stokes length other than the input stokes length
        # is allowed
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True,
                shape=[20, 20, 2, 20]
            )
        )
        
    def test_degenerate_template_stokes_axis_and_input_stokes_length_gt_0(self):
        """Verify correct behavior for the template image having a degenerate stokes axis"""
        myia = self._myia
        imagename = "ac.im"
        myia.fromshape(imagename, [20, 20, 2, 20])
        template = "ac_temp.im"
        myia.fromshape(template, [20, 20, 1, 20])
        csys = myia.coordsys()
        csys.setincrement([-0.9, 0.9, 1, 1500])
        myia.setcoordsys(csys.torecord())
        myia.done()
        output = "ac.out.im"
        # all input stokes in output if shape and axes not specified
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True
            )
        )
        myia.open(output)
        self.assertTrue(myia.shape()[2] == 2)
        myia.done()
        # not allowed if output stokes length different from input stokes length
        self.assertFalse(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, shape=[20, 20, 1, 20],
                overwrite=True
            )
        )
        self.assertFalse(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, shape=[20, 20, 3, 20],
                overwrite=True
            )
        )
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, shape=[20, 20, 2, 20],
                overwrite=True
            )
        )
        
    def test_template_stokes_length_gt_1_and_input_stokes_length_gt_0(self):
        """Verify correct behavior for the template image having a stokes axis of length > 1"""
        myia = self._myia
        imagename = "ad.im"
        myia.fromshape(imagename, [20, 20, 4, 20])
        template = "ad_temp.im"
        myia.fromshape(template, [20, 20, 4, 20])
        csys = myia.coordsys()
        csys.setincrement([-0.9, 0.9, 1, 1500])
        myia.setcoordsys(csys.torecord())
        myia.done()
        output = "ad.out.im"
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True
            )
        )
        myia.open(template)
        csys = myia.coordsys()
        csys.setstokes('XX RL LR YY')
        myia.setcoordsys(csys.torecord())
        myia.done()
        # no match between input and template stokes => not allowed
        self.assertFalse(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True
            )
        )
        csys.setstokes("XX I LL RR")
        myia.open(template)
        myia.setcoordsys(csys.torecord())
        myia.done()
        # specified output stokes axis length != number of common stokes => not allowed
        self.assertFalse(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True,
                shape=[20, 20, 3, 20]
            )
        )
        # no output shape and number of common stokes > 0 => allowed
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True
            )
        )
        myia.open(output)
        expec = ["I"]
        self.assertTrue(myia.coordsys().stokes() == expec)
        myia.done()
        
        csys.setstokes("XX I U RR")
        myia.open(template)
        myia.setcoordsys(csys.torecord())

        myia.done()
        # no output shape and number of common stokes > 0 => allowed
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True
            )
        )
        myia.open(output)
        expec = ["I", "U"]
        self.assertTrue(myia.coordsys().stokes() == expec)
        self.assertTrue((myia.shape() == [20, 20, 2, 20]).all())
        myia.done()
    
    def test_no_input_spectral(self):
        """Verify if input image has no spectral axis, output will not have spectral axis"""
        myia = self._myia
        imagename = "ae.im"
        myia.fromshape(imagename, [20, 20, 4])
        template = "ae_temp.im"
        myia.fromshape(template, [20, 20, 4, 20])
        csys = myia.coordsys()
        csys.setincrement([-0.9, 0.9, 1, 1500])
        myia.setcoordsys(csys.torecord())
        myia.done()
        output = "ae.out.im"
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True
            )
        )
        myia.open(output)
        self.assertTrue((myia.shape() == [20, 20, 4]).all())
        myia.done()
        
    def test_no_template_spectral_axis(self):
        """Verify behavior for when template has no spectral axis, but input does"""
        myia = self._myia
        imagename = "af.im"
        myia.fromshape(imagename, [20, 20, 4, 20])
        template = "af_temp.im"
        myia.fromshape(template, [20, 20, 4])
        csys = myia.coordsys()
        csys.setincrement([-0.9, 0.9, 1])
        myia.setcoordsys(csys.torecord())
        myia.done()
        output = "af.out.im"
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True
            )
        )
        myia.open(output)
        self.assertTrue((myia.shape() == [20, 20, 4, 20]).all())
        myia.done()
        # Cannot explicitly specify to regrid spectral axis if template has no such axis
        self.assertFalse(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True,
                axes=[0, 1, 3]
            )
        )
        
    def test_degenerate_template_spectral_axis(self):
        """Verify correct behavior for when template has a degenerate spectral axis"""
        myia = self._myia
        imagename = "ag.im"
        myia.fromshape(imagename, [20, 20, 4, 20])
        template = "ag_temp.im"
        myia.fromshape(template, [20, 20, 4, 1])
        csys = myia.coordsys()
        csys.setincrement([-0.9, 0.9, 1, 900])
        myia.setcoordsys(csys.torecord())
        myia.done()
        output = "ag.out.im"
        # input spectral axis copied to output
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True
            )
        )
        myia.open(output)
        self.assertTrue((myia.shape() == [20, 20, 4, 20]).all())
    
        # the spectral axis is removed from the list of axes, a warning is emitted
        # that it cannot be regridded, and the input spectral axis is copied to
        # the ouptut image
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True,
                axes=[0, 1, 3]
            )
        )
        myia.open(output)
        self.assertTrue((myia.shape() == [20, 20, 4, 20]).all())
        myia.done()
    
    def test_degenerate_input_spectral_axis(self):
        """Verify correct behavior for when input has a degenerate spectral axis"""
        myia = self._myia
        imagename = "ah.im"
        myia.fromshape(imagename, [20, 20, 4, 1])
        template = "ah_temp.im"
        myia.fromshape(template, [20, 20, 4, 20])
        csys = myia.coordsys()
        csys.setincrement([-0.9, 0.9, 1, 900])
        myia.setcoordsys(csys.torecord())
        myia.done()
        output = "ah.out.im"
        # when spectral axis not specified, input spectral axis is copied to
        # output spectral axis
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True
            )
        )
        myia.open(output)
        self.assertTrue((myia.shape() == [20, 20, 4, 1]).all())
        # if explicitly specified in the axis parameter, the template spectral
        # axis is copied to the output and the output's spectral axis length as
        # the same as the template's spectral axis length. The output pixel values
        # are replicated from the input image, all spectral hyperplanes in the output
        # will have identical pixel value arrays.
        self.assertTrue(
            imregrid(
                imagename=imagename, template=template,
                output=output, decimate=5, overwrite=True,
                axes=[0, 1, 3]
            )
        )
        myia.open(output)
        self.assertTrue((myia.shape() == [20, 20, 4, 20]).all())
        got = myia.coordsys().increment()['numeric']
        expec = csys.increment()['numeric']
        self.assertTrue((got == expec).all())
        myia.done()
    
    def test_bad_shape(self):
        """ Verify that bad shape specification results in exception"""
        myia = self._myia
        imagename = "aj.im"
        myia.fromshape(imagename, [20, 20, 1, 1])
        template = "aj_temp.im"
        myia.fromshape(template, [20, 20, 1, 20])
        csys = myia.coordsys()
        csys.setincrement([-0.9, 0.9, 1, 900])
        myia.setcoordsys(csys.torecord())
        myia.done()
        output = "aj.out.im"
        self.assertFalse(
            imregrid(
                imagename=imagename,
                template=template, output=output, decimate=5,
                overwrite=True, shape=[20, 20, 20, 1]
            )
        )
    
    def test_nested_image(self):
        """ Verify that one image which lies completely inside the other will not cause failure"""
        myia = self._myia
        imagename = "ak.im"
        myia.fromshape(imagename, [20, 20])
        csys = myia.coordsys()
        csys.setreferencevalue([1800, 1800])
        myia.setcoordsys(csys.torecord())
        template = "ak_temp.im"
        myia.fromshape(template, [4, 4])
        csys = myia.coordsys()
        csys.setreferencevalue([1800, 1800])
        csys.setincrement([-0.9, 0.9])
        myia.setcoordsys(csys.torecord())
        myia.done()
        output = "ak.out.im"
        self.assertTrue(
            imregrid(
                imagename=imagename,
                template=template, output=output, decimate=5,
                overwrite=True
            )
        )
        self.assertTrue(
            imregrid(
                imagename=template,
                template=imagename, output=output, decimate=5,
                overwrite=True
            )
        )
    
    def test_regrid_galactic(self):
        """Verify fix for CAS-5534"""
        myia = self._myia
        myia.open(datapath + "ngc5921.clean.image")
        csys = myia.coordsys()
        csys.setreferencecode('GALACTIC', type='direction', adjust=True)
        zz = myia.regrid(outfile='gal_regrid.image', shape=[300, 300, 1, 46], csys=csys.torecord(), overwrite=True)  
        myia.open(datapath + "gal_regrid.image")
        self.assertTrue(numpy.max(numpy.abs(zz.getchunk() - myia.getchunk())) < 1e-8)
        self.assertTrue((zz.getchunk(getmask=True) == myia.getchunk(getmask=True)).all())
        myia.done()
        zz.done()

        
def suite():
    return [imregrid_test]
    
    
