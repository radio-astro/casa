import os
import commands
import math
import shutil
import string
import time
from taskinit import *
import copy
from simple_cluster import simple_cluster

'''
A set of helper functions for the tasks  tclean, xxxx

Summary...

    
'''

debug = False

#############################################
#############################################

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
        self.weightpars = params.getWeightPars()
        # Iteration parameters
        self.iterpars = params.getIterPars() ## Or just params.iterpars

        ## Number of fields ( main + outliers )
        self.NF = len(self.allimpars.keys())
        ## Number of nodes. This gets set for parallel runs
        ## It can also be used serially to process the major cycle in pieces.
        self.NN = 1 

        isvalid = self.checkParameters()
        if isvalid==False:
            print 'Invalid parameters'

#############################################
    def checkParameters(self):
        # Copy the imagename from impars to decpars, for each field.
        for immod in range(0,self.NF):
            self.alldecpars[str(immod)]['imagename'] = self.allimpars[str(immod)]['imagename']
        return True

#############################################
#############################################
    def initializeImagers(self):
        
        ## Initialize the tool for the current node
        self.SItool = casac.synthesisimager()
        
        ## Send in selection parameters for all MSs in the list.
        for mss in sorted( (self.allselpars).keys() ):
            self.SItool.selectdata( **(self.allselpars[mss]) )

        ## For each image-field, define imaging parameters
        nimpars = copy.deepcopy(self.allimpars)
        for fld in range(0,self.NF):
            self.SItool.defineimage( **( nimpars[str(fld)]  ) )
        #for fld in range(0,self.NF):
        #    self.SItool.defineimage( **( self.allimpars[str(fld)]  ) )

        ## Set weighting parameters, and all pars common to all fields.
        self.SItool.setweighting( **(self.weightpars) )


#############################################

    def initializeDeconvolvers(self):
         for immod in range(0,self.NF):
              self.SDtools.append(casac.synthesisdeconvolver())
              self.SDtools[immod].setupdeconvolution(decpars=self.alldecpars[str(immod)])

#############################################
    ## Overloaded by ParallelCont
    def initializeParallelSync(self):
        for immod in range(0,self.NF):
            self.PStools.append(casac.synthesisparsync())
            syncpars = {'imagename':self.allimpars[str(immod)]['imagename']}
            self.PStools[immod].setupparsync(syncpars=syncpars)

#############################################

    def initializeIterationControl(self):
        self.IBtool = casac.synthesisiterbot()
        itbot = self.IBtool.setupiteration(iterpars=self.iterpars)

#############################################

    def restoreImages(self):
         for immod in range(0,self.NF):
              self.SDtools[immod].restore()

#############################################

    def getSummary(self,fignum=1):
        summ = self.IBtool.getiterationsummary()
        self.plotReport( summ, fignum )

#############################################
    def deleteImagers(self):
        if self.SItool != None:
            self.SItool.done()

    def deleteDeconvolvers(self):
         for immod in range(0,len(self.SDtools)):
              self.SDtools[immod].done()

    def deleteParSyncs(self):
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
         self.NN=1
         self.SItool=None
         self.SDtools=[]
         self.PStools=[]
         self.IBtool=None
    
#############################################

    def deleteTools(self):
         self.deleteImagers()
         self.deleteDeconvolvers()
         self.deleteParSyncs()
         self.deleteIterBot()
         self.initDefaults()
         self.deleteCluster()

#############################################

    def hasConverged(self):
        # Merge peak-res info from all fields to decide iteration parameters
         for immod in range(0,self.NF):
              initrec =  self.SDtools[immod].initminorcycle() 
              self.IBtool.mergeinitrecord( initrec );
              print "Peak res of field ",immod, " : " ,initrec['peakresidual']

        # Check with the iteration controller about convergence.
         stopflag = self.IBtool.cleanComplete()
         print 'Converged : ', stopflag
         return stopflag

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
            self.PStools[immod].scattermodel() 

        self.runMajorCycleCore()

        if self.IBtool != None:
            self.IBtool.endmajorcycle()
        ### Gather residuals (if needed) and normalize by weight
        for immod in range(0,self.NF):
            self.PStools[immod].gatherresidual() 
            self.PStools[immod].divideresidualbyweight()


#############################################
## Overloaded for parallel runs
    def makePSFCore(self):
        self.SItool.makepsf()

#############################################
## Overloaded for parallel runs
    def runMajorCycleCore(self):
        ### Run major cycle
        self.SItool.executemajorcycle(controls={})
