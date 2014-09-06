import shutil
import unittest
import os
import numpy
from recipes.pixelmask2cleanmask import *
from tasks import *
from taskinit import *
from __main__ import default
"""
Unit tests for task makemask 

"""

datapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/makemask/'
#datapath = '../localtest/'

#debug=True
debug=False


class makemaskTestBase(unittest.TestCase):
    """ base class for makemask unit test"""
    def compareimpix(self,refimage,inimage):
	"""
	do pixel by pixel comparison of the cube image
	returns true if identical in terms of pixel values
	and shape (does not check coordinates)
	"""
	ia.open(refimage)
	refdata=ia.getchunk()
	refshp=list(ia.shape())
	ia.close()
	ia.open(inimage)
	indata=ia.getchunk()
	inshp=list(ia.shape())
	ia.close()
	if inshp==refshp:
	    for i in range(inshp[3]):
		for j in range(inshp[2]):
		    for k in range(inshp[1]):
			for l in range(inshp[0]):
			    if not indata[l][k][j][i]==refdata[l][k][j][i]:
				return False
	return True

 
class test_copy(makemaskTestBase):
    """test copy mode"""

    #data in repository
    inimage='ngc5921.cube1.mask'
    inimage2='ngc5921.cube2.mask'

    outimage1='ngc5921.cube1.copy.mask'
    outimage2='ngc5921.cube1.copyinmage.mask'
    outimage3='ngc5921.cube2.copyinmage.mask'

    def setUp(self):
        #for img in [self.inimage,self.outimage1,self.outimage2, self.outimage3]:
        #    if os.path.isdir(img):
        #        shutil.rmtree(img)
        for img in [self.inimage,self.inimage2]:
            if not os.path.isdir(img):
                shutil.copytree(datapath+img,img)

    def tearDown(self):
        if not debug:
            for img in [self.inimage,self.inimage2,self.outimage1,self.outimage2,self.outimage3]:
                #pass
                if os.path.isdir(img):
                    shutil.rmtree(img)
        else:
            print "debugging mode: clean-up did not performed" 
        
    def test1_copyimagemask(self):
        """ (copy mode) testcopy1: copying an image mask (1/0 mask) to a new image mask"""
        try:
            makemask(mode='copy',inpimage=self.inimage,inpmask=self.inimage,output=self.outimage1)
        except Exception, e:
            print "\nError running makemask"
            raise e
        
        self.assertTrue(os.path.exists(self.outimage1))           
        self.assertTrue(self.compareimpix(self.inimage,self.outimage1))           
         
    def test2_copyimagemask(self):
        """ (copy mode) testcopy2: copying an image mask (1/0 mask) to a new in-image(T/F) mask"""
        try:
            makemask(mode='copy',inpimage=self.inimage,inpmask=self.inimage,output=self.outimage2+":masknew")
        except Exception, e:
            print "\nError running makemask"
            raise e
         
        self.assertTrue(os.path.exists(self.outimage2))
        # check by converting the output back to a 1/0 mask to see if it 
        # is indentical to the input 1/0 mask.
        if os.path.exists(self.outimage2):
            ia.open(self.outimage2)
            masknames=ia.maskhandler('get')
            if masknames.count('masknew')==1:
                pixelmask2cleanmask(self.outimage2,'masknew','_tmp_im',False)
                self.assertTrue(self.compareimpix(self.inimage,'_tmp_im')) 
                shutil.rmtree('_tmp_im')
            ia.done()

    def test3_copyimagemask(self):
        """ (copy mode) testcopy3: copying an image mask (1/0 amsk) to a new image with different coordinates(regrid)""" 
        try:
            makemask(mode='copy',inpimage=self.inimage2,inpmask=self.inimage, output=self.outimage3)
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage3))

    def test4_copyimagemask(self):
        """ (copy mode) testcopy4: copying a region txt to a new image whose coordinates is defined by inpimage"""
        try:
            makemask(mode='copy',inpimage=self.inimage2,inpmask='ellipse [[13:30:15.79110, +030.13.51.8986], [340.4877arcsec, 299.4327arcsec], 0.00000000deg]', output=self.outimage3+':newmask')
        except Exception, e:
            print "\nError running makemask"
            raise e
           
        self.assertTrue(os.path.exists(self.outimage3))
        if os.path.exists(self.outimage3):
          ia.open(self.outimage3)
          outmask = ia.maskhandler('get')
          self.assertTrue(outmask[0]=='newmask')

