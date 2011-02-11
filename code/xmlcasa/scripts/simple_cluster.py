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
 

#im,cb,ms,tb,fg,af,me,ia,po,sm,cl,cs,rg,sl,dc,vp=gentools()

#from casa import *

hosts=[]
configdone=False
jobs={}
rsrc={}
project=""

###########################################################################
###   cluster verifiction
###########################################################################
def config_cluster(cfg, force=False):
    global configdone, hosts
    if (len(hosts)>0 or configdone) and not force:
        print 'cluster already configured'
        return
    hosts=[]
    jobs={}
    rsrc={}
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
        hosts.append([a, b, c])
    for i in range(len(hosts)):
        rsrc[hosts[i][0]]=[]
    #print hosts

    configdone=validate_hosts()
    if not configdone:
        print 'failed to config the cluster'
    if project=="":
        #print 'project name not set'
        pass

def validate_hosts():
    uhost=set()
    for i in range(len(hosts)):
        uhost.add(hosts[i][0])
    if len(uhost)==0:
        print 'configuration table is empty'
        return False
    if len(uhost)<len(hosts):
        print 'configuration table contains repeated node name'
        return False
    for i in range(len(hosts)):
        if type(hosts[i][1])!=int:
            print 'the number of engines must be an integer'
            return False
    for i in range(len(hosts)):
        if not os.path.exists(hosts[i][2]):
            print 'the directory "%s" does not exist' % hosts[i][2]
            return False
    for i in range(len(hosts)):
        try:
            tfile=hosts[i][2]+'/nosuchfail'
            f = open(tfile, 'w')
            os.remove(tfile)
        except IOError:
            print 'no writhe permision in directory "%s"' % hosts[i][2]
            return False
    return True

###########################################################################
###   project management
###########################################################################
def create_project(proj=""):
    global configdone, project
    if not configdone:
        return
    if type(proj)!=str:
        print 'project name must be string'
        return

    project=proj.strip()
    if project=="":
        tm=time.strftime("%Y%b%d-%Hh%Mm%Ss", time.localtime())
        project='proj'+tm

    for i in range(len(hosts)):
        if not os.path.exists(hosts[i][2]+'/'+proj.strip()):
            cmd='mkdir '+hosts[i][2]+'/'+project
            os.system(cmd)
    print 'output directory: '
    for i in range(len(hosts)):
        print hosts[i][2]+'/'+project

def do_project(proj):
    global configdone, project
    if not configdone:
        return
    if type(proj)!=str or proj.strip()=="":
        print 'project name must be a nonempty string'
        return

    projexist=True
    for i in range(len(hosts)):
        if not os.path.exists(hosts[i][2]+'/'+proj.strip()):
            print 'no project directory found on '+hosts[i][0]
            projexist=False
    if projexist:
        project=proj.strip()
    print 'output directory: '
    for i in range(len(hosts)):
        print hosts[i][2]+'/'+project

def erase_project(proj):
    global configdone, project
    if not configdone:
        return
    if type(proj)!=str or proj.strip()=="":
        print 'project name must be a nonempty string'
        return
    for i in range(len(hosts)):
        cmd='rm -rf '+hosts[i][2]+'/'+proj
        os.system(cmd)
    if proj==project:
        project=""
    if project=="":
        #print 'project name not set'
        pass

def clear_project(proj):
    global configdone, project
    if not configdone:
        return
    if type(proj)!=str or proj.strip()=="":
        print 'project name must be a nonempty string'
        return
    for i in range(len(hosts)):
        cmd='rm -rf '+hosts[i][2]+'/'+proj.strip()+'/*'
        os.system(cmd)

def list_project(proj):
    global configdone, project
    if not configdone:
        return
    if type(proj)!=str or proj.strip()=="":
        print 'project name must be a nonempty string'
        return
    for i in range(len(hosts)):
        print 'host:', hosts[i][0], '------------------------>>>>'
        cmd='ls '+hosts[i][2]+'/'+proj
        os.system(cmd)

