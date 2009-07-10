###############################################
#                                             #
# Regression/Benchmarking Script for NGC 4826 #
#        Single Dish + Synthesis              #
###############################################

import time
import os

os.system('rm -rf n4826_t* mosaic.flux mosaic.model mosaic.image mosaic.residual n12m_gaussian.im')

pathname=os.environ.get('CASAPATH').split()[0]
datapath = pathname + '/data/regression/ATST3/NGC4826/'

print '--Copy data to local directory--'
mspath='cp -r '+datapath+'n4826_both.ms .'
os.system(mspath)
os.system('chmod -R a+wx n4826_both.ms')

startTime = time.time()
startProc = time.clock()


print '--Feather--'
# Starting from:
#    BIMA mosaic image (Moment 0) : n4826_mom0.im
#    NRAO 12m OTF image (Moment 0): n4826_12mmom0.im
default('feather')
feather('n4826_tfeather.im',datapath+'n4826_mom0.im',datapath+'n4826_12mmom0.im')
feathertime = time.time() 
#combo: Max:1.533498e+02        Flux:1.523515e+03 Jy    rms:1.187669e+01
#Pcombo:Max:1.511604e+02        Flux:2.016790e+03 Jy    rms:1.873132e+01
#BIMA:  Max:1.627327e+02        Flux:1.035314e+04 Jy    rms:1.587101e+01
#12m:   Max:9.939910e+02        Flux:2.600573e+03 Jy    rms:1.159703e+02

print '--Feather cube--'
# Starting from:
#    BIMA mosaic image (Moment 0) : n4826_bima.im
#    NRAO 12m OTF image (Moment 0): NGC4826.12motf.chan.fits
default('importfits')
importfits(datapath+'NGC4826.12motf.chan.fits','n4826_t12mchan.im')
default('imhead')
imhead('n4826_t12mchan.im',mode='put',hdkey='bunit',hdvalue='Jy/beam')
#imhead('n4826_t12mchan.im',mode='put',hdkey='beam',hdvalue='55arcsec, 55arcsec, 0deg')
###wow what an idea...more to type to change beam
imhead('n4826_t12mchan.im',mode='put',hdkey='beammajor',hdvalue='55arcsec')
imhead('n4826_t12mchan.im',mode='put',hdkey='beamminor',hdvalue='55arcsec')
imhead('n4826_t12mchan.im',mode='put',hdkey='beampa',hdvalue='0deg')


default('feather')
feather('n4826_tfeathercube.im',datapath+'n4826_bima.im','n4826_t12mchan.im')
feathercubetime = time.time()
# Maximum value   1.881626e+00 at [113, 115, 1, 25] (12:56:45.388, +21.40.51.100, I, 1.15172e+11Hz)
# Flux density  =   1.054628e+02 Jy

print '--Feather cube - create synth image--'
# Starting from:
#    BIMA calibrated visibilities: n4826_both.ms
#    NRAO 12m OTF cube: NGC4826.12motf.chan.fits
default('clean')
clean(vis='n4826_both.ms', imagename='mosaic',
      nchan=30, start=46, width=4, spw='0~2',
      field='0~6',
      cell=[1., 1.], imsize=[256, 256],
      stokes='I',
      mode='channel',
      psfmode='clark',
      imagermode='mosaic',
      niter=500,
      gain=0.1,                     # Specified for certainty, 6/10/2009
      scaletype='SAULT', minpb=0.1) # Changed from 0.01 6/10/2009
default('importfits')
importfits(datapath+'NGC4826.12motf.chan.fits', 'n4826_t12mchan2.im')
default('imhead')
imhead('n4826_t12mchan2.im', mode='put', hdkey='bunit', hdvalue='Jy/beam')
#imhead('n4826_t12mchan2.im',mode='put',hdkey='beam',hdvalue='55arcsec,55arcsec,0deg')
imhead('n4826_t12mchan2.im',mode='put',hdkey='beammajor',hdvalue='55arcsec')
imhead('n4826_t12mchan2.im',mode='put',hdkey='beamminor',hdvalue='55arcsec')
imhead('n4826_t12mchan2.im',mode='put',hdkey='beampa',hdvalue='0deg')
default('feather')
feather('n4826_tfeathersynth.im','mosaic.image','n4826_t12mchan2.im')
#Combo:   Max: 1.881626e+00   Flux: 1.054628e+02 Jy   Rms: 8.911050e-02
#Pcombo:  Max: 1.681789e+00   Flux: 1.454376e+02 Jy   Rms: 8.196454e-02
feathersynthtime = time.time()

