###############################
#
# IRC+10216 HC3N Reduction Script
# Nod Data
#
###############################
import time
import os

os.system('rm -rf IRC+10216_rawACSmod irc_hc3n_reducedSCAN0_CYCLE0_BEAM0_IF0.txt irc_hc3n_reduced.eps irc_hc3n_fit.txt')

casapath=os.environ['CASAPATH'].split()[0]
datapath=casapath+'/data/regression/ATST5/IRC+10216/IRC+10216_rawACSmod'
copystring='cp -r '+datapath+' .'
os.system(copystring)

startTime=time.time()
startProc=time.clock()

#   Project: AGBT06A_018_01
#Observation: GBT(1 antennas)
#
#Tue Jan 30 20:24:29 2007    NORMAL ms::summary:
#Data records: 4992       Total integration time = 4244.15 seconds
#   Observed from   10:31:01   to   11:41:45
#
#Tue Jan 30 20:24:29 2007    NORMAL ms::summary:
#Fields: 3
#  ID   Name          Right Ascension  Declination   Epoch
#  0    IRC+10216     09:47:57.38      +13.16.43.70  J2000
#  1    IRC+10216     09:47:57.38      +13.16.43.70  J2000
#  2    IRC+10216     09:47:57.38      +13.16.43.70  J2000
#
#Tue Jan 30 20:24:29 2007    NORMAL ms::summary:
#Spectral Windows:  (13 unique spectral windows and 1 unique polarization setups)
#  SpwID  #Chans Frame Ch1(MHz)    Resoln(kHz) TotBW(kHz)  Ref(MHz)    Corrs
#  0        4096 LSRK  48263.5858  12.2067945  49999.0303  48288.5853  RR  LL H2CO
#  1        4096 LSRK  45899.1486  12.2067945  49999.0303  45924.1481  RR  LL H213CO
#  2        4096 LSRK  43857.1942  12.2067945  49999.0303  43882.1937  RR  LL 13CS
#  3        4096 LSRK  48970.0031  12.2067945  49999.0303  48995.0026  RR  LL CS
#  7        4096 LSRK  46226.6452  12.2067949  49999.0319  46251.6447  RR  LL 
#  17       4096 LSRK  45469.0638  12.2067955  49999.0343  45494.0633  RR  LL HC3N
#  18       4096 LSRK  45280.5214  12.2067955  49999.0343  45305.5209  RR  LL HCC13CN
#  19       4096 LSRK  44054.8331  12.2067955  49999.0343  44079.8326  RR  LL CH3OH
#  20       4096 LSRK  44146.1063  12.2067955  49999.0343  44171.1058  RR  LL HCCC15N
#  27       4096 LSRK  43941.6784  12.206796   49999.0365  43966.6779  RR  LL HN13CO
#  28       4096 LSRK  42625.1528  12.206796   49999.0365  42650.1523  RR  LL H15NCO
#  29       4096 LSRK  41574.856   12.206796   49999.0365  41599.8556  RR  LL HNC18O
#  30       4096 LSRK  43402.4488  12.206796   49999.0365  43427.4483  RR  LL SiO

# Scans: 230-236,253-256 Setup 1 H2CO et al
# Scans: 237-240,249-252 Setup 2 HC3N et al
# Scans: 241-248         Setup 3 SiO et al

casapath=os.environ['CASAPATH']
import asap as sd			#import ASAP package into CASA			#GBTIDL
os.environ['CASAPATH']=casapath		#IRC+10216 (HC3N line reduction only)
					#Notes:
					#scan numbers (zero-based) as compared to GBTIDL

					#changes made to get to IRC+10216_rawACSmod
					#  -- merge spectral windows with tolerance


s=sd.scantable('IRC+10216_rawACSmod',False)#load the data without averaging		# filein,'IRC.raw.fits'
#Cannot find any matching Tcal at/near the data timestamp. Set Tcal=0.0

#s.summary()				#summary info					# summary
											# fileout,'IRC+10216.reduced.fits'
s.set_fluxunit('K')         		# make 'K' default unit

