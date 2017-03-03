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
An implementation of parallel cube imaging, using synthesisxxxx tools.

Major and minor cycles are parallelized across frequency, by running separate
PySynthesisImagers independently per frequency chunk.
Iteration control is not synchronized,  interactive mask drawing can't be done.
Reference concatenation of all the image products is done at the end.

There are N PySynthesisImager objects, each with their own 
synthesisimager, deconvolvers, normalizers and iterbot. 
   
'''

#############################################
# Parallelize both the major and minor cycle for Cube imaging
# Run a separate instance of PySynthesisImager on each node.
#### ( later, use the live-object interface of ImStore to reference-break the cubes )
#### For nprocesses > nnodes, run the whole 'clean' loop multiple times. 
#############################################
class PyParallelCubeSynthesisImager():

    def __init__(self,params=None):

        self.params=params

        allselpars = params.getSelPars()
        allimagepars = params.getImagePars()
        self.allinimagepars = copy.deepcopy(allimagepars)
        self.allgridpars = params.getGridPars()
        self.allnormpars = params.getNormPars()
        self.weightpars = params.getWeightPars()
        self.decpars = params.getDecPars()
        self.iterpars = params.getIterPars()
        alldataimpars={}
         
        self.PH = PyParallelImagerHelper()
        self.NN = self.PH.NN
        self.NF = len(allimagepars.keys())
        self.listOfNodes = self.PH.getNodeList();
        ## Partition both data and image coords the same way.
        #self.allselpars = self.PH.partitionCubeDataSelection(allselpars)
        #self.allimpars = self.PH.partitionCubeDeconvolution(allimagepars)

        # to define final image coordinates, run selecdata and definemage
        self.SItool = casac.synthesisimager()
        #print "allselpars=",allselpars
        for mss in sorted( allselpars.keys() ):
            self.SItool.selectdata( allselpars[mss] )
        for fid in sorted( allimagepars.keys() ):
            self.SItool.defineimage( allimagepars[fid], self.allgridpars[fid] )
            # insert coordsys record in imagepars 
            # partionCubeSelection works per field ...
            allimagepars[fid]['csys'] = self.SItool.getcsys()
            if allimagepars[fid]['nchan'] == -1:
                allimagepars[fid]['nchan'] = self.SItool.updatenchan()
            alldataimpars[fid] = self.PH.partitionCubeSelection(allselpars,allimagepars[fid])

        #print "********************** ", alldataimpars.keys()
        #for kk in alldataimpars.keys():
        #    print "KEY : ", kk , " --->", alldataimpars[kk].keys()

        # reorganize allselpars and allimpars for partitioned data        
        synu = casac.synthesisutils()
        self.allselpars={}
        self.allimpars={}
        ###print "self.listOfNodes=",self.listOfNodes
        # Repack the data/image parameters per node
        #  - internally it stores zero-based node ids
        #  
        for ipart in self.listOfNodes:
            # convert to zero-based indexing for nodes
            nodeidx = str(ipart-1)
            tnode = str(ipart)
            selparsPerNode= {tnode:{}}
            imparsPerNode= {tnode:{}}
            for fid in allimagepars.iterkeys():
                for ky in alldataimpars[fid][nodeidx].iterkeys():
                    selparsPerNode[tnode]={}
                    if ky.find('ms')==0:
                        # data sel per field
                        selparsPerNode[tnode][ky] = alldataimpars[fid][nodeidx][ky].copy();
                        if alldataimpars[fid][nodeidx][ky]['spw']=='-1':
                            selparsPerNode[tnode][ky]['spw']=''
                        else: 
                            # remove chan selections (will be adjusted by tuneSelectData)
                            newspw=selparsPerNode[tnode][ky]['spw']
                            newspwlist = newspw.split(',')
                            spwsOnly = ''
                            for sp in newspwlist:
                                if spwsOnly!='': spwsOnly+=','
                                spwsOnly+=sp.split(':')[0]   
                            selparsPerNode[tnode][ky]['spw']=spwsOnly

                imparsPerNode[tnode][fid] = allimagepars[fid].copy()
                imparsPerNode[tnode][fid]['csys'] = alldataimpars[fid][nodeidx]['coordsys'].copy()
                imparsPerNode[tnode][fid]['nchan'] = alldataimpars[fid][nodeidx]['nchan']
##                imparsPerNode[tnode][fid]['imagename'] = imparsPerNode[tnode][fid]['imagename'] + '.n'+str(tnode) 
                imparsPerNode[tnode][fid]['imagename'] = self.PH.getpartimagename( imparsPerNode[tnode][fid]['imagename'], ipart )

                # skip this for now (it is not working properly, but should not affect results without this)
                #imparsPerNode[tnode][fid]=synu.updateimpars(imparsPerNode[tnode][fid])
            self.allselpars.update(selparsPerNode)
            self.allimpars.update(imparsPerNode)


        #print "****** SELPARS in init **********", self.allselpars
        #print "****** SELIMPARS in init **********", self.allimpars
        
        joblist=[]
        #### MPIInterface related changes
        #for node in range(0,self.NN):
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("from refimagerhelper import ImagerParameters, PySynthesisImager", node) )
        self.PH.checkJobs( joblist )

        joblist=[]
        #### MPIInterface related changes
        #for node in range(0,self.NN):
        for node in self.listOfNodes:

            joblist.append( self.PH.runcmd("paramList = ImagerParameters()", node) )
            joblist.append( self.PH.runcmd("paramList.setSelPars("+str(self.allselpars[str(node)])+")", node) )
            joblist.append( self.PH.runcmd("paramList.setImagePars("+str(self.allimpars[str(node)])+")", node) )

            joblist.append( self.PH.runcmd("paramList.setGridPars("+str(self.allgridpars)+")", node) )
            joblist.append( self.PH.runcmd("paramList.setWeightPars("+str(self.weightpars)+")", node) )
            joblist.append( self.PH.runcmd("paramList.setDecPars("+str(self.decpars)+")", node) )
            joblist.append( self.PH.runcmd("paramList.setIterPars("+str(self.iterpars)+")", node) )
            joblist.append( self.PH.runcmd("paramList.setNormPars("+str(self.allnormpars)+")", node) )

            joblist.append( self.PH.runcmd("paramList.checkParameters()", node) )

            joblist.append( self.PH.runcmd("imager = PySynthesisImager(params=paramList)", node) )

        self.PH.checkJobs( joblist )

        IBtool = None

    def initializeImagers(self):
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("imager.initializeImagers()", node) )
        self.PH.checkJobs( joblist )

    def initializeDeconvolvers(self):
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("imager.initializeDeconvolvers()", node) )
        self.PH.checkJobs( joblist )

    def initializeNormalizers(self):
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("imager.initializeNormalizers()", node) )
        self.PH.checkJobs( joblist )

    def setWeighting(self):
        ## Set weight parameters and accumulate weight density (natural)
        joblist=[];
        for node in self.listOfNodes:
            ## Set weighting pars
            joblist.append( self.PH.runcmd("imager.setWeighting()", node ) )
        self.PH.checkJobs( joblist )


    def initializeIterationControl(self):

        ## Master iterbot.
        self.IBtool = casac.synthesisiterbot()
        itbot = self.IBtool.setupiteration(iterpars=self.iterpars)

        ## Iterbots per frequency chunk.
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("imager.initializeIterationControl()", node) )
        self.PH.checkJobs( joblist )

    def makePSF(self):
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("imager.makePSF()", node) )
        self.PH.checkJobs( joblist )

    def runMajorMinorLoops(self):
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("imager.runMajorMinorLoops()", node) )
        self.PH.checkJobs( joblist )

    def runMajorCycle(self):
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("for immod in range(0,imager.NF):imager.PStools[immod].dividemodelbyweight();imager.PStools[immod].scattermodel()", node) )   # scattermodel is a no-op here. Ignore ? 
        self.PH.checkJobs( joblist )

        if self.IBtool != None:
            lastcycle = (self.IBtool.cleanComplete(lastcyclecheck=True) > 0)
        else:
            lastcycle = True

        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("imager.runMajorCycleCore("+str(lastcycle)+")", node) )
        self.PH.checkJobs( joblist )

        if self.IBtool != None:
            self.IBtool.endmajorcycle()

        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("for immod in range(0,imager.NF):imager.PStools[immod].gatherresidual();imager.PStools[immod].divideresidualbyweight();imager.PStools[immod].multiplemodelbyweight()", node) )   # gathermodel is a no-op here. Ignore ? 
        self.PH.checkJobs( joblist )



    def runMinorCycle(self):

        # Get iteration control parameters
        iterbotrec = self.IBtool.getminorcyclecontrols()

        #Push this iterbotrec to all nodes ( CHECK SYNTAX HERE )
        for node in self.listOfNodes:
             inrecs[str(node)] = self.PH.pushval("iterbotrec", iterbotrec, node ) ## self.iterbotrec?

        # Exec minor cycle on all nodes
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("execrecs=[]", node) )
            joblist.append( self.PH.runcmd("for immod in range(0,imager.NF):execrecs.append(imager.SDtools[immod].executeminorcycle(iterbotrecord=iterbotrec))", node) )
        self.PH.checkJobs( joblist )


        # Gather all the execrecs here and merge into local IBtool
        exrecs={}
        for node in self.listOfNodes:
             exrecs[str(node)] = self.PH.pullval("execrecs", node )

        # Merge into local IBtool
        for node in self.listOfNodes:
            for arec in exrecs[str(node)]:
                self.IBtool.mergexecrecord(arec)


        for immod in range(0,self.NF):
                if os.environ.has_key('SAVE_ALL_AUTOMASKS') and os.environ['SAVE_ALL_AUTOMASKS']=="true":
                    maskname = self.allimpars[str(immod)]['imagename']+'.mask'
                    tempmaskname = self.allimpars[str(immod)]['imagename']+'.autothresh'+str(self.ncycle)
                    if os.path.isdir(maskname):
                        shutil.copytree(maskname, tempmaskname)
                
                # Some what duplicated as above but keep a copy of the previous mask
                # for interactive automask to revert to it if the current mask
                # is not used (i.e. reached deconvolution stopping condition).
                if self.iterpars['interactive'] and self.alldecpars[str(immod)]['usemask']=='auto-thresh':
                    maskname = self.allimpars[str(immod)]['imagename']+'.mask'
                    prevmaskname=self.allimpars[str(immod)]['imagename']+'.prev.mask'
                    if os.path.isdir(maskname):
                        if os.path.isdir(prevmaskname):
                            shutil.rmtree(prevmaskname)
                        shutil.copytree(maskname, prevmaskname)


    ## Merge the results from all pieces. Maintain an 'active' list of nodes...
    def hasConverged(self):

        # Make initrecs from all deconvolvers
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("initrecs=[]", node) )
            joblist.append( self.PH.runcmd("for immod in range(0,imager.NF):imager.initrecs.append(imager.SDtools[immod].initminorcycle())", node) )
        self.PH.checkJobs( joblist )


        # Gather all the initrecs here and merge into local IBtool
        inrecs={}
        for node in self.listOfNodes:
             inrecs[str(node)] = self.PH.pullval("initrecs", node )

        # Merge into local IBtool
        for node in self.listOfNodes:
            for arec in inrecs[str(node)]:
                self.IBtool.mergeinitrecord(arec)

        # Virtual concatenation or residual (and mask?)
        self.concatimages(type='virtualcopy', imtypes=['residual','mask'])
                
        # Run interactive GUI
        self.runInteractiveGUI2()

        # Send modified mask back to individual nodes (reverse of concatimages...)

        # Check with the iteration controller about convergence.
        stopflag = self.IBtool.cleanComplete()
        #print 'Converged : ', stopflag


        if( stopflag>0 ):
             #stopreasons = ['iteration limit', 'threshold', 'force stop','no change in peak residual across two major cycles']
             stopreasons = ['iteration limit', 'threshold', 'force stop','no change in peak residual across two major cycles', 'peak residual increased by more than 5 times from the previous major cycle','peak residual increased by more than 5 times from the minimum reached']
             casalog.post("Reached global stopping criterion : " + stopreasons[stopflag-1], "INFO")

             # revert the current automask to the previous one 
             if self.iterpars['interactive']:
                 for immod in range(0,self.NF):
                     if self.alldecpars[str(immod)]['usemask']=='auto-thresh':
                        prevmask = self.allimpars[str(immod)]['imagename']+'.prev.mask'
                        if os.path.isdir(prevmask):
                          shutil.rmtree(self.allimpars[str(immod)]['imagename']+'.mask')
                          #shutil.copytree(prevmask,self.allimpars[str(immod)]['imagename']+'.mask')
                          shutil.move(prevmask,self.allimpars[str(immod)]['imagename']+'.mask')

        return (stopflag>0)

    def predictModel(self):
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("imager.predictmodel()", node) )
        self.PH.checkJobs( joblist )

    def restoreImages(self):
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("imager.restoreImages()", node) )
        self.PH.checkJobs( joblist )

    def pbcorImages(self):
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("imager.pbcorImages()", node) )
        self.PH.checkJobs( joblist )

    def makePB(self):
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("imager.makePB()", node) )
        self.PH.checkJobs( joblist )

    def concatImages(self, type='virtualnomove',imtypes=[]):
        import subprocess
        if len(imtypes)==0:
            imtypes=['image','psf','model','residual','mask','pb', 'image.pbcor', 'weight', 'sumwt']
        for immod in range(0,self.NF):
            for ext in imtypes:
                subimliststr="'"
                concatimname=self.allinimagepars[str(immod)]['imagename']+'.'+ ext
                distpath = os.getcwd()
                fullconcatimname = distpath+'/'+concatimname
                for node in self.listOfNodes:
                    #rootimname=self.allinimagepars[str(immod)]['imagename']+'.n'+str(node)
                    #fullimname =  self.PH.getpath(node) + '/' + rootimname 
                    fullimname = self.PH.getpartimagename( self.allinimagepars[str(immod)]['imagename']  , node )
                    if (os.path.exists(fullimname+'.'+ext)):
                        subimliststr+=fullimname+'.'+ext+' '
                subimliststr+="'"
                if subimliststr!="''":
                    # parent images need to be cleaned up for restart=T
                    if self.allinimagepars[str(immod)]['restart'] and os.path.exists(fullconcatimname):
                        try:
                            casalog.post("Cleaning up the existing "+fullconcatimname,"DEBUG")
                            shutil.rmtree(fullconcatimname)
                        except:
                            casalog.post("Cleaning up the existing file named "+fullconcatimname,"DEBUG")
                            os.remove(fullconcatimname)
                    cmd = 'imageconcat inimages='+subimliststr+' outimage='+"'"+fullconcatimname+"'"+' type='+type      
                    # run virtual concat
                    ret=os.system(cmd)
                    if ret!=0:
                        casalog.post("concatenation of "+concatimname+" failed","WARN")
             


    def getSummary(self):
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("summ = imager.getSummary("+str(node)+")", node) )
        self.PH.checkJobs( joblist )

        fullsumm={}
        for node in self.listOfNodes:
             summ = self.PH.pullval("summ", node )
             fullsumm["node"+str(node)] = summ

        return fullsumm

    def deleteTools(self):
        joblist=[]
        for node in self.listOfNodes:
            joblist.append( self.PH.runcmd("imager.deleteTools()", node) )
        self.PH.checkJobs( joblist )

        ## Delete Master iterbot
        if self.IBtool != None:
            self.IBtool.done()

#############################################
