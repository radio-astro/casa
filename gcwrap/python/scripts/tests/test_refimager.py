#################################
## Tool level test script for the refactored imager framework
#################################

#### Specify parameters.
def getparams(testnum=1,parallelrun=False):

     if(testnum==3): ## 2 image fields, each with multiple channels

        allselpars={'vis':'twochan.ms', 'field':'3', 'spw':'4,5', 'usescratch':True} 
        #NF=2
        allimpars={ '0' : {'imagename':'mytest0', 'nchan':5,'imsize':[3,3]} ,\
                           '1' : {'imagename':'mytest1', 'nchan':2,'imsize':[1,1] } }
        allgridpars={ '0' : {'ftmachine':'ft'},\
                             '1' : {'ftmachine':'ft','modelname':'startingmodel1'} }
        alldecpars ={ '0': {'id':0, 'algo':'test'},\
                             '1':{'id':1, 'algo':'test','modelname':'startingmodel1'} }

    if(testnum==2):  ## 2 image-fields, each with one channel

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
        alldecpars ={' 0' : {'id':0, 'algo':'test'} }
        
    if(testnum==1):  ## 1 image-field, one chan

        allselpars={'vis':'twochan.ms', 'field':'3', 'spw':'4,5', 'usescratch':True}
        #NF=1
        allimpars= { '0' : {'imagename':'mytest0', 'nchan':1,'imsize':[3,3]} }
        allgridpars={ '0' : {'ftmachine':'ft'} }
        alldecpars ={ '0': {'id':0, 'algo':'test'} }

    iterpars = {'niter':100, 'cycleniter':100,'threshold':0.001,'loopgain':0.2,'interactive':False}

    if parallelrun==True:
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

    return [ allselpars, allimpars, allgridpars, alldecpars, iterpars, clusterdef ]

#####################################################


#######################################
###    Function to run the Imaging.
#######################################
def doClean( params = [{},{},{},{},{},""] , doplot=True ):

    os.system('rm -rf mytest*')

    imager = PySynthesisImager(params[0],params[1],params[2],params[3],params[4],params[5])

    ### Set up Imagers, Deconvolvers, IterControl, and ParallelSync.
    imager.initializeImagers()
    imager.initializeDeconvolvers()
    imager.initializeParallelSync()
    imager.initializeIterationControl()

    ### Run it.
    imager.runMajorCycle()
    while ( not imager.hasConverged() ):
        imager.runMinorCycle()
        imager.runMajorCycle()

    imager.restoreImages()

    if( doplot == True ):
        summplot1(imager.getSummary());

    imager.deleteTools()
########################################


########################################
########################################
#  Run only Major Cycle
########################################
def doMajor( params = [{},{},{},{},{},""] , doplot=True ):

    os.system('rm -rf mytest*')

    imager = PySynthesisImager(params[0],params[1],params[2],params[3],params[4],params[5])

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
def doMinor( params = [{},{},{},{},{},""] , doplot=True ):

##    os.system('rm -rf mytest*')

    imager = PySynthesisImager(params[0],params[1],params[2],params[3],params[4],params[5])

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


##########################################
###  PySynthesisImager class
##########################################
import copy
from simple_cluster import simple_cluster
#############################################
class PySynthesisImager:

    def __init__(self,selpars={},allimpars={}, \
                     allgridpars={},alldecpars={}, \
                     iterpars={},clusterdef='',):
        ################ Tools
        self.SItools = []
        self.SDtools=[]
        self.PStools=[]
        self.IBtool=None
        ############### Cluster Info
        self.clusterdef=clusterdef
        self.CL=None
        self.sc=None
        ############### Number of nodes to parallelize on
        if len(self.clusterdef) != 0:
            self.NN = self.setupCluster()
            self.allselpars = self.partitionDataSelection(selpars)
        else:
            self.NN = 1
            self.allselpars = [selpars]
        ############### Number of image fields
        self.NF = len(allimpars.keys())
        ############### All input parameters
        self.allimpars = allimpars
        self.allgridpars = allgridpars
        self.alldecpars = alldecpars
        self.iterpars = iterpars

