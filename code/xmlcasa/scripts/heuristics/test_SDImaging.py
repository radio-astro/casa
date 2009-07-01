# sample test script for SDImaging 
# assumes the parameters and results
# from SDpipeline run already exist
# last modified: 2008-09-19 TT
#########################################
import SDBookKeeper as sdb
import SDImaging as  SDI
import re

# example paramters:
# sdMS = 'M16-CO43mod.ASAP.baseline.MS'
# outImage = 'M16-CO43mod.image1'
# mapCenter = 'J2000 18:18:52 -13.49.40'
# nchan = 40
# startchan = 1800
# chanstep = 2
# field = 0
# ngridx=24
# ngridy=24
# cellx = '7arcsec'
# celly = '7arcsec'

#filename='M16-CO43mod.ASAP'
filename='G327-C18O32mod.ASAP'
baselinedData=filename+'.baseline.MS'
logfile=filename+'.logs/PIPELINE.log'

lgf=open(logfile)
pat=re.compile('^radius=')
pat2=re.compile('^iteration=')
matcnt=0
while lgf:
    line=lgf.readline()
    if re.match(pat,line):
        radiusstr=line.split('=')[1].rstrip('[deg]\n')
        matcnt+=1
    elif re.match(pat2,line):
        iteration=line.split('=')[1]
        matcnt+=1
    if matcnt == 2:
        break

radius=float(radiusstr)
bk=sdb.SDbookkeeper(filename+'.History')
# Lines info is item number 5, 11, ...
indx=max(range(5,6*(int(iteration)+1),6))
Lines=bk.read(indx,[],'Lines')
Lines=Lines[0]
spacing=radius / 3.0 * 2.0
# singleimage =True to make a single data cube for all the lines
# willl use max channel width and ignores NChannelMap
singleimage=False
#singleimage=True

(mapCenter, ngridx, ngridy, cellsize, startchans, nchans, chansteps)=SDI.getImParams(baselinedData, Lines, radius, spacing, singleimage)

cellx=cellsize
celly=cellsize
outImagename=None
field=0
spw=0
#moments=[0,1]
moments=[0]

if singleimage is True:
    outImagename=filename+'.image'
    SDI.MakeImage(baselinedData, outImagename, ngridx, ngridy, cellx, celly, mapCenter, nchans, startchans, chansteps,field, spw, moments, showImage=False)
else:
    SDI.MakeImages(baselinedData, outImagename, ngridx, ngridy, cellx, celly, mapCenter, nchans, startchans, chansteps,field, spw, moments, showImage=False)
