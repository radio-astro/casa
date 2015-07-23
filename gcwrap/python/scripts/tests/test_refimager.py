##########################################################################
##########################################################################
#
# Test programs for the refactored imager :  test_refimager
#
# Each of the following categories (classes) has a set of tests within it.
#
#  test_onefield                 # basic tests, deconvolution algorithms
#  test_iterbot                   # iteration control options for mfs and cube
#  test_multifield               # multiple fields of same type and with different shapes/deconvolvers/gridders
#  test_stokes                    # multiple stokes planes, imaging with flagged correlations..
#  test_cube                      # all things cube. Spectral frame setup, handling empty channels, etc
#  test_widefield                # facets, wprojection, imagemosaic, mosaicft, awproject
#  test_mask                      # input mask options : regridding, mask file, automasking, etc
#  test_modelvis                # saving models (column/otf), using starting models, predict-only (setjy)
#
# To run from within casapy :  
#
#  runUnitTest.main(['test_refimager'])                                                                           # Run all tests
#  runUnitTest.main(['test_refimager[test_onefield]'])                                                    # Run tests from test_onefield
#  runUnitTest.main(['test_refimager[test_onefield_mtmfs]'])                                        # Run one specific test
#  runUnitTest.main(['test_refimager[test_onefield_mtmfs,test_onefield_hogbom]'])    # Multiple specific tests
#
# To see the full list of tests :   grep "\"\"\" \[" test_refimager.py
#
#  These tests need data stored in data/regression/unittest/clean/refimager
#
#  For a developer build, to get the datasets locally 
#
#  --- Get the basic data repo :  svn co https://svn.cv.nrao.edu/svn/casa-data/distro data
#  --- Make directories : mkdir -p data/regression/unittest/clean; cd data/regression/unittest/clean
#  --- Get test datasets :  svn co https://svn.cv.nrao.edu/svn/casa-data/trunk/regression/unittest/clean/refimager
#
##########################################################################
#
#  Datasets
#
#  refim_twochan.ms : 2 channels, one 1Jy point source with spectral index of -1.0
#  refim_twopoints_twochan.ms : Two point sources, 1Jy and 5Jy, both with spectral index -1.0. For multifield tests.
#  refim_point.ms : 1-2 GHz, 20 channels, 1 spw, one 1Jy point source with spectral index -1.0.
#  refim_point_withline.ms : refim_point with a 'line' added into 3 channels (just topo)
#  refim_mawproject.ms : Two pointing wideband mosaic with 1 point source in between the two pointings
#  refim_mawproject_offcenter.ms : Two pointing wideband mosaic with 1 point source at center of one pointing
#  refim_point_stokes.ms : RR=1.0, LL=0.8, RL and LR are zero. Stokes I=0.9, V=0.1, U,Q=0.0
#  refim_point_linRL.ms : I=1, Q=2, U=3, V=4  in circular pol basis.
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



## List to be run
def suite():
     return [test_onefield, test_iterbot, test_multifield,test_stokes, test_widefield, test_modelvis]
#     return [test_onefield, test_iterbot, test_multifield,test_stokes,test_cube, test_widefield,test_mask, test_modelvis,test_modelvis_failing,test_widefield_failing]

refdatapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/refimager/'
#refdatapath = "/export/home/riya/rurvashi/Work/ImagerRefactor/Runs/UnitData/"

## Base Test class with Utility functions
class testref_base(unittest.TestCase):

     def setUp(self):
          self.epsilon = 0.05
          self.msfile = ""
          self.img = "tst"

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

     # Helper functions
     def write_file(self,filename,str_text):
          """Save the string in a text file"""
          inp = filename
          cmd = str_text
          # remove file first
          if os.path.exists(inp):
               os.system('rm -f '+ inp)
          # save to a file    
          with open(inp, 'w') as f:
               f.write(cmd)
          f.close()
          return

     def get_max(self,imname):
          """Get Image max"""
          ia.open(imname)
          stat = ia.statistics()
          ia.close()
          return stat['max'],stat['maxpos']

     def get_pix(self,imname,pos):
          """Get Image max"""
          ia.open(imname)
          apos = ia.pixelvalue(pos)
          ia.close()
          if apos == {}:
               return None
          else:
               return apos['value']['value']

     def exists(self,imname):
          """ Image exists """
          return os.path.exists(imname)

     def checkpeakres(self,summ,correctres):
          peakres = self.getpeakres(summ)
          out = True
          if correctres == None and peakres != None: 
               out = False
          if correctres != None and peakres == None: 
               out = False
          if out==True and peakres != None:
               if abs(correctres - peakres)/abs(correctres) > self.epsilon:
                    out=False
          return out,peakres

     def checkmodflux(self,summ,correctmod):
          modflux = self.getmodflux(summ)
          out = True
          if correctmod == None and modflux != None: 
               out = False
          if correctmod != None and modflux == None: 
               out = False
          if out==True and modflux != None:
               if abs(correctmod - modflux)/abs(correctmod) > self.epsilon:
                    out=False
          return out,modflux

#     def checkiterdone(self,summ,correctiterdone):
#          iters = self.getiterdone(summ)
#          out=True
#          if correctiterdone == None and iters != None: 
#               out = False
#          if correctiterdone != None and iters == None: 
#               out = False
#          if out==True and iters != None:
#               if abs(correctiterdone - iters)/correctiterdone > self.epsilon:
#                    out=False
#          return out, iters

     def getpeakres(self,summ):
          if summ.has_key('summaryminor'):
               reslist = summ['summaryminor'][1,:]
               peakres = reslist[ len(reslist)-1 ]
          else:
               peakres = None
          return peakres

     def getmodflux(self,summ):
          if summ.has_key('summaryminor'):
               modlist = summ['summaryminor'][2,:]
               modflux = modlist[ len(modlist)-1 ]
          else:
               modflux = None
          return modflux

     def getiterdone(self,summ):
          if summ.has_key('iterdone'):
               iters = summ['iterdone']
          else:
               iters = None
          return iters

     def verdict(self,boolval):
          if boolval:
               return "Pass"
          else:
               return "Fail"

     def checkret(self,summ,correctres,correctmod):
          testname = inspect.stack()[1][3]
          retres,peakres = self.checkpeakres(summ,correctres)
          retmod,modflux = self.checkmodflux(summ,correctmod)
          
          pstr =  "[" + testname + "] PeakRes is " + str(peakres) + " ("+self.verdict(retres)+" : should be " + str(correctres) + ")\n"
          pstr = pstr + "[" + testname + "] Modflux is " + str(modflux) + " ("+self.verdict(retmod)+" : should be " + str(correctmod) + ")"
          print pstr
          if retres==False or retmod==False:
               self.fail(pstr)

     def checkall(self, ret=None,
                  peakres=None, # a float
                  modflux=None, # a float
                  iterdone=None, # an int
                  nmajordone=None, # an int
                  imexist=None,  # list of image names
                  imexistnot=None, # list of image names
                  imval=None  # list of tuples of (imagename,val,pos)
                  ):
          pstr = ""

          if ret != None and type(ret)==dict:

               try:

                    if peakres != None:
                         pstr += self.checkval( val=self.getpeakres(ret), correctval=peakres, valname="peak res" )

                    if modflux != None:
                         pstr += self.checkval( val=self.getmodflux(ret), correctval=modflux, valname="mod flux" )

                    if iterdone != None:
                         pstr += self.checkval( val=ret['iterdone'], correctval=iterdone, valname="iterdone", exact=True )

                    if nmajordone != None:
                         pstr += self.checkval( val=ret['nmajordone'], correctval=nmajordone, valname="nmajordone", exact=True )

               except Exception as e:
                    print ret
                    raise

          if imexist != None:
               if type(imexist)==list:
                    pstr += self.checkims(imexist, True)

          if imexistnot != None:
               if type(imexistnot)==list:
                    pstr += self.checkims(imexistnot, False)

          if imval != None:
               if type(imval)==list:
                    for ii in imval:
                         if type(ii)==tuple and len(ii)==3:
                              pstr += self.checkpixval(ii[0],ii[1],ii[2])
          
          self.checkfinal(pstr)

     def checkchanvals(self,vallist): # list of tuples of (channumber, relation, value) e.g. (10,">",1.0)
          testname = inspect.stack()[1][3]
          pstr = ""
          for val in vallist:
               if len(val)==3:
                    thisval = self.checkmodelchan(self.msfile,val[0])
                    if val[1]==">":
                         ok = thisval > val[2]
                    elif val[1]=="==":     
                         ok = abs( (thisval - val[2])/val[2] ) < self.epsilon
                    elif val[1]=="<":     
                         ok = thisval < val[2]
                    else:
                         ok=False
                    pstr =  "[" + testname + "] Chan "+ str(val[0]) + "  is " + str(thisval) + " ("+self.verdict(ok)+" : should be " + str(val[1]) + str(val[2]) + ")\n"

          print pstr
          self.checkfinal(pstr)

     def checkfinal(self,pstr=""):
          if( pstr.count("(Fail") > 0 ):
               pstr += "["+inspect.stack()[2][3]+"] : To re-run this test :  runUnitTest.main(['test_refimager["+ inspect.stack()[2][3] +"]']) "
               self.fail("\n"+pstr)

     def checkval(self,val, correctval, valname='Value', exact=False):
          testname = inspect.stack()[2][3]
          
          out = True

          if numpy.isnan(val) or numpy.isinf(val):
               out=False

          if correctval == None and val != None: 
               out = False
          if correctval != None and val == None: 
               out = False
          if out==True and val != None:
               if exact==True:
                    if correctval != val:
                         out=False
               else:
                    if abs(correctval - val)/abs(correctval) > self.epsilon:
                         out=False

          pstr = "[" + testname + "] " + valname + " is " + str(val) + " ("+self.verdict(out)+" : should be " + str(correctval) + ")"
          print pstr
          pstr=pstr+"\n"
