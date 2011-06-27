from parallel_go import *

import os
import math
import random
import time
import shutil
import thread
import commands
import numpy as np
from taskinit import *
import pylab as pl
#import plot_resource as prs
 
from tasksinfo import *
import scipy as sp
import traceback
#from casa import *

#im,cb,ms,tb,fg,af,me,ia,po,sm,cl,cs,rg,sl,dc,vp=gentools()


class simple_cluster:
    '''The simple_cluster creates and maintains an ipcluster environment
    for controlling parallel execution of casa tasks (tools and scripts)
    '''
    def __init__(self):
        self._project=""
        self._hosts=[]
        self._jobs={}
        self._rsrc={}
        self._job_title=1
        self._monitor_on=True
        self._resource_on=True
        self._configdone=False
        self._cluster=cluster()
    
    ###########################################################################
    ###   cluster verifiction
    ###########################################################################
    def config_cluster(self, cfg, force=False):
        '''Read the configuration file and validate cluster definitions.  

        Keyword arguments:
        cfg -- the name of cluster configuration file
        force -- whether or not to reconfigure if a configured cluster exists

        A configuration file is an ASCII text file. Each line defines a node
        (also called host) with comma separated entries of host_name,
        number_of_engines and work_dir. A line that starts with # is ignored.
        The following is the contents of a configuration file:
        ############################################
        casa-dev-07, 4, /home/casa-dev-07/hye/ptest
        #casa-dev-08, 4, /home/casa-dev-08/hye/ptest
        casa-dev-10, 4, /home/casa-dev-10/hye/ptest
        ############################################

        Normally, one does not call this function directly. The init_cluster
        function will trigger this function.

        '''
        if (len(self._hosts)>0 or self._configdone) and not force:
            print 'cluster already configured'
            return
        self._hosts=[]
        self._jobs={}
        self._rsrc={}
        input=open(cfg,'r')
        s=input.readlines()
        for line in s:
            sLine=line.rstrip()
            if str.find(sLine, '#')==0:
                continue
            #print sLine
            xyzd=str.split(sLine, ',')
            #print xyzd, len(xyzd)
            if len(xyzd)<3:
                print 'node config should be: "hostname, numengine, workdir"'
                print '           instead of:', sLine 
                print 'this entry will be ignored'
                continue
            try:
                a=int(xyzd[1])
            except:
                print 'the numofenging should be a integer instead of:', xyzd[1]
                print 'this entry will be ignored'
                continue
            [a, b, c]=[str.strip(xyzd[0]), int(xyzd[1]), str.strip(xyzd[2])]
            if len(a)<1:
                print 'the hostname can not be empty'
                print 'this entry will be ignored'
                continue
            if len(c)<1:
                print 'the workdir can not be empty'
                print 'this entry will be ignored'
                continue
            self._hosts.append([a, b, c])
        for i in range(len(self._hosts)):
            self._rsrc[self._hosts[i][0]]=[]
        #print self._hosts
    
        self._configdone=self.validate_hosts()
        if not self._configdone:
            print 'failed to config the cluster'
        if self._project=="":
            #print 'project name not set'
            pass
    
    def validate_hosts(self):
        '''Validate the cluster specification.

        This function is normally called internally by configure_cluster
        function. 

        '''
        uhost=set()
        for i in range(len(self._hosts)):
            uhost.add(self._hosts[i][0])
        if len(uhost)==0:
            print 'configuration table is empty'
            return False
        if len(uhost)<len(self._hosts):
            print 'configuration table contains repeated node name'
            return False
        for i in range(len(self._hosts)):
            if type(self._hosts[i][1])!=int:
                print 'the number of engines must be an integer'
                return False
        for i in range(len(self._hosts)):
            if not os.path.exists(self._hosts[i][2]):
                print 'the directory "%s" does not exist' % self._hosts[i][2]
                return False
        for i in range(len(self._hosts)):
            try:
                tfile=self._hosts[i][2]+'/nosuchfail'
                f = open(tfile, 'w')
                os.remove(tfile)
            except IOError:
                print 'no writhe permision in directory "%s"' % \
                       self._hosts[i][2]
                return False
        return True
    
    
    ###########################################################################
    ###   project management
    ###########################################################################
    def create_project(self, proj=""):
        '''Create a project. 

        Keyword arguments:
        proj -- the name of project (default: 'proj'+timestamp).  

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        This function is normally called internally by init_cluster function. 

        Example:
        CASA <33>: sl.list_projects
        host: casa-dev-07 ------------------------>>>>
        bProj  bsplit  csplit  my_project
        host: casa-dev-08 ------------------------>>>>
        bProj  bsplit  csplit  my_project
        host: casa-dev-10 ------------------------>>>>
        bProj  bsplit  csplit

        CASA <34>: sl.create_project('dflag')
        output directory:
        /home/casa-dev-07/hye/ptest/dflag
        /home/casa-dev-08/hye/ptest/dflag
        /home/casa-dev-10/hye/ptest/dflag

        CASA <36>: sl.list_projects
        host: casa-dev-07 ------------------------>>>>
        bProj  bsplit  csplit  dflag  my_project
        host: casa-dev-08 ------------------------>>>>
        bProj  bsplit  csplit  dflag  my_project
        host: casa-dev-10 ------------------------>>>>
        bProj  bsplit  csplit  dflag

        '''
        if not self._configdone:
            return
        if type(proj)!=str:
            print 'project name must be string'
            return
    
        self._project=proj.strip()
        if self._project=="":
            tm=time.strftime("%Y%b%d-%Hh%Mm%Ss", time.localtime())
            self._project='proj'+tm
    
        for i in range(len(self._hosts)):
            if not os.path.exists(self._hosts[i][2]+'/'+proj.strip()):
                #print self._hosts[i][2]+'/'+self._project
                cmd='mkdir '+self._hosts[i][2]+'/'+self._project
                os.system(cmd)
        print 'output directory: '
        for i in range(len(self._hosts)):
            print self._hosts[i][2]+'/'+self._project
    
    def do_project(self, proj):
        '''Use a project previously created. 

        Keyword arguments:
        proj -- the name of project.

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        Example:
        CASA <38>: sl._project
        Out[38]: 'dflag'

        CASA <39>: sl.do_project('csplit')
        output directory:
        /home/casa-dev-07/hye/ptest/csplit
        /home/casa-dev-08/hye/ptest/csplit
        /home/casa-dev-10/hye/ptest/csplit

        CASA <40>: sl._project
        Out[40]: 'csplit'

        '''
        if not self._configdone:
            return
        if type(proj)!=str or proj.strip()=="":
            print 'project name must be a nonempty string'
            return
    
        projexist=True
        for i in range(len(self._hosts)):
            if not os.path.exists(self._hosts[i][2]+'/'+proj.strip()):
                print 'no project directory found on '+self._hosts[i][0]
                projexist=False
        if projexist:
            self._project=proj.strip()
        print 'output directory: '
        for i in range(len(self._hosts)):
            print self._hosts[i][2]+'/'+self._project
    
    def erase_project(self, proj):
        '''Erase files and dirs of a project. 

        Keyword arguments:
        proj -- the name of project.

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        Example:
        CASA <30>: sl.list_projects
        host: casa-dev-07 ------------------------>>>>
        aNew  bProj  bsplit  csplit  my_project
        host: casa-dev-08 ------------------------>>>>
        aNew  bProj  bsplit  csplit  my_project
        host: casa-dev-10 ------------------------>>>>
        bProj  bsplit  csplit

        CASA <31>: sl.erase_project('aNew')

        CASA <32>: sl.list_projects
        host: casa-dev-07 ------------------------>>>>
        bProj  bsplit  csplit  my_project
        host: casa-dev-08 ------------------------>>>>
        bProj  bsplit  csplit  my_project
        host: casa-dev-10 ------------------------>>>>
        bProj  bsplit  csplit

        '''
        if not self._configdone:
            return
        if type(proj)!=str or proj.strip()=="":
            print 'project name must be a nonempty string'
            return
        for i in range(len(self._hosts)):
            cmd='rm -rf '+self._hosts[i][2]+'/'+proj
            os.system(cmd)
        if proj==self._project:
            self._project=""
        if self._project=="":
            #print 'project name not set'
            pass
    
    def clear_project(self, proj):
        '''Remove all previous results of the proj

        Keyword arguments:
        proj -- the name of project.

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        Example:
        CASA <27>: sl.list_project('my_project')
        host: casa-dev-07 ------------------------>>>>
        casapy-20101122165601-5.log  sim.alma.csv.mid-f0-s0-b640-e768.flux
        host: casa-dev-08 ------------------------>>>>
        casapy-20101122165601-6.log  sim.alma.csv.mid-f0-s0-b768-e896.flux
        host: casa-dev-10 ------------------------>>>>
        casapy-20101122165601-7.log  sim.alma.csv.mid-f0-s0-b320-e640.flux

        CASA <28>: sl.clear_project('my_project')

        CASA <29>: sl.list_project('my_project')
        host: casa-dev-07 ------------------------>>>>
        host: casa-dev-08 ------------------------>>>>
        host: casa-dev-10 ------------------------>>>>

        '''
        #this can be a slow operation, it is better to do it parallel
        if not self._configdone:
            return
        if type(proj)!=str or proj.strip()=="":
            print 'project name must be a nonempty string'
            return
        for i in range(len(self._hosts)):
            cmd='rm -rf '+self._hosts[i][2]+'/'+proj.strip()+'/*'
            os.system(cmd)
    
    def list_project(self, proj):
        '''List previous results of the proj

        Keyword arguments:
        proj -- the name of project.

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        Example:
        CASA <19>: sl.list_project('bsplit')
        host: casa-dev-07 ------------------------>>>>
        test_regression_TDEM0003-f0-s11.ms   test_regression_TDEM0003-f0-s13.ms
        host: casa-dev-08 ------------------------>>>>
        test_regression_TDEM0003-f0-s10.ms  test_regression_TDEM0003-f3-s9.ms
        host: casa-dev-10 ------------------------>>>>
        test_regression_TDEM0003-f0-s0.ms   test_regression_TDEM0003-f4-s11.ms

        '''
        if not self._configdone:
            return
        if type(proj)!=str or proj.strip()=="":
            print 'project name must be a nonempty string'
            return
        for i in range(len(self._hosts)):
            print 'host:', self._hosts[i][0], '------------------------>>>>'
            cmd='ls '+self._hosts[i][2]+'/'+proj
            os.system(cmd)
    
    def erase_projects(self):
        '''Erase all previous results of all projects

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        '''
        if not self._configdone:
            return
        for i in range(len(self._hosts)):
            cmd='rm -rf '+self._hosts[i][2]+'/*'
            os.system(cmd)
        self._project=""
        #print 'project name not set'
    
    def list_projects(self):
        '''List all previous projects

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        Example:
        <CASA 16:>sl.list_projects
        host: casa-dev-07 ------------------------>>>>
        aNew  bProj  bsplit  csplit  my_project
        host: casa-dev-08 ------------------------>>>>
        aNew  bProj  bsplit  csplit  my_project
        host: casa-dev-10 ------------------------>>>>
        bProj  bsplit  csplit

        '''
        if not self._configdone:
            return
        for i in range(len(self._hosts)):
            print 'host:', self._hosts[i][0], '------------------------>>>>'
            cmd='ls '+self._hosts[i][2]+'/' 
            os.system(cmd)
    
    def reset_project(self):
        '''Erase previous result and reset the status current project.

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        Example:
        CASA <43>: sl.list_project('bProj')
        ....too many here...

        CASA <44>: sl.do_project('bProj')
        output directory:
        /home/casa-dev-07/hye/ptest/bProj
        /home/casa-dev-08/hye/ptest/bProj
        /home/casa-dev-10/hye/ptest/bProj

        CASA <45>: sl.list_project('bProj')
        host: casa-dev-07 ------------------------>>>>
        host: casa-dev-08 ------------------------>>>>
        host: casa-dev-10 ------------------------>>>>

        '''
        if not self._configdone:
            return
        self.stop_monitor()
        self.clear_project(self._project)
        self._jobs={}
        self._job_title=1
        self._monitor_on=True
    
    def get_hosts(self):
        '''List current cluster.

        CASA <48>: sl.get_hosts
        Out[48]:
        [['casa-dev-07', 4, '/home/casa-dev-07/hye/ptest'],
         ['casa-dev-08', 4, '/home/casa-dev-08/hye/ptest'],
         ['casa-dev-10', 4, '/home/casa-dev-10/hye/ptest']]

        '''
        if not self._configdone:
            return
        return self._hosts 
    
    ###########################################################################
    ###   cluster management
    ###########################################################################
    def cold_start(self):
        '''kill all engines on all hosts. Shutdown current cluster.
        
        This is used if a complete restart of the cluster is needed. One can
        rerun init_cluster after this. This also kills possible leftover
        engines from previous sessions.

        '''
        if not self._configdone:
            return
        for i in range(len(self._hosts)):
            cmd='ssh '+self._hosts[i][0]+' "killall -9 ipengine"'
            os.system(cmd)
        self._cluster.stop_cluster()
    
    def stop_nodes(self):
        '''Stop all engines on all hosts of current cluster.
        
        After running this, the cluster contains no engines.

        '''
        if not self._configdone:
            return
        for i in self._cluster.get_nodes():
            self._cluster.stop_node(i)
    
    def start_cluster(self):
        '''Start a cluster with current configuration.
        
        Normally, one does not need to run this function directly. The 
        init_cluster will call this internally.

        '''
        if not self._configdone:
            return
        for i in range(len(self._hosts)):
            self._cluster.start_engine(self._hosts[i][0], self._hosts[i][1], 
                                   self._hosts[i][2]+'/'+self._project)
    
    def get_host(self, id):
        '''Find out the name of the node that hosts this engine.

        Keyword arguments:
        id -- the engine id

        Example:
        CASA <50>: sl.get_host(8)
        Out[50]: 'casa-dev-10'

        '''
        if not self._configdone:
            return
        ids=self._cluster.get_ids()
        if type(id)!=int:
            print 'the argument must be an engine id (int)'
            return ''
        if ids.count(id)!=1:
            print 'engine %d does not exist' % id
            return ''
        e=self._cluster.get_engines()
        for i in range(len(e)):
            if e[i][0]==id:
                return e[i][1] 
    
    def get_engine_store(self, id):
        '''Get the root path where an engine writes out result

        Keyword arguments:
        id -- the engine id

        Example:
        CASA <52>: sl.get_engine_store(8)
        Out[52]: '/home/casa-dev-10/hye/ptest/bProj/'

        '''
        if not self._configdone:
            return
        hst=self.get_host(id)
        for i in range(len(self._hosts)):
            if self._hosts[i][0]==hst:
                pth=self._hosts[i][2]
                sl=''
                if not pth.endswith('/'):
                    sl='/' 
                return pth+sl+self._project+'/'
    
    ###########################################################################
    ###   log management
    ###########################################################################
    def start_logger(self):
        '''Link all engine logs to the current directory. 

        After running this, the current directory contains links to each of
        the engine logs with file name 'engine-[id].log such that one can 
        conveniently browse engine logs with casa logviewer.

        '''
        if not self._configdone:
            return
        lg=self._cluster.get_casalogs()
        os.system('rm -f engine-*.log')
        for i in lg:
            eng='engine'+i[str.rfind(i,'-'):]
            #if os.path.exists(eng):
            #    os.unlink(eng)
            os.symlink(i, eng)
    
    ###########################################################################
    ###   resource management
    ###########################################################################
    def check_resource(self):
        '''Check the resource usage on all hosts.

        This function checks and calculates current %cpu, %iowait, %mem and 
        %memswap on all hosts. The results are stored internally and can be
        viewed by calling show_resource.

        Normally, one does not call this function directly. The init_cluster
        will call this function.

        '''
        if not self._configdone:
            return
        for i in range(len(self._hosts)):
            cpu=[0.]*11
            b=commands.getoutput("ssh "+self._hosts[i][0]+ 
                                 " 'cat /proc/stat | grep cpu\ '")
            lines=b.split('\n')
            for line in lines:
                if not line.startswith('cpu '):
                    continue
                l=line.split()
                cpu[0]=long(l[1])+long(l[2])+long(l[3])+ \
                       long(l[5])+long(l[6])+long(l[7])+ \
                       long(l[4])
                cpu[1]=long(l[1])+long(l[2])+long(l[3])
                cpu[3]=long(l[5])
    
            s=commands.getoutput("ssh "+self._hosts[i][0]+ 
                                 " 'cat /proc/meminfo '")
            lines=s.split('\n')
            mt=mf=st=sf=0
            for line in lines:
                if line.startswith('MemTotal:'):
                    mt=long(line.split()[1])
                if line.startswith('MemFree:'):
                    mf=long(line.split()[1])
                if line.startswith('SwapTotal:'):
                    st=long(line.split()[1])
                if line.startswith('SwapFree:'):
                    sf=long(line.split()[1])
            cpu[5]=mt
            cpu[6]=mf
            cpu[8]=st
            cpu[9]=sf
    
            #print self._rsrc
            old=self._rsrc[self._hosts[i][0]] if self._rsrc.has_key(self._hosts[i][0]) else [] 
            if old!=None and old!=[]:
                dlt=float(cpu[0]-old[0])
                if math.fabs(dlt)<0.00000001:
                    cpu[2]=0.
                    cpu[4]=0.
                else:
                    cpu[2]=(cpu[1]-old[1])/dlt
                    cpu[4]=(cpu[3]-old[3])/dlt
            try:
                cpu[7]=(cpu[5]-cpu[6])/float(cpu[5])
                cpu[10]=(cpu[8]-cpu[9])/float(cpu[8])
            except:
                pass
            self._rsrc[self._hosts[i][0]]=cpu 
    
    def start_resource(self): 
        '''Start monitoring resource usage.

        Four critical resource usage indicators (for parallel execution), 
        namely, %cpu, %iowait, %mem and %memswap on all hosts are continuously
        checked. This infomation can be used to tune the parallel performance.

        Normally, one does not call this function directly. The init_cluster
        will call this function.

        '''
        
        if not self._configdone:
            return
        #fig=pl.figure(98)
        #title='casa cluster resource usage'
    
        #t=fig.text(0.5, 0.95, title, horizontalalignment='center',
        #           verticalalignment='center',
        #           fontproperties=pl.matplotlib.font_manager.FontProperties(size=16))
        #fig.show() this should be the last and should be the only one, use draw()
        self._resource_on=True 
        self._rsrc={}
        return thread.start_new_thread(self.update_resource, ())
    
    def update_resource(self):
        '''Set up repeated resource checking.

        Four critical resource usage indicators (for parallel execution), 
        namely, %cpu, %iowait, %mem and %memswap on all hosts are continuously
        checked. This infomation can be used to tune the parallel performance.

        Normally, one does not call this function directly. The init_cluster
        will call this function.

        '''
        if not self._configdone:
            return
        while self._resource_on:
            time.sleep(5)
            self.check_resource()
    
    def stop_resource(self): 
        '''Stop monitoring resource usage.

        Four critical resource usage indicators (for parallel execution), 
        namely, %cpu, %iowait, %mem and %memswap on all hosts are continuously
        checked. This infomation can be used to tune the parallel performance.

        Normally, one does not call this function directly. The init_cluster
        will call this function.

        '''
        if not self._configdone:
            return
        self._resource_on=False 
        self._rsrc={}
    
    def show_resource(self, long=False):
        '''Display resource usage on all hosts.

        Keyword arguments:
        long -- whether or not to display detailed resource usage info

        Four critical resource usage indicators (for parallel execution), 
        namely, %cpu, %iowait, %mem and %memswap on all hosts are continuously
        checked. This infomation can be used to tune the parallel performance.

        Example:
        CASA <53>: sl.show_resource
        host          cpu    iowait mem    swap
        casa-dev-08   0.13   0.01   0.80   0.00
        casa-dev-10   0.17   0.01   0.94   0.00
        casa-dev-07   0.15   0.00   0.88   0.00

        '''
        if not self._configdone:
            return
        if long:
            return self._rsrc
        else:
            if len(self._rsrc.keys())==0:
                return self._rsrc
            else:
                print ' host          cpu    iowait mem    swap'
                for h in self._rsrc.keys():
                    print "%12s%7.2f%7.2f%7.2f%7.2f" % (
                           h,
                           self._rsrc[h][2], 
                           self._rsrc[h][4],
                           self._rsrc[h][7], 
                           self._rsrc[h][10]
                           )
    
    ###########################################################################
    ###   execution status management
    ###########################################################################
    def check_job(self):
        '''Check the execution status of current noblock jobs  on all engines.

        This function can be used to block the terminal until all submitted
        jobs finish. 

        Example:
        CASA <2>: from simple_cluster import simple_cluster
        CASA <3>: sl=simple_cluster()
        CASA <4>: sl.init_cluster("my_cluster", "csplit")
        CASA <5>: sl.simple_split('/lustre/casa-store/hye/10B-209a_5s.ms/', '')
        CASA <6>: sl.check_job()

        '''

        if not self._configdone:
            return
        done=False
        while(not done):
            time.sleep(5)
            done=True
            for i in sl._jobs.keys():
                done=self._cluster.check_job(i) and done
    
    def check_status(self, notify=False):
        '''Check the execution status of submitted no-block jobs

        Keyword arguments:
        notify -- whether or not to display detailed resource usage info

        Normally, one does not call this function directly. The start_monitor
        will call this function internally.

        '''

        if not self._configdone:
            return
        while self._monitor_on:
            time.sleep(5)
            curr={}
            try:
                pend=self._cluster.queue_status()
                for i in xrange(len(pend)):
                    (a, b)=pend[i]
                    curr[a]=b['pending']
            except:
                pass
            #print 'curr', curr
            for job in self._jobs.keys():
                if type(job)==type(None):
                    self._jobs[job]['status']="unknown"
                else:
                    eng=self._jobs[job]['engine']
                    sht=self._jobs[job]['short']
                    try:
                        x=1
                        try:
                            x=job.get_result(block=False)
                        except:
                            if notify and self._jobs[job]['status']=="scheduled":
                                print 'engine %d job %s broken' % (eng, sht)
                            self._jobs[job]['status']="broken"
 
                        if x==None:
                            cmd=self._jobs[job]['command']
                            if curr.has_key(eng):
                                wk=eval(curr[eng].lstrip('execute(').rstrip(')'))
                                #print 'wk', wk
                                #print 'cmd', cmd
                                if wk==cmd:
                                    self._jobs[job]['status']="running"
                                    if self._jobs[job]['start']=='':
                                        if notify:
                                            print 'engine %d job %s started'%(eng,sht)
                                        self._jobs[job]['start']=time.time()
                                    self._jobs[job]['time']=time.time()-self._jobs[job]['start'] 
                                else:
                                    pass
                        else:
                            #print 'x=', x
                            if self._jobs[job]['status']=="running":
                                if notify:
                                    print 'engine %d job %s finished' % (eng, sht)
                                self._jobs[job]['status']="done"
                            if self._jobs[job]['status']=="scheduled":
                                if isinstance(x, int):
                                    if notify:
                                        print 'engine %d job %s broken' % (eng, sht)
                                    self._jobs[job]['status']="broken"
                                else:
                                    if notify:
                                        print 'engine %d job %s finished' % (eng, sht)
                                    self._jobs[job]['status']="done"
                    except e:
                        if notify and self._jobs[job]['status']=="running":
                            print 'engine %d job %s broken' % (eng, sht)
                        self._jobs[job]['status']="broken"
            gr=set()
            for val in self._jobs.values():
                gr.add(val['jobgroup'])
    	    for i in list(gr):
                eml=''
                jobname='None'
                if i.count(':')>0:
                    pos=i.find(':')
                    jobname=i[pos+1:].strip()
                    eml=i[:pos].strip()
                elif i.count('@')>0:
                    eml=i.strip()
                else:
                    jobname=i.strip()
                
                finish=True
                for val in self._jobs.values():
                    if val['jobgroup']==i and not (val['status']=='done' or 
                                                val['status']=='broken'):
                        finish=False
                if finish==True:
                    tm=time.ctime()
                    jobname=jobname[0:30]
                    msg='\n#### '+jobname+' '+ \
                          '#'*(68-len(jobname)-len(tm))+' '+tm+' ####'
                    msg+='\nengine    status  time(s)  command\n'
                    rmv=[]
                    for job in self._jobs.keys():
                        if self._jobs[job]['jobgroup']==i:
                            msg+="%6d%10s%9d%2s%s\n" % (self._jobs[job]['engine'], 
                                  self._jobs[job]['status'],
                                  int(self._jobs[job]['time']), 
                                  '  ',
                                  self._jobs[job]['command'])
                            rmv.append(self._jobs[job]['jobname'])
                    #print msg
                    if i.strip()!='':
    
                        #append to project result file
                        f=open(self._project+'.result', 'a')
                        #f.write('\n'+i)
                        f.write(msg)
                        f.write('\n')
                        f.close()
    
                        if eml!='' and eml.count(' ')==0:
                            #send email
                            #import smtplib
                            #from email.mime.text import MIMEText
                            #mal=MIMEText(msg)
                            #mal['Subject']='your parallel job finished'
                            #me=os.environ['USER']
                            #mal['From']=me
                            #mal['To']=i 
                            #s=smtplib.SMTP()
                            #s.connect()
                            #s.sendmail(me, [i], mal.as_string())
                            #s.close()
        
                            msgf='/tmp/emailmsg.txt'
                            f=open(msgf, 'w')
                            f.write(msg)
                            f.write('\n')
                            f.close()
                            cmd='/bin/mail -s "parallel job \''+jobname+\
                                '\' finished" '+eml+' < '+msgf
                            #print cmd
                            os.system(cmd)
                             
                        for j in rmv:
                            self.remove_record(j)
                            
    
    def start_monitor(self): 
        '''Start monitoring  execution status of submitted no-block jobs

        Normally, one does not call this function directly. The init_cluster
        will call this function.

        '''
        if not self._configdone:
            return
        self._monitor_on=True 
        return thread.start_new_thread(self.check_status, (True,))
    
    def stop_monitor(self): 
        '''Stop monitoring execution status of submitted no-block jobs

        Normally, one does not call this function directly. 

        '''
        if not self._configdone:
            return
        self.monitor_on=False 
    
    def show_queue(self):
        '''Display job queue.

        Example:
        CASA <2>: from simple_cluster import simple_cluster
        CASA <3>: sl=simple_cluster()
        CASA <4>: sl.init_cluster("my_cluster", "csplit")
        CASA <5>: sl.simple_split('/lustre/casa-store/hye/10B-209a_5s.ms/', 
                                  'you@nrao.edu:3rd split')
        CASA <6>: sl.show_queue()

        '''
        if not self._configdone:
            return
        return self._cluster.queue_status()
    
    def get_status(self, long=False):
        '''Display job execution status.

        Keyword arguments:
        long -- whether or not to display detailed execution status info

        Example:
        CASA <2>: from simple_cluster import simple_cluster
        CASA <3>: sl=simple_cluster()
        CASA <4>: sl.init_cluster("my_cluster", "csplit")
        CASA <5>: sl.simple_split('/lustre/casa-store/hye/10B-209a_5s.ms/', 
                                  'you@nrao.edu:3rd split')
        CASA <6>: sl.get_status()
        engine    status  time(s)     start  command   title
             0      done       31  16:41:56    split      15
             2 scheduled        0              split      78
             7      done       41  16:42:38    split      16
             9   running       51  16:42:59    split      17
             1      done       36  16:41:56    split      18

        '''

        if not self._configdone:
            return
        if long:
            return self._jobs
        else:
            if len(self._jobs.keys())==0:
                return self._jobs
            else:
                print 'engine    status  time(s)     start  command   title'
                for job in self._jobs.keys():
                    print "%6d%10s%9d%10s%9s%8d" % (self._jobs[job]['engine'], 
                           self._jobs[job]['status'],
                           int(self._jobs[job]['time']), 
                           '' if type(self._jobs[job]['start'])==str 
                              else
                                 time.strftime("%H:%M:%S", 
                                        time.localtime(self._jobs[job]['start'])),
                           self._jobs[job]['short'].strip()[:9],
                           self._jobs[job]['jobname'])

    def get_jobId(self, status):
        '''Get a list of jobs of the given status

        Keyword arguments:
        status -- the job status or the job title

        Example:
        CASA <2>: from simple_cluster import simple_cluster
        CASA <3>: sl=simple_cluster()
        CASA <4>: sl.init_cluster("my_cluster", "csplit")
        CASA <5>: sl.simple_split('/lustre/casa-store/hye/10B-209a_5s.ms/', 
                                  'you@nrao.edu:3rd split')
        CASA <6>: sl.get_jobId('done')

        '''

        if not self._configdone:
            return []
        if len(self._jobs.keys())==0:
            return [] 
        else:
            jobId=[] 
            for job in self._jobs.keys():
                if self._jobs[job]['status']==status:
                    jobId.append(self._jobs[job]['jobname'])
            return jobId
    
    def remove_record(self, jobname):
        '''Remove job execution status of a job.

        Keyword arguments:
        jobname -- the jobname or status of job(s) to be removed from display

        '''

        if not self._configdone:
            return
        for job in self._jobs.keys():
            if type(jobname)==int and self._jobs[job]['jobname']==jobname:
               del self._jobs[job]
            if type(jobname)==str and self._jobs[job]['status']==jobname:
               del self._jobs[job]
    
    ###########################################################################
    ###   job distribution functions
    ###########################################################################
    def make_call(self, func, param):
        '''Make a function call string with function name and parameters.

        Keyword arguments:
        func -- the name of the function
        param -- the dictionary of parameters and values

        Example:
        CASA <12>: param=dict()
        CASA <13>: param['vis']='NGC5921.ms'
        CASA <14>: param['spw']='4'
        CASA <15>: sl.make_call('flagdata', param)
          Out[15]: 'flagdata(vis="NGC5921.ms", spw=4)'

        '''
        if type(func)!=str or type(param)!=dict:
            print 'func must be a str and param must be a dictionary'
            return None 
        cmd=func+'('
        for (k, v) in param.iteritems():
            cmd+=k+'='
            if type(v)==str:
                cmd+='"'+v+'"'
            elif type(v)==np.ndarray:
                cmd+=repr(v)
            else:
                cmd+=str(v)
            cmd+=', '
        cmd=cmd[0:-2]
        cmd+=')'
        return cmd
    
    def do_and_record(self, cmd, id, group=''):
        '''Submit a function call to an engine and record its execution status.

        Keyword arguments:
        cmd -- the function call string
        id -- the id of the engine to be assigned 
        group -- the group this cmd belongs to and the receipt of notification
                 group can be an email address or a label of the job or both
                 separated by a ':'. Once all the jobs that has the same label
                 finish, an email notification will be sent.

        Example:
        CASA <12>: param=dict()
        CASA <13>: param['vis']='NGC5921.ms'
        CASA <14>: param['spw']='4'
        CASA <15>: cmd=sl.make_call('flagdata', param)
        CASA <17>: sl.do_and_record(cmd, 7, 'you@nrao.edu:flag ngc5921')

        '''
        if not self._configdone:
            return
        job=self._cluster.odo(cmd, id)
        self._jobs[job]={}
        self._jobs[job]['start']=''
        self._jobs[job]['time']=0
        self._jobs[job]['command']=cmd
        if len(cmd)<9:
            self._jobs[job]['short']=cmd
        else:
            self._jobs[job]['short']=cmd[:str.find(cmd, '(')]
        self._jobs[job]['status']="scheduled"
        self._jobs[job]['engine']=id
        self._jobs[job]['jobname']=self._job_title
        self._jobs[job]['jobgroup']=''
        self._job_title+=1
        return self._job_title-1
    
    ###########################################################################
    ###   result processing functions
    ###########################################################################
    
    def list_result(self):
        '''read the project.result file and write out all labels

        Example:
        CASA <33>: sl.list_result
        Out[33]:
        ['#### new split ####################### Mon Mar 14 14:48:08 2011 ####',
         '#### flag ngc5921 #################### Wed Mar 16 10:43:12 2011 ####']

        '''
        if not self._configdone:
            return
        f=open(self._project+'.result', 'r')
        s=f.readlines()
        vec=[]
        for line in s:
            sLine=line.rstrip()
            if str.find(sLine, '#### ')==0:
                vec.append(sLine.strip())
            else:
                continue
        f.close()
        return vec
    
    def get_result(self, tm):
        '''read the project.result file and write out result for a label

        Keyword arguments:
        tm -- the result label

        Example:
        CASA <33>: sl.list_result
        Out[33]:
        ['#### new split ####################### Mon Mar 14 14:48:08 2011 ####',
         '#### flag ngc5921 #################### Wed Mar 16 10:43:12 2011 ####']
        CASA <34>: sl.get_result('new split')
        Out[34]:
        ....ommit...

        '''

        if not self._configdone:
            return
        f=open(self._project+'.result', 'r')
        s=f.readlines()
        reach=False
        vec=[]
        for line in s:
            sLine=line.strip()
            if str.find(sLine, '#### ')==0:
                if str.count(sLine, ' '+tm+' ')>0 and reach==False:
                    reach=True
                else:
                    reach=False
            else:
                if reach and sLine!='' and not sLine.startswith('engine'):
                    vec.append(sLine)
        f.close()
        return vec

    def erase_result(self, tm):
        '''read the project.result file and erase result for a label

        Keyword arguments:
        tm -- the result label

        Example:
        CASA <33>: sl.list_result
        Out[33]:
        ['#### new split ####################### Mon Mar 14 14:48:08 2011 ####',
         '#### flag ngc5921 #################### Wed Mar 16 10:43:12 2011 ####']
        CASA <34>: sl.erase_result('flag ngc5921')
        CASA <35>: sl.list_result
        Out[35]:
        ['#### new split ####################### Mon Mar 14 14:48:08 2011 ####']

        '''
        if not self._configdone:
            return
        if type(tm)!=str or len(tm)==tm.count('#'):
            return 
        f=open(self._project+'.result', 'r')
        s=f.readlines()
        f.close()
        a=-1
        b=-1
        for line in xrange(len(s)):
            sLine=s[line].strip()
            if str.find(sLine, '#### ')==0:
                if str.count(sLine, ' '+tm+' ')>0 and a==-1:
                    a=line
                else:
                    b=line
                #if b>0:
                #    break
        if a>-1 and b>a:
            f=open(self._project+'.result', 'w')
            f.writelines(s[:a])
            f.writelines(s[b:])
            f.close()
        return
    
    def get_output(self, result, item, **kwargs):
        '''pick from result list the item that meets condistion in kwargs

        Keyword arguments:
        result -- the result label or the result from running get_result 
        item -- the result item to get
        kwargs -- the conditions to limit the result

        Example:
        CASA <33>: sl.list_result
          Out[33]:
        ['#### new split ####################### Mon Mar 14 14:48:08 2011 ####',
         '#### flag ngc5921 #################### Wed Mar 16 10:43:12 2011 ####']
        CASA <34>: sl.get_result('new split')
          Out[34]:
        ['10      done       30  split(vis="/lustre/casa-store/hye/10B-209a_5s.ms", outputvis="/home/casa-dev-10/hye/ptest/csplit/10B-209a_5s-f5-s10.ms", spw="10", datacolumn="DATA", field="J0738+1742")',
         '1      done       40  split(vis="/lustre/casa-store/hye/10B-209a_5s.ms", outputvis="/home/casa-dev-07/hye/ptest/csplit/10B-209a_5s-f3-s2.ms", spw="2", datacolumn="DATA", field="J0738+1742")',
         '2      done       75  split(vis="/lustre/casa-store/hye/10B-209a_5s.ms", outputvis="/home/casa-dev-07/hye/ptest/csplit/10B-209a_5s-f4-s10.ms", spw="10", datacolumn="DATA", field="2MJ0746")',
         ..... many other entries ...]
        CASA <35>: sl.get_output('new split', 'outputvis', field='3C84')
          Out[35]:
        ['/home/casa-dev-07/hye/ptest/csplit/10B-209a_5s-f1-s8.ms',
         '/home/casa-dev-10/hye/ptest/csplit/10B-209a_5s-f1-s14.ms',
         '/home/casa-dev-08/hye/ptest/csplit/10B-209a_5s-f0-s0.ms']

        '''
        if not self._configdone:
            return
        if type(result)==str:
            result=self.get_result(result)
        if type(result)!=list:
            return []
        if len(result)==0:
            return []
        if type(item)!=str:
            print 'the item name must be a string'
            return []
            print "another keyword arg: %s: %s" % (key, kwargs[key])
    
        vals=[]
        for key in kwargs:
            v=kwargs[key]
            if type(v)==str:
                vals.append(str(key)+'="'+v+'"')
            elif type(v)==np.ndarray:
                vals.append(str(key)+'='+repr(v))
            else:
                vals.append(str(key)+'='+str(v))
        #print vals
    
        vec=[]
        for i in result:
            pick=True
            for j in vals:
                if str.count(i, j)==0:
                    pick=False
            if pick:
                a=str.find(i, item)
                a=str.find(i, '=', a)
                b=str.find(i, ',', a+1)
                if a>=0 and b>=0:
                    vec.append(i[a+2:b-1])
        return vec
    
    ###########################################################################
    ###   engine selection functions
    ###########################################################################

    def use_paths(self, dir_list=[]):
        '''use engines that most close to the dirs (or ms)

        Keyword arguments:
        dir_list -- the result label

        Example:
        CASA <33>: sl.list_result
        Out[33]:
        ['#### new split ####################### Mon Mar 14 14:48:08 2011 ####',
        CASA <34>: sl.get_output('new split', 'outputvis', field='3C84')
          Out[34]:
        ['/home/casa-dev-07/hye/ptest/csplit/10B-209a_5s-f1-s8.ms',
         '/home/casa-dev-07/hye/ptest/csplit/10B-209a_5s-f0-s1.ms',
         '/home/casa-dev-07/hye/ptest/csplit/10B-209a_5s-f1-s9.ms']
        CASA <35>: sl.use_paths(
                     '/home/casa-dev-10/hye/ptest/csplit/10B-209a_5s-f1-s14.ms')
          Out[35]: [8]

        '''
        if not self._configdone:
            return
        if len(dir_list)==0:
            print 'dir_list can not be empty'
            return []
        a=[]
        if type(dir_list)!=list:
            a.append(dir_list)
            dir_list=a
        if len(dir_list)>0:
            int_ok=True
            for i in dir_list:
                if type(i)!=str:
                    int_ok=False
            if not int_ok:
                print 'path name in dir_list must be string'
                return []
    
        a=[]
        hst=self.get_hosts()
        for i in dir_list:
            int_ok=False
            for j in range(len(hst)):
                if i.count(hst[j][2])>0:
                    a.append(hst[j][0])
                    int_ok=True
            if not int_ok:
                print 'could not find a host for', i
                return []
        
        e=dict()
        for k in xrange(len(hst)):
            h=hst[k][0]
            e[h]=[]
            for i in self._cluster.get_engines():
                if i[1]==h: 
                    e[h].append(i[0])
        val=e.values()
        key=e.keys()
        lenth=[]
        pos=[]
        for i in xrange(len(val)):
            lenth.append(len(val[i]))
            pos.append(0)
        vec=[]
        for k in a:
            for s in xrange(len(e)):
                if k==key[s]:
                    if pos[s]==lenth[s]:
                       pos[s]=0
                    vec.append(val[s][pos[s]])
                    pos[s]+=1
        return vec

    def use_hosts(self, host_list=[], engines_each=0):
        '''use engines on the given nodes

        Keyword arguments:
        host_list -- the list of hosts
        engines_each -- number of engines to use on each host

        Example:
        CASA <45>: sl.get_hosts
          Out[45]:
                  [['casa-dev-07', 4, '/home/casa-dev-07/hye/ptest'],
                   ['casa-dev-08', 4, '/home/casa-dev-08/hye/ptest'],
                   ['casa-dev-10', 4, '/home/casa-dev-10/hye/ptest']]
        CASA <46>: sl.use_hosts(['casa-dev-07', 'casa-dev-10'], 2)
          Out[46]: [8, 9, 0, 1]

        '''

        if not self._configdone:
            return
        if len(host_list)==0 and  engines_each==0:
            return self._cluster.get_ids()

        hst=self.get_hosts()
        a=[]
        if type(host_list)!=list:
            a.append(host_list)
            host_list=a
        if len(host_list)>0:
            int_ok=True
            for i in host_list:
                if type(i)!=str:
                    int_ok=False
            if not int_ok:
                print 'host name in host_list must be string'
                return []
        else:
            host_list=[]
            for j in range(len(hst)):
                host_list.append(hst[j][0])

        for i in host_list:
            host_ok=False
            for j in range(len(hst)):
                if hst[j][0]==i:
                    host_ok=True
            if not host_ok:
                print 'no host by name', i 
                return []

       
        e=dict()
        for k in host_list:
            e[k]=[]
            for i in self._cluster.get_engines():
                if i[1]==k: 
                    e[k].append(i[0])
        val=e.values()
        vec=[]
        if engines_each==0:
            engines_each=100
        for i in xrange(len(val)):
            j=0
            while j<min(engines_each, len(val[i])):
                vec.append(val[i][j])
                j+=1
        return vec

    def use_engines(self, use_id=[], spreadhost=1):
        '''use engines on from a given list

        Keyword arguments:
        use_id -- the list of engine ids
        spreadhost -- whether to apply host first policy

        Example:
        CASA <52>: sl._cluster.get_ids()
          Out[52]: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
        CASA <54>: sl.use_engines([0, 1, 2, 9])
          Out[54]: [0, 1, 2, 9]
        CASA <55>: sl.use_engines()
          Out[55]: [4, 8, 0, 5, 9, 1, 6, 10, 2, 7, 11, 3]

        '''
        if not self._configdone:
            return
        if len(use_id)>0:
            int_ok=True
            for i in use_id:
                if type(i)!=int:
                    int_ok=False
            if int_ok:
                return use_id
            else:
                print 'engine id in use_id must be integer'
                return []
        elif spreadhost==0:
            return self._cluster.get_ids()
        else:
            e=dict()
            hst=self.get_hosts()
            for i in range(len(hst)):
                e[hst[i][0]]=[]
            for i in self._cluster.get_engines():
                e[i[1]].append(i[0])
            val=e.values()
            lenth=[]
            pos=[]
            for i in xrange(len(val)):
                lenth.append(len(val[i]))
                pos.append(0)
            vec=[]
            #print pos, lenth, val, cluster().get_ids()
            while len(vec)<len(self._cluster.get_ids()):
                for i in xrange(len(val)):
                   if pos[i]<lenth[i]:
                       #print val[i][pos[i]]
                       vec.append(val[i][pos[i]])
                       pos[i]+=1
            return vec
    
    ###########################################################################
    ###   ms knowledge functions
    ###########################################################################
    def get_msname(self, vis):
        '''get the ms name of given vis

        Keyword arguments:
        vis -- the path+name of visibility data

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <16>: sl.get_msname(vis)
          Out[18]: '10B-209a_5s'

        '''

        vs=os.path.abspath(vis)
        msname=vs[str.rfind(vs,'/')+1:]
        if msname.endswith('.ms'):
            msname=msname[:str.rfind(msname, '.ms')]
        return msname
    
    def get_antenna_diam(self, vis):
        '''get the diameter of antennas

        Keyword arguments:
        vis -- the path+name of visibility data

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <19>: sl.get_antenna_diam(vis)
          Out[19]: 25.0

        '''

        tb.open(vis+'/ANTENNA')
        diams=tb.getcol('DISH_DIAMETER')
        diam=np.min(diams)
        if diam==0:
            diam=np.max(diams)
        tb.done()
        return diam
    
    def get_mean_reff(self, vis):
        '''get the mean reference frequency

        Keyword arguments:
        vis -- the path+name of visibility data

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <20>: sl.get_mean_reff(vis)
          Out[20]: 6298222222.2222223

        '''

        tb.open(vis+'/SPECTRAL_WINDOW')
        reff=tb.getcol('REF_FREQUENCY')
        tb.done()
        return reff.mean()
    
    def get_spw_reff(self, vis, spw=0):
        '''get the reference frequency of spw

        Keyword arguments:
        vis -- the path+name of visibility data
        spw -- the spectral window id

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <21>: sl.get_spw_reff(vis, 8)
          Out[21]: 5056000000.0

        '''

        tb.open(vis+'/SPECTRAL_WINDOW')
        if spw<0 or spw>=tb.nrows():
            print 'no such spectral window:', spw
            return
        spw_reff=tb.getcell('REF_FREQUENCY', spw)
        tb.done()
        return spw_reff
    
    def get_spw_chan(self, vis, spw=0):
        '''get the number of channels of spw

        Keyword arguments:
        vis -- the path+name of visibility data
        spw -- the spectral window id

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <24>: sl.get_spw_chan(vis, 8)
          Out[24]: 64

        '''
        tb.open(vis+'/SPECTRAL_WINDOW')
        if spw<0 or spw>=tb.nrows():
            print 'no such spectral window:', spw
            return
        spw_chan=tb.getcell('NUM_CHAN', spw)
        tb.done()
        return spw_chan
    
    def get_pol_corr(self, vis, pol=0):
        '''get the number of coorelation of polarization 

        Keyword arguments:
        vis -- the path+name of visibility data
        pol -- the polarization id

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <31>: sl.get_pol_corr(vis, 0)
          Out[31]: 4

        '''
        tb.open(vis+'/POLARIZATION')
        if pol<0 or pol>=tb.nrows():
            print 'no such polarization:', pol
            return
        pol_corr=tb.getcell('NUM_CORR', pol)
        tb.done()
        return pol_corr
    
    def get_num_field(self, vis):
        '''get the number of fields 

        Keyword arguments:
        vis -- the path+name of visibility data

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <32>: sl.get_num_field(vis)
          Out[32]: 6L

        '''

        tb.open(vis+'/FIELD')
        num_field=tb.nrows()
        tb.done()
        return num_field
    
    def get_field_name(self, vis, id):
        '''get the name of a field 

        Keyword arguments:
        vis -- the path+name of visibility data
        id -- the field id

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <35>: sl.get_field_name(vis, 5)
          Out[35]: 'J0738+1742'

        '''
        tb.open(vis+'/FIELD')
        if id<0 or id>=tb.nrows():
            print 'no such field:', id 
            return
        fn=tb.getcell('NAME', id)
        tb.done()
        return fn
    
    def get_num_spw(self, vis):
        '''get the number of spectral windows 

        Keyword arguments:
        vis -- the path+name of visibility data

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <36>: sl.get_num_spw(vis)
          Out[36]: 18L

        '''

        tb.open(vis+'/SPECTRAL_WINDOW')
        num_spw=tb.nrows()
        tb.done()
        return num_spw
    
    def get_num_desc(self, vis):
        '''get number of data descriptions

        Keyword arguments:
        vis -- the path+name of visibility data

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <37>: sl.get_num_desc(vis)
          Out[37]: 18L

        '''

        tb.open(vis+'/DATA_DESCRIPTION')
        num_desc=tb.nrows()
        tb.done()
        return num_desc
    
    def get_spw_id(self, vis, desc=0):
        '''get spectral window id for desc

        Keyword arguments:
        vis -- the path+name of visibility data
        desc -- the data description id

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <38>: sl.get_spw_id(vis, 17)
          Out[38]: 17

        '''

        tb.open(vis+'/DATA_DESCRIPTION')
        if desc<0 or desc>=tb.nrows():
            print 'no such data description:', desc
            return
        spw_id=tb.getcell('SPECTRAL_WINDOW_ID', desc)
        tb.done()
        return spw_id
    
    def get_pol_id(self, vis, desc=0):
        '''get polarization id for desc

        Keyword arguments:
        vis -- the path+name of visibility data
        desc -- the data description id

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <39>: sl.get_pol_id(vis, 17)
          Out[39]: 0

        '''

        tb.open(vis+'/DATA_DESCRIPTION')
        if desc<0 or desc>=tb.nrows():
            print 'no such data description:', desc
            return
        pol_id=tb.getcell('POLARIZATION_ID', desc)
        tb.done()
        return pol_id
    
    def get_field_desc(self, vis):
        '''get source

        Keyword arguments:
        vis -- the path+name of visibility data

        Example:
        CASA <15>: vis="/lustre/casa-store/hye/10B-209a_5s.ms"
        CASA <40>: sl.get_field_desc(vis)
          Out[40]:
        {(0, 0): {'cost': [0, 82],
                  'desc': 0,
                  'field': 0,
                  'nchan': 64,
                  'ncorr': 4,
                  'nrows': 16848,
                  'pol': 0,
                  'spw': 0},
         (0, 1): {'cost': [0, 82],
                  'desc': 1,
                  'field': 0,
                  'nchan': 64,
                  'ncorr': 4,
                  'nrows': 16848,
                  'pol': 0,
                  'spw': 1}
         ... ommit ...
        }

        '''

        tb.open(vis)
        nrows=tb.nrows() 
        tb.done()
        k=nrows/1000
        t=nrows%1000
        if t>0:
           k+=1
        b={}
        #print k, nrows
        tb.open(vis)
        for i in xrange(k):
            field=tb.getcol('FIELD_ID', i*1000, 1000)
            desc=tb.getcol('DATA_DESC_ID', i*1000, 1000)
            fd=zip(field, desc)
            newset=set(fd)
            for j in newset:
                if b.has_key(j):
                   #print fd.count(j)
                   b[j]['nrows']=b[j]['nrows']+fd.count(j)
                else:
                   a={}
                   a['field']=j[0]
                   a['desc']=j[1]
                   a['nrows']=fd.count(j)
                   b[j]=a
        tb.done()
        d=0
        for i in b.keys():
            b[i]['pol']=self.get_pol_id(vis, b[i]['desc'])
            b[i]['spw']=self.get_spw_id(vis, b[i]['desc'])
            b[i]['ncorr']=self.get_pol_corr(vis, b[i]['pol'])
            b[i]['nchan']=self.get_spw_chan(vis, b[i]['spw'])
            b[i]['cost']=b[i]['nchan']*b[i]['ncorr']*b[i]['nrows']
            d+=b[i]['cost']
        d/=len(b.keys()) 
        for i in b.keys():
            b[i]['cost']=[b[i]['cost']/d, len(b.keys())]
        return b
    
    
    ###########################################################################
    ###   setup -  
    ###########################################################################
    def init_cluster(self, clusterfile='', project=''):
        '''Setup the cluster

        Keyword arguments:
        clusterfile -- the cluster definition file
        project -- the name of project (default: 'proj'+timestamp).  

        A configuration file is an ASCII text file. Each line defines a node
        (also called host) with comma separated entries of host_name,
        number_of_engines and work_dir. A line that starts with # is ignored.
        The following is the contents of a configuration file:
        ############################################
        casa-dev-07, 4, /home/casa-dev-07/hye/ptest
        #casa-dev-08, 4, /home/casa-dev-08/hye/ptest
        casa-dev-10, 4, /home/casa-dev-10/hye/ptest
        ############################################

        A project maintains a subdirectory under each node's work_dir. All 
        output files of an engine hosted on that node will by default store
        under the subdirectory.

        Example:
        CASA <15>: from simple_cluster import *
        CASA <16>: sl=simple_cluster()
        CASA <17>: sl.init_cluster('my_cluster', 'aProj')

        '''

        if project==None or type(project)!=str or project.strip()=="":
            #project name must be a non-empty string, otherwise set default
            project='cluster_project'
            print 'use the default project: cluster_project'
    
        if clusterfile==None or type(clusterfile)!=str or clusterfile.strip()=="":
            #cluster file name must be a non-empty string, otherwise generate
            #a default clusterfile
            import multiprocessing
            ncpu=multiprocessing.cpu_count()
            (sysname, nodename, release, version, machine)=os.uname()
            homedir = os.path.expanduser('~')
            msg=nodename+', '+str(ncpu)+', '+homedir
            #cdir=os.getcwd()
            #print cdir, cdir[:cdir.rfind('/')]
            #msg=nodename+', '+str(ncpu)+', '+cdir[:cdir.rfind('/')]
            #project=cdir[cdir.rfind('/')+1:]
            clusterfile='/tmp/default_cluster'
            f=open(clusterfile, 'w')
            f.write(msg)
            f.close()
    
        self.config_cluster(clusterfile, True)
        if not self._configdone:
            return
        self.create_project(project)
        self.stop_nodes()
        self.stop_resource()
        self.start_cluster()
        self.start_monitor()
        self.start_logger()
        self.start_resource()
    
    
    ###########################################################################
    ###   example to distribute clean task over engines
    ###########################################################################
    def simple_clean(self, vs, nx, ny, mode='channel', email=''):
        '''Make images with a simple cluster

        Keyword arguments:
        vs -- the visibility data
        nx, ny -- the size (pixels) of the image
        mode -- either 'channel' or 'continuum'
        email -- the email address to notify the completion

        Example:
        CASA <15>: from simple_cluster import *
        CASA <16>: sl=simple_cluster()
        CASA <17>: sl.init_cluster('my_cluster', 'aProj')
        CASA <18>: simple_clean(
            vis='/home/casa-dev-09/hye/ptest/sim.alma.csv.mid.ms', 
            nx=256, ny=256, mode='channel')

        '''
        
        vis=os.path.abspath(vs)
        tb.clearlocks(vis)
    
        #determine the cell size
        diam=self.get_antenna_diam(vis)
        freqmean=self.get_mean_reff(vis)
        print 'diam:', diam, 'freqmean:', freqmean 
        fv=(3.0e8/freqmean/diam)*180*60*60/math.pi
        cell=[str(fv/nx)+'arcsec', str(fv/ny)+'arcsec']
    
        print 'vis=', vis
        fdspw=self.get_field_desc(vis)
        ids=self._cluster.get_ids()
        msname=self.get_msname(vis)
    
        if len(fdspw)>len(ids):
            #more job chunks than engines, simply distribute by field and spw
            i=0
            for k in fdspw.values():
                id=ids[i]
                s={}
                s['vis']=vis
                fd=str(k['field'])
                spw=str(k['spw'])
                s['imagename']=self.get_engine_store(id)+msname+'-f'+fd+'-s'+spw
                s['field']=fd
                s['spw']=spw
                s['mode']='channel'
                s['niter']=20000
                s['threshold']='0.001mJy'
                s['psfmode']='hogbom'
                s['imagermode']='csclean'
                s['imsize']=[nx, ny]
                s['cell']=cell
                s['calready']=False
                cmd=self.make_call('clean', s) 
                #print cmd
                self.do_and_record(cmd, id, email)
                i+=1
                if i==len(ids):
                   i=0
        else:
            #less job chanks than engines, need further devide by channel
            i=0
            for k in fdspw.values():
                spwchan=self.get_spw_chan(vis, k['spw'])
                #print 'spwchan:', spwchan
        
                nengs=len(ids)
                nchan=1
                try:
                    nchan=int(ceil(abs(float(spwchan))/nengs))
                except:
                    pass
        
                for j in xrange(len(ids)):
                    id=ids[i]
                    start=j*nchan
                    s={}
                    s['vis']=vis
                    fd=str(k['field'])
                    spw=str(k['spw'])
                    s['imagename']=(self.get_engine_store(id)+msname+'-f'+fd+
                             '-s'+spw+'-b'+str(start)+'-e'+str(start+nchan))
                    s['field']=fd
                    s['spw']=spw
                    s['mode']='channel'
                    s['niter']=20000
                    s['start']=start
                    s['nchan']=nchan
                    s['threshold']='0.001mJy'
                    s['psfmode']='hogbom'
                    s['imagermode']='csclean'
                    s['imsize']=[nx, ny]
                    s['cell']=cell
                    s['calready']=False
                    cmd=self.make_call('clean', s) 
                    #print cmd
                    self.do_and_record(cmd, id, email)
                    i+=1
                    if i==len(ids):
                       i=0
        
        self.get_status()
    
    def simple_split(self, vs, email):
        '''split by source (field, spw) with parallel engines

        Keyword arguments:
        vs -- the visibility data
        email -- the email address to notify the completion

        Example:
        CASA <15>: from simple_cluster import *
        CASA <16>: sl=simple_cluster()
        CASA <17>: sl.init_cluster('my_cluster', 'aProj')
        CASA <18): vis='/home/casa-dev-09/hye/ptest/sim.alma.csv.mid.ms', 
        CASA <18>: simple_split(vis)

        '''
        vis=os.path.abspath(vs)
        tb.clearlocks(vis)
       
        print 'vis=', vis
        fdspw=self.get_field_desc(vis)
        #ids=self._cluster.get_ids()
        ids=self.use_engines()
        msname=self.get_msname(vis)
    
        i=0
        for k in fdspw.values():
            id=ids[i]
            s={}
            s['vis']=vis
            fd=str(k['field'])
            spw=str(k['spw'])
            s['outputvis']=self.get_engine_store(id)+msname+'-f'+ \
                               fd+'-s'+spw+'.ms'
            if os.path.exists(s['outputvis']):
                os.system('rm -rf '+s['outputvis'])
            #s['field']=fd
            s['field']=self.get_field_name(vis, k['field']) 
            s['spw']=spw
            s['datacolumn']='DATA'
            cmd=self.make_call('split', s) 
            #print cmd
            self.do_and_record(cmd, id, email)
            i+=1
            if i==len(ids):
               i=0
        self.get_status()
    
