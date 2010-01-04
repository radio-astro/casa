import signal
import os
import pylab as pl
import string
import commands
import sys
import re
import time
import inspect
from tw_utils import *
from matplotlib.font_manager import  FontProperties

#### usage: python profileplot.py testname RESULT_DIR \
####               webpage_abspath asciidata processname 
t=[0]
y11=[0]
y22=[0]
numfile=[0]

testname='unknown_test'
testname=sys.argv[1]
RESULT_DIR=sys.argv[2]
webpage=sys.argv[3]
asciidata=sys.argv[4]
procnam='casapy'
if(len(sys.argv)>5):
    procnam=sys.argv[5]

def handler(signum, frame):
    try:
        fd.close()  # might not be open yet
    except:
        pass

    print 'saving profile plot'
    a=inspect.stack()
    stacklevel=0
    for k in range(len(a)):
        if (string.find(a[k][1], 'profileplot.py') > 0):
            stacklevel=k
            break
    myf=sys._getframe(stacklevel).f_globals
#    myf=frame.f_globals
    t=myf['t']
    y11=myf['y11']
    y22=myf['y22']
    numfile=myf['numfile']
    pl.plot(t,y11,lw=2)
    pl.plot(t,y22,lw=2)
    if max(y11)>=max(y22):
        pl.axis([0.9*min(t),1.1*max(t),0.9*min(y11),1.1*max(y11)])
    else:
        pl.axis([0.9*min(t),1.1*max(t),0.9*min(y22),1.1*max(y22)])
    pl.xlabel('time (sec)')
    pl.ylabel('memory footprint') #note virtual vs. resident
    font=FontProperties(size='small')
    pl.legend(('virtual','resident'),loc=[0.7,0.85], prop=font)
    ax2 = pl.twinx()
    pl.ylabel('No of open File Descriptors')
    ax2.yaxis.tick_right()
    pl.plot(t,numfile, 'r-.',lw=2)
    pl.legend(['No. of Open FDs'],loc=[0.7,0.8], prop=font)
    pl.title('memory usage of '+myf['procnam']+' for '+testname)

    #s="test-plot.ps" change to PNG for web browser compatibility
    #s="test-plot.png"

    #set up special images directory?
    png_filename=myf['RESULT_DIR']+time.strftime('/%Y_%m_%d/')+myf['testname']+'_profile.png'
    if not os.path.isdir(myf['RESULT_DIR']+time.strftime('/%Y_%m_%d/')):
        os.mkdir(myf['RESULT_DIR']+time.strftime('/%Y_%m_%d/'))

    pl.savefig(png_filename);
    ht=htmlPub(myf['webpage'], 'Memory profile of '+myf['testname'])
    body1=['<pre>Memory profile of run of test %s at %s </pre>'%(myf['testname'],time.strftime('%Y/%m/%d/%H:%M:%S'))]
    body2=['']
    ht.doBlk(body1, body2,myf['testname']+'_profile.png', 'Profile')
    ht.doFooter()

    print 'finished!\nfile written to %s and %s' % (png_filename,asciidata)
    sys.stdout.flush()

    sys.exit()
    return
def getmem(procName='') :
    # First get memory info
    
    cmd='top -n 1 -b -i | grep ' + procName
    #a=commands.getoutput(cmd)   #fixme: top command doesn't work on Mac
    (errorcode, a) = commands.getstatusoutput(cmd)   
    if(len(a)==0 or errorcode != 0) :
        return -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0
    pid = string.split(a)[0]
    if os.path.isfile('/usr/sbin/lsof'):
        numoffile=string.atoi(commands.getoutput('/usr/sbin/lsof -p '+pid+' 2>/dev/null | wc -l'))
    else:
        numoffile=string.atoi(commands.getoutput('/usr/bin/lsof -p '+pid+' 2>/dev/null | wc -l'))
    memstr0=string.atof(string.split(string.split(a)[4],'m')[0])*1024.0*1024.0
    if(len(string.split(string.split(a)[5],'m')) > 1) :
        memstr1=string.atof(string.split(string.split(a)[5],'m')[0])*1024.0*1024.0
    elif(len(string.split(string.split(a)[5],'g')) > 1) :
        memstr1=string.atof(string.split(string.split(a)[5],'g')[0])*1024.0*1024.0*1024.0
    elif(len(string.split(string.split(a)[5],'k')) > 1) :
         memstr1=string.atof(string.split(string.split(a)[5],'g')[0])*1024.0
    else:
        memstr1=string.atof(string.split(a)[5])


    # Get CPU usage.
    # top in batch mode doesn't give accurate numbers
    # with just 1 iterations, so do 2 iterations with .5 sec
    # delay
    cmd='top -b d 0.5 -n2 | grep "^Cpu" | tail -1'
    (errorcode, a) = commands.getstatusoutput(cmd)   

    cpu_us = re.sub('.*[ :]', '', re.sub('%.*', '', a.split(',')[0]))
    cpu_sy = re.sub('.*[ :]', '', re.sub('%.*', '', a.split(',')[1]))
    cpu_id = re.sub('.*[ :]', '', re.sub('%.*', '', a.split(',')[3]))
    cpu_wa = re.sub('.*[ :]', '', re.sub('%.*', '', a.split(',')[4]))

    if len(a)==0 or errorcode != 0:
        return -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0

    return memstr0, memstr1, numoffile, \
           cpu_us, cpu_sy, cpu_id, cpu_wa



signal.signal(signal.SIGHUP, handler)
signal.signal(signal.SIGTERM, handler)
signal.signal(signal.SIGINT, handler)

mylabel="memory footprint of "+procnam
t1=time.time()
y1=0.5
y2=0.5

# dump data to ascii file
# (doing so in the signal handler might be too late
#  because the data will be read from publish_summary)
fd = open(asciidata, "w")
fd.write('#time(s) memory_virtual(Mbytes) memory_resident(Mbytes) no_filedesc CPU_user CPU_system CPU_idle CPU_wait\n')

i=0

while True:
	y1,y2,nfile, cpu_us, cpu_sy, cpu_id, cpu_wa = getmem(procnam)
    	if (y1 > 0.0):
        	time.sleep(2.0)
                tt = time.time()-t1
	        t.append(tt)
        	if y1 > 2.5e9: y1=2.5e9
        	if y2 > 2.5e9: y2=2.5e9
		y11.append(y1)
		y22.append(y2)
                numfile.append(nfile)
                fd.write(str(int(round(tt, 0))) + ' ' + \
                         str(int(round(y1/1000000, 0))) + ' ' + \
                         str(int(round(y2/1000000, 0))) + ' ' + \
                         str(nfile) + ' ' + \
                         str(cpu_us) + ' ' + \
                         str(cpu_sy) + ' ' + \
                         str(cpu_id) + ' ' + \
                         str(cpu_wa) + '\n')
                fd.flush()
	else:

		#sleep(10.0)
		time.sleep(3.0)
	i+=1
	
