##########################################################################
#                                                                        #
# Script for LeoRing EVLA Widar-0 Demo Science 20cm Data                 #
# This is a 1 sub-band 256 channel OSRO mode dataset                     #
#                                                                        #
# Created      STM 2010-01-29 (Release R0)    Regressionized Demo Script #
#                                                                        #
##########################################################################
import time
import os
import pickle

#### Function needed to invert phase of visibilties
# do scan-by-scan, spw-by-spw (ddids) visibility phase flip on requested column
#   assumes pylap is pl, and table component available
#   For column, specify 'DATA', 'CORRECTED_DATA' or 'MODEL_DATA'
# Usage: signflip('data.ms','CORRECTED_DATA')
def signflip (vis=None,column=None):

    tb.open(vis,nomodify=False)
    
    scancol=tb.getcol('SCAN_NUMBER')
    minscan=min(scancol)
    maxscan=max(scancol)
    
    ddidcol=tb.getcol('DATA_DESC_ID')
    minddid=min(ddidcol)
    maxddid=max(ddidcol)
    
    for scan in range(minscan,maxscan+1):
        for ddid in range(minddid,maxddid+1):
            st=tb.query('SCAN_NUMBER=='+str(scan)+' && DATA_DESC_ID=='+str(ddid));
            if (st.nrows()>0):
                print 'scan=',scan, ' ddid=',ddid, ' rows=',st.nrows()
                d=tb.getcol(column)
                d=pl.conjugate(d)
                tb.putcol(column,d)
            st.done()
    tb.close()

##########################################################################
#                                                                        #
# Cleanup then run                                                       #
#                                                                        #
##########################################################################
#### Clear previous runs
os.system('rm -rf leo2pt_regression.*')

#-------------------------------------------------------------------------
#Some variables
#-------------------------------------------------------------------------
regressname = 'LeoRing'
prefix = 'leo2pt_regression'
scriptprefix = 'run_leo2pt_regression'
scriptvers = '20100129'

#-------------------------------------------------------------------------
#Open an output logfile
#-------------------------------------------------------------------------
import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())

outfile='out.'+prefix+'.'+datestring+'.log'
logfile=open(outfile,'w')
print 'Opened log file '+outfile

print >>logfile,'Processing SDM data for project '+regressname
print >>logfile,'Script '+scriptvers

# Some date and version info
myvers = casalog.version()
myuser = os.getenv('USER')
myhost = str( os.getenv('HOST') )
mycwd = os.getcwd()
myos = os.uname()

# Print version info to outfile
print >>logfile,'CASA '+myvers+' on host '+myhost+' running '+str(os.uname()[0]+os.uname()[2])
print >>logfile,'at '+datestring
print >>logfile,''
print >>logfile,''

stagename = []
stagetime = []

startTime=time.time()
startProc=time.clock()

prevTime = startTime

#-------------------------------------------------------------------------
# Import, no autocorrs, then flag unaveraged data
#-------------------------------------------------------------------------
# Note: this raw (1s) SDM is available for download from
# ftp://ftp.aoc.nrao.edu/pub/smyers/LeoRing/leo2pt.55183.452640752315.tgz
# or
# http://casa.nrao.edu/Data/EVLA/LeoRing/leo2pt.55183.452640752315.tgz
print '---ImportASDM---'
sdmfile = 'leo2pt.55183.452640752315'
msfile = 'leo2pt_regression.ms'
importasdm(asdm='leo2pt.55183.452640752315',vis='leo2pt_regression.ms',ocorr_mode='co')

# Some timing and file size stuff
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('import')
fillTime = currTime-prevTime
prevTime = currTime
# get dataset sizes in MB (1024x1024 bytes!)
f = os.popen('du -ms '+sdmfile)
fstr = f.readline()
f.close()
sdmsize = float( fstr.split("\t")[0] )
f = os.popen('du -ms '+msfile)
fstr = f.readline()
f.close()
fillsize = float( fstr.split("\t")[0] )

# Now flag on the unaveraged data
#
print '---Flagdata (Off-source timeranges)---'
cmd = ["field='0' timerange='<10:52:02'",
       "field='4' timerange='13:44:38~13:47:57'"]
flagdata(vis='leo2pt_regression.ms',mode='list', inpfile=cmd)
#flagdata(vis='leo2pt_regression.ms',mode='manualflag',field=['0','4'],timerange=['<10:52:02','13:44:38~13:47:57'])
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('flagtime')
prevTime = currTime

# Antenna 5, 15 supposedly bad
print '---Flagdata (bad antennas)---'
cmd = ["antenna='ea05'","antenna='ea15'"]
flagdata(vis='leo2pt_regression.ms',mode='list', inpfile=cmd)
#flagdata(vis='leo2pt_regression.ms',mode='manualflag',antenna=['ea05','ea15'])
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('flagants')
prevTime = currTime

#
print '---Flagdata (quack)---'
flagdata(vis='leo2pt_regression.ms',mode='quack',quackinterval=20.0)
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('quack')
prevTime = currTime

#
print '---Flagdata (clip zeroes)---'
flagdata(vis='leo2pt_regression.ms',mode='clip', clipzeros=True, clipoutside=False, correlation='ABS_RR,LL')
#flagdata(vis='leo2pt_regression.ms',mode='manualflag',clipminmax=[0.0,1.0E-8],clipoutside=F,clipexpr='ABS RR')
#flagdata(vis='leo2pt_regression.ms',mode='manualflag',clipminmax=[0.0,1.0E-8],clipoutside=F,clipexpr='ABS LL')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('clipdata')
prevTime = currTime

# Now split the data averaging to 10s
#
print '---Split (10s average)---'
datafile = 'leo2pt_regression.split10sec.ms'
split(vis='leo2pt_regression.ms',outputvis='leo2pt_regression.split10sec.ms',datacolumn='data',timebin='10s')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('split')
splitTime = currTime-prevTime
prevTime = currTime
# get dataset sizes in MB (1024x1024 bytes!)
f = os.popen('du -ms '+datafile)
fstr = f.readline()
f.close()
splitsize = float( fstr.split("\t")[0] )

# Change from EVLA to VLA
print '---Vishead---'
vishead(vis='leo2pt_regression.split10sec.ms',mode='put',hdkey='telescope',hdvalue='VLA')

# Create and initialize scratch columns
print '---Clearcal---'
clearcal(vis='leo2pt_regression.split10sec.ms')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('clearcal')
clearTime = currTime-prevTime
prevTime = currTime
# get dataset sizes in MB (1024x1024 bytes!)
f = os.popen('du -ms '+datafile)
fstr = f.readline()
f.close()
clearsize = float( fstr.split("\t")[0] )

print '---Listobs---'
listobs('leo2pt_regression.split10sec.ms')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('listobs')
prevTime = currTime

#### first gaincal on bandpass calibrator (temp)
print '---Gaincal 1 (phase-only)---'
gaincal(vis='leo2pt_regression.split10sec.ms',caltable='leo2pt_regression.gcal1',
        field='4',gaintype='G',calmode='p',refant='ea02',
        solint='int',spw='0:118~137')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('gaincal1')