class test_merge(makemaskTestBase):
    """test merging of multiple masks in copy mode"""

    #data in repository
    inimage='ngc5921.cube1.mask'
    inimage2='ngc5921.cube1.bmask'
    inimage3='ngc5921.cube2.mask'
    infile1='ellipse_rg.txt'

    outimage1='ngc5921.cube1.merge.mask'
    outimage2='ngc5921.cube1.copyinmage.mask'

    refimage1=datapath+'reference/ngc5921.mergetest1.ref.mask'
    refimage2=datapath+'reference/ngc5921.mergetest2.ref.mask'
    refimage3=datapath+'reference/ngc5921.mergetest3.ref.mask'

    def setUp(self):
        #for img in [self.inimage,self.outimage1,self.outimage2, self.outimage3]:
        #    if os.path.isdir(img):
        #        shutil.rmtree(img)
        for img in [self.inimage,self.inimage2,self.inimage3]:
            if not os.path.isdir(img):
                shutil.copytree(datapath+img,img)

    def tearDown(self):
        if not debug:
            for img in [self.inimage,self.inimage2,self.inimage3,self.outimage1,self.outimage2, self.infile1]:
                #pass
                if os.path.isdir(img):
                    shutil.rmtree(img)
        else:
            print "debugging mode: clean-up did not performed"

    def test1_mergemasks(self):
        """ (copy mode) mergetest1: merging image mask (1/0 mask) and T/F mask and put into a new but existing image mask"""
        try:
            shutil.copytree(self.inimage,self.outimage1)
            makemask(mode='copy',inpimage=self.inimage,inpmask=[self.inimage,self.inimage2+':maskoo'], output=self.outimage1, overwrite=True)
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage1))
        self.assertTrue(self.compareimpix(self.refimage1,self.outimage1))
        #shutil.rmtree(self.outimage1)

    def test2_mergemasks(self):
        """ (copy mode) mergetest2 :merging two image mask (1/0 mask) with different chan width and a  T/F mask to create a 1/0 mask"""
        try:
            #shutil.copytree(self.inimage,self.outimage1)
            makemask(mode='copy',inpimage=self.inimage,inpmask=[self.inimage, self.inimage3, self.inimage2+':maskoo'], output=self.outimage1)
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage1))
        self.assertTrue(self.compareimpix(self.refimage2,self.outimage1))
        #shutil.rmtree(self.outimage1)

    def test3_mergemasks(self):
        """ (copy mode) mergetest3: merging multiple masks (image mask, boolean mask, regions"""
        try:
            #shutil.copytree(self.inimage,self.outimage1)
            if not os.path.exists(self.infile1):
                shutil.copy(datapath+self.infile1, self.infile1)
            makemask(mode='copy',inpimage=self.inimage,\
                    inpmask=[self.inimage3, self.inimage2+':maskoo','ellipse_rg.txt','box[[130pix,135pix],[160pix,165pix]]'],\
                    output=self.outimage1)
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage1))
        self.assertTrue(self.compareimpix(self.refimage3,self.outimage1))
        #shutil.rmtree(self.outimage1)


