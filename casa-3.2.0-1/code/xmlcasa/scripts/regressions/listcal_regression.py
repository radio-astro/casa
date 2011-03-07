##########################################################################
# listcal_regression.py
#
# Regression test for task listcal.
#
# Current number of test scenarios: 2
# 
# The testing is performed by comparing the listcal output at runtime
# with "standards" contained in the data repository.  Metadata (all 
# output that is not a floating point number) is compared character by
# character; any difference will cause the test to fail.  Data (all
# floating point numbers in output) are assumed to be sequential 
# amplitude-phase pairs and are required to be equal to within a 
# minimum precision value specified below in this script.  
#
# Data files are built from scratch using functions defined in this script.

import time
import listing as lt
import regression_utility as tstutl

startTime = time.time()

print "BEGIN: listcal_regression.py"

testPassed = 0
testFailed = 0
pathName = os.environ.get('CASAPATH').split()[0] 
localData = pathName + '/data/regression/ngc4826/'
automate = true # set to false for testing or debugging
regressionDir = 'listcal_regression'
if (not os.path.exists(regressionDir)): os.mkdir(regressionDir)

if(automate): 
    print "Running in automated mode."
    print "  - All MS data will be rebuilt from scratch."
    print "  - All test files will be removed after testing."
else:
    print "Running in non-automated mode!"

# For testing:
# casalog.filter('DEBUG2')

