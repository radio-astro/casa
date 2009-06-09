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
mosaic('orion.ms',
       'orion.task',
       'mfs',
       'entropy',
       niter=31,
       sigma='4mJy',
       targetflux='180Jy',
       mask=datapath+'orion.mask6',
       field=[2, 3, 4, 5, 6, 7, 8, 9,10],
       spw=[0, 1],
       stokes='I',
       cell=[2, 2],
       imsize=[300, 300],
       weighting='briggs',
       mosweight=True,
       rmode='norm',
       robust=-1,
       cyclefactor=4,
       cyclespeedup=500,
       prior='',
       phasecenter=6,
       ftmachine='ft',
       minpb=0.1,
       scaletype='PBCOR')

feather('orion_tfeather2.im','orion.task.image',datapath+'orion.gbt.im')
#GBT:   Max:5.129806e+01        Flux:2.425065e+02 Jy    rms:1.277546e+01
#VLA:   Max:8.340111e-01        Flux:1.891523e+02 Jy    rms:1.099514e-01
feathersynthtime = time.time()

print '--Single Dish as Model (multi-scale)--'
## Starting from:
##    VLA calibrated visibilities: orion.ms
##    GBT OTF cube: orion.gbt.im
default('clean')
clean('orion.ms',
      'orion_tsdms',
      field='2~10',
      spw=[0,1],
      mode='mfs',
      niter=10000,
      gain=0.2,
      threshold='10.0mJy',
      imagermode='mosaic',
      mosweight=True,
      ftmachine='ft',
      cyclefactor=4,
      cyclespeedup=500,
      multiscale=[0,3,10,30],
      negcomponent=-1,
      mask=datapath+'orion.mask6',
      imsize=[300,300],
      cell=['2.0arcsec','2.0arcsec'],
      phasecenter=6,
      stokes='I',
      weighting='briggs',
      robust=-1.0,
      pbcor=True,      
      minpb=0.1)
sdmodelmstime = time.time()
###combo: Max:1.195286e+00        Flux:2.873779e+02 Jy    rms:9.069330e-02
###GBT:   Max:5.129806e+01        Flux:2.425065e+02 Jy    rms:1.277546e+01
###VLA:   Max:8.340111e-01        Flux:1.891523e+02 Jy    rms:1.099514e-01
##
print '--Single Dish as Model (MEM)--'
### Starting from:
###    VLA calibrated visibilities: orion.ms
###    GBT OTF cube: orion.gbt.im
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
def joint_deconvolve(datapath):
	print '--Joint deconvolution --'

	#Regrid GBT image onto synth imaging coordinates
	ia.open('orion_tsdmem.image')
	csys = ia.coordsys()
	ia.close()
	ia.open(datapath+'orion.gbt.im')
	ia.regrid(outfile='orion_tgbt_regrid.im',shape=[300,300,1,1],
		  csys=csys.torecord(),overwrite=True)
	ia.close()

	#Deconvolve GBT image
	# Sigh.  dc.open will warn about the lack of a PSF, but I can't seem to
	# define a PSF before calling dc.open.
	dc.open('orion_tgbt_regrid.im', psf='')
	#make gaussian for PSF (best guess for GBT beam based on beamsize
	#report in GBT image)
	dc.makegaussian('gbt_gau.im',bmaj='55arcsec',bmin='55arcsec',
			bpa='0deg',normalize=False)
	dc.close()
	dc.open('orion_tgbt_regrid.im',psf='gbt_gau.im')
	dc.setscales(scalemethod='uservector',uservector=[30.,100.,300.])
	dc.clean(algorithm='msclean',model='orion_tjoint3',niter=500,
		 gain=0.4,mask=datapath+'orion.mask6',threshold='0.1Jy')
	dc.close()
	#default('clean')

	im.open('orion.ms')
	im.selectvis(field=[2,3,4,5,6,7,8,9,10],spw=[0,1])
	im.defineimage(nx=300,cellx='2arcsec',phasecenter=6,spw=[0,1])
	im.setvp(dovp=True)
	#im.setscales(scalemethod='uservector',uservector=[0,3,10,30,100])
	im.setscales(scalemethod='uservector',uservector=[0,3,10,30, 70])
	###if clean component for large scale goes negative continue to use
	##that scale
	im.setmfcontrol(stoplargenegatives=-1, cyclefactor=5, cyclespeedup=100)
	im.weight(type='briggs',rmode='norm',robust=-1,mosaic=True)
	im.clean(algorithm='mfmultiscale', model='orion_tjoint3',
		 image='orion_tjoint3.image', gain=0.2, niter=1000,
		 mask=datapath+'orion.mask6')
	im.close()
	return time.time()

