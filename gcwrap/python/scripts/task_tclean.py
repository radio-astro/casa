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
           imagename='',nchan=1,
           startmodel='',
           niter=0, threshold=0.0, loopgain=0.1, maxcycleniter=-1, cyclefactor=1, minpsffraction=0.05, maxpsffraction=0.8,
           usescratch=True, clusterdef=''):

    #casalog.post('This is an empty task. It is meant only to build/test/maintain the interface for the refactored imaging code. When ready, it will be offered to users for testing.','WARN')

    # Put all parameters into dictionaries
    params={}
    params['dataselection']={'vis':vis, 'field':field, 'spw':spw, 'usescratch':usescratch}
    params['imagedefinition']={'imagename':imagename, 'nchan':nchan}
    params['imaging']={'startmodel':startmodel}
    params['deconvolution']={}
    params['iteration']={'niter':niter, 'threshold':threshold, 'loopgain':loopgain, 'maxcycleniter':maxcycleniter, 'cyclefactor':cyclefactor , 'minpsffraction':minpsffraction, 'maxpsffraction':maxpsffraction}
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
        self.loopcontrols = {}
        self.listofimagedefinitions = []
        self.casalog.origin('tclean')


    def checkParameters(self):
        self.casalog.origin('tclean.checkParameters')
        self.casalog.post('Verifying Input Parameters')
        # Init the error-string
        errs = "" 

        errs = errs + self.checkAndFixSelectionPars( self.params['dataselection'] )

        errs = errs + self.checkAndFixImageCoordPars( self.params['imagedefinition'] )

        ## If there are errors, print a message and exit.
        if len(errs) > 0:
            casalog.post('Parameter Errors : \n' + errs)
            return False
        return True

    def initialize(self, toolsi=None):
        self.casalog.origin('tclean.initialize')
        if toolsi==None:
            #self.toolsi = gentools(['si'])[0]
            self.toolsi = casac.synthesisimager() ##gentools(['si'])[0]
            toolsi = self.toolsi
        toolsi.selectdata(selpars=self.params['dataselection'])

        impars = copy.deepcopy( self.params['imagedefinition'] )

        ## Start a loop on 'multi-fields' here....
        for eachimdef in self.listofimagedefinitions:
            # Fill in the individual parameters
            impars['imagename'] = eachimdef[ 'imagename' ]
            # Init a mapper for each individual field
            toolsi.defineimage(impars=impars)
            toolsi.setupimaging(gridpars=self.params['imaging'])
            toolsi.setupdeconvolution(decpars=self.params['deconvolution'])
            toolsi.initmapper()
        ## End loop on 'multi-fields' here....

        self.loopcontrols = toolsi.setupiteration(iterpars=self.params['iteration'] ) # From ParClean loopcontrols gets overwritten, but it is always with the same thing. Try to clean this up. 

        ##toolsi.initcycles()


    def runMajorCycle(self,toolsi=None):
        self.casalog.origin('tclean.runMajorCycle')
        if toolsi==None:
            toolsi = self.toolsi
        self.loopcontrols.update( toolsi.runmajorcycle( self.loopcontrols ) ) 
        # In this prev statement, send in updated model to override default.


    def runMinorCycle(self, toolsi=None):
        self.casalog.origin('tclean.runMinorCycle')
        if toolsi==None:
            toolsi = self.toolsi
        self.loopcontrols.update( toolsi.runminorcycle( self.loopcontrols ) )

    def runLoops(self):
        self.casalog.origin('tclean.runLoops')
        self.runMajorCycle()
        while not self.loopcontrols['stop']:  # Make the tool take loopcontrols as in/out ( not const ! )
            self.runMinorCycle()
            self.runMajorCycle()

    def finalize(self, toolsi=None):
        if toolsi==None:
            toolsi = self.toolsi
        toolsi.endloops( self.loopcontrols )
        #toolsi.done()

    def returninfo(self):
        return self.loopcontrols


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

        ## FOR NOW... this list is just a list of image names....

        ## One image only
        if type( impars['imagename'] ) == str:
            self.listofimagedefinitions.append( {'imagename':impars['imagename']} )

        ## If multiple images are specified....... 
        if type( impars['imagename'] ) == list:
            for imname in impars['imagename']:
                self.listofimagedefinitions.append( {'imagename':imname} )

        return errs

    ###### End : Parameter-checking functions ##################



###################################################
##  Parallel imaging.
###################################################

class ParallelPySynthesisImager(PySynthesisImager):
    """ Class to do imaging and deconvolution, with major cycles distributed across cluster nodes """

    def __init__(self,casalog,params):
        PySynthesisImager.__init__(self,casalog,params)
        # Read params['other']['clusterdef'] to decide chunking.
        self.nchunks = len(params['other']['clusterdef'])
        # Initialize a list of synthesisimager tools
        self.toollist = []
        for ch in range(0,self.nchunks):
            #self.toollist.append( gentools(['si'])[0]  )
            self.toollist.append( casac.synthesisimager() ) ## gentools(['si'])[0]  )


    def initialize(self):
        selpars = copy.deepcopy( self.params['dataselection'] )
        for ch in range(0,self.nchunks):
            casalog.origin('parallel.tclean.runMinorCycle')
            casalog.post('Initialize for chunk '+str(ch))

            for dat in range(0,len( selpars['spw'] )):
                self.params['dataselection']['spw'][dat] = selpars['spw'][dat] + ':'+str(ch)

            PySynthesisImager.initialize(self, self.toollist[ch] )
        self.params['dataselection'] = copy.deepcopy(selpars)

    def runMajorCycle(self):
        lcontrols = copy.deepcopy( self.loopcontrols )
        for ch in range(0,self.nchunks):
            self.loopcontrols = copy.deepcopy( lcontrols )
            PySynthesisImager.runMajorCycle(self, self.toollist[ch] )  # Send in updated model as startmodel..
        self.gatherImages()

    def runMinorCycle(self):
        casalog.origin('parallel.tclean.runMinorCycle')
        casalog.post('Set combined images for the minor cycle')
        PySynthesisImager.runMinorCycle(self, self.toollist[0] )  # Use the full list for spectral-cube deconv.
        casalog.origin('parallel.tclean.runMinorCycle')
        casalog.post('Mark updated model as input to all major cycles') 

    def finalize(self):
        self.toollist[0].endloops( self.loopcontrols )
        #for ch in range(0,self.nchunks):
        #    self.toollist[ch].done()

    def gatherImages(self):
        casalog.origin('parallel.tclean.gatherimages')
        casalog.post('Gather images from all chunks')

