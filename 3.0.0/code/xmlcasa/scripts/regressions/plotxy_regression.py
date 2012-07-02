##############################################################################
#                                                                            #
# Test Name:                                                                 #
#    Plotxy Regression Script                                                #
#                                                                            #
# Usage: execfile('plotxy_regression.py')                                    #
#    By default, the regression test runs with:                              #
#        benchmarking=False (does not measure execution time)                #
#        interactive=False (does not stop for user input)                    #
#        currentaffair=False (does full regression)                          #
#        setnewbase=True (save pngs as new base for future regression)       #
#    Switch these parameters before execfile as needed.                      #
#                                                                            #
# This script is to run in a writeable directory (it writes png files).      # 
# It creates a directory named 'plotxy_regression' and three subdirectories  # 
# 'plot' , 'prev' and 'data'.  The 'plot' stores pngs from this regression.  #
# The 'prev' stores pngs from previous regression. The 'data' store the      #
# required test data.                                                        #
#                                                                            #
# The first time running this, it pulls data from known casa/data directory. # 
# It then generates all pngs and save them into 'plot'                       #
#                                                                            #
# It is suggested to run subsequent regression from previously created       #
# 'plotxy_regression' directory. Then (1) it pulls a dataset only if it is   #
# not already in 'data' directory; (2) it generates new pngs and saves to    #
# 'plot'; (3) it compares the corresponding pngs and writes the result;      #
# (4) it saves generated pngs to 'prev' directory as a new baseline          # 
#                                                                            #
# It is often the case that you need to test only the current block issue.   #
# You can do that by having the script in the 'test the current issue'       #
# section and set currentaffair=True before run this script.                 #
#                                                                            #
# example: <CASA>cd <path_to_regression>/plotxy_regression                   #
#          <CASA>interactive=False                                           #
#          <CASA>execfile('<path_to_this_script>/plotxy_regression.py')      # 
#                                                                            #
# Output:                                                                    #
#    plot files - plotxy_regression/plotxy.plot.name.png                     #
#    all other results are written to casalogger                             #
#                                                                            #
# Features Tested:                                                           #
# -1. pull test data sets from repository                                    #
# 0. test the current issue                                                  #
# 1. plotxy for possible xy axes                                             #
# 2. plotxy non-average for possible xy axes                                 #
# 3. plotxy for possible iterationss                                         #
# 4. plotxy average detail calculationn                                      #
# 5. plotxy for multi spectral windows                                       #
# 6. plotxy for multi scan                                                   #
# 7. plotxy for multi spw multi scan                                         #
# 8. plotxy flagging                                                         #
# 9. plotxy data selection                                                   #
# 10. plotxy other issues                                                    #
# 11. mp tool for average plotxys                                            #
# 12. compare png files                                                      #
# 13. save png files for future comparison                                   #
#                                                                            #
# Success/failure criteria:                                                  #
# The stdout should show the number of points being plotted for each test    #
# case. Any exception on stdout indicates a possible problem.                #
#                                                                            #
# Rationale for Inclusion:                                                   #
#    This script is used to test plotxy functionalities                      #
#                                                                            #
##############################################################################
#                                                                            #
# Created 2008-04-17                                                         #
#                                                                            #
##############################################################################

import os
import time
import getopt
import shutil
import sys
import datetime
import inspect
import string
import filecmp

import regression_utility as tstutl

#print sys.argv

# Get path to CASA home directory by stipping name from '$CASAPATH'
pathname=os.environ.get('CASAPATH').split()[0]
#pathname='/users/hye/regression/plotxy/'
pathname=pathname+'/data/regression/plotxy/'

# The testdir where all output files will be kept
testdir='plotxy_regression'
testdata='data'
testplot='plot'
prevplot='prev'
this="plotxy_regression"

currentdir=os.curdir
curpathlist=os.path.abspath(currentdir).split('/')
startdir=curpathlist[len(curpathlist)-1]
#print "currentdir:", currentdir
#print "startdir:", startdir
#print "testdir:", testdir


