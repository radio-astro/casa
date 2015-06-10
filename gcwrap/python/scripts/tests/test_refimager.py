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

############################################
############################################
#
# Test Structure
#
# One test suite per MS ( onefield, multifield, cube, mosaic )
#
# Each test suite has a minimal set of tests, followed by specific ones.
#        Minimal set : continuum/cube, one/two stokes , single/multi term,
#                             savemodel options,
#
# Additional options to test for each different MS / test suite
# 
#        'onefield' : facets off/on, weighting, w-proj, deconvolvers, all iteration controls,
#                             niter=0,psf only,niter=10 and cycleniter=3, threshold stop, 
#                         input param type checking, restart options, savemodel
#                         stokes, savemodel
#        'multifield' : outlierfile y/n, uvsub test, overwrite=T/F
#        'cube' : spectral frame defs,data-to-image mapping, empty chans, 
#                     bad psf at edge, restoringbeam options, savemodel, some itercon
#        'mosaic' : mosaic types (stitched, mosft, awp, immos), awp with a,w,ps on/off      
#   
#
############################################
#
#  Datasets
#
#  refim_point_stokes.ms : RR=1.0, LL=0.8, RL and LR are zero. Stokes I=0.9, V=0.1, U,Q=0.0
#  refim_point_linRL.ms : I=1, Q=2, U=3, V=4  in circular pol basis.
#
#
############################################

## List to be run
def suite():
     return [test_onefield, test_iterbot, test_multifield,test_stokes,test_cube, test_widefield,test_mask, test_modelvis]

refdatapath = os.environ.get('CASAPATH').split()[0] + '/data/regression/unittest/clean/'
refdatapath = "/export/home/riya/rurvashi/Work/ImagerRefactor/Runs/UnitData/"

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
          dat = tb.getcol('MODEL_DATA')
          dat.fill( complex(0.0,0.0) )
          tb.putcol('MODEL_DATA', dat)
          tb.close();

     def delmodels(self,msname="",dmodcol=False):
          if dmodcol:
               self.delmodelcol(msname) ## Delete model column
          else:
               self.resetmodelcol(msname)  ## Set model column to zero
          delmod(msname)  ## Get rid of OTF model
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
          tb.open( msname+'/SOURCE' )
          keys = tb.getkeywords()
          if len(keys)>0:
               hasvirmod=True
          else:
               hasvirmod=False
          tb.close()
          print msname , ": modelcol=", hasmodcol, " modsum=", modsum, " virmod=", hasvirmod
          return hasmodcol, modsum, hasvirmod

     def checkmodelchan(self,msname="",chan=0):
          tb.open( msname )
          hasmodcol = (  (tb.colnames()).count('MODEL_DATA')>0 )
          modsum=0.0
          if hasmodcol:
               dat = tb.getcol('MODEL_DATA')[:,chan,:]
               modsum=dat.sum()
          tb.close()
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
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,interactive=0) # default is clark
          self.checkall(ret=ret, peakres=0.392, modflux=0.732, iterdone=10, imexist=[self.img+'.psf', self.img+'.residual', self.img+'.image',self.img+'.model'], imval=[(self.img+'.psf',1.0,[50,50,0,0])])

     def test_onefield_hogbom(self):
          """ [onefield] Test_Onefield_hogbom : mfs with hogbom minor cycle """
          self.prepData('refim_twochan.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,deconvolver='hogbom',interactive=0)
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
          try:
               ## This run should fail with an exception
               ret = tclean(vis=[ms1,ms2],field='0',spw=['0','0'], imagename=self.img,imsize=100,cell='8.0arcsec',deconvolver='hogbom',niter=10)
               correct=False
          except Exception as e:
              correct=True
          self.assertTrue(correct)
          ## This run should go smoothly.
          ret = tclean(vis=[ms1,ms2],field='0',spw=['0','0'], imagename=self.img,imsize=100,cell='8.0arcsec',deconvolver='hogbom',niter=10,datacolumn='data')
          self.checkall(imexist=[self.img+'.psf',self.img+'.residual'])
          self.delData(ms1)
          self.delData(ms2)


     def test_onefield_restart_mfs(self):
          """ [onefield] : test_onefield_restart_mfs : Check recalcpsf,recalcres and ability to restart and continue"""
          ## TODO : Need to add and use info in the return record, when only a major cycle is done. Then check nmajorcycle.
          self.prepData('refim_twochan.ms')

          ## Only psf
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=0,interactive=0,recalcpsf=True,recalcres=False)
          self.checkall(imexist=[self.img+'.psf'], imexistnot=[self.img+'.residual', self.img+'.image'],nmajordone=1)

          ## Only residual
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=0,interactive=0,recalcpsf=False,recalcres=True)
          self.checkall(imexist=[self.img+'.psf', self.img+'.residual'], imexistnot=[self.img+'.image'],nmajordone=1)

          ## Start directly with minor cycle and do only the last major cycle.
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,interactive=0,recalcpsf=False,recalcres=False)
          self.checkall(ret=ret, peakres=0.392, modflux=0.732, imexist=[self.img+'.psf',self.img+'.residual', self.img+'.image'],nmajordone=1)

          ## Re-start from existing model image and continue on...
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,interactive=0,recalcpsf=False,recalcres=False)
          self.checkall(ret=ret, peakres=0.161, modflux=0.991, imexist=[self.img+'.psf',self.img+'.residual', self.img+'.image'],nmajordone=1)


     def test_onefield_restart_mtmfs(self):
          """ [onefield] : test_onefield_restart_mtmfs : Check recalcpsf,recalcres and ability to restart and continue"""
          ## TODO : Need to add and use info in the return record, when only a major cycle is done. Then check nmajorcycle.
          self.prepData('refim_twochan.ms')

          ## Only psf
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=0,interactive=0,recalcpsf=True,recalcres=False,deconvolver='mtmfs')
          self.checkall(imexist=[self.img+'.psf.tt0', self.img+'.psf.tt1'], imexistnot=[self.img+'.residual.tt0', self.img+'.image.tt0'],nmajordone=1)

          ## Only residual
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=0,interactive=0,recalcpsf=False,recalcres=True,deconvolver='mtmfs')
          self.checkall(imexist=[self.img+'.psf.tt0',self.img+'.psf.tt1', self.img+'.residual.tt0', self.img+'.residual.tt1'], imexistnot=[self.img+'.image.tt0'],nmajordone=1)

          ## Start directly with minor cycle and do only the last major cycle.
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,interactive=0,recalcpsf=False,recalcres=False,deconvolver='mtmfs')
          self.checkall(ret=ret, peakres=0.392, modflux=0.732, imexist=[self.img+'.psf.tt1',self.img+'.residual.tt1', self.img+'.image.tt1', self.img+'.alpha'],nmajordone=1,imval=[(self.img+'.alpha',-1.0,[50,50,0,0])])

          ## Re-start from existing model image and continue on...
          ## ( If restart from modified residuals... the alpha is -1.25xx which is wrong. 
          ##   In this case, need to do recalcres=True which will do extra first major cycle (nmajor=2) )
          ## But... current code (as of r33373) makes appropriate restored image but does not mess up residuals.
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,interactive=0,recalcpsf=False,recalcres=False,deconvolver='mtmfs')
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
          write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nnchan=1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nmask=circle[[40pix,40pix],10pix]')
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
          write_file(self.img+'.out.txt', 'imagename='+self.img+'1\n\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nmask=circle[[40pix,40pix],10pix]')
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
          write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\n')
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
          write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nspecmode=mfs\n')
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
          write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nreffreq=1.5GHz\ndeconvolver=mtmfs\n')
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
          write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nreffreq=1.5GHz\ndeconvolver=mtmfs\nspecmode=mfs\n')
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
          write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nmask=circle[[40pix,40pix],10pix]\ngridder=gridft')
#          ## Outlier uses wproject but with different number of planes as the main field
#          write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nmask=circle[[40pix,40pix],10pix]\ngridder=wproject\nwprojplanes=6')
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
          write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nnchan=1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nmask=circle[[40pix,40pix],10pix]')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',outlierfile=self.img+'.out.txt')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',outlierfile=self.img+'.out.txt',overwrite=False)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',overwrite=False) # no outlier...
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',outlierfile=self.img+'.out.txt',overwrite=False)

          self.checkall(imexist=[self.img+'.psf',self.img+'1.psf',self.img+'_2.psf',self.img+'1_2.psf',self.img+'_3.psf',self.img+'_4.psf',self.img+'1_4.psf'], imexistnot=[self.img+'1_3.psf'] )


