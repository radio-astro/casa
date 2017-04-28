import os
import commands
import math
import shutil
import string
import time
import re;
from taskinit import *
import copy


'''
A set of helper functions for the tasks  tclean

Summary...
    
'''


######################################################
######################################################
######################################################
######################################################

class ImagerParameters():
    def __init__(self, 

                 ## Data Selection
                 msname='',
                 field='',
                 spw='',
                 timestr='',
                 uvdist='',
                 antenna='',
                 scan='',
                 obs='',
                 state='',
                 datacolumn='corrected',

                 ## Image Definition
                 imagename='', 
                 imsize=[1,1], 
                 cell=[10.0,10.0],
                 phasecenter='',
                 stokes='I',
                 projection='SIN',
                 startmodel='', 

                 ## Spectral Parameters
                 specmode='mfs', 
                 reffreq='',
                 nchan=1, 
                 start='', 
                 width='',
                 outframe='LSRK', 
                 veltype='radio', 
                 restfreq=[''],
                 sysvel='', 
                 sysvelframe='',
                 interpolation='nearest',

                 gridder="standard",
#                 ftmachine='gridft', 
                 facets=1, 
                 chanchunks=1,

                 wprojplanes=1,

                 vptable="",

                 aterm=True,
                 psterm=True,
                 mterm=True,
                 wbawp = True,
                 cfcache = "",
                 dopointing = False,
                 dopbcorr = True,
                 conjbeams = True,
                 computepastep =360.0,
                 rotatepastep =360.0,
                 
                 pblimit=0.01,
                 normtype='flatnoise',

                 outlierfile='',
                 restart=True,

                 weighting='natural', 
                 robust=0.5,
                 npixels=0,
                 uvtaper=[],

                 niter=0, 
                 cycleniter=0, 
                 loopgain=0.1,
                 threshold='0.0Jy',
                 cyclefactor=1.0,
                 minpsffraction=0.1,
                 maxpsffraction=0.8,
                 interactive=False,

                 deconvolver='hogbom',
                 scales=[],
                 nterms=1,
                 scalebias=0.6,
                 restoringbeam=[],
#                 mtype='default',

                 usemask='user',
                 mask='',
                 pbmask=0.0,
                 maskthreshold='',
                 maskresolution='',
                 nmask=0,
#                 autoadjust=False,

                 sidelobethreshold=5.0,
                 noisethreshold=3.0,
                 lownoisethreshold=3.0,
                 smoothfactor=1.0,
                 minbeamfrac=0.3,
                 cutthreshold=0.01,

#                 usescratch=True,
#                 readonly=True,
                 savemodel="none",

                 workdir='',

                 ## CFCache params
                 cflist=[]
                 ):

        self.defaultKey="0";
        ## Selection params. For multiple MSs, all are lists.
        ## For multiple nodes, the selection parameters are modified inside PySynthesisImager
        self.allselpars = {'msname':msname, 'field':field, 'spw':spw, 'scan':scan,
                           'timestr':timestr, 'uvdist':uvdist, 'antenna':antenna, 'obs':obs,'state':state,
                           'datacolumn':datacolumn,
                           'savemodel':savemodel }