#############################################

    def runMinorCycle(self):
        # Get iteration control parameters
        iterbotrec = self.IBtool.getminorcyclecontrols()
        # Run minor cycle
        for immod in range(0,self.NF):
             exrec = self.SDtools[immod].executeminorcycle( iterbotrecord = iterbotrec )
             self.IBtool.mergeexecrecord( exrec )

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

        pl.plot( minarr[0,:] , minarr[1,:] , 'r.-' , label='peak residual' , linewidth=1.5, markersize=8.0)
        #pl.plot( minarr[0,:] , minarr[2,:] , 'b.-' , label='model flux' )
        pl.plot( minarr[0,:] , minarr[3,:] , 'g,' , label='cycle threshold' )
        bcols = ['b','g','r','y','c']
        minv=1
        niterdone = len(minarr[4,:])
        maxv=niterdone
        for ind in range(1,niterdone):
            if ( minarr[4,ind-1] != minarr[4,ind] )  or  ind == niterdone-1 :
                maxv = ind
                if ind == niterdone-1:
                    maxv = niterdone
                val = int(minarr[4,ind-1])
                pl.bar(minv, 1.0, maxv-minv, 0.0, color=bcols[val%5], alpha=0.1, linewidth=0)
                minv = ind+1
      
        if len(summ['summarymajor'].shape)==1 and summ['summarymajor'].shape[0]>0 :       
            pl.vlines(summ['summarymajor']+0.5,0,1, label='major cycles', linewidth=3.0)

        pl.hlines( summ['threshold'], 0, niterdone , linestyle='dashed' ,label='threshold')
    
        pl.xlabel( 'Iteration Count' )
        pl.ylabel( 'Peak Residual' )

        pl.legend()

        pl.savefig('summaryplot_'+str(fignum)+'.png')
        pl.ion()

        return summ;


#######################################################
#######################################################


#############################################
#############################################
## Parallelize only major cycle.
#############################################
class PyParallelContSynthesisImager(PySynthesisImager):

    def __init__(self,params=None,clusterdef=''):

         PySynthesisImager.__init__(self,params)

         self.PH = PyParallelImagerHelper( clusterdef )
         self.NN = self.PH.NN
         self.allselpars = self.PH.partitionContDataSelection(self.allselpars)

#############################################
#############################################
    def initializeImagers(self):
        joblist=[]
        for node in range(0,self.NN):
            
            ## Initialize the tool for the current node
            self.PH.runcmd("toolsi = casac.synthesisimager()", node)

            ## Send in Selection parameters for all MSs in the list
            for mss in sorted( (self.allselpars[str(node)]).keys() ):
                joblist.append( self.PH.runcmd("toolsi.selectdata( **"+str(self.allselpars[str(node)][mss])+")", node) )

            ## For each image-field, define imaging parameters
            nimpars = copy.deepcopy(self.allimpars)
            for fld in range(0,self.NF):
                if self.NN>1:
                    nimpars[str(fld)]['imagename'] = nimpars[str(fld)]['imagename']+'.n'+str(node)
                joblist.append( self.PH.runcmd("toolsi.defineimage( **" + str( nimpars[str(fld)] ) + ")", node ) )
            
            joblist.append( self.PH.runcmd("toolsi.setweighting( **" + str(self.weightpars) + ")", node ) )

        self.PH.checkJobs( joblist )

#############################################

    def initializeParallelSync(self):
        for immod in range(0,self.NF):
            self.PStools.append(casac.synthesisparsync())
            syncpars = {'imagename':self.allimpars[str(immod)]['imagename']}
            partnames = []
            if(self.NN>1):
                for node in range(0,self.NN):
                    partnames.append( self.allimpars[str(immod)]['imagename']+'.n'+str(node)  )
                syncpars['partimagenames'] = partnames
            self.PStools[immod].setupparsync(syncpars=syncpars)


#############################################

    def deleteImagers(self):
         self.PH.runcmd("toolsi.done()")

    def deleteCluster(self):
         self.PH.takedownCluster()
    
#############################################
    def makePSFCore(self):
        ### Make PSFs
        joblist=[]
        for node in range(0,self.PH.NN):
             joblist.append( self.PH.runcmd("toolsi.makepsf()",node) )
        self.PH.checkJobs( joblist ) # this call blocks until all are done.

#############################################

    def runMajorCycleCore(self):
        ### Run major cycle
        joblist=[]
        for node in range(0,self.PH.NN):
             joblist.append( self.PH.runcmd("toolsi.executemajorcycle(controls={})",node) )
        self.PH.checkJobs( joblist ) # this call blocks until all are done.

#############################################

