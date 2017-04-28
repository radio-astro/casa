import os
import commands
import math
import shutil
import string
import time
import re;
from taskinit import *
import copy

'''
A set of helper functions for tclean.

Summary...
    
'''

#############################################
class PySynthesisImager:

    def __init__(self,params):
        ################ Tools
        self.initDefaults()

        # Check all input parameters, after partitioning setup.

        # Selection Parameters. Dictionary of dictionaries, indexed by 'ms0','ms1',...
        self.allselpars = params.getSelPars()
        # Imaging/Deconvolution parameters. Same for serial and parallel runs
        self.alldecpars = params.getDecPars()
        self.allimpars = params.getImagePars()
        self.allgridpars = params.getGridPars()
        self.allnormpars = params.getNormPars()
        self.weightpars = params.getWeightPars()
        # Iteration parameters
        self.iterpars = params.getIterPars() ## Or just params.iterpars

        # CFCache params
        self.cfcachepars = params.getCFCachePars()
        ## Number of fields ( main + outliers )
        self.NF = len(self.allimpars.keys())
        self.stopMinor = {}  ##[0]*self.NF
        for immod in range(0,self.NF):
            self.stopMinor[str(immod)]=1.0
        ## Number of nodes. This gets set for parallel runs
        ## It can also be used serially to process the major cycle in pieces.
        self.NN = 1 
        ## for debug mode automask incrementation only
        self.ncycle = 0
#        isvalid = self.checkParameters()
#        if isvalid==False:
#            print 'Invalid parameters'

#############################################
#    def checkParameters(self):
#        # Copy the imagename from impars to decpars, for each field.
#        for immod in range(0,self.NF):
#            self.alldecpars[str(immod)]['imagename'] = self.allimpars[str(immod)]['imagename']
#        return True

#############################################
    def makeCFCache(self,exists):
        # Make the CFCache and re-load it.  The following calls become
        # NoOps (in SynthesisImager.cc) if the gridder is not one
        # which uses CFCache.
        if (exists):
            print "CFCache already exists";
        else:
            self.dryGridding();
            self.fillCFCache();
            self.reloadCFCache();
        
#############################################
    def initializeImagers(self):
        
        ## Initialize the tool for the current node
        self.SItool = casac.synthesisimager()
 
        ##print 'impars ', self.allimpars['0']['specmode'], 'frame', self.allimpars['0']['outframe']
        ## Send in selection parameters for all MSs in the list.
        for mss in sorted( (self.allselpars).keys() ):
#            if(self.allimpars['0']['specmode']=='cubedata'):
#                self.allselpars[mss]['outframe']='Undefined'
            self.SItool.selectdata( self.allselpars[mss] )
#            self.SItool.selectdata( **(self.allselpars[mss]) )

        ## For each image-field, define imaging parameters
#        nimpars = copy.deepcopy(self.allimpars)
#        for fld in range(0,self.NF):
#            self.SItool.defineimage( **( nimpars[str(fld)]  ) )
        
        # If cfcache directory already exists, assume that it is
        # usable and is correct.  makeCFCache call then becomes a
        # NoOp.
        cfCacheName=self.allgridpars['0']['cfcache'];
        exists=False;
        if (not (cfCacheName == '')):
            exists = (os.path.exists(cfCacheName) and os.path.isdir(cfCacheName));

        for fld in range(0,self.NF):
            #print "self.allimpars=",self.allimpars,"\n"
            self.SItool.defineimage( self.allimpars[str(fld)] , self.allgridpars[str(fld)] )
    
        # For cube imaging:  align the data selections and image setup
        if self.allimpars['0']['specmode'] != 'mfs' and self.allimpars['0']['specmode'] != 'cubedata':
            self.SItool.tuneselectdata()

        #self.makeCFCache(exists);

#############################################

    def initializeDeconvolvers(self):
         for immod in range(0,self.NF):
              self.SDtools.append(casac.synthesisdeconvolver())
              self.SDtools[immod].setupdeconvolution(decpars=self.alldecpars[str(immod)])

#############################################
    ## Overloaded by ParallelCont
    def initializeNormalizers(self):
        for immod in range(0,self.NF):
            self.PStools.append(casac.synthesisnormalizer())
            normpars = self.allnormpars[str(immod)]
            self.PStools[immod].setupnormalizer(normpars=normpars)

#############################################

    def initializeIterationControl(self):
        self.IBtool = casac.synthesisiterbot()
        itbot = self.IBtool.setupiteration(iterpars=self.iterpars)

#############################################

    def restoreImages(self):
         for immod in range(0,self.NF):
              self.SDtools[immod].restore()

#############################################
    def pbcorImages(self):
         for immod in range(0,self.NF):
              self.SDtools[immod].pbcor()