#                           'usescratch':usescratch, 'readonly':readonly}

        ## Imaging/deconvolution parameters
        ## The outermost dictionary index is image field. 
        ## The '0' or main field's parameters come from the task parameters
        ## The outlier '1', '2', ....  parameters come from the outlier file
        self.outlierfile = outlierfile
        ## Initialize the parameter lists with the 'main' or '0' field's parameters
        ######### Image definition
        self.allimpars = { self.defaultKey :{'imagename':imagename, 'nchan':nchan, 'imsize':imsize, 
                                 'cell':cell, 'phasecenter':phasecenter, 'stokes': stokes,
                                 'specmode':specmode, 'start':start, 'width':width, 'veltype':veltype,
                                 'nterms':nterms,'restfreq':restfreq, 
                                 'outframe':outframe, 'reffreq':reffreq, 'sysvel':sysvel, 'sysvelframe':sysvelframe,
                                 'projection':projection,
                                 'restart':restart, 'startmodel':startmodel,'deconvolver':deconvolver}    }
        ######### Gridding
        self.allgridpars = { self.defaultKey :{'gridder':gridder,
                                   'aterm': aterm, 'psterm':psterm, 'mterm': mterm, 'wbawp': wbawp, 
                                   'cfcache': cfcache,'dopointing':dopointing, 'dopbcorr':dopbcorr, 
                                   'conjbeams':conjbeams, 'computepastep':computepastep,
                                   'rotatepastep':rotatepastep, #'mtype':mtype, # 'weightlimit':weightlimit,
                                   'facets':facets,'chanchunks':chanchunks,
                                   'interpolation':interpolation, 'wprojplanes':wprojplanes,
                                   'deconvolver':deconvolver, 'vptable':vptable }     }
        ######### weighting
        self.weightpars = {'type':weighting,'robust':robust, 'npixels':npixels,'uvtaper':uvtaper}

        ######### Normalizers ( this is where flat noise, flat sky rules will go... )
        self.allnormpars = { self.defaultKey : {#'mtype': mtype,
                                 'pblimit': pblimit,'nterms':nterms,'facets':facets,
                                 'normtype':normtype, 'workdir':workdir,
                                 'deconvolver':deconvolver}     }

        ######### Deconvolution
        self.alldecpars = { self.defaultKey: { 'id':0, 'deconvolver':deconvolver, 'nterms':nterms, 
                                    'scales':scales, 'scalebias':scalebias, 'restoringbeam':restoringbeam, 'usemask':usemask, 
                                    'mask':mask, 'pbmask':pbmask, 'maskthreshold':maskthreshold,
                                    'maskresolution':maskresolution, 'nmask':nmask,
                                    #'maskresolution':maskresolution, 'nmask':nmask,'autoadjust':autoadjust,
                                    'sidelobethreshold':sidelobethreshold, 'noisethreshold':noisethreshold,
                                    'lownoisethreshold':lownoisethreshold, 'smoothfactor':smoothfactor,
                                    'minbeamfrac':minbeamfrac, 'cutthreshold':cutthreshold,
                                    'interactive':interactive, 'startmodel':startmodel} }

        ######### Iteration control. 
        self.iterpars = { 'niter':niter, 'cycleniter':cycleniter, 'threshold':threshold, 
                          'loopgain':loopgain, 'interactive':interactive,
                          'cyclefactor':cyclefactor, 'minpsffraction':minpsffraction, 
                          'maxpsffraction':maxpsffraction, 'savemodel':savemodel}

        ######### CFCache params. 
        self.cfcachepars = {'cflist': cflist};


        #self.reusename=reuse

        ## List of supported parameters in outlier files.
        ## All other parameters will default to the global values.
        self.outimparlist = ['imagename','nchan','imsize','cell','phasecenter','startmodel',
                             'start','width',
                             'nterms','reffreq','specmode']
        self.outgridparlist = ['gridder','deconvolver','wprojplanes']
        self.outweightparlist=[]
        self.outdecparlist=['deconvolver','startmodel','nterms','usemask','mask']
        self.outnormparlist=['deconvolver','weightlimit','nterms']
