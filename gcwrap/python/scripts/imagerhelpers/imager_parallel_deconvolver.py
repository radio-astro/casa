import os
import commands
import math
import shutil
import string
import time
import re;
from taskinit import *
import copy

from imagerhelpers.imager_base import PySynthesisImager
from imagerhelpers.parallel_imager_helper import PyParallelImagerHelper

'''
A set of helper functions for the tasks  tclean

Summary...
    
'''

#############################################

class PyParallelDeconvolver(PySynthesisImager):

    def __init__(self,params):

        PySynthesisImager.__init__(self,params)

        self.PH = PyParallelImagerHelper()
        self.NF = len( allimpars.keys() )
        self.listOfNodes = self.PH.getNodeList();
        #### MPIInterface related changes
        #self.NN = self.PH.NN
        self.NN = len(self.listOfNodes);
        if self.NF != self.NN:
             print 'For now, cannot handle nfields != nnodes. Will implement round robin allocation later.'
             print 'Using only ', self.NN, ' fields and nodes'
             

#############################################
    def initializeDeconvolvers(self):
         joblist=[]
         #### MPIInterface related changes
         #for immod in range(0,self.NF):
         for immod in self.listOfNodes:
              self.PH.runcmd("toolsd = casac.synthesisdeconvolver()", immod )
              joblist.append( self.PH.runcmd("toolsd.setupdeconvolution(decpars="+ str(self.alldecpars[str(immod)]) +")", immod ) )
         self.PH.checkJobs( joblist )

#############################################
    def deleteDeconvolvers(self):
         self.PH.runcmd("toolsd.done()")
              
#############################################
    def restoreImages(self):
         self.PH.runcmdcheck("toolsd.restore()")
 
#############################################
    def pbcorImages(self):
         self.PH.runcmdcheck("toolsd.pbcor()")

#############################################
#############################################

    def hasConverged(self):
        # Merge peak-res info from all fields to decide iteration parameters
        self.PH.runcmdcheck("initrec = toolsd.initminorcycle()")

        #### MPIInterface related changes
        #for immod in range(0,self.NF):
        for immod in self.listOfNodes:
             retrec = self.PH.pullval("initrec", immod )
             self.IBtool.mergeinitrecord( retrec[immod] )
#             print "Peak res of field ",immod, " on node ", immod , ": " ,retrec[immod]['peakresidual']
#             casalog.post("["+self.allimpars[str(immod)]['imagename']+"] : Peak residual : %5.5f"%(initrec['peakresidual']), "INFO")

        # Check with the iteration controller about convergence.
        stopflag = self.IBtool.cleanComplete()
        print 'Converged : ', stopflag
        if( stopflag>0 ):
            stopreasons = ['iteration limit', 'threshold', 'force stop','no change in peak residual across two major cycles', 'peak residual increased by more than 5 times from the previous major cycle','peak residual increased by more than 5 times from the minimum reached']
            casalog.post("Reached global stopping criterion : " + stopreasons[stopflag-1], "INFO")
            if self.iterpars['interactive']:
                for immod in range(0,self.listOfNodes):
                    if self.alldecpars[str(immod)]['usemask']=='auto-thresh':
                        prevmask = self.allimpars[str(immod)]['imagename']+'.prev.mask'
                        if os.path.isdir(prevmask):
                            shutil.rmtree(self.allimpars[str(immod)]['imagename']+'.mask')
                        shutil.move(prevmask,self.allimpars[str(immod)]['imagename']+'.mask')
        return (stopflag>0)


#############################################

    def runMinorCycle(self):
        # Get iteration control parameters
        iterbotrec = self.IBtool.getminorcyclecontrols()
        # Run minor cycle
        self.PH.CL.push( iterbotrec = iterbotrec )

        self.PH.runcmdcheck( "exrec = toolsd.executeminorcycle(iterbotrec)" )

        #### MPIInterface related changes
        #for immod in range(0,self.NF):
        for immod in self.listOfNodes:
             retrec = self.PH.pullval("exrec", immod )
             self.IBtool.mergeexecrecord( retrec[immod] )

#############################################
#############################################