#tstutl.stop("Test stop")
#start the test from the dir name 'plotxy_regression' 
if (startdir!=testdir):
   #create new testdir
   tstutl.maketestdir(testdir)
   os.chdir(testdir)

#clear all previous plots
tstutl.maketestdir(testplot)

if (not os.path.exists(prevplot)):
   tstutl.maketestdir(prevplot)

if (not os.path.exists(testdata)):
   #create new test data directory
   tstutl.maketestdir(testdata)

# The prefix to use for all output files.
prefix='plotxy.'

# Check whether or not to record the total execution time
global bench
try:
   benchmarking
except NameError:
   bench=False
else:
   if benchmarking:
      bench=True
   else:
      bench=False
 
# Check whether or not to wait for user input
global manual
try:
   interactive
except NameError:
   manual=False
else:
   if interactive:
      manual=True
   else:
      manual=False

global current
try:
   currentaffair
except NameError:
   current=False
else:
   if currentaffair:
      current=True
   else:
      current=False

global savebase
try:
   setnewbase
except NameError:
   savebase=True
else:
   if setnewbase:
      savebase=True
   else:
      savebase=False

# Make up logfile name for this session
#datestring=datetime.datetime.isoformat(datetime.datetime.today())
#outfile=prefix+datestring+'.log'
#logfile=open(outfile,'w')

# Redirect stdout
#saveout=sys.stdout
#saveerr=sys.stderr
#if (not manual):
#   sys.stdout=logfile
#   sys.stderr=logfile

   #Turn off logviewer - no, leave it on, system regression need it
   #for pid in logpid:
   #   #print 'pid: ',pid
   #   os.kill(pid,9)

   #Turn off debug log
   #casalog.filter('DEBUG2') 

def plotfile(testName=""):
    return testplot+"/"+prefix+testName+'.png'

#print out usage
usage="plotxy_regression.py interactive=%s benchmarking=%s currentaffair=%s setnewbase=%s" % (manual, bench, current, savebase)
tstutl.note(usage, "INFO", this)

tstutl.note('*****************************************',"INFO",this)
tstutl.note('******plotxy regression test start ******',"INFO",this)

# Start bench clock
if bench:
    startTime=time.time()
    startProc=time.clock()

##############################################################################
# -1. preparing testing dataset                                              #
##############################################################################

tstutl.note('########## Preparing test data... ##########',"INFO",this)
msList=['NGC5921'
        ,'g19_12coall'
        ,'G24_92A_11' 
        ,'polcal_20041110_cband_vla_calaips'
        ,'3C84' 
        ,'whysong' 
        ,'coma' 
        ,'m87test' 
        ,'n4826_16apr98'
        ,'uid___X1eb_Xa30_X1'
        #,'uid___X1eb_X7888_X1'
        #,'uid___X1eb_Xa885_X1'
        ,'n2403'
        #,'3C273XC1'
        ,'testPhase_sdm' 
       ]

taskname='importuvfits'
default(taskname)

hasTestData=True

for k in range(len(msList)):
   # Set up the MS filename and save as new global variable
   dataName=msList[k]
   vis=testdata+'/'+dataName+'.ms'
   
   if (not os.path.exists(vis)):
      fitsfile=pathname+dataName+'.fits'
      msfile=pathname+dataName+'.ms'
      if (os.path.exists(msfile)):
         tstutl.note('copy msfile: '+msfile+'\n===========>'+vis,"INFO",this) 
         shutil.copytree(msfile, vis) 
      elif (os.path.exists(fitsfile)):
         tstutl.note('import fitsfile: '+fitsfile+'\n===============>'+vis,"INFO",this)
         importuvfits() 
      else:
         tstutl.note(vis+" does not exist.","INFO",this)
         tstutl.note("Could not find "+fitsfile+" at "+pathname,"WARN",this)
         tstutl.note("Could not find "+msfile+" at "+pathname,'WARN',this)
         hasTestData=False
   else:
      tstutl.note(vis+' found',"INFO",this) 

tstutl.note('Done preparing test data',"INFO",this)

# Record import time
if bench:
   importtime=time.time()
   tstutl.note('Total data import time %.2f sec.' % (importtime - startTime),"INFO",this) 

