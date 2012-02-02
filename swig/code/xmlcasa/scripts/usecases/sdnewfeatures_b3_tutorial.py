# This script shows examples of features
# added recently (mostly) for P3 developement
#
# 2008-09-25 TT 
#

# load single dish module
asap_init()

import os
import sys


# include total power data analysis demo
tpdemo = True

# include total power plotting demo using sdplot
# FOR THE DEMO DATA SET USED HERE, PLOTTING
# WILL BE VERY SLOW. 
# To skip this set tpdemo2=False
tpdemo2 = False 

#get test data
#default datapaths for the two datasets used in this script
#datasets can be obtained via casa data respository (see casa web page)
datapath1 = os.environ['CASAPATH'].split()[0]+'/data/regression/ATST5/OrionS/'
datapath2 = os.environ['CASAPATH'].split()[0]+'/data/alma/atf/sd/'
datafile1 = 'OrionS_rawACSmod'
datafile2='uid___X1e1_X3197_X1.ms'

print "*******SD analysis demo for Beta Patch 3*******"
print "This script shows new/modified feautures of single dish tools/tasks\n"

#clean up outputs from previous runs
files=['orion_pscal', 'orion_pscal_if0_3000_5000', 'orion_pscal_bs_if0_blparam.txt',
'orion_pscal_bs_if0','orion_pscal_bs_if0_f', 'orion_pscal_bs_if0_f_f','orion_pscal_allif_tave',
'orion_pscal_scaled1.5']
for f in files:
    if os.path.isdir(f) or os.path.isfile(f):
        os.system('rm -rf %s' % f)
       
if os.path.isdir(datapath1+datafile1):
    os.system('mkdir %s;cp -r %s/[!.svn]*  ./%s' % (datafile1, datapath1+datafile1, datafile1))
else:
    if not os.path.isdir(datafile1):
       print "Data file, %s, not found." % datafile1
       sys.exit() 
if os.path.isdir(datapath2+datafile2):
    os.system('mkdir %s;cp -r %s/[!.svn]* ./%s' % (datafile2, datapath2+datafile2, datafile2))
else:
    if not os.path.isdir(datafile2):
       print "Data file, %s, not found." % datafile2
       print "total power data analysis will be skipped"
       tpdemo = False
        

################
# 1. sdaverage #
################
print "###############\n 1. sdaverage \n###############"
default(sdaverage)
desc="* At first, run calibration for the position switch observation\n" \
     "* (taken from ori_hc3n_task_regression.py) to get calibrated data\n"
print desc
sdfile='OrionS_rawACSmod'
fluxunit='K'
calmode='ps'
scanlist=[20,21,22,23]
iflist=[0,1,2,3]
scanaverage=False
timeaverage=True # average in time
tweight='tintsys' # weighted by integ time and Tsys for time averaging
polaverage=True  # average polarization
pweight='tsys'   # weighted by Tsys for pol. averaging
tau=0.09         # do opacity correction
outfile='orion_pscal'
overwrite=True
plotlevel=1
inp(sdaverage)
pause=raw_input('* Hit Return to continue ')

print "\n* Run sdaverage with these parameters...\n"
sdaverage()
print "*** Done sdaverage ****\n"
desc="\n" \
     "* NEW FEATURE: channelrange\n" \
     "* now try new parameter, channelrange to save a subset of the spectra\n" \
     "* with restricted channel range. We do the same calibration as the previous\n" \
     "* sdaverage run but select IF0 only and set a channel range from 3000 to 5000\n"
print desc
iflist=[0]
channelrange=[3000,5000]
outfile='orion_pscal_if0_3000_5000'
inp(sdaverage)
pause=raw_input('* Hit Return to continue ')

print "\n* Run sdaverage with these parameters...\n"
sdaverage()
print "*** Done sdaverage ****\n"

pause=raw_input('* Hit Return to continue ')

desc="\n" \
     "NEW FEATURE: multi-resolution spectra averaging\n"\
     "Set timeaverage=True and averageall=True.  \n" \
     "SKIPPED: This is not test here since we do not have a good test data now\n"
print desc

pause=raw_input('\n* Hit Return to continue ')
#################
# 2. sdbaseline #
#################
print "\n###############\n 2. sdbaseline \n###############"
desc="\n" \
     "* NEW FEATURE: set mask for baseline fitting interactively.\n" \
     "* Set blmode='interact'\n"
