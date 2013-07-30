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
        self.allselpars = { '0' : params.getSelPars() }
        self.alldecpars = params.getDecPars()
        self.allimpars = params.getImagePars()
        self.allgridpars = params.getGridPars()
        self.iterpars = params.getIterPars() ## Or just params.iterpars

        self.NF = len(self.allimpars.keys())
        self.NN = len(self.allselpars.keys())

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

        for node in range(0,self.NN):
            nimpars = copy.deepcopy(self.allimpars)
            if self.NN>1:
                for ff  in range(0,self.NF):
                    nimpars[str(ff)]['imagename'] = nimpars[str(ff)]['imagename']+'.n'+str(node)

            self.SItools.append( casac.synthesisimager() )
            self.SItools[node].initializemajorcycle(self.allselpars[str(node)], nimpars, self.allgridpars)

#############################################

    def initializeDeconvolvers(self):
         for immod in range(0,self.NF):
              self.SDtools.append(casac.synthesisdeconvolver())
              self.SDtools[immod].setupdeconvolution(decpars=self.alldecpars[str(immod)])

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

    def initializeIterationControl(self):
        self.IBtool = casac.synthesisiterbot()
        itbot = self.IBtool.setupiteration(iterpars=self.iterpars)

#############################################

    def restoreImages(self):
         for immod in range(0,self.NF):
              self.SDtools[immod].restore()

#############################################

    def getSummary(self):
        summ = self.IBtool.getiterationsummary()
        return summ

#############################################
    def deleteImagers(self):
         for node in range(0,len(self.SItools)):
              self.SItools[node].done()


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
         print 'no cluster to delete'

    def initDefaults(self):
        # Reset globals/members
         self.NF=1
         self.NN=1
         self.SItools=[]
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
        for node in range(0,self.NN):
             self.SItools[node].makepsf()

#############################################
## Overloaded for parallel runs
    def runMajorCycleCore(self):
        ### Run major cycle
        for node in range(0,self.NN):
             self.SItools[node].executemajorcycle(controls={})
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
         self.allselpars = self.PH.partitionDataSelection(self.allselpars)

#############################################
#############################################
    def initializeImagers(self):
        joblist=[]
        for node in range(0,self.NN):
            nimpars = copy.deepcopy(self.allimpars)
            if self.NN>1:
                for ff  in range(0,self.NF):
                    nimpars[str(ff)]['imagename'] = nimpars[str(ff)]['imagename']+'.n'+str(node)

            ## Later, move out common commands into self.CL.pgc
            self.PH.runcmd("toolsi = casac.synthesisimager()", node)
            joblist.append( self.PH.runcmd("toolsi.initializemajorcycle("+str(self.allselpars[str(node)])+","+str(nimpars)+","+str(self.allgridpars)+")", node) )
        self.PH.checkJobs( joblist )

#############################################
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
    def partitionDataSelection(self,oneselpars={}):
        allselpars = {}
        for node in range(0,self.NN):
            allselpars[str(node)]  = copy.deepcopy(oneselpars['0']) 
            ### Temp modification. Put actual split-chan-range info here.
#            if not allselpars[str(node)].has_key('spw'):
#            allselpars[str(node)]['spw']=str(node)
#            elif allselpars[str(node)]['spw']=='':
#                allselpars[str(node)]['spw']=str(node)

            ######## WARNING : Very special case for SPW 0 of points_2spw.ms
            if node==0:
                allselpars[str(node)]['spw'][0] = allselpars[str(node)]['spw'][0] + ':0~9'
            else:
                allselpars[str(node)]['spw'][0] = allselpars[str(node)]['spw'][0] + ':10~19'
        print 'Partitioned Selection : ', allselpars
        return allselpars