def erase_projects():
    global configdone
    if not configdone:
        return
    for i in range(len(hosts)):
        cmd='rm -rf '+hosts[i][2]+'/*'
        os.system(cmd)
    project=""
    print 'project name not set'

def list_projects():
    global configdone
    if not configdone:
        return
    for i in range(len(hosts)):
        print 'host:', hosts[i][0], '------------------------>>>>'
        cmd='ls '+hosts[i][2]+'/' 
        os.system(cmd)

def reset_project():
    global project
    stop_monitor()
    clear_project(project)
    jobs={}
    job_title=1
    monitor_on=True

###########################################################################
###   cluster management
###########################################################################
def cold_start():
    for i in range(len(hosts)):
        cmd='ssh '+hosts[i][0]+' "killall -9 ipengine"'
        os.system(cmd)
    c.stop_cluster()
   

c=cluster()

def cluster():
    return c

def stop_nodes():
    for i in c.get_nodes():
        c.stop_node(i)

def start_cluster():
    for i in range(len(hosts)):
        c.start_engine(hosts[i][0], hosts[i][1], hosts[i][2]+'/'+project)

###########################################################################
###   log management
###########################################################################
def start_logger():
    lg=c.get_casalogs()
    for i in lg:
        eng='engine'+i[str.rfind(i,'-'):]
        if os.path.exists(eng):
            os.unlink(eng)
        os.symlink(i, eng)

