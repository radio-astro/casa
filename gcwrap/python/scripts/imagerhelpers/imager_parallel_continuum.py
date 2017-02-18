import os
import commands
import math
import shutil
import string
import time
import re;
from taskinit import *
import copy

from imagerhelpers.imager_base import PySynthesisImager
from imagerhelpers.parallel_imager_helper import PyParallelImagerHelper

'''
An implementation of parallel continuum imaging, using synthesisxxxx tools

Datasets are partitioned by row and major cycles are parallelized. 
Gathers and normalization are done before passing the images to a
non-parallel minor cycle. The output model image is them scattered to
all the nodes for the next parallel major cycle.

There are N synthesisimager objects.
There is 1 instance per image field, of the normalizer and deconvolver.
There is 1 iterbot. 
    
'''

#############################################
#############################################
## Parallelize only major cycle.
#############################################
class PyParallelContSynthesisImager(PySynthesisImager):

    def __init__(self,params=None):

         PySynthesisImager.__init__(self,params)

         self.PH = PyParallelImagerHelper()
         self.NN = self.PH.NN
         self.selpars = self.allselpars;
         self.allselpars = self.PH.partitionContDataSelection(self.allselpars)
         # self.allcflist = self.PH.partitionCFCacheList(self.cfcachepars['cflist']);
         # self.allcflist = self.PH.partitionCFCacheList(self.allgridpars['0']);
         self.listOfNodes = self.PH.getNodeList();
         self.coordsyspars = {};

#############################################
    def initializeImagersBase(self,thisSelPars,partialSelPars):
        #
        # Start the imagers on all nodes.
        #
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("toolsi = casac.synthesisimager()", node) );
        self.PH.checkJobs(joblist);

        #
        # Select data.  If partialSelPars is True, use the thisSelPars
        # data structure as a list of partitioned selections.
        #
        joblist=[];
        nodes=self.listOfNodes;#[1];
        if (not partialSelPars):
            nodes = [1];
        for node in nodes:
            for mss in sorted( self.selpars.keys() ):
                if (partialSelPars):
                    selStr=str(thisSelPars[str(node-1)][mss]);
                else:
                    #joblist.append( self.PH.runcmd("toolsi.selectdata( "+str(thisSelPars[mss])+")", node) )
                    selStr=str(thisSelPars[mss]);
                joblist.append( self.PH.runcmd("toolsi.selectdata( "+selStr+")", node) )
        self.PH.checkJobs(joblist);

        #
        # Call defineimage at each node.
        #

        # do the first node only first to set csys and distributed it to other nodes
        nimpars = copy.deepcopy(self.allimpars)
        ngridpars = copy.deepcopy(self.allgridpars)
        for fld in range(0,self.NF):
            joblist=[]
            if self.NN>1:
                nimpars[str(fld)]['imagename'] = self.PH.getpartimagename( nimpars[str(fld)]['imagename'], nodes[0] )
            joblist.append( self.PH.runcmd("toolsi.defineimage( impars=" + str( nimpars[str(fld)] ) 
                                               + ", gridpars=" + str( ngridpars[str(fld)] )   + ")", nodes[0] ) )
            #joblist.append( self.PH.runcmdcheck("fullcoords = toolsi.getcsys()") ) 
            self.PH.checkJobs(joblist);
            self.PH.runcmdcheck("fullcoords = toolsi.getcsys()")
            fullcoords = self.PH.pullval("fullcoords", nodes[0] )
            self.coordsyspars[str(fld)] = fullcoords[1]
         
        # do for the rest of nodes
        joblist=[];
        for node in nodes[1:]:
            ## For each image-field, define imaging parameters
            nimpars = copy.deepcopy(self.allimpars)
            #print "nimpars = ",nimpars;
            ngridpars = copy.deepcopy(self.allgridpars)
            for fld in range(0,self.NF):
                if self.NN>1:
                    #nimpars[str(fld)]['imagename'] = self.PH.getpath(node) + '/' + nimpars[str(fld)]['imagename']+'.n'+str(node)
                    nimpars[str(fld)]['imagename'] = self.PH.getpartimagename( nimpars[str(fld)]['imagename'], node )


                joblist.append( self.PH.runcmd("toolsi.defineimage( impars=" + str( nimpars[str(fld)] ) 
                                               + ", gridpars=" + str( ngridpars[str(fld)] )   + ")", node ) )
        self.PH.checkJobs(joblist);
        
