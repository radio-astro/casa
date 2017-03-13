##########################################################################
#
# Test programs for the refactored imager's parallel runs:  test_refimager_parallel
#
#  Add tests for 
#     - cube/cont parallel with multiple MSs, MMS, w/wo model writes, restarts 
#     - mosaic, AWProj cont and cube
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

from imagerhelpers.test_imager_helper import TestHelpers

_ia = iatool( )

refdatapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/refimager/'
#refdatapath = "/export/home/riya/rurvashi/Work/ImagerRefactor/Runs/UnitData/"
#refdatapath = "/home/vega/rurvashi/TestCASA/ImagerRefactor/Runs/WFtests/"

##############################################
##############################################

## List to be run
def suite():
     return [test_cont,test_cube]

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
          pstr += "["+inspect.stack()[1][3]+"] : To re-run this test :  mpirun -n 4 -xterm 0 `which casa` -c `echo $CASAPATH | awk '{print $1}'`/gcwrap/python/scripts/regressions/admin/runUnitTest.py test_refimager_parallel["+ inspect.stack()[1][3] +"]"
          casalog.post(pstr,'INFO')
          if( pstr.count("(Fail") > 0 ):
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
                                                 (imgpar+'.sumwt.tt0',34390852.0,[0,0,0,0]),
                                                 (imgpar+'.sumwt.tt1',350.618,[0,0,0,0]) ])

               ## Pass or Fail (and why) ?
               self.checkfinal(report1+report2)
 
          else:
               print "MPI is not enabled. This test will be skipped"
 
###################################################
###################################################

     def test_cont_mtmfs_2spws_2MSs(self):
          """ [cont] Test_cont_mtmfs_2spws_2MSs : MT-MFS on multi-spws in separate MSs, to test default reffreq and coordinate system generation (CAS-9518) """

          if self.th.checkMPI() == True:

               ms1 = 'refim_point_onespw0.ms'
               ms2 = 'refim_point_onespw1.ms'
               self.prepData(ms1)
               self.prepData(ms2)

               # Non-parallel run
               ret = tclean(vis=[ms1,ms2],imagename=self.img,imsize=100,cell='8.0arcsec',
                            interactive=0,niter=10,deconvolver='mtmfs',parallel=False)
               report1 = self.th.checkall(ret=ret, 
                                          peakres=0.409, modflux=0.764, iterdone=10, nmajordone=2,
                                          imexist=[self.img+'.psf.tt0', self.img+'.residual.tt0', 
                                                   self.img+'.image.tt0',self.img+'.model.tt0'], 
                                          imval=[(self.img+'.alpha',-2.0,[50,50,0,0]),
                                                 (self.img+'.sumwt.tt0', 94050.05,[0,0,0,0]) ,
                                                 (self.img+'.sumwt.tt1',0.986736,[0,0,0,0]) ])

               # Parallel run
               imgpar = self.img+'.par'
               retpar = tclean(vis=[ms1,ms2],imagename=imgpar,imsize=100,cell='8.0arcsec',
                               interactive=0,niter=10,deconvolver='mtmfs',parallel=True)
               
               checkims = [imgpar+'.psf.tt0', imgpar+'.residual.tt0', imgpar+'.image.tt0',imgpar+'.model.tt0']  
               checkims = checkims + self.th.getNParts( imprefix=imgpar, 
                                                        imexts=['residual.tt0','residual.tt1','psf.tt0','psf.tt1','model.tt0','model.tt1']) 
               report2 = self.th.checkall(ret=retpar, 
                                          peakres=0.409, modflux=0.764, iterdone=10, nmajordone=2,
                                          imexist=checkims, 
                                          imval=[(imgpar+'.alpha',-2.0,[50,50,0,0]),
                                                 (imgpar+'.sumwt.tt0',94050.05,[0,0,0,0]),
                                                 (imgpar+'.sumwt.tt1',0.986736,[0,0,0,0]) ])

               ## Check the reference frequency of the coordinate system of the parallel run.
               _ia.open(imgpar+'.image.tt0')
               csys = _ia.coordsys()
               _ia.close()
               reffreq = csys.referencevalue()['numeric'][3]
               correctfreq = 1489984780.68491
               if  abs(reffreq - correctfreq)/correctfreq > self.epsilon :
                    retres=False
               else:
                    retres=True

               pstr = "[" +  inspect.stack()[0][3]+ "] Ref-Freq is " + str(reffreq) + " ("+self.th.verdict(retres)+" : should be " + str(correctfreq) + ")\n"
               report2 = report2 + pstr

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

     def test_cube_niter10(self):
          """ [cube] Test_cube_niter10 : Data and image parallelization, fixed niter per channel. """
          
          if self.th.checkMPI() == True:
               
               self.prepData('refim_point.ms')
               
               # Non-parallel run
               ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',
                            interactive=0,niter=10,deconvolver='hogbom',specmode='cube',
                            pbcor=True, parallel=False)

               report1 = self.th.checkall(ret=ret, 
                                          peakres=0.241, modflux=0.527, iterdone=200, nmajordone=2,
                                          imexist=[self.img+'.psf', self.img+'.residual', 
                                                   self.img+'.image',self.img+'.model', 
                                                   self.img+'.pb',  self.img+'.image.pbcor' ], 
                                          imval=[(self.img+'.image',1.2,[50,50,0,5]),
                                                 (self.img+'.sumwt',2949.775,[0,0,0,0]) ])

               # Parallel run
               imgpar = self.img+'.par'
               retpar = tclean(vis=self.msfile,imagename=imgpar,imsize=100,cell='8.0arcsec',
                               interactive=0,niter=10,deconvolver='hogbom',specmode='cube',
                               pbcor=True,parallel=True)
               
               checkims =self.th.getNParts( imprefix=imgpar, imexts=['residual','psf','model']) 
               report2 = self.th.checkall(ret=retpar['node1'][1], 
#                                          peakres=0.241, modflux=0.527, iterdone=200, nmajordone=2,
                                          imexist=[self.img+'.psf', self.img+'.residual', 
                                                   self.img+'.image',self.img+'.model', 
                                                   self.img+'.pb',  self.img+'.image.pbcor' ], 
                                          imval=[(self.img+'.image',1.2,[50,50,0,5]),
                                                 (self.img+'.sumwt',2949.775,[0,0,0,0]) ])

               ## Pass or Fail (and why) ?
               self.checkfinal(report1+report2)

          else:
               print "MPI is not enabled. This test will be skipped"

###################################################