### TODO :  Either put a check so that if any fields overlap, an error is thrown. Or, do sensible model choosing for some modes but detect and complain for other modes where it's harder to pick which model image to use.
     def test_multifield_overlap_mfs(self):
          """ [multifield] Test_Multifield_overlap_mfs : Two overlapping image fields, both mfs """
          self.prepData("refim_twopoints_twochan.ms")
          write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[200,200]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:59:02.426 +40.51.14.559')
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
          write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[200,200]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:59:02.426 +40.51.14.559\n')
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

     def test_stokes_mfs_Q(self):
          """ [onefield] Test_Stokes_mfs_Q : mfs with stokes Q"""
          self.prepData('refim_point_linRL.ms')
          tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, stokes='Q')
          self.checkall(imexist=[self.img+'.image'],imval=[(self.img+'.image',2.0,[50,50,0,0]) ] )

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

     def test_stokes_cube_Q(self):
          """ [onefield] Test_Stokes_cube_Q : cube with stokes Q"""
          self.prepData('refim_point_linRL.ms')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, stokes='Q',interactive=0,specmode='cube',interpolation='nearest')
          self.checkall(imexist=[self.img+'.image'],imval=[(self.img+'.image',2.0,[50,50,0,0]),(self.img+'.image',2.0,[50,50,0,1]) ,(self.img+'.image',2.0,[50,50,0,2]) ])

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

     def test_stokes_cube_I_flags(self):
          """ [onefield] Test_Stokes_cube_I_flags : cube with stokes I and only XY or YX flagged"""
          self.prepData('refim_point_linXY.ms')