#############################################

    def initializeImagers(self):

        #---------------------------------------
        #  Check if cfcache exists.
        #
        cfCacheName=self.allgridpars['0']['cfcache'];
        cfcExists=False;
        if (not (cfCacheName == '')):
            cfcExists = (os.path.exists(cfCacheName) and os.path.isdir(cfCacheName));

            if (cfcExists):
                nCFs = len(os.listdir(cfCacheName));
                if (nCFs == 0):
                    casalog.post(cfCacheName + " exists, but is empty.  Attempt is being made to fill it now.","WARN")
                    cfcExists = False;

        # print "##########################################"
        # print "CFCACHE = ",cfCacheName,cfcExists;
        # print "##########################################"

        # Initialize imagers with full data selection at each node.
        # This is required only for node-1 though (for dryGridding
        # later).
        self.initializeImagersBase(self.selpars,False);
        for fld in range(0, self.NF):
            self.allimpars[str(fld)]['csys']=self.coordsyspars[str(fld)]['coordsys'].copy()

        if (not cfcExists):
            self.dryGridding();
            self.fillCFCache();
#        self.reloadCFCache();

        # TRY: Start all over again!  This time do partial data
        # selection at each node using the allselpars data structure
        # which has the partitioned selection.
        self.deleteImagers();

        self.initializeImagersBase(self.allselpars,True);

######################################################################################################################################
        #---------------------------------------
        #  4. call setdata() for images on all nodes
        #
        # joblist=[];
        # for node in self.listOfNodes:
        #     ## Send in Selection parameters for all MSs in the list
        #     #### MPIInterface related changes (the -1 in the expression below)
        #     for mss in sorted( (self.allselpars[str(node-1)]).keys() ):
        #         joblist.append( self.PH.runcmd("toolsi.selectdata( "+str(self.allselpars[str(node-1)][mss])+")", node) )
        # self.PH.checkJobs(joblist);

        #---------------------------------------
        #  5. Call defineImage() on all nodes.  This sets up the FTMs.
        #
#         joblist=[];
#         for node in self.listOfNodes:
#             ## For each image-field, define imaging parameters
#             nimpars = copy.deepcopy(self.allimpars)
#             #print "nimpars = ",nimpars;
#             ngridpars = copy.deepcopy(self.allgridpars)
#             for fld in range(0,self.NF):
#                 if self.NN>1:
#                     nimpars[str(fld)]['imagename'] = self.PH.getpath(node) + '/' + nimpars[str(fld)]['imagename']+'.n'+str(node)
# ###                    nimpars[str(fld)]['imagename'] = self.allnormpars[str(fld)]['workdir'] + '/' + nimpars[str(fld)]['imagename']+'.n'+str(node)
# ###                    nimpars[str(fld)]['imagename'] = nimpars[str(fld)]['imagename']+'.n'+str(node)

# #                    ngridpars[str(fld)]['cfcache'] = ngridpars[str(fld)]['cfcache']+'.n'+str(node)
#                     # # Give the same CFCache name to all nodes
#                     ngridpars[str(fld)]['cfcache'] = ngridpars[str(fld)]['cfcache'];