prevTime = currTime

plotcal(caltable='leo2pt_regression.gcal1', yaxis='phase',
        iteration='antenna',subplot=321,antenna='0~5',
        showgui=F,figfile='leo2pt_regression.gcal1.p1.png')

plotcal(caltable='leo2pt_regression.gcal1', yaxis='phase',
        iteration='antenna',subplot=321,antenna='6~11',
        showgui=F,figfile='leo2pt_regression.gcal1.p2.png')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('plot gcal1')
prevTime = currTime

#### bandpass
print '---Bandpass---'
bandpass(vis='leo2pt_regression.split10sec.ms',caltable='leo2pt_regression.bcal',
         field='4',spw='',gaintable='leo2pt_regression.gcal1',
         interp=['nearest'],
         bandtype='B',solint='inf',combine='scan',refant='ea02',solnorm=True)
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('bandpass1')
prevTime = currTime

plotcal(caltable='leo2pt_regression.bcal',xaxis='freq',yaxis='amp',spw='',
        iteration='antenna',subplot=321,antenna='0~5',
        showgui=F,figfile='leo2pt_regression.bcal.p1.png')

plotcal(caltable='leo2pt_regression.bcal',xaxis='freq',yaxis='amp',spw='',
        iteration='antenna',subplot=321,antenna='6~11',
        showgui=F,figfile='leo2pt_regression.bcal.p2.png')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('plot bcal1')
prevTime = currTime

#### final phasecal
print '---Gaincal 2 (phase-only)---'
gaincal(vis='leo2pt_regression.split10sec.ms',caltable='leo2pt_regression.gcal2',
        gaintable='leo2pt_regression.bcal', interp='nearest',
        field='1,4',gaintype='G',calmode='p',refant='ea02',
        solint='int',spw='0:30~224')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('gaincal2')
prevTime = currTime

plotcal(caltable='leo2pt_regression.gcal2', yaxis='phase', 
        iteration='antenna',subplot=321,antenna='0~5',
        showgui=F,figfile='leo2pt_regression.gcal2.p1.png')

plotcal(caltable='leo2pt_regression.gcal2', yaxis='phase', 
        iteration='antenna',subplot=321,antenna='6~11',
        showgui=F,figfile='leo2pt_regression.gcal2.p2.png')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('plot gcal2')
prevTime = currTime

#### final ampcal

print '---Table (change field name)---'
# For this to work had to change the Field name for field 4
# from J1331+3030 to 1331+305
tb.open('leo2pt_regression.split10sec.ms/FIELD',nomodify=False)
st=tb.selectrows(4)
st.putcol('NAME','1331+305')
st.done()
tb.close()

# Calculates 14.7461Jy
print '---Setjy---'
setjy(vis='leo2pt_regression.split10sec.ms',field='4',modimage='3C286_L.im',scalebychan=False,standard='Perley-Taylor 99')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('setjy')
prevTime = currTime

print '---Gaincal 3 (amp-only)---'
gaincal(vis='leo2pt_regression.split10sec.ms',caltable='leo2pt_regression.gcal3',
        gaintable=['leo2pt_regression.bcal','leo2pt_regression.gcal2'],
        interp=['nearest','nearest'],
        field='1,4',gaintype='G',calmode='a',refant='ea02',
        solint='inf',combine='',spw='0:30~224')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('gaincal3')
prevTime = currTime

plotcal(caltable='leo2pt_regression.gcal3', yaxis='amp',
        iteration='antenna',subplot=321,antenna='0~5',
        showgui=F,figfile='leo2pt_regression.gcal3.p1.png')

plotcal(caltable='leo2pt_regression.gcal3', yaxis='amp',
        iteration='antenna',subplot=321,antenna='6~11',
        showgui=F,figfile='leo2pt_regression.gcal3.p2.png')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('plot gcal3')
prevTime = currTime

print '---Fluxscale---'
fluxscale(vis='leo2pt_regression.split10sec.ms',caltable='leo2pt_regression.gcal3',
          fluxtable='leo2pt_regression.fscale',reference='4')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('fluxscale')
prevTime = currTime

plotcal(caltable='leo2pt_regression.fscale', yaxis='amp',
        iteration='antenna',subplot=321,antenna='0~5',
        showgui=F,figfile='leo2pt_regression.fscale.p1.png')

plotcal(caltable='leo2pt_regression.fscale', yaxis='amp',
        iteration='antenna',subplot=321,antenna='6~11',
        showgui=F,figfile='leo2pt_regression.fscale.p2.png')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('plot fluxscale')
prevTime = currTime

#### final applycal
print '---Applycal (field 1)---'
applycal(vis='leo2pt_regression.split10sec.ms',spw='', field='1',
         gaintable=['leo2pt_regression.bcal','leo2pt_regression.gcal2','leo2pt_regression.fscale'],
         interp=['nearest','nearest','nearest'],gainfield=['4','1','1'])
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('applycal 1')
prevTime = currTime

print '---Applycal (field 2,3)---'
# transfer amp+phase from field 1 to field 2,3
applycal(vis='leo2pt_regression.split10sec.ms',spw='', field='2,3',
         gaintable=['leo2pt_regression.bcal','leo2pt_regression.gcal2','leo2pt_regression.fscale'],
         interp=['nearest','linear','linear'],gainfield=['4','1','1'])
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('applycal 2,3')
prevTime = currTime

print '---Applycal (field 4)---'
applycal(vis='leo2pt_regression.split10sec.ms',spw='', field='4',
         gaintable=['leo2pt_regression.bcal','leo2pt_regression.gcal2','leo2pt_regression.fscale'],
         interp=['nearest','nearest','nearest'],gainfield=['4','4','4'])
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('applycal 4')
prevTime = currTime

#### split calibrated data
print '---Split (field 1)---'
split(vis='leo2pt_regression.split10sec.ms',outputvis='leo2pt_regression.field1.ms',
      datacolumn='corrected',field='1')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('split field 1')
prevTime = currTime

print '---Split (field 2)---'
split(vis='leo2pt_regression.split10sec.ms',outputvis='leo2pt_regression.field2.ms',
      datacolumn='corrected',field='2')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('split field 2')
prevTime = currTime

print '---Split (field 3)---'
split(vis='leo2pt_regression.split10sec.ms',outputvis='leo2pt_regression.field3.ms',
      datacolumn='corrected',field='3')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('split field 3')
prevTime = currTime

print '---Split (field 4)---'
split(vis='leo2pt_regression.split10sec.ms',outputvis='leo2pt_regression.field4.ms',
      datacolumn='corrected',field='4')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('split field 4')
prevTime = currTime

print '---Split (all)---'
split(vis='leo2pt_regression.split10sec.ms',outputvis='leo2pt_regression.all.ms',
      datacolumn='corrected',field='1~4')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('split all')
prevTime = currTime

#### flip signs
print '---Signflip (field 1)---'
signflip('leo2pt_regression.field1.ms','DATA')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('signflip 1')
prevTime = currTime

