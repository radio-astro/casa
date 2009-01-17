##########################################################################
# listvis_regression.py
#
# Regression test for task listvis.
#
# Current number of test scenarios: 4
# 
# The testing is performed by comparing the listvis output at runtime
# with "standards" contained in the data repository.  "Metadata" (all 
# output that is not a floating point number) is compared character by
# character; any difference will cause the test to fail.  "Data" (all
# floating point numbers in output) are assumed to be sequential 
# amplitude-phase pairs and are required to be equal to within a 
# minimum precision value specified below in this script.  
#
# Measurement sets are built from scratch using functions defined in this 
# script.

import time
import listing as lt
import regression_utility as tstutl

startTime = time.time()

print "BEGIN: listvis_regression.py"

testPassed = 0
testFailed = 0
pathName = os.environ.get('CASAPATH').split()[0]
automate = True # set to false for testing or debugging
regressionDir = 'listvis_regression'
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
def load_ngc4826(prefix,msname):
    ##########################################################################
    #                                                                        
    # Clear out previous run results
    print "Removing files and directories: ngc4826.tutorial.*"
    os.system('rm -rf '+prefix+'ngc4826.tutorial.*')
    ##########################################################################
    # Import and concatenate sources
    #
    # Data Description:
    #
    # N4826 - BIMA SONG Data                                                 #
    #                                                                        #
    # This data is from the BIMA Survey of Nearby Galaxies (BIMA SONG)       #
    # Helfer, Thornley, Regan, et al., 2003, ApJS, 145, 259                  #
    #
    # 16apr98
    #	source=ngc4826
    #	phasecal=1310+323
    #	fluxcal=3c273, Flux = 23 Jy on 16apr98
    #	passcal= none - data were observed with online bandpass correction.
    #
    # NOTE: This data has been filled into MIRIAD, line-length correction 
    #	done, and then exported as separate files for each source.
    #	3c273 was not line length corrected since it was observed
    #	for such a short amount of time that it did not need it.  
    #
    # From miriad: source Vlsr = 408; delta V is 20 km/s 
    #
    # NOTE: This data contains correlations of only one polarization, 'YY'.
    # The antennas contain 'X' and 'Y' feeds, but the 'X' data was not 
    # correlated.
    #
    ##########################################################################
    # [ This section derived from the 2008 summer school tutorial. ]
    # [ http://casa.nrao.edu/Tutorial/SIworkshop2008/Scripts/ngc4826_tutorial.py ]
    #
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
    #
    ##########################################################################
    #
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
#=============================================================================

#=============================================================================
def load_jupiter6cm(prefix,msname):
    ######################################################################
    #                                                                    #
    # This is a VLA 6cm dataset that was observed in 1999 to set the     #
    # flux scale for calibration of the VLA.  Included in the program    #
    # were observations of the planets, including Jupiter.               #
    #                                                                    #
    # This is D-config VLA 6cm (4.85GHz) obs
    # Check the observational status summary
    # Primary beam FWHM = 45'/f_GHz = 557"
    # Synthesized beam FWHM = 14"
    # RMS in 10min (600s) = 0.06 mJy (thats now, but close enough)
    #
    ######################################################################
    # [ This section derived from the Jupiter 6cm VLA Use Case Script. ]
    # [ http://casa.nrao.edu/casatraining.shtml#scripts ]

    # Clean up old files
    os.system('rm -rf '+prefix+'*')    
    # This is the output MS file name
    msfile = msname

    # Get to path to the CASA home and stip off the name
    pathname=os.environ.get('CASAPATH').split()[0]
    # This is where the UVFITS data should be
    fitsdata=pathname+'/data/regression/jupiter6cm/jupiter6cm.fits'
    # Can also be found online at
    #http://casa.nrao.edu/Data/VLA/Planets6cm/planets_6cm.fits

    # Import the data from FITS to MS
    print '--Import--'
    print "Use importuvfits to read UVFITS and make an MS"
    # Set up the MS filename and save as new global variable
    print "MS will be called "+msfile
    # Use task importuvfits
    importuvfits(fitsfile=fitsdata,vis=msfile)
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
print "* TEST " + str(testNum) + ": using ngc4826 tutorial data"
print ""

