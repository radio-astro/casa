#################################
## Tool level test script for the refactored imager framework
#################################
import commands
#### Specify parameters.
def getparams(testnum=1,parallelmajor=False,parallelminor=False):

     if(testnum==4): ## 2 image fields, each with multiple channels

         allselpars={'vis':'twochan.ms', 'field':'3', 'spw':'4,5', 'usescratch':True} 
        #NF=2
         allimpars={ '0' : {'imagename':'mytest0', 'nchan':5,'imsize':[3,3]} ,\
                         '1' : {'imagename':'mytest1', 'nchan':2,'imsize':[1,1] } }
         allgridpars={ '0' : {'ftmachine':'ft'},\
                           '1' : {'ftmachine':'ft','modelname':'startingmodel1'} }
         alldecpars ={ '0': {'id':0, 'algo':'test'},\
                          '1':{'id':1, 'algo':'test','modelname':'startingmodel1'} }

     if(testnum==3):  ## 2 image-fields, each with one channel

         allselpars={'vis':'twochan.ms', 'field':'3', 'spw':'4,5', 'usescratch':True} 
        #NF=2
         allimpars={ '0' : {'imagename':'mytest0', 'nchan':1,'imsize':[3,3]} ,\
                         '1' : {'imagename':'mytest1', 'nchan':1,'imsize':[1,1]} }
         allgridpars={ '0': {'ftmachine':'ft'},\
                           '1':{'ftmachine':'ft'} }
         alldecpars = { '0': {'id':0, 'algo':'test'},\
                            '1': {'id':1, 'algo':'test'} }
        
     if(testnum==2):  ## 1 image-field, multiple channels

         allselpars={'vis':'twochan.ms', 'field':'3', 'spw':'4,5', 'usescratch':True}
        #NF=1
         allimpars={ '0' : {'imagename':'mytest0', 'nchan':5,'imsize':[3,3]} }
         allgridpars={ '0': {'ftmachine':'ft'} }
         alldecpars ={ '0' : {'id':0, 'algo':'test'} }
         
     if(testnum==1):  ## 1 image-field, one chan

         allselpars={'vis':'twochan.ms', 'field':'3', 'spw':'4,5', 'usescratch':True}
        #NF=1
         allimpars= { '0' : {'imagename':'mytest0', 'nchan':1,'imsize':[3,3]} }
         allgridpars={ '0' : {'ftmachine':'ft'} }
         alldecpars ={ '0': {'id':0, 'algo':'test'} }
         
     iterpars = {'niter':100, 'cycleniter':100,'threshold':0.001,'loopgain':0.2,'interactive':False}

     if parallelmajor==True or parallelminor==True:
        clusterdef = 'cfgfile.txt'
        defstr = 'vega, 2,'+os.getcwd() +'/aatest'
        # remove file first
        if os.path.exists(clusterdef):
            os.system('rm -f '+ clusterdef)
        
        # save to a file    
        with open(clusterdef, 'w') as f:
            f.write(defstr)
        f.close()
     else:
        clusterdef=""

     return [ allselpars, allimpars, allgridpars, alldecpars, iterpars, clusterdef, parallelmajor, parallelminor ]

#####################################################


#######################################
###    Function to run the Imaging.
#######################################
def doClean( params = [{},{},{},{},{},"",False,False] , doplot=True ):

    os.system('rm -rf mytest*')

    pmajor=params[6]
    pminor=params[7]

    if pmajor==False:
         imager = PySynthesisImager(params[0],params[1],params[2],params[3],params[4])
    else:
         imager = PyParallelContSynthesisImager(params[0],params[1],params[2],params[3],params[4],params[5])

    ### Set up Imagers, Deconvolvers, IterControl, and ParallelSync.
    imager.initializeImagers()
    imager.initializeDeconvolvers()
    imager.initializeParallelSync()
    imager.initializeIterationControl()

    ### Run it.
    imager.runMajorMinorLoops()

    imager.restoreImages()

    if( doplot == True ):
        summplot1(imager.getSummary());

    imager.deleteTools()
########################################


########################################
########################################
#  Run only Major Cycle
########################################
def doMajor( params = [{},{},{},{},{},"",False,False] , doplot=True ):

    os.system('rm -rf mytest*')

    pmajor=params[6]
    pminor=params[7]

    if pmajor==False:
         imager = PySynthesisImager(params[0],params[1],params[2],params[3],params[4])
    else:
         imager = PyParallelContSynthesisImager(params[0],params[1],params[2],params[3],params[4],params[5])

    ### Set up Imagers and ParallelSync.
    imager.initializeImagers()
    imager.initializeParallelSync()

    ### Run it.
    imager.runMajorCycle()

    imager.deleteTools()

########################################
########################################
###   Run only the minor cycle....
########################################
def doMinor( params = [{},{},{},{},{},"",False,False] , doplot=True ):

##    os.system('rm -rf mytest*')

    pminor=params[7]

    if pminor==False:
         imager = PySynthesisImager(params[0],params[1],params[2],params[3],params[4]) 
    else:
         imager = PyParallelDeconvolver(params[0],params[1],params[2],params[3],params[4],params[5])

    ### Set up Deconvolvers and IterControl
    imager.initializeDeconvolvers()
    imager.initializeIterationControl()

    # Run it
    while ( not imager.hasConverged() ):
        imager.runMinorCycle()

    imager.restoreImages()

    if( doplot == True ):
        summplot1(imager.getSummary());

    imager.deleteTools()