#          if out==False:
#               self.fail(pstr)

          return pstr

     def checkims(self,imlist,truth):
          testname = inspect.stack()[2][3]
          imex=[]
          out=True
          for imname in imlist:
               ondisk = self.exists(imname)
               imex.append( ondisk )
               if ondisk != truth:
                    out=False

          pstr = "[" + testname + "] Image made : " + str(imlist) + " = " + str(imex) + "(" + self.verdict(out) + " : should all be " + str(truth) + ")"
          print pstr
          pstr=pstr+"\n"
#          if all(imex) == False:
#               self.fail(pstr)
          return pstr

     def checkpixval(self,imname,theval=0, thepos=[0,0,0,0]):
          testname = inspect.stack()[2][3]
#          maxvals, maxvalposs = self.get_max(imname)
          readval = self.get_pix(imname,thepos)

          res=True

          if numpy.isnan(readval) or numpy.isinf(readval):
               res=False

          if abs(theval)>1e-08:
               if abs(readval - theval)/abs(theval) > self.epsilon: 
                    res = False
               else:
                    res = True
          else:  ## this is to guard against exact zero... sort of.
               if abs(readval - theval) > self.epsilon: 
                    res = False
               else:
                    res = True
               
          pstr =  "[" + testname + "] " + imname + ": Value is " + str(readval) + " at " + str(thepos) + " (" + self.verdict(res) +" : should be " + str(theval) + " )"
          print pstr
          pstr=pstr+"\n"
#          if res==False:
#               self.fail(pstr)
          return pstr

     def modeltype(self,msname):
          """has no model, otf model, modelcol"""
          mtype = 0
          return mtype

     def delmodkeywords(self,msname=""):
          delmod(msname)
          tb.open( msname+'/SOURCE', nomodify=False )
          keys = tb.getkeywords()
          for key in keys:
               tb.removekeyword( key )
          tb.close()

     def resetmodelcol(self,msname=""):
          tb.open( msname, nomodify=False )
          hasmodcol = (  (tb.colnames()).count('MODEL_DATA')>0 )
          if not hasmodcol:
               cb.open(msname)
               cb.close()
          hasmodcol = (  (tb.colnames()).count('MODEL_DATA')>0 )
          if hasmodcol:
               dat = tb.getcol('MODEL_DATA')
               dat.fill( complex(0.0,0.0) )
               tb.putcol('MODEL_DATA', dat)
          tb.close();

     def delmodels(self,msname="",dmodcol=False):
          delmod(msname,scr=True)  ## Get rid of OTF model and model column
#          if dmodcol:
#               self.delmodelcol(msname) ## Delete model column
#          else:
#               self.resetmodelcol(msname)  ## Set model column to one
          cb.open(msname)
          cb.close()
          self.resetmodelcol(msname)  ## Set model column to zero
          self.delmodkeywords(msname) ## Get rid of extra OTF model keywords that sometimes persist...

     def delmodelcol(self,msname=""):
          tb.open( msname, nomodify=False )
          hasmodcol = (  (tb.colnames()).count('MODEL_DATA')>0 )
          if hasmodcol:
               tb.removecols('MODEL_DATA')
          tb.close()

     def checkmodel(self,msname=""):
          tb.open( msname )
          hasmodcol = (  (tb.colnames()).count('MODEL_DATA')>0 )
          modsum=0.0
          if hasmodcol:
               dat = tb.getcol('MODEL_DATA')
               modsum=dat.sum()
          tb.close()

          hasvirmod=False

          tb.open( msname+'/SOURCE' )
          keys = tb.getkeywords()
          if len(keys)>0:
               hasvirmod=True
          tb.close()

          tb.open( msname )
          keys = tb.getkeywords()
          for key in keys:
               if key.count("model_")>0:
                    hasvirmod=True
          tb.close()

          print msname , ": modelcol=", hasmodcol, " modsum=", modsum, " virmod=", hasvirmod
          return hasmodcol, modsum, hasvirmod

     def checkmodelchan(self,msname="",chan=0):
          tb.open( msname )
          hasmodcol = (  (tb.colnames()).count('MODEL_DATA')>0 )
          modsum=0.0
          if hasmodcol:
               dat = tb.getcol('MODEL_DATA')[:,chan,:]
               modsum=dat.mean()
          tb.close()
          ##print modsum
          return modsum


##############################################
##############################################

##Task level tests : one field, 2chan.
class test_onefield(testref_base):
     
     def test_onefield_defaults(self):
          """ [onefield] Test_Onefield_defaults : Defaults """
          self.prepData('refim_twochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',interactive=0)
          self.checkall(imexist=[self.img+'.psf', self.img+'.residual'], imexistnot=[self.img+'.image'],imval=[(self.img+'.psf', 1.0, [50,50,0,0])])

     def test_onefield_clark(self):
          """ [onefield] Test_Onefield_clark : mfs with clark minor cycle """
          self.prepData('refim_twochan.ms')
##          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=100,interactive=0) #,phasecenter='J2000 19h59m57.5s +40d49m00.077s') # default is clark
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=200,cell='8.0arcsec',niter=1000,interactive=0,phasecenter='J2000 19h59m57.5s +40d49m00.077s') # default is clark
          clean(vis=self.msfile,imagename=self.img+'.old',imsize=200,cell='8.0arcsec',niter=1000,psfmode='clark',phasecenter='J2000 19h59m57.5s +40d49m00.077s') # default is clark