print '--Single Dish as Model--'
# Starting from:
#    BIMA calibrated visibilities: n4826_both.ms
#    NRAO 12m OTF cube: n4826_t12mchan.im
default('clean')
clean(vis='n4826_both.ms', imagename='n4826_tjoint1',
      nchan=30, start=46, width=4, spw='0~2',
      field='0~6',
      cell=[1., 1.], imsize=[256, 256],
      stokes='I',
      mode='channel',
      psfmode='clark',
      niter=500,
      imagermode='mosaic',
      modelimage='n4826_t12mchan.im',
      scaletype='SAULT', minpb=0.01)
sdmodeltime = time.time()
#Combo:   Max: 1.891995e+00    Flux: 1.391578e+02 Jy   Rms: 9.056366e-02
#Pcombo:  Max: 1.681789e+00    Flux: 1.454376e+02 Jy   Rms: 8.196454e-02
#        im:=image('n4826_joint1.restored');
#        im_mom0:=im.moments(outfile='n4826_tmom0.im', moments=0, axis=4,
#                mask='indexin(4,[5:26])',includepix=[0.070,1000.0]);
#Combo: Max: 1.891995e+00       Flux:1.098632e+04 Jy    Rms: 1.678347e+01
#BIMA:  Max:1.627327e+02        Flux:1.035314e+04 Jy    rms:1.587101e+01
#Pcombo:Max:1.511604e+02        Flux:2.016790e+03 Jy    rms:1.873132e+01

print '--Joint Deconvolution--'
# Regrid 12m image onto synth imaging coordinates
# Tool-kit only (currently)
ia.open('n4826_tjoint1.image')
csys=ia.coordsys()
ia.close()
#### regrid SD image so that shape and coordinate parameters match
ia.open('n4826_t12mchan.im')
ia.regrid(outfile='n4826_t12motf.chregrid.im',shape=[256,256,1,30],csys=csys.torecord())
ia.close()
#### deconvolve SD image with a guess of PB with msclean
dc.open('n4826_t12motf.chregrid.im', psf='')
dc.makegaussian('n12m_gaussian.im' ,bmaj='55arcsec', bmin='55arcsec', bpa='0deg',
		normalize=false)
dc.close()
dc.open('n4826_t12motf.chregrid.im', psf='n12m_gaussian.im')
dc.setscales(scalemethod='uservector', uservector=[30., 60.])
dc.clean(algorithm='msclean', model='n4826_tjoint2', niter=100, gain=0.3)
#dc.close()
default('clean')
##### Mosaic the interferometer data...use model from obtain from deconvolve
##### SD image as starting model
clean(vis='n4826_both.ms', imagename='n4826_tjoint2',
      nchan=30, start=46, width=4, spw='0~2',
      field='0~6',
      cell=[1., 1.], imsize=[256, 256],
      stokes='I',
      mode='channel',
      psfmode='clark',
      imagermode='mosaic',
      niter=500,
      modelimage='n4826_tjoint2',
      scaletype='SAULT', minpb=0.01)

jointtime = time.time()

endProc = time.clock()
endTime = time.time()

# Regression

test_name_feather1 = 'NGC4826--Feather (Synthesis and SD Moment 0 images'
test_name_feather2 = 'NGC4826--Feather Cube (Synthesis and SD data cube'
test_name_feather3 = 'NGC4826--Create Synth cube; feather with SD FITS cube'
test_name_jc1      = 'NGC4826--Joint deconvolution with SD FITS cube as model'
test_name_jc2      = 'NGC4826--Joint deconvolution with deconvolved SD FITS cube'

ia.open('n4826_tfeather.im')
ia.setbrightnessunit('Jy/beam')
ia.close()