if not hasTestData:
   tstutl.stop('Required test data is not available. plotxy regression cannot continue')
   
taskname='plotxy'

###############################################################################
## 0. test the current issue                                                  #
##   if the currentaffair=true, it stops after done this section of test      # 
##   the test in this section is temporary and changes as neccessary          #
###############################################################################
tstutl.note('########## current issues ##########',"INFO",this)

'''
#test cas-822, crash on iteration
default(taskname)
dataset='3c129_6cm.ms'
vis=testdata+'/'+dataset

#plotxy(vis=msfile, selectdata=True, correlation='RR LL', xaxis='uvdist',
#yaxis='amp', multicolor='both', iteration='antenna', field='0')
yaxis="amp"
xaxis="uvdist"
selectdata=True
correlation='RR LL'
multicolor='both'
iteration='antenna'
antenna="0&1"
field="1"
timebin='0'
width='1'
overplot=False
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin
figfile=plotfile(figname)
tstutl.note('plot '+title,"INFO",this)
plotxy()

'''

'''
#test unable to add a column
default(taskname)
dataset='uid___X1eb_X7888_X1.ms'
vis=testdata+'/'+dataset

xaxis="frequency"
yaxis="amp"
selectdata=True
antenna="0&1"
spw="0"
field="1"
timebin="10000000"
crossscans=True
plotsymbol="-"
plotcolor="blue"
connect="channel"
overplot=False
subplot=211
datacolumn="data"
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin
figfile=plotfile(figname)
tstutl.note('plot '+title,"INFO",this)
plotxy()

yaxis="phase"
subplot=212
plotxy()

default(taskname)
dataset='NGC5921.ms'
vis=testdata+'/'+dataset
timebin='90'
width='8'
showflags=False
interactive=manual
xaxis='azimuth'
yaxis='amp'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin
figfile=plotfile(figname)
tstutl.note('plot '+title,"INFO",this)
plotxy()
'''

#do not remove this if block
if current:
   tstutl.stop("Done testing current issue")

##############################################################################
# 1. plotxy averaged for all possible xy axes                                #
##############################################################################
axisList=['time', 'channel', 'uvdist', 'azimuth', 'elevation',
            'baseline', 'hourangle', 'parallacticangle', 'u', 'v', 'w', 'x',
            'frequency', 'correlation', 'real','imag','amp','phase','weight'
            ,'velocity'
           ]

default(taskname)
dataset='NGC5921.ms'
vis=testdata+'/'+dataset
timebin='90'
width='16'
showflags=False
interactive=manual

tstutl.note('########## possible axes (averaged) ##########','INFO',this)

for k in range(len(axisList)):
   xaxis=axisList[k]
   lyaxis=yaxis
   if (xaxis=='u'):
      lyaxis='v'
   if (xaxis=='x'):
      lyaxis='y'
   selectplot=True
   title=dataset+' '+lyaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
   figname=dataset+'-'+lyaxis+'_'+xaxis+'_'+width+'_'+timebin
   figfile=plotfile(figname)
   tstutl.note('plot '+title,"INFO",this)
   plotxy()

xaxis='real'
yaxis='imag'
selectplot=True
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ 'sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin
figfile=plotfile(figname)
tstutl.note('plot '+title,"INFO",this)
plotxy()

# Record axis plot time
if bench:
   importtime=time.time()
   tstutl.note('Total time for xais plot (averaged): %.2f sec.' % (importtime - startTime),'INFO',this) 

##############################################################################
# 2. plotxy non-average for all possible xy axes                             #
#    this takes a fairly long time                                           #
##############################################################################

axisList=['time', 'channel', 'uvdist', 'azimuth', 'elevation',
            'baseline', 'hourangle', 'parallacticangle', 'u', 'v', 'w', 'x',
            'frequency', 'correlation', 'real','imag','amp','phase','weight'
            ,'velocity'
           ]

default(taskname)
dataset='NGC5921.ms'
vis=testdata+'/'+dataset
timebin='0'
width='1'
showflags=False
interactive=manual

tstutl.note('########## possible axes (non-averaged) ##########','INFO',this)