#=============================================================================
# METHOD: load_ngc4826
#
# Load, edit, and calibrate NGC4826 data.
#
def load_ngc4826(prefix,msname,caltable):
    # Clear out previous run results
    os.system('rm -rf '+prefix+'ngc4826.tutorial.*')
    ##########################################################################
    # Import and concatenate sources
    # Data Description:
    # N4826 - BIMA SONG Data                                                 #
    # This data is from the BIMA Survey of Nearby Galaxies (BIMA SONG)       #
    # Helfer, Thornley, Regan, et al., 2003, ApJS, 145, 259                  #
    # 16apr98
    #	source=ngc4826
    #	phasecal=1310+323
    #	fluxcal=3c273, Flux = 23 Jy on 16apr98
    #	passcal= none - data were observed with online bandpass correction.
    # NOTE: This data has been filled into MIRIAD, line-length correction 
    #	done, and then exported as separate files for each source.
    #	3c273 was not line length corrected since it was observed
    #	for such a short amount of time that it did not need it.  
    # From miriad: source Vlsr = 408; delta V is 20 km/s 
    # NOTE: This data contains correlations of only one polarization, 'YY'.
    # The antennas contain 'X' and 'Y' feeds, but the 'X' data was not 
    # correlated.
    ##########################################################################
    # [ This section derived from the 2008 summer school tutorial. ]
    # [ http://casa.nrao.edu/Tutorial/SIworkshop2008/Scripts/ngc4826_tutorial.py ]
    # USB spectral windows written separately by miriad for 16apr98
    # Assumes these are in sub-directory called "fitsfiles" of working directory
    print '--Importuvfits (16apr98)--'
    default('importuvfits')
    print "Starting from the uvfits files exported by miriad"
    print "The USB spectral windows were written separately by miriad for 16apr98"
    pathName = os.environ.get('CASAPATH').split()[0] 
    localData = pathName + '/data/regression/ngc4826/'
    importuvfits(fitsfile= localData + 'fitsfiles/3c273.fits5',        vis=prefix+'ngc4826.tutorial.3c273.5.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/3c273.fits6',        vis=prefix+'ngc4826.tutorial.3c273.6.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/3c273.fits7',        vis=prefix+'ngc4826.tutorial.3c273.7.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/3c273.fits8',        vis=prefix+'ngc4826.tutorial.3c273.8.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/1310+323.ll.fits9',  vis=prefix+'ngc4826.tutorial.1310+323.ll.9.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/1310+323.ll.fits10', vis=prefix+'ngc4826.tutorial.1310+323.ll.10.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/1310+323.ll.fits11', vis=prefix+'ngc4826.tutorial.1310+323.ll.11.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/1310+323.ll.fits12', vis=prefix+'ngc4826.tutorial.1310+323.ll.12.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/1310+323.ll.fits13', vis=prefix+'ngc4826.tutorial.1310+323.ll.13.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/1310+323.ll.fits14', vis=prefix+'ngc4826.tutorial.1310+323.ll.14.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/1310+323.ll.fits15', vis=prefix+'ngc4826.tutorial.1310+323.ll.15.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/1310+323.ll.fits16', vis=prefix+'ngc4826.tutorial.1310+323.ll.16.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/ngc4826.ll.fits5',   vis=prefix+'ngc4826.tutorial.ngc4826.ll.5.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/ngc4826.ll.fits6',   vis=prefix+'ngc4826.tutorial.ngc4826.ll.6.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/ngc4826.ll.fits7',   vis=prefix+'ngc4826.tutorial.ngc4826.ll.7.ms')
    importuvfits(fitsfile= localData + 'fitsfiles/ngc4826.ll.fits8',   vis=prefix+'ngc4826.tutorial.ngc4826.ll.8.ms')
    ##########################################################################
    print '--Concat--'
    default('concat')
    concat(vis=[prefix+'ngc4826.tutorial.3c273.5.ms',
                prefix+'ngc4826.tutorial.3c273.6.ms',
                prefix+'ngc4826.tutorial.3c273.7.ms',
                prefix+'ngc4826.tutorial.3c273.8.ms',
                prefix+'ngc4826.tutorial.1310+323.ll.9.ms',
                prefix+'ngc4826.tutorial.1310+323.ll.10.ms',
                prefix+'ngc4826.tutorial.1310+323.ll.11.ms',
                prefix+'ngc4826.tutorial.1310+323.ll.12.ms',
                prefix+'ngc4826.tutorial.1310+323.ll.13.ms',
                prefix+'ngc4826.tutorial.1310+323.ll.14.ms',
                prefix+'ngc4826.tutorial.1310+323.ll.15.ms',
                prefix+'ngc4826.tutorial.1310+323.ll.16.ms',
                prefix+'ngc4826.tutorial.ngc4826.ll.5.ms',
                prefix+'ngc4826.tutorial.ngc4826.ll.6.ms',
                prefix+'ngc4826.tutorial.ngc4826.ll.7.ms',
                prefix+'ngc4826.tutorial.ngc4826.ll.8.ms'],
           concatvis=msname,
           freqtol="",dirtol="1arcsec",async=False)
    ##########################################################################
    # Fix up the MS (temporary, changes to importfits underway)
    print '--Fixing up spw rest frequencies in MS--'
    vis=msname
    tb.open(vis+'/SOURCE',nomodify=false)
    spwid=tb.getcol('SPECTRAL_WINDOW_ID')
    #spwid.setfield(-1,int)
    # 64bit imported from ngc4826_tutorial_regression
    spwid.setfield(-1,'int32')
    tb.putcol('SPECTRAL_WINDOW_ID',spwid)
    tb.close()
    # This ensures that the rest freq will be found for all spws. 
    ##########################################################################
    # 16 APR Calibration
    ##########################################################################
    print '--Clearcal--'
    print 'Create scratch columns and initialize in '+msname
    # Force create/initialize of scratch columns
    # NOTE: plotxy will not run properly without this step.
    clearcal(vis=msname)
    # But this data is relatively clean, and flagging will not improve results.
    ##########################################################################
    # Flag end channels
    print '--Flagdata--'
    default('flagdata')
    print ""
    print "Flagging edge channels in all spw"
    print "  0~3:0~1;62~63 , 4~11:0~1;30~31, 12~15:0~1;62~63 "
    print ""
    flagdata(vis=msname, mode='manualflag',
             spw='0~3:0;1;62;63,4~11:0;1;30;31,12~15:0;1;62;63')
    # Flag correlator glitch
    print ""
    print "Flagging bad correlator field 8 antenna 3&9 spw 15 all channels"
    print "  timerange 1998/04/16/06:19:00.0~1998/04/16/06:20:00.0"
    print ""
    flagdata(vis=msname, mode='manualflag', field='8', spw='15', antenna='3&9', timerange='1998/04/16/06:19:00.0~1998/04/16/06:20:00.0')
    print "Completed pre-calibration flagging"
    ##########################################################################
    # Use Flagmanager to save a copy of the flags so far
    print '--Flagmanager--'
    default('flagmanager')
    print "Now will use flagmanager to save a copy of the flags we just made"
    print "These are named myflags"
    flagmanager(vis=msname,mode='save',versionname='myflags',
                comment='My flags',merge='replace')
    ##########################################################################
    # CALIBRATION
    ##########################################################################
    # Bandpasses are very flat because of observing mode used (online bandpass
    # correction) so bandpass calibration is unecessary for these data.
    ##########################################################################
    # Derive gain calibration solutions.
    # We will use VLA-like G (per-scan) calibration:
    ##########################################################################
    # Set the flux density of 3C273 to 23 Jy
    print '--Setjy (3C273)--'
    default('setjy')
    setjy(vis=msname,field='0',fluxdensity=[23.0,0.,0.,0.],spw='0~3')
    # Not really necessary to set spw but you get lots of warning messages if
    # you don't
    ##########################################################################
    # Gain calibration
    print '--Gaincal--'
    default('gaincal')
    # This should be combining all spw for the two calibrators for single
    # scan-based solutions
    print 'Gain calibration for fields 0,1 and spw 0~11'
    print 'Using solint=inf combining over spw'
    print 'Output table ngc4826.tutorial.16apr98.gcal'
    gaincal(vis=msname, caltable=caltable,
        field='0,1', spw='0~11', gaintype='G', minsnr=2.0,
        refant='ANT5', gaincurve=False, opacity=0.0,
        solint='inf', combine='spw')
