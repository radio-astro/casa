################################################################################
#                                                                              #
#           Regression/Benchmarking Script for wideband imaging                #
#                                                                              #
################################################################################
#                                                                              #
# (1) MS-MFS with nterms=3 on VLA_multifrequency_3C286.ms                      #
#     - checks output residual rms and total power                             #
#     - checks output peak intensity, spectral index and spectral curvature    #
#
# (2) Post-deconvolution PB-correction, on the output of (1)
#     - checks output peak corrected intensity, spectral index and curvature.
#                                                                              # 
################################################################################
#                                                                              # 
# More tests that will appear here in the future :                             #
#                                                                              # 
# (2) MS-MFS with wide-band primary-beam correction                            #
# (3) MS-MFS on extended emission : DONE                                             #
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

if(regstate):
   # Test (1) : Run the clean task
   print '--Image with MS-MFS--'
   default('clean')
   npix=1024;
   ret = clean(vis='VLA_multifrequency_3C286.ms',imagename='reg_3C286',imagermode='',
               nterms=3,reffreq='1.4GHz',
               niter=50,gain=0.8,threshold='7.0mJy',imsize=[npix,npix],
               cell=['2.5arcsec','2.5arcsec'],weighting='briggs',usescratch=False);

   # Test (2) : Post-deconvolution PB-correction
   print '--Post deconvolution wideband PB correction--'
   ret = widebandpbcor(vis='VLA_multifrequency_3C286.ms', imagename='reg_3C286',
                       nterms=3, threshold='5mJy', action='pbcor', reffreq='1.4GHz',
                       pbmin=0.2, field='',spwlist=[0, 1, 2, 3, 4, 5, 6], chanlist=[8, 8, 8, 8, 8, 8, 8],
                       weightlist=[1, 1, 1, 1, 1, 1, 1])

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
   # (6) : (V2.5) This is the truth (for active, 19Jan2012) - wrote coefficient residuals to the output residual image, instead of using them only for alpha,beta calculations
   # Changes from previous numbers 'active r17725' are mainly 'noise' levels. 
   correct_sigma = 0.00126019103471;
   correct_sumsq = 1.6652231052;
   correct_intensity = 14.8404045105;
   correct_alpha = -0.471577763557;
   correct_beta = -0.124552100897;
   ## Added on 13 Sep 2012
   correct_pbcor_intensity = 0.234191760421
   correct_pbcor_alpha = -0.910139858723

   # (5) : (V2.3) This is the truth (for active, 02Mar2010) - included coefficient residuals in alpha/beta calcs.
   # Changes from previous numbers 'active r14198' are within the noise, and only for alpha/beta.
   #correct_sigma = 0.00095682840;
   #correct_sumsq = 0.959992192;
   #correct_intensity = 14.84169483;
   #correct_alpha = -0.4716707468;
   #correct_beta = -0.124309256;

   # (4) : (V2.2) This is the truth (for active, 21Jan2010) - removed extra vecpsf0 conv.
   # Changes from previous numbers 'active r13845' are within the noise.
   #correct_sigma = 0.00095682840;
   #correct_sumsq = 0.959992192;
   #correct_intensity = 14.84169483;
   #correct_alpha = -0.471375882;
   #correct_beta = -0.127162337;

   # (3) : This is the truth (for active, 20Dec2010) - change from MTLC to MTMC (+ algorithm fiddling).
   # Changes from previous numbers for 'active r13787' are within the noise.
   #correct_sigma = 0.00095900;
   #correct_sumsq = 0.9644402;
   #correct_intensity = 14.8406848;
   #correct_alpha = -0.47158026;
   #correct_beta = -0.12506663;

   # (2) : This is the truth (for active, 21Oct2010) - with SB's gridding fixes
   #correct_sigma = 0.00099339;
   #correct_sumsq = 1.03476342;
   #correct_intensity = 14.8406724;
   #correct_alpha = -0.4706874;
   #correct_beta = -0.12786445;

   # (1) : This is the truth (for prerelease, 21Oct2010) - without SB's gridding fixes.
   #correct_sigma = 0.0010294;
   #correct_sumsq = 1.11118678;
   #correct_intensity = 14.838494;
   #correct_alpha = -0.47109225;
   #correct_beta = -0.12466369;
   
   # Residual rms noise and sum-sq (total power)
   if(os.path.exists('reg_3C286.residual.tt0')):
      ia.open('reg_3C286.residual.tt0');
      stats = ia.statistics(list=True, verbose=True);
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
      midpix = ia.pixelvalue([npix/2,npix/2])
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
      midpix = ia.pixelvalue([npix/2,npix/2])
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
      midpix = ia.pixelvalue([npix/2,npix/2])
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

   # PB-corrected intensity)
   if(os.path.exists('reg_3C286.pbcor.image.tt0')):
      ia.open('reg_3C286.pbcor.image.tt0');
      offpix = ia.pixelvalue([304,542])
      ia.close();
      diff_int = abs( offpix['value']['value'] - correct_pbcor_intensity )/ abs(correct_pbcor_intensity);
      if(diff_int<0.02): 
         print >>logfile,'* Passed widebandpbcor intensity test ';
      else: 
         print >>logfile,'* FAILED widebandpbcor intensity test '
	 regstate = False;
      print >>logfile,'-- pb-corrected intensity : ' + str(offpix['value']['value']) + ' (' + str(correct_pbcor_intensity) + ')';
   else:
      print >>logfile,'-- FAILED : No pb-corrected intensity map generated';
      regstate = False;

   # PB-corrected alpha (not checking beta)
   if(os.path.exists('reg_3C286.pbcor.image.alpha')):
      ia.open('reg_3C286.pbcor.image.alpha');
      offpix = ia.pixelvalue([304,542])
      ia.close();
      diff_alpha = abs( offpix['value']['value'] - correct_pbcor_alpha )/ abs(correct_pbcor_alpha);
      if(diff_alpha<0.02): 
         print >>logfile,'* Passed widebandpbcor alpha test ';
      else: 
         print >>logfile,'* FAILED widebandpbcor alpha test '
	 regstate = False;
      print >>logfile,'-- pb-corrected spectral index : ' + str(offpix['value']['value']) + ' (' + str(correct_pbcor_alpha) + ')';
   else:
      print >>logfile,'-- FAILED : No pb-corrected spectral index map generated';
      regstate = False;


# Final verdict
if(regstate):
   print >>logfile,'PASSED regression test for wideband-imaging.'
   print ''
   print 'Regression PASSED'
   print ''
else:
   print >>logfile,'FAILED regression test for wideband-imaging.'
   print ''
   print 'Regression FAILED'
   print ''

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


