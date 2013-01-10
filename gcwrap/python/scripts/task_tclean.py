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
        self.nchunks = 1
        self.casalog = casalog
        self.params = params
        self.listofimagedefinitions = []
        self.casalog.origin('tclean')


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

        # Create the tool we are going to use (only one in this case)
        for (selection, images) in self.calculateChunks():
            print selection
            print images
            self.initializeTool(selection, images)

        # Setup the tool to have the iteration control
        self.siTools[0].setupiteration(iterpars=self.params['iteration'])

    def initializeTool(self, selectionParameters, imageParameters):
        # This method adds an additional tool to the list of tools
        self.casalog.origin('tclean.initializeTool')
        siTool = casac.synthesisimager()
        siTool.selectdata(selpars = selectionParameters)
        
        ## Loop over each image for multi-field like cases
        for eachimdef in imageParameters:
            # Init a mapper for each individual field
            siTool.defineimage(impars=eachimdef)
            siTool.setupimaging(gridpars=self.params['imaging'])
            siTool.setupdeconvolution(decpars=self.params['deconvolution'])
            siTool.initmapper()
        ## End loop on 'multi-fields' here....

        self.siTools.append(siTool)

    def calculateChunks(self):
        # This probably will move to c++ at some point but for now:
        # Return tuples of dataselection / imagedefinition pairs one for
        # each tool
        chunkList = []

        if self.nchunks == 1:
            # Nothing to do, single chunk
            return [(self.params['dataselection'],
                     self.listofimagedefinitions)]
            
        for ch in range(0,self.nchunks):
            casalog.origin('parallel.tclean.runMinorCycle')
            casalog.post('Initialize for chunk '+str(ch))

            selpars = copy.deepcopy( self.params['dataselection'] )
            imdefs = copy.deepcopy( self.listofimagedefinitions )

            # Set up the chunks to parallelize on
            for dat in range(0,len(self.params['dataselection']['spw'] )):
                selpars['spw'][dat] = \
                   self.params['dataselection']['spw'][dat] + ':'+str(ch)

            # Change the image name for each chunk
            for im in range(0, len(self.listofimagedefinitions)):
                imdefs[im]['imagename'] = \
                  self.listofimagedefinitions[im]['imagename'] + '_' + str(ch)

            chunkList.append((selpars, imdefs))
        return chunkList

    def runMajorCycle(self):
        self.casalog.origin('tclean.runMajorCycle')

        # Get the controls from the first
        controlRecord = self.siTools[0].getmajorcyclecontrols()
                
        # To Parallelize: move this across all engines
        for siTool in self.siTools:
            siTool.executemajorcycle(controlRecord);

        self.siTools[0].endmajorcycle()
            
    def runMinorCycle(self):
        self.casalog.origin('tclean.runMinorCycle')

        returnBotList = []
        # Get the conrols from the first
        subIterBot = self.siTools[0].getsubiterbot();

        # JSK ToDo: we need to run on all tools once we have selective
        # execution of the deconvolver
        
        # for siTool in self.siTools:
        for siTool in self.siTools[:1]:
            returnBotList.append(siTool.executeminorcycle(subIterBot))

        # Report the results to the first controller
        for returnBot in returnBotList:
            self.siTools[0].endminorcycle(returnBot);
            

    def runLoops(self):
        self.casalog.origin('tclean.runLoops')
        self.runMajorCycle()
        while not self.siTools[0].cleanComplete():
            self.runMinorCycle()
            self.runMajorCycle()

    def finalize(self):
        self.siTools[0].endloops()

    def returninfo(self):
        return self.siTools[0].getiterationsummary()


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