jointmemtime = joint_deconvolve(datapath)

endProc = time.clock()
endTime = time.time()

# Regression
import datetime
datestring = datetime.datetime.isoformat(datetime.datetime.today())
outfile = 'orion.' + datestring + '.log'
logfile = open(outfile, 'w')

test_results = {}
for k, fn in (('Feather 1',           'orion_tfeather.im'),
	      ('Feather 2',           'orion_tfeather2.im'),
	      ('SD Model (MS)',       'orion_tsdms.image'),
	      ('SD Model (MEM)',      'orion_tsdmem.image'),
	      ('Joint Deconvolution', 'orion_tjoint3.image')):
	if ia.open(fn):
		test_results[k] = ia.statistics()
		ia.close()
	else:
		print >>logfile, "Could not open", fn, "for reading!"

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

#              Test name          Stat type Expected  Label irregularities 
test_descs = (('Feather 1',           'max',  0.780,  ' '),
	      ('Feather 2',           'max',  0.868,  ' '),
	      ('SD Model (MS)',       'max',  0.935),
	      ('SD Model (MEM)',      'max',  0.87),
	      ('Joint Deconvolution', 'max',  1.03, '', 'Joint Decon1'), # 1.014
	      ('Feather 1',           'flux', 242.506,  ' '),
	      ('Feather 2',           'flux', 242.506,  ' '),
	      ('SD Model (MS)',       'flux', 187, ' ', 'SD Model (MS)', 'Feather 3'),
	      ('SD Model (MEM)',      'flux', 289, '', 'SD Model (MEM)', 'Joint Deconvolution'),
	      ('Joint Deconvolution', 'flux', 362, '', 'Joint Decon2')) # 360.468

def log_test_result(test_results, testdesc, logfile):
	"""Append testdesc to logfile and return whether or not the test was
	successful."""
	result = test_results[testdesc[0]][testdesc[1]][0]
	reldiff = abs(1.0 - result / testdesc[2])
	if reldiff < 0.05:
		print >>logfile, '* Passed',
		retval = True
	else:
		print >>logfile, '! FAILED',
		retval = False

	# RR 4/18/2009: I think this complication might stem from a bug in the
	# original version of the script, but since it is a regression script I
	# am hesitant to change the output.
	if len(testdesc) > 5:
		title1 = testdesc[5]
	else:
		title1 = testdesc[0]
	if len(testdesc) > 4:
		title2 = testdesc[4]
	else:
		title2 = testdesc[0]
	title2 += ':'
	if testdesc[1] == 'max':
		title1 += ' image max'
		title2 += ' Image max'
	else:
		title1 += ' ' + testdesc[1]
		title2 += ' ' + testdesc[1].title()
	if len(testdesc) > 3:
		title2 += testdesc[3]
		
	print >>logfile, title1, 'test'
	print >>logfile, '*--  ' + title2 + str(result) + ',' + str(testdesc[2])
	return retval

regstate = True
for td in test_descs:
	regstate &= log_test_result(test_results, td, logfile)

if regstate:
        print >>logfile, '---'
        print >>logfile, 'Passed Regression test for Orion'
        print >>logfile, '---'
else:
        print >>logfile, '----FAILED Regression test for Orion'
print >>logfile, '*********************************'

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
