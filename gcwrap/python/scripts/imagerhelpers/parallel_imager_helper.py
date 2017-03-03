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

#############################################
###  Parallel Imager Helper.
#############################################
#casalog.post('Using clustermanager from MPIInterface', 'WARN')
from mpi4casa.MPIInterface import MPIInterface as mpi_clustermanager

class PyParallelImagerHelper():

    def __init__(self):

        ############### Cluster Info
         self.CL=None
         self.sc=None
         self.nodeList=None;
         # Initialize cluster, and partitioning.
        ############### Number of nodes to parallelize on

         # self.nodeList gets filled by setupCluster()
         self.NN = self.setupCluster()

    def getNodeList(self):
        return self.nodeList;

#############################################
    def chunkify(self,lst,n):
        return [ lst[i::n] for i in xrange(n) ]

    def partitionCFCacheList(self,gridPars):

        cfcName = gridPars['cfcache'];
        cflist=[];
        if (not (cfcName == '')):
            cflist=[f for f in os.listdir(cfcName) if re.match(r'CFS*', f)];
        nCF = len(cflist);
        nProcs=len(self.nodeList);
        
        if (nProcs > nCF):
            n=nCF;
        else:
            n=nProcs;
        if (nCF > 0):
            casalog.post("########################################################");
            casalog.post("nCF = " + str(nCF) + " nProcs = " + str(n) + " NodeList=" + str(self.nodeList));
            casalog.post("########################################################");
        xx=self.chunkify(cflist,n);
        allcfs={};
        for i in range(n):
            allcfs[i+1]=xx[i];

        return allcfs;
#############################################
# The above version works better (better balanced chunking).
# Keeping the code below in the file sometime, just in case...(SB).
    # def partitionCFCacheList(self,gridPars):

    #     cfcName = gridPars['cfcache'];
    #     cflist=[];
    #     if (not (cfcName == '')):
    #         cflist=[f for f in os.listdir(cfcName) if re.match(r'CFS*', f)];

    #     nCF = len(cflist);
    #     nProcs=len(self.nodeList);
        
    #     print "########################################################"
    #     print "nCF = ",nCF," nProcs = ",nProcs," NodeList=",self.nodeList;
    #     print "########################################################"

    #     #n0=int(nCF/self.NN);
    #     n0=int(float(nCF)/nProcs+0.5);
    #     if (nProcs >= nCF):
    #         n0 = 1;
    #     allcfs = {};
    #     nUsed=0; i=1;
    #     while (nUsed < nCF):
    #         m = nUsed+n0;
    #         if (m > nCF): 
    #     	m=nCF;
    #         allcfs[i]=cflist[nUsed:m];
    #         nUsed = m;
    #         if (i >= nProcs):
    #             break;
    #         i=i+1;
    #     if (nUsed < nCF):
    #         allcfs[nProcs].append(cflist[i]);
    #     return allcfs;
            
#############################################
## Very rudimentary partitioning - only for tests. The actual code needs to go here.
    def partitionContDataSelection(self,oneselpars={}):

        synu = casac.synthesisutils()
        allselpars =  synu.contdatapartition( oneselpars , self.NN )
        synu.done()

        print 'Partitioned Selection : ', allselpars
        return allselpars

#############################################
## Very rudimentary partitioning - only for tests. The actual code needs to go here.
    def partitionCubeDataSelection(self,oneselpars={}):

        synu = casac.synthesisutils()
        allselpars =  synu.cubedatapartition( oneselpars , self.NN )
        synu.done()

        print 'Partitioned Selection : ', allselpars
        return allselpars

#############################################
    def partitionCubeDeconvolution(self,impars={}):

        synu = casac.synthesisutils()
        allimpars =  synu.cubeimagepartition( impars , self.NN )
        synu.done()

        print 'ImSplit : ', allimpars
        return allimpars

#############################################
    def partitionCubeSelection(self, oneselpars={}, oneimpars={}):
        incsys = oneimpars['csys']
        nchan = oneimpars['nchan']
        synu = casac.synthesisutils()
        allpars = synu.cubedataimagepartition(oneselpars, incsys, self.NN, nchan)
        synu.done()

        #print "Cube Data/Im partitioned selection :", allpars
        return allpars