#if __name__ == "__main__":
    #from simple_cluster import *
    #sl=simple_cluster()
    #sl.init_cluster('my_cluster', 'aProj')
    #vis='/home/casa-dev-09/hye/ptest/sim.alma.csv.mid.ms', 
    #simple_split(vis)

    #get_status()

    #init_cluster('my_cluster', 'my_project')
    #simple_clean(vis='/home/casa-dev-09/hye/ptest/sim.alma.csv.mid.ms', 
    #             nx=256, ny=256, mode='channel')
  
    #simple_clean(vis='/home/casa-dev-09/hye/para/sim.alma.csv.mid.ms', 
    #             nx=256, ny=256, mode='channel')
      
    #simple_clean(vis='/home/casa-dev-09/hye/pclean/sim100g_4chan15kRows.ms',
    #             nx=256, ny=256, mode='channel')
        


###########################################################################
###   job management classes
###########################################################################

class commandBuilder:
    '''
    Simple class to help with the building of the long strings needed to
    execute a command.
    '''
    def __init__(self, commandName):
        # TODO: Ensure that commandName is a string
        self._commandName = commandName
        self._argumentList = {}

    def setArgument(self, argument, value):
        # TODO: Ensure that the argumen is a string
        self._argumentList[argument] = value

    def getCommandString(self):
        firstArgument = True
        output = self._commandName + '('
        for (arg,value) in self._argumentList.items():
            if firstArgument:
                firstArgument = False
            else:
                output += ', '
            if isinstance(value, str):
                output += ("%s = '%s'" % (arg, value))
            else:
                output += ("%s = " % arg) + str(value)
        output += ')'
        return output

