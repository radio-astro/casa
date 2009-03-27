###############################################
#                                             #
# Regression/Benchmarking Script for Orion    #
#        Single Dish + Synthesis              #
###############################################

import time
import os

os.system('rm -rf orion_t* gbt_gau.im')

pathname=os.environ.get('CASAPATH')
datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/ATST3/Orion/'

print '--Copy data to local directory--'
mspath='cp -r '+datapath+'orion.ms .'
os.system(mspath)
os.system('chmod -R a+wx orion.ms')

startTime = time.time()
startProc = time.clock()


print '--Feather--'
# Starting from:
#    VLA Orion mosaic image : orion_vlamem.im
#    GBT OTF image : orion.gbt.im
feather('orion_tfeather.im',datapath+'orion_vlamem.im',datapath+'orion.gbt.im')
feathertime = time.time() 
#GBT:   Max:5.129806e+01        Flux:2.425065e+02 Jy    rms:1.277546e+01
#VLA:   Max:8.340111e-01        Flux:1.891523e+02 Jy    rms:1.099514e-01

print '--Feather - create synth image--'
# Starting from:
#    VLA Orion mosaic image : orion.ms
#    GBT OTF image : orion.gbt.im
#    Some details about the mosaic:
#    primary beam at X-band = 5.4',
#    mosaic field spacing = 2.5'
#    total mosaic size = approx. 9.5' = 570"
#    synthesized beam size = 8.4" in D config at 3.6 cm, 8.3 GHz
#    cell size = 2" and nx,ny = 300 (600" field size)
#    phase center = center field: J2000 05:35:17.470, -005.23.06.790
#    NOTE: field 10 is outside of the 9 point primary mosaic (sitting
#     on M43 -- but the flux is resolved out so there is no use to
#     add it to the mosaic.  The script below leaves it out.
default('mosaic')
mosaic('orion.ms','orion.task','mfs','entropy',niter=31,sigma='4mJy',targetflux='180Jy',mask=datapath+'orion.mask6',field=[2,3,4,5,6,7,8,9,10],spw=[0,1],stokes='I',cell=[2,2],imsize=[300,300],weighting='briggs',mosweight=True,rmode='norm',robust=-1,cyclefactor=4,cyclespeedup=500,prior='',phasecenter=6,ftmachine='ft',minpb=0.1, scaletype='PBCOR')
feather('orion_tfeather2.im','orion.task.image',datapath+'orion.gbt.im')
#GBT:   Max:5.129806e+01        Flux:2.425065e+02 Jy    rms:1.277546e+01
#VLA:   Max:8.340111e-01        Flux:1.891523e+02 Jy    rms:1.099514e-01
feathersynthtime = time.time()