prefix = regressionDir+'/test'+str(testNum)+'/'
msname = prefix+"ngc4826.tutorial.ms"  
outputFilename = prefix+'listvis.ngc4826.short.out'
localData = pathName + '/data/regression/ngc4826/'
standardFilename = localData+'listvis.short.out'

#Remove existing data or load data from scratch?
if (not lt.resetData([msname], automate)):
    print "Using preexisting data."
    lt.removeOut(outputFilename)
else:
    print "Building data from scratch."
    tstutl.maketestdir(prefix) # create test dir, overwrite preexisting
    load_ngc4826(prefix,msname) # Build data from scratch

# Setup listvis input and run
default(listvis)
vis                 = msname            #  Name of input visibility file
options             =       'ap'        #  List options: ap only
datacolumn          =     'data'        #  Column to list: data, corrected, model, residual
field               =         ''        #  Field names or index to be listed: ''==>all
spw                 =  '0~1:0~9'        #  Spectral window:channels: '\*'==>all, spw='1:5~57'
selectdata          =       True        #  Other data selection parameters
antenna             =      '6&7'        #  Antenna/baselines: ''==>all, antenna = '3'
timerange           = '<03:33:48'       #  Time range: ''==>all
correlation         =         ''        #  Correlations: ''==>all, correlation = 'RR RL'
scan                =         ''        #  Scan numbers
feed                =         ''        #  Multi-feed numbers (Not yet implemented)
array               =         ''        #  Array numbers (Not yet implemented)
uvrange             =         ''        #  uv range: ''==>all; not yet implemented
average             =         ''        #  Averaging mode: ==>none (Not yet implemented)
showflags           =      False        #  Show flagged data (Not yet implemented)
pagerows            =         20        #  Rows per page
listfile            = outputFilename   #  Output file
async               =      False        #  If true the taskname must be started using listvis(...)
go(listvis)

comparefilename = prefix + 'compare_' + str(testNum)
if (lt.runTests(outputFilename,standardFilename,'1.000',comparefilename)):
    print "passed listvis output test"
    testPassed +=1
else:
    print "failed listvis output test"
    testFailed +=1
    
##########################################################################                                                                        
# TEST 2 - NGC4826 defaults
# Run listvis using default values.  Using ngc4826 data from above.
#
testNum += 1
print ""
print "* TEST " + str(testNum) + ": default test; using ngc4826 tutorial data."
print """
- Using default input values, except for 'vis' and 'listfile'.
- This data has multiple spws with variable numbers of channels.
  *** listvis does not handle this data appropriately.    ***
  *** WARNing messages will be sent to the logger.        ***
  *** This is a known issue.  The test should still pass. ***

"""

prefix = regressionDir+'/test'+str(testNum)+'/'
outputFilename = prefix + 'listvis.ngc4826.default.out'
standardFilename = localData+'listvis.default.out'

tstutl.maketestdir(prefix) # create test dir, overwrite preexisting
if (not lt.resetData([msname], automate)):
    print "Using data from test 1."
else:
    print "Building data from scratch."
    msname = prefix+"ngc4826.tutorial.ms"  
    load_ngc4826(prefix,msname) # Build data from scratch

default(listvis)
vis                 = msname #  Name of input visibility file
listfile            = outputFilename        #  Output file
go(listvis)

comparefilename = prefix + 'compare_' + str(testNum)
if (lt.runTests(outputFilename,standardFilename,'1.000',comparefilename)):
    print "passed listvis output test"
    testPassed +=1
else:
    print "failed listvis output test"
    testFailed +=1

##########################################################################                                                                        
# TEST - jupiter6cm
# Run listvis using all available (non-default) parameters.
#
testNum += 1
print ""
print "* TEST " + str(testNum) + ": jupiter 6cm test, use all available parameters"
print ""