class JobData:
    def __init__(self, commandLine, jobInfo = None):
        self.commandLine = commandLine
        self.jobInfo = jobInfo
        self.completed = False
        self.jobId     = None
        self.engineId  = None

class JobQueueManager:

    def __init__(self, cluster):
        self.__cluster=cluster
        self.__pendingQueue = []
        self.__completedQueue = []

    def addJob(self, jobData):
        # TODO Check the type of jobData
        self.__pendingQueue.append(jobData)

    def getCompletedJobs(self):
        cmpt=0
        for job in self.__completedQueue:
            if job.completed==True:
                cmpt+=1
        if cmpt==len(self.__completedQueue):
            return self.__completedQueue
        ids=self.__cluster.get_jobId('done')
        if len(ids)!=0:
            completedJobs=[]
            for job in self.__completedQueue:
                if job.jobId in ids:
                    job.completed=True
                    completedJobs.append(job)
            return completedJobs
        else:
            ids=self.__cluster.get_jobId('running')
            for job in self.__completedQueue:
                if job.jobId in ids:
                   cmpt=-1
            ids=self.__cluster.get_jobId('scheduled')
            for job in self.__completedQueue:
                if job.jobId in ids:
                   cmpt=-2
            if cmpt<0:
                return []
            for job in self.__completedQueue:
                job.completed=True
            return self.__completedQueue

    def getUncompletedJobs(self):
        cmpt=0
        for job in self.__completedQueue:
            if job.completed==True:
                cmpt+=1
        if cmpt==len(self.__completedQueue):
            return []
        ids=self.__cluster.get_jobId('done')
        if len(ids)!=0:
            uncompletedJobs=[]
            for job in self.__completedQueue:
                if not job.jobId in ids:
                    uncompletedJobs.append(job)
            return uncompletedJobs
        else:
            ids=self.__cluster.get_jobId('running')
            for job in self.__completedQueue:
                if job.jobId in ids:
                   cmpt=-1
            ids=self.__cluster.get_jobId('scheduled')
            for job in self.__completedQueue:
                if job.jobId in ids:
                   cmpt=-2
            if cmpt<0:
                return self.__completedQueue 
            for job in self.__completedQueue:
                job.completed=True
            return []
        
    def getAllJobs(self):
        allJobs = []
        for job in self.__completedQueue:
            allJobs.append(job)
        return allJobs

    def executeQueue(self):
        self.__pendingQueue.reverse()
        engineList = self.__cluster.use_engines()

        i=-1
        while len(engineList) > 0 and len(self.__pendingQueue) > 0:
            jd = self.__pendingQueue.pop()
            #print "Starting: " + jd.commandLine
            i=i+1
            if i==len(engineList):
                i=0
            jd.jobId=self.__cluster.do_and_record(jd.commandLine, i)
            jd.engineId=i
            jd.completed=False
            self.__completedQueue.append(jd)