#          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10, stokes='IQUV',interactive=0,specmode='cube')
#          self.checkall(imexist=[self.img+'.image'],imval=[(self.img+'.image',1.0,[50,50,0,1]),(self.img+'.image',2.0,[50,50,1,1]), (self.img+'.image',3.0,[50,50,2,1]),(self.img+'.image',4.0,[50,50,4,1]) ])

     def test_stokes_cube_pseudo_I_flags(self):
          """ [onefield] Test_Stokes_cube_pseudo_I_flags : cube with stokes I and one of XX or YY flagged"""
          self.prepData('refim_point_linXY.ms')
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
          qvwidth = qa.quantity("4796.7km/s")
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
                      5:{'imagename':'Cubetest_freqdefstwd2','spw':'0','start':'','width':'40MHz','outframe':'TOPO',
                        'desc':'frequency, default start, width=\'40MHz\', TOPO'},
                      6:{'imagename':'Cubetest_freqst5defwd','spw':'0','start':'1.1GHz','width':'','outframe':'TOPO',
                        'desc':'frequency, start=\'1.1GHz\', default width, TOPO'},
                      7:{'imagename':'Cubetest_freqst5defwdspwsel','spw':'0:4~19','start':'1.1GHz','width':'','outframe':'TOPO',
                        'desc':'frequency, start=\'1.1GHz\', default width, spw=0:6~19, TOPO'},
                      8:{'imagename':'Cubetest_freqst10wdm','spw':'0','start':'1.2GHz','width':'-20MHz','outframe':'TOPO',
                        'desc':'frequency, start=\'1.2GHz\', width=\'-20MHz\', TOPO'},
                      9:{'imagename':'Cubetest_veldefstwd2','spw':'0','start':'','width':'9593.4km/s','outframe':'TOPO',
                        'desc':'frequency, default start, width=\'9593.4km/s\', TOPO'},
                     10:{'imagename':'Cubetest_veldefstwd2m','spw':'0','start':'','width':'-9593.40km/s','outframe':'TOPO',
                        'desc':'velocity, default start, width=\'-9593.40m/s\', TOPO'},
                     11:{'imagename':'Cubetest_velst10defwd','spw':'0','start':'11991.7km/s','width':'','outframe':'TOPO',
                        'desc':'velocity, start=\'11991.7km/s\', default width, TOPO'},
                     12:{'imagename':'Cubetest_velst10defwdbary','spw':'0','start':'11977.6km/s','width':'','outframe':'BARY',
                        'desc':'velocity, start=\'11977.6km/s\', default width, BARY'},
                     # currently 13 is not quite properly working, investigating - 2014.08.27 TT 
                     #13:{'imagename':'Cubetest_optvelst10defwdlsrk','spw':'0','start':'-49962.6km/s','width':'4234.40km/s',
                     ##13:{'imagename':'Cubetest_optvelst10defwdlsrk','spw':'0','start':'-49962.6km/s','width':'5500.0km/s',
                     ##13:{'imagename':'Cubetest_optvelst10defwdlsrk','spw':'0','start':'74952.3km/s','width':'5500.0km/s',
                     ##   'veltype':'optical','outframe':'LSRK',
                     ##   'desc':'velocity, start=\'74952.3km/s\', default width, veltype=optical LSRK'},
                     ### smallest w in vopt = 4792.2km/s (ch19-20), largest w in vopt: 8817km/s (ch0-1)
                     ##13:{'imagename':'Cubetest_optvelst10wdeflsrk','spw':'0','start':'74952.3km/s','width':'',
                     13:{'imagename':'Cubetest_optvelst10wdeflsrk','spw':'0','start':'12494.8km/s','width':'',
                     #13:{'imagename':'Cubetest_optvelst0defwdlsrk','spw':'0','start':'26072.5km/s','width':'8817km/s',
                     #13:{'imagename':'Cubetest_optvelst2defwdlsrk','spw':'0','start':'132605km/s','width':'-8817km/s',
                        'veltype':'optical','outframe':'LSRK',
                     ##   'desc':'velocity, start=\'74952.3km/s\', default width, veltype=optical LSRK'},
                        'desc':'velocity, start=\'12494.8km/s\', default width, veltype=optical LSRK'},
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
                     # this also fails, investigating - 2014.08.27 TT
                     20:{'imagename':'Cubetest_stdopdefwd','spw':'0','start':dop,'width':'','outframe':'TOPO',
                        'desc':'doppler, start=%s, default width, LSRK' % dop}
                    }

          ## Run all tests.
          for testid in testList:

               if testList[testid].has_key('veltype'):
                    inveltype=testList[testid]['veltype']
               else:
                    inveltype='radio'

               print "Cube test : "+testList[testid]['imagename']
               ret = tclean(vis=self.msfile,field='0',imsize=100,cell='8.0arcsec',niter=10,specmode='cube',nchan=10,restfreq=['1.25GHz'],phasecenter="J2000 19:59:28.500 +40.44.01.50",deconvolver='hogbom',spw=testList[testid]['spw'],imagename=self.img+testList[testid]['imagename'],start=testList[testid]['start'], width=testList[testid]['width'],veltype=inveltype,outframe=testList[testid]['outframe'])
               self.assertTrue(os.path.exists(self.img+'.psf') and os.path.exists(self.img+'.residual') )

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

     def test_cube_continuum_subtract(self):
          """ [cube] Test_Cube_continuum_subtract :  """
          self.prepData('refim_pointline.ms')

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

     def test_widefield_imagemosaic(self):
          """ [widefield] Test_Widefield_imagemosaic : Image domain mosaic for single-term mfs (or narrowband)  """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='1',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=30,gridder='imagemosaic',deconvolver='hogbom')
          self.checkall(imexist=[self.img+'.image', self.img+'.psf', self.img+'.weight'],imval=[(self.img+'.image',1.0,[256,256,0,0]),(self.img+'.weight',0.493,[256,256,0,0]) ] )

     ## CHECK NORMALIZATION OF WEIGHTIMAGE = not normed... but internal norming happens to peak=1
     def test_widefield_mosaic_aproj(self):
          """ [widefield] Test_Widefield_mosaic_aproj : MFS with narrowband AWProjection (wbawp=F, 1spw)  stokes I """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='1',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=30,gridder='awproject',cfcache=self.img+'.cfcache',wbawp=False,conjbeams=True,psterm=False,computepastep=360.0,rotatepastep=5.0,deconvolver='hogbom')
          self.checkall(imexist=[self.img+'.image', self.img+'.psf', self.img+'.weight'],imval=[(self.img+'.image',1.0,[256,256,0,0]),(self.img+'.weight',0.493,[256,256,0,0]) ] )
          ## weight is pbsq which is 0.7^2 = 0.49 (approx).

          #do stokes V too.....

     ## CHECK NORMALIZATION OF WEIGHTIMAGE = normed to peak=1
     ## TODO : make vpman recognize EVLA in addition to VLA.
     def test_widefield_mosaicft(self):
          """ [widefield] Test_Widefield_mosaic : MFS with mosaicft  stokes I """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='1',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=30,gridder='mosaicft',deconvolver='hogbom')
          self.checkall(imexist=[self.img+'.image', self.img+'.psf', self.img+'.weight'],imval=[(self.img+'.image',0.9743,[256,256,0,0]),(self.img+'.weight',0.955,[256,256,0,0]) ] )

          #do stokes V too..

     def test_widefield_mosaic_wbaproj_mfs(self):
          """ [widefield] Test_Widefield_3 : MFS with wideband AWProjection (wbawp=T, allspw) and nt=1 stokes I  """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='*',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=30,gridder='awproject',cfcache=self.img+'.cfcache',wbawp=True,conjbeams=True,psterm=False,computepastep=360.0,rotatepastep=5.0,deconvolver='hogbom')
          self.checkall(imexist=[self.img+'.image', self.img+'.psf', self.img+'.weight'],imval=[(self.img+'.image',1.0,[256,256,0,0]),(self.img+'.weight',0.493,[256,256,0,0]) ] )


          #do stokes V too..
     
     def test_widefield_mosaic_wbaproj_mtmfs(self):
          """ [widefield] Test_Widefield_mosaic_wbaproj_mtmfs : MFS with wideband AWProjection (wbawp=T,conjbeams=T, allspw) and nt=2 stokes I  """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='*',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=30,gridder='awproject',cfcache=self.img+'.cfcache',wbawp=True,conjbeams=True,psterm=False,computepastep=360.0,rotatepastep=5.0,deconvolver='mtmfs')
          self.checkall(imexist=[self.img+'.image.tt0', self.img+'.psf.tt0', self.img+'.weight.tt0'],imval=[(self.img+'.image.tt0',1.0,[256,256,0,0]),(self.img+'.weight.tt0',0.49,[256,256,0,0]),(self.img+'.alpha',0.0,[256,256,0,0]) ] )
          ## alpha should be ZERO as the pb spectrum has been taken out.

     def test_widefield_mosaic_aproj_mtmfs(self):
          """ [widefield] Test_Widefield_mosaic_aproj_mtmfs : MFS with AWProjection (wbawp=T,conjbeams=F, allspw) and nt=2 stokes I  """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='*',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=60,gridder='awproject',cfcache=self.img+'.cfcache',wbawp=True,conjbeams=False,psterm=False,computepastep=360.0,rotatepastep=5.0,deconvolver='mtmfs')
          self.checkall(imexist=[self.img+'.image.tt0', self.img+'.psf.tt0', self.img+'.weight.tt0'],imval=[(self.img+'.image.tt0',1.0,[256,256,0,0]),(self.img+'.weight.tt0',0.49,[256,256,0,0]),(self.img+'.alpha',0.0,[256,256,0,0]) ] )
          ## alpha should represent that of the mosaic PB (twice).. -0.1 doesn't look right. Sigh.... well.. it should converge to zero.

     def test_widefield_mosaicft_mtmfs(self):
          """ [widefield] Test_Widefield_mosaicft_mtmfs : MT-MFS with mosaicft  stokes I, alpha """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='*',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",niter=60,gridder='mosaicft',deconvolver='mtmfs')
          self.checkall(imexist=[self.img+'.image.tt0', self.img+'.psf.tt0', self.img+'.weight.tt0'],imval=[(self.img+'.image.tt0',1.0,[256,256,0,0]),(self.img+'.weight.tt0',1.0,[256,256,0,0]),(self.img+'.alpha',0.0,[256,256,0,0]) ] )
          ## alpha should represent that of the mosaic PB (twice)... and should then converge to zero
          

     def test_widefield_mosaicft_cube(self):
          """ [widefield] Test_Widefield_mosaicft_cube : MFS with mosaicft  stokes I """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='*',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",specmode='cube',niter=30,gridder='mosaicft',deconvolver='hogbom')
          self.checkall(imexist=[self.img+'.image', self.img+'.psf', self.img+'.weight'],imval=[(self.img+'.image',0.9743,[256,256,0,0]),(self.img+'.weight',0.955,[256,256,0,0]) ] )

          #do stokes V too..

     def test_widefield_aproj_cube(self):
          """ [widefield] Test_Widefield_mosaic_cube_aproj : Cube with AW-Projection  and rotation off """
          self.prepData("refim_mawproject.ms")
          ret = tclean(vis=self.msfile,spw='*',field='*',imagename=self.img,imsize=512,cell='10.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",specmode='cube',niter=30,gridder='awproject',cfcache=self.img+'.cfcache',wbawp=False,conjbeams=False,psterm=False,computepastep=360.0,rotatepastep=360.0,deconvolver='hogbom')
          self.assertTrue(os.path.exists(self.img+'.psf') and os.path.exists(self.img+'.residual') )

     ## Test normtype too somewhere..

     def test_widefield_mosaic_outlier(self):
          """ [multifield] Test_widefield_mosaic_outlier : Mosaic with an outlier field """
          #### Need another dataset for this.
          self.prepData("refim_mawproject.ms")
          ## Outlier uses gridft
          write_file(self.img+'.out.txt', 'imagename='+self.img+'1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nmask=circle[[40pix,40pix],10pix]\ngridder=gridft')
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',phasecenter="J2000 19:59:28.500 +40.44.01.50",outlierfile=self.img+'.out.txt',niter=10,deconvolver='hogbom',gridder='mosaicft',interactive=0)
          self.checkall(ret=ret, 
                        iterdone=10,  # outlier field has nothing in it :).
                        nmajordone=2,
                        imexist=[self.img+'.image', self.img+'1.image'],
                        imval=[(self.img+'.image',1.075,[50,50,0,0]),
                               (self.img+'1.image',5.590,[40,40,0,0])])




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
          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img, niter=0,savemodel='modelcolumn')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==True and modsum>0.0 and hasvirmod==False )

     def test_modelvis_3(self):
          """ [modelpredict] Test_modelvis_3 : mfs with save virtual model """
          self.prepData("refim_twochan.ms")

          ## Save model after deconvolution
          self.delmodels(self.msfile,True)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,savemodel='virtual')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==True )

          ##Predict from input model image (startmodel)
          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img, niter=0,savemodel='virtual')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==True )

     def test_modelvis_4(self):
          """ [modelpredict] Test_modelvis_4 : mt-mfs with no save model """
          self.prepData("refim_twochan.ms")
          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,deconvolver='mtmfs',savemodel='none')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==False )

     def test_modelvis_5(self):
          """ [modelpredict] Test_modelvis_5 : mt-mfs with save model column """
          self.prepData("refim_twochan.ms")
          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,deconvolver='mtmfs',savemodel='modelcolumn')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==True and modsum>0.0 and hasvirmod==False )

          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img,niter=0,deconvolver='mtmfs',savemodel='modelcolumn')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==True and modsum>0.0 and hasvirmod==False )

     def test_modelvis_6(self):
          """ [modelpredict] Test_modelvis_6 : mt-mfs with save virtual model """
          self.prepData("refim_twochan.ms")
          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,deconvolver='mtmfs',savemodel='virtual')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==True )

          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img,niter=0,deconvolver='mtmfs',savemodel='virtual')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==True )

     def test_modelvis_7(self):
          """ [modelpredict] Test_modelvis_7 : cube with chan selection and save model column """
          ## check explicit channels ...
          self.prepData("refim_point.ms")
          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,spw='0:5~12',imagename=self.img,imsize=100,cell='8.0arcsec',specmode='cube',niter=10,savemodel='modelcolumn')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==True and modsum>0.0 and hasvirmod==False )
          self.assertTrue( self.checkmodelchan(self.msfile,10) > 0.0 and self.checkmodelchan(self.msfile,3) == 0.0 )
          
          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,spw='0',imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img,specmode='cube',niter=0,savemodel='modelcolumn')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==True and modsum>0.0 and hasvirmod==False )
          self.assertTrue( self.checkmodelchan(self.msfile,10) > 0.0 and self.checkmodelchan(self.msfile,3) == 0.0 )


     def test_modelvis_8(self):
          """ [modelpredict] Test_modelvis_8 : cube with chan selection and save virtual model """
          ## check explicit channels ...
          self.delmodels(self.msfile)
          self.prepData("refim_point.ms")
          ret = tclean(vis=self.msfile,spw='0:5~12',imagename=self.img,imsize=100,cell='8.0arcsec',specmode='cube',niter=10,savemodel='virtual')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==True )

          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,spw='0',imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img,specmode='cube',niter=0,savemodel='virtual')
          hasmodcol, modsum, hasvirmod = self.checkmodel(self.msfile)
          self.assertTrue( hasmodcol==False and hasvirmod==True )

     def test_modelvis_9(self):
          """ [modelpredict] Test_modelvis_9 : Don't de-grid channels with zero model. Also test limited-freq mask """
          self.prepData("refim_point.ms")
          masklist=''  # choose only a few channels here.
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec',niter=10,mask=masklist)
          self.assertTrue(self.exists(self.img+'.model') )

          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,imagename=self.img+'1',imsize=100,cell='8.0arcsec',startmodel=self.img,niter=0,savemodel='modelcolumn')

          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img,niter=0,savemodel='virtual')

     def test_modelvis_10(self):
          """ [modelpredict] Test_modelvis_10 : Use input model of different (narrower) freq range than data """
          self.prepData("refim_point.ms")
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec', spw='0:5~12',niter=10)
          self.assertTrue(self.exists(self.img+'.model') )
          self.assertTrue( self.checkmodelchan(self.msfile,10) > 0.0 and self.checkmodelchan(self.msfile,3) == 0.0 )

          ## add model expansion parameter
          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,imagename=self.img+'1',imsize=100,cell='8.0arcsec',startmodel=self.img, spw='0',niter=0,savemodel='modelcolumn')
          self.assertTrue( self.checkmodelchan(self.msfile,10) > 0.0 and self.checkmodelchan(self.msfile,3) > 0.0 )

          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img, spw='0',niter=0,savemodel='virtual')

     def test_modelvis_11(self):
          """ [modelpredict] Test_modelvis_11 : Predict model image over channel gaps not included in imaging """
          self.prepData("refim_point.ms")
          ret = tclean(vis=self.msfile,imagename=self.img,imsize=100,cell='8.0arcsec', spw='0:0~8;12~19',niter=10)
          self.assertTrue(self.exists(self.img+'.model') )
          self.assertTrue( self.checkmodelchan(self.msfile,10) == 0.0 and self.checkmodelchan(self.msfile,3) > 0.0 )

          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,imagename=self.img+'1',imsize=100,cell='8.0arcsec',startmodel=self.img, spw='0',niter=0,savemodel='modelcolumn')
          self.assertTrue( self.checkmodelchan(self.msfile,10) > 0.0 and self.checkmodelchan(self.msfile,3) > 0.0 )

          self.delmodels(self.msfile)
          ret = tclean(vis=self.msfile,imagename=self.img+'2',imsize=100,cell='8.0arcsec',startmodel=self.img, spw='0',niter=0,savemodel='virtual')

     def test_modelvis_12(self):
          """ [modelpredict] Test_modelvis_12 : Regrid input model onto new image grid : mfs (ra/dec) """

     def test_modelvis_13(self):
          """ [modelpredict] Test_modelvis_13 : Regrid input model onto new image grid : cube (ra/dec/specframe)"""

     def test_modelvis_14(self):
          """ [modelpredict] Test_modelvis_14 : Regrid input model onto new image grid : mtmfs (ra/dec/terms)"""