for k in range(len(axisList)):
   xaxis=axisList[k]
   lyaxis=yaxis
   if (xaxis=='u'):
      lyaxis='v'
   if (xaxis=='x'):
      lyaxis='y'
   selectplot=True
   title=dataset+' '+lyaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
   figname=dataset+'-'+lyaxis+'_'+xaxis+'_'+width+'_'+timebin
   figfile=plotfile(figname)
   plotxy()
   tstutl.note('plot '+title,"INFO",this)

xaxis='real'
yaxis='imag'
selectplot=True
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ 'sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin
figfile=plotfile(figname)
tstutl.note('plot '+title,"INFO",this)
plotxy()

# Record axis plot time
if bench:
   importtime=time.time()
   tstutl.note('Total time for axis plot (non-average): %.2f sec.' % (importtime - startTime),'INFO',this) 


##############################################################################
# 3. plotxy for possible iterationss                                         #
##############################################################################
iterList = ['field', 'antenna', 'baseline', 'scan']#, 'feed'], 'corr']

default(taskname)
dataset='NGC5921.ms'
vis=testdata+'/'+dataset
averagemode='scalar'
xaxis='time'
yaxis='phase'
timebin='90'
width='16'
showflags=False
interactive=manual
subplot=121

tstutl.note('########## possible iterations (averaged) ##########','INFO',this)

for k in range(len(iterList)):
   iteration=iterList[k]
   selectplot=True
   title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ 'sec average)'
   figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_iter_'+iteration
   figfile=plotfile(figname)
   tstutl.note('plot iteratrion: '+iteration+' '+title,'INFO',this) 
   plotxy()
   if (manual==True):
      raw_input('mouse click on NEXT to iterate, press a key when done') 

iteration=''
subplot=111

'''
tstutl.note('########## possible iterations (non-averaged) ##########','INFO',this)
#if 3c129 is in repository, add this for non -averaged
plotxy(vis='data/3c129_6cm.ms', selectdata=True, correlation='RR LL',
xaxis='uvdist',yaxis='amp', multicolor='both', iteration='field')
if (manual==True):
   raw_input('mouse click on NEXT to iterate, press a key when done') 

plotxy(vis='data/3c129_6cm.ms', selectdata=True, correlation='RR LL',
xaxis='uvdist',yaxis='amp', multicolor='both', iteration='baseline')
if (manual==True):
   raw_input('mouse click on NEXT to iterate, press a key when done') 

plotxy(vis='data/3c129_6cm.ms', selectdata=True, correlation='RR LL',
xaxis='uvdist',yaxis='amp', multicolor='both', iteration='scan')
if (manual==True):
   raw_input('mouse click on NEXT to iterate, press a key when done') 

plotxy(vis='data/3c129_6cm.ms', selectdata=True, correlation='RR LL',
xaxis='uvdist',yaxis='amp', multicolor='both', iteration='feed')
if (manual==True):
   raw_input('mouse click on NEXT to iterate, press a key when done') 

plotxy(vis='data/3c129_6cm.ms', selectdata=True, correlation='RR LL',
xaxis='uvdist',yaxis='amp', multicolor='both', iteration='antenna', field='0')
if (manual==True):
   raw_input('mouse click on NEXT to iterate, press a key when done') 

'''
# Record axis plot time
if bench:
   importtime=time.time()
   tstutl.note('Total time for iteration plot (averaged): %.2f sec.' % (importtime - startTime),'INFO',this) 


##############################################################################
# 4. plotxy average detail calculationn                                      #
##############################################################################

binList = ['0', '20', '30', '40', '50', '60', '70']
colorList = ['green', 'blue', 'yellow', 'pink', 'black', 'black', 'black']
symList = ['o', 'o', 'o', 'o', 'x', 'x', 'x']

tstutl.note('########## average detail calculation ##########','INFO',this)

default(taskname)
dataset='3C84.ms'
vis=testdata+'/'+dataset
yaxis='phase'
xaxis='time'
interactive=manual

#select a small section of data
selectdata=true
field='0'
correlation='RR'
spw='0:5~6'
scan=''
feed=''
array=''
uvrange=''
antenna='4&8'
timerange='10:39:00~10:40:00'