from collections import namedtuple
import os
class MultiMS:
    MSEntry = namedtuple('MSEntry', 'use')
    BoilerPlate ='''#!MultiMS Specification File: Version 1.0
# This file includes the specification for a multiMS for reduction using CASA
# Please do not modify the first line of this file.
# Any line's beginning with a '#' are treated as comments
# Processing is disabled for MS definition lines beginning with a '!'
# All paths are considered relative to the location of this specification file
''' 
    def __init__(self, mmsName, mmsSpec=None, initialize = True):
        self.__mmsName = mmsName
        self.__entryDict={}
        if initialize:
            self.readSpec(mmsSpec)

    def readSpec(self, mmsSpec = None):
        fd = open(self.__createSpecFilename(mmsSpec), 'r')
        self.__getVersion(fd)
        for line in fd.readlines():
            if line[0] == '#':
                continue
            if line[0] == '!':
                self.addMS(line[1:].rstrip(), False)
            else:
                self.addMS(line.rstrip(), true)
        fd.close()
        print self.__entryDict


    def writeSpec(self, mmsSpec = None):
        fd = open(self.__createSpecFilename(mmsSpec), 'w')
        fd.write(MultiMS.BoilerPlate)
        for path in self.__getSortedEntries():
            if not self.__entryDict[path].use:
                fd.write('!')
            fd.write(path +'\n')
        fd.close()

    def show(self):
        print self.__entryDict

    def addMS(self, msName, use=True):
        '''
        This method adds the specified MS to the MultiMS, the MS Name must
        be unique or an exception in thrown.
        '''
        if msName in self.__entryDict:
            # TODO raise an exception for duplicate entries
            pass
        self.__entryDict[msName] = MultiMS.MSEntry(use)

    def rmMS(self, msName):
        if msName not in self._entryDict:
            # TODO raise an exception for non-existant entry
            pass
        self.__entryDict.pop(msName)

    def writeFile(self, msgf):
        f=open(msgf, 'w')
        for entryKey in self.__getSortedEntries():
            if self.__entryDict[entryKey].use:
                f.write(self.__createPath(entryKey))
                f.write('\n')
        f.close()

    def getMSPathList(self):
        pathList = []
        for entryKey in self.__getSortedEntries():
            if self.__entryDict[entryKey].use:
                pathList.append(self.__createPath(entryKey))
        return pathList
                       
    def __getSortedEntries(self):
        entryKeys = self.__entryDict.keys()
        entryKeys.sort()
        return entryKeys
        
    def __createSpecFilename(self, mmsSpec):
        if mmsSpec is None:
            mmsSpec = self.__mmsName.replace('.mms', '.mmsSpec')  
        return self.__createPath(mmsSpec)

    def __createPath(self, filename):
        return "%s/%s" % (self.__mmsName, filename)

    def __getVersion(self, fileDescriptor):
        pass