#                 joblist.append( self.PH.runcmd("toolsi.defineimage( impars=" + str( nimpars[str(fld)] ) + ", gridpars=" + str( ngridpars[str(fld)] )   + ")", node ) )
#         self.PH.checkJobs(joblist);

        #---------------------------------------
        #  6. If cfcache does not exist, call fillCFCache()
        #       This will fill the "empty" CFCache in parallel
        #  7. Now call reloadCFCache() on all nodes.
        #     This reloads the latest cfcahce.



        # TRY: Start all over again!
        # self.deleteImagers();

        # joblist=[]

        # for node in self.listOfNodes:
        #     joblist.append( self.PH.runcmd("toolsi = casac.synthesisimager()", node) );
        # self.PH.checkJobs(joblist);

        # joblist=[];
        # nodes=self.listOfNodes;#[1];
        # for node in nodes:
        #     for mss in sorted( (self.allselpars[str(node-1)]).keys() ):
        #         joblist.append( self.PH.runcmd("toolsi.selectdata( "+str(self.allselpars[str(node-1)][mss])+")", node) )
        #             # for mss in sorted( self.selpars.keys() ):
        #             #     joblist.append( self.PH.runcmd("toolsi.selectdata( "+str(self.selpars[mss])+")", node) )
        # self.PH.checkJobs(joblist);

        # joblist=[];
        # for node in self.listOfNodes:
        #     nimpars = copy.deepcopy(self.allimpars)
        #     ngridpars = copy.deepcopy(self.allgridpars)
        #     for fld in range(0,self.NF):
        #         if self.NN>1:
        #             nimpars[str(fld)]['imagename'] = self.PH.getpath(node) + '/' + nimpars[str(fld)]['imagename']+'.n'+str(node)
        #             # # Give the same CFCache name to all nodes
        #             ngridpars[str(fld)]['cfcache'] = ngridpars[str(fld)]['cfcache'];

        #         joblist.append( self.PH.runcmd("toolsi.defineimage( impars=" + str( nimpars[str(fld)] ) + ", gridpars=" + str( ngridpars[str(fld)] )   + ")", node ) )
        # self.PH.checkJobs(joblist);


#############################################

    def initializeNormalizers(self):
        for immod in range(0,self.NF):
            self.PStools.append(casac.synthesisnormalizer())
            normpars = copy.deepcopy( self.allnormpars[str(immod)] )
            partnames = []
            if(self.NN>1):
                #### MPIInterface related changes
                #for node in range(0,self.NN):
                for node in self.listOfNodes:
                    partnames.append( self.PH.getpartimagename( self.allimpars[str(immod)]['imagename'], node ) )
                    #onename = self.allimpars[str(immod)]['imagename']+'.n'+str(node)
                    #partnames.append( self.PH.getpath(node) + '/' + onename  )
                    #self.PH.deletepartimages( self.PH.getpath(node), onename ) # To ensure restarts work properly.
                    self.PH.deletepartimages( self.allimpars[str(immod)]['imagename'] ,  node ) # To ensure restarts work properly.
                normpars['partimagenames'] = partnames
            self.PStools[immod].setupnormalizer(normpars=normpars)


#############################################
    def setWeighting(self):

        ## Set weight parameters and accumulate weight density (natural)
        joblist=[];
        for node in self.listOfNodes:
            ## Set weighting pars
            joblist.append( self.PH.runcmd("toolsi.setweighting( **" + str(self.weightpars) + ")", node ) )
        self.PH.checkJobs( joblist )

        ## If only one field, do the get/gather/set of the weight density.
        if self.NF == 1 and self.allimpars['0']['stokes']=="I":   ## Remove after gridded wts appear for all fields correctly (i.e. new FTM).
   
          if self.weightpars['type'] != 'natural' :  ## For natural, this array isn't created at all.
                                                                       ## Remove when we switch to new FTM

            casalog.post("Gathering/Merging/Scattering Weight Density for PSF generation","INFO")

            joblist=[];
            for node in self.listOfNodes:
                joblist.append( self.PH.runcmd("toolsi.getweightdensity()", node ) )
            self.PH.checkJobs( joblist )

           ## gather weightdensity and sum and scatter
            print "******************************************************"
            print " gather and scatter now "
            print "******************************************************"
            for immod in range(0,self.NF):
                self.PStools[immod].gatherweightdensity()
                self.PStools[immod].scatterweightdensity()

           ## Set weight density for each nodel
            joblist=[];
            for node in self.listOfNodes:
                joblist.append( self.PH.runcmd("toolsi.setweightdensity()", node ) )
            self.PH.checkJobs( joblist )



    def deleteImagers(self):
         self.PH.runcmd("toolsi.done()")

    def deleteCluster(self):
         self.PH.takedownCluster()
    
