#====================================================================
#                                                                    
# Script for simulating and imaging VLA Orion continuum mosaic       
#                                                                    
# Created STM 2008-11-04 from SMA mosaic simulation
# Updated STM 2008-11-06 formatting improvements
# Updated STM 2008-11-19 Patch3 final version 640x640
# Updated STM 2009-06-16 Patch4 800x800
# Updated STM 2009-06-23 Patch4 web download of pickle file
# Updated STM 2009-06-25 minor improvements
# Updated STM 2009-08-24 Release0, new immath syntax, noise
# Updated STM 2009-08-27 pbcor=F on clean
# Updated STM 2009-08-27 noise only option
# Updated STM 2009-10-22 point to Release0 pickle file
#         RI  2012-01-15 prefer pickle in DATADIR/regressions/ if exists
#
# Simulates point sources and cleans
#                                                                       
# Script Notes:                                                         
#    o Starts with a template MS <templatems> which must be present
#      (set <templatems> to point to this)
#    o The results are written out as a dictionary in a pickle file     
#         <scriptprefix>.regression.<datestring>.pickle           
#      as well as in a text file                                        
#         out.<scriptprefix>.<datestring>.log                         
#      (these are not auto-deleted at start of script)                  
#    o If you want to show regression differences from a previous
#      run then you can provide a file from a previous run   
#         <scriptprefix>.pickle
#                                                                       
#====================================================================
# IMPORTANT: VERSIONING HERE
#====================================================================
myscriptvers = '2012-01-15 RI'

import time
import os
import pickle

# needed for existence check
from os import F_OK

scriptprefix='run_orionmos4sim'
prefix='simtest_orionmos4sim'

# Clean up old output files
os.system('rm -rf '+prefix+'*')

#====================================================================
# IMPORTANT: SET STUFF UP HERE
#====================================================================
mytmp     = '==========================='
mydataset = 'Orion VLA mosaic simulation'

print "========================"+mytmp
print "Simulating and Cleaning "+mydataset
print "========================"+mytmp
print "Script version "+myscriptvers
print "Output will be prefixed with "+prefix

#Start from existing MS
templatems = 'orion_calsplit.ms'

if os.access(templatems,F_OK):
    # already in current directory
    print "  Using "+templatems+" found in current directory"
else:
    pathname=os.environ.get('CASAPATH').split()[0]
    #datapath=pathname+'/data/regression/ATST3/Orion/'
    datapath=pathname+'/data/regression/orionmos4sim/'
    msname='orion.ms'
    mspath=datapath+msname
    # Path to web archive
    webms = 'http://casa.nrao.edu/Data/VLA/Orion/'+msname+'.tgz'
    # Alternate path at AOC
    altms = '/home/ballista/casa/active/data/regression/ATST3/Orion/'+msname
    if os.access(msname,F_OK):
        # ms in current directory
        print "  Using "+msname+" found in current directory"
    elif os.access(mspath,F_OK):
        print '--Copy data to local directory--'
        print "  Using "+mspath
        os.system("cp -r "+mspath+" .")
        os.system('chmod -R a+wx '+msname)
    elif os.access(altms,F_OK):
        print '--Copy data to local directory--'
        print "  Using "+altms
        os.system("cp -r "+altms+" .")
        os.system('chmod -R a+wx '+msname)
    else:
        if os.access(msname+'.tgz',F_OK):
            # ms tarball in current directory
            print "  Using "+msname+".tgz found in current directory"
        else:
            # try web retrieval
            print '--Retrieve data from '+webms
            # Use curl (for Mac compatibility)
            os.system('curl '+webms+' -f -o '+msname+'.tgz')
            # NOTE: could also use wget
            #os.system('wget '+webms)
        
        print '--Unpacking tarball '
        os.system('tar xzf '+msname+'.tgz')
        if os.access(msname,F_OK):
            # should now be in current directory
            print "  Using "+msname+" found in current directory"
        else:
            raise IOError," ERROR: "+msname+" not found"

    # Starting from orion.ms which has already been calibrated
    print '--Split--'
    split(vis=msname,outputvis=templatems,datacolumn='corrected')
    print "  Created "+templatems

#Clean params
myimsize = 800
#myimsize = 640
mycell   = "2.0arcsec"
mycen    = "J2000 05:35:17.470 -5.23.06.790"
myniter  = 400
#mymask   = "save.oriontest_mosaic.mask"
mymask   = ""
myfield  = "2~10"
myspw    = ""
myftmachine = "mosaic"
mycyclefactor = 1.5
myminpb = 0.2

#Sim params
myflux = 1.0
mynoise = ""
#mynoise = "0.019Jy"
# EVLA X-band SEFD is 300Jy (1Hz,1sec) so 50MHz,5sec would be 19mJy per vis

print "Will use template MS "+ templatems
print "Will image with imsize %5i and cell %s " % (myimsize,mycell)
print "Will clean "+str(myniter)+" iterations"
if mycen!="":
    print "Will use phasecenter "+mycen
if myfield!="":
    print "Will image fields "+myfield
if myspw!="":
    print "Will use spw "+myspw
if myftmachine!="":
    print "Will mosaic using ftmachine "+myftmachine
if mymask!="":
    print "Will use clean mask "+mymask
if mynoise!="":
    print "Will add noise level of "+mynoise

