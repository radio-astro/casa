#################################
## Tool level test script for the refactored imager framework
#################################

#### Specify parameters.
def getparams(testnum=1):
    
    global NN,NF

    if(testnum==5): ## 2 image-fields, each with multiple channels, and 2 nodes for the major cycle
        NN=2
        allselpars=[ {'vis':'twochan.ms', 'field':'3', 'spw':'4', 'usescratch':True} , \
                         {'vis':'twochan.ms', 'field':'3', 'spw':'5', 'usescratch':True} ]

        NF=2
        allimpars=[ {'imagename':'mytest0', 'nchan':5,'imsize':[3,3]} ,\
                    {'imagename':'mytest1', 'nchan':2,'imsize':[1,1] } ]
        allgridpars=[ {'ftmachine':'ft'}, {'ftmachine':'ft','modelname':'startingmodel1'} ]
        alldecpars = [ {'id':0, 'algo':'test'}, {'id':1, 'algo':'test','modelname':'startingmodel1'} ]

    if(testnum==4):  ## 1 image-field, one chan.  2 nodes for the major cycle
        NN=2
        allselpars=[ {'vis':'twochan.ms', 'field':'3', 'spw':'4', 'usescratch':True} , \
                         {'vis':'twochan.ms', 'field':'3', 'spw':'5', 'usescratch':True} ]
        NF=1
        allimpars=[ {'imagename':'mytest0', 'nchan':1,'imsize':[3,3]}] 
        allgridpars=[ {'ftmachine':'ft'} ]
        alldecpars = [ {'id':0, 'algo':'test'} ]
    
    if(testnum==3): ## 2 image fields, each with multiple channels
        NN=1
        allselpars=[ {'vis':'twochan.ms', 'field':'3', 'spw':'4,5', 'usescratch':True} ]
        NF=2
        allimpars=[ {'imagename':'mytest0', 'nchan':5,'imsize':[3,3]} ,\
                        {'imagename':'mytest1', 'nchan':2,'imsize':[1,1] } ]
        allgridpars=[ {'ftmachine':'ft'}, {'ftmachine':'ft','modelname':'startingmodel1'} ]
        alldecpars = [ {'id':0, 'algo':'test'}, {'id':1, 'algo':'test','modelname':'startingmodel1'} ]

    if(testnum==2):  ## 2 image-fields, each with one channel
        NN=1
        allselpars=[ {'vis':'twochan.ms', 'field':'3', 'spw':'4,5', 'usescratch':True} ]
        NF=2
        allimpars=[ {'imagename':'mytest0', 'nchan':1,'imsize':[3,3]} ,\
                        {'imagename':'mytest1', 'nchan':1,'imsize':[1,1]} ]
        allgridpars=[ {'ftmachine':'ft'}, {'ftmachine':'ft'} ]
        alldecpars = [ {'id':0, 'algo':'test'}, {'id':1, 'algo':'test'} ]
        
    if(testnum==1.5):  ## 1 image-field, multiple channels
        NN=1
        allselpars=[ {'vis':'twochan.ms', 'field':'3', 'spw':'4,5', 'usescratch':True} ]
        NF=1
        allimpars=[ {'imagename':'mytest0', 'nchan':5,'imsize':[3,3]} ]
        allgridpars=[ {'ftmachine':'ft'} ]
        alldecpars = [ {'id':0, 'algo':'test'} ]
        
    if(testnum==1):  ## 1 image-field, one chan
        NN=1
        allselpars=[ {'vis':'twochan.ms', 'field':'3', 'spw':'4,5', 'usescratch':True} ]
        NF=1
        allimpars=[ {'imagename':'mytest0', 'nchan':1,'imsize':[3,3]} ]
        allgridpars=[ {'ftmachine':'ft'} ]
        alldecpars = [ {'id':0, 'algo':'test'} ]

    iterpars = {'niter':100, 'cycleniter':100,'threshold':0.001,'loopgain':0.2,'interactive':False}

    return [ allselpars, allimpars, allgridpars, alldecpars, iterpars ]

###############################################################