print '---Signflip (field 2)---'
signflip('leo2pt_regression.field2.ms','DATA')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('signflip 2')
prevTime = currTime

print '---Signflip (field 3)---'
signflip('leo2pt_regression.field3.ms','DATA')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('signflip 3')
prevTime = currTime

print '---Signflip (field 4)---'
signflip('leo2pt_regression.field4.ms','DATA')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('signflip 4')
prevTime = currTime

print '---Signflip (all)---'
signflip('leo2pt_regression.all.ms','DATA')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('signflip all')
prevTime = currTime

print '---ExportUVFITS---'
fitsfile = 'leo2pt_regression.all.uvfits'
exportuvfits(vis='leo2pt_regression.all.ms',fitsfile='leo2pt_regression.all.uvfits',
             datacolumn='data',multisource=True,async=False)
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('exportuvfits')
exportTime = currTime-prevTime
prevTime = currTime
# get dataset sizes in MB (1024x1024 bytes!)
f = os.popen('du -ms leo2pt_regression.all.ms')
fstr = f.readline()
f.close()
msallsize = float( fstr.split("\t")[0] )
# get dataset sizes in MB (1024x1024 bytes!)
f = os.popen('du -ms '+fitsfile)
fstr = f.readline()
f.close()
fitssize = float( fstr.split("\t")[0] )
#
##########################################################################
#
# Get MS stats
#
print '---Visstat (J1042+1203)---'
visstat_cal=visstat('leo2pt_regression.field1.ms')
vismean_cal=visstat_cal['DATA']['mean']
print "Found vis mean on J1042+1203 = "+str(vismean_cal)
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('visstat fld 1')
prevTime = currTime

print '---Visstat (1331+305)---'
visstat_3c286=visstat('leo2pt_regression.field4.ms')
vismean_3c286=visstat_3c286['DATA']['mean']
print "Found vis mean on 1331+305 = "+str(vismean_3c286)
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('visstat fld 4')
prevTime = currTime
#
##########################################################################
#First cvel to bary

print '---Cvel (field 2)---'
cvel(vis='leo2pt_regression.field2.ms',
     outputvis ='leo2pt_regression.field2_chanbary.ms',
     mode='channel',nchan=-1,start=0,width=1,
     interpolation='nearest',
     phasecenter='',
     spw='',
     restfreq='1420405751.786Hz',
     outframe='BARY')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('cvel fld 2')
prevTime = currTime

print '---Cvel (field 3)---'
cvel(vis='leo2pt_regression.field3.ms',
     outputvis ='leo2pt_regression.field3_chanbary.ms',
     mode='channel',nchan=-1,start=0,width=1,
     interpolation='nearest',
     phasecenter='',
     spw='',
     restfreq='1420405751.786Hz',
     outframe='BARY')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('cvel fld 3')
prevTime = currTime

#### Combine Leo-1 and Leo-2 fields
#  2    NONE Leo-1        10:47:22.0000 +12.16.38.0000 J2000   2     301290 
#  3    NONE Leo-2        10:46:45.0000 +11.50.38.0000 J2000   3     300960 
print '---Concat field 2 and 3 (Leo-1 and Leo-2)---'
concat(vis = ['leo2pt_regression.field2_chanbary.ms','leo2pt_regression.field3_chanbary.ms'],
       freqtol='150Hz',
       concatvis = 'leo2pt_regression.concat_chanbary.ms')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('concat')
prevTime = currTime

#listobs('leo2pt_regression.concat_chanbary.ms')

#  combine with center at 10:47:03.50 +12.03.38.0000
print '---Clean (dirty mosaic)---'
clean(vis='leo2pt_regression.concat_chanbary.ms',
      imagename='leo2pt_regression.concat_chanbary.dirtymos',
      imsize=[500,500],cell=[12.0,12.0],niter=0,
      mode='channel',nchan=-1,start=0,width=1,
      interpolation='nearest',
      psfmode='hogbom',imagermode='mosaic',ftmachine='ft',
      minpb=0.1,pbcor=False,
      phasecenter='J2000 10h47m03.50s 12d03m38.0s',
      restfreq='1420405751.786Hz',weighting='natural')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('dirty')
prevTime = currTime

#viewer('leo2pt_regression.concat_chanbary.dirtymos.image')
#
# Statistics on dirty image cube
#
print '--ImStat (Dirty cube)--'
dirtystat = imstat('leo2pt_regression.concat_chanbary.dirtymos.image')
print "Found dirty image max = "+str(dirtystat['max'][0])
print "Found dirty image rms = "+str(dirtystat['sigma'][0])

dirtyoff = imstat('leo2pt_regression.concat_chanbary.dirtymos.image',
                 chans='',box='175,190,210,230')
print "Found off-source dirty image rms = "+str(dirtyoff['sigma'][0])

currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('imstat dirty')
prevTime = currTime

#-------------------------------------------
# Clean all at once
print '---Clean (ft mosaic)---'
clean(vis='leo2pt_regression.concat_chanbary.ms',
      imagename='leo2pt_regression.concat_chanbary.cleanmosft',
      imsize=[500,500],cell=[12.0,12.0],
      niter=40000,threshold='20mJy',
      mode='channel',nchan=256,start=0,width=1,
      interpolation='nearest',
      psfmode='hogbom',imagermode='mosaic',ftmachine='ft',
      minpb=0.1,pbcor=False,
      phasecenter='J2000 10h47m03.50s 12d03m38.0s',
      restfreq='1420405751.786Hz',weighting='natural')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('clean')
prevTime = currTime

#Fitted beam used in restoration: 86.4579 by 48.7073 (arcsec) at pa 178.107 (deg)

#viewer('leo2pt_regression.concat_chanbary.cleanmosft.image')
#
# Statistics on clean image cube
#
print '--ImStat (Clean cube)--'
cleanstat = imstat('leo2pt_regression.concat_chanbary.cleanmosft.image')
print "Found clean image max = "+str(cleanstat['max'][0])
print "Found clean image rms = "+str(cleanstat['sigma'][0])

offstat = imstat('leo2pt_regression.concat_chanbary.cleanmosft.image',
                 chans='',box='175,190,210,230')
print "Found off-source clean image rms = "+str(offstat['sigma'][0])

# Statistics on clean model
print '--ImStat (Clean model)--'
modstat = imstat('leo2pt_regression.concat_chanbary.cleanmosft.model')
print "Found total model flux = "+str(modstat['sum'][0])

currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('imstat clean')
prevTime = currTime

#-------------------------------------------
# Clean all at once with uvtaper
#3klambda:
#Fitted beam used in restoration: 137.297 by 58.0881 (arcsec) at pa 158.811 (deg) 

#2.2klambda
#Fitted beam used in restoration: 153.274 by 66.2216 (arcsec) at pa 160.137 (deg) 

#1.6klambda
#Fitted beam used in restoration: 180.444 by 79.4114 (arcsec) at pa 156.049 (deg) 