########################################



#######################################
def doParCubeClean( params = [{},{},{},{},{},"",False,False] , doplot=True ):

    os.system('rm -rf mytest*')

    pmajor=params[6]
    pminor=params[7]
    clusterdef = params[5]

    selpars = { '0' : params[0] }
    allimpars = params[1]

    PH = PyParallelImagerHelper( clusterdef )
    NN = PH.NN
    allselpars = PH.partitionDataSelection(selpars)
    allimpars = PH.partitionCubeDeconvolution(allimpars)

    cmd = (commands.getoutput('echo $CASAPATH')).split()[0] + '/gcwrap/python/scripts/tests/test_refimager.py'
    cmd = "execfile('"+cmd+"')"
    #cmd = "execfile('/home/vega/rurvashi/TestCASA/ImagerRefactor/Runs/test_refimager.py')"
    PH.runcmdcheck( cmd )

    joblist=[]
    for node in range(0,NN):
         joblist.append( PH.runcmd("imager = PySynthesisImager("+str(allselpars[str(node)])+","+str(allimpars[str(node)])+","+str(params[2])+","+str(params[3])+","+str(params[4])+")", node) )
    PH.checkJobs( joblist )

    ### Set up Imagers, Deconvolvers, IterControl, and ParallelSync.
    cmd = "imager.initializeImagers();imager.initializeDeconvolvers();imager.initializeParallelSync();imager.initializeIterationControl()"
    PH.runcmdcheck( cmd )

    ### Run it.
    cmd = "imager.runMajorMinorLoops()"
    PH.runcmdcheck( cmd )

    cmd = "imager.restoreImages()"
    PH.runcmdcheck( cmd )

    if( doplot == True ):
         cmd = "summvar = imager.getSummary()"
         PH.runcmdcheck( cmd )
         for node in range(0,NN):
              summvar = PH.pullval( "summvar", node)
              summplot1(summvar[node],node+1);

    cmd = "imager.deleteTools()"
    PH.runcmdcheck( cmd )

########################################



##########################################
###  PySynthesisImager class
##########################################
import copy
from simple_cluster import simple_cluster
#############################################
class PySynthesisImager:

    def __init__(self,selpars={},allimpars={}, \
                     allgridpars={},alldecpars={}, \
                     iterpars={}):
        ################ Tools
        self.initDefaults()

        # Check all input parameters, after partitioning setup.
        self.allselpars = { '0' : selpars }
        self.alldecpars = alldecpars
        self.allimpars = allimpars
        self.allgridpars = allgridpars
        self.iterpars = iterpars

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
    def runMajorCycle(self):
        for immod in range(0,self.NF):
            self.PStools[immod].scattermodel() 

        self.runMajorCycleCore()

        if self.IBtool != None:
            self.IBtool.endmajorcycle()
        ### Gather residuals (if needed) and normalize by weight
        for immod in range(0,self.NF):
            self.PStools[immod].gatherresidual() 

#############################################

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

#############################################
#############################################
## Parallelize only major cycle.
#############################################
class PyParallelContSynthesisImager(PySynthesisImager):

    def __init__(self,selpars={},allimpars={}, \
                     allgridpars={},alldecpars={}, \
                     iterpars={},clusterdef=''):

         PySynthesisImager.__init__(self,selpars,allimpars,allgridpars,alldecpars,iterpars)

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
    def runMajorCycleCore(self):
        ### Run major cycle
        joblist=[]
        for node in range(0,self.PH.NN):
             joblist.append( self.PH.runcmd("toolsi.executemajorcycle(controls={})",node) )
        self.PH.checkJobs( joblist ) # this call blocks until all are done.

#############################################



class PyParallelDeconvolver(PySynthesisImager):

    def __init__(self,selpars={},allimpars={}, \
                     allgridpars={},alldecpars={}, \
                     iterpars={},clusterdef=''):


        PySynthesisImager.__init__(self,selpars,allimpars,allgridpars,alldecpars,iterpars)

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
    def partitionDataSelection(self,oneselpars={}):
        allselpars = {}
        for node in range(0,self.NN):
            allselpars[str(node)]  = copy.deepcopy(oneselpars['0']) 
            ### Temp modification. Put actual split-chan-range info here.
            if not allselpars[str(node)].has_key('spw'):
                allselpars[str(node)]['spw']=str(node)
            else:
                allselpars[str(node)]['spw'] = allselpars[str(node)]['spw'] + ':' + str(node)
        print 'Selection : ', allselpars
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


#######################################################
#######################################################
##  Function to plot the summary output 
#######################################################
#######################################################

def summplot1( summ={} ,fignum=1 ):
    if not ( summ.has_key('summaryminor') and summ.has_key('summarymajor') and summ.has_key('threshold') and summ['summaryminor'].shape[0]==6 ):
        print 'Cannot make summary plot. Please check contents of the output dictionary from tclean.'
        return summ

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
#######################################################

