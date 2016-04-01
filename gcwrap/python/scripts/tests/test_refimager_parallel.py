##########################################################################
#
# Test programs for the refactored imager's parallel runs:  test_refimager_parallel
#
##########################################################################


import os
import sys
import shutil
import commands
import numpy
from __main__ import default
from tasks import *
from taskinit import *
import unittest
import inspect

from refimagerhelper import TestHelpers


refdatapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/refimager/'
#refdatapath = "/export/home/riya/rurvashi/Work/ImagerRefactor/Runs/UnitData/"
#refdatapath = "/home/vega/rurvashi/TestCASA/ImagerRefactor/Runs/WFtests/"

##############################################
##############################################

## List to be run
def suite():
     return [test_cont]

###################################################
## Base Test class with Utility functions
###################################################
class testref_base_parallel(unittest.TestCase):

     def setUp(self):
          self.epsilon = 0.05
          self.msfile = ""
          self.img = "tst"

          self.th = TestHelpers()

     def tearDown(self):
          """ don't delete it all """
#          self.delData()

     # Separate functions here, for special-case tests that need their own MS.
     def prepData(self,msname=""):
          os.system('rm -rf ' + self.img+'*')
          if msname != "":
               self.msfile=msname
          if (os.path.exists(self.msfile)):
               os.system('rm -rf ' + self.msfile)
          shutil.copytree(refdatapath+self.msfile, self.msfile)
          
     def delData(self,msname=""):
          if msname != "":
               self.msfile=msname
          if (os.path.exists(self.msfile)):
               os.system('rm -rf ' + self.msfile)
          os.system('rm -rf ' + self.img+'*')

     def checkfinal(self,pstr=""):
          if( pstr.count("(Fail") > 0 ):
               pstr += "["+inspect.stack()[2][3]+"] : To re-run this test :  runUnitTest.main(['test_refimager_parallel["+ inspect.stack()[2][3] +"]']) "
               self.fail("\n"+pstr)


###################################################
#### Test parallel continuum imaging
###################################################
class test_cont(testref_base_parallel):
     
     def test_cont_hogbom_gridft(self):
          """ [cont] Test_cont_hogbom_gridft : Basic Hogbom clean with gridft gridder. Only data parallelization """

          if self.th.checkMPI() == True:

               self.prepData('refim_point.ms')
               
               ## Non-parallel run
               ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',
                            interactive=0,niter=10,parallel=False)
               
               self.th.checkall(ret=ret, peakres=0.332, 
                                modflux=0.726, iterdone=10, 
                                imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image',self.img+'.model'], 
                                imval=[(self.img+'.sumwt', 34390852.0,[0,0,0,0])])

               ## Parallel run
               imgpar = self.img+'.par'
               retpar = tclean(vis=self.msfile,imagename=imgpar,imsize=100,cell='8.0arcsec',
                               interactive=0,niter=10,parallel=True)

               checkims = [imgpar+'.psf', imgpar+'.residual', imgpar+'.image',imgpar+'.model']
               checkims = checkims + self.th.getNParts( imprefix=imgpar, imexts=['residual','psf','model'] ) 

               report = self.th.checkall(ret=retpar, peakres=0.332, 
                                         modflux=0.726, iterdone=10, 
                                         imexist=checkims, 
                                         imval=[(imgpar+'.sumwt' ,34390852.0,[0,0,0,0])]) 

               ## Pass or Fail (and why) ?
               self.checkfinal(report)

          else:
               print "MPI is not enabled. This test will be skipped"

###################################################

     def test_cont_mtmfs_gridft(self):
          """ [cont] Test_cont_mtmfs_gridft : MT-MFS with gridft gridder. Only data parallelization """

          if self.th.checkMPI() == True:

               self.prepData('refim_point.ms')
               
               # Non-parallel run
               ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',
                            interactive=0,niter=10,deconvolver='mtmfs',parallel=False)
               report1 = self.th.checkall(ret=ret, 
                                          peakres=0.369, modflux=0.689, iterdone=10, nmajordone=2,
                                          imexist=[self.img+'.psf.tt0', self.img+'.residual.tt0', 
                                                   self.img+'.image.tt0',self.img+'.model.tt0'], 
                                          imval=[(self.img+'.alpha',-1.032,[50,50,0,0]),
                                                 (self.img+'.sumwt.tt0', 34390852.0,[0,0,0,0]) ,
                                                 (self.img+'.sumwt.tt1',350.618,[0,0,0,0]) ])

               # Parallel run
               imgpar = self.img+'.par'
               retpar = tclean(vis=self.msfile,imagename=imgpar,imsize=100,cell='8.0arcsec',
                               interactive=0,niter=10,deconvolver='mtmfs',parallel=True)
               
               checkims = [imgpar+'.psf.tt0', imgpar+'.residual.tt0', imgpar+'.image.tt0',imgpar+'.model.tt0']  
               checkims = checkims + self.th.getNParts( imprefix=imgpar, 
                                                        imexts=['residual.tt0','residual.tt1','psf.tt0','psf.tt1','model.tt0','model.tt1']) 
               report2 = self.th.checkall(ret=retpar, 
                                          peakres=0.369, modflux=0.689, iterdone=10, nmajordone=2,
                                          imexist=checkims, 
                                          imval=[(imgpar+'.alpha',-1.032,[50,50,0,0]),
                                                 (self.img+'.sumwt.tt0',34390852.0,[0,0,0,0]),
                                                 (self.img+'.sumwt.tt1',350.618,[0,0,0,0]) ])

               ## Pass or Fail (and why) ?
               self.checkfinal(report1+report2)
 
          else:
               print "MPI is not enabled. This test will be skipped"
 
###################################################

     def test_cont_mtmfs_aproj(self):
          """ [cont] Test_cont_mtmfs_aproj : MT-MFS with aprojection gridder (checks .weight.pb). Data and CFCache parallelization """

          if self.th.checkMPI() == True:
               print "This test is currently empty"
          else:
               print "MPI is not enabled. This test will be skipped"
 
###################################################


###################################################
#### Test parallel continuum imaging
###################################################
class test_cube(testref_base_parallel):

     def test_cube_1(self):
          """ [cube] Test_cube_1 : Data and image parallelization """

          if self.th.checkMPI() == True:

               print "This test is currently empty"
               ## Fill test here

          else:
               print "MPI is not enabled. This test will be skipped"
 
###################################################