print desc
# NEW FEATURE:
# Set mask(s) for baseline fitting interactively

default(sdbaseline)
# single panel case
sdfile='orion_pscal'
specunit='channel'
iflist=0
blmode='interact'
outfile=sdfile+'_bs'+'_if0'
plotlevel=1
inp(sdbaseline)
pause=raw_input('\n* Hit Return to continue ')
print "\n* Run sdbaseline...\n"
desc="\n" \
     "* By using mouse bottons, specify region(s).\n" \
     "* To mask (included in baseline fitting) using\n"\
     "* the left mouse button or to unmask (exculded in baseline fitting)\n" \
     "* using the right mouse button.\n" \
     "*\n" \
     "* For the example here, initially no masklist was given in the task parameter,\n" \
     "* which will be defaulted to put mask to entire spectrum, shown in the plot in yellow.\n" \
     "*\n" \
     "* Using right mouse button to unmask the line region...\n" \
     "* Once you satisfy with your masks, go back to the casapy console, \n" \
     "* and hit Return key to start processing.\n"
print desc
sdbaseline()
print "*** Done sdbaseline ****\n"

# you will see something like this...
#UNmask:  [3753.4292059553341, 4415.2132258064503]
#The current mask window unit is channel
#The current mask window unit is channel
#
#final mask list (Channel) = [[0.0, 3753.0], [4416.0, 8191.0]]
#The current mask window unit is channel
#The current mask window unit is channel

#--------------------------------------------------
#  rms
#--------------------------------------------------
#Scan[0] (OrionS_ps) Time[2006/01/19/01:52:05]:
# IF[0] = 0.166
#--------------------------------------------------
print "* Saved the fit paramters and rms after baseline fit"
print "!cat %s" % outfile+'_blparam.txt' 
# fitting parameters are stored in
# <outfile>_blparam.txt
os.system('cat %s' % outfile+'_blparam.txt')
#IPython system call: cat orion_hc3n_pscal_bs_blparam.txt
#############################################################
#Source Table: orion_pscal
# Output File: orion_pscal_bs_if0
#   Flux Unit: K
#     Abcissa: channel
#   Fit order: 5
#    Fit mode: interact
#
#   Fit Range: [[0.0, 3753.0], [4416.0, 8191.0]]
#############################################################
#------------------------------------------------------------
# Scan[0] Beam[0] IF[0] Pol[0] Cycle[0]:
#Baseline parameters
#  p0= 2.763137,  p1= 0.000157,  p2= -0.000000,  p3= 0.000000,  p4= -0.000000,  p5= 0.000000
#Results of baseline fit
#  rms = 0.166347
#------------------------------------------------------------


pause=raw_input('\n* Hit Return to continue ')
print "\n###############\n 3. sdflag \n###############"
#############
# 3. sdflag #
#############
#flag
desc="\n" \
     "* Flag\n" \
     "* use the baselined data in the previous step.\n" \
     "* set plotlevel=1 to plot the spectrum with flagged region(s).\n"
print desc
default(sdflag)
sdfile='orion_pscal_bs_if0'
specunit='channel'
maskflag=[[0,100],[2000,3000]]
flagmode='flag'
plotlevel=1 
inp(sdflag)
pause=raw_input('\n* Hit Return to continue ')
print "* Run sdflag...\n"
print "* Type y when asked to apply flag." 

sdflag()
print "*** Done sdflag ****\n"
# output->orion_pscal_bs_if0_f

desc="\n" \
     "* NEW FEATURE: restore\n" \
     "* get flag masks applied to the data"
print desc      
# NEW FEATURE:
# check flagged region(s)  
default(sdflag)
sdfile='orion_pscal_bs_if0_f'
flagmode='restore'
inp(sdflag)
pause=raw_input('\n* Hit Return to continue ')
print "\n* Run sdflag...\n"
sdflag()
print "* You will see all flag masks applied to the data...\n"
print "*** Done sdflag ****\n"
#--------------------------------------------------
#History of channel flagging:
#          DATE: 2008/10/03 10:42:01
#    APPLIED TO: SCANS=ALL IFS=ALL POLS=ALL
#       FLAGGED: MODE='flag' MASK=[[0, 100], [2000, 3000]]
#
#--------------------------------------------------
#SCAN[0] IF[0] POL[0]: Current mask is  [[0, 100], [2000, 3000]]
#  Out[30]: [[[0, 100], [2000, 3000]]]