#        self.outnormparlist=['imagename','mtype','weightlimit','nterms']

        ret = self.checkParameters()
        if ret==False:
            casalog.post('Found errors in input parameters. Please check.', 'WARN')

        self.printParameters()

    def getSelPars(self):
        return self.allselpars
    def getImagePars(self):
        return self.allimpars
    def getGridPars(self):
        return self.allgridpars
    def getWeightPars(self):
        return self.weightpars
    def getDecPars(self):
        return self.alldecpars
    def getIterPars(self):
        return self.iterpars
    def getNormPars(self):
        return self.allnormpars
    def getCFCachePars(self):
        return self.cfcachepars;

    def setSelPars(self,selpars):
        self.allselpars = selpars
    def setImagePars(self,impars):
        self.allimpars = impars
    def setGridPars(self,gridpars):
        self.allgridpars = gridpars
    def setWeightPars(self,weightpars):
        self.weightpars = weightpars
    def setDecPars(self,decpars):
        self.alldecpars = decpars
    def setIterPars(self,iterpars):
        self.iterpars = iterpars
    def setNormPars(self,normpars):
        self.allnormpars = normpars



    def checkParameters(self):
        #casalog.origin('refimagerhelper.checkParameters')
        casalog.post('Verifying Input Parameters')
        # Init the error-string
        errs = "" 
        errs += self.checkAndFixSelectionPars()
        errs += self.makeImagingParamLists()
        errs += self.checkAndFixIterationPars()
        errs += self.checkAndFixNormPars()

        for mss in sorted( self.allselpars.keys() ):
            if(self.allimpars['0']['specmode']=='cubedata'):
                self.allselpars[mss]['outframe']='Undefined'

        ### MOVE this segment of code to the constructor so that it's clear which parameters go where ! 
        ### Copy them from 'impars' to 'normpars' and 'decpars'
        self.iterpars['allimages']={}
        for immod in self.allimpars.keys() :
            self.allnormpars[immod]['imagename'] = self.allimpars[immod]['imagename']
            self.alldecpars[immod]['imagename'] = self.allimpars[immod]['imagename']
            self.allgridpars[immod]['imagename'] = self.allimpars[immod]['imagename']
            self.iterpars['allimages'][immod] = { 'imagename':self.allimpars[immod]['imagename'] , 'multiterm': (self.alldecpars[immod]['deconvolver']=='mtmfs') }

        ## Integers need to be NOT numpy versions.
        self.fixIntParam(self.allimpars, 'imsize')
        self.fixIntParam(self.allimpars, 'nchan')
        self.fixIntParam(self.allimpars,'nterms')
        self.fixIntParam(self.allnormpars,'nterms')
        self.fixIntParam(self.alldecpars,'nterms')
        self.fixIntParam(self.allgridpars,'facets')
        self.fixIntParam(self.allgridpars,'chanchunks')
 
        ## If there are errors, print a message and exit.
        if len(errs) > 0:
#            casalog.post('Parameter Errors : \n' + errs,'WARN')
            raise Exception("Parameter Errors : \n" + errs)
 #           return False
        return True

    ###### Start : Parameter-checking functions ##################


    def checkAndFixSelectionPars(self):
        errs=""

        # If it's already a dict with ms0,ms1,etc...leave it be.
        ok=True
        for kk in self.allselpars.keys():
            if kk.find('ms')!=0:
                ok=False

        if ok==True:
            #print "Already in correct format"
            return errs

        # msname, field, spw, etc must all be equal-length lists of strings, or all except msname must be of length 1.
        if not self.allselpars.has_key('msname'):
            errs = errs + 'MS name(s) not specified'
        else:

            selkeys = self.allselpars.keys()

            # Convert all non-list parameters into lists.
            for par in selkeys:
                if type( self.allselpars[par] ) != list:
                    self.allselpars[par] = [ self.allselpars[par]  ]
                    
            # Check that all are the same length as nvis
            # If not, and if they're single, replicate them nvis times
            nvis = len(self.allselpars['msname'])
            for par in selkeys:
                if len( self.allselpars[par] ) > 1 and len( self.allselpars[par] ) != nvis:
                    errs = errs + str(par) + ' must have a single entry, or ' + str(nvis) + ' entries to match vis list \n'
                    return errs
                else: # Replicate them nvis times if needed.
                    if len( self.allselpars[par] ) == 1:
                        for ms in range(1,nvis):
                            self.allselpars[par].append( self.allselpars[par][0] )
                    

            # Now, all parameters are lists of strings each of length 'nvis'.
            # Put them into separate dicts per MS.
            selparlist={}
            for ms in range(0,nvis):
                selparlist[ 'ms'+str(ms) ] = {}
                for par in selkeys:
                    selparlist[ 'ms'+str(ms) ][ par ] = self.allselpars[par][ms]

                synu = casac.synthesisutils()
                selparlist[ 'ms'+str(ms) ] = synu.checkselectionparams( selparlist[ 'ms'+str(ms)] )
                synu.done()

