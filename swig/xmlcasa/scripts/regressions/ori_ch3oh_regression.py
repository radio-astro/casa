###############################
#
# ORION-S CH3OH Reduction Script
# Position-Switched data
#
###############################
#
import time
import os

os.system('rm -rf OrionS_rawACSmod orions_ch3oh_reducedSCAN0_CYCLE0_BEAM0_IF0.txt orions_ch3oh_reduced.eps orions_ch3oh_fit.txt')

casapath=os.environ['CASAPATH']
datapath=casapath.split()[0]+'/data/regression/ATST5/OrionS/OrionS_rawACSmod'
copystring='cp -r '+datapath+' .'
os.system(copystring)

startTime=time.time()
startProc=time.clock()

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

import asap as sd			#import ASAP package into CASA			#GBTIDL
					#Orion-S (CH3OH line reduction only)
					#Notes:
					#scan numbers (zero-based) as compared to GBTIDL

					#changes made to get to OrionS_rawACSmod
					#modifications to label sig/ref positions

os.environ['CASAPATH']=casapath


s=sd.scantable('OrionS_rawACSmod',False)#load the data without averaging		# filein,'Orion-S.raw.fits'

#s.summary()				#summary info					# summary
											# fileout,'Orion-S-reduced.fits'
s.set_fluxunit('K')         		# make 'K' default unit

scal=sd.calps(s,[20,21,22,23])		# Calibrate CH3OH scans				# for i=21,24,2 do begin getps,i,ifnum=2,plnum=0,units='Ta*',
del s                                   # remove s from memory
# recalculate az/el (NOT needed for GBT data)
antennaname = scal.get_antennaname()
if ( antennaname != 'GBT'): scal.recalc_azel()      # recalculate az/el to 		# tau=0.09 & accum & getps, i, ifnum=2,plnum=1,units='Ta*',
scal.opacity(0.09)			# do opacity correction				# tau=0.09 & accum & end & ave
sel=sd.selector()			# Prepare a selection
sel.set_ifs(2)				# select CH3OH IF
scal.set_selection(sel)			# get this IF
stave=sd.average_time(scal,weight='tintsys')	# average in time
spave=stave.average_pol(weight='tsys')	# average polarizations;Tsys-weighted (1/Tsys**2) average
sd.plotter.plot(spave)			# plot

# what is going on with autoscaling?

					# do some smoothing
spave.smooth('boxcar',10)		# boxcar 10					# boxcar,10
#spave.auto_poly_baseline(order=5)	# baseline fit order=5				# nfit,5
					# you can also set the baseline region              or
basemask=spave.create_mask([350,2700],[3500,7500])                                      # nregion,[500,3500,5000,7500]
spave.poly_baseline(basemask,order=5)							# nfit,5
sd.plotter.plot(spave)			# plot						# baseline

spave.set_unit('GHz')									# freq
sd.plotter.plot(spave)
sd.plotter.set_histogram(hist=True)     # draw spectrum using histogram                 # histogram
sd.plotter.axhline(color='r',linewidth=2)   # zline                                     # zline
sd.plotter.save('orions_ch3oh_reduced.eps') # save postscript spectrum			# write_ps,'orions_ch3oh.ps'

spave.set_unit('channel')								# chan
rmsmask=spave.create_mask([4000,6000])  # get rms of line free regions                  # stats,4000,6000
rms=spave.stats(stat='rms',mask=rmsmask)                                                                
					#---------------------------------------------- # Chans   bchan   echan      Xmin      Xmax      Ymin      Ymax
											#  2001    4000    6000    4000.0    6000.0  -0.26343   0.23227
					#  rms
					#---------------------------------------------- #                 Mean      Median      RMS  Variance      Area
					#Scan[0] (OrionS_ps) Time[2006/01/19/02:07:54]: #          -0.00044638  -0.0023551 0.092609 0.0085763  -0.89321
					# IF[2] = 0.092
					#----------------------------------------------
					# LINE