class test_expand(makemaskTestBase):
    """test expand mode"""

    #data in repository
    inimage='ngc5921.cont.mask'
    inimage2='ngc5921.cube1.mask'
    inimage3='ngc5921.cube2.mask'
    inimage4='ngc5921.cube1.bmask' #T/F mask

    outimage='ngc5921.cube1.expand.mask'
    outimage2='ngc5921.cube1.copyinmage.mask'
    outimage3='ngc5921.cube2.expand.mask' # non-existent mask

    refimage1=datapath+'reference/ngc5921.expandtest1.ref.mask'
    refimage2=datapath+'reference/ngc5921.expandtest2.ref.mask'
    refimage3=datapath+'reference/ngc5921.expandtest5.ref.mask'
    refimage4=datapath+'reference/ngc5921.expandtest6.ref.mask'
    refimage5=datapath+'reference/ngc5921.expandtest2b.ref.mask'

    def setUp(self):
        #for img in [self.inimage,self.outimage1,self.outimage2, self.outimage3]:
        #    if os.path.isdir(img):
        #        shutil.rmtree(img)
        for img in [self.inimage,self.inimage2,self.inimage3,self.inimage4]:
            if not os.path.isdir(img):
                shutil.copytree(datapath+img,img)

    def tearDown(self):
        if not debug:
            for img in [self.inimage,self.inimage2,self.inimage3,self.inimage4,
                        self.outimage,self.outimage2,self.outimage3]:
                if os.path.isdir(img):
                    shutil.rmtree(img)
                    #pass
        else:
            print "debugging mode: clean-up did not performed"

    def test1_expandmask(self):
        """ (expand mode) test1: an image mask from continuum clean to a cube mask"""
        try:
            shutil.copytree(self.inimage2,self.outimage)
            makemask(mode='expand',inpimage=self.inimage,inpmask=self.inimage, output=self.outimage, overwrite=True)
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage))
        self.assertTrue(self.compareimpix(self.refimage1,self.outimage))

    #def test2_expandmask(self):
    #    """ (expand mode) test2: an image mask from continuum clean to a cube mask with outfreqs by channel indices with existing ouput image(over-written)"""
    #    # this is probably not recommended the resultant mask is added to the existing outimage which already have mask in some channels.
    #    # this works while inpimage here is a continuum mask image and not a correct template, since the output image exist it will use
    #    # as is when inpmask is a continuum image. 
    #    try:
    #        shutil.copytree(self.inimage2,self.outimage)
    #        makemask(mode='expand',inpimage=self.inimage,inpmask=self.inimage, output=self.outimage, outfreqs=[4,5,6,7],overwrite=True)
    #        shutil.copytree(self.outimage,'test2result.im')
    #    except Exception, e:
    #        print "\nError running makemask"
    #        raise e
    #
    #    self.assertTrue(os.path.exists(self.outimage))
    #    self.assertTrue(self.compareimpix(self.refimage2,self.outimage))

    # replacing with this
    def test2_expandmask(self):
        """ (expand mode) test2: an image mask from continuum clean to a cube mask with outfreqs by channel indices"""
        try:
            makemask(mode='expand',inpimage=self.inimage2,inpmask=self.inimage, output=self.outimage, outfreqs=[4,5,6,7],overwrite=False)
            #shutil.copytree(self.outimage,'test2bresult.im')
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage))
        self.assertTrue(self.compareimpix(self.refimage5,self.outimage))


    def test3_expandmask(self):
        """ (expand mode) test3: an image mask from continuum clean to a cube mask with outfreqs by a frequency range"""
        # will be the same range as test2 will be masked (ch4,5,6,7) and one in original output mask (ch9)
        try:
            shutil.copytree(self.inimage2,self.outimage)
            makemask(mode='expand',inpimage=self.inimage,inpmask=self.inimage, output=self.outimage, 
              outfreqs='1413.007MHz~1413.08MHz',overwrite=True)
            #shutil.copytree(self.outimage,'test3result.im')
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage))
        self.assertTrue(self.compareimpix(self.refimage2,self.outimage))


    def test4_expandmask(self):
        """ (expand mode) test4: an image mask from continuum clean to a cube mask with outfreqs by a velocity range"""
        try:
            shutil.copytree(self.inimage2,self.outimage)
            makemask(mode='expand',inpimage=self.inimage,inpmask=self.inimage, output=self.outimage, 
              outfreqs='1561.62km/s~1546.16km/s',overwrite=True)
            #shutil.copytree(self.outimage,'test4result.im')
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage))
        self.assertTrue(self.compareimpix(self.refimage2,self.outimage))

    def test5_expandmask(self):
        """ (expand mode) test5: an image mask from a cube mask to another cube that sepecified by a template"""
        # pick up a channel with a mask in inpimagei(20ch-cube), and copy and expand the mask to 
        # another cube (chan width=2, 10ch-cube) - corresponds to ch 4,5,6,7 of outimage
        try:
            makemask(mode='expand',inpimage=self.inimage3, inpmask=self.inimage2, inpfreqs='1413.029MHz~1413.229MHz', 
              output=self.outimage3, outfreqs='1413.117MHz~1413.263MHz')
            #shutil.copytree(self.outimage3,'test5result.im')
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage3))
        self.assertTrue(self.compareimpix(self.refimage3,self.outimage3))

    def test6_expandmask(self):
        """ (expand mode) test6: T/F mask from a cube mask to another cube that sepecified by a template"""
        # mask at chan 4, 1561.62km/s lsrk 1413.01MHz (will drop the empty channel planes)
        # expand to chan 2 - 6
        try:
            makemask(mode='expand',inpimage=self.inimage3,inpmask=self.inimage4+':maskoo', inpfreqs='1561km/s~1556km/s', 
              output=self.outimage3, outfreqs='1559.04km/s~1517.82km/s')
            #shutil.copytree(self.outimage3,'test6result.im')
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage3))
        self.assertTrue(self.compareimpix(self.refimage4,self.outimage3))

    def test7_expandmask(self):
        """ (expand mode) test7: T/F mask from a cube mask to another cube that sepecified by a template"""
        # mask at chan 4, 1561.62km/s lsrk 1413.01MHz (will drop the empty channel planes)
        # expand to chan 2 - 6
        try:
            makemask(mode='expand',inpimage=self.inimage2,inpmask=self.inimage4+':maskoo', inpfreqs='1561km/s~1556km/s', 
              output=self.inimage3, outfreqs='1559.04km/s~1517.82km/s',overwrite=True)
            #shutil.copytree(self.outimage3,'test6result.im')
        except Exception, e:
            print "\nError running makemask"
            raise e

        #self.assertTrue(os.path.exists(self.outimage3))
        #self.assertTrue(self.compareimpix(self.refimage4,self.outimage3))