#plot the data
averagemode = 'vector'
width = '1'
timebin = '0'
overplot = false
plotsymbol = '.'
plotcolor = 'red'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

#channel average by 2, timeaverage 0-70
width='2'

for k in range(len(binList)):
   timebin=binList[k]
   overplot=true
   plotsymbol=symList[k]
   plotcolor=colorList[k]
   title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
   figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin
   figfile=plotfile(figname)
   tstutl.note('plot '+title,'INFO',this) 
   plotxy()

##############################################################################
# 5. plotxy for multi spectral windows                                      #
##############################################################################

tstutl.note('########## multi spectral windows ##########','INFO',this)

default(taskname)
dataset='testPhase_sdm.ms'
vis=testdata+'/'+dataset
field=''
selectdata=true
correlation=''
antenna=''
timerange=''
yaxis='amp'
xaxis='channel'
averagemode='vector'
overplot=false
plotsymbol='.'
#plotcolor='green'
interactive=manual

#plot the data
averagemode = 'vector'
width = '1'
timebin = '0'
overplot = false
plotsymbol = '.'
plotcolor = 'red'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_chan'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

width = '2'
timebin = '60'
spw = '0,1,2,3'
xaxis='channel'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) spw=0,1,2,3'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'chan_spw=0,1,2,3'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw = '4,5'
xaxis='channel'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) spw=4,5'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'chan_spw=4,5'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw = '6,7'
xaxis='channel'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) spw=6,7'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'chan_spw=6,7'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw = '0,1,2,3'
xaxis='frequency'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) spw=0,1,2,3'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'freq_spw=0,1,2,3'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw = '4,5'
xaxis='frequency'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) spw=4,5'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'freq_spw=4,5'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw = '6,7'
xaxis='frequency'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+' sec average) spw=6,7'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'freq_spw=6,7'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

averagemode = 'vector'
width='1'
timebin='0'
xaxis='channel'
overplot=false
plotsymbol='.'
plotcolor='red'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_chan'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw = '0,1,2,3'
xaxis='channel'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) spw=0,1,2,3'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'chan_spw=0,1,2,3'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw = '4,5'
xaxis='channel'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) spw=4,5'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'chan_spw=4,5'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw = '6,7'
xaxis='channel'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) spw=6,7'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'chan_spw=6,7'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

#tstutl.note('test of non-average freq conversion disabled','WARN',this) 
spw = '0,1,2,3'
xaxis='frequency'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) spw=0,1,2,3'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'freq_spw=0,1,2,3'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw = '4,5'
xaxis='frequency'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) spw=4,5'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'freq_spw=4,5'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw = '6,7'
xaxis='frequency'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) spw=6,7'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'freq_spw=6,7'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

##############################################################################
# 6. plotxy multi scan                                                       #
##############################################################################

tstutl.note('########## multi scan ##########','INFO',this)

default(taskname)
dataset='g19_12coall.ms'
vis=testdata+'/'+dataset
interactive=manual
selectdata = true
correlation = ''
spw = ''
antenna = ''
timerange = ''
yaxis = 'amp'
averagemode = 'vector'
width = '1'
timebin = '0'
subplot=111
overplot = false
plotsymbol = '.'
plotcolor = 'green'

field = '2'
width='2'
selectplot=true

crossscans=false
xaxis='time'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) field=2'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossscans=false
xaxis='time'
timebin='600'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) field=2'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossscans=true
xaxis='time'
timebin='600'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) field=2,crossscans=1'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,crossscans=1'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossscans=false
timebin='60000'
xaxis='time'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) field=2,crossscans=0'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,crossscans=0'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossscans=true
timebin='60000'
xaxis='time'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) field=2,crossscans=1'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,crossscans=1'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossscans=true
timebin='60000'
antenna='2&3'
xaxis='time'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) field=2,crossscans=1,antenna=2&3'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,crossscans=1,antenna=2&3'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

