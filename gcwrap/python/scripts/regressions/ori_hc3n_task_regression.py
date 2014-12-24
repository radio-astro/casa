###############################
#
# ORION-S HC3N Reduction Script
# using SD tasks 
# Position-Switched data
# Formatted for wide screen (150
# characters wide)
#
# used tasks
# sdlist
# sdcal
# sdsmooth
# sdbaseline
# sd
###############################
#
import time
import os

os.system('rm -rf OrionS_rawACSmod orions_hc3n_reducedSCAN0_CYCLE0_BEAM0_IF0.txt orions_hc3n_reduced.eps orions_hc3n_fit.txt')

#enable/disable plotting
doplot = False

casapath = os.environ['CASAPATH']
datapath = casapath.split()[0]+'/data/regression/ATST5/OrionS/OrionS_rawACSmod'
#copystring = 'cp -r '+datapath+' .'
#os.system(copystring)
shutil.copytree(datapath,'OrionS_rawACSmod')

startTime = time.time()
startProc = time.clock()

#           MeasurementSet Name:  /home/rohir3/jmcmulli/SD/OrionS_rawACSmod      MS Version 2
#
# Project: AGBT06A_018_01
# Observation: GBT(1 antennas)
#
#Data records: 256       Total integration time = 1523.13 seconds
#   Observed from   01:45:58   to   02:11:21
#
#Fields: 4
#  ID   Name          Right Ascension  Declination   Epoch
#  0    OrionS        05:15:13.45      -05.24.08.20  J2000
#  1    OrionS        05:35:13.45      -05.24.08.20  J2000
#  2    OrionS        05:15:13.45      -05.24.08.20  J2000
#  3    OrionS        05:35:13.45      -05.24.08.20  J2000
#
#Spectral Windows:  (8 unique spectral windows and 1 unique polarization setups)
#  SpwID  #Chans Frame Ch1(MHz)    Resoln(kHz) TotBW(kHz)  Ref(MHz)    Corrs
#  0        8192 LSRK  45464.3506  6.10423298  50005.8766  45489.3536  RR  LL HC3N
#  1        8192 LSRK  45275.7825  6.10423298  50005.8766  45300.7854  RR  LL HN15CO
#  2        8192 LSRK  44049.9264  6.10423298  50005.8766  44074.9293  RR  LL CH3OH
#  3        8192 LSRK  44141.2121  6.10423298  50005.8766  44166.2151  RR  LL HCCC15N
#  12       8192 LSRK  43937.1232  6.10423356  50005.8813  43962.1261  RR  LL HNCO
#  13       8192 LSRK  42620.4173  6.10423356  50005.8813  42645.4203  RR  LL H15NCO
#  14       8192 LSRK  41569.9768  6.10423356  50005.8813  41594.9797  RR  LL HNC18O
#  15       8192 LSRK  43397.8198  6.10423356  50005.8813  43422.8227  RR  LL SiO

# Scans: 21-24  Setup 1 HC3N et al
# Scans: 25-28  Setup 2 SiO et al

asap_init()                             #load ASAP module
					#Orion-S (SiO line reduction only)
					#Notes:
					#scan numbers (zero-based) as compared to GBTIDL

					#changes made to get to OrionS_rawACSmod
					#modifications to label sig/ref positions
#os.environ['CASAPATH'] = casapath


# summary
#default(sdlist)
#infile = 'OrionS_rawACSmod'
#sdlist()

if doplot:
   localplotlevel = 1
else:
   localplotlevel = 0

# calibartion and averaging
# calibrate position-switched HC3N scans (IF=0) 
default(sdcal)
infile = 'OrionS_rawACSmod'
fluxunit = 'K' 
calmode = 'ps'
#scanlist = [21,22,23,24]
scan = '21~24'
#iflist = [0]
spw = '0'
tau = 0.09         # do opacity correction 
overwrite = True
plotlevel = localplotlevel  
sdcal() 
# output
localoutfile = infile+'_cal'


#  averaging and smoothing
default(sdaverage)
infile = localoutfile
#averaging
# do time and polarization average
timeaverage = True # average in time
tweight = 'tintsys' # weighted by integ time and Tsys for time averaging
scanaverage = False
polaverage = True  # average polarization
pweight = 'tsys'   # weighted by Tsys for pol. averaging 
#smoothing
# do boxcar smoothing with channel width=5
kernel = 'boxcar'
kwidth = 5

overwrite = True
plotlevel = localplotlevel
sdaverage()
localoutfile = infile+'_ave'

#fit and remove baselines
# do baseline fit with polynomial order of 2
# automatically detect lines to exclude from fitting
default(sdbaseline)
infile = localoutfile
maskmode = 'auto'
edge = [50]
thresh = 5
avg_limit = 4
blfunc = 'poly'
order = 2
overwrite = True
plotlevel = localplotlevel
sdbaseline()
localoutfile = infile+'_bs'
#sd.plotter.plot(spave)			# plot						# baseline