#          self.checkall(ret=ret, peakres=0.392, modflux=0.732, iterdone=10, imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image',self.img+'.model'], imval=[(self.img+'.psf',1.0,[50,50,0,0])])

     def test_onefield_hogbom(self):
          """ [onefield] Test_Onefield_hogbom : mfs with hogbom minor cycle """
          self.prepData('refim_twochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=100,deconvolver='hogbom',interactive=0)#,phasecenter='J2000 19h59m57.5s +40d49m00.077s')
          self.checkall(ret=ret, peakres=0.35, modflux=0.77, iterdone=10, imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image',self.img+'.model'], imval=[(self.img+'.psf',1.0,[50,50,0,0])])
         
     def test_onefield_mem(self):
          """ [onefield] Test_Onefield_mem : mfs with mem minor cycle """
          self.prepData('refim_eptwochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=200,cell='8.0arcsec',niter=10,deconvolver='mem',interactive=0)
          self.checkall(ret=ret, peakres=7.84, modflux=6.49, iterdone=10, imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image',self.img+'.model'], imval=[(self.img+'.psf',1.0,[100,100,0,0])])

     def test_onefield_multiscale(self):
          """ [onefield] Test_Onefield_multiscale : mfs with multiscale minor cycle """
          self.prepData('refim_eptwochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=200,cell='8.0arcsec',niter=10,deconvolver='multiscale',scales=[0,20,40,100],interactive=0)
          self.checkall(ret=ret, peakres=2.3, modflux=2.87, iterdone=10, imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image',self.img+'.model'], imval=[(self.img+'.psf',1.0,[100,100,0,0])])

     def test_onefield_mtmfs(self):
          """ [onefield] Test_Onefield_mtmfs : mt-mfs with minor cycle iterations """
          self.prepData('refim_twochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,deconvolver='mtmfs',interactive=0)
          self.checkall(ret=ret, peakres=0.392, modflux=0.732, iterdone=10, imexist=[self.img+'.psf.tt0', self.img+'.residual.tt0', self.img+'.image.tt0', self.img+'.model.tt0',self.img+'.model.tt1',self.img+'.alpha'], imval=[(self.img+'.psf.tt0',1.0,[50,50,0,0]),(self.img+'.psf.tt1',1.039e-05,[50,50,0,0])])
          ## iterdone=11 only because of the return (iterdone_p+1) in MultiTermMatrixCleaner::mtclean() !

     def test_onefield_autonames(self):
          """ [onefield] Test_Onefield_autonames : Test auto increment of image names """
          self.prepData('refim_twochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',overwrite=False)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',overwrite=False)
          self.checkall(imexist=[self.img+'.psf',self.img+'_2.psf',self.img+'_3.psf'] )

     def test_onefield_twoMS(self):
          """ [onefield] Test_Onefield_twoMS : One field, two input MSs """
          ms1 = 'refim_point_onespw0.ms'
          ms2 = 'refim_point_onespw1.ms'
          self.prepData(ms1)
          self.prepData(ms2)
#          try:
#               ## This run should fail with an exception
#               ret = tclean(vis=[ms1,ms2],field='0',spw=['0','0'], imagename=self.img,imsize=100,cell='8.0arcsec',deconvolver='hogbom',niter=10)
#               correct=False
#          except Exception as e:
#              correct=True
#          self.assertTrue(correct)
          ## This run should go smoothly.
          ret = tclean(vis=[ms1,ms2],field='0',spw=['0','0'], imagename=self.img,imsize=100,cell='8.0arcsec',deconvolver='hogbom',niter=10,datacolumn='data')
          self.checkall(imexist=[self.img+'.psf',self.img+'.residual'])
          self.delData(ms1)
          self.delData(ms2)


     def test_onefield_restart_mfs(self):
          """ [onefield] : test_onefield_restart_mfs : Check calcpsf,calcres and ability to restart and continue"""
          ## TODO : Need to add and use info in the return record, when only a major cycle is done. Then check nmajorcycle.
          self.prepData('refim_twochan.ms')

          ## Only psf
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=0,interactive=0,calcpsf=True,calcres=False)
          self.checkall(imexist=[self.img+'.psf'], imexistnot=[self.img+'.residual', self.img+'.image'],nmajordone=1)

          ## Only residual
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=0,interactive=0,calcpsf=False,calcres=True)
          self.checkall(imexist=[self.img+'.psf', self.img+'.residual'], imexistnot=[self.img+'.image'],nmajordone=1)

          ## Start directly with minor cycle and do only the last major cycle.
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,interactive=0,calcpsf=False,calcres=False)
          self.checkall(ret=ret, peakres=0.392, modflux=0.732, imexist=[self.img+'.psf',self.img+'.residual', self.img+'.image'],nmajordone=1)

          ## Re-start from existing model image and continue on...
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,interactive=0,calcpsf=False,calcres=False)
          self.checkall(ret=ret, peakres=0.161, modflux=0.991, imexist=[self.img+'.psf',self.img+'.residual', self.img+'.image'],nmajordone=1)


     def test_onefield_restart_mtmfs(self):
          """ [onefield] : test_onefield_restart_mtmfs : Check calcpsf,calcres and ability to restart and continue"""
          ## TODO : Need to add and use info in the return record, when only a major cycle is done. Then check nmajorcycle.
          self.prepData('refim_twochan.ms')

          ## Only psf
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=0,interactive=0,calcpsf=True,calcres=False,deconvolver='mtmfs')
          self.checkall(imexist=[self.img+'.psf.tt0', self.img+'.psf.tt1'], imexistnot=[self.img+'.residual.tt0', self.img+'.image.tt0'],nmajordone=1)

          ## Only residual
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=0,interactive=0,calcpsf=False,calcres=True,deconvolver='mtmfs')
          self.checkall(imexist=[self.img+'.psf.tt0',self.img+'.psf.tt1', self.img+'.residual.tt0', self.img+'.residual.tt1'], imexistnot=[self.img+'.image.tt0'],nmajordone=1)

          ## Start directly with minor cycle and do only the last major cycle.
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,interactive=0,calcpsf=False,calcres=False,deconvolver='mtmfs')
          self.checkall(ret=ret, peakres=0.392, modflux=0.732, imexist=[self.img+'.psf.tt1',self.img+'.residual.tt1', self.img+'.image.tt1', self.img+'.alpha'],nmajordone=1,imval=[(self.img+'.alpha',-1.0,[50,50,0,0])])

          ## Re-start from existing model image and continue on...
          ## ( If restart from modified residuals... the alpha is -1.25xx which is wrong. 
          ##   In this case, need to do calcres=True which will do extra first major cycle (nmajor=2) )
          ## But... current code (as of r33373) makes appropriate restored image but does not mess up residuals.
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,interactive=0,calcpsf=False,calcres=False,deconvolver='mtmfs')
          self.checkall(ret=ret, peakres=0.136, modflux=0.988, imexist=[self.img+'.psf.tt1',self.img+'.residual.tt1', self.img+'.image.tt1', self.img+'.alpha'],nmajordone=1,imval=[(self.img+'.alpha',-1.0,[50,50,0,0])])



##############################################
##############################################