linemask=spave.create_mask([2900,3150])
max=spave.stats('max',linemask)		#  IF[2] = 1.275
sum=spave.stats('sum',linemask)	        #  IF[2] = 90.350
median=spave.stats('median',linemask)   #  IF[2] = 0.231
mean=spave.stats('mean',linemask)	#  IF[2] = 0.360
											# Chans bchan    echan       Xmin      Xmax        Ymin     Ymax
											#   251  2900     3150     2900.0    3150.0    -0.20952   1.2829
											#                 Mean     Median       RMS    Variance     Area
											#              0.36219    0.23209   0.39342     0.15478   90.909
											#
					# Fitting
spave.set_unit('channel')		# set units to channel				# chan
sd.plotter.plot(spave)			# plot spectrum				
f=sd.fitter()
msk=spave.create_mask([2900,3150])	# create region around line			# gregion,[2900,3150]
f.set_function(gauss=1)			# set a single gaussian component		# ngauss,1
f.set_scan(spave,msk)			# set the data and region for the fitter	
#f.set_gauss_parameters(1.2,3100.,100.)  # set initial guess				# gparamvalues,0,[1.2,3100.,100.]
f.fit()					# auto fit					# gauss
f.plot(residual=True)			# plot residual
f.get_parameters()			# retrieve fit parameters			#
#   0: peak = 1.054 K , centre = 3037.182 channel, FWHM = 71.654 channel
#      area = 80.382 K channel
#
# Regression values of CASA 2.3(#6654)+ASAP 2.2.0(#1448)
# on 64bit REL5.2 (2008/12/01)
#   0: peak = 1.110 K , centre = 3037.176 channel, FWHM = 71.676 channel
#      area = 84.727 K channel

											# ***** Initial Guesses
											#           G#      Height Center (chan)    FWHM (chan) 
											# Init:      1       1.200 3100.0000         100.0
											# 
											# ***** Fitted Gaussians
											#        Height            Center (chan)      FWHM (chan)
											#  1       1.117 (0.02187) 3037.1758 (0.6879) 71.68 (1.620)

f.store_fit('orions_ch3oh_fit.txt') 	# store fit					# *copy and paste from log*

# Save the spectrum
spave.save('orions_ch3oh_reduced','ASCII',True)	# save the spectrum			# write_ascii,'orions_ch3oh.spc'
#spave.save('orions_ch3oh_reduced.ms','MS2',True) # save as an MS			# NA
#
endProc = time.clock()
endTime = time.time()

# --- end of orion-s ch3oh script

#ori_max=1.275
#ori_rms=0.0921
#ori_sum=90.350
# Regression values of CASA 2.3(#6654)+ASAP 2.2.0(#1448)
# on 64bit REL5.2 (2008/12/01)
ori_max=1.275
ori_rms=0.09209
ori_sum=90.35
diff_max = abs((ori_max-max[0])/ori_max)
diff_rms = abs((ori_rms-rms[0])/ori_rms)
diff_sum = abs((ori_sum-sum[0])/ori_sum)

import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='ori.ch3oh.'+datestring+'.log'
logfile=open(outfile,'w')

print >>logfile,''
print >>logfile,'********** Regression ***********'
print >>logfile,'*                               *'
if (diff_max < 0.05): print >>logfile,'* Passed spectrum max test '
print >>logfile,'*  Spectrum max '+str(max)
if (diff_rms < 0.05): print >>logfile,'* Passed spectrum rms test '
print >>logfile,'*  Spectrum rms '+str(rms)
if (diff_sum < 0.05): print >>logfile,'* Passed spectrum (line) sum test'
print >>logfile,'*  Line integral '+str(sum)
if ((diff_max<0.05) & (diff_rms<0.05) & (diff_sum<0.05)):
	regstate=True
        print >>logfile,'---'
        print >>logfile,'Passed Regression test for OrionS-CH3OH'
        print >>logfile,'---'
else:
	regstate=False
        print >>logfile,'----FAILED Regression test for OrionS-CH3OH'
print >>logfile,'*********************************'

print >>logfile,''
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
#print 'Processing rate MB/s  was: ', 35.1/(endTime - startTime)

logfile.close()