print '---Clean (ft mosaic tapered)---'
clean(vis='leo2pt_regression.concat_chanbary.ms',
      imagename='leo2pt_regression.concat_chanbary.taper1600.cleanmosft',
      imsize=[400,400],cell=[20.0,20.0],
      niter=40000,threshold='25mJy',
      mode='channel',nchan=256,start=0,width=1,
      interpolation='nearest',
      psfmode='hogbom',imagermode='mosaic',ftmachine='ft',
      minpb=0.2,pbcor=False,cyclefactor=1.0,
      phasecenter='J2000 10h47m03.50s 12d03m38.0s',
      uvtaper=T,outertaper=['1600lambda'],
      restfreq='1420405751.786Hz',weighting='natural')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('clean tapered')
prevTime = currTime

#viewer('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.image')
#
# Statistics on clean image cube
#
print '--ImStat (Clean tapered image)--'
taperstat = imstat('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.image')
print "Found tapered image max = "+str(taperstat['max'][0])
print "Found tapered image rms = "+str(taperstat['sigma'][0])

taperoff = imstat('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.image',
                 chans='',box='175,190,210,230')
print "Found off-source tapered image rms = "+str(taperoff['sigma'][0])

# Statistics on tapered clean model
print '--ImStat (Clean tapered model)--'
tapermod = imstat('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.model')
print "Found total tapered model flux = "+str(tapermod['sum'][0])

currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('imstat tapered')
prevTime = currTime

##########################################################################
#Moment 0
print '---Immoments (0)---'
immoments(imagename='leo2pt_regression.concat_chanbary.taper1600.cleanmosft.image',
          moments=[0],axis='spectral',
          excludepix=[-1000.,0.015],
          outfile='leo2pt_regression.concat_chanbary.taper1600.cleanmosft.moment0')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('moment0')
prevTime = currTime

# Remove the pixel mask (for better display)
ia.open('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.moment0')
ia.maskhandler('delete','mask0')
ia.close()

#Moment 1
print '---Immoments (1)---'
immoments(imagename='leo2pt_regression.concat_chanbary.taper1600.cleanmosft.image',
          moments=[1],axis='spectral',
          excludepix=[-1000.,0.025],
          outfile='leo2pt_regression.concat_chanbary.taper1600.cleanmosft.moment1')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('moment1')
prevTime = currTime

#viewer('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.moment0')

#
# Statistics on moment images
#
print '--ImStat (Moment images)--'
momzerostat=imstat('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.moment0')
try:
    print "Found moment 0 max = "+str(momzerostat['max'][0])
    print "Found moment 0 rms = "+str(momzerostat['rms'][0])
except:
    pass

momonestat=imstat('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.moment1')
try:
    print "Found moment 1 median = "+str(momonestat['median'][0])
except:
    pass

# Get velocities of first and last planes of cube
print '---Immoments (chans 0 and 255)---'
# Do a moment one on channel 0 to check that the indexing is right
try:
    immoments(imagename='leo2pt_regression.concat_chanbary.taper1600.cleanmosft.image',
              moments=[1],includepix=[],
              chans='0',
              outfile='leo2pt_regression.concat_chanbary.taper1600.cleanmosft.plane0.mom1') 
except:
    pass

# Do a moment one on channel 255 to check that the indexing is right
try:
    immoments(imagename='leo2pt_regression.concat_chanbary.taper1600.cleanmosft.image',
	  moments=[1],includepix=[],
	  chans='255',
          outfile='leo2pt_regression.concat_chanbary.taper1600.cleanmosft.plane255.mom1')
except:
    pass

ia.open('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.image')
csys=ia.coordsys()
vel0=0.0
vel255=0.0

try:
    momoneplane0=imstat('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.plane0.mom1')
    print "Found plane 0 moment 1 value = "+str(momoneplane0['median'][0])
except:
    pass


try:
    momoneplane255=imstat('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.plane255.mom1')
    print "Found plane 255 moment 1 value = "+str(momoneplane255['median'][0])
except:
    pass

if(type(momoneplane0)==bool):
    vel0=csys.frequencytovelocity(ia.toworld([0,0,0,0])['numeric'][3])
if(type(momoneplane255)==bool):
    vel255=csys.frequencytovelocity(ia.toworld([0,0,0,255])['numeric'][3])

currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('imstat moments')
prevTime = currTime
#
##########################################################################
#
# Manually correct for mosaic response pattern using .image/.flux images
print '--ImMath (PBcor)--'
immath(outfile='leo2pt_regression.concat_chanbary.taper1600.cleanmosft.pbcor',
       imagename=['leo2pt_regression.concat_chanbary.taper1600.cleanmosft.image',
                  'leo2pt_regression.concat_chanbary.taper1600.cleanmosft.flux'],
       mode='evalexpr',expr='IM0/IM1')

# now pbcor the model, be careful to mask zeros
immath(outfile='leo2pt_regression.concat_chanbary.taper1600.cleanmosft.pbcormod',
       imagename=['leo2pt_regression.concat_chanbary.taper1600.cleanmosft.model',
                  'leo2pt_regression.concat_chanbary.taper1600.cleanmosft.flux'],
       mode='evalexpr',expr='IM0/IM1[IM1!=0.0]')
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('immath pbcor')
prevTime = currTime

#
# Statistics on PBcor image cube
#
print '--ImStat (PBcor cube)--'
pbcorstat = imstat('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.pbcor')
print "Found pbcor image max = "+str(pbcorstat['max'][0])

pbcoroffstat = imstat('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.pbcor',
                      chans='',box='175,190,210,230')
print "Found pbcor off-source rms = "+str(pbcoroffstat['sigma'][0])

#
# Statistics on PBcor model cube
#
print '--ImStat (PBcor model)--'
pbcormodstat = imstat('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.pbcormod')
print "Found total model flux = "+str(pbcormodstat['sum'][0])

currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('imstat pbcor')
prevTime = currTime

#
##########################################################################
#
#DS9 shape files for viewer
#From ALFALFA
#j2000; text 10:46:45.7  11:49:12   # text={M96} color=blue   
#j2000; text 10:47:20.1  12:23:15   # text={205505} color=green  
#j2000; text 10:46:41.3  12:19:37   # text={202027} color=cyan 
#From EVLA
#j2000; text 10:46:05.7  12:08:03   # text={A} color=magenta
#j2000; text 10:47:56.6  12:22:18   # text={B} color=yellow
#j2000; text 10:48:17.2  12:21:48   # text={C} color=red
#j2000; text 10:46:51.4  11:54:53   # text={D} color=blue

#pixels:
#M96       210,156
#205505
#202027    209,247
#A
#B
#C         146,255

#----------------------------
#Make spectrum at M96
print '---Imval (M96)---'
myval = imval('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.image',box='210,156,210,156')
M96_sparr = myval['data']

ia.open('leo2pt_regression.concat_chanbary.taper1600.cleanmosft.image')
chlist = range(256)
fqlist = []
#find ra,dec
mys = ia.toworld([210,156,0,0],'s')
M96_ra = mys['string'][0]
M96_dec = mys['string'][1]
#find spectral coordinates
for i in chlist:
    myw = ia.toworld([210,156,0,i],'n')
    fqlist.append(myw['numeric'][3])