#############################################
# Parallelize both the major and minor cycle for Cube imaging
# Run a separate instance of PySynthesisImager on each node.
#### ( later, use the live-object interface of ImStore to reference-break the cubes )
#### For nprocesses > nnodes, run the whole 'clean' loop multiple times. 
#############################################
class PyParallelCubeSynthesisImager():

    def __init__(self,params=None,clusterdef=''):

        self.params=params

        allselpars = params.getSelPars()
        allimagepars = params.getImagePars()
        self.weightpars = params.getWeightPars()
        self.decpars = params.getDecPars()
        self.iterpars = params.getIterPars()
        
        self.PH = PyParallelImagerHelper( clusterdef )
        self.NN = self.PH.NN
        ## Partition both data and image coords the same way.
        self.allselpars = self.PH.partitionCubeDataSelection(allselpars)
        self.allimpars = self.PH.partitionCubeDeconvolution(allimagepars)

        joblist=[]
        for node in range(0,self.NN):
            joblist.append( self.PH.runcmd("from refimagerhelper import ImagerParameters, PySynthesisImager", node) )
        self.PH.checkJobs( joblist )
            

    def initializeImagers(self):
        joblist=[]
        for node in range(0,self.NN):

            joblist.append( self.PH.runcmd("paramList = ImagerParameters()", node) )
            joblist.append( self.PH.runcmd("paramList.setSelPars("+str(self.allselpars[str(node)])+")", node) )
            joblist.append( self.PH.runcmd("paramList.setImagePars("+str(self.allimpars[str(node)])+")", node) )
            joblist.append( self.PH.runcmd("paramList.setWeightPars("+str(self.weightpars)+")", node) )
            joblist.append( self.PH.runcmd("paramList.setDecPars("+str(self.decpars)+")", node) )
            joblist.append( self.PH.runcmd("paramList.setIterPars("+str(self.iterpars)+")", node) )

            joblist.append( self.PH.runcmd("imager = PySynthesisImager(params=paramList)", node) )
            joblist.append( self.PH.runcmd("imager.initializeImagers()", node) )

        self.PH.checkJobs( joblist )

    def initializeDeconvolvers(self):
        joblist=[]
        for node in range(0,self.NN):
            joblist.append( self.PH.runcmd("imager.initializeDeconvolvers()", node) )
        self.PH.checkJobs( joblist )

    def initializeParallelSync(self):
        joblist=[]
        for node in range(0,self.NN):
            joblist.append( self.PH.runcmd("imager.initializeParallelSync()", node) )
        self.PH.checkJobs( joblist )

    def initializeIterationControl(self):
        joblist=[]
        for node in range(0,self.NN):
            joblist.append( self.PH.runcmd("imager.initializeIterationControl()", node) )
        self.PH.checkJobs( joblist )

    def makePSF(self):
        joblist=[]
        for node in range(0,self.NN):
            joblist.append( self.PH.runcmd("imager.makePSF()", node) )
        self.PH.checkJobs( joblist )

    def runMajorMinorLoops(self):
        joblist=[]
        for node in range(0,self.NN):
            joblist.append( self.PH.runcmd("imager.runMajorMinorLoops()", node) )
        self.PH.checkJobs( joblist )

    def runMajorCycle(self):
        joblist=[]
        for node in range(0,self.NN):
            joblist.append( self.PH.runcmd("imager.runMajorCycle()", node) )
        self.PH.checkJobs( joblist )

    def restoreImages(self):
        joblist=[]
        for node in range(0,self.NN):
            joblist.append( self.PH.runcmd("imager.restoreImages()", node) )
        self.PH.checkJobs( joblist )

    def getSummary(self):
        joblist=[]
        for node in range(0,self.NN):
            joblist.append( self.PH.runcmd("imager.getSummary("+str(node)+")", node) )
        self.PH.checkJobs( joblist )

    def deleteTools(self):
        joblist=[]
        for node in range(0,self.NN):
            joblist.append( self.PH.runcmd("imager.deleteTools()", node) )
        self.PH.checkJobs( joblist )

#############################################
#############################################
#############################################

class PyParallelDeconvolver(PySynthesisImager):

    def __init__(self,params,clusterdef=''):

        PySynthesisImager.__init__(self,params)

        self.PH = PyParallelImagerHelper( clusterdef )
        self.NN = self.PH.NN
        self.NF = len( allimpars.keys() )
        if self.NF != self.NN:
             print 'For now, cannot handle nfields != nnodes. Will implement round robin allocation later.'
             print 'Using only ', self.NN, ' fields and nodes'
             

#############################################
    def initializeDeconvolvers(self):
         joblist=[]
         for immod in range(0,self.NF):
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
#############################################

    def hasConverged(self):
        # Merge peak-res info from all fields to decide iteration parameters
        self.PH.runcmdcheck("initrec = toolsd.initminorcycle()")

        for immod in range(0,self.NF):
             retrec = self.PH.pullval("initrec", immod )
             self.IBtool.mergeinitrecord( retrec[immod] )
             print "Peak res of field ",immod, " on node ", immod , ": " ,retrec[immod]['peakresidual']

        # Check with the iteration controller about convergence.
        stopflag = self.IBtool.cleanComplete()
        print 'Converged : ', stopflag
        return stopflag

