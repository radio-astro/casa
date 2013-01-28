################################################
# Refactored Clean task
#
# v1.0: 2012.10.05, U.R.V.
#
################################################

from taskinit import *

import os
import shutil
import numpy
from taskinit import *
import copy

def tclean(vis='', field='', spw='',
           imagename='',nchan=1,imsize=[1,1],
           outlierfile='',
           startmodel='',
           niter=0, threshold=0.0, loopgain=0.1,
           maxcycleniter=-1, cyclefactor=1,
           minpsffraction=0.05, maxpsffraction=0.8,
           usescratch=True, interact=False, clusterdef=''):

    #casalog.post('This is an empty task. It is meant only to build/test/maintain the interface for the refactored imaging code. When ready, it will be offered to users for testing.','WARN')

    # Put all parameters into dictionaries
    params={}
    params['dataselection']={'vis':vis, 'field':field, 'spw':spw, 'usescratch':usescratch}
    params['imagedefinition']={'imagename':imagename, 'nchan':nchan,'imsize':imsize, 'outlierfile':outlierfile}
    params['imaging']={'startmodel':startmodel}
    params['deconvolution']={}
    params['iteration']={'niter':niter, 'threshold':threshold, 'loopgain':loopgain, 'cycleniter':maxcycleniter, 'cyclefactor':cyclefactor , 'minpsffraction':minpsffraction, 'maxpsffraction':maxpsffraction, 'interactive':interact}
    params['other']={'clusterdef':clusterdef}

    # Instantiate the Imager class
    if len(clusterdef)==0:
        imager = PySynthesisImager(casalog,params)
    else:
        imager = ParallelPySynthesisImager(casalog,params)

    # Check input parameters (types, etc..)
    if not imager.checkParameters():
        return False

    # All setup steps
    imager.initialize()

    # Run major/minor cycle loops
    imager.runLoops()

    # Restore and Clean up.
    imager.finalize()

    # Save history - Params, Flux Cleaned, Peak Residual, Niter, TimeTaken, Image Names?
    return imager.returninfo()


###################################################
##  Single-node imaging.
###################################################