ia.close()
M96_fqlist = fqlist

fqarr = pl.array(fqlist)
#convert to GHz from Hz
M96_fqarr = fqarr*1.0E-9

print 'LeoRing spectrum at M96 : ',M96_ra,' ',M96_dec
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('imval m96')
prevTime = currTime

# Now use matplotlib to plot this spectrum
print '---Matplotlib M96---'
pl.plot(M96_fqarr, M96_sparr, linewidth=1.0)
pl.ylim(-0.04,1.4)
pl.xlim(23.6,26.7)
pl.xlabel('Barycentric Frequency (GHz)')
pl.ylabel('Flux Density (Jy/beam)')
pl.title('EVLA Leo Ring at M96 = 10:46:45.7 11d49m12s')
# Save plot
pl.savefig('leo2pt_regression.M96_spectrum.png',format='png')
pl.savefig('leo2pt_regression.M96_spectrum.eps',dpi=600,format='eps')
pl.savefig('leo2pt_regression.M96_spectrum.pdf',dpi=600,format='pdf')
pl.close()

#Write out spectrum to ascii file:
outfile = 'leo2pt_regression.M96_spectrum.txt'
specfile=open(outfile,'w')

print >>specfile,'#EVLA Leo Ring at M96 = 10:46:45.7 11d49m12s'
print >>specfile,'#256 points: BARYvelocity(km/s) FluxDensity(Jy/beam)'
for i in chlist:
    print >>specfile,'%10.7f %12.6f' % (M96_fqarr[i], M96_sparr[i])

specfile.close()
currTime=time.time()
stagetime.append(currTime-prevTime)
stagename.append('matplotlib m96')
prevTime = currTime

#================================================================================
# Done
endProc=time.clock()
endTime=time.time()

#================================================================================
# Set up the regression values
# Defaults
dirty_image_max = 1.0
dirty_offsrc_rms = 1.0
clean_image_max = 1.0
clean_offsrc_rms = 1.0
taper_image_max = 1.0
taper_offsrc_rms = 1.0
clean_momentzero_max = 1.0
clean_momentzero_rms = 1.0
clean_momentone_median = 0.0
clean_momentone_planezero = 0.0
clean_momentone_planelast = 0.0
vis_mean_cal = 1.0
vis_mean_3c286 = 1.0
model_sum = 1.0
model_clean_sum = 1.0
model_taper_sum = 1.0
model_pbcor_sum = 1.0

# STM 2010-01-29 Version 3.0.1 build 10067
testdate = '2010-01-29 (STM)'
testvers = 'CASA Version 3.0.1 Rev 10067'
dirty_image_max = 0.14097676
dirty_offsrc_rms = 0.0129723958478
clean_image_max = 0.120128452778
clean_offsrc_rms = 0.00723671344488
taper_image_max = 0.177342906594
taper_offsrc_rms = 0.00835683070787
clean_momentzero_max = 28.124853
clean_momentzero_rms = 0.650879
clean_momentone_median = 852.958618
clean_momentone_planezero = 1089.798584
clean_momentone_planelast = 669.364807
vis_mean_cal = 3.565428
vis_mean_3c286 = 14.837324
model_clean_sum = 51.4040303375
model_taper_sum = 51.293914702
model_pbcor_sum = 73.011602

canonical = {}
canonical['exist'] = True

canonical['date'] = testdate
canonical['version'] = testvers
canonical['user'] = 'smyers'
canonical['host'] = 'rishi'
canonical['cwd'] = '/home/rishi2/smyers/3.0.1/LeoRing/'
print "Using internal regression from "+canonical['version']+" on "+canonical['date']

canonical_results = {}
canonical_results['dirty_image_max'] = {}
canonical_results['dirty_image_max']['value'] = dirty_image_max
canonical_results['dirty_image_offsrc_max'] = {}
canonical_results['dirty_image_offsrc_max']['value'] = dirty_offsrc_rms
canonical_results['clean_image_max'] = {}
canonical_results['clean_image_max']['value'] = clean_image_max
canonical_results['clean_image_offsrc_max'] = {}
canonical_results['clean_image_offsrc_max']['value'] = clean_offsrc_rms
canonical_results['taper_image_max'] = {}
canonical_results['taper_image_max']['value'] = taper_image_max
canonical_results['taper_image_offsrc_max'] = {}
canonical_results['taper_image_offsrc_max']['value'] = taper_offsrc_rms
canonical_results['clean_momentzero_max'] = {}
canonical_results['clean_momentzero_max']['value'] = clean_momentzero_max
canonical_results['clean_momentzero_rms'] = {}
canonical_results['clean_momentzero_rms']['value'] = clean_momentzero_rms
canonical_results['clean_momentone_median'] = {}
canonical_results['clean_momentone_median']['value'] = clean_momentone_median
canonical_results['clean_momentone_planezero'] = {}
canonical_results['clean_momentone_planezero']['value'] = clean_momentone_planezero
canonical_results['clean_momentone_planelast'] = {}
canonical_results['clean_momentone_planelast']['value'] = clean_momentone_planelast

canonical_results['vis_mean_cal'] = {}
canonical_results['vis_mean_cal']['value'] = vis_mean_cal
canonical_results['vis_mean_3c286'] = {}
canonical_results['vis_mean_3c286']['value'] = vis_mean_3c286

canonical_results['model_clean_sum'] = {}
canonical_results['model_clean_sum']['value'] = model_clean_sum
canonical_results['model_taper_sum'] = {}
canonical_results['model_taper_sum']['value'] = model_taper_sum
canonical_results['model_pbcor_sum'] = {}
canonical_results['model_pbcor_sum']['value'] = model_pbcor_sum

canonical['results'] = canonical_results

print "Canonical Regression (default) from "+canonical['date']

#
# Try and load previous results from regression file
#
regression = {}
regressfile = scriptprefix + '.pickle'
prev_results = {}

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
    
#
##########################################################################
# Calculate test values
##########################################################################
#
print '--Calculate Results--'
print ''

try:
    dirtymax = dirtystat['max'][0]
except:
    dirtymax = 0.0

try:
    dirtyoffrms = dirtyoff['sigma'][0]
except:
    dirtyoffrms = 0.0

try:
    cleanmax = cleanstat['max'][0]
except:
    cleanmax = 0.0

try:
    cleanoffrms = offstat['sigma'][0]
except:
    cleanoffrms = 0.0

try:
    tapermax = taperstat['max'][0]
except:
    tapermax = 0.0

try:
    taperoffrms = taperoff['sigma'][0]
except:
    taperoffrms = 0.0

try:
    momzero_max = momzerostat['max'][0]
except:
    momzero_max = 0.0

try:
    momzero_rms = momzerostat['rms'][0]
except:
    momzero_rms = 0.0

try:
    momone_median = momonestat['median'][0]
except:
    momone_median = 0.0

try:
    momone_plane0 = momoneplane0['median'][0]
except:
    momone_plane0 = vel0

try:
    momone_plane255 = momoneplane255['median'][0]
except:
    momone_plane255 = vel255