#############################################

    def getSummary(self,fignum=1):
        summ = self.IBtool.getiterationsummary()
        self.plotReport( summ, fignum )
        return summ

#############################################
    def deleteImagers(self):
        if self.SItool != None:
            self.SItool.done()

    def deleteDeconvolvers(self):
         for immod in range(0,len(self.SDtools)):
              self.SDtools[immod].done()

    def deleteNormalizers(self):
         for immod in range(0,len(self.PStools)):
            self.PStools[immod].done()

    def deleteIterBot(self):
         if self.IBtool != None:
              self.IBtool.done()

    def deleteCluster(self):
#         print 'no cluster to delete'
        return

    def initDefaults(self):
        # Reset globals/members
         self.NF=1
         self.stopMinor={'0':1.0}  # Flag to call minor cycle for this field or not.
         self.NN=1
         self.SItool=None
         self.SDtools=[]
         self.PStools=[]
         self.IBtool=None
    
#############################################

    def deleteTools(self):
         self.deleteImagers()
         self.deleteDeconvolvers()
         self.deleteNormalizers()
         self.deleteIterBot()
         self.initDefaults()
         self.deleteCluster()

#############################################

    def hasConverged(self):
        # Merge peak-res info from all fields to decide iteration parameters
         self.IBtool.resetminorcycleinfo() 
         for immod in range(0,self.NF):
              initrec =  self.SDtools[immod].initminorcycle() 
              self.IBtool.mergeinitrecord( initrec );

#         # Run interactive masking (and threshold/niter editors)
#         self.runInteractiveGUI2()

         # Check with the iteration controller about convergence.
         #print "check convergence"
         stopflag = self.IBtool.cleanComplete()
         #print 'Converged : ', stopflag
         if( stopflag>0 ):
             #stopreasons = ['iteration limit', 'threshold', 'force stop','no change in peak residual across two major cycles']
             stopreasons = ['iteration limit', 'threshold', 'force stop','no change in peak residual across two major cycles', 'peak residual increased by more than 5 times from the previous major cycle','peak residual increased by more than 5 times from the minimum reached','zero mask']
             casalog.post("Reached global stopping criterion : " + stopreasons[stopflag-1], "INFO")

#             # revert the current automask to the previous one 
#             if self.iterpars['interactive']:
#                 for immod in range(0,self.NF):
#                     if self.alldecpars[str(immod)]['usemask']=='auto-thresh':
#                        prevmask = self.allimpars[str(immod)]['imagename']+'.prev.mask'
#                        if os.path.isdir(prevmask):
#                          shutil.rmtree(self.allimpars[str(immod)]['imagename']+'.mask')
#                          #shutil.copytree(prevmask,self.allimpars[str(immod)]['imagename']+'.mask')
#                          shutil.move(prevmask,self.allimpars[str(immod)]['imagename']+'.mask')

         return (stopflag>0)

#############################################
    def updateMask(self):
        # Setup mask for each field ( input mask, and automask )
        maskchanged = False
        for immod in range(0,self.NF):
            maskchanged = maskchanged | self.SDtools[immod].setupmask() 
        
        # Run interactive masking (and threshold/niter editors), if interactive=True
        maskchanged = maskchanged | self.runInteractiveGUI2()

        ## Return a flag to say that the mask has changed or not.
        return maskchanged

#############################################
    def runInteractiveGUI2(self):
        maskchanged = False
        forcestop = True
        if self.iterpars['interactive'] == True:
            self.stopMinor = self.IBtool.pauseforinteraction()
            #print "Actioncodes in python : " , self.stopMinor

            for akey in self.stopMinor:
                if self.stopMinor[akey] < 0:
                    maskchanged = True
                    self.stopMinor[akey] = abs( self.stopMinor[akey] )

            #Check if force-stop has happened while savemodel != "none".
            # If so, warn the user that unless the Last major cycle has happened,
            # the model won't have been written into the MS, and to do a 'predict' run.
            forcestop=True;
            for akey in self.stopMinor:
                forcestop = forcestop and self.stopMinor[akey]==3

            if self.iterpars['savemodel'] != "none":
                if forcestop==True:
                    self.predictModel()
                    #if self.iterpars['savemodel'] == "modelcolumn":
                    #    wstr = "Saving model column"
                    #else:
                    #    wstr = "Saving virtual model"
                    #casalog.post("Model visibilities may not have been saved in the MS even though you have asked for it. Please check the logger for the phrases 'Run (Last) Major Cycle'  and  '" + wstr +"'. If these do not appear, then please save the model via a separate tclean run with niter=0,calcres=F,calcpsf=F. It will pick up the existing model from disk and save/predict it.   Reason for this : For performance reasons model visibilities are saved only in the last major cycle. If the X button on the interactive GUI is used to terminate a run before this automatically detected 'last' major cycle, the model isn't written. However, a subsequent tclean run as described above will predict and save the model. ","WARN")

        #print 'Mask changed during interaction  : ', maskchanged
        return ( maskchanged or forcestop )