#unflag
desc="\n" \
     "* Try unflag...\n" \
     "* Set flagmode='unflag' \n" 
print desc
sdfile='orion_pscal_bs_if0_f'
flagmode='unflag'
maskflag=[2000,3000]
plotlevel=1
inp(sdflag)
pause=raw_input('\n* Hit Return to continue ')
print "\n* Run sdflag...\n"
print "* Type y when asked to apply flag." 
sdflag()
print "*** Done sdflag ****\n"

pause=raw_input('\n* Hit Return to continue ')
#############
# 4. sdplot #
#############
print "\n###############\n 4. sdplot \n###############"
desc="\n" \
     "* First, generate calibrated data without polarization average.\n" \
     "* This time use sdcal.\n"
print desc
#
# Following step is just to save default setting.
# This is currently needed to restore the default setting
# since some plot control parameters set globally.
#
#default_colormap=sd.plotter._plotter.colormap
#default_ls=sd.plotter._plotter.linestyles
#lsalias=sd.plotter._plotter.lsalias
#default_linestyles=[]
#for lstyle in default_ls:
#   for key,val in lsalias.iteritems():
#      if lstyle==val: default_linestyles.append(key)

default(sdcal)
sdfile='OrionS_rawACSmod'
calmode='ps'
scanlist=[20, 21, 22, 23]
iflist=[0, 1, 2, 3]
average=True
timeaverage=True
tweight='tintsys'
tau=0.09
blmode='auto'
outfile='orion_pscal_allif_tave'
inp(sdcal)
pause=raw_input('\n* Hit Return to continue ')
print "\n* Run sdcal...\n"
sdcal()
print "*** Done sdcal ****\n"

desc="\n" \
     "* NEW FEATURE: plot color control\n" \
     "* specify colors to be used for plot lines\n" \
     "* stacking IFs, different panel for each polarization.\n"
print desc

default(sdplot)
sdfile='orion_pscal_allif_tave'
stack='i'
panel='p'
flrange=[-1,1.5]
sprange=[2500,4500]
colormap='c m g b' # ='cyan, magenta, green, blue'
inp(sdplot)
pause=raw_input('\n* Hit Return to continue ')
print "\n* Run sdplot...\n"
sdplot()
print "*** Done sdplot ****\n"

desc="\n" \
     "* NEW FEATURE:\n"\
     "* click left mouse button on one of the lines, a pop-up window will\n" \
     "* appear to display spectral data values as you move the mouse along\n" \
     "*  horizontal direction while keep holding the left mouse button.\n" 
print desc

pause=raw_input('\n* Hit Return to continue ')

desc="\n" \
     "* NEW FEATURE: line styles \n" \
     "* specify line styles to be used for plot line. \n"
print desc
colormap='none'
linestyles='line dotted dashed dashdot'
linewidth=2
pollist=0
sprange=[2500,4500]
inp(sdplot)
pause=raw_input('\n* Hit Return to continue ')
print "\n* Run sdplot...\n"
sdplot()
print "*** Done sdplot ****\n"

desc="\n" \
     "* Currently flrange, and sprange are set globally. \n" \
     "* For now, to reset, you need to run sdplot with \n" \
     "* default parameters.\n"
print desc

default(sdplot)
sdfile='orion_pscal_allif_tave'
iflist=0
pollist=0
print "* Run sdplot...\n"
sdplot()
print "*** Done sdplot ****\n"

# reset colormap and linestyles
#sd.plotter.set_colors(default_colormap)
#sd.plotter.set_linestyles(linestyles=default_linestyles,linewidth=1)
############################

if tpdemo2:
    desc="\n" \
     "* NEW FEATURE: total power plotting\n" \
     "* (NOTE: functionality is still limited and processing speed is slow)\n" \
     "* This example needs the ATF total power data converted from ASDM to MS. \n" \
     "* Also, currently most of the plot control parameters are no effect or \n" \
     "* ignored in total power plotting mode. \n"
    print desc
    sdfile='uid___X1e1_X3197_X1.ms'
    plottype='totalpower'
    inp(sdplot)
    pause=raw_input('\n* Hit Return to continue ')
    print "\n* Run sdplot...\n"
    sdplot()
    print "*** Done sdplot ****\n"