print '--Single Dish as Model (multi-scale)--'
## Starting from:
##    VLA calibrated visibilities: orion.ms
##    GBT OTF cube: orion.gbt.im
default('mosaic')
mosaic('orion.ms','orion_tsdms','mfs','multiscale',niter=10000,gain=0.2,threshold=10.,mask=datapath+'orion.mask6',field=range(2,11),spw=[0,1],stokes='I',cell=[2,2],imsize=[300,300],weighting='briggs',mosweight=True,rmode='norm',robust=-1,negcomponent=-1,cyclefactor=4,cyclespeedup=500,phasecenter=6,scales=[0,3,10,30],ftmachine='ft',minpb=0.1, scaletype='PBCOR')
sdmodelmstime = time.time()
###combo: Max:1.195286e+00        Flux:2.873779e+02 Jy    rms:9.069330e-02
###GBT:   Max:5.129806e+01        Flux:2.425065e+02 Jy    rms:1.277546e+01
###VLA:   Max:8.340111e-01        Flux:1.891523e+02 Jy    rms:1.099514e-01
##
print '--Single Dish as Model (MEM)--'
### Starting from:
###    VLA calibrated visibilities: orion.ms
###    GBT OTF cube: orion.gbt.im
datapath=os.environ.get('CASAPATH').split()[0]+'/data/regression/ATST3/Orion/'
default('mosaic')
mosaic('orion.ms','orion_tsdmem','mfs','entropy',niter=3,sigma='4mJy',targetflux='240Jy',mask=datapath+'orion.mask6',field=[2,3,4,5,6,7,8,9,10],spw=[0,1],stokes='I',cell=[2,2],imsize=[300,300],weighting='briggs',mosweight=True,rmode='norm',robust=-1,cyclefactor=4,cyclespeedup=500,phasecenter=6,modelimage='orion_tsdmem',sdimage=datapath+'orion.gbt.im',ftmachine='ft',prior='orion_tsdmem',minpb=0.1, scaletype='PBCOR')
sdmodelmemtime=time.time()
###GBT:   Max:5.129806e+01        Flux:2.425065e+02 Jy    rms:1.277546e+01
###VLA:   Max:8.340111e-01        Flux:1.891523e+02 Jy    rms:1.099514e-01
##
######
#### Create synthesis (BIMA) data cube, deconvolve single dish cube
#### DO joint deconvolution
######
print '--Joint deconvolution --'
#
#Regrid GBT image onto synth imaging coordinates
ia.open('orion_tsdmem.image')
csys=ia.coordsys()
ia.close()
ia.open(datapath+'orion.gbt.im')
ia.regrid(outfile='orion_tgbt_regrid.im',shape=[300,300,1,1],csys=csys.torecord(),overwrite=True)
ia.close()
#
#Deconvolve GBT image
dc.open('orion_tgbt_regrid.im',psf='')
#make gaussian for psf (best guess for GBT beam b ased on beamsize report in GBT image)
dc.makegaussian('gbt_gau.im',bmaj='55arcsec',bmin='55arcsec',bpa='0deg',normalize=False)
dc.close()
dc.open('orion_tgbt_regrid.im',psf='gbt_gau.im')
dc.setscales(scalemethod='uservector',uservector=[30.,100.,300.])
dc.clean(algorithm='msclean',model='orion_tjoint3',niter=500,gain=0.4,mask=datapath+'orion.mask6',threshold='0.1Jy')
dc.close()
#default('mosaic')

im.open('orion.ms')
im.selectvis(field=[2,3,4,5,6,7,8,9,10],spw=[0,1])
im.defineimage(nx=300,cellx='2arcsec',phasecenter=6,spw=[0,1])
im.setvp(dovp=True)
im.setscales(scalemethod='uservector',uservector=[0,3,10,30,100]);
###if clean componenent for large scale goes negative continue to use that scale
im.setmfcontrol(stoplargenegatives=-1, cyclefactor=4, cyclespeedup=500)
im.weight(type='briggs',rmode='norm',robust=-1,mosaic=True);
im.clean(algorithm='mfmultiscale',model='orion_tjoint3',image='orion_tjoint3.image', gain=0.2,niter=1000,mask=datapath+'orion.mask6')
jointmemtime = time.time()
#
endProc = time.clock()
endTime = time.time()

# Regression

feather1_im=ia.open('orion_tfeather.im')
f1_stats=ia.statistics();ia.close()
feather2_im=ia.open('orion_tfeather2.im')
f2_stats=ia.statistics();ia.close()
sdmodel1_im=ia.open('orion_tsdms.image')
sd1_stats=ia.statistics();ia.close()
sdmodel2_im=ia.open('orion_tsdmem.image')
sd2_stats=ia.statistics();ia.close()
jd1_im=ia.open('orion_tjoint3.image')
jd1_stats=ia.statistics()

feather1_immax=f1_stats['max'][0]
feather1_flux=f1_stats['flux'][0]
feather2_immax=f2_stats['max'][0]
feather2_flux=f2_stats['flux'][0]
sd1_immax=sd1_stats['max'][0]
sd1_imflux=sd1_stats['flux'][0]
sd2_immax=sd2_stats['max'][0]
sd2_imflux=sd2_stats['flux'][0]
jd1_immax=jd1_stats['max'][0]
jd1_imflux=jd1_stats['flux'][0]
#jd1_immax=1.014
#jd1_imflux=360.468
#

