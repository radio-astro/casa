#################################
## Tool level test script for the refactored imager framework
#################################
from refimagerhelper import PySynthesisImager, PyParallelContSynthesisImager,PyParallelCubeSynthesisImager, PyParallelDeconvolver, PyParallelImagerHelper,ImagerParameters

import commands
#### Specify parameters.
def getparams(testnum=1,parallelmajor=False,parallelminor=False,parallelcube=False):

     # Iteration parameters - common to all tests below
     niter=200
     cycleniter=40
     threshold=0.001
     loopgain=0.1

     # Interaction ON or OFF
     interactive=False

     if(testnum==6):  ## 1 image-field, mfs --- WB AWP
          casalog.post("==================================");
          casalog.post("Test 6 image-field, mfs --- WB AWP");
          casalog.post("==================================");
          paramList = ImagerParameters(msname='DataTest/point_twospws.ms',field='0',spw='*',\
                                       usescratch=True,readonly=True,\
                                       imagename='mytest0', nchan=1,freqstart='1.0GHz', freqstep='4.0GHz',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='awprojectft', startmodel='', weighting='natural',\
                                       aterm=True, psterm=True, mterm=False, wbawp = True, cfcache = "deleteme.cf",\
                                       dopointing = False, dopbcorr = True, conjbeams = True, computepastep =360.0, rotatepastep =5.0,\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)

     if(testnum==5):  ## 1 image-field, mfs, multiple input MSs --- Real Imaging.
          casalog.post("==================================");
          casalog.post("Test 5 image-field, mfs, multiple input MSs --- Real Imaging.");
          casalog.post("==================================");
          paramList = ImagerParameters(msname=['DataTest/point_onespw0.ms','DataTest/point_onespw1.ms'],\
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


     if(testnum==4):  ## 2 image-fields, one cube, one mfs --- Real Imaging.
          casalog.post("==================================");
          casalog.post("Test 4 image-fields, one cube, one mfs --- Real Imaging.");
          casalog.post("==================================");
          
          write_file('out4.txt', 'imagename=mytest1\nnchan=1\nimsize=[80,80]\ncellsize=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nfreqstep=4.0GHz')
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',\
                                       field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       outlierfile='out4.txt',\
                                       imagename='mytest0', nchan=2, freqstart='1.0GHz', freqstep='1.0GHz',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='test',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)


     if(testnum==3):  ## 2 image-fields, mfs --- Real Imaging.
          casalog.post("==================================");
          casalog.post("Test 3 image-fields, mfs --- Real Imaging.");
          casalog.post("==================================");
          
          write_file('out3.txt', 'imagename=mytest1\nnchan=1\nimsize=[80,80]\ncellsize=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543')
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',\
                                       field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       outlierfile='out3.txt',\
                                       imagename='mytest0', nchan=1, freqstart='1.0GHz', freqstep='4.0GHz',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)
     
     
     if(testnum==2):  ## 1 image-field, cube --- Real Imaging.
          casalog.post("==================================");
          casalog.post("Test 2 image-field, cube --- Real Imaging.");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/point_twospws.ms', field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       imagename='mytest0', nchan=10,freqstart='1.0GHz', freqstep='40MHz',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)
     

     if(testnum==1):  ## 1 image-field, mfs --- Real Imaging.
          casalog.post("==================================");
          casalog.post("Test 1 image-field, mfs --- Real Imaging.");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/point_twospws.ms',field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       imagename='mytest0', nchan=1,freqstart='1.0GHz', freqstep='4.0GHz',\
                                       imsize=[110,110],\
                                       cellsize=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)



     ### Make a cluster def file if a parallel test is to run.
     if parallelmajor==True or parallelminor==True or parallelcube==True:
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
        return [None, "", False, False,False]

     paramList.printParameters()

     return [ paramList , clusterdef, parallelmajor, parallelminor, parallelcube ]

#####################################################


#######################################
###    Function to run the Imaging.
#######################################
def doClean( params = [None,"",False,False,False] , doplot=True ):

    os.system('rm -rf mytest*')

    pmajor = params[2]
    pcube = params[4]

    if pmajor==False and pcube==False:
         imager = PySynthesisImager(params[0])
    elif pmajor==True:
         imager = PyParallelContSynthesisImager(params[0],params[1])
    elif pcube==True:
         imager = PyParallelCubeSynthesisImager(params[0],params[1])
    else:
         print 'Invalid parallel combination in doClean.'
         return

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
         imager.getSummary();

    imager.deleteTools()
########################################


########################################
########################################
#  Run only Major Cycle
########################################
def doMajor( params = [None,"",False,False,False] , doplot=True , tomake='both'):

    os.system('rm -rf mytest*')

    pmajor=params[2]
    pcube = params[4]

    if pmajor==False and pcube==False:
         imager = PySynthesisImager(params[0])
    elif pmajor==True:
         imager = PyParallelContSynthesisImager(params[0],params[1])
    elif pcube==True:
         imager = PyParallelCubeSynthesisImager(params[0],params[1])
    else:
         print 'Invalid parallel combination in doClean.'
         return


    ### Set up Imagers and ParallelSync.
    imager.initializeImagers()
    imager.initializeParallelSync()

    ### Run it.
    if tomake=='both' or tomake=='psf':
         imager.makePSF()
    if tomake=='both' or tomake=='residual':
         imager.runMajorCycle()

    imager.deleteTools()

########################################
########################################
###   Run only the minor cycle....
########################################
def doMinor( params = [None,"",False,False,False] , doplot=True ):

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
         imager.getSummary();

    imager.deleteTools()
########################################

########################################
#
#   Predict Model only (for use by setjy too)
#
########################################
########################################
#
#   Restore model image, Inputs : model, residual, psf.
#
########################################
def doRestore( params = [None,"",False,False,False] ):

    pminor=params[3]
    
    imager = PySynthesisImager(params[0]) 

    imager.initializeDeconvolvers()

    imager.restoreImages()

    imager.deleteTools()


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

     ## Make parameter lists.
     #paramList = ImagerParameters(msname=['x1.ms','x2.ms'], field='0',spw=['0','2'], usescratch=True)
     ## Sync input lists to the same size.
     #paramList.checkParameters()

     params = getparams( testnum=3 ,parallelmajor=True )
     paramList = params[0]
     clusterdeffile = params[1]

     selpars = paramList.getSelPars()
     impars = paramList.getImagePars()

     synu = casac.synthesisutils()
     print synu.contdatapartition( selpars , 2)

     print synu.cubedatapartition( selpars, 2)
     
     print synu.cubeimagepartition( impars, 2)

     synu.done()

#     ppar = PyParallelImagerHelper(clusterdef=clusterdeffile)
#
#     print 'Selpars : ', selpars
#     newselpars = ppar.partitionCubeDataSelection( selpars )
#
#     print 'Impars : ', impars
#     newimpars = ppar.partitionCubeDeconvolution( impars )
#
#     ppar.takedownCluster()
#

     # The output dictionary should be indexed as follows ( for 2 nodes, and 2 MSs )
     #{ '0' : { 'ms0' : { 'msname':xxx1, 'spw':yyy1 } ,
     #            'ms1' : { 'msname':xxx2, 'spw':yyy1 } 
     #         }
     #  '1' : { 'ms0' : { 'msname':xxx1, 'spw':yyy2 } ,
     #            'ms1' : { 'msname':xxx2, 'spw':yyy2 } 
     #        }
     # }
     #


def checkPars():

     params = getparams( testnum=1 )
     paramList = params[0]

     selpars = paramList.getSelPars()
     impars = paramList.getImagePars()

     synu = casac.synthesisutils()

#     print selpars['ms0']
#     fixrec = synu.checkselectionparams( selpars['ms0'] )
#     print fixrec
#     print synu.checkselectionparams( fixrec )


     print impars['0']
     fixrec = synu.checkimageparams( impars['0'] )
     print "---------------------------------------------"
     print fixrec
     print "---------------------------------------------"
     print synu.checkimageparams( fixrec )

     synu.done()


def testmakeimage():
     params = getparams( testnum=1 )
     paramList = params[0]
     impars = (paramList.getImagePars())['0']

     os.system('rm -rf ' + impars['imagename'])

     synu = casac.synthesisutils()
     synu.makeimage( impars , 'DataTest/twopoints_twochan.ms')
     synu.done()