try:
    cleanmodflux = modstat['sum'][0]
except:
    cleanmodflux = 0.0

try:
    tapermodflux = tapermod['sum'][0]
except:
    tapermodflux = 0.0

try:
    pbcormodflux = pbcormodstat['sum'][0]
except:
    pbcormodflux = 0.0

#
# Store results in dictionary
#
new_regression = {}

# Some date and version info
import datetime
datestring=datetime.datetime.isoformat(datetime.datetime.today())

# System info
myvers = casalog.version()
try:
    myuser = os.getlogin()
except:
    myuser = os.getenv('USER')
#myhost = str( os.getenv('HOST') )
myuname = os.uname()
myhost = myuname[1]
myos = myuname[0]+' '+myuname[2]+' '+myuname[4]
mycwd = os.getcwd()
mypath = os.environ.get('CASAPATH')

mydataname = 'LeoRing 18-Dec-2009 EVLA'
mydataset = sdmfile

# Save info in regression dictionary
new_regression['date'] = datestring
new_regression['version'] = myvers
new_regression['user'] = myuser
new_regression['host'] = myhost
new_regression['cwd'] = mycwd
new_regression['os'] = myos
new_regression['uname'] = myuname
new_regression['aipspath'] = mypath

new_regression['dataname'] = mydataname
new_regression['dataset'] = mydataset

# Fill results
# Note that 'op' tells what to do for the diff :
#    'divf' = abs( new - prev )/prev
#    'diff' = new - prev

results = {}

op = 'divf'
tol = 0.08
results['dirty_image_max'] = {}
results['dirty_image_max']['name'] = 'Dirty image max'
results['dirty_image_max']['value'] = dirtymax
results['dirty_image_max']['op'] = op
results['dirty_image_max']['tol'] = tol

results['dirty_image_offsrc_max'] = {}
results['dirty_image_offsrc_max']['name'] = 'Dirty image off-src rms'
results['dirty_image_offsrc_max']['value'] = dirtyoffrms
results['dirty_image_offsrc_max']['op'] = op
results['dirty_image_offsrc_max']['tol'] = tol

results['clean_image_max'] = {}
results['clean_image_max']['name'] = 'Clean image max'
results['clean_image_max']['value'] = cleanmax
results['clean_image_max']['op'] = op
results['clean_image_max']['tol'] = tol

results['clean_image_offsrc_max'] = {}
results['clean_image_offsrc_max']['name'] = 'Clean image off-src rms'
results['clean_image_offsrc_max']['value'] = cleanoffrms
results['clean_image_offsrc_max']['op'] = op
results['clean_image_offsrc_max']['tol'] = tol

results['taper_image_max'] = {}
results['taper_image_max']['name'] = 'Tapered image max'
results['taper_image_max']['value'] = tapermax
results['taper_image_max']['op'] = op
results['taper_image_max']['tol'] = tol

results['taper_image_offsrc_max'] = {}
results['taper_image_offsrc_max']['name'] = 'Tapered image off-src rms'
results['taper_image_offsrc_max']['value'] = taperoffrms
results['taper_image_offsrc_max']['op'] = op
results['taper_image_offsrc_max']['tol'] = tol

results['clean_momentzero_max'] = {}
results['clean_momentzero_max']['name'] = 'Moment 0 image max'
results['clean_momentzero_max']['value'] = momzero_max
results['clean_momentzero_max']['op'] = op
results['clean_momentzero_max']['tol'] = tol

results['clean_momentzero_rms'] = {}
results['clean_momentzero_rms']['name'] = 'Moment 0 image rms'
results['clean_momentzero_rms']['value'] = momzero_rms
results['clean_momentzero_rms']['op'] = op
results['clean_momentzero_rms']['tol'] = tol

op = 'diff'
tol = 0.1
results['clean_momentone_median'] = {}
results['clean_momentone_median']['name'] = 'Moment 1 image median'
results['clean_momentone_median']['value'] = momone_median
results['clean_momentone_median']['op'] = op
results['clean_momentone_median']['tol'] = tol

results['clean_momentone_planezero'] = {}
results['clean_momentone_planezero']['name'] = 'Moment 1 plane 0'
results['clean_momentone_planezero']['value'] = momone_plane0
results['clean_momentone_planezero']['op'] = op
results['clean_momentone_planezero']['tol'] = tol

results['clean_momentone_planelast'] = {}
results['clean_momentone_planelast']['name'] = 'Moment 1 plane 255'
results['clean_momentone_planelast']['value'] = momone_plane255
results['clean_momentone_planelast']['op'] = op
results['clean_momentone_planelast']['tol'] = tol

op = 'divf'
tol = 0.08
results['vis_mean_cal'] = {}
results['vis_mean_cal']['name'] = 'Vis mean of cal'
results['vis_mean_cal']['value'] = vismean_cal
results['vis_mean_cal']['op'] = op
results['vis_mean_cal']['tol'] = tol

results['vis_mean_3c286'] = {}
results['vis_mean_3c286']['name'] = 'Vis mean of 3c286'
results['vis_mean_3c286']['value'] = vismean_3c286
results['vis_mean_3c286']['op'] = op
results['vis_mean_3c286']['tol'] = tol

results['model_clean_sum'] = {}
results['model_clean_sum']['name'] = 'Clean Model image sum'
results['model_clean_sum']['value'] = cleanmodflux
results['model_clean_sum']['op'] = op
results['model_clean_sum']['tol'] = tol

results['model_taper_sum'] = {}
results['model_taper_sum']['name'] = 'Tapered Model image sum'
results['model_taper_sum']['value'] = tapermodflux
results['model_taper_sum']['op'] = op
results['model_taper_sum']['tol'] = tol

results['model_pbcor_sum'] = {}
results['model_pbcor_sum']['name'] = 'PBcor Model image sum'
results['model_pbcor_sum']['value'] = pbcormodflux
results['model_pbcor_sum']['op'] = op
results['model_pbcor_sum']['tol'] = tol

# Now go through and regress
resultlist = ['dirty_image_max','dirty_image_offsrc_max',
              'clean_image_max','clean_image_offsrc_max',
              'taper_image_max','taper_image_offsrc_max',
              'clean_momentzero_max','clean_momentzero_rms','clean_momentone_median',
              'clean_momentone_planezero','clean_momentone_planelast',
              'vis_mean_cal','vis_mean_3c286',
              'model_clean_sum','model_taper_sum','model_pbcor_sum']