#=============================================================================

#=============================================================================
#
# The use of this calibration function has not yet been implemented.
# It can be used to generate and MS and calibration tables to test listcal.
#
def load_jupiter6cm(prefix,msname,caltable):
    ######################################################################
    # Use Case Script for Jupiter 6cm VLA                                #
    # Trimmed down from Use Case jupiter6cm_usecase.py                   #
    # Updated STM 2008-05-15 (Beta Patch 2.0)                            #
    # Updated STM 2008-06-11 (Beta Patch 2.0)                            #
    # Updated STM 2008-06-12 (Beta Patch 2.0) for summer school demo     #
    # This is a VLA 6cm dataset that was observed in 1999 to set the     #
    # flux scale for calibration of the VLA.  Included in the program    #
    # were observations of the planets, including Jupiter.               #
    # This is D-configuration data, with resolution of around 14"        #
    # Includes polarization imaging and analysis                         #
    ######################################################################
    import time
    import os
    #=====================================================================
    # This script has some interactive commands: scriptmode = True
    # if you are running it and want it to stop during interactive parts.
    scriptmode = False
##     #=====================================================================
##     # Set up some useful variables - these will be set during the script
##     # also, but if you want to restart the script in the middle here
##     # they are in one place:
##     
##     # This will prefix all output file names
##     prefix='jupiter6cm.demo'
##     
##     # Clean up old files
##     os.system('rm -rf '+prefix+'*')
##     
##     # This is the output MS file name
##     msfile = prefix + '.ms'
##     
##     #
    #=====================================================================
    # Calibration variables
    #
    # Use same prefix as rest of script
    calprefix = prefix
    
    # spectral windows to process
    usespw = ''
    usespwlist = ['0','1']
    
    # prior calibration to apply
    usegaincurve = True
    gainopacity = 0.0
    
    # reference antenna 11 (11=VLA:N1)
    calrefant = '11'
    
    gtable = calprefix + '.gcal'
    ftable = calprefix + '.fluxscale'
    atable = calprefix + '.accum'
    
    #
    #=====================================================================
    # Polarization calibration setup
    #
    dopolcal = True
    
    ptable = calprefix + '.pcal'
    xtable = calprefix + '.polx'
    
    # Pol leakage calibrator
    poldfield = '0137+331'
    
    # Pol angle calibrator
    polxfield = '1331+305'
    # At Cband the fractional polarization of this source is 0.112 and
    # the R-L PhaseDiff = 66deg (EVPA = 33deg)
    polxfpol = 0.112
    polxrlpd_deg = 66.0
    # Dictionary of IPOL in the spw
    polxipol = {'0' : 7.462,
                '1' : 7.510}
    
    # Make Stokes lists for setjy
    polxiquv = {}
    for spw in ['0','1']:
        ipol = polxipol[spw]
        fpol = polxfpol
        ppol = ipol*fpol
        rlpd = polxrlpd_deg*pi/180.0
        qpol = ppol*cos(rlpd)
        upol = ppol*sin(rlpd)
        polxiquv[spw] = [ipol,qpol,upol,0.0]
    
    #
    # Split output setup
    #
    srcname = 'JUPITER'
    srcsplitms = calprefix + '.' + srcname + '.split.ms'
    calname = '0137+331'
    calsplitms = calprefix + '.' + calname + '.split.ms'
    
    #
    #=====================================================================
    #
    # Intensity imaging parameters
    #
    # Same prefix for this imaging demo output
    #
    imprefix = prefix
    
    # This is D-config VLA 6cm (4.85GHz) obs
    # Check the observational status summary
    # Primary beam FWHM = 45'/f_GHz = 557"
    # Synthesized beam FWHM = 14"
    # RMS in 10min (600s) = 0.06 mJy (thats now, but close enough)
    
    # Set the output image size and cell size (arcsec)
    # 4" will give 3.5x oversampling
    clncell = [4.,4.]
    
    # 280 pix will cover to 2xPrimaryBeam
    # clean will say to use 288 (a composite integer) for efficiency
    clnalg = 'clark'
    clnmode = ''
    # For Cotton-Schwab use
    clnmode = 'csclean'
    clnimsize = [288,288]
    
    # iterations
    clniter = 10000
    
    # Also set flux residual threshold (0.04 mJy)
    # From our listobs:
    # Total integration time = 85133.2 seconds
    # With rms of 0.06 mJy in 600s ==> rms = 0.005 mJy
    # Set to 10x thermal rms
    clnthreshold=0.05
    
    #
    # Filenames
    #
    imname1 = imprefix + '.clean1'
    clnimage1 = imname1+'.image'
    clnmodel1 = imname1+'.model'
    clnresid1 = imname1+'.residual'
    clnmask1  = imname1+'.clean_interactive.mask'
    
    imname2 = imprefix + '.clean2'
    clnimage2 = imname2+'.image'
    clnmodel2 = imname2+'.model'
    clnresid2 = imname2+'.residual'
    clnmask2  = imname2+'.clean_interactive.mask'
    
    imname3 = imprefix + '.clean3'
    clnimage3 = imname3+'.image'
    clnmodel3 = imname3+'.model'
    clnresid3 = imname3+'.residual'
    clnmask3  = imname3+'.clean_interactive.mask'
    
    #
    # Selfcal parameters
    #
    # reference antenna 11 (11=VLA:N1)
    calrefant = '11'
    
    #
    # Filenames
    #
    selfcaltab1 = imprefix + '.selfcal1.gtable'
    
    selfcaltab2 = imprefix + '.selfcal2.gtable'
    smoothcaltab2 = imprefix + '.smoothcal2.gtable'
    
    #
    #=====================================================================
    #
    # Polarization imaging parameters
    #
    # New prefix for polarization imaging output
    #
    polprefix = prefix + '.polimg'
    
    # Set up clean slightly differently
    polclnalg = 'hogbom'
    polclnmode = 'csclean'
    
    polimname = polprefix + '.clean'
    polimage  = polimname+'.image'
    polmodel  = polimname+'.model'
    polresid  = polimname+'.residual'
    polmask   = polimname+'.clean_interactive.mask'
    
    #
    # Other files
    #
    ipolimage = polimage+'.I'
    qpolimage = polimage+'.Q'
    upolimage = polimage+'.U'
    
    poliimage = polimage+'.poli'
    polaimage = polimage+'.pola'
    
    #
    #=====================================================================
    # Start processing
    #=====================================================================
    # Get to path to the CASA home and stip off the name
    pathname=os.environ.get('CASAPATH').split()[0]
    # This is where the UVFITS data should be
    fitsdata=pathname+'/data/regression/jupiter6cm/jupiter6cm.fits'
    #=====================================================================
    # Data Import and List
    #=====================================================================
    # Import the data from FITS to MS
    print '--Import--'
    # Safest to start from task defaults
    default('importuvfits')
    print "Use importuvfits to read UVFITS and make an MS"
    # Set up the MS filename and save as new global variable
    msfile = prefix + '.ms'
    print "MS will be called "+msfile
    # Use task importuvfits
    fitsfile = fitsdata
    vis = msfile
    importuvfits()
    #=====================================================================
    # Data Examination and Flagging
    # REMOVED: All flagging was interactive. Could be replaced with 
    # automatic flagging.
    #=====================================================================
    # Calibration
    #=====================================================================
    # Set the fluxes of the primary calibrator(s)
    #
    print '--Setjy--'
    default('setjy')
    
    print "Use setjy to set flux of 1331+305 (3C286)"
    
    vis = msfile
    
    #
    # 1331+305 = 3C286 is our primary calibrator
    field = '1331+305'     
    
    # Setjy knows about this source so we dont need anything more
    
    setjy()
    
    #
    # You should see something like this in the logger and casapy.log file:
    #
    # 1331+305  spwid=  0  [I=7.462, Q=0, U=0, V=0] Jy, (Perley-Taylor 99)
    # 1331+305  spwid=  1  [I=7.51, Q=0, U=0, V=0] Jy, (Perley-Taylor 99)
    # 
    
    print "Look in logger for the fluxes (should be 7.462 and 7.510 Jy)"
    
    #
    #=====================================================================
    #
    # Initial gain calibration
    #
    print '--Gaincal--'
    default('gaincal')
    
    print "Solve for antenna gains on 1331+305 and 0137+331"
    print "We have 2 single-channel continuum spw"
    print "Do not want bandpass calibration"
    
    vis = msfile
    
    # set the name for the output gain caltable
    caltable = gtable
    
    print "Output gain cal table will be "+gtable
    
    # Gain calibrators are 1331+305 and 0137+331 (FIELD_ID 7 and 0)
    # We have 2 IFs (SPW 0,1) with one channel each
    
    # selection is via the field and spw strings
    field = '1331+305,0137+331'
    spw = ''
    
    # a-priori calibration application
    gaincurve = usegaincurve
    opacity = gainopacity
    
    # scan-based G solutions for both amplitude and phase
    gaintype = 'G'
    calmode = 'ap'
    
    # one solution per scan
    solint = 'inf'
    combine = ''
    
    # do not apply parallactic angle correction (yet)
    parang = False
    
    # reference antenna
    refant = calrefant
    
    # minimum SNR 3
    minsnr = 3
    
    gaincal()
    
    #
    #=====================================================================
    #
    # Bootstrap flux scale
    #
    print '--Fluxscale--'
    default('fluxscale')
    
    print "Use fluxscale to rescale gain table to make new one"
    
    vis = msfile
    
    # set the name for the output rescaled caltable
    fluxtable = ftable
    
    print "Output scaled gain cal table is "+ftable
    
    # point to our first gain cal table
    caltable = gtable
    
    # we will be using 1331+305 (the source we did setjy on) as
    # our flux standard reference
    reference = '1331+305'
    
    # we want to transfer the flux to our other gain cal source 0137+331
    # to bring its gain amplitues in line with the absolute scale
    transfer = '0137+331'
    
    fluxscale()
    
    # You should see in the logger something like:
    #Flux density for 0137+331 in SpW=0 is:
    #   5.42575 +/- 0.00285011 (SNR = 1903.7, nAnt= 27)
    #Flux density for 0137+331 in SpW=1 is:
    #   5.46569 +/- 0.00301326 (SNR = 1813.88, nAnt= 27)
    
    #
    #---------------------------------------------------------------------
    # Plot calibration
    #
    print '--PlotCal--'
    default('plotcal')
    
    showgui = True
        
    caltable = ftable
    multiplot = True
    yaxis = 'amp'
    
    showgui = True
        
    plotcal()
    
    print ""
    print "-------------------------------------------------"
    print "Plotcal"
    print "Looking at amplitude in cal-table "+caltable
    
    # Pause script if you are running in scriptmode
    if scriptmode:
        user_check=raw_input('Return to continue script\n')
    
    #
    # Now go back and plot to file
    #
    showgui = False
    
    yaxis = 'amp'
    
    figfile = caltable + '.plotcal.amp.png'
    print "Plotting calibration to file "+figfile
    #saveinputs('plotcal',caltable.plotcal.amp.saved')
    plotcal()
    
    yaxis = 'phase'
    
    figfile = caltable + '.plotcal.phase.png'
    print "Plotting calibration to file "+figfile
    #saveinputs('plotcal',caltable.plotcal.phase.saved')
    plotcal()
    
    #
    #=====================================================================
    # Polarization Calibration
    #=====================================================================
    #
    if (dopolcal):
        print '--Polcal (D)--'
        default('polcal')
        
        print "Solve for polarization leakage on 0137+331"
        print "Pretend it has unknown polarization"
    
        vis = msfile
    
        # Start with the un-fluxscaled gain table
        gaintable = gtable
    
        # use settings from gaincal
        gaincurve = usegaincurve
        opacity = gainopacity
        
        # Output table
        caltable = ptable
    
        # Use a 3C48 tracked through a range of PA
        field = '0137+331'
        spw = ''
    
        # No need for further selection
        selectdata=False
    
        # Polcal mode (D+QU = unknown pol for D)
        poltype = 'D+QU'
    
        # One solution for entire dataset
        solint = 'inf'
        combine = 'scan'
    
        # reference antenna
        refant = calrefant
    
        # minimum SNR 3
        minsnr = 3
    
        #saveinputs('polcal',calprefix+'.polcal.saved')
        polcal()
        
        #=====================================================================
        #
        # List polcal solutions
        #
        print '--Listcal (PolD)--'
    
        listfile = caltable + '.list'
    
        print "Listing calibration to file "+listfile
    
        listcal()
        
        #=====================================================================
        #
        # Plot polcal solutions
        #
        print '--Plotcal (PolD)--'
        
        iteration = ''
        showgui = False
        
        xaxis = 'real'
        yaxis = 'imag'
        figfile = caltable + '.plotcal.reim.png'
        print "Plotting calibration to file "+figfile
        #saveinputs('plotcal',caltable+'.plotcal.reim.saved')
        plotcal()
    
        xaxis = 'antenna'
        yaxis = 'amp'
        figfile = caltable + '.plotcal.antamp.png'
        print "Plotting calibration to file "+figfile
        #saveinputs('plotcal',caltable+'.plotcal.antamp.saved')
        plotcal()
    
        xaxis = 'antenna'
        yaxis = 'phase'
        figfile = caltable + '.plotcal.antphase.png'
        print "Plotting calibration to file "+figfile
        #saveinputs('plotcal',caltable+'.plotcal.antphase.saved')
        plotcal()
    
        xaxis = 'antenna'
        yaxis = 'snr'
        figfile = caltable + '.plotcal.antsnr.png'
        print "Plotting calibration to file "+figfile
        #saveinputs('plotcal',caltable+'.plotcal.antsnr.saved')
        plotcal()
    
        #=====================================================================
        # Do Chi (X) pol angle calibration
        #=====================================================================
        # First set the model
        print '--Setjy--'
        default('setjy')
            
        vis = msfile
            
        print "Use setjy to set IQU fluxes of "+polxfield
        field = polxfield
        
        for spw in usespwlist:
            fluxdensity = polxiquv[spw]
            
            #saveinputs('setjy',calprefix+'.setjy.polspw.'+spw+'.saved')
            setjy()
        
        #
        # Polarization (X-term) calibration
        #
        print '--PolCal (X)--'
        default('polcal')
        
        print "Polarization R-L Phase Calibration (linear approx)"
        
        vis = msfile
        
        # Start with the G and D tables
        gaintable = [gtable,ptable]
        
        # use settings from gaincal
        gaincurve = usegaincurve
        opacity = gainopacity
        
        # Output table
        caltable = xtable
    
        # previously set with setjy
        field = polxfield
        spw = ''
        
        selectdata=False
        
        # Solve for Chi
        poltype = 'X'
        solint = 'inf'
        combine = 'scan'
        
        # reference antenna
        refant = calrefant
        
        # minimum SNR 3
        minsnr = 3
        
        #saveinputs('polcal',calprefix+'.polcal.X.saved')
        polcal()