prefix = regressionDir+'/test'+str(testNum)+'/'
msname = prefix+"jupiter6cm.demo.ms"  
outputFilename = prefix+'listvis.jupiter6cm.nondefault.out'
localData = pathName + '/data/regression/jupiter6cm/'
standardFilename = localData+'listvis.nondefault.out'

#Remove existing data or load data from scratch?
if (not lt.resetData([msname], automate)):
    print "Using preexisting data."
    lt.removeOut(outputFilename)
else:
    print "Building data from scratch."
    tstutl.maketestdir(prefix) # create test dir, overwrite preexisting
    load_jupiter6cm(prefix,msname) # Build data from scratch

default(listvis)
vis                 = msname            #  Name of input visibility file
options             =       'ap'        #  List options: ap only
datacolumn          =     'data'        #  Column to list: data, corrected, model, residual
field               =       '12'        #  Field names or index to be listed: ''==>all
spw                 =        '0'        #  Spectral window:channels: '\*'==>all, spw='1:5~57'
selectdata          =       True        #  Other data selection parameters
antenna             =    '16&22'        #  Antenna/baselines: ''==>all, antenna = '3'
timerange           = '<18:31:36.6'     #  Time range: ''==>all
correlation         = 'LL LR RR'        #  Correlations: ''==>all, correlation = 'RR RL'
scan                =       '76'        #  Scan numbers
feed                =         ''        #  Multi-feed numbers (Not yet implemented)
array               =         ''        #  Array numbers (Not yet implemented)
uvrange             =         ''        #  uv range: ''==>all; not yet implemented
average             =         ''        #  Averaging mode: ==>none (Not yet implemented)
showflags           =      False        #  Show flagged data (Not yet implemented)
pagerows            =          5        #  Rows per page
listfile            = outputFilename    #  Output file
async               =      False        #  If true the taskname must be started using listvis(...)
go(listvis)

comparefilename = prefix + 'compare_' + str(testNum)
if (lt.runTests(outputFilename,standardFilename,'1.000',comparefilename)):
    print "passed listvis output test"
    testPassed +=1
else:
    print "failed listvis output test"
    testFailed +=1

##########################################################################                                                                        
# TEST - jupiter6cm, mostly defaults
# Run listvis using mostly default values.  Some selection is required;
# otherwise the output file will be 227MB!
# Using jupiter6cm data from above.
#
testNum += 1
print ""
print "* TEST " + str(testNum) + ": jupiter 6cm data, mostly defaults"
print ""

prefix = regressionDir+'/test'+str(testNum)+'/'
outputFilename = prefix + 'listvis.jupiter6cm.default.out'
standardFilename = localData+'listvis.default.out'

tstutl.maketestdir(prefix) # create test dir, overwrite preexisting
#Remove existing data or load data from scratch?
if (not lt.resetData([msname], automate)):
    print "Using data from previous test."
else:
    print "Building data from scratch."
    msname = prefix+"jupiter6cm.demo.ms"  
    load_jupiter6cm(prefix,msname) # Build data from scratch

default(listvis)
vis                 = msname            #  Name of input visibility file
selectdata          =       True        #  Other data selection parameters
antenna             =       '10'        #  Antenna/baselines: ''==>all, antenna = '3'
scan                =        '1'        #  Scan numbers
listfile            = outputFilename    #  Output file
go(listvis)

comparefilename = prefix + 'compare_' + str(testNum)
if (lt.runTests(outputFilename,standardFilename,'1.000',comparefilename)):
    print "passed listvis output test"
    testPassed +=1
else:
    print "failed listvis output test"
    testFailed +=1

##########################################################################                                                                        
# Test complete, summarize.
#

print ""
print "* listvis regression test complete"
print "SUMMARY:"
print "  number of tests PASSED: " + str(testPassed)
print "  number of tests FAILED: " + str(testFailed)
print ""

print "END: listvis_regression.py"

if (testFailed > 0):
    regstate = False
else:
    regstate = True
    
endTime = time.time()