for keys in resultlist:
    res = results[keys]
    if prev_results.has_key(keys):
        # This is a known regression
        prev = prev_results[keys]
        new_val = res['value']
        prev_val = prev['value']
        if res['op'] == 'divf':
            new_diff = (new_val - prev_val)/prev_val
        else:
            new_diff = new_val - prev_val

        if abs(new_diff)>res['tol']:
            new_status = 'Failed'
        else:
            new_status = 'Passed'
        
        results[keys]['prev'] = prev_val
        results[keys]['diff'] = new_diff
        results[keys]['status'] = new_status
        results[keys]['test'] = 'Last'
    elif canonical_results.has_key(keys):
        # Go back to canonical values
        prev = canonical_results[keys]
        new_val = res['value']
        prev_val = prev['value']
        if res['op'] == 'divf':
            new_diff = (new_val - prev_val)/prev_val
        else:
            new_diff = new_val - prev_val

        if abs(new_diff)>res['tol']:
            new_status = 'Failed'
        else:
            new_status = 'Passed'
        
        results[keys]['prev'] = prev_val
        results[keys]['diff'] = new_diff
        results[keys]['status'] = new_status
        results[keys]['test'] = 'Canon'
    else:
        # Unknown regression key
        results[keys]['prev'] = 0.0
        results[keys]['diff'] = 1.0
        results[keys]['status'] = 'Missed'
        results[keys]['test'] = 'none'

# Done filling results
new_regression['results'] = results

#
##########################################################################
# Now the timing results
##########################################################################
#
datasize_raw =  sdmsize
datasize_fill = fillsize          # (after fill)
datasize_ms = fillsize

datasize_split = splitsize        # (after split)
datasize_ms = splitsize

datasize_clear = clearsize        # (after clearcal)
datasize_ms = clearsize

datasize_final = msallsize        # (after final split)

datasize_fits = fitssize          # (after export)

new_regression['datasize'] = {}
new_regression['datasize']['raw'] = datasize_raw
new_regression['datasize']['filled'] = datasize_fill
new_regression['datasize']['ms'] = datasize_ms
new_regression['datasize']['split'] = datasize_split
new_regression['datasize']['clear'] = datasize_clear
new_regression['datasize']['final'] = datasize_final
new_regression['datasize']['fits'] = datasize_fits

# Save timing to regression dictionary
timing = {}

total = {}
total['wall'] = (endTime - startTime)
total['cpu'] = (endProc - startProc)
total['rate_raw'] = (datasize_raw/(endTime - startTime))
total['rate_ms'] = (datasize_ms/(endTime - startTime))

nstages = stagetime.__len__()
timing['total'] = total
timing['nstages'] = nstages
timing['stagename'] = stagename
timing['stagetime'] = stagetime

new_regression['timing'] = timing

#
##########################################################################
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

#
##########################################################################
# Printing out results
##########################################################################
#
print ''
print >>logfile,''

# Print out comparison:
res = {}
resultlist = ['dirty_image_max','dirty_image_offsrc_max',
              'clean_image_max','clean_image_offsrc_max',
              'taper_image_max','taper_image_offsrc_max',
              'clean_momentzero_max','clean_momentzero_rms','clean_momentone_median',
              'clean_momentone_planezero','clean_momentone_planelast',
              'vis_mean_cal','vis_mean_3c286',
              'model_clean_sum','model_taper_sum','model_pbcor_sum']

# First versus canonical values
print >>logfile,'---'
print >>logfile,'Regression versus previous values:'
print >>logfile,'---'
print '---'
print 'Regression versus previous values:'
print '---'

if regression['exist']:
    print >>logfile,"  Regression results filled from "+regressfile
    print >>logfile,"  Regression from version "+regression['version']+" on "+regression['date']
    print >>logfile,"  Regression platform "+regression['host']
    
    print "  Regression results filled from "+regressfile
    print "  Regression from version "+regression['version']+" on "+regression['date']
    print "  Regression platform "+regression['host']
    if regression.has_key('aipspath'):
        print >>logfile,"  Regression casapath "+regression['aipspath']
        print "  Regression casapath "+regression['aipspath']
    
else:
    print >>logfile,"  No previous regression file"

print ""
print >>logfile,""

final_status = 'Passed'
for keys in resultlist:
    res = results[keys]
    print '--%30s : %12.6f was %12.6f %4s %12.6f (%6s) %s ' % ( res['name'], res['value'], res['prev'], res['op'], res['diff'], res['status'], res['test'] )
    print >>logfile,'--%30s : %12.6f was %12.6f %4s %12.6f (%6s) %s ' % ( res['name'], res['value'], res['prev'], res['op'], res['diff'], res['status'], res['test'] )
    if res['status']=='Failed':
        final_status = 'Failed'

if (final_status == 'Passed'):
    regstate=True
    print >>logfile,'---'
    print >>logfile,'Passed Regression test for LeoRing'
    print >>logfile,'---'
    print 'Passed Regression test for LeoRing'
else:
    regstate=False
    print >>logfile,'----FAILED Regression test for LeoRing'
    print '----FAILED Regression test for LeoRing'
    
print ''
print 'Total wall clock time was: '+str(endTime - startTime)
print 'Total CPU        time was: '+str(endProc - startProc)
print >>logfile,''
print >>logfile,'Total wall clock time was: '+str(endTime - startTime)
print >>logfile,'Total CPU        time was: '+str(endProc - startProc)

print ''
print 'Canonical wall clock time was: '+str(2354.63)+' on 2010-01-29 (rishi)'
print 'Canonical CPU        time was: '+str(2354.63)+' on 2010-01-29 (rishi)'
print >>logfile,''
print >>logfile,'Canonical wall clock time was: '+str(2354.63)+' on 2010-01-29 (rishi)'
print >>logfile,'Canonical CPU        time was: '+str(2354.63)+' on 2010-01-29 (rishi)'

print ''
print 'SDM datasize (MB)               : '+str(datasize_raw)
print 'Filled MS datasize (MB)         : '+str(datasize_fill)
print >>logfile,''
print >>logfile,'SDM datasize (MB)               : '+str(datasize_raw)
print >>logfile,'Filled MS datasize (MB)         : '+str(datasize_fill)

print 'Split MS datasize (MB)          : '+str(datasize_split)
print >>logfile,'Split MS datasize (MB)          : '+str(datasize_split)

print 'Clearcal MS datasize (MB)       : '+str(datasize_clear)
print >>logfile,'Clearcal MS datasize (MB)       : '+str(datasize_clear)

print 'Final MS datasize (MB)          : '+str(datasize_final)
print >>logfile,'Clearcal MS datasize (MB)       : '+str(datasize_final)

print 'Export UVFITS datasize (MB)     : '+str(datasize_fits)
print >>logfile,'Export UVFITS datasize (MB)     : '+str(datasize_fits)

print 'Reported sizes are in MB from "du -ms" (1024x1024 bytes)'
print >>logfile,'Reported sizes are in MB from "du -ms" (1024x1024 bytes)'

print ''
print >>logfile,''

print 'Net SDMtoMS filling I/O rate MB/s   : '+str((datasize_raw+datasize_fill)/fillTime)
print >>logfile,'Net SDMtoMS filling I/O rate MB/s   : '+str((datasize_raw+datasize_fill)/fillTime)

print 'Net Splitting I/O rate MB/s was     : '+str((datasize_fill+datasize_split)/splitTime)
print >>logfile,'Net Splitting I/O rate MB/s was     : '+str((datasize_fill+datasize_split)/splitTime)

print 'Net Clearcal I/O rate MB/s was      : '+str((datasize_ms)/clearTime)
print >>logfile,'Net Clearcal I/O rate MB/s was      : '+str((datasize_ms)/clearTime)