#############################################

    def runMinorCycle(self):
        # Get iteration control parameters
        iterbotrec = self.IBtool.getminorcyclecontrols()
        # Run minor cycle
        self.PH.CL.push( iterbotrec = iterbotrec )

        self.PH.runcmdcheck( "exrec = toolsd.executeminorcycle(iterbotrec)" )

        for immod in range(0,self.NF):
             retrec = self.PH.pullval("exrec", immod )
             self.IBtool.mergeexecrecord( retrec[immod] )

#############################################
#############################################



#############################################
###  Parallel Imager Helper.
#############################################
class PyParallelImagerHelper():

    def __init__(self,clusterdef=''):

        ############### Cluster Info
         self.clusterdef=clusterdef
         self.CL=None
         self.sc=None

         # Initialize cluster, and partitioning.
        ############### Number of nodes to parallelize on
         if len(self.clusterdef) != 0:
              self.NN = self.setupCluster()
         else:
              self.NN = 1

#############################################
## Very rudimentary partitioning - only for tests. The actual code needs to go here.
    def partitionContDataSelection(self,oneselpars={}):

        synu = casac.synthesisutils()
        allselpars =  synu.contdatapartition( oneselpars , self.NN )
        synu.done()

#        allselpars = {}
#        print oneselpars
#        for node in range(0,self.NN):
#            ## Replicate the Selection pars for all nodes, before modifying them.
#            allselpars[str(node)]  = copy.deepcopy(oneselpars) 
#
#            ######## WARNING : Very special case for SPW 0 of points_2spw.ms
#            if allselpars[str(node)]['ms0']['msname'] == "DataTest/point_twospws.ms":
#                if node==0:
#                    allselpars[str(node)]['ms0']['spw'] = allselpars[str(node)]['ms0']['spw'] + ':0~9'
#                else:
#                    allselpars[str(node)]['ms0']['spw'] = allselpars[str(node)]['ms0']['spw'] + ':10~19'
#
#            ######## WARNING : Very special case for twopoints_twochan.ms
#            if allselpars[str(node)]['ms0']['msname'] == "DataTest/twopoints_twochan.ms":
#                if node==0:
#                    allselpars[str(node)]['ms0']['spw'] = allselpars[str(node)]['ms0']['spw'] + ':0'
#                else:
#                    allselpars[str(node)]['ms0']['spw'] = allselpars[str(node)]['ms0']['spw'] + ':1'
#
#
        print 'Partitioned Selection : ', allselpars
        return allselpars

#############################################
## Very rudimentary partitioning - only for tests. The actual code needs to go here.
    def partitionCubeDataSelection(self,oneselpars={}):

        synu = casac.synthesisutils()
        allselpars =  synu.cubedatapartition( oneselpars , self.NN )
        synu.done()

#        allselpars = {}
#        print oneselpars
#        for node in range(0,self.NN):
#            ## Replicate the Selection pars for all nodes, before modifying them.
#            allselpars[str(node)]  = copy.deepcopy(oneselpars) 
#
#            ######## WARNING : Very special case for SPW 0 of points_2spw.ms
#            if allselpars[str(node)]['ms0']['msname'] == "DataTest/point_twospws.ms":
#                if node==0:
#                    allselpars[str(node)]['ms0']['spw'] = allselpars[str(node)]['ms0']['spw'] + ':0~9'
#                else:
#                    allselpars[str(node)]['ms0']['spw'] = allselpars[str(node)]['ms0']['spw'] + ':10~19'
#
#            ######## WARNING : Very special case for twopoints_twochan.ms
#            if allselpars[str(node)]['ms0']['msname'] == "DataTest/twopoints_twochan.ms":
#                if node==0:
#                    allselpars[str(node)]['ms0']['spw'] = allselpars[str(node)]['ms0']['spw'] + ':0'
#                else:
#                    allselpars[str(node)]['ms0']['spw'] = allselpars[str(node)]['ms0']['spw'] + ':1'
#

        print 'Partitioned Selection : ', allselpars
        return allselpars

#############################################
    def partitionCubeDeconvolution(self,impars={}):

        synu = casac.synthesisutils()
        allimpars =  synu.cubeimagepartition( impars , self.NN )
        synu.done()

#        allimpars={}
#        for node in range(0,self.NN):
#            allimpars[str(node)]  = copy.deepcopy(impars) 
#            for field in allimpars[str(node)].keys():
#                 print allimpars[str(node)][field]
#                 if not allimpars[str(node)][field].has_key('nchan'):
#                      allimpars[str(node)][field]['nchan']=1
#                 else:
#                     allimpars[str(node)][field]['nchan'] = int( ( allimpars[str(node)][field]['nchan'])/self.NN )
#                     ############# WARNING : Very special case for points_2spw.ms
#                     if node==0:
#                         allimpars[str(node)][field]['freqstart'] = '1.0GHz'
#                     else:
#                         allimpars[str(node)][field]['freqstart'] = '1.2GHz'
#                 allimpars[str(node)][field]['imagename'] = allimpars[str(node)][field]['imagename']+'.n'+str(node)
        print 'ImSplit : ', allimpars
        return allimpars