# Save the parameters used in clean etc.
params = {}
# Version of script
params['version'] = myscriptvers
# User set params
params['user'] = {}
params['user']['templatems'] = templatems
params['user']['myimsize'] = myimsize
params['user']['mycell'] = mycell
params['user']['mycen'] = mycen
params['user']['myniter'] = myniter
params['user']['mymask'] = mymask
params['user']['myfield'] = myfield
params['user']['myspw'] = myspw
params['user']['myftmachine'] = myftmachine
params['user']['mycyclefactor'] = mycyclefactor
params['user']['myminpb'] = myminpb
params['user']['mynoise'] = mynoise

#====================================================================

# Get this into a direction measure
phc = me.direction(mycen.split()[0],mycen.split()[1],mycen.split()[2])
# Reference pixel of image
myref = int((myimsize+0.5)/2.0)
dcell = qa.convert(mycell,'arcsec')['value']

#====================================================================
# Dictionary with components to simulated

# Comp A is at the peak of the .flux image:
#     05:35:22.024, -05.24.14.789
# Note that only comp A is centered on a pixel by fiat
complist = {}
complist['A'] = {}
complist['A']['rf'] = 'J2000'
complist['A']['RA'] = '05:35:22.024'
complist['A']['Dec'] = '-05.24.14.789' 
complist['A']['flux'] = myflux

# Comp B is in center of field 6 (phs center), at 96% of peak .flux
#     05:35:17.47      -05.23.06.79
complist['B'] = {}
complist['B']['rf'] = 'J2000'
complist['B']['RA'] = '05:35:17.47'
complist['B']['Dec'] = '-05.23.06.79'
complist['B']['flux'] = myflux

# Comp C is center of field 10, at 75% of peak .flux
#     05:35:27.52      -05.20.37.52
complist['C'] = {}
complist['C']['rf'] = 'J2000'
complist['C']['RA'] = '05:35:27.52'
complist['C']['Dec'] = '-05.20.37.52'
complist['C']['flux'] = myflux

# Comp D is SSE of field 2, at 33% of peak .flux
#     05:35:08.746    -05.27.19.013
complist['D'] = {}
complist['D']['rf'] = 'J2000'
complist['D']['RA'] = '05:35:08.746'
complist['D']['Dec'] = '-05.27.19.013'
complist['D']['flux'] = myflux

# Which of these to use
compnames = ['A','B','C','D']

# Box around each component in image
dbox = 10
for comp in compnames:
    # get as direction
    mydir = me.direction(complist[comp]['rf'],complist[comp]['RA'],complist[comp]['Dec'])
    # get offset and angle
    off_sep = me.separation(phc,mydir)
    off_sec = qa.convert(off_sep,'arcsec')['value']
    off_pa = me.posangle(phc,mydir)
    off_par = qa.convert(off_pa,'rad')['value']
    # offsets in pixels
    off_x = off_sec*sin(off_par)/dcell
    off_y = off_sec*cos(off_par)/dcell
    xpix = int( myref + 0.5 - off_x )
    ypix = int( myref + 0.5 + off_y )
    # store these in dictionary
    complist[comp]['xpix'] = xpix
    complist[comp]['ypix'] = ypix
    # now make box for clean and stats
    blcx = xpix - dbox
    blcy = ypix - dbox
    trcx = xpix + dbox
    trcy = ypix + dbox
    mybox = str(blcx)+","+ str(blcy)+","+ str(trcx)+","+ str(trcy)
    complist[comp]['box'] = mybox
    myclnbox = [blcx,blcy,trcx,trcy]
    complist[comp]['clnbox'] = myclnbox

#====================================================================
# Start actual processing
#====================================================================
startTime=time.time()
startProc=time.clock()

#====================================================================
# Set up component list with 1Jy point source
clfile = prefix + "_sim.cl"
print "Creating componentlist "

for comp in compnames:
    mydirection = "J2000 "+complist[comp]['RA']+" "+complist[comp]['Dec']
    compflux = complist[comp]['flux']
    cl.addcomponent(dir=mydirection, flux=compflux, freq='230.0GHz', spectrumtype='constant')
    print "  Added comp "+comp+" "+str(compflux)+" Jy at "+mydirection+" ("+str(complist[comp]['xpix'])+","+str(complist[comp]['ypix'])+")"

cl.rename(clfile)
mycl = cl.torecord()
cl.close()

print "Created componentlist "+clfile

#Copy a new MS to work from template
msfile = prefix + "_sim.ms"

print "Copying "+templatems+" to "+msfile

os.system('cp -rf '+templatems+' '+msfile)

sm.openfromms(msfile)
sm.setvp(dovp=T, usedefaultvp=T, dosquint=F)

#Add components
sm.predict(complist=clfile)

#Add noise if desired
if mynoise!="":
    print 'Adding noise '+mynoise
    sm.setvp(dovp=F)
    sm.setnoise(mode='simplenoise',simplenoise=mynoise)
    sm.corrupt()

sm.close()

print "Completed simulating MS "+msfile

listobs(msfile)