#############################################

    def partitionDataSelection(self,selpars={}):
        allselpars = []
        for node in range(0,self.NN):
            allselpars.append( copy.deepcopy(selpars) )
            ### Temp modification. Put actual split-chan-range info here.
            if not allselpars[node].has_key('spw'):
                allselpars[node]['spw']=str(node)
            else:
                allselpars[node]['spw'] = allselpars[node]['spw'] + ':' + str(node)
        print 'Selection : ', allselpars
        return allselpars


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
        if self.clusterdef != "":
            self.checkJobs()
            print 'Stopping cluster'
            self.sc.stop_cluster()

#############################################
    # This is a blocking call that will wait until jobs are done.
    def checkJobs(self):
        numcpu = len(self.CL.get_engines())
        out = range(numcpu)
        for k in range(numcpu):
            out[k] = self.CL.odo('casalog.post("node '+str(k)+' has completed its job")', k)
        
        over=False
        while(not over):
            overone=True
            time.sleep(1)
            for k in range(len(self.CL.get_engines())):
                overone =  self.CL.check_job(out[k],False) and overone
            over = overone

#############################################
#############################################
    def initializeImagers(self):

        for node in range(0,self.NN):
            nimpars = copy.deepcopy(self.allimpars)
            if self.NN>1:
                for ff  in range(0,self.NF):
                    nimpars[str(ff)]['imagename'] = nimpars[str(ff)]['imagename']+'.n'+str(node)

            if self.clusterdef == "":
                toolsi = casac.synthesisimager()
                toolsi.initializemajorcycle(self.allselpars[node], nimpars, self.allgridpars)
                self.SItools.append(toolsi)
            else:
            ## Later, move out common commands into self.CL.pgc
                self.CL.odo("toolsi = casac.synthesisimager()", node)
                self.CL.odo("toolsi.initializemajorcycle("+str(self.allselpars[node])+","+str(nimpars)+","+str(self.allgridpars)+")", node)
                self.checkJobs()


#############################################

    def initializeDeconvolvers(self):
        for immod in range(0,self.NF):
            self.SDtools.append(casac.synthesisdeconvolver())
            decpars = self.alldecpars[str(immod)]
            decpars['imagename'] = self.allimpars[str(immod)]['imagename']
            self.SDtools[immod].setupdeconvolution(decpars=decpars)

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

    def deleteTools(self):
        # Delete Imagers
        if self.clusterdef=="":
            for node in range(0,len(self.SItools)):
                self.SItools[node].done()
        else:
            self.CL.pgc("toolsi.done()")
            self.takedownCluster()

        # Delete Deconvolvers
        for immod in range(0,len(self.SDtools)):
            self.SDtools[immod].done()
        # Delete ParSyncs
        for immod in range(0,len(self.PStools)):
            self.PStools[immod].done()
        # Delete IterBot
        if self.IBtool != None:
            self.IBtool.done()
        # Reset globals/members
        self.NN=1
        self.NF=1
        self.SItools=[]
        self.SDtools=[]
        self.PStools=[]
        self.IBtool=None
    
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
        ### Run major cycle

        if self.clusterdef=="":
            for node in range(0,self.NN):
                self.SItools[node].executemajorcycle(controls={})
        else:
            for node in range(0,self.NN):
                self.CL.odo("toolsi.executemajorcycle(controls={})",node)
            self.checkJobs() # this call blocks until all are done.

        if self.IBtool != None:
            self.IBtool.endmajorcycle()
        ### Gather residuals (if needed) and normalize by weight
        for immod in range(0,self.NF):
            self.PStools[immod].gatherresidual() 

#############################################

#############################################

    def runMinorCycle(self):
        # Get iteration control parameters
        iterbotrec = self.IBtool.getminorcyclecontrols()
        # Run minor cycle
        for immod in range(0,self.NF):
            exrec = self.SDtools[immod].executeminorcycle( iterbotrecord = iterbotrec )
            self.IBtool.mergeexecrecord( exrec )


#############################################
#############################################


#######################################################
#######################################################
##  Function to plot the summary output 
#######################################################
#######################################################

def summplot1( summ={} ):
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

    pl.figure(1)
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

    pl.savefig('summaryplot.png')
    pl.ion()

    return summ;


#######################################################
#######################################################
#######################################################