###########
# sdscale #
###########
print "###############\n 5. sdscale \n###############"
desc="\n" \
     "*  NEW FEATURE: print out Tsys values before and after scaling\n"
print desc
default(sdscale)
sdfile='orion_pscal'
factor=1.5
inp(sdscale)
pause=raw_input('\n* Hit Return to continue ')
print "\n* Run sdscale...\n"
sdscale()
print "*** Done sdscale ****\n"
#Scaled spectra and Tsys by 1.5
#--------------------------------------------------
# Original Tsys
#--------------------------------------------------
#Scan[0] (OrionS_ps) Time[2006/01/19/01:52:05]:
# IF[0] = 118.099
#--------------------------------------------------
#
#--------------------------------------------------
# Scaled Tsys
#--------------------------------------------------
#Scan[0] (OrionS_ps) Time[2006/01/19/01:52:05]:
# IF[0] = 177.149
#--------------------------------------------------


pause=raw_input('\n* Hit Return to continue ')
##########
# sdstat #
##########
print "\n###############\n 6. sdstat \n###############"
desc="\n" \
     "* NEW FEATURE: statistics to an ASCII file \n" \
     "* Specify by statfile parameter\n"
print desc
# 2. an option to set mask for evalulating statistics interactively
# 3 integrated intensity 
#   currently only show in the screeen but not included in 
#   returned dictionary
default(sdstat)
sdfile='orion_pscal'
specunit='GHz'
iflist=0
interactive=True
statfile='orion_stat.txt'
inp(sdstat)
pause=raw_input('\n* Hit Return to continue ')
print "* Run sdstat...\n"
desc="\n" \
     "* Initial mask is set to the entire spectrum\n" \
     "* as shown in yellow. The behavior for multi-IF\n" \
     "* cases is same as that of sdbaseline.\n"\
     "* Use right mouse botton to deselect regions\n"\
     "* Hit Return in the casapy console after you \n"\
     "* satisfy your mask selection.\n"
print desc
sdstat()
print "*** Done sdstat ****\n"
# specify line region, hit return in the console
# then you will see output looks like this.

#final mask list (GHz) = [[45.492970859204817, 45.495370089189244]]
#--------------------------------------------------
#  max
#--------------------------------------------------
#Scan[0] (OrionS_ps) Time[2006/01/19/01:52:05]:
# IF[0] = 3.722
#--------------------------------------------------
#
#--------------------------------------------------
#  min
#--------------------------------------------------
#Scan[0] (OrionS_ps) Time[2006/01/19/01:52:05]:
# IF[0] = 2.493
#--------------------------------------------------
#
#--------------------------------------------------
#  sum
#--------------------------------------------------
#Scan[0] (OrionS_ps) Time[2006/01/19/01:52:05]:
# IF[0] = 1153.775
#--------------------------------------------------
#
#--------------------------------------------------
#  mean
#--------------------------------------------------
#Scan[0] (OrionS_ps) Time[2006/01/19/01:52:05]:
# IF[0] = 2.928
#--------------------------------------------------
#
#--------------------------------------------------
#  median
#--------------------------------------------------
#Scan[0] (OrionS_ps) Time[2006/01/19/01:52:05]:
# IF[0] = 2.830
#--------------------------------------------------
#
#--------------------------------------------------
#  rms
#--------------------------------------------------
#Scan[0] (OrionS_ps) Time[2006/01/19/01:52:05]:
# IF[0] = 2.940
#--------------------------------------------------
#
#--------------------------------------------------
#  stddev
#--------------------------------------------------
#Scan[0] (OrionS_ps) Time[2006/01/19/01:52:05]:
# IF[0] = 0.264
#--------------------------------------------------
#
#The current mask window unit is GHz
#The current mask window unit is GHz
#
#--------------------------------------------------
#  eqw [ GHz ]
#--------------------------------------------------
#Scan[0] (OrionS_ps) Time[2006/01/19/01:52:05]:
# IF[0] = 0.002
#--------------------------------------------------
#
#--------------------------------------------------
#  Integrated intensity [ K * GHz ]
#--------------------------------------------------
#Scan[0] (OrionS_ps) Time[2006/01/19/01:52:05]:
# IF[0] = 0.007
#--------------------------------------------------

# You will also see these information is saved
# in orion_hc3n_stat.txt