#test cross baselines
default(taskname)
dataset='NGC5921.ms'
vis=testdata+'/'+dataset
interactive=manual
selectdata = true
xaxis='time'
yaxis='amp'
datacolumn='data'
iteration=''
selectdata=True
antenna=''
timerange=''
correlation='RR'
scan=''
feed=''
array=''
uvrange=''
spw='0:50'
field='2'
crossscans=False
averagemode='vector'
width='4'

timebin='600'
crossbls=True
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) field=2,RR,crossbls=1'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,RR,crosssbls=1'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossbls=False
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) field=2,RR,corssbls=0'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,RR,crossscans=0'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

timebin='6000'
crossbls=True
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) field=2,RR,crossbls=1'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,RR,crosssbls=1'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossbls=False
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) field=2,RR,corssbls=0'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,RR,crossscans=0'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

timebin='60000'
crossbls=True
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) field=2,RR,crossbls=1'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,RR,crosssbls=1'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossbls=False
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average) field=2,RR,crossbls=0'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,RR,crossblss=0'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()


##############################################################################
# 8. plotxy multi spw multi scan                                             #
##############################################################################

tstutl.note('########## multi spws multi scan ##########','INFO',this)

default(taskname)
dataset='n4826_16apr98.ms'
vis=testdata+'/'+dataset
selectdata = true
correlation = ''
spw = ''
antenna = ''
timerange = ''
yaxis = 'amp'
averagemode = 'vector'
width = '1'
timebin = '0'
subplot=111
overplot = false
plotsymbol = '.'
plotcolor = 'green'
interactive=manual

selectplot=true

crossscans=false
xaxis='time'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' '+timebin+ ' sec average) crossscans=0'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'crossscans=0'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

field = '2'
spw='2'

crossscans=false
xaxis='time'
width='2'
timebin='6000'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' '+timebin+ ' sec average) field=2,spw=2,crossscans=0'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,spw=2,crossscans=0'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossscans=true
xaxis='time'
timebin='6000'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' '+timebin+ ' sec average) field=2,spw=2,crossscans=1'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,spw=2,crossscans=1'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossscans=false
timebin='60000'
xaxis='time'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' '+timebin+ ' sec average) field=2,spw=2,crossscans=0'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,spw=2,crossscans=0'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossscans=true
timebin='60000'
xaxis='time'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' '+timebin+ ' sec average) field=2,spw=2,crossscans=1'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,spw=2,crossscans=1'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossscans=true
timebin='60000'
antenna='2'
width='4'
xaxis='time'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' '+timebin+ ' sec average) field=2,spw=2,antenna=2,crossscans=1'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,spw=2,antenna=2,crossscans=1'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossscans=true
field='4'
width='16'
antenna='2&3'
xaxis='time'
timebin='6000'
connect='channel'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' '+timebin+ ' sec average) field=2,spw=2,antenna=2,3,crossscans=1'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,spw=2,antenna=2,antenna=2,3,crossscans=1'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossscans=false
timebin='6000'
xaxis='time'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' '+timebin+ ' sec average) field=2,spw=2,antenna=2,3,crossscans=1'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_'+'field=2,spw=2,antenna=2,antenna=2,3,crossscans=1'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

##############################################################################
# 9. plotxy data selection                                                   #
##############################################################################

tstutl.note('########## data selection ##########','INFO',this)

default(taskname)
dataset='testPhase_sdm.ms'
vis=testdata+'/'+dataset
xaxis='channel'
yaxis='amp'
datacolumn='data'
averagemode ='vector'
timebin='100'
crossscans=False
interactive=manual

width='4'
field=''
spw='4:3~51^2,5:7~15^9'
selectdata=True
antenna='1&2'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'carat'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

default(taskname)
dataset='NGC5921.ms'
vis=testdata+'/'+dataset
interactive=manual
xaxis='uvdist'
yaxis='amp'
datacolumn='data'
averagemode='vector'
timebin='100'
width='4'
selectdata=true
uvrange='1~2klambda,3~4klambda'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_uvrange'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

timebin='0'
width='1'
selectdata=true
uvrange='1~2klambda,3~4klambda'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_uvrange'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