##Task level tests : iteration controls
class test_iterbot(testref_base):

     def test_iterbot_mfs_1(self):
          """ [iterbot] Test_Iterbot_Mfs_1 : Zero Iterations """
          self.prepData('refim_twochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=0,interactive=0)
          self.checkall(imexist=[self.img+'.psf', self.img+'.residual'], imexistnot=[self.img+'.image'])

     def test_iterbot_mfs_2(self):
          """ [iterbot] Test_Iterbot_Mfs_2 : Iterations with low gain """
          self.prepData('refim_twochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,gain=0.1,interactive=0)
          self.checkall(ret=ret, peakres=0.392, modflux=0.732, iterdone=10,nmajordone=2,imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image'])

     def test_iterbot_mfs_3(self):
          """ [iterbot] Test_Iterbot_Mfs_3 : Cycleniter test """
          self.prepData('refim_twochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,cycleniter=3,interactive=0)
          self.checkall(ret=ret, peakres=0.392, modflux=0.732, iterdone=10, nmajordone=5,imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image'])

     def test_iterbot_mfs_4(self):
          """ [iterbot] Test_Iterbot_Mfs_4 : Iterations with high gain """
          self.prepData('refim_twochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, gain=0.5,interactive=0)
          self.checkall(ret=ret, peakres=0.026, modflux=1.274, iterdone=10, nmajordone=3,imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image'])

     def test_iterbot_mfs_5(self):
          """ [iterbot] Test_Iterbot_Mfs_5 : Threshold test """
          self.prepData('refim_twochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,threshold='0.1Jy',gain=0.5,interactive=0)
          self.checkall(ret=ret, peakres=0.0924, modflux=1.129, iterdone=5, nmajordone=2,imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image'])

     def test_iterbot_mfs_6(self):
          """ [iterbot] Test_Iterbot_Mfs_6 : Cycleniter and threshold """
          self.prepData('refim_twochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, cycleniter=3, threshold='0.1Jy',gain=0.5,interactive=0)
          self.checkall(ret=ret, peakres=0.0924, modflux=1.129, iterdone=5, nmajordone=3,imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image'])

     def test_iterbot_mfs_7(self):
          """ [iterbot] Test_Iterbot_Mfs_7 : Threshold + cyclefactor to trigger major cycles earlier """
          self.prepData('refim_twochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,threshold='0.01Jy', gain=0.5,cyclefactor=10.0,interactive=0)
          self.checkall(ret=ret, peakres=0.026, modflux=1.274, iterdone=10, nmajordone=4,imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image'])

     def test_iterbot_mfs_8(self):
          """ [iterbot] Test_Iterbot_Mfs_8 : minpsffraction to trigger major cycles earlier. """
          self.prepData('refim_twochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=20,threshold='0.01Jy', minpsffraction = 0.5,interactive=0)
          self.checkall(ret=ret, peakres=0.16127, modflux=0.9919, iterdone=20, nmajordone=4,imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image'])

     def test_iterbot_mfs_9(self):
          """ [iterbot] Test_Iterbot_Mfs_9 : maxpsffraction """
          self.prepData('refim_twochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=20,threshold='0.01Jy', minpsffraction=0.8,maxpsffraction=0.5,interactive=0)
          self.checkall(ret=ret, peakres=0.16127, modflux=0.9919, iterdone=20, nmajordone=4,imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image'])

     def test_iterbot_cube_1(self):
          """ [iterbot] Test_Iterbot_cube_1 : iteration counting across channels (>niter) """
          self.prepData('refim_point_withline.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',specmode='cube',niter=10,threshold='0.75Jy',interactive=0)
          self.checkall(ret=ret, iterdone=90,nmajordone=2,imexist=[self.img+'.psf', self.img+'.residual'])
          ## Only chans 6 and 7 reach cycleniter, others reach threshold in fewer than 10 iters per chan.

     def test_iterbot_cube_2(self):
          """ [iterbot] Test_Iterbot_cube_2 : High threshold, iterate only on line channels. """
          self.prepData('refim_point_withline.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',specmode='cube',niter=10,threshold='1.75Jy',interactive=0)
          self.checkall(ret=ret, peakres=1.73, modflux=0.407,iterdone=12,nmajordone=2,imexist=[self.img+'.psf', self.img+'.residual'])


     def test_iterbot_mfs_deconvolvers(self):
          """ [iterbot] : test_iterbot_deconvolvers : Do all minor cycle algorithms respond in the same way to iteration controls ? No ! """
          # clark and hogbom reach niter first, but multiscale gets to cyclethreshold first. Check peakres and iterdone.
          self.prepData('refim_twochan.ms')
          ret1 = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,threshold='0.1Jy', interactive=0,deconvolver='clark')
          self.checkall(ret=ret1, peakres=0.3922, modflux=0.732, iterdone=10, nmajordone=2,imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image'])

          ret2 = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,threshold='0.1Jy', interactive=0,deconvolver='hogbom')
          self.checkall(ret=ret2, peakres=0.148, modflux=1.008, iterdone=10, nmajordone=2,imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image'])

          ret3 = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,threshold='0.1Jy', interactive=0,deconvolver='multiscale')
          self.checkall(ret=ret3, peakres=0.0984, modflux=1.096, iterdone=8, nmajordone=2,imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image'])
     
          
##############################################
##############################################

##Task level tests : multi-field, 2chan.
### For some of these tests, do the same with uvsub and compare ? 
class test_multifield(testref_base):
     
     def test_multifield_both_mfs(self):
          """ [multifield] Test_Multifield_both_mfs : Two fields, both mfs """
          self.prepData("refim_twopoints_twochan.ms")
          self.write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nnchan=1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nmask=circle[[40pix,40pix],10pix]')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",outlierfile=self.img+'.out.txt',niter=10,deconvolver='hogbom',interactive=0)
          self.checkall(ret=ret, 
                        iterdone=20, ## should be 20 but for the iter+1 returned from mtcleaner...
                        nmajordone=2,
                        imexist=[self.img+'.image', self.img+'1.image'],
                        imval=[(self.img+'.image',1.075,[50,50,0,0]),
                               (self.img+'1.image',5.590,[40,40,0,0])])


     def test_multifield_both_mtmfs(self):
          """ [multifield] Test_Multifield_both_mtmfs : Two fields, both mt-mfs """
          self.prepData("refim_twopoints_twochan.ms")
          self.write_file(self.img+'.out.txt', 'imagename='+self.img+'1\n\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nmask=circle[[40pix,40pix],10pix]')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",outlierfile=self.img+'.out.txt',niter=10,deconvolver='mtmfs',interactive=0)
          self.checkall(ret=ret, 
                        iterdone=20,
                        nmajordone=2,
                        imexist=[self.img+'.image.tt0', self.img+'1.image.tt0',self.img+'.image.tt1', self.img+'1.image.tt1', self.img+'.alpha', self.img+'1.alpha'],
                        imval=[(self.img+'.image.tt0',1.094,[50,50,0,0]),
                               (self.img+'1.image.tt0',5.577,[40,40,0,0]),
                               (self.img+'.alpha',-0.90,[50,50,0,0]),
                               (self.img+'1.alpha',-1.0,[40,40,0,0])])


     def test_multifield_both_cube(self):
          """ [multifield] Test_Multifield_both_cube : Two fields, both cube"""
          self.prepData("refim_twopoints_twochan.ms")
          self.write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\n')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",outlierfile=self.img+'.out.txt',niter=10,deconvolver='hogbom',interactive=0,specmode='cube',nchan=2,interpolation='nearest')
          self.checkall(ret=ret, 
                        iterdone=38,
                        nmajordone=2,
                        imexist=[self.img+'.image', self.img+'1.image'],
                        imval=[(self.img+'.image',1.434,[50,50,0,0]),
                               (self.img+'1.image',7.452,[40,40,0,0]),
                               (self.img+'.image',0.762,[50,50,0,1]),
                               (self.img+'1.image',3.702,[40,40,0,1]) ])

     def test_multifield_cube_mfs(self):
          """ [multifield] Test_Multifield_cube_mfs : Two fields, one cube and one mfs"""
          self.prepData("refim_twopoints_twochan.ms")
          self.write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nspecmode=mfs\n')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",outlierfile=self.img+'.out.txt',niter=10,deconvolver='hogbom',interactive=0,specmode='cube',nchan=2,interpolation='nearest')
          self.checkall(ret=ret, 
                        iterdone=30,
                        nmajordone=2,
                        imexist=[self.img+'.image', self.img+'1.image'],
                        imval=[(self.img+'.image',1.4,[50,50,0,0]),
                               (self.img+'1.image',5.6,[40,40,0,0]),
                               (self.img+'.image',0.75,[50,50,0,1])])

     def test_multifield_mfs_mtmfs(self):
          """ [multifield] Test_Multifield_mfs_mtmfs : Two fields, one mt-mfs and one mfs (i.e. different deconvolvers)"""
          self.prepData("refim_twopoints_twochan.ms")
          self.write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nreffreq=1.5GHz\ndeconvolver=mtmfs\n')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",outlierfile=self.img+'.out.txt',niter=10,deconvolver='hogbom',interactive=0)
          self.checkall(ret=ret, 
                        iterdone=20,
                        nmajordone=2,
                        imexist=[self.img+'.image', self.img+'1.image.tt0',self.img+'1.alpha'],
                        imval=[(self.img+'.image',1.094,[50,50,0,0]),
                               (self.img+'1.image.tt0',5.57,[40,40,0,0]), 
                               (self.img+'1.alpha', -1.0, [40,40,0,0])  ])

     def test_multifield_cube_mtmfs(self):
          """ [multifield] Test_Multifield_cube_mtmfs : Two fields, one cube and one mtmfs"""
          self.prepData("refim_twopoints_twochan.ms")
          self.write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nreffreq=1.5GHz\ndeconvolver=mtmfs\nspecmode=mfs\n')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",outlierfile=self.img+'.out.txt',niter=10,deconvolver='hogbom',interactive=0,specmode='cube',nchan=2,interpolation='nearest')
          self.checkall(ret=ret, 
                        iterdone=30,  # two chans in one field, and one chan in the other
                        nmajordone=2,
                        imexist=[self.img+'.image', self.img+'1.image.tt0',self.img+'1.alpha'],
                        imval=[(self.img+'.image',1.427,[50,50,0,0]),
                               (self.img+'1.image.tt0',5.575,[40,40,0,0]),
                               (self.img+'.image',0.762,[50,50,0,1]) , 
                               (self.img+'1.alpha', -1.0, [40,40,0,0])  ])


     def test_multifield_diff_gridders(self):
          """ [multifield] Test_Multifield_diff_gridders : Two fields, both mfs, gridft and wproject """
          self.prepData("refim_twopoints_twochan.ms")
#          ##Outlier uses gridft
          self.write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nmask=circle[[40pix,40pix],10pix]\ngridder=gridft')
#          ## Outlier uses wproject but with different number of planes as the main field
#          self.write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nmask=circle[[40pix,40pix],10pix]\ngridder=wproject\nwprojplanes=6')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",outlierfile=self.img+'.out.txt',niter=10,deconvolver='hogbom',gridder='wproject',wprojplanes=4,interactive=0)
          self.checkall(ret=ret, 
                        iterdone=20,
                        nmajordone=2,
                        imexist=[self.img+'.image', self.img+'1.image'],
                        imval=[(self.img+'.image',1.075,[50,50,0,0]),
                               (self.img+'1.image',5.590,[40,40,0,0])])


     def test_multifield_autonames(self):
          """ [multifield] Test_Multifield_4 : Test auto increment of image names """
          self.prepData("refim_twopoints_twochan.ms")
          self.write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nnchan=1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nmask=circle[[40pix,40pix],10pix]')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',outlierfile=self.img+'.out.txt')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',outlierfile=self.img+'.out.txt',overwrite=False)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',overwrite=False) # no outlier...
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',outlierfile=self.img+'.out.txt',overwrite=False)

          self.checkall(imexist=[self.img+'.psf',self.img+'1.psf',self.img+'_2.psf',self.img+'1_2.psf',self.img+'_3.psf',self.img+'_4.psf',self.img+'1_4.psf'], imexistnot=[self.img+'1_3.psf'] )


### TODO :  Either put a check so that if any fields overlap, an error is thrown. Or, do sensible model choosing for some modes but detect and complain for other modes where it's harder to pick which model image to use.
     def test_multifield_overlap_mfs(self):
          """ [multifield] Test_Multifield_overlap_mfs : Two overlapping image fields, both mfs """
          self.prepData("refim_twopoints_twochan.ms")
          self.write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[200,200]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:59:02.426 +40.51.14.559')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',phasecenter="J2000 19:58:39.580 +40.55.55.931",outlierfile=self.img+'.out.txt',niter=20,deconvolver='hogbom',interactive=0)

          self.checkall(ret=ret, 
                        iterdone=40, ## both images see the brightest source. 
                        nmajordone=2,
                        imexist=[self.img+'.image', self.img+'1.image'],
                        imval=[(self.img+'.image',5.575,[48,51,0,0]),
                               (self.img+'1.image',5.574,[130,136,0,0])]) ## both images have correct flux (not twice or zero !)


     def test_multifield_overlap_mtmfs(self):
          """ [multifield] Test_Multifield_overlap_mtmfs : Two overlapping image fields, both mt-mfs """
          self.prepData("refim_twopoints_twochan.ms")
          self.write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[200,200]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:59:02.426 +40.51.14.559\n')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',phasecenter="J2000 19:58:39.580 +40.55.55.931",outlierfile=self.img+'.out.txt',niter=20,deconvolver='mtmfs',interactive=0)
          self.checkall(ret=ret, 
                        iterdone=40, ## both images see the brightest source.
                        nmajordone=2,
                        imexist=[self.img+'.image.tt0', self.img+'1.image.tt0'],
                        imval=[(self.img+'.image.tt0',5.53,[48,51,0,0]),
                                (self.img+'1.image.tt0',5.53,[130,136,0,0]),
                               (self.img+'.alpha',-0.965,[48,51,0,0]),
                               (self.img+'1.alpha',-0.965,[130,136,0,0])]) 

##############################################
##############################################

##Task level tests : Stokes imaging options
class test_stokes(testref_base):

     def test_stokes_mfs_I(self):
          """ [onefield] Test_Stokes_I_mfs mfs with stokes I"""
          self.prepData('refim_point_linRL.ms')
          tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, stokes='I')
          self.checkall(imexist=[self.img+'.image'],imval=[(self.img+'.image',1.0,[50,50,0,0])])

     def test_stokes_mfs_IV(self):
          """ [onefield] Test_Stokes_mfs_IV : mfs with stokes IV"""
          self.prepData('refim_point_linRL.ms')
          tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, stokes='IV')
          self.checkall(imexist=[self.img+'.image'],imval=[(self.img+'.image',1.0,[50,50,0,0]),(self.img+'.image',4.0,[50,50,1,0])  ])

#     def test_stokes_mfs_Q(self):
#          """ [onefield] Test_Stokes_mfs_Q : mfs with stokes Q"""
#          self.prepData('refim_point_linRL.ms')
#          tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, stokes='Q')
#          self.checkall(imexist=[self.img+'.image'],imval=[(self.img+'.image',2.0,[50,50,0,0]) ] )

     def test_stokes_cube_I(self):
          """ [onefield] Test_Stokes_cube_I : cube with stokes I"""
          self.prepData('refim_point_linRL.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, stokes='I',interactive=0,specmode='cube',interpolation='nearest')
          self.checkall(imexist=[self.img+'.image'],imval=[(self.img+'.image',1.0,[50,50,0,0]),(self.img+'.image',1.0,[50,50,0,1]),(self.img+'.image',1.0,[50,50,0,2]) ] )

     def test_stokes_cube_IV(self):
          """ [onefield] Test_Stokes_stokes_IV : cube with stokes V"""
          self.prepData('refim_point_linRL.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, stokes='IV',interactive=0,specmode='cube',interpolation='nearest')
          self.checkall(imexist=[self.img+'.image'],imval=[(self.img+'.image',1.0,[50,50,0,0]),(self.img+'.image',1.0,[50,50,0,1]),(self.img+'.image',1.0,[50,50,0,2]),  (self.img+'.image',4.0,[50,50,1,0]),(self.img+'.image',4.0,[50,50,1,1]),(self.img+'.image',4.0,[50,50,1,2])] )

#     def test_stokes_cube_Q(self):
#          """ [onefield] Test_Stokes_cube_Q : cube with stokes Q"""
#          self.prepData('refim_point_linRL.ms')
#          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, stokes='Q',interactive=0,specmode='cube',interpolation='nearest')
#          self.checkall(imexist=[self.img+'.image'],imval=[(self.img+'.image',2.0,[50,50,0,0]),(self.img+'.image',2.0,[50,50,0,1]) ,(self.img+'.image',2.0,[50,50,0,2]) ])

     def test_stokes_cube_IQUV_fromRL(self):
          """ [onefield] Test_Stokes_cube_IQUV_fromRL : cube with stokes IQUV"""
          self.prepData('refim_point_linRL.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, stokes='IQUV',interactive=0,specmode='cube')
          self.checkall(imexist=[self.img+'.image'],imval=[(self.img+'.image',1.0,[50,50,0,1]),(self.img+'.image',2.0,[50,50,1,1]), (self.img+'.image',3.0,[50,50,2,1]),(self.img+'.image',4.0,[50,50,3,1]) ])

     def test_stokes_cube_IQUV_fromXY(self):
          """ [onefield] Test_Stokes_cube_IQUV_fromXY : cube with stokes IQUV"""
          self.prepData('refim_point_linXY.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, stokes='IQUV',interactive=0,specmode='cube')
          self.checkall(imexist=[self.img+'.image'],imval=[(self.img+'.image',1.0,[50,50,0,1]),(self.img+'.image',2.0,[50,50,1,1]), (self.img+'.image',3.0,[50,50,2,1]),(self.img+'.image',4.0,[50,50,3,1]) ])

#     def test_stokes_cube_I_flags(self):
#          """ [onefield] Test_Stokes_cube_I_flags : cube with stokes I and only XY or YX flagged"""
#          self.prepData('refim_point_linXY.ms')
#          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, stokes='IQUV',interactive=0,specmode='cube')
#          self.checkall(imexist=[self.img+'.image'],imval=[(self.img+'.image',1.0,[50,50,0,1]),(self.img+'.image',2.0,[50,50,1,1]), (self.img+'.image',3.0,[50,50,2,1]),(self.img+'.image',4.0,[50,50,4,1]) ])

#     def test_stokes_cube_pseudo_I_flags(self):
#          """ [onefield] Test_Stokes_cube_pseudo_I_flags : cube with stokes I and one of XX or YY flagged"""
#          self.prepData('refim_point_linXY.ms')
#          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, stokes='IQUV',interactive=0,specmode='cube')
#          self.checkall(imexist=[self.img+'.image'],imval=[(self.img+'.image',1.0,[50,50,0,1]),(self.img+'.image',2.0,[50,50,1,1]), (self.img+'.image',3.0,[50,50,2,1]),(self.img+'.image',4.0,[50,50,4,1]) ])

##############################################
##############################################

##Task level tests : cube.
class test_cube(testref_base):
     
     def setUp(self):
          self.msfile = 'refim_point.ms'
          self.msfile2 = 'refim_Cband.G37line.ms'
          ret = None
          self.img = 'tst'
          self.prepData(self.msfile)

     def test_cube_1(self):
          """ [cube] Test_Cube_1 : 20 basic cube tests """

          ## chan 5 (TOPO)
          qfstart=qa.quantity("1.1GHz")
          #qvstart=qa.quantity("-59958.5km/s")
          # for restf=1.25GHz
          #qvstart=qa.quantity("35975.1km/s")
          # ch10
          qvstart=qa.quantity("16788.4km/s")

          ##mfstart=me.frequency('LSRK',qa.quantity("1.1GHz"))
          mfstart=me.frequency('LSRK',qa.quantity("1.09999GHz"))
          #mvstart=me.radialvelocity('BARY',qa.quantity("-59976.1km/s"))
          mvstart=me.radialvelocity('BARY',qa.quantity("11977.6km/s"))
          #dop = me.todoppler('radio',mfstart,qa.quantity('1.0GHz'))
          mfstart10=me.frequency('LSRK',qa.quantity(" 1.17999GHz"))                                                        
          dop = me.todoppler('radio',mfstart10,qa.quantity('1.25GHz'))                                              
          #1chan width 
          #qvwidth = qa.quantity("11991.700km/s")
          #qvwidth = qa.quantity("4796.7km/s")
          qvwidth = qa.quantity("11991.7km/s")
          mvwidth = me.radialvelocity('TOPO',qvwidth)

          # restf = 1.25GHz
          # vel range: 59961.1 -  -31174.7 km/s (lsrk/radio)
          #            74952.3 -  -28238.3 km/s (lsrk/optical)  

          testList = {
                      0:{'imagename':'Cubetest_chandefstdefwidth','spw':'0','start':0,'width':1,'outframe':'LSRK',
                        'desc':'channel, default start and width, LSRK'},
                      1:{'imagename':'Cubetest_chandefstdefwidthtopo','spw':'0','start':0,'width':1, 'outframe':'TOPO',
                        'desc':'channel, default start and width, TOPO'},
                      2:{'imagename':'Cubetest_chandefstwidth2','spw':'0','start':0,'width':2, 'outframe':'LSRK',
                        'desc':'channel, default start, width=2, LSRK'},
                      3:{'imagename':'Cubetest_chanst5wd1','spw':'0','start':5,'width':1, 'outframe':'LSRK',
                        'desc':'channel, start=5, default width, LSRK'},
                      # this will result in blank channnel images (calcChanFreqs requires start and width in channel       
                      # mode to be given in chan index                                                                 
                      4:{'imagename':'Cubetest_chandefstwd1spwsel','spw':'0:5~19','start':0,'width':1, 'outframe':'LSRK',
                        'desc':'channel, spw=0:5~19, LSRK'},
                      #5:{'imagename':'Cubetest_freqdefstwd2','spw':'0','start':'','width':'40MHz','outframe':'TOPO',
                      #  'desc':'frequency, default start, width=\'40MHz\', TOPO'},
                      # data set changed!
                      5:{'imagename':'Cubetest_freqdefstwd2','spw':'0','start':'','width':'100MHz','outframe':'TOPO',
                        'desc':'frequency, default start, width=\'100MHz\', TOPO'},
                      6:{'imagename':'Cubetest_freqst5defwd','spw':'0','start':'1.1GHz','width':'','outframe':'TOPO',
                        'desc':'frequency, start=\'1.1GHz\', default width, TOPO'},
                      7:{'imagename':'Cubetest_freqst5defwdspwsel','spw':'0:4~19','start':'1.1GHz','width':'','outframe':'TOPO',
                        'desc':'frequency, start=\'1.1GHz\', default width, spw=0:6~19, TOPO'},
                      8:{'imagename':'Cubetest_freqst10wdm','spw':'0','start':'1.2GHz','width':'-50MHz','outframe':'TOPO',
                        'desc':'frequency, start=\'1.2GHz\', width=\'-50MHz\', TOPO'},
                      9:{'imagename':'Cubetest_veldefstwd2','spw':'0','start':'','width':'23983.4km/s','outframe':'TOPO',
                        'desc':'frequency, default start, width=\'23983.4km/s\', TOPO'},
                     #10:{'imagename':'Cubetest_veldefstwd2m','spw':'0','start':'','width':'-9593.40km/s','outframe':'TOPO',
                     #   'desc':'velocity, default start, width=\'-9593.40m/s\', TOPO'},
                     10:{'imagename':'Cubetest_veldefstwd2m','spw':'0','start':'','width':'-23983.4km/s','outframe':'TOPO',
                        'desc':'velocity, default start, width=\'-23983.4m/s\', TOPO'},
                     11:{'imagename':'Cubetest_velst10defwd','spw':'0','start':'11991.7km/s','width':'','outframe':'TOPO',
                        'desc':'velocity, start=\'11991.7km/s\', default width, TOPO'},
                     12:{'imagename':'Cubetest_velst10defwdbary','spw':'0','start':'11977.6km/s','width':'','outframe':'BARY',
                        'desc':'velocity, start=\'11977.6km/s\', default width, BARY'},
                     # currently 13 is not quite properly working, investigating - 2014.08.27 TT 
                     # skip 13 for now
                     13: {},
                     # for refim_point.ms ch10=-499626km/s (opt)
                     #13:{'imagename':'Cubetest_optvelst10wdeflsrk','spw':'0','start':'-49962.6km/s','width':'',
                     ##13:{'imagename':'Cubetest_optvelst10wdeflsrk','spw':'0','start':'12494.8km/s','width':'',
                     ##13:{'imagename':'Cubetest_optvelst0defwdlsrk','spw':'0','start':'26072.5km/s','width':'8817km/s',
                     ##13:{'imagename':'Cubetest_optvelst2defwdlsrk','spw':'0','start':'132605km/s','width':'-8817km/s',
                     #   'veltype':'optical','outframe':'LSRK',
                     ##   'desc':'velocity, start=\'74952.3km/s\', default width, veltype=optical LSRK'},
                     #   'desc':'velocity, start=\'-49962.6km/s\', default width, veltype=optical LSRK'},
                     14:{'imagename':'Cubetest_stqfreqdefwd','spw':'0','start':qfstart,'width':'', 'veltype':'radio','outframe':'',
                        'desc':'frequency, start=%s, default width, veltype=radio TOPO' % qfstart},
                     15:{'imagename':'Cubetest_stmfreqdefwd','spw':'0','start':mfstart,'width':'', 'veltype':'radio','outframe':'',
                        'desc':'frequency, start=%s, default width, veltype=radio LSRK' % mfstart},
                     16:{'imagename':'Cubetest_stqveldefwd','spw':'0','start':qvstart,'width':'','outframe':'TOPO',
                        'desc':'velocity, start=%s, default width, TOPO' % qvstart},
                     17:{'imagename':'Cubetest_stmveldefwd','spw':'0','start':mvstart,'width':'','outframe':'TOPO',
                        'desc':'velocity, start=%s, default width, BARY' % mvstart},
                     18:{'imagename':'Cubetest_veldefstqvwidth','spw':'0','start':'','width':qvwidth,'outframe':'TOPO',
                        'desc':'velocity, default start, width=%s, TOPO' % qvwidth},
                     19:{'imagename':'Cubetest_veldefstmvwidth','spw':'0','start':'','width':mvwidth,'outframe':'TOPO',
                        'desc':'velocity, default start, width=%s, TOPO' % mvwidth},
                     20:{'imagename':'Cubetest_stdopdefwd','spw':'0','start':dop,'width':'','outframe':'TOPO',
                        'desc':'doppler, start=%s, default width, TOPO' % dop},
                     # with a gap in spw channel sel
                     21:{'imagename':'Cubetest_st5gap','spw':'0:5~9,12~14','start':5,'width':'','outframe':'LSRK',
                        'desc':'channel, start=%s, default width, channel gap (10-11) LSRK' % 5},
                     # stride > 1
                     22:{'imagename':'Cubetest_st5gap','spw':'0:0~10^2','start':0,'width':'','outframe':'LSRK',
                        'desc':'channel, start=%s, default width, step=2 LSRK' % 0}
                    }

          ## Run all tests.
          skipped = []
          passed = []
          for testid in testList:
               if testList[testid] != {}:
                    if testList[testid].has_key('veltype'):
                         inveltype=testList[testid]['veltype']
                    else:
                         inveltype='radio'

                    print "\n *** %s (testid=%s)" % (testList[testid]['imagename'], testid)
                    ret = tclean(vis=self.msfile,field='0',imsize=100,cell='8.0arcsec',niter=10,specmode='cube',nchan=10,restfreq=['1.25GHz'],phasecenter="J2000 19:59:28.500 +40.44.01.50",deconvolver='hogbom',spw=testList[testid]['spw'],imagename=self.img+testList[testid]['imagename'],start=testList[testid]['start'], width=testList[testid]['width'],veltype=inveltype,outframe=testList[testid]['outframe'])
                    self.assertTrue(os.path.exists(self.img+testList[testid]['imagename']+'.psf') and os.path.exists(self.img+testList[testid]['imagename']+'.residual') )
                    passed.append(testid)
               else:
                    print "sub test #:", testid, 'was skipped'
                    skipped.append(testid)

          print "Passed: testid=",passed
          print "Skipped: testid=",skipped
              
     def test_cube_D1(self):
          """ [cube] Test_Cube_D1 : specmode cubedata - No runtime doppler corrections """
          ret = tclean(vis=self.msfile2,field='1',spw='0:105~135',specmode='cubedata',nchan=30,start=105,width=1,veltype='radio',imagename=self.img,imsize=256,cell='0.01arcmin',phasecenter=1,deconvolver='hogbom',niter=10)
          self.assertTrue(os.path.exists(self.img+'.psf') and os.path.exists(self.img+'.residual') )

     def test_cube_D2(self):
          """ [cube] Test_Cube_D2 : specmode cube - WITH doppler corrections """
          ret = tclean(vis=self.msfile2,field='1',spw='0:105~135',specmode='cube',nchan=30,start=105,width=1,veltype='radio',imagename=self.img,imsize=256,cell='0.01arcmin',phasecenter=1,deconvolver='hogbom',niter=10)
          self.assertTrue(os.path.exists(self.img+'.psf') and os.path.exists(self.img+'.residual') )

     def test_cube_D3(self):
          """ [cube] Test_Cube_D3 : specmode cubesrc - Doppler correct to a SOURCE ephemeris """
#          ret = tclean(vis=self.msfile,field='1',spw='0:105~135',specmode='cubesrc',nchan=30,start=105,width=1,veltype='radio',imagename=self.img,imsize=256,cell='0.01arcmin',phasecenter=1,deconvolver='hogbom',niter=10)
#          self.assertTrue(os.path.exists(self.img+'.psf') and os.path.exists(self.img+'.residual') )

     def test_cube_continuum_subtract_uvsub(self):
          """ [cube] Test_Cube_continuum_subtract :  Using uvsub """
          self.prepData('refim_point_withline.ms')
          self.delmodels(msname=self.msfile)
          plotms(vis=self.msfile,xaxis='frequency',yaxis='amp',ydatacolumn='data',customsymbol=True,symbolshape='circle',symbolsize=5,showgui=False,plotfile=self.img+'.plot.step0data.png',title="original data")
          plotms(vis=self.msfile,xaxis='frequency',yaxis='amp',ydatacolumn='model',customsymbol=True,symbolshape='circle',symbolsize=5,showgui=False,plotfile=self.img+'.plot.step0model.png',title="empty model")

          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec', spw='0:12~19',niter=50,gain=0.2,savemodel='modelcolumn',deconvolver='mtmfs')
#          self.assertTrue(self.exists(self.img+'.model') )
#          self.assertTrue( self.checkmodelchan(self.msfile,10) == 0.0 and self.checkmodelchan(self.msfile,3) > 0.0 )
          plotms(vis=self.msfile,xaxis='frequency',yaxis='amp',ydatacolumn='model',customsymbol=True,symbolshape='circle',symbolsize=5,showgui=False,plotfile=self.img+'.plot.step1.png',title="model after partial mtmfs on some channels")

          self.delmodels(msname=self.msfile)
          ret = tclean(vis=self.msfile,imagename=self.img+'1',imsize=100,cell='8.0arcsec',startmodel=self.img, spw='0',niter=0,savemodel='modelcolumn',deconvolver='mtmfs')
#          self.assertTrue( self.checkmodelchan(self.msfile,10) > 0.0 and self.checkmodelchan(self.msfile,3) > 0.0 
          plotms(vis=self.msfile,xaxis='frequency',yaxis='amp',ydatacolumn='model',customsymbol=True,symbolshape='circle',symbolsize=5,showgui=False,plotfile=self.img+'.plot.step2.png',title="model after mtmfs predict on full spw" )

          plotms(vis=self.msfile,xaxis='frequency',yaxis='amp',ydatacolumn='data-model',customsymbol=True,symbolshape='circle',symbolsize=5,showgui=False,plotfile=self.img+'.plot.step3data.png',title="data-model")
          

     def test_cube_continuum_subtract_otf(self):
          """ [cube] Test_Cube_continuum_subtract :  On-The-Fly using multifield """
          self.prepData('refim_pointline.ms')

     def test_cube_badchannel_restoringbeam(self):
          """ [cube] Test auto restoring beam with a bad edge channel """
          self.prepData('refim_point.ms')

##############################################
##############################################

##Task level tests : masks and clean boxes.
class test_mask(testref_base):

     def test_mask_1(self):
          """ [mask] test_mask_1 : Input mask as file and string : mfs """

     def test_mask_2(self):
          """ [mask] test_mask_2 :  Input mask as file and string : cube (few channels) """

     def test_mask_3(self):
          """ [mask] test_mask_3 : Input mask as image-to-be-regridded (ra/dec) : mfs """

     def test_mask_4(self):
          """ [mask] test_mask_4 :  Input mask as image-to-be-regridded(ra/dec/specframe) : cube """

     def test_mask_autobox(self):
          """ [mask] test_mask_autobox :  Autobox """

     def test_mask_pbmask(self):
          """ [mask] test_mask_pbmask :  pb mask """

     def test_mask_combined_1(self):
          """ [mask] test_mask_combined_1 :  string + pbmask """

     def test_mask_combined_2(self):
          """ [mask] test_mask_combined_2 :  Autobox + pbmask """

     def test_mask_outlier(self):
          """ [mask] test_mask_outlier : With outlier fields """



##############################################
##############################################

##Task level tests : awproject and mosaics
class test_widefield(testref_base):
     
     def test_widefield_wproj_mfs(self):
          """ [widefield] Test_Widefield_wproj : W-Projection """ 
          ### Need better test dataset for this.....
          self.prepData("refim_twopoints_twochan.ms")
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=200,cell='8.0arcsec',phasecenter="J2000 19:59:00.2 +40.50.15.50",niter=30,gridder='widefield',wprojplanes=4,deconvolver='hogbom')
          self.checkall(imexist=[self.img+'.image'],imval=[(self.img+'.psf',1.0,[100,100,0,0]),(self.img+'.image',5.56,[127,143,0,0]) ] )

     def test_widefield_facets_mfs(self):
          """ [multifield] Test_Multifield_mfs_facets : Facetted imaging (mfs) """
          self.prepData("refim_twopoints_twochan.ms")
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=200,cell='8.0arcsec',phasecenter="J2000 19:59:00.2 +40.50.15.50",facets=2,deconvolver='hogbom',niter=30)
          self.checkall(imexist=[self.img+'.image', self.img+'.psf'],imval=[(self.img+'.psf',1.0,[100,100,0,0]),(self.img+'.image',5.56,[127,143,0,0]) ] )

     def test_widefield_facets_mtmfs(self):
          """ [multifield] Test_facets_mtmfs : Facetted imaging (mt-mfs) """
          self.prepData("refim_twopoints_twochan.ms")
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=200,cell='8.0arcsec',phasecenter="J2000 19:59:00.2 +40.50.15.50",facets=2,deconvolver='mtmfs',niter=30)
          self.checkall(imexist=[self.img+'.image.tt0', self.img+'.psf.tt0', self.img+'.alpha'],imval=[(self.img+'.psf.tt0',1.0,[100,100,0,0]),(self.img+'.image.tt0',5.56,[127,143,0,0]),(self.img+'.alpha',-1.0,[127,143,0,0]) ] )

     def test_widefield_aproj_mfs(self):
          """ [widefield] Test_Widefield_aproj : MFS with narrowband AWProjection (wbawp=F, 1spw)  stokes I """
          self.prepData("refim_mawproject.ms")
##          ret = tclean(vis=self.msfile,spw='1',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=30,gridder='awproject',cfcache=self.img+'.cfcache',wbawp=False,conjbeams=True,psterm=False,computepastep=360.0,rotatepastep=360.0,deconvolver='hogbom')
          ret = tclean(vis=self.msfile,spw='2',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=30,gridder='awproject',wbawp=False,conjbeams=True,psterm=False,computepastep=360.0,rotatepastep=360.0,deconvolver='hogbom')
          self.checkall(imexist=[self.img+'.image', self.img+'.psf', self.img+'.weight'],imval=[(self.img+'.image',1.0,[256,256,0,0]),(self.img+'.weight',0.493,[256,256,0,0]) ] )
          ## weight is pbsq which is 0.7^2 = 0.49 (approx).

          #do stokes V too.....
     def test_widefield_aproj_cube(self):
          """ [widefield] Test_Widefield_mosaic_cube_aproj : Cube with AW-Projection  and rotation off """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='*',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",specmode='cube',niter=1,gain=1.0,gridder='awproject',cfcache=self.img+'.cfcache',wbawp=True,conjbeams=False,psterm=False,computepastep=360.0,rotatepastep=360.0,deconvolver='hogbom')
          self.assertTrue(os.path.exists(self.img+'.psf') and os.path.exists(self.img+'.residual') )

     ## Test normtype too somewhere..


     def test_widefield_wbaproj_mfs(self):
          """ [widefield] Test_Widefield_wbaproj_mfs : MFS with wideband AWProjection (wbawp=T, allspw) and nt=1 stokes I  """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='*',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=30,gridder='awproject',cfcache=self.img+'.cfcache',wbawp=True,conjbeams=True,psterm=False,computepastep=360.0,rotatepastep=360.0,deconvolver='hogbom',pblimit=0.3)
          self.checkall(imexist=[self.img+'.image', self.img+'.psf', self.img+'.weight'],imval=[(self.img+'.image',1.0,[256,256,0,0]),(self.img+'.weight',0.493,[256,256,0,0]) ] )


          #do stokes V too..
     
     ## CHECK NORMALIZATION OF WEIGHTIMAGE = normed to peak=1
     ## TODO : make vpman recognize EVLA in addition to VLA.
     def test_widefield_mosaicft_mfs(self):
          """ [widefield] Test_Widefield_mosaic : MFS with mosaicft  stokes I """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='1',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=30,gridder='mosaicft',deconvolver='hogbom',pblimit=0.3)
          self.checkall(imexist=[self.img+'.image', self.img+'.psf', self.img+'.weight'],imval=[(self.img+'.image',0.9743,[256,256,0,0]),(self.img+'.weight',0.392,[256,256,0,0]) ] )
          #ret = clean(vis=self.msfile,spw='1',field='*',imagename=self.img+'.old',imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=30,imagermode='mosaic',psfmode='hogbom')

          #do stokes V too..

     def test_widefield_mosaicft_mtmfs(self):
          """ [widefield] Test_Widefield_mosaicft_mtmfs : MT-MFS with mosaicft  stokes I, alpha """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='*',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=60,gridder='mosaicft',deconvolver='mtmfs')
          self.checkall(imexist=[self.img+'.image.tt0', self.img+'.psf.tt0', self.img+'.weight.tt0'],imval=[(self.img+'.image.tt0',0.93,[256,256,0,0]),(self.img+'.weight.tt0',0.392,[256,256,0,0]),(self.img+'.alpha',0.0,[256,256,0,0]) ] )
          ## alpha should represent that of the mosaic PB (twice)... and should then converge to zero
          

     def test_widefield_mosaicft_cube(self):
          """ [widefield] Test_Widefield_mosaicft_cube : MFS with mosaicft  stokes I """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='*',field='0',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",specmode='cube',niter=10,gridder='mosaicft',deconvolver='hogbom',gain=0.1)
          self.checkall(imexist=[self.img+'.image', self.img+'.psf', self.img+'.weight'],imval=[(self.img+'.image',0.9743,[256,256,0,0]),(self.img+'.weight',0.46,[256,256,0,0]) ] )

          #do stokes V too..

class test_widefield_failing(testref_base):

     def test_widefield_imagemosaic(self):
          """ [widefield] Test_Widefield_imagemosaic : Image domain mosaic for single-term mfs (or narrowband)  """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='1',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=30,gridder='imagemosaic',deconvolver='hogbom')
          self.checkall(imexist=[self.img+'.image', self.img+'.psf', self.img+'.weight'],imval=[(self.img+'.image',1.0,[256,256,0,0]),(self.img+'.weight',0.493,[256,256,0,0]) ] )

     def test_widefield_aproj_mtmfs(self):
          """ [widefield] Test_Widefield_aproj_mtmfs : MFS with AWProjection (wbawp=T,conjbeams=F, allspw) and nt=2 stokes I  """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='*',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=60,gridder='awproject',cfcache=self.img+'.cfcache',wbawp=True,conjbeams=False,psterm=False,computepastep=360.0,rotatepastep=360.0,deconvolver='mtmfs')
          self.checkall(imexist=[self.img+'.image.tt0', self.img+'.psf.tt0', self.img+'.weight.tt0'],imval=[(self.img+'.image.tt0',1.0,[256,256,0,0]),(self.img+'.weight.tt0',0.49,[256,256,0,0]),(self.img+'.alpha',0.0,[256,256,0,0]) ] )
          ## alpha should represent that of the mosaic PB (twice).. -0.1 doesn't look right. Sigh.... well.. it should converge to zero.

     def test_widefield_wbaproj_mtmfs(self):
          """ [widefield] Test_Widefield_wbaproj_mtmfs : MFS with wideband AWProjection (wbawp=T,conjbeams=T, allspw) and nt=2 stokes I  """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='*',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=30,gridder='awproject',cfcache=self.img+'.cfcache',wbawp=True,conjbeams=True,psterm=False,computepastep=360.0,rotatepastep=360.0,deconvolver='mtmfs',pblimit=0.1)
          self.checkall(imexist=[self.img+'.image.tt0', self.img+'.psf.tt0', self.img+'.weight.tt0'],imval=[(self.img+'.image.tt0',0.95,[256,256,0,0]),(self.img+'.weight.tt0',0.49,[256,256,0,0]),(self.img+'.alpha',0.0009,[256,256,0,0]) ] )
          ## alpha should be ZERO as the pb spectrum has been taken out.


     def test_widefield_mosaic_outlier(self):
          """ [multifield] Test_widefield_mosaic_outlier : Mosaic with an outlier field """
          #### Need another dataset for this.
          self.prepData("refim_mawproject.ms")
          ## Outlier uses gridft
          self.write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:42.0 +40.55.58.543\nmask=circle[[40pix,40pix],10pix]\ngridder=gridft')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",outlierfile=self.img+'.out.txt',niter=10,deconvolver='hogbom',gridder='mosaicft',interactive=0)
          self.checkall(ret=ret, 
                        iterdone=10,  # outlier field has nothing in it :).
                        nmajordone=2,
                        imexist=[self.img+'.image', self.img+'1.image'],
                        imval=[(self.img+'.image',0.933,[256,256,0,0]),
                               (self.img+'1.image',0.0,[40,40,0,0])])
          #### NOT WORKING as the model is being picked from tst1, so residual after 0.6 model and one major cycle is same as dirty and the output image is 1.6 instead of 0.93.

##############################################
##############################################

##Task level tests : model prediction.
class test_modelvis(testref_base):
     
     def test_modelvis_1(self):
          """ [modelpredict] Test_modelvis_1 : mfs with no save model """
          self.prepData("refim_twochan.ms")
          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,savemodel='none')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==False )

     def test_modelvis_2(self):
          """ [modelpredict] Test_modelvis_2 : mfs with save model column """
          self.prepData("refim_twochan.ms")

          ## Save model after deconvolution
          self.delmodels(self.msfile,True)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,savemodel='modelcolumn')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==True and modsum>0.0 and hasvirmod==False )

          ##Predict from input model image (startmodel)
          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img, niter=0,savemodel='modelcolumn')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==True and modsum>0.0 and hasvirmod==False )

     def test_modelvis_3(self):
          """ [modelpredict] Test_modelvis_3 : mfs with save virtual model """
          self.prepData("refim_twochan.ms")

          ## Save model after deconvolution
          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,savemodel='virtual')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==True )

          ##Predict from input model image (startmodel)
          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img, niter=0,savemodel='virtual')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==True )

     def test_modelvis_4(self):
          """ [modelpredict] Test_modelvis_4 : mt-mfs with no save model """
          self.prepData("refim_twochan.ms")
          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,deconvolver='mtmfs',savemodel='none')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==False )

     def test_modelvis_5(self):
          """ [modelpredict] Test_modelvis_5 : mt-mfs with save model column """
          self.prepData("refim_twochan.ms")
          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,deconvolver='mtmfs',savemodel='modelcolumn')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==True and modsum>0.0 and hasvirmod==False )

          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img,niter=0,deconvolver='mtmfs',savemodel='modelcolumn')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==True and modsum>0.0 and hasvirmod==False )

     def test_modelvis_6(self):
          """ [modelpredict] Test_modelvis_6 : mt-mfs with save virtual model """
          self.prepData("refim_twochan.ms")
          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,deconvolver='mtmfs',savemodel='virtual')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==True )

          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img,niter=0,deconvolver='mtmfs',savemodel='virtual')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==True )

     def test_modelvis_7(self):
          """ [modelpredict] Test_modelvis_7 : cube with chan selection and save model column """
          ## check explicit channels ...
          self.prepData("refim_point.ms")
          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,spw='0:5~12',imagename=self.img,imsize=100,cell='8.0arcsec',specmode='cube',niter=10,savemodel='modelcolumn',start=5,nchan=8,interpolation='nearest')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==True and modsum>0.0 and hasvirmod==False )
          self.checkchanvals([(10,">",0.0),(3,"==",1.0)])
          
          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,spw='0',imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img,specmode='cube',niter=0,savemodel='modelcolumn')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==True and modsum>0.0 and hasvirmod==False )
          self.checkchanvals([(10,">",0.0),(3,"==",self.checkmodelchan(self.msfile,1))])


     def test_modelvis_8(self):
          """ [modelpredict] Test_modelvis_8 : cube with chan selection and save virtual model """
          ## check explicit channels ...
          self.prepData("refim_point.ms")
          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,spw='0:5~12',imagename=self.img,imsize=100,cell='8.0arcsec',specmode='cube',niter=10,savemodel='virtual')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==True )

          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,spw='0',imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img,specmode='cube',niter=0,savemodel='virtual')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==True )

     def test_modelvis_9(self):
          """ [modelpredict] Test_modelvis_9 : Don't de-grid channels with zero model. Also test limited-freq mask """
          self.prepData("refim_point.ms")
          masklist=''  # choose only a few channels here.
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,mask=masklist)
          self.assertTrue(self.exists(self.img+'.model') )

          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img+'1',imsize=100,cell='8.0arcsec',startmodel=self.img,niter=0,savemodel='modelcolumn')

          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img,niter=0,savemodel='virtual')

     def test_modelvis_10(self):
          """ [modelpredict] Test_modelvis_10 : Use input model of different (narrower) freq range than data """
          self.prepData("refim_point.ms")
          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec', spw='0:5~12',niter=10,savemodel='modelcolumn')
          self.assertTrue(self.exists(self.img+'.model') )