#############################################
    def setupCluster(self):
        # Initialize cluster

        # * Terminal: Client logs + Server logs
        # * casapy-<timestamp>.log: Client logs
        # * casapy-<timestamp>.log-server-<rank>-host-<hostname>-pid-<pid>: Server logs 
        mpi_clustermanager.set_log_mode('redirect');

        self.sc=mpi_clustermanager.getCluster()
        self.sc.set_log_level('DEBUG')

        self.CL=self.sc._cluster
        self.nodeList = self.CL.get_engines();
        numproc=len(self.CL.get_engines())
        numprocperhost=len(self.nodeList)/len(self.nodeList) if (len(self.nodeList) >0 ) else 1

        owd=os.getcwd()
        self.CL.pgc('import os')
        self.CL.pgc('from numpy import array,int32')
        self.CL.pgc('os.chdir("'+owd+'")')
        os.chdir(owd)
        print "Setting up ", numproc, " engines."
        return numproc

#############################################
    def takedownCluster(self):
        # Check that all nodes have returned, before stopping the cluster
         self.checkJobs()
         print 'Ending use of cluster, but not closing it. Call clustermanager.stop_cluster() to close it if needed.'
#         self.sc.stop_cluster()
         self.CL=None
         self.sc=None

#############################################
    # This is a blocking call that will wait until jobs are done.
    def checkJobs(self,joblist=[]):
        #### MPIInterface related changes
        numcpu = len(self.nodeList)
        
        if len(joblist)==0:
             joblist = range(numcpu)
             #for k in range(numcpu):
             for k in self.nodeList:
                 joblist[k-1] = self.CL.odo('casalog.post("node '+str(k)+' has completed its job")', k)

        print 'Blocking for nodes to finish'
        over=False
        while(not over):
            overone=True
            time.sleep(1)
            for k in range(len(joblist)):
                try:
                    overone =  self.CL.check_job(joblist[k],False) and overone
                except Exception,e:
                     raise Exception(e)
            over = overone
        print '...done'

#############################################
    def runcmd(self, cmdstr="", node=-1):
         if node >= 0:
              return self.CL.odo( cmdstr , node)
         else:
              self.CL.pgc( cmdstr )

#############################################
    def runcmdcheck(self, cmdstr):
         joblist=[]
         #### MPIInterface related changes
         #for node in range(0,self.NN):
         for node in self.nodeList:
              joblist.append( self.CL.odo( cmdstr, node ) )
         self.checkJobs( joblist )

#############################################
    def pullval(self, varname="", node=0):
         return self.CL.pull( varname , node )

#############################################
    def pushval(self, varname="", val=None, node=0):
         return self.CL.push( varname , val, node )

#############################################
    def getpath(self, node=0):
        enginepath = self.sc.get_engine_store(node)
        if enginepath==None:
            return ""
        else:
            return enginepath
#############################################
#    def deletepartimages(self, dirname, imname):
#        namelist = shutil.fnmatch.filter( os.listdir(dirname), imname+".*" )
#        #print "Deleting : ", namelist, ' from ', dirname, ' starting with ', imname
#        for aname in namelist:
#            shutil.rmtree( dirname + "/" + aname )
#############################################
    def deletepartimages(self, imagename, node):
        namelist = shutil.fnmatch.filter( os.listdir(self.getworkdir(imagename, node)), "*" )
        #print "Deleting : ", namelist, ' from ', dirname, ' starting with ', imname
        for aname in namelist:
              shutil.rmtree( self.getworkdir(imagename, node) + "/" + aname )
#############################################
    def getworkdir(self, imagename, nodeid):
        workdir = ''
        workdir = self.getpath(nodeid) + '/' + imagename+'.workdirectory'

        if( not os.path.exists(workdir) ):
            os.mkdir( workdir )

        return workdir
                                    
#############################################
    def getpartimagename(self, imagename, nodeid):
        return self.getworkdir(imagename,nodeid) + '/' + imagename + '.n'+str(nodeid)

#############################################