#test plotrange 
default(taskname)
dataset='NGC5921.ms'
vis=testdata+'/'+dataset
interactive=manual
xaxis='time'
yaxis='amp'
datacolumn='data'
averagemode='vector'
timebin='0'
width='1'
plotrange=['09:50:24, 10:30:20', 0.02, 0.20]
selectdata=false
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_plotrangetime'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

plotrange=''
selectdata=false
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_plotrangedefault'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

'''
#test selection based on corr type
default(taskname)
dataset='uid___X1eb_Xa885_X1.ms'
vis=testdata+'/'+dataset
interactive=manual
xaxis='channel'
yaxis='amp'
datacolumn='data'
averagemode='vector'
timebin='all'
width='1'
crossscans=true
crossbls=false
spw='0'
selectdata=true
antenna='*&&*'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_2antenna_all_corr'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossbls=true
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_2antenna_all_corr_crossbls'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossbls=false
antenna='*&*'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_2antenna_cross_corr'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

antenna='*&&&'
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_2antenna_self_corr'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

crossbls=true
title=dataset+' '+yaxis+' vs '+xaxis+' ('+width+' chan '+timebin+ ' sec average)'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_2antenna_self_corr_crossbls'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()
'''

##############################################################################
# 10. other issues                                                           #
##############################################################################

tstutl.note('########## other issues ##########','INFO',this)

default(taskname)
dataset='polcal_20041110_cband_vla_calaips.ms'
vis=testdata+'/'+dataset
interactive=manual
xaxis="uvdist"
yaxis="phase"
datacolumn="data"
iteration=""
selectdata=True
antenna=""
field="0137+331"
uvrange=""
timerange="02:37:19~02:37:22"
correlation="RR"
scan=""
feed=""
array=""
averagemode="vector"
width="1"
timebin="0.001"
crossscans=False
plotsymbol="."
plotcolor="blue"
markersize=5.0
linewidth=1.0
connect="none"
plotrange=[0,600,-180,180]
multicolor="corr"
selectplot=True
overplot=False
newplot=False
clearpanel="Auto"
skipnrows=1
xlabels=""
ylabels=""
fontsize=10.0
windowsize=1.0
showflags=False
interactive=manual


spw=""
subplot=121
overplot=false
title='spw=,averaged,uvdist'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_uvdist'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw="0,1"
subplot=122
overplot=false
title='spw=0,1,averaged,uvdist'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_uvdist_spw01'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw='0'
subplot=121
overplot=false
plotcolor='red'
title='spw=0,averaged,uvdist'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_uvdist0'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw='1'
subplot=122
overplot=false
title='spw=1,averaged,uvdist'
plotcolor='blue'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_uvdist1'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw='1'
subplot=121
overplot=false
title='spw=1,averaged,uvdist'
plotcolor='blue'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw='0'
subplot=121
overplot=true
plotcolor='red'
title='spw=0-1,averaged,uvdist'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_uvdist0over1'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw='0'
subplot=122
overplot=false
plotcolor='red'
title='spw=0,averaged,uvdist'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_uvdist'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw='1'
subplot=122
overplot=true
plotcolor='blue'
title='spw=1-0,averaged,uvdist'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_uvdist1over0'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

timebin='0'
width='1'
spw='0,1'
subplot=121
overplot=false
plotcolor='blue'
title='spw=1,0,non-averaged,uvdist'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_uvdist'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw='0'
subplot=122
overplot=false
plotcolor='blue'
title='spw=1,0,non-averaged,uvdist'
#figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_uvdist'
#figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

spw='1'
subplot=122
overplot=true
plotcolor='red'
title='spw=1,0,non-averaged,uvdist'
figname=dataset+'-'+yaxis+'_'+xaxis+'_'+width+'_'+timebin+'_uvdist1over1'
figfile=plotfile(figname)
tstutl.note('plot '+title,'INFO',this) 
plotxy()

##############################################################################
# 11. use mp tool for average plotxys                                        #
##############################################################################
dataset='NGC5921.ms'
vis=testdata+'/'+dataset
mp.open(vis)

tstutl.note('########## use mp tool (averaged) ##########','INFO',this)