# #############################################
    def dryGridding(self):
        nodes=[1];
        joblist=[];
        for node in nodes:
            dummy=[''];
            cmd = "toolsi.drygridding("+str(dummy)+")";
            joblist.append(self.PH.runcmd(cmd,node));
        self.PH.checkJobs(joblist);

#############################################
    def reloadCFCache(self):
        joblist=[];
        for node in self.listOfNodes:
            cmd = "toolsi.reloadcfcache()";
            print "CMD = ",node," ",cmd;
            joblist.append(self.PH.runcmd(cmd,node));
        self.PH.checkJobs(joblist);
#############################################
    def fillCFCache(self):
        #print "-----------------------fillCFCache------------------------------------"
        # cflist=[f for f in os.listdir(self.allgridpars['cfcache']) if re.match(r'CFS*', f)];
        # partCFList = 
        allcflist = self.PH.partitionCFCacheList(self.allgridpars['0']);
        cfcPath = "\""+str(self.allgridpars['0']['cfcache'])+"\"";
        ftmname = "\""+str(self.allgridpars['0']['gridder'])+"\"";
        psTermOn = str(self.allgridpars['0']['psterm']);
        aTermOn = str(self.allgridpars['0']['aterm']);
        conjBeams = str(self.allgridpars['0']['conjbeams']);
        #aTermOn = str(True);
        # print "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
        # print "AllCFList = ",allcflist;
        m = len(allcflist);
        # print "No. of nodes used: ", m,cfcPath,ftmname;
        # print "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";

        joblist=[];
        for node in self.listOfNodes[:m]:
            #print "#!$#!%#!$#@$#@$ ",allcflist;
            cmd = "toolsi.fillcfcache("+str(allcflist[node])+","+str(ftmname)+","+str(cfcPath)+","+psTermOn+","+aTermOn+","+conjBeams+")";
            # print "CMD = ",node," ",cmd;
            joblist.append(self.PH.runcmd(cmd,node));
        self.PH.checkJobs(joblist);

        # Linear code
        # cfcName = self.allgridpars['0']['cfcache'];
        # cflist=[f for f in os.listdir(cfcName) if re.match(r'CFS*', f)];
        # self.cfcachepars['cflist']=cflist;
        # self.SItool.fillcfcache(**(self.cfcachepars)) ;
#############################################
    def makePSFCore(self):
        ### Make PSFs
        joblist=[]
        #### MPIInterface related changes
        #for node in range(0,self.PH.NN):
        for node in self.listOfNodes:
             joblist.append( self.PH.runcmd("toolsi.makepsf()",node) )
        self.PH.checkJobs( joblist ) # this call blocks until all are done.

#############################################
    def makePBCore(self):
        joblist=[]
        # Only one node needs to make the PB. It reads the freq from the image coordsys
        joblist.append( self.PH.runcmd("toolsi.makepb()",self.listOfNodes[0]) )
        self.PH.checkJobs( joblist )

#############################################

    def runMajorCycleCore(self, lastcycle):
        casalog.post("-----------------------------  Running Parallel Major Cycle ----------------------------","INFO")
        ### Run major cycle
        joblist=[]
        #### MPIInterface related changes
        #for node in range(0,self.PH.NN):
        for node in self.listOfNodes:
             joblist.append( self.PH.runcmd("toolsi.executemajorcycle(controls={'lastcycle':"+str(lastcycle)+"})",node) )
        self.PH.checkJobs( joblist ) # this call blocks until all are done.

#############################################
    def predictModelCore(self):
        joblist=[]
        #### MPIInterface related changes
        #for node in range(0,self.PH.NN):
        for node in self.listOfNodes:
             joblist.append( self.PH.runcmd("toolsi.predictmodel()",node) )
        self.PH.checkJobs( joblist ) # this call blocks until all are done.
