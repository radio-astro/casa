################################################################################
#                                                                              #
#           Regression/Benchmarking Script for wideband imaging                #
#                                                                              #
################################################################################
#                                                                              #
# (1) MS-MFS with nterms=3 on VLA_multifrequency_3C286.ms                      #
#     - checks output residual rms and total power                             #
#     - checks output peak intensity, spectral index and spectral curvature    #
#                                                                              # 
################################################################################
#                                                                              # 
# More tests that will appear here in the future :                             #
#                                                                              # 
# (2) MS-MFS on extended emission                                              #
# (3) MS-MFS with wide-band primary-beam correction                            #
# (4) MS-MFS with mosaicing                                                    #
#                                                                              #
################################################################################

import time
import os

# Data : VLA_multifrequency_3C286.ms
pathname=os.environ.get('CASAPATH').split()[0]

# Initialize status flag
regstate = True;

# Start timers
startTime=time.time()
startProc=time.clock()

# Mark time
copyTime=time.time()

# Test (1) : Run the clean task
if(regstate):
   print '--Image with MS-MFS--'
   default('clean')
   ret = clean(vis='VLA_multifrequency_3C286.ms',imagename='reg_3C286',nterms=3,reffreq='1.4GHz',
               niter=50,gain=0.8,threshold='7.0mJy',imsize=[1024,1024],
               cell=['2.5arcsec','2.5arcsec'],weighting='briggs',calready=False);

# Stop timers
endProc=time.clock()
endTime=time.time()

# Start printing info.
import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())
outfile='reg_3C286.'+datestring+'.log'
logfile=open(outfile,'w')

# Data summary
print >>logfile,'**************** Regression-test for wide-band imaging *************************'
print >>logfile,'**                                                                            **'
print >>logfile,'******************  3C286 wideband data (L-Band) *******************************'
print >>logfile,''
print >>logfile,'Observation: VLA'
print >>logfile,'Data records: 145600       Total integration time = 35435 seconds'
print >>logfile,'   Observed from   28-Apr-2008/00:53:22.5   to   28-Apr-2008/10:43:57.5 (UTC)'
print >>logfile,'Fields: 1'
print >>logfile,'  ID   Code Name         RA            Decl           Epoch   SrcId nVis   '
print >>logfile,'  0    A    1331+305     13:31:08.2879 +30.30.32.9580 J2000   0     145600 '
print >>logfile,'   (nVis = Total number of time/baseline visibilities per field) '
print >>logfile,'Spectral Windows:  (7 unique spectral windows and 1 unique polarization setups)'
print >>logfile,'  SpwID  #Chans Frame Ch1(MHz)    ChanWid(kHz)TotBW(kHz)  Ref(MHz)    Corrs   '
print >>logfile,'  0          15 TOPO  1184.0625   1562.5      23437.5     1195        RR  LL  '
print >>logfile,'  1          15 TOPO  1301.0625   1562.5      23437.5     1312        RR  LL  '
print >>logfile,'  2          15 TOPO  1401.0625   1562.5      23437.5     1412        RR  LL  '
print >>logfile,'  3          15 TOPO  1494.0625   1562.5      23437.5     1505        RR  LL  '
print >>logfile,'  4          15 TOPO  1676.0625   1562.5      23437.5     1687        RR  LL  '
print >>logfile,'  5          15 TOPO  1751.0625   1562.5      23437.5     1762        RR  LL  '
print >>logfile,'  6          15 TOPO  1864.0625   1562.5      23437.5     1875        RR  LL  '


# Perform the checks
print >>logfile,''
print >>logfile,'*********************** Comparison of results **********************************'
print >>logfile,'**                                                                            **'
print >>logfile,'**      (1) MS-MFS on the 3C286 field with nterms=3 and reffreq=1.4GHz        **'
print >>logfile,'**                                                                            **'
print >>logfile,'********************************************************************************'

if(not regstate):
   print >>logfile,'* Data file VLA_multifrequency_3C286.ms cannot be found';