class PySynthesisImager:
    """ Class to do imaging and deconvolution """

    def __init__(self,casalog,params):
        # SI Tool is a list, for the serial case of length 1
        self.siTools = []
        slef.dcTools = []
        self.ibTool = casac.synthesisiterbot()
        self.casalog = casalog
        self.params = params
        self.listofimagedefinitions = []
        self.casalog.origin('tclean')

    def calculateImages(self):
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


    def checkParameters(self):
        self.casalog.origin('tclean.checkParameters')
        self.casalog.post('Verifying Input Parameters')
        # Init the error-string
        errs = "" 
        errs += self.checkAndFixSelectionPars( self.params['dataselection'] )
        errs += self.checkAndFixImageCoordPars(self.params['imagedefinition'] )

        ## If there are errors, print a message and exit.
        if len(errs) > 0:
            casalog.post('Parameter Errors : \n' + errs,'WARN')
            return False
        return True

    def initialize(self):
        self.casalog.origin('tclean.initialize')

        imagingPlan = self.calculateImages()

        for imageSet in imagingPlan:
            for selection in imageSet['dataselection']:
                siTool = casac.synthesisimager()
                siTool.selectdata(selpars = selection)
                for image in imageSet.deconvolverField:
                    siTool.defineImage(image) #Part Image Name
                    siTool.setupimaging(gridpars=self.params['imaging'])
                    siTool.initmapper()
                self.siTools.append(siTool)

            for image in imageSet.deconvolverField:
                dcTool = casac.synthesisdeconvolver()
                dcTool.setupdeconvolution(decpars={}) #List of Part Names
                self.dcTools.append(dcTool)

        # Setup the tool to have the iteration control
        self.ibTool.setupiteration(iterpars=self.params['iteration'])

    def runMajorCycle(self):
        self.casalog.origin('tclean.runMajorCycle')

        # Get the controls from the first
        controlRecord = self.ibTools.getmajorcyclecontrols()
                
        # To Parallelize: move this across all engines
        for siTool in self.siTools:
            siTool.executemajorcycle(controlRecord);

        self.ibTools.endmajorcycle()
            
    def runMinorCycle(self):
        self.casalog.origin('tclean.runMinorCycle')

        execSummaryList = []
        # Get the conrols from the first
        minorCycleControls = self.ibTool.getsubiterbot();

        for dcTool in self.dcTools:
            minorExecSummayList.append(dcTool.executeminorcycle(minorCycleControls))
            
        # Report the results to the first controller
        for execSummary in execSummaryList:
            self.ibTools.endminorcycle(execSummary)
            

    def runLoops(self):
        self.casalog.origin('tclean.runLoops')
        self.runMajorCycle()
        while not self.ibTool.cleanComplete():
            self.runMinorCycle()
            self.runMajorCycle()

    def finalize(self):
        #self.ibTools.endloops()
        pass

    def returninfo(self):
        return self.ibTool.getiterationsummary()


    ###### Start : Parameter-checking functions ##################
    def checkAndFixSelectionPars(self, selpars={} ):
        errs=""
        # vis, field, spw, etc must all be equal-length lists of strings.
        if not selpars.has_key('vis'):
            errs = errs + 'MS name(s) not specified'
        else:
            if type(selpars['vis'])==str:
                selpars['vis']=[selpars['vis']]
            nvis = len(selpars['vis'])
            selkeys = ['field','spw']
            for par in selkeys:
                if selpars.has_key(par):
                    if selpars[par]=='':
                        selpars[par] = []
                        for it in range(0,nvis):
                            selpars[par].append('')
                    if type(selpars[par])==str:
                        selpars[par]=[selpars[par]]
                    if len(selpars[par]) != nvis:
                        errs = errs + "Selection for " + par + " must be a list of " + str(nvis) + " selection strings\n"
                else:
                    errs = errs + "Selection for " + par + " is unspecified\n"

        return errs

    def checkAndFixImageCoordPars(self, impars={} ):
        errs=""

        ### Get a list of image-coord pars from the multifield outlier file + main field...
        ### Go through this list, and do setup. :  Fill in self.listofimagedefinitions with dicts of params

        ## Convert lists per parameters, into a list of parameter-sets.
        
        ## Main field is always in 'impars'.
        self.listofimagedefinitions.append( {'imagename':impars['imagename'], 'nchan':impars['nchan'], 'imsize':impars['imsize'] } )
        
        ## Multiple images have been specified. 
        ## (1) Parse the outlier file and fill a list of imagedefinitions
        ## OR (2) Parse lists per input parameter into a list of parameter-sets (imagedefinitions)
        ### The code below implements (1)
        if len(impars['outlierfile'])>0:
            self.listofimagedefinitions = self.listofimagedefinitions +( self.parseOutlierFile(impars['outlierfile']) )

        #print 'BEFORE : ', self.listofimagedefinitions

        ## Synchronize parameter types here. 
        for eachimdef in self.listofimagedefinitions:

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

    ###### End : Parameter-checking functions ##################

    ## Parse outlier file and construct a list of imagedefinitions (dictionaries).
    def parseOutlierFile(self, outlierfilename="" ):
        if not os.path.exists( outlierfilename ):
             print 'Cannot find outlier file : ', outlierfilename
             return {}

        returnlist = []

        fp = open( outlierfilename, 'r' )
        thelines = fp.readlines()
        tempd = {}
        for oneline in thelines:
            parpair = oneline.replace(' ','').replace('\n','').split("=")  
            #print parpair
            if len(parpair) != 2:
                print 'Error in line containing : ', oneline
                print returnlist
                return returnlist
            if parpair[0] == 'imagename' and tempd != {}:
                returnlist.append(tempd)
                tempd={}
            tempd [ parpair[0] ] = parpair[1] 

        returnlist.append(tempd)
        #print returnlist
        return returnlist


###################################################
##  Parallel imaging.
###################################################

class ParallelPySynthesisImager(PySynthesisImager):
    '''
    Class to do imaging and deconvolution, with major cycles
    distributed across cluster nodes
    '''

    def __init__(self,casalog,params):
        PySynthesisImager.__init__(self,casalog,params)
        self.nchunks = len(params['other']['clusterdef'])

    def runMajorCycle(self):
        # For now just call the serial case, but in the future this
        # would span it out across the engines
        PySynthesisImager.runMajorCycle(self)
        
        # We may want to do this in all cases
        self.gatherImages()

    def runMinorCycle(self):
        # For now just call the serial case, but in the future this
        # would span it out across the engines
        PySynthesisImager.runMinorCycle(self)

    def initializeTool(self, selectionParameters, imageParameters):
        # For now this just runs the serial case, but should be starting
        # each tool on a different engine
        PySynthesisImager.initializeTool(self, selectionParameters, \
                                         imageParameters)
        
    def gatherImages(self):
        casalog.origin('parallel.tclean.gatherimages')
        casalog.post('Gather images from all chunks')