#############################################
    def setupCluster(self):
        # Initialize cluster
        if((self.clusterdef != '') and os.path.exists(self.clusterdef)):
            self.sc=simple_cluster()
            if(self.sc.get_status()==None):
                self.sc.init_cluster(self.clusterdef,'aatest')
                
        self.CL=simple_cluster.getCluster()._cluster
        numproc=len(self.CL.get_engines())
        numprocperhost=len(self.CL.get_engines())/len(self.CL.get_nodes()) if (len(self.CL.get_nodes()) >0 ) else 1

        owd=os.getcwd()
        self.CL.pgc('import os')
        self.CL.pgc('os.chdir("'+owd+'")')
        os.chdir(owd)
        print "Setting up ", numproc, " engines."
        return numproc

#############################################
    def takedownCluster(self):
        # Check that all nodes have returned, before stopping the cluster
         self.checkJobs()
         print 'Stopping cluster'
         self.sc.stop_cluster()
         self.CL=None
         self.sc=None

#############################################
    # This is a blocking call that will wait until jobs are done.
    def checkJobs(self,joblist=[]):
        numcpu = len(self.CL.get_engines())
        
        if len(joblist)==0:
             joblist = range(numcpu)
             for k in range(numcpu):
                  joblist[k] = self.CL.odo('casalog.post("node '+str(k)+' has completed its job")', k)

        print 'Blocking for nodes to finish'
        over=False
        while(not over):
            overone=True
            time.sleep(1)
            for k in range(len(joblist)):
                try:
                    overone =  self.CL.check_job(joblist[k],False) and overone
                except Exception,e:
                     raise Exception(e)
            over = overone
        print '...done'

#############################################
    def runcmd(self, cmdstr="", node=-1):
         if node >= 0:
              return self.CL.odo( cmdstr , node)
         else:
              self.CL.pgc( cmdstr )

#############################################
    def runcmdcheck(self, cmdstr):
         joblist=[]
         for node in range(0,self.NN):
              joblist.append( self.CL.odo( cmdstr, node ) )
         self.checkJobs( joblist )

#############################################
#############################################
    def pullval(self, varname="", node=0):
         return self.CL.pull( varname , node )

##########################################################################################

    def OLD_calculateImages(self):
        # If any of the input images is a cube, all fields must be cubes
        # with the same LSRK output channelization

        # Do Cube Image if:
        # 1) All output images have the same LSRK channelization
        # 2) Total Pixel * nchan (memory footprint) is greater than some
        #    threshold (like 2 core's worth)

        # Information from the cluster
        numNodes = 1
        numCorePerNode = 4
        memoryPerNode = 24

        cubeImage = False
        totalPixels = 0
        for imdef in imageParameters:
            totalPixels += 1
            
            if imdef.nchan > 1:
                cubeImage = True
                break

        if cubeImage:
            # Partition on output channel and time

            # If we partition only based on memory how many chunks do we need
            numChunks = math.ceil(totalPixels * imageParameters[0].nchan
                                  / (float(memoryPerNode)/numCorePerNode))

            # Match numChunks to an integral number * the number of cores
            numChunks = math.ceil(numChunks/(numCorePerNode * numNodes)) * \
                        numCorePerNode * numNodes
            
            # All fields have numChunk deconvoler subimages specified
            # by range of output channel.
            
            

        else:
            # Continuum case partition on input channel and time
            # could still be multiple channels here (particularly if we
            # are in the case of small images w/ different LSRK channels

            # See how many engines we can get per node
            enginesPerNode = numCorePerNode
            while totalPixels > memoryPerNode/enginesPerNode:
                enginesPerNode -= 1

            numChunks = enginesPerNode * numNodes

            # Here all deconvolvers see all fields the data selection can
            # be in time or data channel (or both)



        imagingPlan = {'imagingSet':[], 'imageCombination':{}}

        imageSet = {'deconvolverField':[imageParameters],
                    'dataselection':[dataselection]}

        imagingPlan['imagingSet'].append(imageSet)

        return imagingPlan

######################################################
######################################################
######################################################
######################################################

