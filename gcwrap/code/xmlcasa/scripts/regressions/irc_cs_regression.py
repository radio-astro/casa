###############################
#
# IRC+10216 13CS Reduction Script
# Nod data
#
###############################
import time
import os

os.system('rm -rf IRC+10216_rawACSmod irc_cs_reducedSCAN0_CYCLE0_BEAM0_IF0.txt irc_cs_reduced.eps irc_cs_fit.txt')

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

# Scans: 230-231,233-236,253-256 Setup 1 H2CO et al
# Scans: 237-240,249-252 Setup 2 HC3N et al
# Scans: 241-248         Setup 3 SiO et al

casapath=os.environ['CASAPATH']
import asap as sd			#import ASAP package into CASA			#GBTIDL
os.environ['CASAPATH']=casapath		#IRC+10216 (CS line reduction only)
					#Notes:
					#scan numbers (zero-based) as compared to GBTIDL

					#changes made to get to IRC+10216_rawACSmod
					#  -- merge spectral windows with tolerance


s=sd.scantable('IRC+10216_rawACSmod',False)#load the data without averaging		# filein,'IRC.raw.fits'
#Cannot find any matching Tcal at/near the data timestamp. Set Tcal=0.0

#s.summary()				#summary info					# summary
											# fileout,'IRC+10216.reduced.fits'
s.set_fluxunit('K')         		# make 'K' default unit

scal=sd.calnod(s,[229,230])		# Calibrate CS scans	                        # for i=230,231,2 do begin getps,i,ifnum=3,plnum=0,units='Ta*',
del s                                   # remove s from memory
# recalculate az/el (NOT needed for GBT data)
antennaname = scal.get_antennaname()
if ( antennaname != 'GBT'): scal.recalc_azel() 	  # recalculate az/el to 		# tau=0.09 & accum & getps, i, ifnum=3,plnum=1,units='Ta*',
scal.opacity(0.09)			# do opacity correction				# tau=0.09 & accum & end & ave
sel=sd.selector()			# Prepare a selection				# 
sel.set_ifs(3)				# select CS IF					#
scal.set_selection(sel)			# get this IF					#
stave=sd.average_time(scal,weight='tintsys')	# average in time			# 
spave=stave.average_pol(weight='tsys')	# average polarizations;Tsys-weighted average   #
sd.plotter.plot(spave)			# plot						# 
											# 
					# do some smoothing				# 
spave.smooth('boxcar',5)		# boxcar 5					# boxcar,5
spave.auto_poly_baseline(order=1,threshold=5,chan_avg_limit=4)	# baseline fit order=2	# nregion,[200,1500,2600,3500]
sd.plotter.plot(spave)			# plot						# nfit,2
											# baseline
spave.set_unit('GHz')									# freq
sd.plotter.plot(spave)
sd.plotter.set_histogram(hist=True)     # draw spectrum using histogram                 # histogram
sd.plotter.axhline(color='r',linewidth=2) # zline                                       # zline
sd.plotter.save('irc_cs_reduced.eps')	# save postscript spectrum			# write_ps,'irc_cs.ps'

spave.set_unit('channel')								# chan
rmsmask=spave.create_mask([800,1500])  # get rms of line free regions                   # stats,800,1500
rms=spave.stats(stat='rms',mask=rmsmask)                                                                
					#---------------------------------------------- # Chans   bchan   echan      Xmin      Xmax      Ymin      Ymax
											#   701     800    1500     800.0    1500.0  -0.29621   0.36313
					#  rms
					#---------------------------------------------- #              Mean      Median      RMS  Variance      Area
					#Scan[0] (IRC+10216_nod) Time[2006/01/19/01:52:05]: #     -0.0037430   -0.014653 0.10816 0.011699  -2.6239
					# IF[3] = 0.147
					#----------------------------------------------
					# LINE
linemask=spave.create_mask([1850,2300])
max=spave.stats('max',linemask)	   	#  IF[3] = 3.325
sum=spave.stats('sum',linemask)	   	#  IF[3] = 627.922
median=spave.stats('median',linemask) 	#  IF[3] = 1.305
mean=spave.stats('mean',linemask)	#  IF[3] = 1.392
											#  Chans  bchan    echan      Xmin      Xmax       Ymin    Ymax
											#    451   1850     2300    1850.0    2300.0   -0.36467  2.3805
											#                   Mean    Median       RMS   Variance    Area
											#                0.98944   0.92886   0.54541    0.29747  446.24

# Save the spectrum
spave.save('irc_cs.spc','ASCII',True)	# save the spectrum				# write_ascii,'irc_cs.spc'
#spave.save('irc_cs.ms','MS2',True) # save as an MS					# NA
#
endProc = time.clock()
endTime = time.time()

# --- end of irc cs script
#irc_max=3.3
#irc_rms=0.147
#irc_sum=627.5
# Regression values of CASA 2.3(#6654)+ASAP 2.2.0(#1448)
# on 64bit REL5.2 (2008/12/01)
irc_max=3.325
irc_rms=0.1473
irc_sum=627.9
diff_max = abs((irc_max-max[0])/irc_max)
diff_rms = abs((irc_rms-rms[0])/irc_rms)
diff_sum = abs((irc_sum-sum[0])/irc_sum)

import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='irc.cs.'+datestring+'.log'
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
        print >>logfile,'Passed Regression test for IRC-CS'
        print >>logfile,'---'
else:
	regstate=False
        print >>logfile,'----FAILED Regression test for IRC-CS'
print >>logfile,'*********************************'

print >>logfile,''
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
#print 'Processing rate MB/s  was: ', 35.1/(endTime - startTime)

logfile.close()