#          self.assertTrue( self.checkmodelchan(self.msfile,10) > 0.0 and self.checkmodelchan(self.msfile,3) == 1.0 )
          self.checkchanvals([(10,">",0.0),(3,"==",1.0)])

          ## add model expansion parameter
          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img+'1',imsize=100,cell='8.0arcsec',startmodel=self.img, spw='0',niter=0,savemodel='modelcolumn')
#          self.assertTrue( self.checkmodelchan(self.msfile,10) > 0.0 and self.checkmodelchan(self.msfile,3) > 0.0 )
          self.checkchanvals([(10,">",0.0),(3,">",0.0)])

          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img, spw='0',niter=0,savemodel='virtual')
          ## cannot check anything here....  just that it runs without error

     def test_modelvis_11(self):
          """ [modelpredict] Test_modelvis_11 : Predict model image over channel gaps not included in imaging """
          self.prepData("refim_point.ms")
          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec', spw='0:0~8;12~19',niter=10,savemodel='modelcolumn')
          self.assertTrue(self.exists(self.img+'.model') )
          self.assertTrue( self.checkmodelchan(self.msfile,10) == 0.0 and self.checkmodelchan(self.msfile,3) > 0.0 )

          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img+'1',imsize=100,cell='8.0arcsec',startmodel=self.img, spw='0',niter=0,savemodel='modelcolumn')
          self.assertTrue( self.checkmodelchan(self.msfile,10) > 0.0 and self.checkmodelchan(self.msfile,3) > 0.0 )

          self.delmodels(msname=self.msfile,dmodcol=True)
          ret = tclean(vis=self.msfile,imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img, spw='0',niter=0,savemodel='virtual')
          ## cannot check anything here....  just that it runs without error

class test_modelvis_failing(testref_base):
     def test_modelvis_12(self):
          """ [modelpredict] Test_modelvis_12 : Regrid input model onto new image grid : mfs (ra/dec) """

     def test_modelvis_13(self):
          """ [modelpredict] Test_modelvis_13 : Regrid input model onto new image grid : cube (ra/dec/specframe)"""

     def test_modelvis_14(self):
          """ [modelpredict] Test_modelvis_14 : Regrid input model onto new image grid : mtmfs (ra/dec/terms)"""

     def test_modelvis_startmodel_mfs(self):
          """ [modelvis] test_modelvis_startmodel_mfs : Restart a run with existing 'startmodel'."""

##############################################