class ImagerParameters():

    def __init__(self,
                 msname='',field='',spw='',usescratch=True,readonly=True,
                 outlierfile='',
                 imagename='', nchan=1, freqstart='1.0GHz', freqstep='1.0GHz',
                 imsize=[1,1], cellsize=[10.0,10.0],phasecenter='19:59:28.500 +40.44.01.50',
                 ftmachine='ft', startmodel='', weighting='natural',

                 aterm=True,
                 psterm=True,
                 mterm=False,
                 wbawp = True,
                 cfcache = "",
                 dopointing = False,
                 dopbcorr = True,
                 conjbeams = True,
                 computepastep =360.0,
                 rotatepastep =5.0,

                 algo='test',
                 niter=0, cycleniter=0, cyclefactor=1.0,
                 minpsffraction=0.1,maxpsffraction=0.8,
                 threshold='0.0Jy',loopgain=0.1,
                 interactive=False):

        ## Selection params. For multiple MSs, all are lists.
        ## For multiple nodes, the selection parameters are modified inside PySynthesisImager
        self.allselpars = {'msname':msname, 'field':field, 'spw':spw, 'usescratch':usescratch, 'readonly':readonly}

        ## Imaging/deconvolution parameters
        ## The outermost dictionary index is image field. 
        ## The '0' or main field's parameters come from the task parameters
        ## The outlier '1', '2', ....  parameters come from the outlier file
        self.outlierfile = outlierfile
        ## Initialize the parameter lists with the 'main' or '0' field's parameters
        self.allimpars = { '0' :{'imagename':imagename, 'nchan':nchan, 'imsize':imsize, 'cellsize':cellsize, 'phasecenter':phasecenter, 
                                 'freqstart':freqstart, 'freqstep':freqstep, 'ftmachine':ftmachine, 'startmodel':startmodel,
                                 'aterm': aterm, 'psterm':psterm, 'mterm': mterm, 'wbawp': wbawp, 'cfcache': cfcache,
                                 'dopointing':dopointing, 'dopbcorr':dopbcorr, 'conjbeams':conjbeams, 'computepastep':computepastep,
                                 'rotatepastep':rotatepastep
                                 } 
                           }
        self.weightpars = {'type':weighting } 
        self.alldecpars = { '0' : { 'id':0, 'algo':algo } }

        ## Iteration control. 
        self.iterpars = { 'niter':niter, 'cycleniter':cycleniter, 'threshold':threshold, 'loopgain':loopgain, 'interactive':interactive }  # Ignoring cyclefactor, minpsffraction, maxpsffraction for now.

        ## List of supported parameters in outlier files.
        ## All other parameters will default to the global values.
        self.outimparlist = ['imagename','nchan','imsize','cellsize','phasecenter','ftmachine','startmodel','freqstart','freqstep']
        self.outweightparlist=[]
        self.outdecparlist=['algo','startmodel']


    def getSelPars(self):
        return self.allselpars
    def getImagePars(self):
        return self.allimpars
    def getWeightPars(self):
        return self.weightpars
    def getDecPars(self):
        return self.alldecpars
    def getIterPars(self):
        return self.iterpars

    def setSelPars(self,selpars):
        self.allselpars = selpars
    def setImagePars(self,impars):
        self.allimpars = impars
    def setWeightPars(self,weightpars):
        self.weightpars = weightpars
    def setDecPars(self,decpars):
        self.alldecpars = decpars
    def setIterPars(self,iterpars):
        self.iterpars = iterpars



    def checkParameters(self):
        casalog.origin('tclean.checkParameters')
        casalog.post('Verifying Input Parameters')
        # Init the error-string
        errs = "" 
        errs += self.checkAndFixSelectionPars()
        errs += self.makeImagingParamLists()
        errs += self.checkAndFixImagingPars()
        errs += self.checkAndFixIterationPars()

        ## If there are errors, print a message and exit.
        if len(errs) > 0:
            casalog.post('Parameter Errors : \n' + errs,'WARN')
            return False
        return True

    ###### Start : Parameter-checking functions ##################
    def OldcheckAndFixSelectionPars(self):
        errs=""
        # msname, field, spw, etc must all be equal-length lists of strings.
        if not self.allselpars.has_key('msname'):
            errs = errs + 'MS name(s) not specified'
        else:
            if type(self.allselpars['msname'])==str:
                self.allselpars['msname']=[self.allselpars['msname']]
            nmsname = len(self.allselpars['msname'])
            selkeys = ['field','spw']
            for par in selkeys:
                if self.allselpars.has_key(par):
                    if self.allselpars[par]=='':
                        self.allselpars[par] = []
                        for it in range(0,nvis):
                            self.allselpars[par].append('')
                    if type(self.allselpars[par])==str:
                        self.allselpars[par]=[self.allselpars[par]]
                    if len(self.allselpars[par]) != nvis:
                        errs = errs + "Selection for " + par + " must be a list of " + str(nvis) + " selection strings\n"
                else:
                    errs = errs + "Selection for " + par + " is unspecified\n"

        return errs

    def checkAndFixSelectionPars(self):
        errs=""
        # msname, field, spw, etc must all be equal-length lists of strings, or all except msname must be of length 1.
        if not self.allselpars.has_key('msname'):
            errs = errs + 'MS name(s) not specified'
        else:

            selkeys = self.allselpars.keys()

            # Convert all non-list parameters into lists.
            for par in selkeys:
                if type( self.allselpars[par] ) != list:
                    self.allselpars[par] = [ self.allselpars[par]  ]
                    
            # Check that all are the same length as nvis
            # If not, and if they're single, replicate them nvis times
            nvis = len(self.allselpars['msname'])
            for par in selkeys:
                if len( self.allselpars[par] ) > 1 and len( self.allselpars[par] ) != nvis:
                    errs = errs + str(par) + ' must have a single entry, or ' + str(nvis) + ' entries to match vis list \n'
                    return errs
                else: # Replicate them nvis times if needed.
                    if len( self.allselpars[par] ) == 1:
                        for ms in range(1,nvis):
                            self.allselpars[par].append( self.allselpars[par][0] )
                    

            # Now, all parameters are lists of strings each of length 'nvis'.
            # Put them into separate dicts per MS.
            selparlist={}
            for ms in range(0,nvis):
                selparlist[ 'ms'+str(ms) ] = {}
                for par in selkeys:
                    selparlist[ 'ms'+str(ms) ][ par ] = self.allselpars[par][ms]