#====================================================================
#DS9 shape files for viewer
#j2000; text 05:35:07.42 -05:25:36.07 # text={2} color=blue   
#j2000; text 05:35:17.42 -05:25:36.79 # text={3} color=blue   
#j2000; text 05:35:27.42 -05:25:37.52 # text={4} color=blue   
#j2000; text 05:35:27.47 -05:23:07.52 # text={5} color=blue   
#j2000; text 05:35:17.47 -05:23:06.79 # text={6} color=blue   
#j2000; text 05:35:07.47 -05:23:06.07 # text={7} color=blue   
#j2000; text 05:35:07.52 -05:20:36.07 # text={8} color=blue   
#j2000; text 05:35:17.52 -05:20:36.80 # text={9} color=blue   
#j2000; text 05:35:27.52 -05:20:37.52 # text={10} color=blue   
#j2000; text 05:35:32.61 -05:16:07.88 # text={11} color=blue   
# 
# From listobs:
#    Fields: 12
# 	  ID   Code Name          Right Ascension  Declination   Epoch   
# 	  0    A    0518+165      05:21:09.89      +16.38.22.04  J2000   
# 	  1    A    0539-057      05:41:38.09      -05.41.49.43  J2000   
# 	  2         ORION1        05:35:07.42      -05.25.36.07  J2000   
# 	  3         ORION2        05:35:17.42      -05.25.36.79  J2000   
# 	  4         ORION3        05:35:27.42      -05.25.37.52  J2000   
# 	  5         ORION4        05:35:27.47      -05.23.07.52  J2000   
# 	  6         ORION5        05:35:17.47      -05.23.06.79  J2000   
# 	  7         ORION6        05:35:07.47      -05.23.06.07  J2000   
# 	  8         ORION7        05:35:07.52      -05.20.36.07  J2000   
# 	  9         ORION8        05:35:17.52      -05.20.36.80  J2000   
# 	  10        ORION9        05:35:27.52      -05.20.37.52  J2000   
# 	  11        ORION10       05:35:32.61      -05.16.07.88  J2000   
# Spectral Windows:
# SpwID  #Chans Frame Ch1(MHz)   ChanWid(kHz)TotBW(kHz)  Ref(MHz)   Corrs
# 0      1 TOPO  8435.1   50000    50000       8435.1      RR  LL  RL  LR  
# 1      1 TOPO  8485.1   50000    50000       8485.1      RR  LL  RL  LR  
#====================================================================

sim1time=time.time()

#====================================================================
#  Some details about the mosaic (from old aips++ glish script): 
#    primary beam at X-band = 5.4', 
#    mosaic field spacing = 2.5'
#    total mosaic size = approx. 9.5' = 570"
#    synthesized beam size = 8.4" in D config at 3.6 cm, 8.3 GHz
#    cell size = 2" and nx,ny = 300 (600" field size)
#    phase center = center field: J2000 05:35:17.470, -005.23.06.790
#    NOTE: field 10 is outside of the 9 point primary mosaic (sitting 
#     on M43 -- but the flux is resolved out so there is no use to 
#     add it to the mosaic.  The script below leaves it out.  
#====================================================================
# Clean
print "Preparing to image data"

# Set up clean boxes
clnbox = []
for comp in compnames: 
    clnbox.append(complist[comp]['clnbox'])

print ""
print "Will be cleaning in box(es) "+str(clnbox)

# MFS imaging for continuum
default("clean")
vis                =  msfile
field              =  myfield
spw                =  myspw
selectdata         =  False
timerange          =  ""
uvrange            =  ""
antenna            =  ""
scan               =  ""
mode               =  "mfs"
gain               =  0.1
threshold          =  "0.0mJy"
psfmode            =  "clark"
imagermode         =  "mosaic"
ftmachine          =  myftmachine
mosweight          =  False
scaletype          =  "SAULT"
flatnoise           =  False
multiscale         =  []
negcomponent       =  -1
interactive        =  F
mask               =  []
nchan              =  1
start              =  0
width              =  1
phasecenter        =  mycen
imsize             =  myimsize
cell               =  mycell
restfreq           =  ""
stokes             =  "I"
weighting          =  "natural"
robust             =  0.0
uvtaper            =  False
outertaper         =  []
innertaper         =  []
modelimage         =  ""
restoringbeam      =  ['']
minpb              =  myminpb
noise              =  "1.0Jy"
npixels            =  0
npercycle          =  100
cyclefactor        =  mycyclefactor
cyclespeedup       =  -1

# Set phasecenter either on field 1 or using Crystal's
#phasecenter        =  "1"
print "Will be mosaicing using phasecenter "+phasecenter

print "First make dirty image"
pbcor              =  T
dirtimag = prefix + ".dirty"
imagename          =  dirtimag
niter              =  0
clean()
print "Output images are called "+imagename

dirty1time=time.time()

if mymask == "":
    mask = clnbox
    print "Will be cleaning using box(es) "+str(mask)
else:
    mask = mymask
    print "Will be cleaning using mask "+mask

#Clean image
contimag = prefix + ".clean"
pbcor              =  F
imagename          =  contimag
niter              =  myniter
print "Now actually clean the image using niter = "+str(niter)
clean()
print "Output images are called "+imagename

clean1time=time.time()

# What Crystal's been doing
#
#contimag='g19_d2usb_targets.ms.cont.im'
#clean(vis=vis,imagename=contimag,
#      field='',spw='0~12,16~23',
#      mode='mfs',
#      niter=100,gain=0.1,threshold=0.0,
#      psfmode='clark',imagermode='mosaic',scaletype='SAULT',
#      mask='g19_d2usb_cont.model.mask',
#      interactive=F,imsize=400,cell="0.5arcsec",
#      phasecenter="J2000 18h25m56.09 -12d04m28.20",
#      pbcor=T,minpb=0.15)

dirimage = dirtimag+'.image'
clnimage = contimag+'.image'
fluximage = contimag+'.flux'
modimage = contimag+'.model'
resimage = contimag+'.residual'
covimage = contimag+'.flux.pbcoverage'

#====================================================================
# pbcor the model, be careful to mask zeros