scal=sd.calnod(s,[236,237,238,239,248,249,250,251])	# Calibrate HC3N scans		# for i=237,240,2 do begin getps,i,ifnum=0,plnum=0,units='Ta*',
del s                                   # remove s from memory
# recalculate az/el (NOT needed for GBT data)
antennaname = scal.get_antennaname()
if ( antennaname != 'GBT'): scal.recalc_azel()      # recalculate az/el to 		# tau=0.09 & accum & getps, i, ifnum=0,plnum=1,units='Ta*',
scal.opacity(0.09)			# do opacity correction				# tau=0.09 & accum & end & ave
sel=sd.selector()			# Prepare a selection				# copy,0,9
sel.set_ifs(17)				# select HC3N IF				# for i=250,252,2 do begin getps,i,ifnum=0,plnum=0,units='Ta*',
scal.set_selection(sel)			# get this IF					# tau=0.09 & accum & getps, i, ifnum=0,plnum=1,units='Ta*',
stave=sd.average_time(scal,weight='tintsys')	# average in time			# tau=0.09 & accum & end & ave
spave=stave.average_pol(weight='tsys')	# average polarizations;Tsys-weighted average   # accum
sd.plotter.plot(spave)			# plot						# copy,9,0
											# accum
					# do some smoothing
spave.smooth('boxcar',5)		# boxcar 5					# boxcar,5
spave.auto_poly_baseline(order=2,threshold=5,chan_avg_limit=4)	# baseline fit order=2	# nfit,2
sd.plotter.plot(spave)			# plot						# baseline

spave.set_unit('GHz')									# freq
sd.plotter.plot(spave)
sd.plotter.set_histogram(hist=True)     # draw spectrum using histogram                 # histogram
sd.plotter.axhline(color='r',linewidth=2) # zline                                       # zline
sd.plotter.save('irc_hc3n_reduced.eps')	  # save postscript spectrum			# write_ps,'irc_hc3n.ps'

spave.set_unit('channel')								# chan
rmsmask=spave.create_mask([200,1500])  # get rms of line free regions                   # stats,200,1500
rms=spave.stats('rms',mask=rmsmask)                                                                
					#---------------------------------------------- # Chans   bchan   echan      Xmin      Xmax      Ymin      Ymax
											#  1301     200    1500     200.0    1500.0  -0.14659   0.12467
					#  rms
					#---------------------------------------------- #                 Mean      Median      RMS  Variance      Area
					#Scan[0] (IRC+10216_nod) Time[2006/01/19/11:08:54]:#            0.0044323  0.0036073 0.035442 0.0012561  -37.672
					# IF[17] = 0.022
					#----------------------------------------------
					# LINE
linemask=spave.create_mask([1800,2400])
max=spave.stats('max',linemask)	   	#  IF[17] = 1.827
sum=spave.stats('sum',linemask)	   	#  IF[17] = 474.135
median=spave.stats('median',linemask) 	#  IF[17] = 1.172
mean=spave.stats('mean',linemask)	#  IF[17] = 0.789
											# Chans  bchan    echan      Xmin      Xmax        Ymin     Ymax
											#   601   1800     2400    1800.0    2400.0   -0.089494   1.8982
											#                  Mean    Median       RMS    Variance     Area
											#               0.80958    1.1883   0.65248     0.42573   486.55
# Save the spectrum
spave.save('irc_hc3n','ASCII',True)	# save the spectrum			# write_ascii,'irc_hc3n.spc'
#spave.save('irc_hc3n.ms','MS2',True) # save as an MS				# NA
#
endProc = time.clock()
endTime = time.time()

# --- end of irc hc3n script

#irc_max=1.827
#irc_rms=0.022
#irc_sum=474.123
# Regression values of CASA 2.3(#6654)+ASAP 2.2.0(#1448)
# on 64bit REL5.2 (2008/12/01)
irc_max=1.827
irc_rms=0.02213
irc_sum=474.1
diff_max = abs((irc_max-max[0])/irc_max)
diff_rms = abs((irc_rms-rms[0])/irc_rms)
diff_sum = abs((irc_sum-sum[0])/irc_sum)

import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='irc.hc3n.'+datestring+'.log'
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
        print >>logfile,'Passed Regression test for IRC-HC3N'
        print >>logfile,'---'
else:
	regstate=False
        print >>logfile,'----FAILED Regression test for IRC-HC3N'
print >>logfile,'*********************************'

print >>logfile,''
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
#print 'Processing rate MB/s  was: ', 35.1/(endTime - startTime)

logfile.close()
