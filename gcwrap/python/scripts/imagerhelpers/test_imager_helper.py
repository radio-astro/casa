import os
import commands
import math
import shutil
import string
import time
import re;
from taskinit import *
import copy

_ia = iatool( )
_cb = cbtool( )

'''
A set of helper functions for the tasks  tclean

Summary...
    
'''

import time
import resource
class PerformanceMeasure():
    def __init__(self):
        self.t0=self.timestart()
        self.t1=0.0
        self.mem=0.0

    def timestart(self):
        self.t0 = time.time()

    def gettime(self,label=""):
        self.t1 = time.time()
        return "'%s: time=%s'"%(label,self.t1-self.t0)

    def getresource(self,label=""):
        usage=resource.getrusage(resource.RUSAGE_SELF)
        return '''%s: usertime=%s systime=%s mem=%s mb '''%(label,usage[0],usage[1], (usage[2]*resource.getpagesize())/1000000.0 )


##########################################
import os
import sys
import shutil
import commands
import numpy
import inspect
#from tasks import delmod

class TestHelpers():
     def __init__(self):
         self.epsilon = 0.05
    
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
          _ia.open(imname)
          stat = _ia.statistics()
          _ia.close()
          return stat['max'],stat['maxpos']

     def get_pix(self,imname,pos):
          """Get Image val"""
          _ia.open(imname)
          apos = _ia.pixelvalue(pos)
          _ia.close()
          if apos == {}:
               return None
          else:
               return apos['value']['value']

     def get_pixmask(self,imname,pos):
          """Get Image Mask val"""
          _ia.open(imname)
          apos = _ia.pixelvalue(pos)
          _ia.close()
          if apos == {}:
               return None
          else:
               return apos['mask']

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
                  imval=None,  # list of tuples of (imagename,val,pos)
                  immask=None,  #list of tuples to check mask value
                  tabcache=True
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

          if immask != None:
               if type(immask)==list:
                    for ii in immask:
                         if type(ii)==tuple and len(ii)==3:
                              pstr += self.checkpixmask(ii[0],ii[1],ii[2])

          if tabcache==True:
               opentabs = tb.showcache()
               if len(opentabs)>0 : 
                    pstr += "["+inspect.stack()[1][3]+"] "+self.verdict(False) + ": Found open tables after run "
          
          return pstr
          #self.checkfinal(pstr)

     def checkchanvals(self,msname,vallist): # list of tuples of (channumber, relation, value) e.g. (10,">",1.0)
          testname = inspect.stack()[1][3]
          pstr = ""
          for val in vallist:
               if len(val)==3:
                    thisval = self.checkmodelchan(msname,val[0])
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
          return pstr
          #pstr = self.checkfinal(pstr)

#     def checkfinal(self,pstr=""):
#          if( pstr.count("(Fail") > 0 ):
#              pstr += "["+inspect.stack()[2][3]+"] : To re-run this test :  runUnitTest.main(['test_refimager["+ inspect.stack()[2][3] +"]']) "
#              #self.fail("\n"+pstr)
#              return False
#          else:
#              return True

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

          if readval==None:
               res=False
          elif numpy.isnan(readval) or numpy.isinf(readval):
               res=False
          else:
               if abs(theval)>self.epsilon:
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

     def checkpixmask(self,imname,theval=True, thepos=[0,0,0,0]):
          testname = inspect.stack()[2][3]
          readval = self.get_pixmask(imname,thepos)

          res=True

          if readval==None:
               res=False
          elif numpy.isnan(readval) or numpy.isinf(readval) or type(readval)!=bool:
               res=False
          else:

              if readval == theval:
                  res = True
              else:
                  res = False
              
          pstr =  "[" + testname + "] " + imname + ": Mask is " + str(readval) + " at " + str(thepos) + " (" + self.verdict(res) +" : should be " + str(theval) + " )"
          print pstr
          pstr=pstr+"\n"
#          if res==False:
#               self.fail(pstr)
          return pstr
   
     def checkspecframe(self,imname,frame, crval=0.0):
          testname = inspect.stack()[1][3]
          pstr = ""
          if os.path.exists(imname):
               res = True
               coordsys = self.getcoordsys(imname)
               baseframe = coordsys['spectral2']['system']
               basecrval = coordsys['spectral2']['wcs']['crval']
               if baseframe != frame:
                    res = False 
               else:
                    res = True
                    if crval!=0.0:
                         if abs(basecrval - crval)/crval > 1.0e-6: 
                              res = False
                    else:
                         # skip the crval test
                         thecrval = ""
               thecorrectans = frame + " "+ str(crval) 
               pstr =  "[" + testname + "] " + imname + ": Spec frame is " +\
               str(baseframe) + " with crval " + str(basecrval) + " (" +\
               self.verdict(res) +" : should be " + thecorrectans +" )"
               print pstr
               pstr=pstr+"\n"
          #self.checkfinal(pstr)
          return pstr
        
     def getcoordsys(self,imname):
         _ia.open(imname)
         csys = _ia.coordsys().torecord()
         _ia.close()
         return csys


     def modeltype(self,msname):
          """has no model, otf model, modelcol"""
          mtype = 0
          return mtype

     def delmodkeywords(self,msname=""):
#          delmod(msname)
          tb.open( msname+'/SOURCE', nomodify=False )
          keys = tb.getkeywords()
          for key in keys:
               tb.removekeyword( key )
          tb.close()

     def resetmodelcol(self,msname="",val=0.0):
          tb.open( msname, nomodify=False )
          hasmodcol = (  (tb.colnames()).count('MODEL_DATA')>0 )
          if not hasmodcol:
               _cb.open(msname)
               _cb.close()
          hasmodcol = (  (tb.colnames()).count('MODEL_DATA')>0 )
          if hasmodcol:
               dat = tb.getcol('MODEL_DATA')
               dat.fill( complex(val,0.0) )
               tb.putcol('MODEL_DATA', dat)
          tb.close();

     def delmodels(self,msname="",modcol='nochange'):
#          delmod(msname)  ## Get rid of OTF model and model column
          self.delmodkeywords(msname) ## Get rid of extra OTF model keywords that sometimes persist...

          if modcol=='delete':
               self.delmodelcol(msname) ## Delete model column
          if modcol=='reset0':
               self.resetmodelcol(msname,0.0)  ## Set model column to zero
          if modcol=='reset1':
               self.resetmodelcol(msname,1.0)  ## Set model column to one

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

     def checkMPI(self):
          from mpi4casa.MPIInterface import MPIInterface as mpi_clustermanager
          try:
               self.nproc = len(mpi_clustermanager.getCluster()._cluster.get_engines())
               return True
          except Exception as e:
               self.nproc = 0
               return False
          

     def getNParts(self,imprefix='', imexts=[]):
          
          from mpi4casa.MPIInterface import MPIInterface as mpi_clustermanager
          try:
               self.nproc = len(mpi_clustermanager.getCluster()._cluster.get_engines())
          except Exception as e:
               self.nproc = 0

          if( self.nproc>0 ):
               
               imlist=[];
               for imext in imexts:
                    for part in range(1,self.nproc+1):
                         imlist.append( imprefix+'.workdirectory/'+imprefix + '.n'+str(part)+'.'+imext )
               #self.checkall(imexist = imlist)

          else:
               print 'Not a parallel run of CASA'

          return imlist

##############################################