pbmodimage = contimag+'.pbcormod'
pbcorimage = contimag+'.pbcorimg'
# Old syntax
#if myflux!=0.0:
#    immath(outfile=pbmodimage,
#           mode='evalexpr',
#           expr="'"+modimage+"'['"+fluximage+"'!=0.0]/'"+fluximage+"'")
#immath(outfile=pbcorimage,
#       mode='evalexpr',
#       expr="'"+clnimage+"'['"+fluximage+"'!=0.0]/'"+fluximage+"'")

# New syntax (STM 2009-08-24)
if myflux!=0.0:
    immath(outfile=pbmodimage,
           mode='evalexpr',
           imagename=[modimage,fluximage],
           expr="IM0[IM1!=0.0]/IM1")

immath(outfile=pbcorimage,
       mode='evalexpr',
       imagename=[clnimage,fluximage],
       expr="IM0[IM1!=0.0]/IM1")

# stats
#====================================================================
# Some continuum image statistics

# Set up off-src box
# Offset from first component
#offbox = str(myref+45)+','+str(myref+30)+','+str(myref+110)+','+str(myref+100)
doffx = 75
doffy = 65
doffbox = 30
xpix = complist[compnames[0]]['xpix']
ypix = complist[compnames[0]]['ypix']
blcx = xpix + doffx - doffbox
blcy = ypix + doffy - doffbox
trcx = xpix + doffx + doffbox
trcy = ypix + doffy + doffbox
offbox = str(blcx)+","+ str(blcy)+","+ str(trcx)+","+ str(trcy)
print "Off-source stats in box "+offbox

# Do dirty image stats
dirstats = imstat(imagename=dirimage,box = '')
dirtyimg_max = dirstats['max'][0]
dirtyimg_rms = dirstats['sigma'][0]

diroffstats = imstat(imagename=dirimage,box=offbox)
dirtyoff_rms = diroffstats['sigma'][0]

# Do clean image
allstats = imstat(imagename=pbcorimage,box = '')
imageall_max = allstats['max'][0]
imageall_rms = allstats['sigma'][0]

offstats = imstat(imagename=pbcorimage,box=offbox)
imageoff_rms = offstats['sigma'][0]

# Do residual image
resstats = imstat(imagename=resimage,box = '')
residall_max = resstats['max'][0]
residall_rms = resstats['sigma'][0]

resoffstats = imstat(imagename=resimage,box=offbox)
residoff_rms = resoffstats['sigma'][0]

if myflux!=0.0:
    # Do whole model
    modstats = imstat(imagename=modimage,box = '')
    modelall_flux = modstats['sum'][0]

    pbmodstats = imstat(imagename=pbmodimage,box = '')
    pbmodelall_flux = pbmodstats['sum'][0]

# Flux image effective area (STM 2009-06-18)
fluxstats = imstat(imagename=fluximage,box = '')
fluximg_sum = fluxstats['sum'][0]
fluximg_npt = fluxstats['npts'][0]
# Extract pixel area (sq.arcsec.)
pixsec = qa.convert(qa.quantity(mycell),'arcsec')
pixarea = qa.quantity(pixsec)['value']**2

stats1time=time.time()

#====================================================================
# Some image fitting
# There is a 1Jy source at center of field 1 in image
# Comp Restored image
# Using phasecenter='1'
# A    467,372  at peak between 2,3,4 18:25:55.116 -12.04.32.500
# B    400,400  at 75% center field 1 18:25:57.400 -12.04.18.50
# C    360,462  at 40% N of field 0   18:25:58.764 -12.03.47.504
# D    511,420  at 20% NW of field 3  18:25:53.607 -12.04.08.582

# Using phasecenter="J2000 18h25m56.09 -12d04m28.20"
# Note that only comp A is centered on a pixel by fiat
# A    429,391  at peak between 2,3,4 18:25:55.101 -12.04.32.700
# B    362,420  at 75% center field 1 18:25:57.400 -12.04.18.50
# C    321,481  at 40% N of field 0   18:25:58.764 -12.03.47.504
# D    473,440  at 20% NW of field 3  18:25:53.607 -12.04.08.582

# Use 21x21 boxes centered on above coordinates
# Calculate:
#    (a) Peak in restored image (Jy/bm)
#    (b) Flux in imfit of restored image (Jy)
#    (c) Flux in box in sim model (Jy)
#

totalflux = 0.0
# Box around each component in image
for comp in compnames:
    compflux = complist[comp]['flux']
    totalflux = totalflux + compflux
    mybox = complist[comp]['box']
    print "Component "+comp+" stats in box "+mybox
    # Dirty image
    xstat_dirimg = imstat(imagename=dirimage,box=mybox)
    # Clean image
    xstat_img = imstat(imagename=pbcorimage,box=mybox)
    if myflux!=0.0:
        # Clean model
        xstat_mod = imstat(imagename=modimage,box=mybox)
        xstat_pbmod = imstat(imagename=pbmodimage,box=mybox)
    # Residual image
    xstat_res = imstat(imagename=resimage,box=mybox)
    # Save in complist
    complist[comp]['dirtystat'] = xstat_dirimg
    complist[comp]['imstat'] = xstat_img
    if myflux!=0.0:
        complist[comp]['modstat'] = xstat_mod
        complist[comp]['pbmodstat'] = xstat_pbmod
    complist[comp]['resstat'] = xstat_res

#====================================================================
# Done
#====================================================================
endProc=time.clock()
endTime=time.time()