#            print selparlist

            self.allselpars = selparlist

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
            self.allnormpars[ modelid ] = copy.deepcopy(self.allnormpars[ '0' ])
            self.allnormpars[ modelid ].update(outlierpars[immod]['normpars'])
            self.alldecpars[ modelid ][ 'id' ] = immod+1  ## Try to eliminate.


        #print self.allimpars

#
#        print "REMOVING CHECKS to check..."
#### This does not handle the conversions of the csys correctly.....
####
#        for immod in self.allimpars.keys() :
#            tempcsys = {}
#            if self.allimpars[immod].has_key('csys'):
#                tempcsys = self.allimpars[immod]['csys']
#
#            synu = casac.synthesisutils()
#            self.allimpars[immod] = synu.checkimageparams( self.allimpars[immod] )
#            synu.done()
#
#            if len(tempcsys.keys())==0:
#                self.allimpars[immod]['csys'] = tempcsys

        ## Check for name increments, and copy from impars to decpars and normpars.
        self.handleImageNames()

        return errs

    def handleImageNames(self):

            for immod in self.allimpars.keys() :
                inpname = self.allimpars[immod]['imagename']

                ### If a directory name is embedded in the image name, check that the dir exists.
                if inpname.count('/'):
                    splitname = inpname.split('/')
                    prefix = splitname[ len(splitname)-1 ]
                    dirname = inpname[0: len(inpname)-len(prefix)]   # has '/' at end
                    if not os.path.exists( dirname ):
                        casalog.post('Making directory : ' + dirname, 'INFO')
                        os.mkdir( dirname )
                    
            ### Check for name increments 
            #if self.reusename == False:

            if self.allimpars['0']['restart'] == False:   # Later, can change this to be field dependent too.
                ## Get a list of image names for all fields (to sync name increment ids across fields)
                inpnamelist={}
                for immod in self.allimpars.keys() :
                    inpnamelist[immod] = self.allimpars[immod]['imagename'] 

                newnamelist = self.incrementImageNameList( inpnamelist )

                if len(newnamelist) != len(self.allimpars.keys()) :
                    casalog.post('Internal Error : Non matching list lengths in refimagerhelper::handleImageNames. Not updating image names','WARN')
                else : 
                    for immod in self.allimpars.keys() :
                        self.allimpars[immod]['imagename'] = newnamelist[immod]
                
    def checkAndFixIterationPars(self ):
        errs=""

        # Bother checking only if deconvolution iterations are requested
        if self.iterpars['niter']>0:
            # Make sure cycleniter is less than or equal to niter. 
            if self.iterpars['cycleniter']<=0 or self.iterpars['cycleniter'] > self.iterpars['niter']:
                if self.iterpars['interactive']==False:
                    self.iterpars['cycleniter'] = self.iterpars['niter']
                else:
                    self.iterpars['cycleniter'] = min(self.iterpars['niter'] , 100)

        return errs

    def checkAndFixNormPars(self):  
        errs=""

#        for modelid in self.allnormpars.keys():
#            if len(self.allnormpars[modelid]['workdir'])==0:
#                self.allnormpars[modelid]['workdir'] = self.allnormpars['0']['imagename'] + '.workdir'

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
        tempweightpar={}
        tempdecpar={}
        tempnormpar={}
        for oneline in thelines:
            aline = oneline.replace('\n','')
