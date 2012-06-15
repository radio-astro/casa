#finished 7-10
from pylab import *
import string
import commands
import sys
from time import *
from os import *

ioff() #turn interactive plotting mode off
mem1=list()
m=0
procnam="valgrind"
procnam=sys.argv[1]

def getmem(procName='') :
    cmd='top -n 1 -b -i | grep ' + procName
    a=commands.getoutput(cmd)
    if(len(a)==0) :
        return -1.0,-1.0
    memstr0=string.atof(string.split(string.split(a)[4],'m')[0])*1024.0*1024.0
    if(len(string.split(string.split(a)[5],'m')) > 1) :
        memstr1=string.atof(string.split(string.split(a)[5],'m')[0])*1024.0*1024.0
    elif(len(string.split(string.split(a)[5],'g')) > 1) :
        memstr1=string.atof(string.split(string.split(a)[5],'g')[0])*1024.0*1024.0*1024.0
    elif(len(string.split(string.split(a)[5],'k')) > 1) :
         memstr1=string.atof(string.split(string.split(a)[5],'g')[0])*1024.0
    else:
        memstr1=string.atof(string.split(a)[5])
    return memstr0, memstr1

mylabel="memory footprint of "+procnam
t1=time()
y1=0.5
y2=0.5

t=[0]
y11=[0]
y22=[0]

i=0

#while (1) :
#while i<100:
while i< 10000:
	y1,y2=getmem(procnam)
    	if (y1 > 0.0):
        	sleep(2.0)
	        t.append(time()-t1)
        	if( y1 > 2.5e9): y1=2.5e9
        	if(y2 > 2.5e9): y2=2.5e9
		y11.append(y1)
		y22.append(y2)
	else:
       		print 'no ', procnam
		#sleep(10.0)
		sleep(3.0)
	i+=1
	print i
plot(t,y11,lw=2)
plot(t,y22,lw=2)
if max(y11)>=max(y22): axis([0.9*min(t),1.1*max(t),0.9*min(y11),1.1*max(y11)])
else: axis([0.9*min(t),1.1*max(t),0.9*min(y22),1.1*max(y22)])

xlabel('time (sec)')
ylabel('memory footprint') #note virtual vs. resident
legend(('virtual','resident'))
title('memory usage of '+procnam)

#s="test-plot.ps" change to PNG for web browser compatibility
#s="test-plot.png"

#set up special images directory?
s='IMAGES'
if access(s,F_OK) is False:
        print s+' directory DNE, so am making one!'
        mkdir(s)
else: print s+' directory exists; will add to it!'

#label image according to time created
t=gmtime( time() )
s+=('/%s-%s-%s-%s-%s.png') %(t[0],t[1],t[2],t[3],t[4])
print 'fin-ished!\nfile written to %s'%(s)
savefig(s);
show() #not really necessary