import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())

myvers = casalog.version()
myuser = os.getenv('USER')
myhost = str( os.getenv('HOST') )
mycwd = os.getcwd()
myos = os.uname()
mypath = os.environ.get('CASAPATH')

walltime = (endTime - startTime)
cputime = (endProc - startProc)

# Print to terminal, and also save most things to a logfile
outfile = 'out.'+prefix+'.'+datestring+'.log'
logfile = open(outfile,'w')

def lprint(msg, lfile):
    """
    Prints msg to both stdout and lfile.
    """
    print msg
    print >> logfile, msg
    
lprint('Running '+myvers+' on host '+myhost, logfile)
lprint('  at '+datestring, logfile)
lprint('  using '+mypath, logfile)

#
##########################################################################
# Write some more stuff to logfile
lprint('', logfile)
lprint('---', logfile)
lprint('Script version: '+params['version'], logfile)
lprint('User set parameters used in execution:', logfile)
lprint('---', logfile)

for keys in params['user'].keys():
    lprint('  %s  =  %s ' % ( keys, params['user'][keys] ), logfile)

lprint('', logfile)

#
##########################################################################
#
# Save info in regression dictionary
new_regression = {}

new_regression['date'] = datestring
new_regression['version'] = myvers
new_regression['user'] = myuser
new_regression['host'] = myhost
new_regression['cwd'] = mycwd
new_regression['os'] = myos
new_regression['aipspath'] = mypath

new_regression['dataset'] = mydataset

# Store the parameters used in clean etc.
new_regression['params'] = params

# Store the results of this regression
results = {}

# First the overall results
op = 'divf'
tol = 0.01
results['clean_dirtyimg_max'] = {}
results['clean_dirtyimg_max']['name'] = 'Dirty image (all) max'
results['clean_dirtyimg_max']['value'] = dirtyimg_max
results['clean_dirtyimg_max']['op'] = op
results['clean_dirtyimg_max']['tol'] = tol

results['clean_dirtyimg_rms'] = {}
results['clean_dirtyimg_rms']['name'] = 'Dirty image (all) rms'
results['clean_dirtyimg_rms']['value'] = dirtyimg_rms
results['clean_dirtyimg_rms']['op'] = op
results['clean_dirtyimg_rms']['tol'] = tol

results['clean_dirtyoff_rms'] = {}
results['clean_dirtyoff_rms']['name'] = 'Dirty image (off-src) rms'
results['clean_dirtyoff_rms']['value'] = dirtyoff_rms
results['clean_dirtyoff_rms']['op'] = op
results['clean_dirtyoff_rms']['tol'] = tol

results['clean_imageall_max'] = {}
results['clean_imageall_max']['name'] = 'Clean image (all) max'
results['clean_imageall_max']['value'] = imageall_max
results['clean_imageall_max']['op'] = op
results['clean_imageall_max']['tol'] = tol

results['clean_imageall_rms'] = {}
results['clean_imageall_rms']['name'] = 'Clean image (all) rms'
results['clean_imageall_rms']['value'] = imageall_rms
results['clean_imageall_rms']['op'] = op
results['clean_imageall_rms']['tol'] = tol

results['clean_imageoff_rms'] = {}
results['clean_imageoff_rms']['name'] = 'Clean image (off-src) rms'
results['clean_imageoff_rms']['value'] = imageoff_rms
results['clean_imageoff_rms']['op'] = op
results['clean_imageoff_rms']['tol'] = tol

results['clean_residall_max'] = {}
results['clean_residall_max']['name'] = 'Residual image (all) max'
results['clean_residall_max']['value'] = residall_max
results['clean_residall_max']['op'] = op
results['clean_residall_max']['tol'] = tol

results['clean_residall_rms'] = {}
results['clean_residall_rms']['name'] = 'Residual image (all) rms'
results['clean_residall_rms']['value'] = residall_rms
results['clean_residall_rms']['op'] = op
results['clean_residall_rms']['tol'] = tol

results['clean_residoff_rms'] = {}
results['clean_residoff_rms']['name'] = 'Residual image (off-src) rms'
results['clean_residoff_rms']['value'] = residoff_rms
results['clean_residoff_rms']['op'] = op
results['clean_residoff_rms']['tol'] = tol

if myflux!=0.0:
    results['clean_modelall_flux'] = {}
    results['clean_modelall_flux']['name'] = 'Model image (all) flux'
    results['clean_modelall_flux']['value'] = modelall_flux
    results['clean_modelall_flux']['op'] = op
    results['clean_modelall_flux']['tol'] = tol

    results['clean_pbmodelall_flux'] = {}
    results['clean_pbmodelall_flux']['name'] = 'PBCOR Model image (all) flux'
    results['clean_pbmodelall_flux']['value'] = pbmodelall_flux
    results['clean_pbmodelall_flux']['op'] = op
    results['clean_pbmodelall_flux']['tol'] = tol

    results['sim_comp_flux'] = {}
    results['sim_comp_flux']['name'] = 'Sim Model total flux'
    results['sim_comp_flux']['value'] = totalflux
    results['sim_comp_flux']['op'] = op
    results['sim_comp_flux']['tol'] = tol

results['flux_area'] = {}
results['flux_area']['name'] = 'Eff. mosaic area (sq.arcmin.)'
results['flux_area']['value'] = fluximg_sum*pixarea/3600.0
results['flux_area']['op'] = op
results['flux_area']['tol'] = tol

# Done filling results
new_regression['results'] = results

