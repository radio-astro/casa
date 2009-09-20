import os
import time


# Enable benchmarking?
benchmarking = True
usedasync = False

# 
# Set up some useful variables
#
# Get path to CASA home directory by stipping name from '$CASAPATH'
pathname=os.environ.get('CASAPATH').split()[0]

# This is where the NGC5921 UVFITS data will be
fitsdata=pathname+'/data/demo/3DDAT.fits'

os.system('rm -rf n3ddat*')

# Start benchmarking
if benchmarking:
    startTime = time.time()
    startProc = time.clock()

#
#=====================================================================
#
# Import the data from FITS to MS
#
print '--Import--'

# Safest to start from task defaults
default('importuvfits')

prefix='n3ddat'
# Set up the MS filename and save as new global variable
msfile = prefix + '.ms'

# Use task importuvfits
fitsfile = fitsdata
vis = msfile
antnamescheme = 'new'
importuvfits()

# Note that there will be a ngc5921.ms.flagversions
# containing the initial flags as backup for the main ms flags.

# Record import time
if benchmarking:
    importtime = time.time()

flagdata(vis=msfile, mode='manualflag', antenna='VA10', spw='*', field='0')

print '--Widefield--'
default('widefield')

# Pick up our split source data
vis = msfile

# Make an image root file name
imname = prefix + '.clean'
imagename = imname

# Set up the output image cube
mode = 'mfs'


# This is a single-source MS with one spw
field = '0'
spw = '*'

selectdata=True
#uvrange='0.15~10000 klambda'
# Standard gain factor 0.1
gain = 0.1

# Set the output image size and cell size (arcsec)
imsize = [3000,3000]

# Do a simple Clark clean
psfmode = 'clark'

cell = ['30arcsec','30arcsec']

# Fix maximum number of iterations
niter = 6000

# Also set flux residual threshold (in mJy)
threshold=0.0

# Set up the weighting
weighting = 'natural'

#ftmachine
ftmachine='wproject'
wprojplanes=200
cyclefactor=3

# No clean mask or cleanbox for now
mask = ''
#cleanbox = []

# But if you had a cleanbox saved in a file, e.g. "regionfile.txt"
# you could use it:
#cleanbox='regionfile.txt'
#
# and if you wanted to use interactive clean
#cleanbox='interactive'

widefield()

#
# Following is should be the equivalent clean task setup
#
#clean(vis="n3ddat.ms",imagename="ttt",outlierfile="",field="0",spw="*",selectdata=False,timerange="",uvrange="",antenna="",scan="",mode="mfs",gridmode="widefield",wprojplanes=200,facets=1,cfcache="cfcache.dir",painc=360.0,epjtable="",interpolation="nearest",niter=6000,gain=0.1,threshold=0.0,psfmode="clark",imagermode="csclean",ftmachine="mosaic",mosweight=False,scaletype="SAULT",multiscale=[],negcomponent=-1,smallscalebias=0.6,interactive=False,mask=[],nchan=1,start=0,width=1,imsize=[3000, 3000],cell=['30arcsec', '30arcsec'],phasecenter="",restfreq="",stokes="I",weighting="natural",robust=0.0,uvtaper=False,outertaper=[],innertaper=[],modelimage="",restoringbeam=[''],pbcor=False,minpb=0.1,calready=False,noise="1.0Jy",npixels=0,npercycle=100,cyclefactor=3,cyclespeedup=-1,nterms=1,reffreq="")



# Should find stuff in the logger like:
#
# Fitted beam used in restoration: 51.5643 by 45.6021 (arcsec)
#     at pa 14.5411 (deg)
#
# It will have made the images:
# -----------------------------
# ngc5921.clean.image
# ngc5921.clean.model
# ngc5921.clean.residual

clnimage = imname+'.image'

# Record clean completion time
if benchmarking:
    cleantime = time.time()

print '--Imstat--'
default('imstat')

imagename = clnimage

cubestats =imstat()

#
#=====================================================================
#
# Can do some image statistics if you wish
# Treat this like a regression script
# WARNING: currently requires toolkit
#
print ""
print ' 3DDAT results '
print ' =============== '

print ''
print ' --Regression Tests--'
print ''
# Pull the max from the cubestats dictionary
# created above using imstat
thistest_immax=cubestats['max'][0]
oldtest_immax = 7.50
print ' Clean image max should be ',oldtest_immax
print ' Found : Image Max = ',thistest_immax
diff_immax = abs((oldtest_immax-thistest_immax)/oldtest_immax)
print ' Difference (fractional) = ',diff_immax

print ''
# Pull the rms from the cubestats dictionary
thistest_imrms=cubestats['rms'][0]
oldtest_imrms = 0.134
print ' Clean image rms should be ',oldtest_imrms
print ' Found : Image rms = ',thistest_imrms
diff_imrms = abs((oldtest_imrms-thistest_imrms)/oldtest_imrms)
print ' Difference (fractional) = ',diff_imrms



# Record processing completion time
if benchmarking:
    endProc = time.clock()
    endTime = time.time()

#
#=====================================================================
#
if not benchmarking:
    print ''
    print '--- Done ---'
else:
    import datetime
    datestring=datetime.datetime.isoformat(datetime.datetime.today())
    outfile='wproj3ddat.'+datestring+'.log'
    logfile=open(outfile,'w')

    print >>logfile,''
    print >>logfile,''


    if (diff_immax < 0.05):
        passimmax = True
        print >>logfile,'* Passed image max test'
    else:
        passimmax = False
        print >>logfile,'* FAILED image max test'
    print >>logfile,'*  Image max '+str(thistest_immax)

    if (diff_imrms < 0.05):
        passimrms = True
        print >>logfile,'* Passed image rms test'
    else:
        passimrms = False
        print >>logfile,'* FAILED image rms test'
    print >>logfile,'*  Image rms '+str(thistest_imrms)

    if (passimmax and passimrms ): 
	regstate=True
	print >>logfile,'---'
	print >>logfile,'Passed Regression test for wproj3ddat'
	print >>logfile,'---'
    else: 
	regstate=False
	print >>logfile,'---'
	print >>logfile,'FAILED Regression test for wproj3ddat'
	print >>logfile,'---'
    print >>logfile,'*********************************'

    print >>logfile,''
    print >>logfile,''
    print >>logfile,'********* Benchmarking *****************'
    print >>logfile,'*                                      *'
    print >>logfile,'Total wall clock time was: ', endTime - startTime
    print >>logfile,'Total CPU        time was: ', endProc - startProc
    print >>logfile,'Processing rate MB/s  was: ', 35.1/(endTime - startTime)
    print >>logfile,'* Breakdown:                           *'
    print >>logfile,'*   import       time was: '+str(importtime-startTime)
    print >>logfile,'*   clean        time was: '+str(cleantime-importtime)
    print >>logfile,'*****************************************'
    print >>logfile,'basho (test cpu) time was: ?? seconds'

    logfile.close()

    print ""
    print "Done!"
        
#exit()
#
#=====================================================================