#            aline = oneline.replace(' ','').replace('\n','')
            if len(aline)>0 and aline.find('#')!=0:
                parpair = aline.split("=")  
                parpair[0] = parpair[0].replace(' ','')
                #print parpair
                if len(parpair) != 2:
                    errs += 'Error in line containing : ' + oneline + '\n'
                if parpair[0] == 'imagename' and tempimpar != {}:
                    #returnlist.append({'impars':tempimpar, 'gridpars':tempgridpar, 'weightpars':tempweightpar, 'decpars':tempdecpar} )
                    returnlist.append({'impars':tempimpar, 'gridpars':tempgridpar, 'weightpars':tempweightpar, 'decpars':tempdecpar, 'normpars':tempnormpar} )
                    tempimpar={}
                    tempgridpar={}
                    tempweightpar={}
                    tempdecpar={}
                    tempnormpar={}
                usepar=False
                if parpair[0] in self.outimparlist:
                    tempimpar[ parpair[0] ] = parpair[1]
                    usepar=True
                if parpair[0] in self.outgridparlist:
                    tempgridpar[ parpair[0] ] = parpair[1]
                    usepar=True
                if parpair[0] in self.outweightparlist:
                    tempweightpar[ parpair[0] ] = parpair[1]
                    usepar=True
                if parpair[0] in self.outdecparlist:
                    tempdecpar[ parpair[0] ] = parpair[1]
                    usepar=True
                if parpair[0] in self.outnormparlist:
                    tempnormpar[ parpair[0] ] = parpair[1]
                    usepar=True
                if usepar==False:
                    print 'Ignoring unknown parameter pair : ' + oneline

        if len(errs)==0:
            returnlist.append( {'impars':tempimpar,'gridpars':tempgridpar, 'weightpars':tempweightpar, 'decpars':tempdecpar, 'normpars':tempnormpar} )

        ## Extra parsing for a few parameters.
        returnlist = self.evalToTarget( returnlist, 'impars', 'imsize', 'intvec' )
        returnlist = self.evalToTarget( returnlist, 'impars', 'nchan', 'int' )
        returnlist = self.evalToTarget( returnlist, 'impars', 'cell', 'strvec' )
        returnlist = self.evalToTarget( returnlist, 'impars', 'nterms', 'int' )
        returnlist = self.evalToTarget( returnlist, 'decpars', 'nterms', 'int' )
        returnlist = self.evalToTarget( returnlist, 'normpars', 'nterms', 'int' )
        returnlist = self.evalToTarget( returnlist, 'gridpars', 'wprojplanes', 'int' )