# Now the component results
new_regression['compnames'] = compnames
#Store raw statistics
new_regression['compstats'] = complist

compresults = {}
for comp in compnames:
    compresults[comp] = {}
    
    xstat_dirimg = complist[comp]['dirtystat']
    xstat_img = complist[comp]['imstat']
    if myflux!=0.0:
        xstat_mod = complist[comp]['modstat']
        xstat_pbmod = complist[comp]['pbmodstat']
    xstat_res = complist[comp]['resstat']

    compresults[comp]['box'] = complist[comp]['box']

    op = 'divf'
    tol = 0.08
    compresults[comp]['dirty_image_max'] = {}
    compresults[comp]['dirty_image_max']['name'] = 'Dirty image max'
    compresults[comp]['dirty_image_max']['value'] = xstat_dirimg['max'][0]
    compresults[comp]['dirty_image_max']['op'] = op
    compresults[comp]['dirty_image_max']['tol'] = tol
    
    op = 'diff'
    tol = 0.5
    compresults[comp]['dirty_image_maxposx'] = {}
    compresults[comp]['dirty_image_maxposx']['name'] = 'Dirty image max pos x'
    compresults[comp]['dirty_image_maxposx']['value'] = xstat_dirimg['maxpos'][0]
    compresults[comp]['dirty_image_maxposx']['op'] = op
    compresults[comp]['dirty_image_maxposx']['tol'] = tol
    
    compresults[comp]['dirty_image_maxposy'] = {}
    compresults[comp]['dirty_image_maxposy']['name'] = 'Dirty image max pos y'
    compresults[comp]['dirty_image_maxposy']['value'] = xstat_dirimg['maxpos'][1]
    compresults[comp]['dirty_image_maxposy']['op'] = op
    compresults[comp]['dirty_image_maxposy']['tol'] = tol
    
    op = 'divf'
    tol = 0.08
    compresults[comp]['clean_image_max'] = {}
    compresults[comp]['clean_image_max']['name'] = 'Restored image max'
    compresults[comp]['clean_image_max']['value'] = xstat_img['max'][0]
    compresults[comp]['clean_image_max']['op'] = op
    compresults[comp]['clean_image_max']['tol'] = tol

    op = 'diff'
    tol = 0.5
    compresults[comp]['clean_image_maxposx'] = {}
    compresults[comp]['clean_image_maxposx']['name'] = 'Restored image max pos x'
    compresults[comp]['clean_image_maxposx']['value'] = xstat_img['maxpos'][0]
    compresults[comp]['clean_image_maxposx']['op'] = op
    compresults[comp]['clean_image_maxposx']['tol'] = tol
    
    compresults[comp]['clean_image_maxposy'] = {}
    compresults[comp]['clean_image_maxposy']['name'] = 'Restored image max pos y'
    compresults[comp]['clean_image_maxposy']['value'] = xstat_img['maxpos'][1]
    compresults[comp]['clean_image_maxposy']['op'] = op
    compresults[comp]['clean_image_maxposy']['tol'] = tol
    
    op = 'divf'
    tol = 0.08
    compresults[comp]['residual_image_max'] = {}
    compresults[comp]['residual_image_max']['name'] = 'Residual image max'
    compresults[comp]['residual_image_max']['value'] = xstat_res['max'][0]
    compresults[comp]['residual_image_max']['op'] = op
    compresults[comp]['residual_image_max']['tol'] = tol
    
    compresults[comp]['residual_image_rms'] = {}
    compresults[comp]['residual_image_rms']['name'] = 'Residual image rms'
    compresults[comp]['residual_image_rms']['value'] = xstat_res['sigma'][0]
    compresults[comp]['residual_image_rms']['op'] = op
    compresults[comp]['residual_image_rms']['tol'] = tol

    if myflux!=0.0:
        compresults[comp]['model_image_sum'] = {}
        compresults[comp]['model_image_sum']['name'] = 'Model image flux'
        compresults[comp]['model_image_sum']['value'] = xstat_mod['sum'][0]
        compresults[comp]['model_image_sum']['op'] = op
        compresults[comp]['model_image_sum']['tol'] = tol
        
        compresults[comp]['pbmodel_image_sum'] = {}
        compresults[comp]['pbmodel_image_sum']['name'] = 'PBCOR Model image flux'
        compresults[comp]['pbmodel_image_sum']['value'] = xstat_pbmod['sum'][0]
        compresults[comp]['pbmodel_image_sum']['op'] = op
        compresults[comp]['pbmodel_image_sum']['tol'] = tol
        
        compresults[comp]['sim_comp_flux'] = {}
        compresults[comp]['sim_comp_flux']['name'] = 'Simulated comp flux'
        compresults[comp]['sim_comp_flux']['value'] = complist[comp]['flux']
        compresults[comp]['sim_comp_flux']['op'] = op
        compresults[comp]['sim_comp_flux']['tol'] = tol

        op = 'diff'
        tol = 0.5
        compresults[comp]['sim_comp_posx'] = {}
        compresults[comp]['sim_comp_posx']['name'] = 'Simulated comp pos x'
        compresults[comp]['sim_comp_posx']['value'] = complist[comp]['xpix']
        compresults[comp]['sim_comp_posx']['op'] = op
        compresults[comp]['sim_comp_posx']['tol'] = tol
    
        compresults[comp]['sim_comp_posy'] = {}
        compresults[comp]['sim_comp_posy']['name'] = 'Simulated comp pos y'
        compresults[comp]['sim_comp_posy']['value'] = complist[comp]['ypix']
        compresults[comp]['sim_comp_posy']['op'] = op
        compresults[comp]['sim_comp_posy']['tol'] = tol

