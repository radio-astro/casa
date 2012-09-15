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

debug=True


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
        
def suite():
    return [test_copy]