print 'Net MStoUVFITS export I/O rate MB/s : '+str((datasize_final+datasize_fits)/exportTime)
print >>logfile,'Net MStoUVFITS export I/O rate MB/s : '+str((datasize_final+datasize_fits)/exportTime)
print ''
print >>logfile,''
print 'Net SDM processing rate MB/s        : '+str(datasize_raw/(endTime - startTime))
print >>logfile,'Net SDM processing rate MB/s        : '+str(datasize_raw/(endTime - startTime))
print 'Net MS processing rate MB/s         : '+str(datasize_ms/(endTime - startTime))
print >>logfile,'Net MS processing rate MB/s         : '+str(datasize_ms/(endTime - startTime))

print '* Breakdown:                               *'
print >>logfile,'* Breakdown:                               *'

for i in range(nstages):
    print '* %40s * time was: %10.3f ' % (stagename[i],stagetime[i])
    print >>logfile,'* %40s * time was: %10.3f ' % (stagename[i],stagetime[i])

logfile.close()

print "Done with regression for "+mydataset
# End of script
#
#================================================================================
#MeasurementSet Name:  /home/rishi/smyers/LeoRing/leo2pt.55183.452640752315.ms
#MS Version 2
#================================================================================
#   Observer: leo2pt     Project: T.B.D.  
#Observation: EVLA
#Data records: 745470       Total integration time = 11294 seconds
#   Observed from   18-Dec-2009/10:51:51.5   to   18-Dec-2009/14:00:05.5 (UTC)
#
#   ObservationID = 0         ArrayID = 0
#  Date        Timerange (UTC)          Scan  FldId FieldName    nVis   Int(s)   SpwIds
#  18-Dec-2009/10:51:51.5 - 10:53:47.5     1      0 J1042+1203   7722   1        [0]
#              10:53:48.5 - 10:56:53.5     2      1 J1042+1203   12276  1        [0]
#              10:56:54.5 - 11:06:59.5     3      2 Leo-1        39996  1        [0]
#              11:07:00.5 - 11:17:05.5     4      3 Leo-2        39996  1        [0]
#              11:17:06.5 - 11:22:12.5     5      2 Leo-1        20262  1        [0]
#              11:22:13.5 - 11:27:18.5     6      3 Leo-2        20196  1        [0]
#              11:27:19.5 - 11:30:26.5     7      1 J1042+1203   12408  1        [0]
#              11:30:27.5 - 11:40:32.5     8      2 Leo-1        39996  1        [0]
#              11:40:33.5 - 11:50:38.5     9      3 Leo-2        39996  1        [0]
#              11:50:39.5 - 11:55:44.5    10      2 Leo-1        20196  1        [0]
#              11:55:45.5 - 12:00:51.5    11      3 Leo-2        20262  1        [0]
#              12:00:52.5 - 12:03:58.5    12      1 J1042+1203   12342  1        [0]
#              12:03:59.5 - 12:14:05.5    13      2 Leo-1        40062  1        [0]
#              12:14:06.5 - 12:24:10.5    14      3 Leo-2        39930  1        [0]
#              12:24:11.5 - 12:29:17.5    15      2 Leo-1        20262  1        [0]
#              12:29:18.5 - 12:34:23.5    16      3 Leo-2        20196  1        [0]
#              12:34:24.5 - 12:37:31.5    17      1 J1042+1203   12408  1        [0]
#              12:37:32.5 - 12:47:37.5    18      2 Leo-1        39996  1        [0]
#              12:47:38.5 - 12:57:43.5    19      3 Leo-2        39996  1        [0]
#              12:57:44.5 - 13:02:50.5    20      2 Leo-1        20262  1        [0]
#              13:02:51.5 - 13:07:56.5    21      3 Leo-2        20196  1        [0]
#              13:07:57.5 - 13:11:04.5    22      1 J1042+1203   12408  1        [0]
#              13:11:05.5 - 13:21:10.5    23      2 Leo-1        39996  1        [0]
#              13:21:11.5 - 13:31:16.5    24      3 Leo-2        39996  1        [0]
#              13:31:17.5 - 13:36:23.5    25      2 Leo-1        20262  1        [0]
#              13:36:24.5 - 13:41:29.5    26      3 Leo-2        20196  1        [0]
#              13:41:30.5 - 13:44:37.5    27      1 J1042+1203   12408  1        [0]
#              13:44:38.5 - 14:00:05.5    28      4 J1331+3030   61248  1        [0]
#           (nVis = Total number of time/baseline visibilities per scan) 
#Fields: 5
#  ID   Code Name         RA            Decl           Epoch   SrcId nVis   
#  0    NONE J1042+1203   10:42:44.6052 +12.03.31.2641 J2000   0     7722   
#  1    D    J1042+1203   10:42:44.6052 +12.03.31.2641 J2000   1     74250  
#  2    NONE Leo-1        10:47:22.0000 +12.16.38.0000 J2000   2     301290 
#  3    NONE Leo-2        10:46:45.0000 +11.50.38.0000 J2000   3     300960 
#  4    K    J1331+3030   13:31:08.2880 +30.30.32.9589 J2000   4     61248  
#   (nVis = Total number of time/baseline visibilities per field) 
#Spectral Windows:  (1 unique spectral windows and 1 unique polarization setups)
#  SpwID  #Chans Frame Ch1(MHz)    ChanWid(kHz)TotBW(kHz)  Ref(MHz)    Corrs   
#  0         256 TOPO  1415.3756   7.8125      2000        1415.3756   RR  LL  
#Sources: 6
#  ID   Name         SpwId RestFreq(MHz)  SysVel(km/s) 
#  0    J1042+1203   0     -              -            
#  1    J1042+1203   0     -              -            
#  2    Leo-1        0     -              -            
#  3    Leo-2        0     -              -            
#  4    J1331+3030   0     -              -            
#  5    J1331+3030   0     -              -            
#Antennas: 12:
#  ID   Name  Station   Diam.    Long.         Lat.         
#  0    ea02  E02       25.0 m   -107.37.04.4  +33.54.01.1  
#  1    ea03  E09       25.0 m   -107.36.45.1  +33.53.53.6  
#  2    ea04  W01       25.0 m   -107.37.05.9  +33.54.00.5  
#  3    ea05  W08       25.0 m   -107.37.21.6  +33.53.53.0  
#  4    ea08  N01       25.0 m   -107.37.06.0  +33.54.01.8  
#  5    ea09  E06       25.0 m   -107.36.55.6  +33.53.57.7  
#  6    ea15  W06       25.0 m   -107.37.15.6  +33.53.56.4  
#  7    ea19  W04       25.0 m   -107.37.10.8  +33.53.59.1  
#  8    ea24  W05       25.0 m   -107.37.13.0  +33.53.57.8  
#  9    ea25  N02       25.0 m   -107.37.06.2  +33.54.03.5  
#  10   ea27  E03       25.0 m   -107.37.02.8  +33.54.00.5  
#  11   ea28  N08       25.0 m   -107.37.07.5  +33.54.15.8  