#plotting the reslut
#plot the spectrum and save to a postscript file 
if doplot:
   default(sdplot)
   infile = localoutfile
   specunit = 'GHz'
   outfile = 'orions_hc3n_reduced.eps'
   #sd.plotter.set_histogram(hist=True)     # draw spectrum using histogram                 # histogram
   #sd.plotter.axhline(color='r',linewidth=2) # zline                                       # zline
   sdplot()
else:
   print "Plotting the result is skipped."

# statistics
default(sdstat)
# select line free regions to get rms
infile = localoutfile
#masklist = [5000,7000]
spw = '*:5000~7000'
xstat = sdstat()
curr_rms = xstat['rms']
#rms= 0.04910755529999733
#rms= 0.049121532589197159 [CASA 2.3(#6654)+ASAP 2.2.0(#1448)]
#
# select the line region
#masklist = [3900,4200]
spw = '*:3900~4200'
xstat = sdstat()
xstat
#{'eqw': 70.905397021125211,
# 'max': 0.91880249977111816,
# 'mean': 0.21643872559070587,
# 'median': 0.093559741973876953,
# 'min': -0.14174103736877441,
# 'rms': 0.34947043657302856,
# 'stddev': 0.27483600378036499,
# 'sum': 65.148056030273438}
# 
# Regression values of CASA 2.3(#6654)+ASAP 2.2.0(#1448)
# on 64bit REL5.2 (2008/12/01)
#{'eqw': 70.858384499609528,
# 'max': 0.91861748695373535,
# 'mean': 0.2162516713142395,
# 'median': 0.093370199203491211,
# 'min': -0.14193272590637207,
# 'rms': 0.3493553102016449,
# 'stddev': 0.27483683824539185,
# 'sum': 65.091751098632812}
curr_max = xstat['max']
curr_sum = xstat['sum']
# fitting
default(sdfit)
infile = localoutfile
#sd.plotter.plot(spave)			# plot spectrum
fitmode = 'list'
#maskline = [3928,4255]	# create region around line			# gregion,[4000,4200]
spw = '*:3928~4255'	# create region around line			# gregion,[4000,4200]
nfit = 1
plotlevel = localplotlevel
outfile = 'orions_hc3n_fit.txt'
xstat = sdfit()
xstat  # print fit statistics 
#{'cent': [[4091.243408203125, 0.55986660718917847]],
# 'fwhm': [[70.907455444335938, 1.318385124206543]],
# 'nfit': 1,
# 'peak': [[0.80756759643554688, 0.013003586791455746]]}
# 
# Regression values of CASA 2.3(#6654)+ASAP 2.2.0(#1448)
# on 64bit REL5.2 (2008/12/01)
#{'cent': [[[4091.24755859375, 0.55954688787460327]]],
# 'fwhm': [[[70.871696472167969, 1.3176323175430298]]],
# 'nfit': [1],
# 'peak': [[[0.80750846862792969, 0.013001702725887299]]]}

# Save the spectrum
# in different formats
default(sdsave)
infile = localoutfile
outfile = 'orions_hc3n_reduced'
outform = 'ASCII'
overwrite = True
sdsave()
outfile = 'orions_hc3n_reduced.ms'
outform = 'MS2'
sdsave()

endProc = time.clock()
endTime = time.time()
#
# --- end of orion-s hc3n script
# Regression
#hc3n_max=0.918
#hc3n_rms=0.049
#hc3n_sum=64.994
# Regression values of CASA 2.3(#6654)+ASAP 2.2.0(#1448)
# on 64bit REL5.2 (2008/12/01)
hc3n_max = 0.9186
hc3n_rms = 0.04912
hc3n_sum = 65.09
diff_max = abs( (hc3n_max - curr_max) / hc3n_max )
diff_rms = abs( (hc3n_rms - curr_rms) / hc3n_rms )
diff_sum = abs( (hc3n_sum - curr_sum) / hc3n_sum )

import datetime
datestring = datetime.datetime.isoformat(datetime.datetime.today())
outfile = 'ori.hc3n.task.'+datestring+'.log'
logfile = open(outfile,'w')
print >>logfile,''
print >>logfile,'********** Regression ***********'
print >>logfile,'*                               *'
if (diff_max < 0.05): print >>logfile,'* Passed spectrum max test '
print >>logfile,'*  Spectrum max '+str(curr_max)
if (diff_rms < 0.05): print >>logfile,'* Passed spectrum rms test '
print >>logfile,'*  Spectrum rms '+str(curr_rms)
if (diff_sum < 0.05): print >>logfile,'* Passed spectrum (line) sum test'
print >>logfile,'*  Line integral '+str(curr_sum)
if ((diff_max<0.05) & (diff_rms<0.05) & (diff_sum<0.05)): 
	regstate = True
        print >>logfile,'---'
        print >>logfile,'Passed Regression test for OrionS-HC3N'
        print >>logfile,'---'
        print ''
        print 'Regression PASSED'
        print ''
else: 
	regstate = False
        print >>logfile,'----FAILED Regression test for OrionS-HC3N'
        print ''
        print 'Regression FAILED'
        print ''

print >>logfile,'*********************************'

print >>logfile,''
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
#print 'Processing rate MB/s  was: ', 35.1/(endTime - startTime)

logfile.close()
