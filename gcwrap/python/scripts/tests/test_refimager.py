#################################
## Tool level test script for the refactored imager framework
#################################
from refimagerhelper import PySynthesisImager, PyParallelContSynthesisImager, PyParallelDeconvolver, PyParallelImagerHelper,ImagerParameters

import commands
#### Specify parameters.
def getparams(testnum=1,parallelmajor=False,parallelminor=False):

     # Iteration parameters - common to all tests below
     niter=100
     cycleniter=100
     threshold=0.001
     loopgain=0.2

     # Interaction ON or OFF
     interactive=False

     if(testnum==7):  ## 2 image-fields, mfs --- Real Imaging.
          
          write_file('out7.txt', 'imagename=mytest1\nnchan=1\nimsize=[80,80]\ncellsize=[8.0arcsec,8.0arcsec]\nphasecenter=19:58:40.895 +40.55.58.543')
          paramList = ImagerParameters(casalog=casalog,\
                                       msname='DataTest/twopoints_twochan.ms',\
                                       field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       outlierfile='out7.txt',\
                                       imagename='mytest0', nchan=1, freqstart='1.0GHz', freqstep='4.0GHz',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)
     

     if(testnum==6):  ## 1 image-field, mfs, multiple input MSs --- Real Imaging.
          
          paramList = ImagerParameters(casalog=casalog,\
                                       msname=['DataTest/point_onespw0.ms','DataTest/point_onespw1.ms'],\
                                       field='0',spw=['0','0'],\
                                       usescratch=True,readonly=True,\
                                       imagename='mytest0', nchan=1,  freqstart='1.0GHz', freqstep='4.0GHz',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)


     if(testnum==5):  ## 1 image-field, mfs --- Real Imaging.
          
          paramList = ImagerParameters(casalog=casalog,\
                                       msname='DataTest/point_twospws.ms', field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       imagename='mytest0', nchan=1,freqstart='1.0GHz', freqstep='4.0GHz',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)
     

     if(testnum==4): ## 2 image fields, each with multiple channels

          write_file('out4.txt', 'imagename=mytest1\nnchan=2\nimsize=[1,1]\nstartmodel=startingmodel1')
          paramList = ImagerParameters(casalog=casalog,\
                                       msname='point_twospws.ms', field='0',spw='0,1', usescratch=True,\
                                       outlierfile='out4.txt',\
                                       imagename='mytest0', nchan=5, imsize=[3,3],\
                                       ftmachine='ft', startmodel='',\
                                       algo='test',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)

     if(testnum==3):  ## 2 image-fields, each with one channel

          write_file('out3.txt', 'imagename=mytest1\nnchan=1\nimsize=[1,1]')
          paramList = ImagerParameters(casalog=casalog,\
                                       msname='point_twospws.ms', field='0',spw='0,1', usescratch=True,\
                                       outlierfile='out3.txt',\
                                       imagename='mytest0', nchan=1, imsize=[3,3],\
                                       ftmachine='ft', startmodel='',\
                                       algo='test',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)
        
     if(testnum==2):  ## 1 image-field, multiple channels

          paramList = ImagerParameters(casalog=casalog,\
                                       msname='point_twospws.ms', field='0',spw='0,1', usescratch=True,\
                                       imagename='mytest0', nchan=5, imsize=[3,3],\
                                       ftmachine='ft', startmodel='',\
                                       algo='test',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)
              
     if(testnum==1):  ## 1 image-field, one chan
          
          paramList = ImagerParameters(casalog=casalog,\
                                       msname='point_twospws.ms', field='0',spw='0,1', usescratch=True,\
                                       imagename='mytest0', nchan=1, imsize=[3,3],\
                                       ftmachine='ft', startmodel='',\
                                       algo='test',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)
     

     ### Make a cluster def file if a parallel test is to run.
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

     ### Check input parameters, and parse outlier files.
     if paramList.checkParameters() == False:
        return [None, "", False, False]

     paramList.printParameters()

     return [ paramList , clusterdef, parallelmajor, parallelminor ]

#####################################################


#######################################
###    Function to run the Imaging.
#######################################
def doClean( params = [None,"",False,False] , doplot=True ):

    os.system('rm -rf mytest*')

    pmajor=params[2]

    if pmajor==False:
         imager = PySynthesisImager(params[0])
    else:
         imager = PyParallelContSynthesisImager(params[0],params[1])

    ### Set up Imagers, Deconvolvers, IterControl, and ParallelSync.
    imager.initializeImagers()
    imager.initializeDeconvolvers()
    imager.initializeParallelSync()
    imager.initializeIterationControl()

    ### Run it.
    imager.makePSF()
    imager.runMajorMinorLoops()

    imager.restoreImages()

    if( doplot == True ):
        imager.plotReport(imager.getSummary());

    imager.deleteTools()
########################################