#=============================================================================

# # # # TESTS BEGIN HERE # # # # # # # # # # # # # # # # # # # # # # # # # # #

testNum = 0

##########################################################################                                                                        
# TEST1
# Using ngc4826 data.  This data contains multiple spectral windows with
# variable numbers of channels; only one correlation, YY.
#

testNum += 1
print ""
print "* TEST " + str(testNum) + ": default test; using ngc4826 tutorial data and calibration"
print """
- This data has multiple spws with variable numbers of channels.
- This data has only one correlation, YY
> Using default input values where possible
"""

prefix = regressionDir+'/test'+str(testNum)+'/'
msname = prefix+"ngc4826.tutorial.ms"  
caltableName = prefix+"ngc4826.tutorial.16apr98.gcal"
outputFilename = prefix+'listcal.ngc4826.default.out'
standardFileName = localData + 'listcal.default.out'

# Use existing data or load data from scratch?
if (not lt.resetData([msname,caltableName], automate)):
    print "Using preexisting data." 
    lt.removeOut(outputFilename)
else:
    print "Building data from scratch."
    tstutl.maketestdir(prefix) # create test dir, overwrite preexisting
    load_ngc4826(prefix,msname,caltableName) # Build data from scratch

# Setup listcal input and run
default(listcal)
vis                 =     msname        #  Name of input visibility file
caltable            = caltableName      #  Input calibration table to list
field               =         ''        #  Field name or index; ''==>all
antenna             =         ''        #  Antenna name or index; ''==>all; antenna='3'
spw                 =         ''        #  Spectral window and channel: ''==>all;
                                        #   spw='5:0~10'