else:
   # This is the truth (for active, 20 Oct 2010)
   #correct_sigma = 0.00099339;
   #correct_sumsq = 1.03476342;
   #correct_intensity = 14.8406724;
   #correct_alpha = -0.47218161;
   #correct_beta = -0.1136824;

   # This is the truth (for prerelease, 20 Oct 2010)
   correct_sigma = 0.0010294;
   correct_sumsq = 1.11118678;
   correct_intensity = 14.838494;
   correct_alpha = -0.4712090;
   correct_beta = -0.12474468;
   
   # Residual rms noise and sum-sq (total power)
   if(os.path.exists('reg_3C286.residual.tt0')):
      ia.open('reg_3C286.residual.tt0');
      stats = ia.statistics();
      ia.close();
      diff_sigma = abs( (stats['sigma'][0]) - correct_sigma )/correct_sigma;
      diff_sumsq = abs( (stats['sumsq'][0]) - correct_sumsq )/correct_sumsq;
      if(diff_sigma<0.05):
         print >>logfile,'* Passed residual sigma test ';
      else: 
         print >>logfile,'* FAILED residual sigma test '
	 regstate = False;
      print >>logfile,'-- residual sigma : ' + str((stats['sigma'][0])) + ' (' + str(correct_sigma) + ')';
      if(diff_sumsq<0.05): 
         print >>logfile,'* Passed residual total-power test ';
      else: 
         print >>logfile,'* FAILED residual total-power test '
	 regstate = False
      print >>logfile,'-- residual sumsq : ' + str((stats['sumsq'][0])) + ' (' + str(correct_sumsq) + ')';
   else:
      print >>logfile,' FAILED : No residual image generated.'
      regstate = False;
   
   # Intensity
   if(os.path.exists('reg_3C286.image.tt0')):
      ia.open('reg_3C286.image.tt0');
      midpix = ia.pixelvalue([512,512])
      ia.close();
      diff_intensity = abs( midpix['value']['value'] - correct_intensity )/ abs(correct_intensity);
      if(diff_intensity<0.02): 
         print >>logfile,'* Passed peak intensity test ';
      else: 
         print >>logfile,'* FAILED peak intensity test '
	 regstate = False;
      print >>logfile,'-- peak intensity : ' + str(midpix['value']['value']) + ' (' + str(correct_intensity) + ')';
   else:
      print >>logfile,'-- FAILED : No intensity map generated';
      regstate = False;

   # Alpha
   if(os.path.exists('reg_3C286.image.alpha')):
      ia.open('reg_3C286.image.alpha');
      midpix = ia.pixelvalue([512,512])
      ia.close();
      diff_alpha = abs( midpix['value']['value'] - correct_alpha )/ abs(correct_alpha);
      if(diff_alpha<0.02): 
         print >>logfile,'* Passed spectral index test ';
      else: 
         print >>logfile,'* FAILED spectral index test '
	 regstate = False;
      print >>logfile,'-- spectral index : ' + str(midpix['value']['value']) + ' (' + str(correct_alpha) + ')';
   else:
      print >>logfile,'-- FAILED : No spectral index map generated';
      regstate = False;

   # Beta
   if(os.path.exists('reg_3C286.image.beta')):
      ia.open('reg_3C286.image.beta');
      midpix = ia.pixelvalue([512,512])
      ia.close();
      diff_beta = abs( midpix['value']['value'] - correct_beta )/ abs(correct_beta);
      if(diff_beta<0.02): 
         print >>logfile,'* Passed spectral curvature test ';
      else: 
         print >>logfile,'* FAILED spectral curvature test '
	 regstate = False;
      print >>logfile,'-- spectral curvature : ' + str(midpix['value']['value']) + ' (' + str(correct_beta) + ')';
   else:
      print >>logfile,'-- FAILED : No spectral curvature map generated';
      regstate = False;

# Final verdict
if(regstate):
   print >>logfile,'PASSED regression test for wideband-imaging.'
else:
   print >>logfile,'FAILED regression test for wideband-imaging.'
print >>logfile,''

# Print timing info
print >>logfile,'********************************************************************************'
print >>logfile,'**                         Benchmarking                                       **'
print >>logfile,'********************************************************************************'
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,'Processing rate MB/s  was: '+str(278./(endTime - startTime))
print >>logfile,'* Breakdown:                                                                   *'
print >>logfile,'*   copy         time was: '+str(copyTime-startTime)
print >>logfile,'*   imaging      time was: '+str(endTime-copyTime)
print >>logfile,'*                                                                              *'
print >>logfile,'********************************************************************************'

logfile.close()