###########################################################################
###   resource management
###########################################################################
def check_resource():
    for i in range(len(hosts)):
        cpu=[0.]*11
        b=commands.getoutput("ssh "+hosts[i][0]+ 
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

        s=commands.getoutput("ssh "+hosts[i][0]+ 
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

        #print rsrc
        old=rsrc[hosts[i][0]] if rsrc.has_key(hosts[i][0]) else [] 
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
        rsrc[hosts[i][0]]=cpu 

def start_resource(): 
    #fig=pl.figure(98)
    #title='casa cluster resource usage'

    #t=fig.text(0.5, 0.95, title, horizontalalignment='center',
    #           verticalalignment='center',
    #           fontproperties=pl.matplotlib.font_manager.FontProperties(size=16))
    #fig.show() this should be the last and should be the only one, use draw()
    global resource_on
    resource_on=True 
    rsrc={}
    return thread.start_new_thread(update_resource, ())

def update_resource():
    global resource_on
    while resource_on:
        time.sleep(5)
        check_resource()

def stop_resource(): 
    global resource_on
    resource_on=False 
    rsrc={}

def show_resource(long=False):
    if long:
        return rsrc
    else:
        if len(rsrc.keys())==0:
            return rsrc
        else:
            print ' host          cpu    iowait mem    swap'
            for h in rsrc.keys():
                print "%12s%7.2f%7.2f%7.2f%7.2f" % (
                       h,
                       rsrc[h][2], 
                       rsrc[h][4],
                       rsrc[h][7], 
                       rsrc[h][10]
                       )

###########################################################################
###   execution status management
###########################################################################
def check_job():
    done=False
    while(not done):
        time.sleep(5)
        done=True
        for i in jobs.keys():
            done=c.check_job(i) and done

monitor_on=True
resource_on=True
job_title=1

def check_status(notify=False):
    global monitor_on
    while monitor_on:
        time.sleep(5)
        curr={}
        try:
            pend=c.queue_status()
            for i in xrange(len(pend)):
                (a, b)=pend[i]
                curr[a]=b['pending']
        except:
            pass
        #print 'curr', curr
        for job in jobs.keys():
            if type(job)==type(None):
                jobs[job]['status']="unknown"
            else:
                eng=jobs[job]['engine']
                sht=jobs[job]['short']
                try:
                    x=job.get_result(block=False)
                    if x==None:
                        cmd=jobs[job]['command']
                        if curr.has_key(eng):
                            wk=eval(curr[eng].lstrip('execute(').rstrip(')'))
                            #print 'wk', wk
                            #print 'cmd', cmd
                            if wk==cmd:
                                jobs[job]['status']="running"
                                if jobs[job]['start']=='':
                                    if notify:
                                        print 'engine %d job %s started'%(eng,sht)
                                    jobs[job]['start']=time.time()
                                jobs[job]['time']=time.time()-jobs[job]['start'] 
                            else:
                                pass
                    else:
                        if notify and jobs[job]['status']=="running":
                            print 'engine %d job %s finished' % (eng, sht)
                        jobs[job]['status']="done"
                except e:
                    if notify and jobs[job]['status']=="running":
                        print 'engine %d job %s broken' % (eng, sht)
                    jobs[job]['status']="broken"

def start_monitor(): 
    global monitor_on
    monitor_on=True 
    return thread.start_new_thread(check_status, (True,))

def stop_monitor(): 
    global monitor_on
    monitor_on=False 

def show_queue():
    return c.queue_status()

def get_status(long=False):
    if long:
        return jobs
    else:
        if len(jobs.keys())==0:
            return jobs
        else:
            print 'engine    status  time(s)     start  command   title'
            for job in jobs.keys():
                print "%6d%10s%9d%10s%9s%8d" % (jobs[job]['engine'], 
                       jobs[job]['status'],
                       int(jobs[job]['time']), 
                       '' if type(jobs[job]['start'])==str 
                          else
                             time.strftime("%H:%M:%S", 
                                    time.localtime(jobs[job]['start'])),
                       jobs[job]['short'],
                       jobs[job]['jobname'])

def remove_record(jobname):
    for job in jobs.keys():
        if type(jobname)==int and jobs[job]['jobname']==jobname:
           del jobs[job]
        if type(jobname)==str and jobs[job]['status']==jobname:
           del jobs[job]

###########################################################################
###   job distribution functions
###########################################################################
def make_call(func, param):
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

def do_and_record(cmd, id):
    global job_title
    job=c.odo(cmd, id)
    jobs[job]={}
    jobs[job]['start']=''
    jobs[job]['time']=0
    jobs[job]['command']=cmd
    jobs[job]['short']=cmd[:str.find(cmd, '(')]
    jobs[job]['status']="scheduled"
    jobs[job]['engine']=id
    jobs[job]['jobname']=job_title
    job_title+=1


###########################################################################
###   example to distribute clean task over engines
###########################################################################
def simple_clean(vis, nx, ny, mode='channel'):

    tb.clearlocks(vis)

    #determine the cell size
    diam=get_antenna_diam(vis)
    freqmean=get_mean_reff(vis)
    print 'diam:', diam, 'freqmean:', freqmean 
    fv=(3.0e8/freqmean/diam)*180*60*60/math.pi
    cell=[str(fv/nx)+'arcsec', str(fv/ny)+'arcsec']

    print 'vis=', vis
    fdspw=get_field_desc(vis)
    ids=c.get_ids()
    pth=c.pull('work_dir')
    msname=vis[str.rfind(vis,'/')+1:]
    if msname.endswith('.ms'):
        msname=msname[:str.rfind(msname, '.ms')]

    if len(fdspw)>len(ids):
        #more job chunks than engines, simply distribute by field and spw
        i=0
        for k in fdspw.values():
            id=ids[i]
            s={}
            s['vis']=vis
            fd=str(k['field'])
            spw=str(k['spw'])
            sl=''
            if not pth[id].endswith('/'):
                sl='/' 
            s['imagename']=pth[id]+sl+msname+'-f'+fd+'-s'+spw
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
            cmd=make_call('clean', s) 
            #print cmd
            do_and_record(cmd, id)
            i+=1
            if i==len(ids):
               i=0
    else:
        #less job chanks than engines, need further devide by channel
        i=0
        for k in fdspw.values():
            spwchan=get_spw_chan(vis, k['spw'])
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
                sl=''
                if not pth[id].endswith('/'):
                    sl='/' 
                s['imagename']=(pth[id]+sl+msname+'-f'+fd+'-s'+spw+
                                '-b'+str(start)+'-e'+str(start+nchan))
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
                cmd=make_call('clean', s) 
                #print cmd
                do_and_record(cmd, id)
                i+=1
                if i==len(ids):
                   i=0
    
    get_status()

###########################################################################
###   ms knowledge functions
###########################################################################
def get_antenna_diam(vis):
    tb.open(vis+'/ANTENNA')
    diams=tb.getcol('DISH_DIAMETER')
    diam=np.min(diams)
    if diam==0:
        diam=np.max(diams)
    tb.done()
    return diam

def get_mean_reff(vis):
    tb.open(vis+'/SPECTRAL_WINDOW')
    reff=tb.getcol('REF_FREQUENCY')
    tb.done()
    return reff.mean()

def get_spw_reff(vis, spw=0):
    tb.open(vis+'/SPECTRAL_WINDOW')
    spw_reff=tb.getcell('REF_FREQUENCY', spw)
    tb.done()
    return spw_reff

def get_spw_chan(vis, spw=0):
    tb.open(vis+'/SPECTRAL_WINDOW')
    spw_chan=tb.getcell('NUM_CHAN', spw)
    tb.done()
    return spw_chan

def get_pol_corr(vis, pol=0):
    tb.open(vis+'/POLARIZATION')
    pol_corr=tb.getcell('NUM_CORR', pol)
    tb.done()
    return pol_corr

def get_num_field(vis):
    tb.open(vis+'/FIELD')
    num_field=tb.nrows()
    tb.done()
    return num_field

def get_num_spw(vis):
    tb.open(vis+'/SPECTRAL_WINDOW')
    num_spw=tb.nrows()
    tb.done()
    return num_spw

def get_num_desc(vis):
    tb.open(vis+'/DATA_DESCRIPTION')
    num_desc=tb.nrows()
    tb.done()
    return num_desc

def get_spw_id(vis, desc=0):
    tb.open(vis+'/DATA_DESCRIPTION')
    spw_id=tb.getcell('SPECTRAL_WINDOW_ID', desc)
    tb.done()
    return spw_id

def get_pol_id(vis, desc=0):
    tb.open(vis+'/DATA_DESCRIPTION')
    pol_id=tb.getcell('POLARIZATION_ID', desc)
    tb.done()
    return pol_id

def get_field_desc(vis):
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
        b[i]['pol']=get_pol_id(vis, b[i]['desc'])
        b[i]['spw']=get_spw_id(vis, b[i]['desc'])
        b[i]['ncorr']=get_pol_corr(vis, b[i]['pol'])
        b[i]['nchan']=get_spw_chan(vis, b[i]['spw'])
        b[i]['cost']=b[i]['nchan']*b[i]['ncorr']*b[i]['nrows']
        d+=b[i]['cost']
    d/=len(b.keys()) 
    for i in b.keys():
        b[i]['cost']=[b[i]['cost']/d, len(b.keys())]
    return b


###########################################################################
###   setup -  
###########################################################################
def init_cluster(clusterfile, project):
    if clusterfile==None or type(clusterfile)!=str or clusterfile.strip()=="":
        print 'cluster file name must be a non-empty string'
        return
    if project==None or type(project)!=str or project.strip()=="":
        print 'project name must be a non-empty string'
        return

    config_cluster(clusterfile, True)
    if not configdone:
        return
    create_project(project)
    stop_nodes()
    stop_resource()
    start_cluster()
    start_monitor()
    start_logger()
    start_resource()

#if __name__ == "__main__":
#    init_cluster('my_cluster', 'my_project')
    #simple_clean(vis='/home/casa-dev-09/hye/ptest/sim.alma.csv.mid.ms', 
    #             nx=256, ny=256, mode='channel')

    #simple_clean(vis='/home/casa-dev-09/hye/para/sim.alma.csv.mid.ms', 
    #             nx=256, ny=256, mode='channel')
  
    #simple_clean(vis='/home/casa-dev-09/hye/pclean/sim100g_4chan15kRows.ms',
    #             nx=256, ny=256, mode='channel')
    
    #check_status()

