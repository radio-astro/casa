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
        self.toolsi=None #gentools(['si'])[0]
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

    def initialize(self, toolsi=None, initializeIteration = True):
        self.casalog.origin('tclean.initialize')
        if toolsi==None:
            print "Creating tool"
            self.toolsi = casac.synthesisimager()
            print "Back from Creating tool"
            toolsi = self.toolsi
        toolsi.selectdata(selpars=self.params['dataselection'])

        ## Start a loop on 'multi-fields' here....
        for eachimdef in self.listofimagedefinitions:
            # Init a mapper for each individual field
            toolsi.defineimage(impars=eachimdef)
            toolsi.setupimaging(gridpars=self.params['imaging'])
            toolsi.setupdeconvolution(decpars=self.params['deconvolution'])
            toolsi.initmapper()
        ## End loop on 'multi-fields' here....

        if initializeIteration:
            toolsi.setupiteration(iterpars=self.params['iteration'])
        # From ParClean loopcontrols gets overwritten, but it is always with the same thing. Try to clean this up. 


    def runMajorCycle(self,toolsi=None):
        self.casalog.origin('tclean.runMajorCycle')
        if toolsi==None:
            toolsi = self.toolsi
        toolsi.runmajorcycle()

    def runMinorCycle(self, toolsi=None):
        self.casalog.origin('tclean.runMinorCycle')
        if toolsi==None:
            toolsi = self.toolsi
        toolsi.runminorcycle()

    def runLoops(self, toolsi=None):
        self.casalog.origin('tclean.runLoops')
        if toolsi==None:
            toolsi = self.toolsi
        self.runMajorCycle()
        while not toolsi.cleanComplete():
            self.runMinorCycle()
            self.runMajorCycle()

    def finalize(self, toolsi=None):
        if toolsi==None:
            toolsi = self.toolsi
        toolsi.endloops()

    def returninfo(self, toolsi=None):
        if toolsi==None:
            toolsi = self.toolsi
        return toolsi.getiterationsummary()


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
        # Read params['other']['clusterdef'] to decide chunking.
        self.nchunks = len(params['other']['clusterdef'])
        # Initialize a list of synthesisimager tools
        self.toollist = []
        for ch in range(0,self.nchunks):
            self.toollist.append( casac.synthesisimager() ) 

    def initialize(self):
        selpars = copy.deepcopy( self.params['dataselection'] )
        imdefs = copy.deepcopy( self.listofimagedefinitions )
        for ch in range(0,self.nchunks):
            casalog.origin('parallel.tclean.runMinorCycle')
            casalog.post('Initialize for chunk '+str(ch))

            # Set up the chunks to parallelize on
            for dat in range(0,len( selpars['spw'] )):
                self.params['dataselection']['spw'][dat] =  selpars['spw'][dat] + ':'+str(ch)

            # Change the image name for each chunk
            for im in range(0, len(self.listofimagedefinitions)):
                self.listofimagedefinitions[im]['imagename'] = imdefs[im]['imagename'] + '_' + str(ch)

            PySynthesisImager.initialize(self, self.toollist[ch], ch == 0 )

        self.params['dataselection'] = copy.deepcopy(selpars)
        self.listofimagedefinitions = copy.deepcopy(imdefs)

    def runMajorCycle(self):
        # Get the controls from the first
        controlRecord = self.toollist[0].getmajorcyclecontrols()
        
        
        # To Parallelize: move this across all engines
        for tool in self.toollist:
            tool.executemajorcycle(controlRecord);

        self.toollist[0].endMajorCycle()
            
        # Send in updated model as startmodel..
        #PySynthesisImager.runMajorCycle(self, self.toollist[ch])
        self.gatherImages()

    def runMinorCycle(self):
        casalog.origin('parallel.tclean.runMinorCycle')
        casalog.post('Set combined images for the minor cycle')
        # subIterBot = self.toollist[0].getsubiterbot();
        
        #         for tool in self.toollist:
        #             returnBot = tool.executeminorcycle(subIterBot);
        #             self.toollist[0].endminorcycle(returnBot);

        PySynthesisImager.runMinorCycle(self, self.toollist[0] )  # Use the full list for spectral-cube deconv.
        casalog.origin('parallel.tclean.runMinorCycle')
        casalog.post('Mark updated model as input to all major cycles') 

    def finalize(self):
        self.toollist[0].endloops()

    def gatherImages(self):
        casalog.origin('parallel.tclean.gatherimages')
        casalog.post('Gather images from all chunks')