if tpdemo:
   # NEW TASK ####
   ###############
   # sdtpimaging #
   ###############
   print "###############\n 7. New task: sdtpimaging \n###############"
   desc="* Sdtpimaging is a new task to do data analysis of the total power\n" \
        "* raster scan data.\n"
   print desc
   default(sdtpimaging)
   # plot the data
   plotlevel=2
   antenna='0'
   sdfile='uid___X1e1_X3197_X1.ms'
   inp()
   desc="\n" \
        "* For this example, we do it in three steps to illustrate features,\n" \
        "* but it can be run the entire process in a single run of sdtpimaiging.\n" \
        "* First, do just plotting of the uncalibrated data leaving \n" \
        "* calmode and createimage to defaults (calmode='none', createimage=False).\n" \
        "* If there is CORRECTED_DATA column in the MS data, the calibrated data\n"\
        "* will be plotted instead.\n"
   print desc
   pause=raw_input('\n* Hit Return to continue ')
   print "\n* Run sdtpimaging...\n"
   sdtpimaging()
   print "*** sdtpimaging done***\n"

   calmode='baseline'
   masklist=[50] # use 50 data points from each end of scan for fitting
   plotlevel=1
   #plotlevel=2 to see progress of each fitting
   inp()
   desc="\n"\
        "* Now try to calibrate the data, currently only a simple baseline subtraction\n" \
        "* is available.\n"
   print desc
   pause=raw_input('\n* Hit Return to continue ')
   print "\n* Run sdtpimaging...\n"
   sdtpimaging()
   print "*** sdtpimaging done***\n"
   # Do imaging only
   calmode='none'
   createimage=True
   imagename='moon.im'
   imsize=[200,200]
   cell=[0.2] # in arcmin
   phasecenter='AZEL 187d54m22s 41d03m0s'
   ephemsrcname='moon' # can be omitted 
   inp()
   desc="\n" \
        "* Finally, to create an image, set createimage=True.\n" \
        "* The data contain the observation of the Moon so set\n"\
        "* ephemsrcname (the task automatically look up SOURCE table\n"\
        "* to check if it is ephemeris source or not, even you left\n"\
        "* this blank.\n"
   print desc
   pause=raw_input('\n* Hit Return to continue ')
   print "\n* Run sdtpimaging...\n"
   sdtpimaging()
   print "*** sdtpimaging done***\n"
   
   print "* Let's check the create image"
   pause=raw_input('\n* Hit Return to continue ')
   viewer(imagename)

   
  
print "\n###############\n Tool changes \n###############"


desc="\n" \
     "* Now ASAP scantable supports the storage of multiple rest\n" \
     "* frequencies. As the result, behaviors of setting/getting rest \n" \
     "* frequencies slightly changed\n" \
     "*\n" \
     "* For example, \n"\
     "s=sd.scantable('orion_pscal') \n" \
     "restfreqs=s.get_restfreqs() \n" 
print desc

s=sd.scantable('orion_pscal')
restfreqs=s.get_restfreqs()
desc="* Returned rest frequencies are in a dictionary, and only list\n" \
     "* one(s) current used.\n";
print desc 
print "restfreqs=", restfreqs

pause=raw_input('\n* Hit Return to continue ')
desc="\n* Set new rest frequencies for IF=0.\n" \
     "* let\n"\
     "newrestfreqs=[45490258000.0,45590258000.0]\n"\
     "*(Note: arbitary values used as an example)\n" \
     "sel=sd.selector()\n"\
     "sel.set_ifs(0)\n" \
     "s.set_selection(sel)\n"\
     "s.set_restfreqs(newrestfreqs)\n"
print desc
newrestfreqs=[45490258000.0,45590258000.0]
sel=sd.selector()
sel.set_ifs(0)
s.set_selection(sel)
s.set_restfreqs(newrestfreqs)

desc="* Check the new values\n" \
     "s.get_restfreqs() \n"
print desc
newrestfs=s.get_restfreqs()
print newrestfs
pause=raw_input('\n* Hit Return to continue ')

desc="\n* List all the rest frequencies currently set for all IFs\n" \
     "sel.reset()\n"\
     "s.set_selection(sel)\n"\
     "s.get_restfreqs()\n"
print desc
sel.reset()
s.set_selection(sel)
newrestfs=s.get_restfreqs()
print newrestfs

print "*** done get/set_restfreqs demo ***"

print "**** End of the script ****"