f1_max=0.780
f1_flux=242.506
f2_max=0.868
f2_flux=242.506
sd1_max=0.935
sd1_flux=187
sd2_max=0.87
sd2_flux=289
jd1_max=1.03
jd1_flux=362
#
diff_f1=abs((f1_max-feather1_immax)/f1_max)
diff_f1f=abs((f1_flux-feather1_flux)/f1_flux)
diff_f2=abs((f2_max-feather2_immax)/f2_max)
diff_f2f=abs((f2_flux-feather2_flux)/f2_flux)
diff_sd1=abs((sd1_max-sd1_immax)/sd1_max)
diff_sd1f=abs((sd1_flux-sd1_imflux)/sd1_flux)
diff_sd2=abs((sd2_max-sd2_immax)/sd2_max)
diff_sd2f=abs((sd2_flux-sd2_imflux)/sd2_flux)
diff_jd1=abs((jd1_max-jd1_immax)/jd1_max)
diff_jd1f=abs((jd1_flux-jd1_imflux)/jd1_flux)
#
import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='orion.'+datestring+'.log'
logfile=open(outfile,'w')

print >>logfile,''
print >>logfile,'********** Data Summary *********'
print >>logfile,'           GBT image '
print >>logfile,'Image name       : orion.gbt.im'
print >>logfile,'Object name      :'
print >>logfile,'Image type       : PagedImage'
print >>logfile,'Image quantity   : Intensity'
print >>logfile,'Pixel mask(s)    : mask0'
print >>logfile,'Region(s)        : None'
print >>logfile,'Image units      : Jy/beam'
print >>logfile,'Restoring Beam   : 98.0547 arcsec, 98.0547 arcsec, 79.25 deg'
print >>logfile,'Direction reference : J2000'
print >>logfile,'Spectral  reference : LSRK'
print >>logfile,'Velocity  type      : RADIO'
print >>logfile,'Rest frequency      : 1 Hz'
print >>logfile,'Telescope           : GBT'
print >>logfile,'Observer            : UNKNOWN'
print >>logfile,'Date observation    : UNKNOWN'
print >>logfile,'Axis Coord Type      Name             Proj Shape Tile   Coord value at pixel    Coord incr Units'
print >>logfile,'------------------------------------------------------------------------------------------------'
print >>logfile,'1    1     Direction Right Ascension   SIN   300  300  05:35:17.470   151.00 -2.000000e+00 arcsec'
print >>logfile,'2    1     Direction Declination       SIN   300  300 -05.23.06.790   151.00  2.000000e+00 arcsec'
print >>logfile,'3    2     Stokes    Stokes                    1    1             I'
print >>logfile,'4    3     Spectral  Frequency                 1    1    8.4351e+09     1.00  6.050000e+07 Hz'
print >>logfile,'                     Velocity                                  -inf     1.00           nan km/s'
print >>logfile,'   Observer: unavailable     Project: DSTST'
print >>logfile,'Observation: VLA(26 antennas)'
print >>logfile,'Data records: 1093716       Total integration time = 8545 seconds'
print >>logfile,'   Observed from   11:15:48   to   13:38:13'
print >>logfile,'Fields: 12'
print >>logfile,'  ID   Name          Right Ascension  Declination   Epoch'
print >>logfile,'  0    0518+165      05:21:09.89      +16.38.22.04  J2000'
print >>logfile,'  1    0539-057      05:41:38.09      -05.41.49.43  J2000'
print >>logfile,'  2    ORION1        05:35:07.42      -05.25.36.07  J2000'
print >>logfile,'  3    ORION2        05:35:17.42      -05.25.36.79  J2000'
print >>logfile,'  4    ORION3        05:35:27.42      -05.25.37.52  J2000'
print >>logfile,'  5    ORION4        05:35:27.47      -05.23.07.52  J2000'
print >>logfile,'  6    ORION5        05:35:17.47      -05.23.06.79  J2000'
print >>logfile,'  7    ORION6        05:35:07.47      -05.23.06.07  J2000'
print >>logfile,'  8    ORION7        05:35:07.52      -05.20.36.07  J2000'
print >>logfile,'  9    ORION8        05:35:17.52      -05.20.36.80  J2000'
print >>logfile,'  10   ORION9        05:35:27.52      -05.20.37.52  J2000'
print >>logfile,'  11   ORION10       05:35:32.61      -05.16.07.88  J2000'
print >>logfile,'Spectral Windows:  (2 unique spectral windows and 1 unique polarization setups)'
print >>logfile,'  SpwID  #Chans Frame Ch1(MHz)  Resoln(kHz) TotBW(kHz) Ref(MHz) Corrs'
print >>logfile,'  0           1 TOPO  8435.1    50000       50000      8435.1   RR  RL  LR  LL'
print >>logfile,'  1           1 TOPO  8485.1    50000       50000      8485.1   RR  RL  LR  LL'
print >>logfile,'*********************************'
print >>logfile,''
print >>logfile,'********** Regression ***********'
print >>logfile,'*                               *'
if (diff_f1 < 0.05): print >>logfile,'* Passed Feather 1 image max test '
print >>logfile,'*--  Feather 1: Image max'+str(feather1_immax)+','+str(f1_max)
if (diff_f2 < 0.05): print >>logfile,'* Passed Feather 2 image max test'
print >>logfile,'*--  Feather 2: Image max '+str(feather2_immax)+','+str(f2_max)
if (diff_sd1 < 0.05): print >>logfile,'* Passed SD Model (MS) image max test'
print >>logfile,'*--  SD Model (MS): Image max'+str(sd1_immax)+','+str(sd1_max)
if (diff_sd2 < 0.05): print >>logfile,'* Passed SD Model (MEM) image max test'
print >>logfile,'*--  SD Model (MEM): Image max'+str(sd2_immax)+','+str(sd2_max)
if (diff_jd1 < 0.05): print >>logfile,'* Passed Joint Deconvolution image max test'
print >>logfile,'*--  Joint Decon1: Image max'+str(jd1_immax)+','+str(jd1_max)
if (diff_f1f < 0.05): print >>logfile,'* Passed Feather 1 flux test '
print >>logfile,'*--  Feather 1: Flux '+str(feather1_flux)+','+str(f1_flux)
if (diff_f2f < 0.05): print >>logfile,'* Passed Feather 2 flux test'
print >>logfile,'*--  Feather 2: Flux '+str(feather2_flux)+','+str(f2_flux)
if (diff_sd1f < 0.05): print >>logfile,'* Passed Feather 3 flux test'
print >>logfile,'*--  SD Model (MS): Flux '+str(sd1_imflux)+','+str(sd1_flux)
if (diff_sd2f < 0.05): print >>logfile,'* Passed Joint Deconvolution flux test'
print >>logfile,'*--  SD Model (MEM): Flux '+str(sd2_imflux)+','+str(sd2_flux)
if (diff_jd1f < 0.05): print >>logfile,'* Passed Joint Deconvolution flux test'
print >>logfile,'*--  Joint Decon2: Flux '+str(jd1_imflux)+','+str(jd1_flux)