plotList = ['vischannel', 'visfreq', 'vistime', 'uvdist', 
            'uvcoverage', 'viscorr', 'visvelocity', 'hourangle', 
            'azimuth', 'elevation', 'parallacticangle', 'weight'
           ]
width='4'
timebin='90'
averagemode='vector'
avevel=false
for k in range(len(plotList)):
   plotname=plotList[k]
   if (plotname=='visvelocity'):
      avevel=true
   mp.setdata()
   mp.avedata(averagemode='vector', averagetime=timebin, averagechan=width, averagevel=avevel)
   title=dataset+' '+plotname+' ('+width+' chan '+timebin+ ' sec average)'
   figname=dataset+'_'+plotname+'_'+width+'_'+timebin+'.png'
   figfile=plotfile(figname)
   tstutl.note('plot '+title,'INFO',this) 
   mp.plot(plotname)
   mp.savefig(figfile)

mp.closeMS()
mp.close()

##############################################################################
# 12. diff png files                                                         #
##############################################################################

tstutl.note('########## diff png files ##########','INFO',this)

prevlist=os.listdir(prevplot)
thislist=os.listdir(testplot)
oldplot=len(prevlist)
newplot=len(thislist)
tstutl.note("There are %s plots in prev directory (previous regression)" % oldplot,'INFO',this)
tstutl.note("There are %s plots in plot directory (this regression)" % newplot,'INFO',this)

if (newplot==0 or oldplot==0):
   tstutl.note("There is no png files to compare","INFO",this)
else:
   
   cmplist=[]
   
   for k in range(len(prevlist)):
      pplot=prevlist[k]
      hasnew=False
      for j in range(len(thislist)):
         if (thislist[j]==pplot):
            hasnew=True
            break
      if (not hasnew):
         tstutl.note(pplot+' is not in %s directory' % testplot,'WARN',this)
      else:
         cmplist.append(pplot)
   
   for k in range(len(thislist)):
      pplot=thislist[k]
      hasold=False
      for j in range(len(prevlist)):
         if (prevlist[j]==pplot):
            hasold=True
            break
      if (not hasold):
         tstutl.note(pplot+' is not in %s directory' % prevplot,'WARN',this)
      else:
         already=False
         for j in range(len(cmplist)):
            if (cmplist[j]==pplot):
               already=True
               break;
         if (not already):
            cmplist.append(pplot)
   
   if (len(cmplist)==0):
      tstutl.note("There is no same png files to compare")
   else:
      tstutl.note("There are %s png files to compare" % len(cmplist))
   
   for k in range(len(cmplist)):
      pplot=testplot+'/'+cmplist[k]
      nplot=prevplot+'/'+cmplist[k]
      if (filecmp.cmp(nplot,pplot)):
         tstutl.note(cmplist[k]+' is same','INFO',this)
      else:
         tstutl.note(cmplist[k]+' differs ','WARN',this)
   
##############################################################################
# 13. save png files                                                          #
##############################################################################

#everything should be good if it reaches here
if (savebase and os.path.exists(testplot)):
   if (os.path.exists(prevplot)):
      tstutl.cleanup(prevplot)
   shutil.copytree(testplot, prevplot) 
   tstutl.note("save plot files to prev as a new baseline",'INFO',this)
   
##############################################################################
#done with all the test 
if bench:
    endTime = time.time()
    endProc = time.clock()
    tstutl.note('Total wall clock time %.2f sec.' % (endTime - startTime),'INFO',this)
    tstutl.note('Total CPU        time %.2f sec.' % (endProc - startProc),'INFO',this)

tstutl.note('******plotxy regression test finish******','INFO',this)
tstutl.note('*****************************************','INFO',this)

# Restore stdout and stderr
#sys.stdout=saveout
#sys.stderr=saveerr
#logfile.close()


##############################################################################
'''
import regression_utility as tstutl
this='plotxy_regression'
taskname='plotxy'
testdata='data'
testplot='plot'
prefix='plotxy.'
logfile=sys.stdout
manual=True
def plotfile(testName=""):
    return testplot+"/"+prefix+testName+'.png'
'''

