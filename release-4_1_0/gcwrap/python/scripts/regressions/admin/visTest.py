from  casac import *
from tw_utils import *
import os
from time import *
import numpy as numarray
# very simple for now, class to do MS testing in regression framework

import pylab
import string
import tw_func
from plotms import plotms

class VisTest:
    def __init__(self, msName, write=True,
                 resultDir='WEBPAGES/visTest/',
                 imDir='IMAGES/'):

        import shutil
        self.msTool=casac.ms()
        self.msTool.open(msName)             
	self.write=write
	self.msName=msName

	if self.write:
         self.resultDir=resultDir+strftime('/%Y_%m_%d/')
         if os.access(self.resultDir,os.F_OK) is False:
          print self.resultDir+' directory DNE, so am making one!'
          os.mkdir(self.resultDir)
         else: 
          print self.resultDir+' directory exists; will add to it!'
	 self.imDir=imDir
	 if os.access(imDir,os.F_OK) is False:
	  print imDir+' directory DNE, so am making one!'
	  os.mkdir(imDir)
	 else: 
	  print imDir+' directory exists; will add to it!'

         t=localtime( time() )
         self.fname='Regression-%s-%s-%s-%s-%s-%s.html'%(t[0],t[1],t[2],t[3],t[4],t[5])
	 self.html=self.resultDir+self.fname
         self.body1=[]
         self.body2=[]
         self.htmlPub=htmlPub(self.html,'Measurement Set tests')
        else:
	 print 'stats-only mode; will not write to html file!'


    def simple_stats(self,sigma=10):
        # likely improvements later: iterate through spws, 

        # add a conformity test, for nrows etc
        
        ampstats=self.msTool.statistics("DATA","amp")
        phastats=self.msTool.statistics("DATA","phase")

	if self.write:
            listnam=string.split(self.msTool.name(), '/')
            imnam=listnam[len(listnam)-2]+'_'+listnam[len(listnam)-1]
            saveDir=self.imDir+imnam+'-ampuv.png'
            self.ampuv(saveDir)
            header='amp vs. uvdist of %s' % self.msName
            body1=['The figure generated with msplot:']
            body2=['maximum: %f'%(ampstats['DATA']['rms']),'minimum: %f'%(ampstats['DATA']['min']),'rms: %f'%(ampstats['DATA']['rms'])]
            self.htmlPub.doBlk(body1, body2, saveDir,header)
        returnFlag= 1
        if(ampstats['DATA']['rms'] > 2*sigma): returnFlag=-1
        return ampstats['DATA']['rms'], ampstats['DATA']['max'], ampstats['DATA']['min'], phastats['DATA']['rms'], phastats['DATA']['max'], phastats['DATA']['min'], returnFlag
        # return rms1, max1, min1, returnFlag
        
    def done(self) :
	if self.write:
       	 self.htmlPub.doFooter()
	 print 'webpage construction successful!'
	 print 'images in '+os.path.abspath(self.imDir)
	 print 'webpage at '+os.path.abspath(self.html)
         return '%s'%(os.path.abspath(self.html))
        else: #return 0 if no writing of file is done
         return 'none'

    def ampuv(self,saveDir):
        plotms(self.msName,xaxis="uvdist",avgchannel="all",coloraxis="baseline",plotfile=saveDir)
        