new_regression['compresults'] = compresults

# Dataset size info
datasize_raw = 110.0 # MB
datasize_ms =  110.0 # MB
new_regression['datasize'] = {}
new_regression['datasize']['raw'] = datasize_raw
new_regression['datasize']['ms'] = datasize_ms

# Save timing to regression dictionary
new_regression['timing'] = {}

total = {}
total['wall'] = (endTime - startTime)
total['cpu'] = (endProc - startProc)
total['rate_raw'] = (datasize_raw/(endTime - startTime))
total['rate_ms'] = (datasize_ms/(endTime - startTime))

new_regression['timing']['total'] = total

nstages = 4
new_regression['timing']['nstages'] = nstages

stages = {}
stages[0] = ['simulate',(sim1time-startTime)]
stages[1] = ['invert',(dirty1time-sim1time)]
stages[2] = ['clean',(clean1time-dirty1time)]
stages[3] = ['stats',(endTime-clean1time)]

new_regression['timing']['stages'] = stages

#
##########################################################################
# See if a previous pickfile can be found
#
# Try and load previous results from regression file
#
regression = {}
regressfile = scriptprefix + '.pickle'
prev_results = {}

repodir=os.getenv("CASAPATH")
repodir=repodir.split()[0]
regressdirfile= repodir+ "/data/regression/orionmos4sim/"+scriptprefix + '.pickle'

# Path to web archive (latest version)
#webfile = 'http://casa.nrao.edu/Doc/Scripts/'+scriptprefix+'.pickle'
#webfile = 'http://casa.nrao.edu/Patch4/Doc/Scripts/'+scriptprefix+'.pickle'
webfile = 'http://casa.nrao.edu/Release0/Doc/Scripts/'+scriptprefix+'.pickle'

if os.access(regressfile,F_OK):
    # pickle file in current directory
    print "  Found "+regressfile+" in current directory"
else:
    if os.access(regressdirfile,F_OK):
        # pickle file in local copy of regressions dir
        print "  Found "+regressdirfile+" in your local data repo"
        regressfile=regressdirfile
    else:
        print "Trying web download of "+webfile
        os.system('curl '+webfile+' -f -o '+regressfile)
        # NOTE: could also use wget
        # os.system('wget '+webfile)

# Now try to access this file
try:
    fr = open(regressfile,'r')
except:
    print "No previous regression results file "+regressfile
    regression['exist'] = False
else:
    u = pickle.Unpickler(fr)
    regression = u.load()
    fr.close()
    print "Regression results filled from "+regressfile
    print "Regression from version "+regression['version']+" on "+regression['date']
    regression['exist'] = True

    prev_results = regression['results']

    prev_compresults = regression['compresults']

#
##########################################################################
# Now print regression results if a previous pickfile can be found
if myflux!=0.0:
    resultlist = ['clean_dirtyimg_max','clean_dirtyimg_rms',
                  'clean_imageall_max','clean_imageall_rms','clean_imageoff_rms',
                  'clean_residall_max','clean_residall_rms','clean_residoff_rms',
                  'clean_modelall_flux','clean_pbmodelall_flux',
                  'sim_comp_flux','flux_area']

    compreslist = ['dirty_image_max','dirty_image_maxposx','dirty_image_maxposy',
                   'clean_image_max','clean_image_maxposx','clean_image_maxposy',
                   'residual_image_max','residual_image_rms',
                   'model_image_sum','pbmodel_image_sum',
                   'sim_comp_flux','sim_comp_posx','sim_comp_posy']
else:
    resultlist = ['clean_dirtyimg_max','clean_dirtyimg_rms',
                  'clean_imageall_max','clean_imageall_rms','clean_imageoff_rms',
                  'clean_residall_max','clean_residall_rms','clean_residoff_rms',
                  'flux_area']
    
    compreslist = ['dirty_image_max','dirty_image_maxposx','dirty_image_maxposy',
                   'clean_image_max','clean_image_maxposx','clean_image_maxposy',
                   'residual_image_max','residual_image_rms']

testresults = {}
testcompresults = {}

if regression['exist']:
    lprint('---', logfile)
    lprint('Regression versus previous values:', logfile)
    lprint('---', logfile)
    lprint("  Regression results filled from "+regressfile, logfile)
    lprint("  Regression from version " + regression['version'] + " on " +
           regression['date'], logfile)
    lprint("  Regression platform " + regression['host'] + " using " +
           regression['aipspath'], logfile)    
    final_status = 'Passed'
else:
    # Just print results
    lprint('---', logfile)
    lprint('Results of imaging:', logfile)
    lprint('---', logfile)
    lprint("  No previous regression file", logfile)

