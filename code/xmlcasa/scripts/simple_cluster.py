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
                print 'no writhe permision in directory "%s"' % self._hosts[i][2]
                return False
        return True
    
    
    ###########################################################################
    ###   project management
    ###########################################################################
    def create_project(self, proj=""):
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
                cmd='mkdir '+self._hosts[i][2]+'/'+self._project
                os.system(cmd)
        print 'output directory: '
        for i in range(len(self._hosts)):
            print self._hosts[i][2]+'/'+self._project
    
    def do_project(self, proj):
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
        '''remove all previous results of the proj'''
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
        if not self._configdone:
            return
        for i in range(len(self._hosts)):
            cmd='rm -rf '+self._hosts[i][2]+'/*'
            os.system(cmd)
        self._project=""
        print 'project name not set'
    
    def list_projects(self):
        if not self._configdone:
            return
        for i in range(len(self._hosts)):
            print 'host:', self._hosts[i][0], '------------------------>>>>'
            cmd='ls '+self._hosts[i][2]+'/' 
            os.system(cmd)
    
    def reset_project(self):
        self.stop_monitor()
        self.clear_project(self._project)
        self._jobs={}
        self._job_title=1
        self._monitor_on=True
    
    def get_hosts(self):
        return self._hosts 
    
    ###########################################################################
    ###   cluster management
    ###########################################################################
    def cold_start(self):
        for i in range(len(self._hosts)):
            cmd='ssh '+self._hosts[i][0]+' "killall -9 ipengine"'
            os.system(cmd)
        self._cluster.stop_cluster()
    
    def stop_nodes(self):
        for i in self._cluster.get_nodes():
            self._cluster.stop_node(i)
    
    def start_cluster(self):
        for i in range(len(self._hosts)):
            self._cluster.start_engine(self._hosts[i][0], self._hosts[i][1], 
                                   self._hosts[i][2]+'/'+self._project)
    
    def get_host(self, id):
        '''get hostname of an engine'''
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
        '''get the root path where an engine write out result'''
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
            cpu[7]=(cpu[5]-cpu[6])/float(cpu[5])
            cpu[10]=(cpu[8]-cpu[9])/float(cpu[8])
            self._rsrc[self._hosts[i][0]]=cpu 
    
    def start_resource(self): 
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
        while self._resource_on:
            time.sleep(5)
            self.check_resource()
    
    def stop_resource(self): 
        self._resource_on=False 
        self._rsrc={}
    
    def show_resource(self, long=False):
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
        done=False
        while(not done):
            time.sleep(5)
            done=True
            for i in jobs.keys():
                done=self._cluster.check_job(i) and done
    
    def check_status(self, notify=False):
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
                        x=job.get_result(block=False)
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
                            if notify and self._jobs[job]['status']=="running":
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
                finish=True
                for val in self._jobs.values():
                    if val['jobgroup']==i and not (val['status']=='done' or 
                                                val['status']=='broken'):
                        finish=False
                if finish==True:
                    msg='\n'
                    msg+='Label: '+time.ctime()
                    msg+='\n\nengine    status  time(s)  command\n'
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
                    if i!='':
    
                        #append to project result file
                        f=open(self._project+'.result', 'a')
                        #f.write('\n'+i)
                        f.write(msg)
                        f.write('\n')
                        f.close()
    
                        if i.count(' ')==0 and i.count('@'):
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
                            cmd='/bin/mail -s "your parallel job finished" '+i+' < '+msgf
                            #print cmd
                            os.system(cmd)
                             
                        for j in rmv:
                            self.remove_record(j)
                            
    
    def start_monitor(self): 
        self._monitor_on=True 
        return thread.start_new_thread(self.check_status, (True,))
    
    def stop_monitor(self): 
        self.monitor_on=False 
    
    def show_queue(self):
        return self._cluster.queue_status()
    
    def get_status(self, long=False):
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
                           self._jobs[job]['short'],
                           self._jobs[job]['jobname'])
    
    def remove_record(self, jobname):
        for job in self._jobs.keys():
            if type(jobname)==int and self._jobs[job]['jobname']==jobname:
               del self._jobs[job]
            if type(jobname)==str and self._jobs[job]['status']==jobname:
               del self._jobs[job]
    
    ###########################################################################
    ###   job distribution functions
    ###########################################################################
    def make_call(self, func, param):
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
        job=self._cluster.odo(cmd, id)
        self._jobs[job]={}
        self._jobs[job]['start']=''
        self._jobs[job]['time']=0
        self._jobs[job]['command']=cmd
        self._jobs[job]['short']=cmd[:str.find(cmd, '(')]
        self._jobs[job]['status']="scheduled"
        self._jobs[job]['engine']=id
        self._jobs[job]['jobname']=self._job_title
        self._jobs[job]['jobgroup']=group.strip()
        self._job_title+=1
    
    ###########################################################################
    ###   result processing functions
    ###########################################################################
    
    def list_result(self):
        '''read the project.result file and write out all labels'''
        f=open(self._project+'.result', 'r')
        s=f.readlines()
        vec=[]
        for line in s:
            sLine=line.rstrip()
            if str.find(sLine, 'Label:')==0:
                vec.append(sLine[6:].strip())
            else:
                continue
        f.close()
        return vec
    
    def get_result(self, tm):
        '''read the project.result file and write out result for a label'''
        f=open(self._project+'.result', 'r')
        s=f.readlines()
        reach=False
        vec=[]
        for line in s:
            sLine=line.strip()
            if str.find(sLine, 'Label:')==0:
                if str.count(sLine, tm)>0 and reach==False:
                    reach=True
                else:
                    reach=False
            else:
                if reach and sLine!='' and not sLine.startswith('engine'):
                    vec.append(sLine)
        f.close()
        return vec
    
    def get_output(self, result, item, **kwargs):
        '''pick from result list the item that meets condistion in kwargs'''
        if type(result)!=list:
            print 'result must be a list of strings'
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
        '''use engines that most close to the dirs'''
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
        '''use engines on the given nodes'''
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
        '''get a ordered engine list'''
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
        '''get the ms name of given vis'''
        vs=os.path.abspath(vis)
        msname=vs[str.rfind(vs,'/')+1:]
        if msname.endswith('.ms'):
            msname=msname[:str.rfind(msname, '.ms')]
        return msname
    
    def get_antenna_diam(self, vis):
        '''get the diameter of antennas'''
        tb.open(vis+'/ANTENNA')
        diams=tb.getcol('DISH_DIAMETER')
        diam=np.min(diams)
        if diam==0:
            diam=np.max(diams)
        tb.done()
        return diam
    
    def get_mean_reff(self, vis):
        '''get the mean reference frequency'''
        tb.open(vis+'/SPECTRAL_WINDOW')
        reff=tb.getcol('REF_FREQUENCY')
        tb.done()
        return reff.mean()
    
    def get_spw_reff(self, vis, spw=0):
        '''get the reference frequency of spw'''
        tb.open(vis+'/SPECTRAL_WINDOW')
        spw_reff=tb.getcell('REF_FREQUENCY', spw)
        tb.done()
        return spw_reff
    
    def get_spw_chan(self, vis, spw=0):
        '''get the number of channels of spw'''
        tb.open(vis+'/SPECTRAL_WINDOW')
        spw_chan=tb.getcell('NUM_CHAN', spw)
        tb.done()
        return spw_chan
    
    def get_pol_corr(self, vis, pol=0):
        '''get the number of coorelation of polarization '''
        tb.open(vis+'/POLARIZATION')
        pol_corr=tb.getcell('NUM_CORR', pol)
        tb.done()
        return pol_corr
    
    def get_num_field(self, vis):
        '''get the number of fields '''
        tb.open(vis+'/FIELD')
        num_field=tb.nrows()
        tb.done()
        return num_field
    
    def get_field_name(self, vis, id):
        '''get the name of a field '''
        tb.open(vis+'/FIELD')
        fn=tb.getcell('NAME', id)
        tb.done()
        return fn
    
    def get_num_spw(self, vis):
        '''get the number of spectral windows '''
        tb.open(vis+'/SPECTRAL_WINDOW')
        num_spw=tb.nrows()
        tb.done()
        return num_spw
    
    def get_num_desc(self, vis):
        '''get number of data descriptions'''
        tb.open(vis+'/DATA_DESCRIPTION')
        num_desc=tb.nrows()
        tb.done()
        return num_desc
    
    def get_spw_id(self, vis, desc=0):
        '''get spectraol window id for desc'''
        tb.open(vis+'/DATA_DESCRIPTION')
        spw_id=tb.getcell('SPECTRAL_WINDOW_ID', desc)
        tb.done()
        return spw_id
    
    def get_pol_id(self, vis, desc=0):
        '''get polarization id for desc'''
        tb.open(vis+'/DATA_DESCRIPTION')
        pol_id=tb.getcell('POLARIZATION_ID', desc)
        tb.done()
        return pol_id
    
    def get_field_desc(self, vis):
        '''get source'''
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
        '''split by source (field, spw)'''
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
    #init_cluster('my_cluster', 'my_project')
    #simple_clean(vis='/home/casa-dev-09/hye/ptest/sim.alma.csv.mid.ms', 
    #             nx=256, ny=256, mode='channel')
  
    #simple_clean(vis='/home/casa-dev-09/hye/para/sim.alma.csv.mid.ms', 
    #             nx=256, ny=256, mode='channel')
      
    #simple_clean(vis='/home/casa-dev-09/hye/pclean/sim100g_4chan15kRows.ms',
    #             nx=256, ny=256, mode='channel')
        
    #check_status()