#            print selparlist

            self.allselpars = selparlist

        return errs


    def makeImagingParamLists(self ):
        errs=""

        ## Multiple images have been specified. 
        ## (1) Parse the outlier file and fill a list of imagedefinitions
        ## OR (2) Parse lists per input parameter into a list of parameter-sets (imagedefinitions)
        ### The code below implements (1)
        outlierpars=[]
        parseerrors=""
        if len(self.outlierfile)>0:
            outlierpars,parseerrors = self.parseOutlierFile(self.outlierfile) 

        if len(parseerrors)>0:
            errs = errs + "Errors in parsing outlier file : " + parseerrors
            return errs

        # Initialize outlier parameters with defaults
        # Update outlier parameters with modifications from outlier files
        for immod in range(0, len(outlierpars)):
            modelid = str(immod+1)
            self.allimpars[ modelid ] = copy.deepcopy(self.allimpars[ '0' ])
            self.allimpars[ modelid ].update(outlierpars[immod]['impars'])
#            self.weightpars[ modelid ] = copy.deepcopy(self.weightpars[ '0' ])
#            self.weightpars[ modelid ].update(outlierpars[immod]['weightpars'])
            self.alldecpars[ modelid ] = copy.deepcopy(self.alldecpars[ '0' ])
            self.alldecpars[ modelid ].update(outlierpars[immod]['decpars'])
            self.alldecpars[ modelid ][ 'id' ] = immod+1  ## Try to eliminate.

        return errs

    def checkAndFixImagingPars(self ):
        errs=""

        ## Check all Image Definition Parameters
        for imkeys in self.allimpars.keys():
            eachimdef = self.allimpars[ imkeys ]

            ## Check/fix nchan : Must be a single integer.
            if eachimdef.has_key('nchan'):
                if type( eachimdef['nchan'] ) != int:
                    try:
                        eachimdef['nchan'] = eval( eachimdef['nchan'] )
                    except:
                        errs = errs + 'nchan must be an integer for field ' + eachimdef['imagename'] + '\n'
            else:
                errs = errs + 'nchan is not specified for field ' + eachimdef['imagename'] + '\n'

            ## Check/fix imsize : Must be an array with 2 integers
            if eachimdef.has_key('imsize'):
                tmpimsize = eachimdef['imsize']
                if type(tmpimsize) == str:
                    try:
                        tmpimsize = eval( eachimdef['imsize'] )
                    except:
                        errs = errs + 'imsize must be a single integer, or a list of 2 integers'

                if type(tmpimsize) == list:
                    if len(tmpimsize) == 2:
                        eachimdef['imsize'] = tmpimsize;  # not checking that elements are ints...
                    else:
                        errs = errs + 'imsize must be a single integer, or a list of 2 integers'
                elif type(tmpimsize) == int:
                    eachimdef['imsize'] = [tmpimsize, tmpimsize] 
                else:
                    errs = errs + 'imsize must be a single integer, or a list of 2 integers'
            else:
                errs = errs + 'imsize is not specified for field ' + eachimdef['imagename'] + '\n'

            ## Replace imsize by nx, ny
            eachimdef['nx']=eachimdef['imsize'][0]
            eachimdef['ny']=eachimdef['imsize'][1]
            eachimdef.pop( 'imsize' );

            ## Check/fix cellsize : Must be an array with 2 integers or strings
            if eachimdef.has_key('cellsize'):
                tmpcellsize = eachimdef['cellsize']
                if type(tmpcellsize) != str and type(tmpcellsize) != list:
                    errs = errs + 'cellsize must be a single string, or a list of 2 strings'

                if type(tmpcellsize) == list:
                    if len(tmpcellsize) == 2:
                        eachimdef['cellsize'] = tmpcellsize;  # not checking that elements are strings
                    else:
                        errs = errs + 'cellsize must be a single string, or a list of 2 strings'
                elif type(tmpcellsize) == str:
                    eachimdef['cellsize'] = [tmpcellsize, tmpcellsize] 
                else:
                    errs = errs + 'cellsize must be a single integer, or a list of 2 integers'
            else:
                errs = errs + 'cellsize is not specified for field ' + eachimdef['imagename'] + '\n'
            
            ## Replace imsize by nx, ny
            eachimdef['cellx']=eachimdef['cellsize'][0]
            eachimdef['celly']=eachimdef['cellsize'][1]
            eachimdef.pop( 'cellsize' );