###########################################################################
###   job management example
###########################################################################
#
#from simple_cluster import *
#
#c=simple_cluster()
#c.cold_start()
#c.init_cluster('', '')
#
#option_ms='X306/X306.ms'
#option_mmsName='X306.mms'
#option_mmsSpec=None
#
#option_ms=os.path.abspath(option_ms)
#
#if not os.path.exists(option_mmsName):
#    os.mkdir(option_mmsName)
#
#msFileRoot=option_mmsName.replace('.mms','')
#
#command=commandBuilder("split")
#command.setArgument('vis', option_ms)
#                   
#jobManager=JobQueueManager(c)
#for scan in range(5,10):
#    command.setArgument('scan',str(scan))
#    outputVis=msFileRoot + ('.%05d.ms' % scan)
#    command.setArgument('outputvis', "%s/%s" % (option_mmsName,outputVis))
#    #print command.getCommandString()
#    jobManager.addJob(JobData(command.getCommandString(), outputVis))
#
#jobManager.executeQueue()
#
#
#print 'after all jobs finished'
#
#mms=MultiMS(option_mmsName, option_mmsSpec, False)
#for job in jobManager.getCompletedJobs():
#    mms.addMS(job.jobInfo)
#
#mms.writeFile('myMms')
#
#mms=MultiMS(option_mmsName, option_mmsSpec)
#print mms.getMSPathList()
#