##############################################
##############################################

##(2) Tool level tests

################################
## Task level tests for the refactored framework
################################





#################################
## Tool level test script for the refactored imager framework
#################################
from refimagerhelper import PySynthesisImager, PyParallelContSynthesisImager,PyParallelCubeSynthesisImager, PyParallelDeconvolver, PyParallelImagerHelper,ImagerParameters

import commands
#### Specify parameters.
def getparams(testnum=1,testid=0, parallelmajor=False,parallelminor=False,parallelcube=False):

     # Iteration parameters - common to all tests below
     niter=200
     cycleniter=100
     threshold=0.001
     loopgain=0.1

     restoringbeam=[]
     #restoringbeam='30.0arcsec'

     # Interaction ON or OFF
     interactive=False
     #mask='ttt.mask'
     mask=''
     #mask = 'circle[[50pix,50pix],10pix]'

     if(testnum==23):  ## Cube with AWP (and mosft)
          casalog.post("==================================");
          casalog.post("Test 23 Cube with AWP (and mosft)");
          casalog.post("==================================");
          paramList = ImagerParameters(msname='DataTest/reg_mawproject.ms', #_offcenter.ms',
                                       field='*',scan='',
                                       spw='*',\
                                       usescratch=True,readonly=True,\
                                       mode='cube',\
                                       imagename='mytest0', nchan=-1,\
                                       imsize=[512,512],\
                                       cell=['10.0arcsec','10.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='awprojectft', 
                                       startmodel='', weighting='natural',\
                                       aterm=True, psterm=False, mterm=True,\
                                       wbawp = False, 
                                       cfcache = "perm.mytest0.cube.cfcache.mos",
                                       dopointing = False, dopbcorr = True, conjbeams = True, 
                                       computepastep =360.0, rotatepastep =5.0,\
                                       deconvolver='hogbom',\
                                       #pblimit=0.1,normtype='flatsky',
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)


     if(testnum==22):  ## 22 image-field, mfs --- readonly/savevirtualmodel/savemodelcolumn.
          casalog.post("==================================");
          casalog.post("Test 22 image-field, mfs --- readonly/savevirtualmodel/savemodelcolumn.");
          casalog.post("==================================");

          msname = 'DataTest/point_twospws.ms'
          resetmodelcol(msname)  ## Set model column to zero
          delmod(msname)  ## Get rid of OTF model
          delmodkeywords(msname) ## Get rid of extra OTF model keywords that sometimes persist...

          print "At start : ", checkmodel(msname)

          testList = {
               ## readonly
               0:{'readonly':True,  'usescratch':True, 'deconvolver':'hogbom', 'ntaylorterms':1, 'mtype':'default'},
               ## readonly
               1:{'readonly':True,  'usescratch':False, 'deconvolver':'hogbom', 'ntaylorterms':1, 'mtype':'default'}, 
               ## save model column in last major cycle
               2:{'readonly':False,  'usescratch':True, 'deconvolver':'hogbom', 'ntaylorterms':1, 'mtype':'default'}, 
               ## save virtual model in last major cycleexit
               3:{'readonly':False,  'usescratch':False, 'deconvolver':'hogbom', 'ntaylorterms':1, 'mtype':'default'}, 
               ## Multi-term test : save model column in last major cycle
               4:{'readonly':False,  'usescratch':True, 'deconvolver':'mtmfs', 'ntaylorterms':2, 'mtype':'multiterm'}, 
               ## Multi-term test : save virtual model in last major cycle
               5:{'readonly':False,  'usescratch':False, 'deconvolver':'mtmfs', 'ntaylorterms':2, 'mtype':'multiterm'} 
               }

          ###  Note : 2 or 3 has a bug. When trying to read the virtual model (in plotms), the plotted values are
          ###           different when using a modelcolumn or a virtual model. Needs more checking.

          if testid > 5:
               print 'No such test.'
               return

          paramList = ImagerParameters(msname=msname,field='0',spw='0',\
                                       usescratch=testList[testid]['usescratch'],readonly=testList[testid]['readonly'],\
                                       mode='mfs',\
                                       ntaylorterms=testList[testid]['ntaylorterms'],mtype=testList[testid]['mtype'],
                                       imagename='mytest0', nchan=1,\
                                       imsize=[110,110],\
                                       cell=['8.0arcsec','8.0arcsec'],stokes='I',\
                                       phasecenter=0,
                                       ftmachine='gridft', startmodel='', weighting='briggs',\
                                       deconvolver=testList[testid]['deconvolver'],\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)
          print 'Run to check model  :  checkmodel("' + msname + '")'

     if(testnum==21):  ## 21 image-field, mfs --- Multiple Stokes planes -- Clark
          casalog.post("==================================");
          casalog.post("Test 21 image-field, mfs --- Multiple Stokes planes -- Clark.");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/point_stokes.ms',field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,\
                                       imsize=[110,110],\
                                       cell=['8.0arcsec','8.0arcsec'],stokes='I',\
                                       phasecenter=0,
                                       ftmachine='gridft', startmodel='', weighting='briggs',\
                                       deconvolver='clarkstokes',
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)


     if(testnum==20):  ## 1 image-field, mfs, multiscale
          casalog.post("==================================");
          casalog.post("Test 20 MFS 1 term + multiscale");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/eptwochan.ms',field='0',spw='0:0',\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,\
                                       imsize=[200,200],\
                                       cell=['8.0arcsec','8.0arcsec'],\
                                       ### center
                                       #phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ### offset
                                       #phasecenter="J2000 19:59:23.591 +40.44.01.50",\
                                       phasecenter=0,
                                       ftmachine='gridft', startmodel='', weighting='natural',\
                                       deconvolver='multiscale',scales=[0,20,40,100],\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)


     if(testnum==19): ## mode=cubesrc (For interface test purpose only, the mode is not fully implemeted yet)
          casalog.post("==================================");
          casalog.post("Test 19 image-field, cubesrc --- Imaging with various cube parameter specifications");
          #casalog.post("==================================");
          paramList = ImagerParameters(msname='DataTest/point_twospws.ms', field='0',\
                                       spw='0',\
                                       usescratch=True,readonly=True,\
                                       imagename="CubesrcTest", mode='cubesrc',\
                                       nchan=10,\
                                       start="1.1GHz",\
                                       width="",\
                                       veltype='radio',\
                                       sysvel="-3000km/s",\
                                       sysvelframe="LSRK",\
                                       restfreq=["1.0GHz"],\
                                       imsize=[100,100],\
                                       cell=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='gridft', startmodel='', weighting='natural',\
                                       deconvolver='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)

     if(testnum==18): ## mode=cube (with doppler correction)
          casalog.post("==================================");
          casalog.post("Test 18 image-field, cube --- With doppler corrections");
          #casalog.post("==================================");
          paramList = ImagerParameters(msname='DataTest/Cband.G37line.DopplerTest.ms', field='1',\
                                       spw='0:105~135',\
                                       usescratch=True,readonly=True,\
                                       imagename="mytest0", mode='cube',\
                                       nchan=30,\
                                       start=105,\
                                       width=1,\
                                       veltype='radio',\
                                       imsize=[256,256],\
                                       cell='0.01arcmin',\
                                       phasecenter=1,
                                       ftmachine='gridft', weighting='natural',\
                                       deconvolver='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)

     if(testnum==17): ## mode=cubedata 
          casalog.post("==================================");
          casalog.post("Test 17 image-field, cubedata --- No runtime dopper corrections");
          #casalog.post("==================================");
          paramList = ImagerParameters(msname='DataTest/Cband.G37line.DopplerTest.ms', field='1',\
                                       spw='0:105~135',\
                                       usescratch=True,readonly=True,\
                                       imagename="mytest0", mode='cubedata',\
                                       nchan=30,\
                                       start=105,\
                                       width=1,\
                                       veltype='radio',\
                                       imsize=[256,256],\
                                       cell='0.01arcmin',\
                                       phasecenter=1,
                                       ftmachine='gridft', weighting='natural',\
                                       deconvolver='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)

     if(testnum==16):  ## 2 image-fields, mfs - one with nterms 1 and one with nterms 2
          casalog.post("==================================");
          casalog.post("Test 16 image-fields, mfs (one with nterms=1. one with nterms=2).");
          casalog.post("==================================");
          
          write_file('out16.txt', 'imagename=mytest1\nnchan=1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nntaylorterms=2\nmtype=multiterm\nreffreq=1.5GHz\ndeconvolver=mtmfs')
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',\
                                       field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       outlierfile='out16.txt',\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,\
                                       imsize=[100,100],\
                                       cell=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='gridft', startmodel='', weighting='natural',\
                                       deconvolver='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                        restoringbeam=restoringbeam,
                                      interactive=interactive,mask=mask)
     
     if(testnum==15):  ## 2 image-fields, mfs, Overlapping models. Both multi-term
          casalog.post("==================================");
          casalog.post("Test 15 two overlapping image-fields, mfs, both multi term");
          casalog.post("==================================");
          
          write_file('out15.txt', 'imagename=mytest1\nnchan=1\nimsize=[200,200]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:59:02.426 +40.51.14.559\n')
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',\
                                       field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       outlierfile='out15.txt',\
                                       mode='mfs',\
                                       ntaylorterms=2,mtype='multiterm',restfreq=['1.5GHz'],\
                                       deconvolver='mtmfs',\
                                       imagename='mytest0', nchan=1,\
                                       imsize=[100,100],\
                                       cell=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:58:39.580 +40.55.55.931",\
                                       ftmachine='gridft', startmodel='', weighting='natural',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)


     if(testnum==14):  ## 2 image-fields, mfs, Overlapping models. Single term
          casalog.post("==================================");
          casalog.post("Test 14 two overlapping image-fields, mfs, single term");
          casalog.post("==================================");
          
          write_file('out14.txt', 'imagename=mytest1\nnchan=1\nimsize=[200,200]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:59:02.426 +40.51.14.559')
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',\
                                       field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       outlierfile='out14.txt',\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,\
                                       imsize=[100,100],\
                                       cell=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:58:39.580 +40.55.55.931",\
                                       ftmachine='gridft', startmodel='', weighting='natural',\
                                       deconvolver='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)


     if(testnum==13): ## 1 image-field, cube --- Imaging with various cube parameter specifications
          casalog.post("==================================");
          casalog.post("Test 13 image-field, cube --- Imaging with various cube parameter specifications");
          #casalog.post("==================================");
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
          qvwidth = qa.quantity("4796.7km/s")
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
                      5:{'imagename':'Cubetest_freqdefstwd2','spw':'0','start':'','width':'40MHz','outframe':'TOPO',
                        'desc':'frequency, default start, width=\'40MHz\', TOPO'},
                      6:{'imagename':'Cubetest_freqst5defwd','spw':'0','start':'1.1GHz','width':'','outframe':'TOPO',
                        'desc':'frequency, start=\'1.1GHz\', default width, TOPO'},
                      7:{'imagename':'Cubetest_freqst5defwdspwsel','spw':'0:4~19','start':'1.1GHz','width':'','outframe':'TOPO',
                        'desc':'frequency, start=\'1.1GHz\', default width, spw=0:6~19, TOPO'},
                      8:{'imagename':'Cubetest_freqst10wdm','spw':'0','start':'1.2GHz','width':'-20MHz','outframe':'TOPO',
                        'desc':'frequency, start=\'1.2GHz\', width=\'-20MHz\', TOPO'},
                      9:{'imagename':'Cubetest_veldefstwd2','spw':'0','start':'','width':'9593.4km/s','outframe':'TOPO',
                        'desc':'frequency, default start, width=\'9593.4km/s\', TOPO'},
                     10:{'imagename':'Cubetest_veldefstwd2m','spw':'0','start':'','width':'-9593.40km/s','outframe':'TOPO',
                        'desc':'velocity, default start, width=\'-9593.40m/s\', TOPO'},
                     11:{'imagename':'Cubetest_velst10defwd','spw':'0','start':'11991.7km/s','width':'','outframe':'TOPO',
                        'desc':'velocity, start=\'11991.7km/s\', default width, TOPO'},
                     12:{'imagename':'Cubetest_velst10defwdbary','spw':'0','start':'11977.6km/s','width':'','outframe':'BARY',
                        'desc':'velocity, start=\'11977.6km/s\', default width, BARY'},
                     # currently 13 is not quite properly working, investigating - 2014.08.27 TT 
                     #13:{'imagename':'Cubetest_optvelst10defwdlsrk','spw':'0','start':'-49962.6km/s','width':'4234.40km/s',
                     ##13:{'imagename':'Cubetest_optvelst10defwdlsrk','spw':'0','start':'-49962.6km/s','width':'5500.0km/s',
                     ##13:{'imagename':'Cubetest_optvelst10defwdlsrk','spw':'0','start':'74952.3km/s','width':'5500.0km/s',
                     ##   'veltype':'optical','outframe':'LSRK',
                     ##   'desc':'velocity, start=\'74952.3km/s\', default width, veltype=optical LSRK'},
                     ### smallest w in vopt = 4792.2km/s (ch19-20), largest w in vopt: 8817km/s (ch0-1)
                     ##13:{'imagename':'Cubetest_optvelst10wdeflsrk','spw':'0','start':'74952.3km/s','width':'',
                     13:{'imagename':'Cubetest_optvelst10wdeflsrk','spw':'0','start':'12494.8km/s','width':'',
                     #13:{'imagename':'Cubetest_optvelst0defwdlsrk','spw':'0','start':'26072.5km/s','width':'8817km/s',
                     #13:{'imagename':'Cubetest_optvelst2defwdlsrk','spw':'0','start':'132605km/s','width':'-8817km/s',
                        'veltype':'optical','outframe':'LSRK',
                     ##   'desc':'velocity, start=\'74952.3km/s\', default width, veltype=optical LSRK'},
                        'desc':'velocity, start=\'12494.8km/s\', default width, veltype=optical LSRK'},
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
                     # this also fails, investigating - 2014.08.27 TT
                     20:{'imagename':'Cubetest_stdopdefwd','spw':'0','start':dop,'width':'','outframe':'TOPO',
                        'desc':'doppler, start=%s, default width, LSRK' % dop}
                    }

          casalog.post("Sub-test "+str(testid)+":"+testList[testid]['desc']);
          casalog.post("==================================");
          if testList[testid].has_key('veltype'):
               inveltype=testList[testid]['veltype']
          else:
               inveltype='radio'
          paramList = ImagerParameters(msname='DataTest/point_twospws.ms', field='0',\
                                       spw=testList[testid]['spw'],\
                                       usescratch=True,readonly=True,\
                                       imagename=testList[testid]['imagename'], mode='cube',\
                                       #nchan=3,\
                                       nchan=10,\
                                       #nchan=20,\
                                       restfreq=['1.25GHz'],\
                                       #restfreq=['1.5GHz'],\
                                       start=testList[testid]['start'],\
                                       width=testList[testid]['width'],\
                                       veltype=inveltype,\
                                       outframe=testList[testid]['outframe'],\
                                       imsize=[100,100],\
                                       cell=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='gridft', startmodel='', weighting='natural',\
                                       deconvolver='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)



     if(testnum==12):  ## 1 image-field, mfs --- WB AWP ( multi term mosaic )
          casalog.post("==================================");
          casalog.post("Test 12 image-field, mfs --- WB AWP( multi term )");
          casalog.post("==================================");
          paramList = ImagerParameters(msname='DataTest/reg_mawproject.ms',
                                       field='*',scan='',
                                       spw='*',\
                                       usescratch=True,readonly=True,\
                                       imagename='mytest0', nchan=1,\
                                       imsize=[512,512],\
                                       cell=['10.0arcsec','10.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ntaylorterms=2,mtype='multiterm',restfreq=['1.5GHz'],\
                                       ftmachine='awprojectft', 
                                       startmodel='', weighting='natural',\
                                       aterm=True, psterm=False, mterm=True,\
                                       wbawp = True, 
                                       cfcache = "perm.wb.nt2.mytest0.cfcache",\
                                       dopointing = False, dopbcorr = True, conjbeams = True, 
                                       computepastep =360.0, rotatepastep =360.0,#5.0,\
                                       deconvolver='mtmfs',\
                                       pblimit=0.1,normtype='flatnoise',
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)

     if(testnum==11):  ## 1 image-field, mfs --- WB AWP ( single term )
          casalog.post("==================================");
          casalog.post("Test 11 image-field, mfs --- WB AWP (single term)");
          casalog.post("==================================");
          ## Note : For this test, if doMajor peak comes to 0.006 instead of 0.69, 
          ##           it means unnormalized weight is being used (refilled perhaps but not normed).
          paramList = ImagerParameters(msname='DataTest/reg_mawproject.ms',
                                       field='1',scan='',
                                       spw='*',\
                                       usescratch=True,readonly=True,\
                                       imagename='mytest0', nchan=1,start='1.5GHz', width='1.0GHz',\
                                       imsize=[512,512],\
                                       cell=['10.0arcsec','10.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='awprojectft', 
                                       startmodel='', weighting='natural',\
                                       aterm=True, psterm=False, mterm=True,\
                                       wbawp = True, 
                                       cfcache = "perm.wb.mytest0.cfcache",\
                                       dopointing = False, dopbcorr = True, conjbeams = True, 
                                       computepastep =360.0, rotatepastep =5.0,\
                                       deconvolver='hogbom',\
                                       pblimit=0.1,normtype='flatnoise',
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)

     if(testnum==10):  ## 1 image-field, mfs --- Narrow-band AWP
          casalog.post("==================================");
          casalog.post("Test 10 image-field, mfs --- WB AWP");
          casalog.post("==================================");
          paramList = ImagerParameters(msname='DataTest/reg_mawproject.ms', #_offcenter.ms',
                                       field='*',scan='',
                                       spw='1',\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,\
                                       imsize=[512,512],\
                                       cell=['10.0arcsec','10.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       stokes='V',\
                                       ftmachine='awprojectft', 
                                       startmodel='', weighting='natural',\
                                       aterm=True, psterm=False, mterm=True,\
                                       wbawp = False, 
                                       cfcache = "perm.mytest0.cfcache.mos",#.offcenter",\
                                       dopointing = False, dopbcorr = True, conjbeams = True, 
                                       computepastep =360.0, rotatepastep =5.0,\
                                       deconvolver='hogbom',\
                                       #pblimit=0.1,normtype='flatsky',
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)


     if(testnum==9):  ## Image domain mosaic for single-term (narrow band )
          casalog.post("==================================");
          casalog.post("Test 9 : Image domain mosaic for single-term");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/reg_mawproject.ms',\
                                       field='',spw='1',scan='',\
#                                       usescratch=True,readonly=True,\
                                       savemodel='none',\
                                       specmode='mfs',\
                                       imagename='mytest0', nchan=1,\
                                       ntaylorterms=1,gridder='imagemosaic',restfreq=['1.5GHz'],\
                                       imsize=[512,512],\
                                       cell=['10.0arcsec','10.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       #phasecenter=1,\
                                       #ftmachine='gridft',
                                       startmodel='', weighting='natural',\
                                       deconvolver='hogbom',\
                                       #pblimit=0.1,normtype='flatsky',
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)
 
     if(testnum==8):  # MTMFS + Facets
          casalog.post("==================================");
          casalog.post("Test 8 : 1 image-field, mfs, nt=1, 2x2 facets ");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,\
                                       ntaylorterms=2,mtype='multiterm',restfreq=['1.5GHz'],\
                                       imsize=[200,200], facets=2,\
                                       cell=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:00.2 +40.50.15.50",\
                                       ftmachine='gridft', startmodel='', weighting='natural',\
                                       deconvolver='mtmfs',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)


     if(testnum==7):  # MTMFS 
          casalog.post("==================================");
          casalog.post("Test 7 : 1 image-field, mfs, With ntaylorterms=2 ");
          casalog.post("==================================");
          
#          paramList = ImagerParameters(msname='DataTest/eptwochan.ms',field='0',spw='*',\
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,\
                                       ntaylorterms=2,mtype='multiterm',
                                       reffreq='1.6GHz',\
                                       #restfreq=['1.5GHz'],\
                                       imsize=[200,200],\
                                       cell=['8.0arcsec','8.0arcsec'],\
                                       #phasecenter="J2000 19:59:00.2 +40.50.15.50",\
                                       ftmachine='gridft', startmodel='', weighting='natural',\
                                       deconvolver='mtmfs',\
#                                       scales=[0,20,40,100],\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)


     if(testnum==6): ## Facetted imaging
          casalog.post("==================================");
          casalog.post("Test 6 : 1 image-field, mfs, nt=1, 2x2 facets ");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,\
                                       ntaylorterms=1,mtype='default',restfreq=['1.5GHz'],\
                                       imsize=[200,200], facets=2,\
                                       cell=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:00.2 +40.50.15.50",\
                                       ftmachine='gridft', startmodel='', weighting='natural',\
                                       deconvolver='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)


     if(testnum==5):  ## 1 image-field, mfs, multiple input MSs --- Imaging.
          casalog.post("==================================");
          casalog.post("Test 5 image-field, mfs, multiple input MSs --- Imaging.");
          casalog.post("==================================");
          paramList = ImagerParameters(msname=['DataTest/point_onespw0.ms','DataTest/point_onespw1.ms'],\
                                       field='0',spw=['0','0'],\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,\
                                       imsize=[100,100],\
                                       cell=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='gridft', startmodel='', weighting='natural',\
                                       deconvolver='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)


     if(testnum==4):  ## 2 image-fields, one cube, one mfs --- Imaging.
          casalog.post("==================================");
          casalog.post("Test 4 image-fields, one cube, one mfs --- Imaging.");
          casalog.post("==================================");
          
          write_file('out4.txt', 'imagename=mytest1\nnchan=1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nmode=mfs\nstart=1.0GHz\nwidth=2.0GHz')
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',\
                                       field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       outlierfile='out4.txt',\
                                       mode='cube',\
                                       imagename='mytest0', nchan=2,\
                                       start='1.0GHz', width='1.0GHz',
                                       #start='1.0GHz', width='2.0GHz',
                                       #start=0, width=1,
                                       outframe='TOPO',\
                                       imsize=[100,100],\
                                       cell=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='gridft', startmodel='', weighting='natural',\
                                       deconvolver='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)


     
     if(testnum==3):  ## 2 image-fields, mfs --- Imaging.
          casalog.post("==================================");
          casalog.post("Test 3 image-fields, mfs --- Imaging.");
          casalog.post("==================================");
          
          if len(mask)==0:
               write_file('out3.txt', 'imagename=mytest1\nnchan=1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543')
          else:
               write_file('out3.txt', 'imagename=mytest1\nnchan=1\nimsize=[80,80]\ncell=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nmask=circle[[40pix,40pix],10pix]')
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',\
                                       field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       outlierfile='out3.txt',\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,\
                                       imsize=[100,100],\
                                       cell=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='gridft', startmodel='', weighting='natural',\
                                       deconvolver='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)
     
     
     if(testnum==2):  ## 1 image-field, cube --- Imaging.
          casalog.post("==================================");
          casalog.post("Test 2 image-field, cube --- Imaging.");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/point_twospws.ms', field='0',spw='',\
                                       usescratch=True,readonly=True,\
                                       mode='cube',\
                                       imagename='mytest0', nchan=10,start='1.0GHz', width='40MHz',\
                                       imsize=[100,100],\
                                       cell=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='gridft', startmodel='', weighting='natural',\
                                       deconvolver='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)
     

     if(testnum==1):  ## 1 image-field, mfs --- Imaging.
          casalog.post("==================================");
          casalog.post("Test 1 image-field, mfs --- Imaging.");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/point_twospws.ms',field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,\
                                       imsize=[110,110],\
                                       cell=['8.0arcsec','8.0arcsec'],stokes='I',\
                                       ### center
                                       #phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ### offset
                                       #phasecenter="J2000 19:59:23.591 +40.44.01.50",\
                                       phasecenter=0,
                                       ftmachine='gridft', startmodel='', weighting='briggs',robust=0.5,\
                                       deconvolver='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       restoringbeam=restoringbeam,
                                       interactive=interactive,mask=mask)



     ### Make a cluster def file if a parallel test is to run.
     if parallelmajor==True or parallelminor==True or parallelcube==True:
        clusterdef = 'cfgfile.txt'
        defstr = ''
        defstr = defstr + 'riya, 2,'+os.getcwd() #+'/aatest'
#        defstr = defstr + '\n'
#        defstr = defstr + 'tara, 2,'+os.getcwd() # +'/aatest'
        # remove file first
        if os.path.exists(clusterdef):
            os.system('rm -f '+ clusterdef)
        
        # save to a file    
        with open(clusterdef, 'w') as f:
            f.write(defstr)
        f.close()

        ### Open/init a cluster.
        #from simple_cluster import simple_cluster
        #sc = simple_cluster()  
        #sc.init_cluster("cfgfile.txt", "mycluster") 
        if not clustermanager.isClusterRunning():
             clustermanager.init_cluster("cfgfile.txt", "mycluster") 

     ### Check input parameters, and parse outlier files.
     #if paramList.checkParameters() == False:
     #   return [None, "", False, False,False]

     #paramList.printParameters()

     return [ paramList , parallelmajor, parallelminor, parallelcube ]

#####################################################


#######################################
###    Function to run the Imaging.
#######################################
def doClean( params = [None,False,False,False] , doplot=True ):

    os.system('rm -rf mytest*')

    pmajor = params[1]
    pcube = params[3]

    if pmajor==False and pcube==False:
         imager = PySynthesisImager(params[0])
    elif pmajor==True:
         imager = PyParallelContSynthesisImager(params[0])
    elif pcube==True:
         imager = PyParallelCubeSynthesisImager(params[0])
    else:
         print 'Invalid parallel combination in doClean.'
         return

    ### Set up Imagers, Deconvolvers, IterControl, and ParallelSync.
    imager.initializeImagers()
    imager.initializeDeconvolvers()
    imager.initializeNormalizers()
    imager.initializeIterationControl()

    ### Run it.
    imager.makePSF()
    imager.runMajorMinorLoops()

    imager.restoreImages()

    if( doplot == True ):
         imager.getSummary();

    imager.deleteTools()
########################################


########################################
########################################
#  Run only Major Cycle
########################################
def doMajor( params = [None,False,False,False] , doplot=True , tomake='both'):

    os.system('rm -rf mytest*')

    pmajor=params[1]
    pcube = params[3]

    if pmajor==False and pcube==False:
         imager = PySynthesisImager(params[0])
    elif pmajor==True:
         imager = PyParallelContSynthesisImager(params[0])
    elif pcube==True:
         imager = PyParallelCubeSynthesisImager(params[0])
    else:
         print 'Invalid parallel combination in doClean.'
         return


    ### Set up Imagers and ParallelSync.
    imager.initializeImagers()
    imager.initializeNormalizers()

    ### Run it.
    if tomake=='both' or tomake=='psf':
         imager.makePSF()
    if tomake=='both':
         imager.runMajorCycle()

    imager.deleteTools()

########################################
########################################
###   Run only the minor cycle....
########################################
def doMinor( params = [None,False,False,False] , doplot=True ):

##    os.system('rm -rf mytest*')

    pminor=params[2]

    if pminor==False:
         imager = PySynthesisImager(params[0]) 
    else:
         imager = PyParallelDeconvolver(params[0])

    ### Set up Deconvolvers and IterControl
    imager.initializeDeconvolvers()
    imager.initializeIterationControl()

    # Run it
    while ( not imager.hasConverged() ):
        imager.runMinorCycle()

    imager.restoreImages()

    if( doplot == True ):
         imager.getSummary();

    imager.deleteTools()
########################################

########################################
#
#   Predict Model only (for use by setjy too)
#
########################################
########################################
#
#   Restore model image, Inputs : model, residual, psf.
#
########################################
def doRestore( params = [None,False,False,False] ):

    pminor=params[2]
    imager = PySynthesisImager(params[0]) 
    imager.initializeDeconvolvers()
    imager.restoreImages()
    imager.deleteTools()

########################################
def doSetjy( params = [None,False,False,False] ):

    pminor=params[2]
    imager = PySynthesisImager(params[0]) 
    imager.initializeDeconvolvers()

    imager.predictModel()

    imager.deleteTools()


########################################

def write_file(filename,str_text):
    '''Save the string in a text file'''
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


########################################

## Test function for live ImageStore transport between tools.
def toolTestMajorCycle( testnum=1 ):
     params = getparams( testnum=testnum )[0]

     SItool = casac.synthesisimager()
     SItool.initializemajorcycle( params.getSelPars(), params.getImagePars(), params.getGridPars() )

     PStool = casac.synthesisnormalizer()
     normpars = {'imagename':params.getImagePars()['0']['imagename']}
     PStool.setupnormalizer( normpars )

     PStool.scattermodel()
     SItool.makepsf()
     SItool.executemajorcycle()

     PStool.gatherpsfweight( )
     PStool.gatherresidual( )
     
     PStool.dividepsfbyweight()
     PStool.divideresidualbyweight()

     SItool.done()
     PStool.done()


###################################################

## Test function for parameter-list interface for the major cycle.
def toolTestMajorCycle2( testnum=1 ):

     os.system('rm -rf mytest*')

     params = getparams( testnum=testnum )[0]
     allselpars = params.getSelPars()
     allimagepars = params.getImagePars()
     allgridpars = params.getGridPars()

     SItool = casac.synthesisimager()

     for mss in sorted(allselpars.keys()):
          SItool.selectdata( **(allselpars[ mss ] ) )

     for fld in sorted(allimagepars.keys()):
          print 'Setup imaging and image for field ' + str(fld)
          SItool.defineimage( **(allimagepars[fld]) )

     SItool.setweighting( **allgridpars )

     PStools = []
     nfld = len( allimagepars.keys() )
     for fld in range(0, nfld ):
          PStool.append(casac.synthesisnormalizer())
          normpars = {'imagename':allimagepars[ (allimagepars.keys())[fld] ]['imagename']}
          PStool.setupnormalizer( normpars )

     for fld in range(0, nfld):
          PStool[fld].scattermodel()

     SItool.makepsf()
     SItool.executemajorcycle()

     for fld in range(0, nfld):
          PStool[fld].gatherpsfweight( )
          PStool[fld].gatherresidual( )
     
          PStool[fld].dividepsfbyweight()
          PStool[fld].divideresidualbyweight()
          
     SItool.done()

     for fld in range(0, nfld):
          PStool[fld].done()


###################################################


def checkDataPartitioningCode():

     ## Make parameter lists.
     #paramList = ImagerParameters(msname=['x1.ms','x2.ms'], field='0',spw=['0','2'], usescratch=True)
     ## Sync input lists to the same size.
     #paramList.checkParameters()

     params = getparams( testnum=2 ,parallelmajor=True )
     paramList = params[0]

     selpars = paramList.getSelPars()
     impars = paramList.getImagePars()

     synu = casac.synthesisutils()
     print synu.contdatapartition( selpars , 2)

     print synu.cubedatapartition( selpars, 2)
     
     print synu.cubeimagepartition( impars, 2)

     synu.done()

#     ppar = PyParallelImagerHelper()
#
#     print 'Selpars : ', selpars
#     newselpars = ppar.partitionCubeDataSelection( selpars )
#
#     print 'Impars : ', impars
#     newimpars = ppar.partitionCubeDeconvolution( impars )
#
#     ppar.takedownCluster()
#

     # The output dictionary should be indexed as follows ( for 2 nodes, and 2 MSs )
     #{ '0' : { 'ms0' : { 'msname':xxx1, 'spw':yyy1 } ,
     #            'ms1' : { 'msname':xxx2, 'spw':yyy1 } 
     #         }
     #  '1' : { 'ms0' : { 'msname':xxx1, 'spw':yyy2 } ,
     #            'ms1' : { 'msname':xxx2, 'spw':yyy2 } 
     #        }
     # }
     #


def checkPars():

     params = getparams( testnum=7 )
     paramList = params[0]

     selpars = paramList.getSelPars()
     impars = paramList.getImagePars()

     synu = casac.synthesisutils()

#     print selpars['ms0']
#     fixrec = synu.checkselectionparams( selpars['ms0'] )
#     print fixrec
#     print synu.checkselectionparams( fixrec )


     print impars['0']
     fixrec = synu.checkimageparams( impars['0'] )
     print "---------------------------------------------"
     print fixrec
     print "---------------------------------------------"
     print synu.checkimageparams( fixrec )

     synu.done()

def testImageCoordinates( testnum=1, testid=0):
    
    multitest=False
    if testnum==13: multitest=True
    if multitest and testid==-1: testid=range(21)
    if type(testid)==int: testid=[testid]
    for tst in testid:
      params = getparams( testnum, testid=tst )
      paramList = params[0]
      impars=paramList.getImagePars()['0']
      if os.path.exists(impars['imagename']+'.residual'):
        os.system('rm -rf '+impars['imagename']+'*')

      doMajor(params)

      ok = ia.open(impars['imagename']+'.residual')
      if ok:
        selpars = paramList.getSelPars()
        outcsys=ia.coordsys().torecord()
        imsummary=ia.summary()
        ia.done()
        #print selpars
        nchan = impars['nchan']
        if (nchan!=imsummary['shape'][3]): 
          print "No. of image channel =%s while nchan asked is %s" % (imsummary['shape'][3], nchan) 
        checkimcoord(outcsys,selpars,impars) 

def checkimcoord(csys,selpars,impars):
    """
    check the output image from testImageCoordinates if it is consistent
    with the input data and image parameters (selpars, impars)
    """

    errs=[]
    # freq frame dict
    specframes={0:'REST',1:'LSRK',2:'LSRD',3:'BARY',4:'GEO',5:'TOPO',6:'GALACTO',7:'LGROUP',8:'CMB',64:'Undefined'}

    msname=selpars['ms0']['msname']
    selspw=selpars['ms0']['spw'] 
    selfield=selpars['ms0']['field'] 
    if selspw=='': selspw='*'
    ms.open(msname)    
    sel={}
    sel['spw']=selspw
    sel['field']=selfield
    ms.msselect(sel)
    selected = ms.msselectedindices()
    #print "selected=",selected
    tb.open(msname+'/SPECTRAL_WINDOW')
    chanfreqs=tb.getcell('CHAN_FREQ',selected['spw'])
    chanw=tb.getcell('CHAN_WIDTH',selected['spw'])
    msframe=tb.getcell('MEAS_FREQ_REF',selected['spw'][0])
    tb.close()
    dataframe=specframes[msframe]
    #print "dataframe=",dataframe
    chansel = selected['channel'][0][1]
    chanselend = selected['channel'][0][2]
    #print "start freq=",chanfreqs[chansel]
    #print "csys=",csys 
    # kower edge freq
    descendingfreq = False
    if chanw[0] < 0:
      descendingfreq = True

    floedge = chanfreqs[chansel]-chanw[chansel]/2.0
    fhiedge = chanfreqs[chanselend]+chanw[chansel]/2.0
    #print "floedge=",floedge, "fhiedge=",fhiedge
    outframe=impars['outframe']
    veltype=impars['veltype']
    start=impars['start']
    width=impars['width']
    restf=impars['restfreq'][0]

    reversechanorder=False
    # default start handling with width specification
    # - also depends on freq ordering in the data
    if type(start)==str and start=="":
      widthval=0
      if type(width)==dict:
        if width.has_key('type') and width['type']=='radialvelocity':
          widthval = width['m0']['value']
        elif width.has_key('unit') and width['unit'].count('m/s'):
          widthval = width['value']
        if (descendingfreq and widthval < 0) or (not descendingfreq and widthval > 0):
          reversechanorder=True
      elif width.count('m/s')!=0:
        widthval = qa.quantity(width)['value']
        if (descendingfreq and widthval < 0) or (not descendingfreq and widthval > 0):
          reversechanorder=True
          
    #print "descendingfreq=",descendingfreq
    #print "reversechanorder=",reversechanorder
    needfreqconv=False
    if dataframe!=outframe:
       needfreqconv=True

    # from output image --------------------------------
    # freqframe
    if csys.has_key('spectral2'):
      specCoord = csys['spectral2']
      if specCoord.has_key('tabular'):
        imch0=specCoord['tabular']['crval'][0]
        iminc=specCoord['tabular']['cdelt'][0]
      else:
        imch0=specCoord['wcs']['crval']
        iminc=specCoord['wcs']['cdelt']

    imframe=specCoord['system']
    # conversion layer- this should be LSRK by default
    convframe=specCoord['conversion']['system']
    obsdate=csys['obsdate']
    # if image channel reversed, check last channel 
    if reversechanorder:
      imch0=imch0+iminc*(impars['nchan']-1)
    # check frames
    if convframe!="LSRK":
      errs.append('Error: conversion layer is not set to LSRK!')
    if imframe!=outframe:
      errs.append('Error: inconsistent output spec frame:%s, should be %s' % (imframe,outframe))

    startfreq = 0
    imch0conv = imch0
    vmode = False
    if type(start)==int:
      if start>=chansel:
        if type(width)==int and width > 0:
          range = start + width - 1   
          startfreq = (chanfreqs[range] + chanfreqs[start]) /2.0 
        else:
          startfreq=chanfreqs[start]
      else:
        # start chan idx less than chansel by spw. Will results in some blank channel images
        startfreq = chanfreqs[start]
    else:
      if type(start)==str:
      # start parameter 
        if start=="":
          freq0q = qa.quantity(str(chanfreqs[chansel])+'Hz')
        elif start.count('Hz') !=0:
          freq0q = qa.quantity(start)
        elif start.count('m/s') !=0:
          freq0q = vfconv(start,outframe,restf,veltype)['m0']
          vmode = True
      elif type(start)==dict:
        # measure freq/vel format
        if start.has_key('type'):
          mframe = start['refer']
          if start['type']=='frequency':
            freq0q = start['m0']
          else:
            freq0q = vfconv(str(start['m0']['value'])+start['m0']['unit'], mframe, restf, veltype)['m0']
            vmode = True  
        else:
          if start['unit'].count('Hz') != 0:
            freq0q = start
          elif start['unit'].count('m/s') != 0:
            freq0q = vfconv(str(start['value'])+start['unit'],outframe,restf,veltype)['m0']
            vmode = True  

      # step(width)
      qhalfw = 0
      if type(width)==str and width!="":
        if width.count('Hz') != 0:
          qhalfw = qa.div(qa.quantity(width),2)
        elif width.count('m/s') != 0:
          qhalfw = qa.div(qa.quantity(width),2) 
          vmode = True

      if qhalfw:
        # add width part to the starting frequency
        if vmode:
          qvst = qa.quantity(start)
          if qvst['value']==0.0:
            qvst = vfconv(str(freq0q['value'])+freq0q['unit'],outframe, restf, veltype)['m0']
          qvstedge = qa.sub(qvst,qhalfw)
          qfstedge = vfconv(str(qvstedge['value'])+qvstedge['unit'], outframe, restf, veltype)['m0']
          fstedge = qa.convert(qfstedge, 'Hz')['value']

          if reversechanorder:
            if (fstedge > floedge and not descendingfreq) or (fstedge < floedge and descendingfreq):
              qvelcen = qa.sub(vfconv(str(floedge)+'Hz',outframe, restf,veltype)['m0'],qhalfw)
            else:
              qvelcen = qvst 
          else:
            if (fstedge < floedge and not descendingfreq) or (fstedge > floedge and descendingfreq):
              # shift start to fit into data....
              #qvelcen = qa.add(qa.quantity(start),qhalfw)
              qvelcen = qa.add(vfconv(str(floedge)+'Hz',outframe, restf,veltype)['m0'],qhalfw)
            else:
              qvelcen = qvst 
          qstfreq = vfconv(str(qvelcen['value'])+qvelcen['unit'], outframe, restf, veltype)['m0']
          startfreq = qa.convert(qstfreq,'Hz')['value']
        else:
          qfstedge = qa.sub(freq0q,qhalfw)
          fstedge = qa.convert(qfstedge, 'Hz')['value']
          if (fstedge < floedge and not descendingfreq) or (fstedge > floedge and descendingfreq):
            qfrqcen = qa.add(qa.quantity(floedge,'Hz'),qhalfw)
            qstfreq = qa.convert(qfrqcen,'Hz')
          else:
            qstfreq = freq0q
          startfreq = qa.convert(qstfreq,'Hz')['value']
      else:
        startfreq = qa.convert(freq0q,'Hz')['value']
        
    #print "startfreq=",startfreq
    if needfreqconv:
      #print "converting image ch0 freq to freq in dataframe=",dataframe
      me.doframe(me.epoch(obsdate['refer'],str(obsdate['m0']['value'])+obsdate['m0']['unit']))
      me.doframe(csys['telescopeposition'])
      me.doframe(me.direction(csys['direction0']['conversionSystem'],str(csys['direction0']['crval'][0])+'rad',str(csys['direction0']['crval'][1])+'rad'))
      print "******************\n"
      #print me.showframe()
      #print "imch0=", imch0
      imch0conv=me.measure(me.frequency(imframe,str(imch0)+'Hz'),dataframe)['m0']['value']
    #print "dataframe=",dataframe, " outframe=",outframe, " imframe=",imframe
    if (abs(imch0conv-startfreq)/startfreq < 0.0001):      
      if (reversechanorder):
        print "OK, image last chan freq match with the data selections and start specification: start freq=", startfreq 
      else:
        print "OK, image ch0 freq match with the data selections and start specification: start freq=", startfreq 
    else:
      print "Error, image ch0 freq does not match with the data selections+start specification, imch0(in %s)=%s start freq=%s" % (dataframe,imch0conv,startfreq)


def vfconv(velorfreqstr, frame, restfstr, veltype):
    """
    vel to freq or freq to vel convesion
    velorfreq - input vel of freq in a string with unit
    """
    if (qa.quantity(velorfreqstr)['unit'].find('m/s') > -1 ):
      dop = me.doppler(veltype, qa.quantity(velorfreqstr))
      retvf = me.tofrequency(frame, dop, qa.quantity(restfstr))
    elif (qa.quantity(velorfreqstr)['unit'].find('Hz') > -1 ):
      mfreq = me.frequency(frame, qa.quantity(velorfreqstr))
      retvf = me.todoppler(veltype, mfreq, qa.quantity(restfstr))
    else:
      print "Error! cannot convert ", velorfreqstr
    return retvf
    

#def testmakeimage():
#     params = getparams( testnum=1 )
#     paramList = params[0]
#     impars = (paramList.getImagePars())['0']
#
#     os.system('rm -rf ' + impars['imagename'])
#
#     synu = casac.synthesisutils()
#     #synu.makeimage( impars , 'DataTest/twopoints_twochan.ms')
#     synu.makeimage( impars , 'DataTest/point_twospws.ms')
#     synu.done()


def delmodkeywords(msname=""):
     delmod(msname)
     tb.open( msname+'/SOURCE', nomodify=False )
     keys = tb.getkeywords()
     for key in keys:
          tb.removekeyword( key )
     tb.close()

def resetmodelcol(msname=""):
     tb.open( msname, nomodify=False )
     dat = tb.getcol('MODEL_DATA')
     dat.fill( complex(0.0,0.0) )
     tb.putcol('MODEL_DATA', dat)
     tb.close();

def delmodels(msname=""):
     resetmodelcol(msname)  ## Set model column to zero
     delmod(msname)  ## Get rid of OTF model
     delmodkeywords(msname) ## Get rid of extra OTF model keywords that sometimes persist...

def checkmodel(msname=""):
     tb.open( msname )
     hasmodcol = (  (tb.colnames()).count('MODEL_DATA')>0 )
     modsum=0.0
     if hasmodcol:
          dat = tb.getcol('MODEL_DATA')
          modsum=dat.sum()
     tb.close()
     tb.open( msname+'/SOURCE' )
     keys = tb.getkeywords()
     if len(keys)>0:
          hasvirmod=T
     else:
          hasvirmod=F
     tb.close()
     print msname , ": modelcol=", hasmodcol, " modsum=", modsum, " virmod=", hasvirmod