#############################################
    def partitionCubeDeconvolution(self,impars={}):
        allimpars={}
        for node in range(0,self.NN):
            allimpars[str(node)]  = copy.deepcopy(impars) 
            for field in allimpars[str(node)].keys():
                 print allimpars[str(node)][field]
                 if not allimpars[str(node)][field].has_key('nchan'):
                      allimpars[str(node)][field]['nchan']=1
                 else:
                      allimpars[str(node)][field]['nchan'] = int( ( allimpars[str(node)][field]['nchan'])/self.NN )
                 allimpars[str(node)][field]['imagename'] = allimpars[str(node)][field]['imagename']+'.n'+str(node)
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

    def __init__(self,casalog,
                 vis='',field='',spw='',usescratch=True,
                 outlierfile='',
                 imagename='', nchan=1, imsize=[1,1],
                 cellsize=[10.0,10.0],phasecenter='19:59:28.500 +40.44.01.50',
                 ftmachine='ft', startmodel='', weighting='natural',
                 algo='test',
                 niter=0, cycleniter=0, cyclefactor=1.0,
                 minpsffraction=0.1,maxpsffraction=0.8,
                 threshold='0.0Jy',loopgain=0.1,
                 interactive=False):

        self.casalog = casalog

        self.allselpars = {'vis':vis, 'field':field, 'spw':spw, 'usescratch':usescratch}
        self.outlierfile = outlierfile
        self.allimpars = { '0' :{'imagename':imagename, 'nchan':nchan, 'imsize':imsize, 'cellsize':cellsize, 'phasecenter':phasecenter} }
        self.allgridpars = { '0' : {'ftmachine':ftmachine, 'startmodel':startmodel, 'weighting':weighting } }
        self.alldecpars = { '0' : { 'id':0, 'algo':algo } }
        self.iterpars = { 'niter':niter, 'cycleniter':cycleniter, 'threshold':threshold, 'loopgain':loopgain, 'interactive':interactive }  # Ignoring cyclefactor, minpsffraction, maxpsffraction for now.

        ## List of supported parameters in outlier files.
        ## All other parameters will default to the global values.
        self.outimparlist = ['imagename','nchan','imsize','cellsize','phasecenter']
        self.outgridparlist=['ftmachine','startmodel','weighting']
        self.outdecparlist=['algo','startmodel']


    def getSelPars(self):
        return self.allselpars

    def getImagePars(self):
        return self.allimpars
        
    def getGridPars(self):
        return self.allgridpars

    def getDecPars(self):
        return self.alldecpars

    def getIterPars(self):
        return self.iterpars

    def setSelPars(self,selpars):
        self.allselpars = selpars
    def setImagePars(self,impars):
        self.allimpars = impars

    def checkParameters(self):
        self.casalog.origin('tclean.checkParameters')
        self.casalog.post('Verifying Input Parameters')
        # Init the error-string
        errs = "" 
        errs += self.checkAndFixSelectionPars()
        errs += self.makeImagingParamLists()
        errs += self.checkAndFixImagingPars()
        errs += self.checkAndFixIterationPars()

        ## If there are errors, print a message and exit.
        if len(errs) > 0:
            self.casalog.post('Parameter Errors : \n' + errs,'WARN')
            return False
        return True

    ###### Start : Parameter-checking functions ##################
    def checkAndFixSelectionPars(self):
        errs=""
        # vis, field, spw, etc must all be equal-length lists of strings.
        if not self.allselpars.has_key('vis'):
            errs = errs + 'MS name(s) not specified'
        else:
            if type(self.allselpars['vis'])==str:
                self.allselpars['vis']=[self.allselpars['vis']]
            nvis = len(self.allselpars['vis'])
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
            self.allgridpars[ modelid ] = copy.deepcopy(self.allgridpars[ '0' ])
            self.allgridpars[ modelid ].update(outlierpars[immod]['gridpars'])
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
            
        #print 'AFTER : ', self.listofimagedefinitions

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
        tempgridpar={}
        tempdecpar={}
        for oneline in thelines:
            aline = oneline.replace(' ','').replace('\n','')
            if len(aline)>0 and aline.find('#')!=0:
                parpair = aline.split("=")  
                #print parpair
                if len(parpair) != 2:
                    errs += 'Error in line containing : ' + oneline + '\n'
                if parpair[0] == 'imagename' and tempimpar != {}:
                    returnlist.append({'impars':tempimpar, 'gridpars':tempgridpar, 'decpars':tempdecpar} )
                    tempimpar={}
                    tempgridpar={}
                    tempdecpar={}
                usepar=False
                if parpair[0] in self.outimparlist:
                    tempimpar[ parpair[0] ] = parpair[1]
                    usepar=True
                if parpair[0] in self.outgridparlist:
                    tempgridpar[ parpair[0] ] = parpair[1]
                    usepar=True
                if parpair[0] in self.outdecparlist:
                    tempdecpar[ parpair[0] ] = parpair[1]
                    usepar=True
                if usepar==False:
                    print 'Ignoring unknown parameter pair : ' + oneline

        if len(errs)==0:
            returnlist.append( {'impars':tempimpar, 'gridpars':tempgridpar, 'decpars':tempdecpar} )
        #print returnlist
        return returnlist, errs


    def printParameters(self):
        self.casalog.post('SelPars : ' + str(self.allselpars), 'INFO')
        self.casalog.post('ImagePars : ' + str(self.allimpars), 'INFO')
        self.casalog.post('GridPars : ' + str(self.allgridpars), 'INFO')
        self.casalog.post('DecPars : ' + str(self.alldecpars), 'INFO')
        self.casalog.post('IterPars : ' + str(self.iterpars), 'INFO')