listfile            = outputFilename    #  Disk file to write output: ''==>to terminal
pagerows            =         50        #  Rows per page
async               =      False        #  If true the taskname must be started using
go(listcal)

# Remove first line of listcal output (contains hard-coded path)
lt.listcalFix(outputFilename)

compareFilename = prefix + 'compare'
if (lt.runTests(outputFilename,standardFileName,'1.000',compareFilename)): 
    print "Passed listcal output test"
    testPassed +=1
else:       
    print "FAILED listcal output test"
    testFailed +=1

##########################################################################                                                                        
# TEST - NGC4826 
# Using ngc4826 data from above.
#
testNum += 1
print ""
print "* TEST " + str(testNum) + ": using ngc4826 tutorial data and calibration."
print """
- Using same data as above
> Using all non-default values where possible

"""

prefix = regressionDir+'/test'+str(testNum)+'/'
tstutl.maketestdir(prefix)
outputFilename = prefix+'listcal.ngc4826.nondefault.out'
standardFileName = localData + 'listcal.nondefault.out'
compareFilename = prefix + 'compare'

default(listcal)
vis                 =     msname        #  Name of input visibility file
caltable            = caltableName      #  Input calibration table to list
field               = '1310+323'        #  Field name or index; ''==>all
antenna             =   '3~5,10'        #  Antenna name or index; ''==>all; antenna='3'
spw                 =        '0'        #  Spectral window and channel: ''==>all; spw='5:0~10'
listfile            = outputFilename    #  Disk file to write output: ''==>to terminal
pagerows            =          9        #  Rows per page
async               =      False        #  If true the taskname must be started using
go(listcal)

# Remove first line of listcal output (contains hard-coded path)
lt.listcalFix(outputFilename)

if (lt.runTests(outputFilename,standardFileName,'1.000',compareFilename)): 
    print "Passed listcal output test"
    testPassed +=1
else:       
    print "FAILED listcal output test"
    testFailed +=1

##########################################################################                                                                        
# Test complete, summarize.
#

print ""
print "* listcal regression test complete"
print "SUMMARY:"
print "  number of tests PASSED: " + str(testPassed)
print "  number of tests FAILED: " + str(testFailed)
print ""

# # If running in automated mode, remove all data files.
# if (automate): 
#     print "Removing all listcal_regression files..."
#     os.system('rm -f '+regressionDir) # remove all listcal output

print "END: listcal_regression.py"

if (testFailed > 0):   
    regstate=False
else:
    regstate=True
    
endTime = time.time()
