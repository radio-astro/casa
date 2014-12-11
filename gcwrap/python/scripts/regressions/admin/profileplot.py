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

testname=sys.argv[1]
RESULT_DIR=sys.argv[2]
webpage=sys.argv[3]
asciidata=sys.argv[4]
casapy_pid = sys.argv[5]

def handler(signum, frame):
    try:
        fd.close()  # might not be open yet
    except:
        pass

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
    pl.title('memory usage of casapy for '+testname)

    #s="test-plot.ps" change to PNG for web browser compatibility
    #s="test-plot.png"

    #set up special images directory?
    if os.path.basename(myf['RESULT_DIR']) == myf['testname']:
        png_path=myf['RESULT_DIR']
    else:
        png_path=myf['RESULT_DIR']+time.strftime('/%Y_%m_%d')

    png_filename=png_path +'/'+myf['testname']+'_profile.png'
    if not os.path.isdir(png_path):
        os.mkdir(png_path)

    pl.savefig(png_filename);
    ht=htmlPub(myf['webpage'], 'Memory profile of '+myf['testname'])
    body1=['<pre>Memory profile of run of test %s at %s </pre>'%(myf['testname'],time.strftime('%Y/%m/%d/%H:%M:%S'))]
    body2=['']
    ht.doBlk(body1, body2,myf['testname']+'_profile.png', 'Profile')
    ht.doFooter()

    sys.stdout.flush()

    sys.exit()
    return

# return memory usage in KB
def getmem(pid):
    # First get memory info

    lsof = "/usr/sbin/lsof"
    if not os.path.isfile(lsof):
        lsof = "/usr/bin/lsof"
    if not os.path.isfile(lsof):
        print "Warning: Could not find lsof at /usr/sbin/lsof or /usr/bin/lsof"

    (errorcode, numoffile) = commands.getstatusoutput(lsof + ' -p ' + str(pid) + ' 2>/dev/null | wc -l')
    if errorcode != 0:
        numoffile = -1

    (errorcode, mem_virtual)  = commands.getstatusoutput('env -i ps -p ' + str(pid) + ' -o vsz | tail -1')
    if errorcode != 0:
        mem_virtual = -1
    (errorcode, mem_resident) = commands.getstatusoutput('env -i ps -p ' + str(pid) + ' -o rss | tail -1')
    if errorcode != 0:
        mem_resident = -1

    # Get CPU usage.
    # top in batch mode doesn't give accurate numbers
    # with just 1 iterations, so do 2 iterations with 1 sec
    # delay

    if os.uname()[0] == "Linux":
        cmd = 'top -b -d 1 -n2 | grep -E "^Cpu" | tail -1'
        (errorcode, a) = commands.getstatusoutput(cmd)   
        if len(a)==0 or errorcode != 0:
            return -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0
        
        cpu_us = re.sub('.*[ :]', '', re.sub('%.*', '', a.split(',')[0]))
        cpu_sy = re.sub('.*[ :]', '', re.sub('%.*', '', a.split(',')[1]))
        cpu_id = re.sub('.*[ :]', '', re.sub('%.*', '', a.split(',')[3]))
        cpu_wa = re.sub('.*[ :]', '', re.sub('%.*', '', a.split(',')[4]))

    elif os.uname()[0] == "Darwin":
        cmd = 'top -l 2 -s 1 | grep -Eo "CPU.usage.*" | tail -1'
        (errorcode, a) = commands.getstatusoutput(cmd)
        if len(a)==0 or errorcode != 0:
            return -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0
        
        cpu_us = re.sub('.*[ :]', '', re.sub('%.*', '', a.split(',')[0]))
        cpu_sy = re.sub('.*[ :]', '', re.sub('%.*', '', a.split(',')[1]))
        cpu_id = re.sub('.*[ :]', '', re.sub('%.*', '', a.split(',')[2]))
        cpu_wa = 0 # CPU wait not available from top on Mac
    else:
        raise Exception, "Unrecognized os.uname()[0] = " + str(os.uname()[0])

    return int(mem_virtual), int(mem_resident), numoffile, \
           cpu_us, cpu_sy, cpu_id, cpu_wa



signal.signal(signal.SIGHUP, handler)
signal.signal(signal.SIGTERM, handler)
signal.signal(signal.SIGINT, handler)

mylabel="memory footprint of casapy"
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
	y1,y2,nfile, cpu_us, cpu_sy, cpu_id, cpu_wa = getmem(casapy_pid)
    	if (y1 > 0.0):
        	time.sleep(2.0)
                tt = time.time()-t1
	        t.append(tt)

		y11.append(y1)
		y22.append(y2)
                numfile.append(nfile)
                fd.write(str(int(round(tt, 0))) + ' ' + \
                         str(int(round(y1/1024, 0))) + ' ' + \
                         str(int(round(y2/1024, 0))) + ' ' + \
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
	