#        returnlist = self.evalToTarget( returnlist, 'impars', 'reffreq', 'strvec' )


        #print returnlist
        return returnlist, errs


    def evalToTarget(self, globalpars, subparkey, parname, dtype='int' ):
        try:
            for fld in range(0, len( globalpars ) ):
                if globalpars[ fld ][subparkey].has_key(parname):
                    if dtype=='int' or dtype=='intvec':
                        val_e = eval( globalpars[ fld ][subparkey][parname] )
                    if dtype=='strvec':
                        tcell =  globalpars[ fld ][subparkey][parname]
                        tcell = tcell.replace(' ','').replace('[','').replace(']','').replace("'","")
                        tcells = tcell.split(',')
                        val_e = []
                        for cell in tcells:
                            val_e.append( cell )

                    globalpars[ fld ][subparkey][parname] = val_e
        except:
            print 'Cannot evaluate outlier field parameter "' + parname + '"'

        return globalpars


    def printParameters(self):
        casalog.post('SelPars : ' + str(self.allselpars), 'INFO2')
        casalog.post('ImagePars : ' + str(self.allimpars), 'INFO2')
        casalog.post('GridPars : ' + str(self.allgridpars), 'INFO2')
        casalog.post('NormPars : ' + str(self.allnormpars), 'INFO2')
        casalog.post('Weightpars : ' + str(self.weightpars), 'INFO2')
        casalog.post('DecPars : ' + str(self.alldecpars), 'INFO2')
        casalog.post('IterPars : ' + str(self.iterpars), 'INFO2')


    def incrementImageName(self,imagename):
        dirname = '.'
        prefix = imagename

        if imagename.count('/'):
            splitname = imagename.split('/')
            prefix = splitname[ len(splitname)-1 ]
            dirname = './' + imagename[0: len(imagename)-len(prefix)]   # has '/' at end

        inamelist = [fn for fn in os.listdir(dirname) if any([fn.startswith(prefix)])];

        if len(inamelist)==0:
            newimagename = dirname[2:] + prefix
        else:
            nlen = len(prefix)
            maxid=1
            for iname in inamelist:
                startind = iname.find( prefix+'_' )
                if startind==0:
                    idstr = ( iname[nlen+1:len(iname)] ).split('.')[0]
                    if idstr.isdigit() :
                        val = eval(idstr)
                        if val > maxid : 
                            maxid = val
            newimagename = dirname[2:] + prefix + '_' + str(maxid+1)

        print 'Using : ',  newimagename
        return newimagename

    def incrementImageNameList(self, inpnamelist ):

        dirnames={}
        prefixes={}

        for immod in inpnamelist.keys() : 
            imagename = inpnamelist[immod]
            dirname = '.'
            prefix = imagename

            if imagename.count('/'):
                splitname = imagename.split('/')
                prefix = splitname[ len(splitname)-1 ]
                dirname = './' + imagename[0: len(imagename)-len(prefix)]   # has '/' at end

            dirnames[immod] = dirname
            prefixes[immod] = prefix


        maxid=0
        for immod in inpnamelist.keys() : 
            prefix = prefixes[immod]
            inamelist = [fn for fn in os.listdir(dirnames[immod]) if any([fn.startswith(prefix)])];
            nlen = len(prefix)

            if len(inamelist)==0 : 
                locmax=0
            else: 
                locmax=1

            cleanext = ['.image','.residual','.model','.psf','.sumwt','.tt0']
            incremented=False
            for iname in inamelist:
                rootname,ext = os.path.splitext(iname)
                if ext in cleanext:
                    startind = iname.find( prefix+'_' )
                    if startind==0:
                        idstr = ( iname[nlen+1:len(iname)] ).split('.')[0]
                        if idstr.isdigit() :
                            val = eval(idstr)
                            incremented=True
                            if val > locmax : 
                                locmax = val
                    elif startind==-1:
                        if ext=='.tt0':
                           # need one more pass to extract rootname 
                           rootname,ext = os.path.splitext(rootname)
                        if rootname==prefix:
                            # the file name with root file name only
                            incremented=True
                             
            if not incremented: 
                locmax = 0; 
            if locmax > maxid:
                maxid = locmax

        
        newimagenamelist={}
        for immod in inpnamelist.keys() : 
            if maxid==0 : 
                newimagenamelist[immod] = inpnamelist[immod]
            else:
                newimagenamelist[immod] = dirnames[immod][2:] + prefixes[immod] + '_' + str(maxid+1) 

#        print 'Input : ',  inpnamelist
#        print 'Dirs : ', dirnames
#        print 'Pre : ', prefixes
#        print 'Max id : ', maxid
#        print 'Using : ',  newimagenamelist
        return newimagenamelist

    ## Guard against numpy int32,int64 types which don't convert well across tool boundary.
    ## For CAS-8250. Remove when CAS-6682 is done.
    def fixIntParam(self, allpars, parname ):
        for immod in allpars.keys() :
            if allpars[immod].has_key(parname):
                ims = allpars[immod][parname]
                if type(ims) != list:
                    ims = int(ims)
                else:
                    for el in range(0,len(ims)):
                        ims[el] = int(ims[el])
                allpars[immod][parname] = ims
      ############################