feather1_im=ia.open('n4826_tfeather.im')
f1_stats=ia.statistics();ia.close()
feather2_im=ia.open('n4826_tfeathercube.im')
f2_stats=ia.statistics();ia.close()
feather3_im=ia.open('n4826_tfeathersynth.im')
f3_stats=ia.statistics();ia.close()
jcfeather1_im=ia.open('n4826_tjoint1.image')
jc1_stats=ia.statistics();ia.close()
jcfeather2_im=ia.open('n4826_tjoint2.image')
jc2_stats=ia.statistics()

feather1_immax=f1_stats['max'][0]
feather1_flux=f1_stats['flux'][0]
feather2_immax=f2_stats['max'][0]
feather2_flux=f2_stats['flux'][0]
feather3_immax=f3_stats['max'][0]
feather3_flux=f3_stats['flux'][0]
jc1_immax=jc1_stats['max'][0]
joint1_flux=jc1_stats['flux'][0]
jc2_immax=jc2_stats['max'][0]
joint2_flux=jc2_stats['flux'][0]


#Note flux values differ from AIPS++; now correcting for primary beam so raises
#the noise at the edges; should do stats on central region (soon)
###these numbers are really fragile to minor changes...like the total flux
### makes no sense at all. better crieria needed
f1_max=153.3498
f1_flux=1523.515
f2_max=1.8816
f2_flux=105.4628
#f3_max=1.67
f3_max=1.52
f3_flux=104.25
jc1_max=1.61
#jc1_max=1.71
#jc1_flux=168.87
jc1_flux=243.57
#jc2_max=1.68
jc2_max=1.50
#jc2_flux=67.27
jc2_flux=144.49

diff_f1=abs((f1_max-feather1_immax)/f1_max)
diff_f1f=abs((f1_flux-feather1_flux)/f1_flux)
diff_f2=abs((f2_max-feather2_immax)/f2_max)
diff_f2f=abs((f2_flux-feather2_flux)/f2_flux)
diff_f3=abs((f3_max-feather3_immax)/f3_max)
diff_f3f=abs((f3_flux-feather3_flux)/f3_flux)
diff_jc1=abs((jc1_max-jc1_immax)/jc1_max)
diff_jc1f=abs((jc1_flux-joint1_flux)/jc1_flux)
diff_jc2=abs((jc2_max-jc2_immax)/jc2_max)
diff_jc2f=abs((jc2_flux-joint2_flux)/jc2_flux)

import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='ngc4826c.'+datestring+'.log'
logfile=open(outfile, 'w')