if ((diff_f1<0.05) & (diff_f2<0.05) & (diff_sd1<0.05) & (diff_sd2<0.05) & (diff_jd1<0.05) & (diff_f1f<0.05) & (diff_f2f<0.05) & (diff_sd1f<0.05) & (diff_sd2f<0.05) & (diff_jd1f < 0.05)):
        regstate=True
        print >>logfile,'---'
        print >>logfile,'Passed Regression test for Orion'
        print >>logfile,'---'
else:
        regstate=False
        print >>logfile,'----FAILED Regression test for Orion'
print >>logfile,'*********************************'

print >>logfile,''
print >>logfile,''
print >>logfile,'********* Benchmarking *****************'
print >>logfile,'*                                      *'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,'Processing rate MB/s  was: '+str(760./(endTime - startTime))
print >>logfile,'* Breakdown:                           *'
print >>logfile,'*   feather      time was: '+str(feathertime-startTime)
print >>logfile,'*   feathersynth time was: '+str(feathersynthtime-feathertime)
print >>logfile,'*   sdmodel (MS) time was: '+str(sdmodelmstime-feathersynthtime)
print >>logfile,'*   sdmodel(MEM) time was: '+str(sdmodelmemtime-sdmodelmstime)
print >>logfile,'*   joint decon  time was: '+str(jointmemtime-sdmodelmemtime)
print >>logfile,'*****************************************'
#
logfile.close()