########################################
########################################
#  Run only Major Cycle
########################################
def doMajor( params = [None,"",False,False] , doplot=True ):

    os.system('rm -rf mytest*')

    pmajor=params[2]

    if pmajor==False:
         imager = PySynthesisImager(params[0])
    else:
         imager = PyParallelContSynthesisImager(params[0],params[1])

    ### Set up Imagers and ParallelSync.
    imager.initializeImagers()
    imager.initializeParallelSync()

    ### Run it.
    imager.makePSF()
    imager.runMajorCycle()

    imager.deleteTools()

########################################
########################################
###   Run only the minor cycle....
########################################
def doMinor( params = [None,"",False,False] , doplot=True ):

##    os.system('rm -rf mytest*')

    pminor=params[3]

    if pminor==False:
         imager = PySynthesisImager(params[0]) 
    else:
         imager = PyParallelDeconvolver(params[0],params[1])

    ### Set up Deconvolvers and IterControl
    imager.initializeDeconvolvers()
    imager.initializeIterationControl()

    # Run it
    while ( not imager.hasConverged() ):
        imager.runMinorCycle()

    imager.restoreImages()

    if( doplot == True ):
        imager.plotReport(imager.getSummary());

    imager.deleteTools()
########################################



#######################################
def doParCubeClean( params = [None,"",False,False] , doplot=True ):

    #### THIS MAY NOT WORK RIGHT NOW. 

    os.system('rm -rf mytest*')

    pmajor=params[2]
    pminor=params[3]
    clusterdef = params[1]

    allselpars = params[0].getSelPars()
    allimpars = params[0].getImagePars()

    PH = PyParallelImagerHelper( clusterdef )
    NN = PH.NN
    allselpars = PH.partitionDataSelection(allselpars)
    allimpars = PH.partitionCubeDeconvolution(allimpars)

    cmd = (commands.getoutput('echo $CASAPATH')).split()[0] + '/gcwrap/python/scripts/tests/test_refimager.py'
    cmd = "execfile('"+cmd+"')"
    #cmd = "execfile('/home/vega/rurvashi/TestCASA/ImagerRefactor/Runs/test_refimager.py')"
    PH.runcmdcheck( cmd )

    joblist=[]
    for node in range(0,NN):
         params[0].setSelPars(allselpars[str(node)])
         params[0].setImagePars(allimpars[str(node)])

         # Push the param object in.
         PH.CL.push( paramList=params[0] )

         joblist.append( PH.runcmd("imager = PySynthesisImager(params=paramList)", node) )
####         joblist.append( PH.runcmd("imager = PySynthesisImager("+str(allselpars[str(node)])+","+str(allimpars[str(node)])+","+str(params[2])+","+str(params[3])+","+str(params[4])+")", node) )
    PH.checkJobs( joblist )

    ### Set up Imagers, Deconvolvers, IterControl, and ParallelSync.
    cmd = "imager.initializeImagers();imager.initializeDeconvolvers();imager.initializeParallelSync();imager.initializeIterationControl()"
    PH.runcmdcheck( cmd )

    ### Run it.
    cmd = "imager.makePSF()"
    PH.runcmdcheck( cmd )

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

     PStool = casac.synthesisparsync()
     syncpars = {'imagename':params.getImagePars()['0']['imagename']}
     PStool.setupparsync( syncpars )

     PStool.scattermodel()
     SItool.makepsf()
     SItool.executemajorcycle()
#     istore = SItool.getimstore(0)
#     PStool.setimstore(istore)

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
          PStool.append(casac.synthesisparsync())
          syncpars = {'imagename':allimagepars[ (allimagepars.keys())[fld] ]['imagename']}
          PStool.setupparsync( syncpars )

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

     # Make parameter lists.
#     paramList = ImagerParameters(casalog=casalog, msname=['x1.ms','x2.ms'], field='0',spw=['0','2'], usescratch=True)

     params = getparams( testnum=5 ,parallelmajor=True )

     paramList = params[0]

     # Sync input lists to the same size.
#     paramList.checkParameters()

     # The '0' is for single node. This is done in PySynthesisImager __init__
     selpars = { '0' : paramList.getSelPars() }

     print 'Selpars : ', selpars
     
     # For parallel runs, make this from a config file.
     ppar = PyParallelImagerHelper(clusterdef=params[1])
     
     # This is the function that does the data partitioning.
     newselpars = ppar.partitionDataSelection( selpars )

     print 'NewSelPars : ', newselpars

     # The output dictionary should be indexed as follows ( for 2 nodes, and 2 MSs )
     #{ '0' : { 'ms0' : { 'msname':xxx1, 'spw':yyy1 } ,
     #            'ms1' : { 'msname':xxx2, 'spw':yyy1 } 
     #         }
     #  '1' : { 'ms0' : { 'msname':xxx1, 'spw':yyy2 } ,
     #            'ms1' : { 'msname':xxx2, 'spw':yyy2 } 
     #        }
     # }
     #
