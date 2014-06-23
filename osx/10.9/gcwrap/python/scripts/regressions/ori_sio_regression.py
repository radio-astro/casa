###############################
#
# ORION-S SiO Reduction Script
# Position-Switched data
#
###############################
#
import time
import os

os.system('rm -rf OrionS_rawACSmod orions_sio_reducedSCAN0_CYCLE0_BEAM0_IF0.txt orions_sio_reduced.eps orions_sio_fit.txt')

casapath = os.environ['CASAPATH']
datapath = casapath.split()[0]+'/data/regression/ATST5/OrionS/OrionS_rawACSmod'
copystring = 'cp -r '+datapath+' .'
os.system(copystring)

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


import asap as sd			#import ASAP package into CASA			#GBTIDL
					#Orion-S (SiO line reduction only)
					#Notes:
					#scan numbers (zero-based) as compared to GBTIDL

					#changes made to get to OrionS_rawACSmod
					#modifications to label sig/ref positions

os.environ['CASAPATH'] = casapath


s = sd.scantable('OrionS_rawACSmod', False)#load the data without averaging		# filein,'Orion-S.raw.fits'

#s.summary()				#summary info					# summary
											# fileout,'Orion-S-reduced.fits'
s.set_fluxunit('K')         		# make 'K' default unit

#scal = sd.calps(s, [24,25,26,27])		# Calibrate SiO scans				# for i=25,28,2 do begin getps,i,ifnum=3,plnum=0,units='Ta*',
scal = sd.calps(s, [25,26,27,28])		# Calibrate SiO scans				# for i=25,28,2 do begin getps,i,ifnum=3,plnum=0,units='Ta*',
del s                                   # remove s from memory
# recalculate az/el (NOT needed for GBT data)
antennaname = scal.get_antennaname()
if ( antennaname != 'GBT'): scal.recalc_azel()      # recalculate az/el to 		# tau=0.09 & accum & getps, i, ifnum=3,plnum=1,units='Ta*',
scal.opacity(0.09)			# do opacity correction				# tau=0.09 & accum & end & ave
sel = sd.selector()			# Prepare a selection
sel.set_ifs(15)				# select SiO IF
scal.set_selection(sel)			# get this IF
stave = sd.average_time(scal, weight='tintsys')	# average in time
spave = stave.average_pol(weight='tsys')	# average polarizations;Tsys-weighted (1/Tsys**2) average
sd.plotter.plot(spave)			# plot

# what is going on with autoscaling?

					# do some smoothing
spave.smooth('boxcar', 10)		# boxcar 10					# boxcar,10
#spave.auto_poly_baseline(order=5)	# baseline fit order=5				# nfit,5
					# you can also set the baseline region              or
basemask = spave.create_mask([500,3500],[5000,7500])                                      # nregion,[500,3500,5000,7500]
spave.poly_baseline(basemask, order=5)							# nfit,5
sd.plotter.plot(spave)			# plot						# baseline

spave.set_unit('GHz')									# freq
sd.plotter.plot(spave)
sd.plotter.set_histogram(hist=True)     # draw spectrum using histogram                 # histogram
sd.plotter.axhline(color='r', linewidth=2)  # zline                                      # zline
sd.plotter.save('orions_sio_reduced.eps')  # save postscript spectrum			# write_ps,'orions_sio.ps'

spave.set_unit('channel')								# chan
rmsmask = spave.create_mask([1000,3000])  # get rms of line free regions                  # stats,1000,3000
curr_rms = spave.stats(stat='rms', mask=rmsmask)                                                                
					#---------------------------------------------- # Chans   bchan   echan      Xmin      Xmax      Ymin      Ymax
											#  2001    1000    3000    1000.0    3000.0  -0.09437   0.15836
					#  rms
					#---------------------------------------------- #                 Mean      Median      RMS  Variance      Area
					#Scan[0] (OrionS_ps) Time[2006/01/19/02:07:54]: #            0.0030781   0.0019092 0.037550 0.0014100    6.1593
					# IF[15] = 0.037
					#----------------------------------------------
					# LINE

linemask = spave.create_mask([3900,4300])
curr_max = spave.stats('max', linemask)	#  IF[15] = 0.366
curr_sum = spave.stats('sum', linemask)	#  IF[15] = 49.949
curr_median = spave.stats('median', linemask) #  IF[15] = 0.094
curr_mean = spave.stats('mean', linemask)	#  IF[15] = 0.125
											# Chans  bchan    echan      Xmin      Xmax        Ymin     Ymax
											#   301   3900     4300    3900.0    4300.0    -0.069310 0.37076
                       									#                  Mean    Median       RMS    Variance     Area
											#               0.12613   0.095303   0.095872    0.0091914   50.579
					# Fitting
spave.set_unit('channel')		# set units to channel				# chan
sd.plotter.plot(spave)			# plot spectrum				
f = sd.fitter()
msk = spave.create_mask([3900,4300])	# create region around line			# gregion,[4000,4200]
f.set_function(gauss=1)			# set a single gaussian component		# ngauss,1
f.set_scan(spave, msk)			# set the data and region for the fitter	
f.set_gauss_parameters(0.5, 4100., 100.)  # set initial guess				# gparamvalues,0,[1.,4100.,100.]
f.fit()					# auto fit					# gauss
f.plot(residual=True)			# plot residual
f.get_parameters()			# retrieve fit parameters			#
#   0: peak = 0.273 K , centre = 4096.952 channel, FWHM = 155.109 channel
#      area = 45.074 K channel
#
# Regression values of CASA 2.3(#6654)+ASAP 2.2.0(#1448)
# on 64bit REL5.2 (2008/12/01)
#   0: peak = 0.275 K , centre = 4097.001 channel, FWHM = 156.038 channel
#      area = 45.751 K channel
											#***** Initial Guesses
											#          G#      Height  Center (chan)    FWHM (chan) 
											#Init:      1      1.0000         4100.0000         100.0
											#
											#***** Fitted Gaussians
											#        Height            Center (chan)       FWHM (chan)
											# 1      0.3167 ( 0.005652)4096.4663 (1.018)  116.4 ( 2.586)

f.store_fit('orions_sio_fit.txt') 	# store fit					# *copy and paste from log*

# Save the spectrum
spave.save('orions_sio_reduced', 'ASCII', True)	# save the spectrum			# write_ascii,'orions_sio.spc'
#spave.save('orions_sio_reduced.ms','MS2',True) # save as an MS				# NA
#
endProc = time.clock()
endTime = time.time()

# --- end of orion-s sio script
#ori_max=0.366
#ori_rms=0.037
#ori_sum=49.9
# Regression values of CASA 2.3(#6654)+ASAP 2.2.0(#1448)
# on 64bit REL5.2 (2008/12/01)
ori_max = 0.3662
ori_rms = 0.03720
ori_sum = 49.95
diff_max = abs( (ori_max - curr_max[0]) / ori_max )
diff_rms = abs( (ori_rms - curr_rms[0]) / ori_rms )
diff_sum = abs( (ori_sum - curr_sum[0]) / ori_sum )

import datetime
datestring = datetime.datetime.isoformat(datetime.datetime.today())
outfile = 'ori.sio.'+datestring+'.log'
logfile = open(outfile, 'w')

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
        print >>logfile,'Passed Regression test for OrionS-SiO'
        print >>logfile,'---'
        print ''
        print 'Regression PASSED'
        print ''
else:
	regstate = False
        print >>logfile,'----FAILED Regression test for OrionS-SiO'
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