#        print 'AFTER : ', self.allimpars

        return errs

    def checkAndFixIterationPars(self ):
        errs=""

        # Bother checking only if deconvolution iterations are requested
        if self.iterpars['niter']>0:
            # Make sure cycleniter is less than or equal to niter. 
            if self.iterpars['cycleniter']<=0 or self.iterpars['cycleniter'] > self.iterpars['niter']:
                self.iterpars['cycleniter'] = self.iterpars['niter']

        return errs

    ###### End : Parameter-checking functions ##################

    ## Parse outlier file and construct a list of imagedefinitions (dictionaries).
    def parseOutlierFile(self, outlierfilename="" ):
        returnlist = []
        errs=""  #  must be empty for no error

        if len(outlierfilename)>0 and not os.path.exists( outlierfilename ):
             errs +=  'Cannot find outlier file : ' +  outlierfilename + '\n'
             return returnlist, errs

        fp = open( outlierfilename, 'r' )
        thelines = fp.readlines()
        tempimpar={}
        tempweightpar={}
        tempdecpar={}
        for oneline in thelines:
            aline = oneline.replace('\n','')
#            aline = oneline.replace(' ','').replace('\n','')
            if len(aline)>0 and aline.find('#')!=0:
                parpair = aline.split("=")  
                parpair[0] = parpair[0].replace(' ','')
                #print parpair
                if len(parpair) != 2:
                    errs += 'Error in line containing : ' + oneline + '\n'
                if parpair[0] == 'imagename' and tempimpar != {}:
                    returnlist.append({'impars':tempimpar, 'weightpars':tempweightpar, 'decpars':tempdecpar} )
                    tempimpar={}
                    tempweightpar={}
                    tempdecpar={}
                usepar=False
                if parpair[0] in self.outimparlist:
                    tempimpar[ parpair[0] ] = parpair[1]
                    usepar=True
                if parpair[0] in self.outweightparlist:
                    tempweightpar[ parpair[0] ] = parpair[1]
                    usepar=True
                if parpair[0] in self.outdecparlist:
                    tempdecpar[ parpair[0] ] = parpair[1]
                    usepar=True
                if usepar==False:
                    print 'Ignoring unknown parameter pair : ' + oneline

        if len(errs)==0:
            returnlist.append( {'impars':tempimpar, 'weightpars':tempweightpar, 'decpars':tempdecpar} )

        ## Extra parsing for a few parameters.
        ## imsize
        try:
            for fld in range(0, len( returnlist ) ):
                if returnlist[ fld ]['impars'].has_key('imsize'):
                    imsize_e = eval( returnlist[ fld ]['impars']['imsize'] )
                    returnlist[ fld ]['impars']['imsize'] = imsize_e
        except:
            print 'Cannot evaluate outlier field parameter "imsize"'
        ## nchan
        try:
            for fld in range(0, len( returnlist ) ):
                if returnlist[ fld ]['impars'].has_key('nchan'):
                    nchan_e = eval( returnlist[ fld ]['impars']['nchan'] )
                    returnlist[ fld ]['impars']['nchan'] = nchan_e
        except:
            print 'Cannot evaluate outlier field parameter "nchan"'
        ## cellsize
        try:
            for fld in range(0, len( returnlist ) ):
                if returnlist[ fld ]['impars'].has_key('cellsize'):
                    tcell =  returnlist[ fld ]['impars']['cellsize']
                    tcell = tcell.replace(' ','').replace('[','').replace(']','').replace("'","")
                    tcells = tcell.split(',')
                    cellsize_e = []
                    for cell in tcells:
                        cellsize_e.append( cell )
                    returnlist[ fld ]['impars']['cellsize'] = cellsize_e
        except:
            print 'Cannot evaluate outlier field parameter "cellsize"'

        #print returnlist
        return returnlist, errs


    def printParameters(self):
        casalog.post('SelPars : ' + str(self.allselpars), 'INFO')
        casalog.post('ImagePars : ' + str(self.allimpars), 'INFO')
        casalog.post('Weightpars : ' + str(self.weightpars), 'INFO')
        casalog.post('DecPars : ' + str(self.alldecpars), 'INFO')
        casalog.post('IterPars : ' + str(self.iterpars), 'INFO')