# Do the component results
for comp in compnames:
    lprint("", logfile)
    lprint(" Component " + comp + " :", logfile)

    if regression['exist']:
        # Set up storage for regression results
        testcompresults[comp] = {}
        
    for keys in compreslist:
        res = compresults[comp][keys]
        new_val = res['value']
        
        if regression['exist']:
            testcompresults[comp][keys] = {}
            testres = {}
            testres['value'] = new_val
            testres['op'] = res['op']
            testres['tol'] = res['tol']
            
            if prev_compresults.has_key(comp):
                # This is a known comp
                if prev_compresults[comp].has_key(keys):
                    # This is a known regression key
                    prev = prev_compresults[comp][keys]
                    prev_val = prev['value']
                    if res['op'] == 'divf':
                        new_diff = (new_val - prev_val)/prev_val
                    else:
                        new_diff = new_val - prev_val
                        
                    if abs(new_diff) > res['tol']:
                        new_status = 'Failed'
                    else:
                        new_status = 'Passed'
                        
                    testres['prev'] = prev_val
                    testres['diff'] = new_diff
                    testres['status'] = new_status
                    testres['test'] = 'Last'
                    
                else:
                    # Unknown regression key
                    testres['prev'] = 0.0
                    testres['diff'] = 1.0
                    testres['status'] = 'Missed'
                    testres['test'] = 'none'
                    
            else:
                # Unknown regression comp
                testres['prev'] = 0.0
                testres['diff'] = 1.0
                testres['status'] = 'Unknown'
                testres['test'] = 'none'
                
            # Save results
            testcompresults[comp][keys] = testres
                            
            if testres['status']=='Failed':
                final_status = 'Failed'
                
            # Print results
            lprint('--%30s : %12.6f was %12.6f %4s %12.6f (%6s) %s ' % (res['name'],
                                                                       testres['value'],
                                                                       testres['prev'],
                                                                       testres['op'],
                                                                       testres['diff'],
                                                                       testres['status'],
                                                                       testres['test']),
                   logfile)
        else:
            # Just print results
            lprint('--%30s : %12.6f ' % ( res['name'], res['value'] ), logfile)
            
# Do the overall results
lprint("", logfile)
lprint(" Whole image :", logfile)
for keys in resultlist:
    res = results[keys]
    new_val = res['value']
    if regression['exist']:
        # Set up storage for regression results
        testresults[keys] = {}
        testres = {}
        testres['value'] = new_val
        testres['op'] = res['op']
        testres['tol'] = res['tol']
    
        # Compute regression results
        if prev_results.has_key(keys):
            # This is a known regression key
            prev = prev_results[keys]
            prev_val = prev['value']
            if res['op'] == 'divf':
                new_diff = (new_val - prev_val)/prev_val
            else:
                new_diff = new_val - prev_val
            
            if abs(new_diff)>res['tol']:
                new_status = 'Failed'
            else:
                new_status = 'Passed'
            
            testres['prev'] = prev_val
            testres['diff'] = new_diff
            testres['status'] = new_status
            testres['test'] = 'Last'
        else:
            # Unknown regression key
            testres['prev'] = 0.0
            testres['diff'] = 1.0
            testres['status'] = 'Missed'
            testres['test'] = 'none'

        # Save results
        testresults[keys] = testres

        # Print results
        lprint('--%30s : %12.6f was %12.6f %4s %12.6f (%6s) %s ' % (res['name'],
                                                                    testres['value'],
                                                                    testres['prev'],
                                                                    testres['op'],
                                                                    testres['diff'],
                                                                    testres['status'],
                                                                    testres['test'] ),
               logfile)
        if testres['status']=='Failed':
            final_status = 'Failed'

    else:
        # Just print the current results
        lprint('--%30s : %12.6f ' % ( res['name'], res['value'] ), logfile)
    
# Done
                    
if regression['exist']:
    # Final regression status
    if (final_status == 'Passed'):
        regstate=True
        print >>logfile,'---'
        lprint('Passed Regression test for ' + mydataset, logfile)
        print >>logfile,'---'
        print ''
        print 'Regression PASSED'
        print ''
    else:
        regstate=False
        lprint('----FAILED Regression test for ' + mydataset, logfile)
        
    # Write these regression test results to the dictionary
    new_regression['testresults'] = testresults
    new_regression['testcompresults'] = testcompresults
else:
    regstate=False
    lprint('----Unable to complete regression test for '+ mydataset, logfile)

#
##########################################################################
# Final stats and timing

lprint('', logfile)
lprint('********* Benchmarking *************************', logfile)
lprint('*                                              *', logfile)
lprint('Total wall clock time was: %10.3f ' % total['wall'], logfile)
lprint('Total CPU        time was: %10.3f ' % total['cpu'], logfile)
lprint('Raw processing rate MB/s was: %8.1f ' % total['rate_raw'], logfile)
lprint('MS  processing rate MB/s was: %8.1f ' % total['rate_ms'], logfile)
lprint('', logfile)
lprint('* Breakdown:                                   *', logfile)

for i in range(nstages):
    lprint('* %16s * time was: %10.3f ' % tuple(stages[i]), logfile)

lprint('************************************************', logfile)
if regression['exist']:
    lprint('Previous wall time was %10.3f sec on %s ' %
           (regression['timing']['total']['wall'], regression['host'] ), logfile)
    lprint('Previous CPU  time was %10.3f sec on %s ' %
           (regression['timing']['total']['cpu'], regression['host'] ), logfile)

#
##########################################################################
#
# Save regression results as dictionary using Pickle
#
pickfile = 'out.'+prefix + '.regression.'+datestring+'.pickle'
f = open(pickfile,'w')
p = pickle.Pickler(f)
p.dump(new_regression)
f.close()

print ""
print "Regression result dictionary saved in "+pickfile
print ""
print "Use Pickle to retrieve these"
print ""

# e.g.
# f = open(pickfile)
# u = pickle.Unpickler(f)
# clnmodel = u.load()
# polmodel = u.load()
# f.close()

lprint("", logfile)
lprint("Done with " + mydataset, logfile)

logfile.close()
print "Results are in "+outfile

#
##########################################################################
