###############################
#
# ORION-S HC3N Reduction Script
# Position-Switched data
# Formatted for wide screen (150
# characters wide)
#
###############################
#
import time
import os

os.system('rm -rf OrionS_rawACSmod orions_hc3n_reducedSCAN0_CYCLE0_BEAM0_IF0.txt orions_hc3n_reduced.eps orions_hc3n_fit.txt')

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

scal = sd.calps(s, [20,21,22,23])		# Calibrate HC3N scans				# for i=21,24,2 do begin getps,i,ifnum=0,plnum=0,units='Ta*',
#
del s					# remove s from memory
antennaname = scal.get_antennaname()
# recalculate az/el (NOT needed for GBT data)
if ( antennaname != 'GBT'): scal.recalc_azel()      # recalculate az/el to 		# tau=0.09 & accum & getps, i, ifnum=0,plnum=1,units='Ta*',
scal.opacity(0.09)			# do opacity correction				# tau=0.09 & accum & end & ave
sel = sd.selector()			# Prepare a selection
sel.set_ifs(0)				# select HC3N IF
scal.set_selection(sel)			# get this IF
stave = sd.average_time(scal, weight='tintsys')	# average in time
spave = stave.average_pol(weight='tsys')	# average polarizations;Tsys-weighted (1/Tsys**2) average
sd.plotter.plot(spave)			# plot

# what is going on with autoscaling?

					# do some smoothing
spave.smooth('boxcar',5)		# boxcar 5					# boxcar,5
spave.auto_poly_baseline(edge=[50], order=2, threshold=5, chan_avg_limit=4)   # baseline fit order=2	  # chan & nregion,[500,3000,5000,7500] & nfit,2
sd.plotter.plot(spave)			# plot						# baseline

spave.set_unit('GHz')									# freq
sd.plotter.plot(spave)
sd.plotter.set_histogram(hist=True)     # draw spectrum using histogram                 # histogram
sd.plotter.axhline(color='r', linewidth=2) # zline                                       # zline
sd.plotter.save('orions_hc3n_reduced.eps')# save postscript spectrum		        # write_ps,'orions_hc3n.ps'

spave.set_unit('channel')								# chan
rmsmask = spave.create_mask([5000,7000])  # get rms of line free regions                  # stats,5000,7000
curr_rms = spave.stats(stat='rms', mask=rmsmask)                                                                
                                        #---------------------------------------------- # Chans   bchan   echan      Xmin      Xmax      Ymin      Ymax
											#  2001    5000    7000    5000.0    7000.0  -0.33294   0.27287
					#  rms
					#---------------------------------------------- #                 Mean      Median      RMS  Variance      Area
					#Scan[0] (OrionS_ps) Time[2006/01/19/01:52:05]: #            -0.018826   -0.016566 0.098964 0.0097939   -37.672
					# IF[0] = 0.049
					#----------------------------------------------
					# LINE
linemask = spave.create_mask([3900,4200])
curr_max = spave.stats('max', linemask)		#  IF[0] = 0.919
curr_sum = spave.stats('sum', linemask)		#  IF[0] = 65.092
curr_median = spave.stats('median', linemask)	#  IF[0] = 0.093
curr_mean = spave.stats('mean', linemask)	#  IF[0] = 0.216
											# Chans  bchan    echan      Xmin      Xmax        Ymin     Ymax
											#   301   3900     4200    3900.0    4200.0    -0.21815   1.0648
                       									#                  Mean    Median       RMS    Variance     Area
											#               0.22148   0.15098   0.30922    0.095619   66.664
					# Fitting
spave.set_unit('channel')		# set units to channel				# chan
sd.plotter.plot(spave)			# plot spectrum				
f = sd.fitter()
msk = spave.create_mask([3928,4255])	# create region around line			# gregion,[4000,4200]
f.set_function(gauss=1)			# set a single gaussian component		# ngauss,1
f.set_scan(spave, msk)			# set the data and region for the fitter	# gparamvalues,0,[1.,4100.,100.]
f.fit()					# fit						# gauss
f.plot(residual=True)			# plot residual
f.get_parameters()			# retrieve fit parameters
#   0: peak = 0.786 K , centre = 4091.236 channel, FWHM = 70.586 channel
#      area = 59.473 K channel
#
# Regression values of CASA 2.3(#6654)+ASAP 2.2.0(#1448)
# on 64bit REL5.2 (2008/12/01)
#   0: peak = 0.808 K , centre = 4091.247 channel, FWHM = 70.873 channel
#      area = 60.919 K channel
											#***** Initial Guesses
											#          G#      Height  Center (chan)  FWHM (chan) 
											#Init:      1       1.000      4100.0000       100.0
											#***** Fitted Gaussians
											#        Height            Center (chan)       FWHM (chan)
											# 1      0.8281 ( 0.01670) 4091.8367 ( 0.7487) 75.73 (     1.763)

f.store_fit('orions_hc3n_fit.txt') 	# store fit					# *copy and paste from log*

# Save the spectrum
spave.save('orions_hc3n_reduced', 'ASCII', True)	# save the spectrum			# write_ascii,'orions_hc3n.spc'
#spave.save('orions_hc3n_reduced.ms','MS2',True) # save as an MS				# NA

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
diff_max = abs( (hc3n_max - curr_max[0]) / hc3n_max )
diff_rms = abs( (hc3n_rms - curr_rms[0]) / hc3n_rms )
diff_sum = abs( (hc3n_sum - curr_sum[0]) / hc3n_sum )

import datetime
datestring = datetime.datetime.isoformat(datetime.datetime.today())
outfile = 'ori.hc3n.'+datestring+'.log'
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