class test_inmask(makemaskTestBase):
    """internal mask manupilations"""

    #data in repository
    inimage='ngc5921.cube1.bmask2' #T/F mask

    def setUp(self):
        for img in [self.inimage]:
            if not os.path.isdir(img):
                shutil.copytree(datapath+img,img)

    def tearDown(self):
        if not debug:
            for img in [self.inimage]:
                if os.path.isdir(img):
                    shutil.rmtree(img)
        else:
            print "debugging mode: clean-up did not performed"

    def test_deletemask(self):
        """ (delete mode) delete an internal mask from the image"""
        try:
            makemask(mode='delete',inpmask=self.inimage+':'+'mask2')
        except Exception, e:
            print "\nError running makemask"
            raise e

        ia.open(self.inimage)
        mlist=ia.maskhandler('get')
        ia.close()
        self.assertTrue(mlist.count('mask2')==0)

    def test_setdefault(self):
        """ (setdefaultmask mode) set an internal mask as a default mask"""
        try:
            makemask(mode='setdefaultmask',inpmask=self.inimage+':'+'mask2')
        except Exception, e:
            print "\nError running makemask"
            raise e

        ia.open(self.inimage)
        defaultmask=ia.maskhandler('default')[0]
        ia.close()
        self.assertTrue(defaultmask=='mask2')

def suite():
    #return [test_inmask]
    return [test_merge,test_expand,test_copy,test_inmask]
