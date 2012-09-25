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
    outimage3='ngc5921.cube2.mask'

    outimage1='ngc5921.cube1.copy.mask'
    outimage2='ngc5921.cube1.copyinmage.mask'

    def setUp(self):
        #for img in [self.inimage,self.outimage1,self.outimage2, self.outimage3]:
        #    if os.path.isdir(img):
        #        shutil.rmtree(img)
        for img in [self.inimage,self.outimage3]:
            if not os.path.isdir(img):
                shutil.copytree(datapath+img,img)

    def tearDown(self):
        if not debug:
            for img in [self.inimage,self.outimage1,self.outimage2,self.outimage3]:
                if os.path.isdir(img):
                    shutil.rmtree(img)
        else:
            print "debugging mode: clean-up did not performed" 
        
    def test1_copyimagemask(self):
        """ Test copying an image mask (1/0 mask) to a new image mask"""
        try:
            makemask(inpimage=self.inimage,inpmask='',outimage=self.outimage1, outmask='')
        except Exception, e:
            print "\nError running makemask"
            raise e
        
        self.assertTrue(os.path.exists(self.outimage1))           
        self.assertTrue(self.compareimpix(self.inimage,self.outimage1))           
         
    def test2_copyimagemask(self):
        """ Test copying an image mask (1/0 mask) to a new in-image mask"""
        try:
            makemask(inpimage=self.inimage,inpmask='',outimage=self.outimage2, outmask='masknew')
        except Exception, e:
            print "\nError running makemask"
            raise e
         
        self.assertTrue(os.path.exists(self.outimage2))
        if os.path.exists(self.outimage2):
            ia.open(self.outimage2)
            maskname=ia.maskhandler('get')[0]
            if maskname=='masknew':
                pixelmask2cleanmask(self.outimage2,'masknew','_tmp_im',True)
                self.assertTrue(self.compareimpix(self.inimage,'_tmp_im')) 
            ia.done()
            shutil.rmtree('_tmp_im')

    def test3_copyimagemask(self):
        """Test copying an image mask (1/0 amsk) to a new image with different coordinates(regrid)""" 
        try:
            makemask(inpimage=self.inimage,inpmask='',outimage=self.outimage3, outmask='')
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage3))
        
class test_merge(makemaskTestBase):
    """test merge mode"""

    #data in repository
    inimage='ngc5921.cube1.mask'
    inimage2='ngc5921.cube1.bmask'
    inimage3='ngc5921.cube2.mask'

    outimage1='ngc5921.cube1.merge.mask'
    outimage2='ngc5921.cube1.copyinmage.mask'

    refimage1=datapath+'reference/ngc5921.mergetest1.ref.mask'
    refimage2=datapath+'reference/ngc5921.mergetest2.ref.mask'

    def setUp(self):
        #for img in [self.inimage,self.outimage1,self.outimage2, self.outimage3]:
        #    if os.path.isdir(img):
        #        shutil.rmtree(img)
        for img in [self.inimage,self.inimage2,self.inimage3]:
            if not os.path.isdir(img):
                shutil.copytree(datapath+img,img)

    def tearDown(self):
        if not debug:
            for img in [self.inimage,self.inimage2,self.inimage3,self.outimage1,self.outimage2]:
                if os.path.isdir(img):
                    shutil.rmtree(img)
        else:
            print "debugging mode: clean-up did not performed"

    def test1_mergemasks(self):
        """ Test merging image mask (1/0 mask) and T/F mask to a new image mask"""
        try:
            shutil.copytree(self.inimage,self.outimage1)
            makemask(mode='merge',inpimage=self.inimage,inpmask=self.inimage2+':maskoo', outimage=self.outimage1, outmask='')
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage1))
        self.assertTrue(self.compareimpix(self.refimage1,self.outimage1))
        #shutil.rmtree(self.outimage1)

    def test2_mergemasks(self):
        """ Test merging two image mask (1/0 mask) with different chan width to a new T/F mask"""
        try:
            #shutil.copytree(self.inimage,self.outimage1)
            makemask(mode='merge',inpimage=[self.inimage, self.inimage3],inpmask=self.inimage2+':maskoo', outimage=self.outimage1, outmask='')
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage1))
        self.assertTrue(self.compareimpix(self.refimage2,self.outimage1))
        #shutil.rmtree(self.outimage1)