#############################################
    def makePSF(self):

        self.makePSFCore()

        ### Gather PSFs (if needed) and normalize by weight
        for immod in range(0,self.NF):
            self.PStools[immod].gatherpsfweight() 
            self.PStools[immod].dividepsfbyweight()

#############################################

    def runMajorCycle(self):
        for immod in range(0,self.NF):
            self.PStools[immod].dividemodelbyweight()
            self.PStools[immod].scattermodel() 

        if self.IBtool != None:
            lastcycle = (self.IBtool.cleanComplete(lastcyclecheck=True) > 0)
        else:
            lastcycle = True
        self.runMajorCycleCore(lastcycle)

        if self.IBtool != None:
            self.IBtool.endmajorcycle()
        ### Gather residuals (if needed) and normalize by weight
        for immod in range(0,self.NF):
            self.PStools[immod].gatherresidual() 
            self.PStools[immod].divideresidualbyweight()
            self.PStools[immod].multiplymodelbyweight()

#############################################
    def predictModel(self):
        for immod in range(0,self.NF):
            self.PStools[immod].dividemodelbyweight()
            self.PStools[immod].scattermodel() 

        self.predictModelCore()
        ###return the model images back to whatever state they were 
        for immod in range(0,self.NF):
            self.PStools[immod].multiplymodelbyweight()
##        self.SItool.predictmodel();

#############################################
    def dryGridding(self):
        self.SItool.drygridding(**(self.cfcachepars)) ;
#############################################
## Overloaded for parallel runs
    def fillCFCache(self):
        cfcName = self.allgridpars['0']['cfcache'];
        cflist=[];
        if (not (cfcName == '')):
            cflist=[f for f in os.listdir(cfcName) if re.match(r'CFS*', f)];
        #cflist = ["CFS_0_0_CF_0_0_0.im"];
        self.cfcachepars['cflist']=cflist;

        #self.SItool.fillcfcache(**(self.cfcachepars), self.allgridpars['0']['gridder'],cfcName);
        
        self.SItool.fillcfcache(cflist, self.allgridpars['0']['gridder'],
                                cfcName,
                                self.allgridpars['0']['psterm'],
                                self.allgridpars['0']['aterm'],
                                self.allgridpars['0']['conjbeams']);
                  
#############################################
    def reloadCFCache(self):
        self.SItool.reloadcfcache();

#############################################
    def makePB(self):
        self.makePBCore()
        for immod in range(0,self.NF):
            self.PStools[immod].normalizeprimarybeam() 

#############################################
    def makePBCore(self):
        self.SItool.makepb()

#############################################

## Overloaded for parallel runs
    def setWeighting(self):
        ## Set weighting parameters, and all pars common to all fields.
        self.SItool.setweighting( **(self.weightpars) )
        
 #       print "get set density from python"
 #       self.SItool.getweightdensity()
 #       self.SItool.setweightdensity()

        
#############################################
## Overloaded for parallel runs
    def makePSFCore(self):
        self.SItool.makepsf()
#############################################
## Overloaded for parallel runs
    def runMajorCycleCore(self, lastcycle):
        self.SItool.executemajorcycle(controls={'lastcycle':lastcycle})
#############################################
## Overloaded for parallel runs
    def predictModelCore(self):
        self.SItool.predictmodel()
#############################################

    def runMinorCycle(self):
        self.runMinorCycleCore()
#############################################

    def runMinorCycleCore(self):
        # Get iteration control parameters
        iterbotrec = self.IBtool.getminorcyclecontrols()
        ##print "Minor Cycle controls : ", iterbotrec

        self.IBtool.resetminorcycleinfo() 

        #
        # Run minor cycle
        self.ncycle+=1
        for immod in range(0,self.NF):  
            if self.stopMinor[str(immod)]<3 :
                exrec = self.SDtools[immod].executeminorcycle( iterbotrecord = iterbotrec )
                #print '.... iterdone for ', immod, ' : ' , exrec['iterdone']
                self.IBtool.mergeexecrecord( exrec )
                if os.environ.has_key('SAVE_ALL_AUTOMASKS') and os.environ['SAVE_ALL_AUTOMASKS']=="true":
                    maskname = self.allimpars[str(immod)]['imagename']+'.mask'
                    tempmaskname = self.allimpars[str(immod)]['imagename']+'.autothresh'+str(self.ncycle)
                    if os.path.isdir(maskname):
                        shutil.copytree(maskname, tempmaskname)
                
                # Some what duplicated as above but keep a copy of the previous mask
                # for interactive automask to revert to it if the current mask
                # is not used (i.e. reached deconvolution stopping condition).
                #if self.iterpars['interactive'] and self.alldecpars[str(immod)]['usemask']=='auto-thresh':
                if self.alldecpars[str(immod)]['usemask']=='auto-thresh':
                    maskname = self.allimpars[str(immod)]['imagename']+'.mask'
                    prevmaskname=self.allimpars[str(immod)]['imagename']+'.prev.mask'
                    if os.path.isdir(maskname):
                        if os.path.isdir(prevmaskname):
                            shutil.rmtree(prevmaskname)
                        shutil.copytree(maskname, prevmaskname)