#######################################
###    Function to run the Imaging.
#######################################
def doClean( params = [{},{},{}] , doplot=True ):

    os.system('rm -rf mytest*')

    selpars = params[0]
    impars = params[1]
    gridpars = params[2]
    decpars = params[3]
    iterpars = params[4]

    imager = PySynthesisImager()

    ### Set up Imagers, Deconvolvers, IterControl, and ParallelSync.
    imager.initializeImagers(selpars,impars,gridpars)
    imager.initializeDeconvolvers(impars,decpars)
    imager.initializeParallelSync(impars)
    imager.initializeIterationControl(iterpars)

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
def doMajor( params = [{},{},{}] , doplot=True ):

    os.system('rm -rf mytest*')

    selpars = params[0]
    impars = params[1]
    gridpars = params[2]
    decpars = params[3]
    iterpars = params[4]

    imager = PySynthesisImager()

    ### Set up Imagers and ParallelSync.
    imager.initializeImagers(selpars,impars,gridpars)
    imager.initializeParallelSync(impars)

    ### Run it.
    imager.runMajorCycle()

    imager.deleteTools()

########################################
########################################
###   Run only the minor cycle....
########################################
def doMinor( params = [{},{},{}] , doplot=True ):

##    os.system('rm -rf mytest*')

    selpars = params[0]
    impars = params[1]
    gridpars = params[2]
    decpars = params[3]
    iterpars = params[4]

    imager = PySynthesisImager()

    ### Set up Deconvolvers and IterControl
    imager.initializeDeconvolvers(impars,decpars)
    imager.initializeIterationControl(iterpars)

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
#############################################
class PySynthesisImager:

    def __init__(self):
        self.SItools = []
        self.SDtools=[]
        self.PStools=[]
        self.IBtool=None
        self.NN=1
        self.NF=1

#############################################

    def initializeImagers(self,allselpars, allimpars, allgridpars):
        for node in range(0,NN):
            toolsi = casac.synthesisimager()
            selpars=allselpars[node]
            toolsi.selectdata(selpars=selpars)
            for immod in range(0,NF):
                impars=copy.deepcopy(allimpars[immod])
                if(NN>1):
                    impars['imagename'] = impars['imagename']+'.n'+str(node)
                toolsi.defineimage(impars=impars)
                toolsi.setupimaging(gridpars=allgridpars[immod])
                toolsi.initmapper()
            self.SItools.append(toolsi)

#############################################

    def initializeDeconvolvers(self,allimpars, alldecpars):
        for immod in range(0,NF):
            self.SDtools.append(casac.synthesisdeconvolver())
            decpars = alldecpars[immod]
            decpars['imagename'] = allimpars[immod]['imagename']
            self.SDtools[immod].setupdeconvolution(decpars=decpars)

#############################################

    def initializeParallelSync(self,allimpars):
        print 'init par sync'
        for immod in range(0,NF):
            self.PStools.append(casac.synthesisparsync())
            syncpars = {'imagename':allimpars[immod]['imagename']}
            partnames = []
            if(NN>1):
                for node in range(0,NN):
                    partnames.append( allimpars[immod]['imagename']+'.n'+str(node)  )
                syncpars['partimagenames'] = partnames
            self.PStools[immod].setupparsync(syncpars=syncpars)

#############################################

    def initializeIterationControl(self,iterpars):
        self.IBtool = casac.synthesisiterbot()
        itbot = self.IBtool.setupiteration(iterpars=iterpars)

#############################################

    def restoreImages(self):
        for immod in range(0,NF):
            self.SDtools[immod].restore()

#############################################

    def getSummary(self):
        summ = self.IBtool.getiterationsummary()
        return summ

#############################################

    def deleteTools(self):
        for immod in range(0,len(self.SDtools)):
            self.SDtools[immod].done()
        for immod in range(0,len(self.PStools)):
            self.PStools[immod].done()
        for node in range(0,len(self.SItools)):
            self.SItools[node].done()
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
        for immod in range(0,NF):
            initrec =  self.SDtools[immod].initminorcycle() 
            self.IBtool.mergeinitrecord( initrec );
            print "Peak res of field ",immod, " : " ,initrec['peakresidual']
        # Check with the iteration controller about convergence.
        stopflag = self.IBtool.cleanComplete()
        print 'Converged : ', stopflag
        return stopflag

#############################################

    def runMajorCycle(self):
        for immod in range(0,NF):
            self.PStools[immod].scattermodel() 
        ### Run major cycle
        for node in range(0,NN):
            self.SItools[node].executemajorcycle(controls={})
        if self.IBtool != None:
            self.IBtool.endmajorcycle()
        ### Gather residuals (if needed) and normalize by weight
        for immod in range(0,NF):
            self.PStools[immod].gatherresidual() 

#############################################

    def runMinorCycle(self):
        # Get iteration control parameters
        iterbotrec = self.IBtool.getminorcyclecontrols()
        # Run minor cycle
        for immod in range(0,NF):
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

    return summ;


#######################################################
#######################################################
#######################################################