class test_expand(makemaskTestBase):
    """test merge mode"""

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
        else:
            print "debugging mode: clean-up did not performed"

    def test1_expandmask(self):
        """ expand an image mask from continuum clean to a cube mask"""
        try:
            shutil.copytree(self.inimage2,self.outimage)
            makemask(mode='expand',inpimage=self.inimage,inpmask='', outimage=self.outimage, outmask='')
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage))
        self.assertTrue(self.compareimpix(self.refimage1,self.outimage))

    def test2_expandmask(self):
        """ expand an image mask from continuum clean to a cube mask with outfreqs by channel indices"""
        try:
            shutil.copytree(self.inimage2,self.outimage)
            makemask(mode='expand',inpimage=self.inimage,inpmask='', outimage=self.outimage, outmask='', outfreqs=[4,5,6,7])
            #shutil.copytree(self.outimage,'test2result.im')
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage))
        self.assertTrue(self.compareimpix(self.refimage2,self.outimage))

    def test3_expandmask(self):
        """ expand an image mask from continuum clean to a cube mask with outfreqs by a frequency range"""
        try:
            shutil.copytree(self.inimage2,self.outimage)
            makemask(mode='expand',inpimage=self.inimage,inpmask='', outimage=self.outimage, outmask='', 
              outfreqs='1413.007MHz~1413.08MHz')
            #shutil.copytree(self.outimage,'test3result.im')
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage))
        self.assertTrue(self.compareimpix(self.refimage2,self.outimage))


    def test4_expandmask(self):
        """ expand an image mask from continuum clean to a cube mask with outfreqs by a velocity range"""
        try:
            shutil.copytree(self.inimage2,self.outimage)
            makemask(mode='expand',inpimage=self.inimage,inpmask='', outimage=self.outimage, outmask='', 
              outfreqs='1561.62km/s~1546.16km/s')
            #shutil.copytree(self.outimage,'test4result.im')
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage))
        self.assertTrue(self.compareimpix(self.refimage2,self.outimage))

    def test5_expandmask(self):
        """ expand an image mask from a cube mask to another cube that sepecified by a template"""
        # pick up a channel with a mask in inpimagei(20ch-cube), and copy and expand the mask to 
        # another cube (chan width=2, 10ch-cube) - corresponds to ch 4,5,6,7 of outimage
        try:
            makemask(mode='expand',inpimage=self.inimage2,inpmask='', inpfreqs='1413.029MHz~1413.229MHz', 
              outimage=self.outimage3, outmask='', outfreqs='1413.117MHz~1413.263MHz', template=self.inimage3)
            #shutil.copytree(self.outimage3,'test5result.im')
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage3))
        self.assertTrue(self.compareimpix(self.refimage3,self.outimage3))

    def test6_expandmask(self):
        """ expand T/F mask from a cube mask to another cube that sepecified by a template"""
        # mask at chan 4, 1561.62km/s lsrk 1413.01MHz (will drop the empty channel planes)
        # expand to chan 2 - 6
        try:
            makemask(mode='expand',inpimage=self.inimage4,inpmask='maskoo', inpfreqs='1561km/s~1556km/s', 
              outimage=self.outimage3, outmask='', outfreqs='1559.04km/s~1517.82km/s', template=self.inimage3)
            #shutil.copytree(self.outimage3,'test6result.im')
        except Exception, e:
            print "\nError running makemask"
            raise e

        self.assertTrue(os.path.exists(self.outimage3))
        self.assertTrue(self.compareimpix(self.refimage4,self.outimage3))

def suite():
    return [test_copy, test_merge, test_expand]