print >>logfile, ''
print >>logfile, '********** Data Summary *********'
print >>logfile, ''
print >>logfile, '*   Observer:      Project: t108c115.n48'
print >>logfile, '*Observation: BIMA(10 antennas)'
print >>logfile, '*  Telescope Observation Date    Observer       Project'
print >>logfile, '*  BIMA      [                   4.39941e+09,  4.39942e+09]               t108c115.n48'
print >>logfile, '*Data records: 109260       Total integration time = 628492 seconds'
print >>logfile, '*   Observed from   04:04:31   to   10:39:23'
print >>logfile, '*Fields: 7'
print >>logfile, '*  ID   Name          Right Ascension  Declination   Epoch'
print >>logfile, '*  0    NGC4826-F0    12:56:44.24      +21.41.05.10  J2000'
print >>logfile, '*  1    NGC4826-F1    12:56:41.08      +21.41.05.10  J2000'
print >>logfile, '*  2    NGC4826-F2    12:56:42.66      +21.41.43.20  J2000'
print >>logfile, '*  3    NGC4826-F3    12:56:45.82      +21.41.43.20  J2000'
print >>logfile, '*  4    NGC4826-F4    12:56:47.40      +21.41.05.10  J2000'
print >>logfile, '*  5    NGC4826-F5    12:56:45.82      +21.40.27.00  J2000'
print >>logfile, '*  6    NGC4826-F6    12:56:42.66      +21.40.27.00  J2000'
print >>logfile, '*Spectral Windows:  (4 unique spectral windows and 1 unique polarization setups)'
print >>logfile, '*  SpwID  #Chans Frame Ch1(MHz)    Resoln(kHz) TotBW(kHz)  Ref(MHz)    Corrs'
print >>logfile, '*  0          64 LSRD  114950.387  1562.5      100000      115271.2    YY'
print >>logfile, '*  1          64 LSRD  115040.402  1562.5      100000      115271.2    YY'
print >>logfile, '*  2          64 LSRD  115130.143  1562.5      100000      115271.2    YY'
print >>logfile, '*  3          64 LSRD  115220.157  1562.5      100000      115271.2    YY'
print >>logfile, '*Antennas: 10'
print >>logfile, '*   ID=   1-3: ANT1=UNKNOWN, ANT2=UNKNOWN, ANT3=UNKNOWN, ANT4=UNKNOWN,'
print >>logfile,'*   ID=   5-7: ANT5=UNKNOWN, ANT6=UNKNOWN, ANT7=UNKNOWN, ANT8=UNKNOWN,'
print >>logfile,'*   ID=   9-9: ANT9=UNKNOWN, ANT10=UNKNOWN'
print >>logfile,'*********************************'
print >>logfile,''
print >>logfile,'********** Regression ***********'
print >>logfile,'*                               *'
if (diff_f1 < 0.05): print >>logfile,'* Passed Feather 1 image max test '
print >>logfile,'*--  Feather 1: Image max '+str(feather1_immax)+','+str(f1_max)
if (diff_f2 < 0.05): print >>logfile,'* Passed Feather 2 image max test'
print >>logfile,'*--  Feather 2: Image max '+str(feather2_immax)+','+str(f2_max)
if (diff_f3 < 0.05): print >>logfile,'* Passed Feather 3 image max test'
print >>logfile,'*--  Feather 3: Image max '+str(feather3_immax)+','+str(f3_max)
if (diff_jc1 < 0.05): print >>logfile,'* Passed Joint Deconvolution 1 image max test' 
print >>logfile,'*--  Joint Decon1: Image max '+str(jc1_immax)+','+str(jc1_max)
if (diff_jc2 < 0.05): print >>logfile,'* Passed Joint Deconvolution 2 image max test'
print >>logfile,'*--  Joint Decon2: Image max '+str(jc2_immax)+','+str(jc2_max)
if (diff_f1f < 0.05): print >>logfile,'* Passed Feather 1 flux test '
print >>logfile,'*--  Feather 1: Flux '+str(feather1_flux)+','+str(f1_flux)
if (diff_f2f < 0.05): print >>logfile,'* Passed Feather 2 flux test'
print >>logfile,'*--  Feather 2: Flux '+str(feather2_flux)+','+str(f2_flux)
if (diff_f3f < 0.05): print >>logfile,'* Passed Feather 3 flux test'
print >>logfile,'*--  Feather 3: Flux '+str(feather3_flux)+','+str(f3_flux)
if (diff_jc1f < 0.05): print >>logfile,'* Passed Joint Deconvolution flux test'
print >>logfile,'*--  Joint Decon1: Flux '+str(joint1_flux)+','+str(jc1_flux)
if (diff_jc2f < 0.05): print >>logfile,'* Passed Joint Deconvolution flux test'
print >>logfile,'*--  Joint Decon2: Flux '+str(joint2_flux)+','+str(jc2_flux)

if ((diff_f1<0.05) & (diff_f2<0.05) & (diff_f3<0.05) & (diff_jc1<0.05) & (diff_f1f<0.05) & (diff_f2f<0.05) & (diff_f3f<0.05) & (diff_jc1f<0.05)): 
	regstate=True
	print >>logfile,'---'
	print >>logfile,'Passed Regression test for NGC4826'
	print >>logfile,'---'
else: 
	regstate=False
	print >>logfile,'----FAILED Regression test for NGC4826'
print >>logfile,'*********************************'

print >>logfile,''
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,'Processing rate MB/s  was: '+str(760./(endTime - startTime))
# n4826_both.ms x 3 plus image sizesish
print >>logfile,'* Breakdown:                           *'
print >>logfile,'*   feather      time was: '+str(feathertime-startTime)
print >>logfile,'*   feathercube  time was: '+str(feathercubetime-feathertime)
print >>logfile,'*   feathersynth time was: '+str(feathersynthtime-feathercubetime)
print >>logfile,'*   sdmodel      time was: '+str(sdmodeltime-feathersynthtime)
print >>logfile,'*   joint decon  time was: '+str(jointtime-sdmodeltime)
print >>logfile,'*****************************************'

logfile.close()