#############################################
    def runMajorMinorLoops(self):
         self.runMajorCycle()
         while ( not self.hasConverged() ):
              self.runMinorCycle()
              self.runMajorCycle()

#############################################

    def plotReport( self, summ={} ,fignum=1 ):

        if not ( summ.has_key('summaryminor') and summ.has_key('summarymajor') and summ.has_key('threshold') and summ['summaryminor'].shape[0]==6 ):
            print 'Cannot make summary plot. Please check contents of the output dictionary from tclean.'
            return summ

        import pylab as pl
        from numpy import max as amax

        # 0 : iteration number (within deconvolver, per cycle)
        # 1 : peak residual
        # 2 : model flux
        # 3 : cyclethreshold
        # 4 : deconvolver id
        # 5 : subimage id (channel, stokes..)

        pl.ioff()

        pl.figure(fignum)
        pl.clf();
        minarr = summ['summaryminor']
        if minarr.size==0:
            casalog.post("Zero iteration: no summary plot is generated.", "WARN")
        else:
	    iterlist = minarr[0,:]
	    eps=0.0
	    peakresstart=[]
	    peakresend=[]
	    modfluxstart=[]
	    modfluxend=[]
	    itercountstart=[]
	    itercountend=[]
	    peakresstart.append( minarr[1,:][0] )
	    modfluxstart.append( minarr[2,:][0] )
	    itercountstart.append( minarr[0,:][0] + eps )
	    peakresend.append( minarr[1,:][0] )
	    modfluxend.append( minarr[2,:][0] )
	    itercountend.append( minarr[0,:][0] + eps )
	    for ii in range(0,len(iterlist)-1):
		if iterlist[ii]==iterlist[ii+1]:
		    peakresend.append( minarr[1,:][ii] )
		    peakresstart.append( minarr[1,:][ii+1] ) 
		    modfluxend.append( minarr[2,:][ii] )
		    modfluxstart.append( minarr[2,:][ii+1] )
		    itercountend.append( iterlist[ii]-eps )
		    itercountstart.append( iterlist[ii]+eps )

	    peakresend.append( minarr[1,:][len(iterlist)-1] )
	    modfluxend.append( minarr[2,:][len(iterlist)-1] )
	    itercountend.append( minarr[0,:][len(iterlist)-1] + eps )

    #        pl.plot( iterlist , minarr[1,:] , 'r.-' , label='peak residual' , linewidth=1.5, markersize=8.0)
    #        pl.plot( iterlist , minarr[2,:] , 'b.-' , label='model flux' )
    #        pl.plot( iterlist , minarr[3,:] , 'g--' , label='cycle threshold' )

	    pl.plot( itercountstart , peakresstart , 'r.--' , label='peak residual (start)')
	    pl.plot( itercountend , peakresend , 'r.-' , label='peak residual (end)',linewidth=2.5)
	    pl.plot( itercountstart , modfluxstart , 'b.--' , label='model flux (start)' )
	    pl.plot( itercountend , modfluxend , 'b.-' , label='model flux (end)',linewidth=2.5 )
	    pl.plot( iterlist , minarr[3,:] , 'g--' , label='cycle threshold', linewidth=2.5 )
	    maxval = amax( minarr[1,:] )
	    maxval = max( maxval, amax( minarr[2,:] ) )
	    
	    bcols = ['b','g','r','y','c']
	    minv=1
	    niterdone = len(minarr[4,:])
	  
	    if len(summ['summarymajor'].shape)==1 and summ['summarymajor'].shape[0]>0 :       
		pl.vlines(summ['summarymajor'],0,maxval, label='major cycles', linewidth=2.0)

	    pl.hlines( summ['threshold'], 0, summ['iterdone'] , linestyle='dashed' ,label='threshold')
	
	    pl.xlabel( 'Iteration Count' )
	    pl.ylabel( 'Peak Residual (red), Model Flux (blue)' )

	    ax = pl.axes()
	    box = ax.get_position()
	    ax.set_position([box.x0, box.y0, box.width, box.height*0.8])

	    pl.legend(loc='lower center', bbox_to_anchor=(0.5, 1.05),
		      ncol=3, fancybox=True, shadow=True)

	    pl.savefig('summaryplot_'+str(fignum)+'.png')
	    pl.ion()

        return summ;

#######################################################
#######################################################

