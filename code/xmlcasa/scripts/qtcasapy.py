
import os
import sys

sys.path.append('/Library/Frameworks/Python.framework/Versions/2.5/lib/python2.5/site-packages/IPython')
sys.path.append('/opt/casa/darwin/python/2.5')

import IPython
import casac

#
# setup available tools
imager = casac.homefinder.find_home_by_name('imagerHome')
im = imager.create()
calibrater = casac.homefinder.find_home_by_name('calibraterHome')
cb = calibrater.create()
mstool = casac.homefinder.find_home_by_name('msHome')
ms = mstool.create()
tptool = casac.homefinder.find_home_by_name('tableplotHome')
tp = tptool.create()
mptool = casac.homefinder.find_home_by_name('msplotHome')
mp = mptool.create()
pmtool = casac.homefinder.find_home_by_name('plotmsHome')
pm = pmtool.create()
cptool = casac.homefinder.find_home_by_name('calplotHome')
cp = cptool.create()
tbtool = casac.homefinder.find_home_by_name('tableHome')
tb = tbtool.create()
aftool = casac.homefinder.find_home_by_name('autoflagHome')
af = aftool.create()
metool = casac.homefinder.find_home_by_name('measuresHome')
me = metool.create()
qatool = casac.homefinder.find_home_by_name('quantaHome')
qa = qatool.create()
iatool = casac.homefinder.find_home_by_name('imageHome')
ia = iatool.create()
smtool = casac.homefinder.find_home_by_name('simulatorHome')
sm = smtool.create()
cltool = casac.homefinder.find_home_by_name('componentlistHome')
cl = cltool.create()
coordsystool = casac.homefinder.find_home_by_name('coordsysHome')
cs = coordsystool.create()
rgtool = casac.homefinder.find_home_by_name('regionmanagerHome')
rg=rgtool.create()
vftaskhome = casac.homefinder.find_home_by_name('vlafillertaskHome')
vftask = vftaskhome.create()
vlafiller=vftask.fill
vrtool = casac.homefinder.find_home_by_name('viewerHome')
vr=vrtool.create()

#setup logger
loghome = casac.homefinder.find_home_by_name('logsinkHome')
log = loghome.create()
if (os.uname()[0]=='Darwin'):
	from Carbon.Launch import LSFindApplicationForInfo
	from Carbon.CoreFoundation import kCFURLPOSIXPathStyle
	kLSUnknownCreator = '\x00\x00\x00\x00'
	fsRef, cfURL = LSFindApplicationForInfo(kLSUnknownCreator, None, "casalogger.app")
	logger_path = os.path.join(fsRef.as_pathname(), 'Contents', 'MacOS', 'casalogger')
	logpid=os.spawnvp(os.P_NOWAIT, logger_path, [logger_path, 'casapy.log'])
	#logpid=os.system('open -a casalogger.app casapy.log')
elif (os.uname()[0]=='Linux'):
	logpid=os.spawnlp(os.P_NOWAIT,'casalogger','casalogger','casapy.log')
else:
	print 'Unrecognized OS: No logger available'

####################
# Task Interface

# Defaults for all arguments of tasks

accumtime=-1.0
alg='hogbom'
antenna=''
antennaid=-1
append=False
archivefiles=False
asdm=''
bandname='*'
bandtype='B'
baseline=[-1]
blbased=False
solnorm=True
bptable=''
caldescid=-1
calfield=''
caltable=''
calselect=''
calwt=True
cell=[1,1]
channels=[]
chans=[-1]
cleanbox=[]
clipfunction='ABS'
clipcorr='I'
clipminmax=[0.0,0.0]
combinespw=False
complist=''
comptype='P'
constpb=0.4
correlations=''
datacolumn='corrected'
degamp=3
degphase=3
diskwrite=False
doppler='radio'
expr=''
field=''
fieldid=0
field=''
file=''
fitmode='subtract'
fitorder=0
fitsfile=''
fixpar=[]
fluxdensity=-1
fluxtable=''
fontsize=1.
freqdep=False
freqtol=150000.
gain=0.1
gaincurve=True
gainselect=''
gaintable=''
gaintype='G'
highres=''
image=''
imagename=''
imsize=[256,256]
incrtable=''
incremental=False
interactive=False
interp='linear'
iteration=''
lowres=''
mask=['']
maskcenter=1
maskedge=5
mfalg='mfclark'
minpb=0.1
mode='channel'
model=''
msselect=''
multiplot=False
multisource=False
nchan=-1
niter=0
npointaver=10
nxpanel=1
nypanel=1
opacity=True
outputvis=''
overplot=False
phasecenter=0
phaseonly=False
phasewrap=250
pixelorder=True
plotsymbol=','
pointtable=''
refant=-1
reference=[]
refspwmap=[-1]
region=[0.0]
rmode='none'
robust=0.0
scaletype='NONE'
smoothtype='median'
smoothtime=60.0
spwmap=[-1]
spwid=-1
sourcepar=[1,0,0]
subplot=111
solint=0.0
calmode='ap'
splinetime=10800.
spwid=-1
standard='Perley-Taylor 99'
start=0
step=1
stokes='I'
tablein=''
tablename=''
taskname='listobs'
threshold=0.0
timebin='-1s'
timerange=''
title=''
transfer=[]
unflag=False
uvfilter=False
uvfilterbmaj=1.
uvfilterbmin=1.
uvfilterbpa=0.
uvrange=[0]
verbose = False
vis = ''
visnorm=False
vwrpid=9999
weighting='natural'
width=1
windowsize=1.0
writestation=False
writesyscal=False
xaxis=''
xlabels=''
yaxis='amp'
ylabels=''
#76

def go(taskname=None):
	""" Execute taskname: """
        myf=sys._getframe(0).f_globals
        if taskname==None: taskname=myf['taskname']
        myf['taskname']=taskname

	try:
		parameter_checktype(['taskname'],[taskname],str)
	except TypeError, e:
		print "go -- TypeError: ",e
		return

        fulltaskname=taskname+'()'
        exec(fulltaskname)

def selectfield(vis,minstring):
        """Derive the fieldid from  minimum matched string(s): """

        tb.open(vis+'/FIELD')
        fields=tb.getcol('NAME')#get fieldname list
        tb.close()              #close table
        indexlist=list()        #initialize list
	stringlist=list()

        fldlist=minstring.split()#split string into elements
	print 'fldlist is ',fldlist
        for fld in fldlist:     #loop over fields
                _iter=fields.__iter__() #create iterator for fieldnames
                while 1:
                        try:
                                x=_iter.next() # has first value of field name
                        except StopIteration:
                                break
                        #
                        if (x.find(fld)!=-1): 
				indexlist.append(fields.index(x))
				stringlist.append(x)

	print 'Selected fields are: ',stringlist
        return indexlist

def selectantenna(vis,minstring):
        """Derive the antennaid from matched string(s): """

        tb.open(vis+'/ANTENNA')
        ants=tb.getcol('NAME')#get fieldname list
        tb.close()              #close table
        indexlist=list()        #initialize list
	stringlist=list()

        antlist=minstring.split()#split string into elements
        for ant in antlist:     #loop over fields
        	try:
			ind=ants.index(ant)
			indexlist.append(ind)
			stringlist.append(ant)
                except ValueError:
                        pass

	print 'Selected antennas are: ',stringlist
	print 'indexlist: ',indexlist
        return indexlist[0]

def readboxfile(boxfile):
	""" Read a file containing clean boxes (compliant with AIPS BOXFILE)

	Format is:
	#FIELDID BLC-X BLC-Y TRC-X TRC-Y
	0       110   110   150   150 
	or
	0       hh:mm:ss.s dd.mm.ss.s hh:mm:ss.s dd.mm.ss.s

	Note all lines beginning with '#' are ignored.

	"""
	union=[]
	f=open(boxfile)
	while 1:
		try: 
			line=f.readline()
			if (line.find('#')!=0): 
				splitline=line.split('\n')
				splitline2=splitline[0].split()
				if (len(splitline2[1])<6): 
					boxlist=[int(splitline2[1]),int(splitline2[2]),
					int(splitline2[3]),int(splitline2[4])]
				else:
					boxlist=[splitline2[1],splitline2[2],splitline2[3],
 					splitline2[4]]
	
				union.append(boxlist)
	
		except:
			break

	f.close()
	return union


def inp(taskname=None):
	""" Print out current input values for a specified task: """
        myf=sys._getframe(0).f_globals
	if taskname==None: taskname=myf['taskname']
        myf['taskname']=taskname
	if type(taskname)!=str: 
		taskname=taskname.__name__
		myf['taskname']=taskname

        try:
                parameter_checktype(['taskname'],taskname,str)
                parameter_checkmenu('taskname',taskname,['accum','bandpass','blcal','clean','contsub','correct','exportuvfits','feather','flagautocorr','flagdata','flagxy','fluxscale','fringecal','ft','gaincal','imhead','importvla','importasdm','importfits','importuvfits','clearcal','listhistory','listobs','makemask','mosaic','plotants','plotcal','plotxy','pointcal','smooth','setjy','split','uvmodelfit','viewer','browsetable','restore'])
        except TypeError, e:
                print "inp -- TypeError: ", e
                return
        except ValueError, e:
                print "inp -- OptionError: ", e
                return

	if taskname=='accum':
		print ''
		print 'accum -- Accumulate incremental calibration solutions into a cumulative calibration table'
		print ''
                print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
		print 'tablein     = "'+str(tablein)+'"','\t# Input (cumulative) calibration table'
		print 'incrtable   = "'+str(incrtable)+'"','\t# Input incremental calibration table'
                print 'caltable    = "'+str(caltable)+'"','\t# Output (cumulative) calibration table'
		print 'field       = "'+str(field)+'"','\t# List of field names to update in cum. table'
		print 'calfield    = "'+str(calfield)+'"','\t# List of field names in incremental table'
		print 'interp      = "'+str(interp)+'"','\t# Interpolation mode to use on incremental'
		print 'accumtime   = ',accumtime,'\t# Cumulative table timescale when creating'
		print 'spwmap      = ',spwmap,'\t# Spectral windows to apply'

	if taskname=='bandpass':
		print ''
		print 'bandpass -- Calculate a bandpass calibration solution:'
		print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
		print 'caltable    = "'+str(caltable)+'"','\t# Name of output bandpass calibration table'
		print 'mode        = "'+str(mode)+'"','\t# Type of data selection (channel,velocity,none(=all))'
		print 'nchan       = ',nchan,'\t# Number of channels to select (mode=channel)'
		print 'start       = ',start,'\t# Start channel, 0-relative'
		print 'step        = ',step,'\t# Increment between channels/velocity'
		print 'msselect    = "'+str(msselect)+'"','\t# Optional subset of data to select'
		print 'solint      = ',solint,'\t# Solution interval (sec)'
		print 'refant      = ',refant,'\t# Reference Antenna'
		print 'bandtype    = "'+str(bandtype)+'"','\t# Type of bandpass solution (B or BPOLY)'
		print 'append      = ',append,'\t# Append solutions to (existing) table'
		print 'degamp      = ',degamp,'\t# Polynomial degree for amplitude solution'
		print 'degphase    = ',degphase,'\t# Polynomial degree for phase solution'
		print 'visnorm     = ',visnorm,'\t# Normalize data prior to solution'
		print 'solnorm      = ',solnorm,'\t# Normalize result?'
		print 'maskcenter  = ',maskcenter,'\t# Number of channels to avoid in center of band'
		print 'maskedge    = ',maskedge,'\t# Fraction of channels to avoid at each band edge (in %)'
		print 'gaincurve   = ',gaincurve,'\t# Apply VLA antenna gain curve correction'
		print 'opacity     = ',opacity,'\t# Opacity correction to apply (nepers)'
		print 'gaintable   = "'+str(gaintable)+'"','\t# Gain calibration solutions to apply'
		print 'gainselect  = "'+str(gainselect)+'"','\t# Select subset of calibration solutions from gaintable'
		print 'bptable     = "'+str(bptable)+'"','\t# Bandpass calibration solutions to apply'
		print 'pointtable  = "'+str(pointtable)+'"','\t# Pointing calibration solutions to apply'

        if taskname=='blcal':
                print ''
                print 'blcal -- Calculate a baseline-based calibration solution (gain or bandpass):'
                print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
                print 'caltable    = "'+str(caltable)+'"','\t# Name of output Gain calibration table'
		print 'mode        = "'+str(mode)+'"','\t# Type of data selection (channel, velocity, none(=all))'
                print 'nchan       = ',nchan,'\t# Number of channels to select (mode=channel)'
                print 'start       = ',start,'\t# Start channel, 0-relative'
                print 'step        = ',step,'\t# Increment between channels/velocity'
		print 'msselect    = "'+str(msselect)+'"','\t# Optional subset of data to select'
                print 'gaincurve   = ',gaincurve,'\t# Apply VLA antenna gain curve correction'
		print 'opacity     = ',opacity,'\t# Opacity correction to apply (nepers)'
                print 'gaintable   = "'+str(gaintable)+'"','\t# Gain calibration solutions to apply'
                print 'gainselect  = "'+str(gainselect)+'"','\t# Select subset of calibration solutions from gaintable'
                print 'solint      = ',solint,'\t# Solution interval (sec)'
                print 'refant      = ',refant,'\t# Reference Antenna'
		print 'freqdep     = ',freqdep,'\t# Solve for frequency dependent solutions'

	if taskname=='browsetable':
		print ''
		print 'browsetable -- View tabular display of visibility data set (MS)'
		print ''
		print 'tablename   = "'+str(tablename)+'"','\t# Name of table file on disk'

	if taskname=='clean':
		print ''
		print 'clean -- Calculate a deconvolved image with selected clean algorithm:'
		print ''
                print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
                print 'imagename   = "'+str(imagename)+'"','\t# Name of output images: restored=imagename.restored \n\t\t\t residual=imagename.residual, model=imagename.model'
                print 'mode        = "'+str(mode)+'"','\t# Type of selection (mfs, channel, velocity)'
                print 'alg         = "'+str(alg)+'"','\t# Algorithm to use (clark,hogbom,csclean,multiscale)'
                print 'niter       = ',niter,'\t# Number of iterations; set to zero for no CLEANing'
                print 'gain        = ',gain,'\t# Loop gain for CLEANing'
                print 'threshold   = ',threshold,'\t# Flux level to stop CLEANing (mJy)'
                if type(mask)==list: print 'mask        = ',mask,'\t# Name(s) of mask image(s) used in CLEANing'
                if type(mask)==str: print 'mask        = "'+str(mask)+'"','\t# Name(s) of mask image(s) used in CLEANing'
		if type(cleanbox)==list: print 'cleanbox    = ',cleanbox,'\t# clean box regions'
		if type(cleanbox)==str:  print 'cleanbox    = "'+str(cleanbox)+'"','\t# clean box file'
                print 'nchan       = ',nchan,'\t# Number of channels to select (mode=channel)'
                print 'start       = ',start,'\t# Start channel, 0-relative'
                print 'width       = ',width,'\t# Channel width (value > 1 indicates channel averaging)'
                print 'step        = ',step,'\t# Increment between channels/velocity'
                print 'imsize      = ',imsize,'Image size i spatial pixels (x,y)'
                print 'cell        = ',cell,'\t# Cell size in arcseconds (x,y)'
                #if type(stokes)==list: print 'stokes      = ',stokes,'\t# Stokes parameter to image (I,IV,IQU,IQUV)'
                if type(stokes)==str: print 'stokes      = "'+str(stokes)+'"','\t# Stokes parameter to image (I,IV,IQU,IQUV)'
                print 'fieldid     = ',fieldid,'\t# Field index identifier (0-based)'
		print 'field       = "'+str(field)+'"','\t# Field name(s) - minimum matches; use spaces to separate fields'
                print 'spwid       = ',spwid,'\t# Spectral window identifier (0-based)'
		print '--Weighting--'
                print 'weighting   = "'+str(weighting)+'"','# Weighting to apply to visibilities\n\t\t\t (natural, uniform, briggs, radial, superuniform)'
                print 'rmode       = "'+str(rmode)+'"','\t# Robustness mode (for Briggs weighting)'
                print 'robust      = ',robust,'\t# Briggs robustness parameter'
		print 'uvfilter    = ',uvfilter,'\t# Apply additional filtering/uv tapering of the visibilities'
		print 'uvfilterbmaj= ',uvfilterbmaj,'\t# Major axis of filter (arcseconds)'
		print 'uvfilterbmin= ',uvfilterbmin,'\t# Minor axis of filter (arcseconds)'
		print 'uvfilterbpa = ',uvfilterbpa,'\t# Position angle of filter (degrees)'


	if taskname=='clearcal':
		print ''
		print 'clearcal -- Re-initialize calibration for visibility data set:'
		print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
	
	if taskname=='contsub':
		print ''
		print 'contsub -- Continuum fitting and subtraction in the uv plane:'
		print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
		print 'fieldid     = ',fieldid,'\t# Field index identifier (0-based)'
                print 'field       = "'+str(field)+'"','\t# Field name(s) - minimum matches; use spaces to separate fields'
		print 'spwid       = ',spwid,'\t# Spectral window identifier (0-based)'
		print 'channels    = ',channels,'\t# Range of channels to fit'
		print 'solint      = ',solint,'\t# Solution interval (sec)'
		print 'fitorder    = ',fitorder,'\t# Polynomial order for the fit'
		print 'fitmode     = "'+str(fitmode)+'"','# Use of the continuum fit (subtract,replace,model)'

	if taskname=='correct':
		print ''
		print 'correct -- Apply calibration solution(s) to data:'
		print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
		print 'msselect    = "'+str(msselect)+'"','\t# Optional subset of data to select'
		print 'gaincurve   = ',gaincurve,'\t# Apply VLA antenna gain curve correction'
		print 'opacity     = ',opacity,'\t# Opacity correction to apply (nepers)'
		print 'gaintable   = "'+str(gaintable)+'"','\t# Gain calibration solutions to apply'
		print 'gainselect  = "'+str(gainselect)+'"','\t# Select subset of calibration solutions from gaintable'
		print 'bptable     = "'+str(bptable)+'"','\t# Bandpass calibration solutions to apply'
		print 'blbased     = ',blbased,'\t# Apply baseline-based solutions (from blcal)'
		print 'pointtable  = "'+str(pointtable)+'"','\t# Pointing calibration solutions to apply'
		print 'calwt       = ',calwt,'\t# Apply calibration also to the WEIGHTS'

	if taskname=='exportuvfits':
		print ''
		print 'exportuvfits -- Convert a CASA visibility data set (MS) into a UVFITS file'
		print ''
                print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
		print 'fitsfile    = "'+str(fitsfile)+'"','\t# Name of output UVFITS file'
                print 'datacolumn  = "'+str(datacolumn)+'"','\t# Data column (data (raw), corrected, model)'
                print 'fieldid     = ',fieldid,'\t# Field index identifier (0-based)'
                print 'field       = "'+str(field)+'"','\t# Field name(s) - minimum matches; use spaces to separate fields'
                print 'spwid       = ',spwid,'\t# Spectral window identifier (0-based)'
                print 'nchan       = ',nchan,'\t# Number of channels to select (mode=channel)'
                print 'start       = ',start,'\t# Start channel, 0-relative'
                print 'width       = ',width,'\t# Channel width (value > 1 indicates averaging)'
		print 'writesyscal = ',writesyscal,'\t# Write GC and TY tables'
		print 'multisource = ',multisource,'\t# Write in multi-source format'
		print 'combinespw  = ',combinespw,'\t# Handle spectral window as IF'
		print 'writestation= ',writestation,'\t# Write station name instead of antenna name'

	if taskname=='feather':
		print ''
		print 'feather -- Combine and interferometer and single dish image in the Fourier plane:'
		print ''
		print 'imagename   = "'+str(imagename)+'"','\t# Name of output feathered image'
		print 'highres     = "'+str(highres)+'"','\t# Name of high resolution (synthesis) image'
		print 'lowres      = "'+str(lowres)+'"','\t# Name of low resolution (single dish) image'

	if taskname=='flagautocorr':
		print ''
		print 'flagautocorr -- Flag autocorrelations (typically in filled VLA data set):'
		print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'

	if taskname=='flagdata':
		print ''
		print 'flagdata -- Flag specified data selections'
		print ''
                print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
                print 'antennaid   = ',antennaid,'\t# Antenna index identifier; -1 indicates all'
		print 'baseline    = ',baseline,'\t# Baseline identifier [ant1,ant2]'
		print 'chans       = ',chans,'\t# Channel ranges to flag.'
		print 'clipfunction= "'+str(clipfunction)+'"','\t# Clip function'
		print 'clipcorr    = "'+str(clipcorr)+'"','\t# Correlations to clip'
		print 'clipminmax  = ',clipminmax,'\t# Range of data outside of which is flagged'
                print 'fieldid     = ',fieldid,'\t# Field index identifier (0-based)'
                print 'field       = "'+str(field)+'"','\t# Field name(s) - minimum matches; use spaces to separate fields'
                print 'spwid       = ',spwid,'\t# Spectral window identifier (0-based)'
                print 'timerange   = "'+str(timerange)+'"'
		print 'unflag      = ',unflag,'\t# Unflag the data specified'

	if taskname=='flagxy':
		print ''
		print 'flagxy -- Plot points for flagging selected X and Y axes'
		print ''
                print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
                print 'xaxis       = "'+str(xaxis)+'"','\t# Data to plot along x-axis (azimuth, baseline, \n\t\t\t channel, elevation, hourangle, parallacticangle, time, u, uvdist, x)'
                print 'yaxis       = "'+str(yaxis)+'"','\t# Data to plot along y-axis (amp, phase, v)'
                print 'datacolumn  = "'+str(datacolumn)+'"','\t# Data column (data (raw), corrected, model)'
                print 'antennaid   = ',antennaid,'\t# Antenna index identifier; -1 indicates all'
                print 'spwid       = ',spwid,'\t# Spectral window identifier (0-based)'
                print 'fieldid     = ',fieldid,'\t# Field index identifier (0-based)'
                print 'field       = "'+str(field)+'"','\t# Field name(s) - minimum matches; use spaces to separate fields'
                print 'timerange   = "'+str(timerange)+'"'
                print 'correlations= "'+str(correlations)+'"','\t# Correlation(s) to plot (RR,LL,RR LL,XX,YY,XX YY)'
                print 'nchan       = ',nchan,'\t# Number of channels to select (mode=channel)'
                print 'start       = ',start,'\t# Start channel, 0-relative'
                print 'width       = ',width,'\t# Channel width (value > 1 indicates averaging)'
                print 'subplot     = ',subplot,'\t# Panel number on display screen (yxn)'
                print 'overplot    = ',overplot,'\t# Overplot values on current plot (if possible)'
                print 'plotsymbol  = "'+str(plotsymbol)+'"','\t# pylab plot symbol'
                print 'title       = "'+str(title)+'"','\t# Plot title (above plot)'
                print 'xlabels     = "'+str(xlabels)+'"','\t# Label for x-axis'
                print 'ylabels     = "'+str(ylabels)+'"','\t# Label for y-axis'
                print 'iteration   = "'+str(iteration)+'"','\t# Plot each value of this data axis on separate panel \n\t\t\t (antenna, baseline, time)'
                print 'fontsize    = ',fontsize,'\t# Font size for labels'
                print 'windowsize  = ',windowsize,'\t# Window size'
		print 'region      = ',region,'\t# Flagging region specified as [xmin,xmax,ymin,ymax]'
		print 'diskwrite   = ',diskwrite,'\t# Write flags to disk or just display them (True,False)'

	if taskname=='fluxscale':
		print ''
		print 'fluxscale -- Bootstrap the flux density scale from standard calibrators:'
		print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
		print 'caltable    = "'+str(caltable)+'"','\t# Name of input calibration table'
		print 'fluxtable   = "'+str(fluxtable)+'"','\t# Name of output, flux-scaled calibration table'
		print 'reference   = ',reference,'\t# Reference field name (transfer flux scale from)'
		print 'transfer    = ',transfer,'\t# Transfer field name(s)'
		print 'append      = ',append,'\t# Append solutions?'
		print 'refspwmap   = ',refspwmap,'\t# Scale across spectral window boundaries'

        if taskname=='fringecal':
                print ''
                print 'fringecal -- Calculate a baseline-based calibration solution (fringe):'
                print ''
                print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
                print 'caltable    = "'+str(caltable)+'"','\t# Name of output Gain calibration table'
                print 'mode        = "'+str(mode)+'"','\t# Type of data selection (channel, velocity, none)'
                print 'nchan       = ',nchan,'\t# Number of channels to select (mode=channel)'
                print 'start       = ',start,'\t# Start channel, 0-relative'
                print 'step        = ',step,'\t# Increment between channels/velocity'
                print 'msselect    = "'+str(msselect)+'"','\t# Optional subset of data to select'
                print 'gaincurve   = ',gaincurve,'\t# Apply VLA antenna gain curve correction'
		print 'opacity     = ',opacity,'\t# Opacity correction to apply (nepers)'
                print 'gaintable   = "'+str(gaintable)+'"','\t# Gain calibration solutions to apply'
                print 'gainselect  = "'+str(gainselect)+'"','\t# Select subset of calibration solutions from gaintable'
                print 'solint      = ',solint,'\t# Solution interval (sec)'
                print 'refant      = ',refant,'\t# Reference Antenna'

	if taskname=='ft':
		print ''
		print 'ft -- Fourier transform specified model or component list'
		print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
		print 'fieldid     = ',fieldid,'\t# Field index identifier'
                print 'field       = "'+str(field)+'"','\t# Field name(s) - minimum matches; use spaces to separate fields'
		print 'model       = "'+str(model)+'"','\t# Name of input model image'
		print 'complist    = "'+str(complist)+'"','\t# Name of component list (on disk)'
		print 'incremental = "'+str(incremental)+'"','\t# Add to the existing MODEL_DATA column?'

        if taskname=='gaincal':
                print ''
                print 'gaincal -- Solve for gain calibration:'
                print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
                print 'caltable    = "'+str(caltable)+'"','\t# Name of output Gain calibration table'
		print 'mode        = "'+str(mode)+'"','\t# Type of data selection (channel, velocity, none)'
                print 'nchan       = ',nchan,'\t# Number of channels to select (mode=channel)'
                print 'start       = ',start,'\t# Start channel, 0-relative'
                print 'step        = ',step,'\t# Increment between channels/velocity'
		print 'msselect    = "'+str(msselect)+'"','\t# Optional subset of data to select'
		print 'uvrange     = ',uvrange,'\t# Limit uv range (units:kilo lambda)'
                print 'solint      = ',solint,'\t# Solution interval (sec)'
                print 'refant      = ',refant,'\t# Reference Antenna'
		print 'gaintype    = "'+str(gaintype)+'"','\t# G or GSPLINE'
		print 'calmode     = "'+str(calmode)+'"','\t# Type of solution'
		print 'append      = ',append,'\t# Append solutions?'
		print 'splinetime  = ',splinetime,'\t# Spline timescale (sec)'
		print 'npointaver  = ',npointaver,'\t# points to average for phase wrap'
		print 'phasewrap   = ',phasewrap,'\t# Wrap phase when greater than this'
                print 'gaincurve   = ',gaincurve,'\t# Apply VLA antenna gain curve correction'
		print 'opacity     = ',opacity,'\t# Opacity correction to apply (nepers)'
                print 'gaintable   = "'+str(gaintable)+'"','\t# Gain calibration solutions to apply'
		print 'bptable     = "'+str(bptable)+'"','\t# Bandpass calibration solutions to apply'
		print 'pointtable  = "'+str(pointtable)+'"','\t# Pointing calibration solutions to apply'

	if taskname=='imhead':
		print ''
		print 'imhead --  Print image header information to the logger (and return a dictionary of it)'
		print ''
		print 'imagename   = "'+str(imagename)+'"','\t# Name of input image file'
		print 'doppler     = "'+str(doppler)+'"','\t# Display spectral axis with this doppler'
		print 'pixelorder  = ',pixelorder,'\t# List axis descriptors in pixel or world order'
	
	if taskname=='importvla':
		print ''
		print 'importvla -- Convert VLA archive file(s) to a CASA visibility data set (MS):'
		print ''
		print 'archivefiles= ',archivefiles,'\t# Name of input VLA archive file(s)'
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
		print 'bandname    = "'+str(bandname)+'"','\t# VLA frequency band name'
		print 'freqtol     = ',freqtol,'\t# Tolerance in frequency shift in naming windows (Hz)'

	if taskname=='importasdm':
		print ''
		print 'importasdm -- Convert ALMA Science Data Model observation to CASA visibility set (MS):'
		print ''
		print 'asdm        = "'+str(asdm)+'"','\t# Name of ASDM on disk'

	if taskname=='importfits':
		print ''
		print 'importfits -- Convert an input FITS image to CASA image format:'
		print ''
		print 'fitsfile    = "'+str(fitsfile)+'"','\t# Name of input image FITS file'
		print 'vis         = "'+str(vis)+'"','\t# Name of output visibility file (MS)'

	if taskname=='importuvfits':
                print ''
                print 'importuvfits -- Convert a UVFITS file to CASA visibility data set (MS):'
                print ''
                print 'fitsfile    = "'+str(fitsfile)+'"','\t# Name of input UV FITS file'
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'

        if taskname=='invert':
                print ''
                print 'invert -- Calculate the dirty image and dirty beam:'
                print ''
                print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
                print 'imagename   = "'+str(imagename)+'"','\t# Name of output images'
                print 'mode        = "'+str(mode)+'"','\t# Type of selection (mfs, channel, velocity)'
                print 'nchan       = ',nchan,'\t# Number of channels to select (mode=channel)'
                print 'start       = ',start,'\t# Start channel, 0-relative'
                print 'width       = ',width,'\t# Channel width (value > 1 indicates channel averaging)'
                print 'step        = ',step,'\t# Increment between channels/velocity'
                print 'imsize      = ',imsize,'# Image size i spatial pixels (x,y)'
                print 'cell        = ',cell,'\t# Cell size in arcseconds (x,y)'
                if type(stokes)==list: print 'stokes      = ',stokes,'\t# Stokes parameter to image (I,IV,IQU,IQUV)'
                if type(stokes)==str: print 'stokes      = "'+str(stokes)+'"','\t# Stokes parameter to image (I,IV,IQU,IQUV)'

                print 'fieldid     = ',fieldid,'\t# Field index identifier (0-based)'
                print 'field       = "'+str(field)+'"','\t# Field name(s) - minimum matches; use spaces to separate fields'
                print 'spwid       = ',spwid,'\t# Spectral window identifier (0-based)'
                print 'weighting   = "'+str(weighting)+'"','# Weighting to apply to visibilities\n\t\t\t (natural, uniform, briggs, radial, superuniform)'
                print 'rmode        = "'+str(rmode)+'"','\t# Robustness mode (for Briggs weighting)'
                print 'robust      = ',robust,'\t# Briggs robustness parameter'

	if taskname=='listhistory':
		print ''
		print 'listhistory -- List the processing history of a data set:'
		print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'

	if taskname=='listobs':
		print ''
		print 'listobs -- List the observations in a data set:'
		print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
		print 'verbose     = ',verbose,'\t# List each observation in addition to summary'

	if taskname=='makemask':
		print ''
		print 'makemask -- Derive mask image from image or visibility data set'
		print ''
                print 'image       = "'+str(image)+'"'
                print 'interactive = ',interactive
                if type(cleanbox)==list: print 'cleanbox    = ',cleanbox,'\t# clean box regions'
                if type(cleanbox)==str:  print 'cleanbox    = "'+str(cleanbox)+'"','\t# clean box file'
                print 'expr        = "'+str(expr)+'"','\t# mask expression'
                print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
                print 'imagename   = "'+str(imagename)+'"','\t# Name of output images'
                print 'mode        = "'+str(mode)+'"','\t# Type of selection (mfs, channel, velocity)'
                print 'nchan       = ',nchan,'\t# Number of channels to select (mode=channel)'
                print 'start       = ',start,'\t# Start channel, 0-relative'
                print 'width       = ',width,'\t# Channel width (value > 1 indicates channel averaging)'
                print 'step        = ',step,'\t# Increment between channels/velocity'
                print 'imsize      = ',imsize,'# Image size i spatial pixels (x,y)'
                print 'cell        = ',cell,'\t# Cell size in arcseconds (x,y)'
                if type(stokes)==list: print 'stokes      = ',stokes,'\t# Stokes parameter to image (I,IV,IQU,IQUV)'
                if type(stokes)==str: print 'stokes      = "'+str(stokes)+'"','\t# Stokes parameter to image (I,IV,IQU,IQUV)'

                print 'fieldid     = ',fieldid,'\t# Field index identifier (0-based)'
                print 'field       = "'+str(field)+'"','\t# Field name(s) - minimum matches; use spaces to separate fields'
                print 'spwid       = ',spwid,'\t# Spectral window identifier (0-based)'

        if taskname=='mosaic':
                print ''
                print 'mosaic -- Calculate a multi-field deconvolved image with selected clean algorithm:'
                print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
                print 'imagename   = "'+str(imagename)+'"','\t# Name of output images: restored=imagename.restored \n\t\t\t residual=imagename.residual, model=imagename.model'
		print 'mode        = "'+str(mode)+'"','\t# Type of selection (mfs, channel, velocity)'
                print 'mfalg       = "'+str(mfalg)+'"','# Algorithm to use (mfclark,mfhogbom,mfmultiscale)'
                print 'niter       = ',niter,'\t# Number of iterations; set to zero for no CLEANing'
                print 'gain        = ',gain,'\t# Loop gain for CLEANing'
                print 'threshold   = ',threshold,'\t# Flux level to stop CLEANing (mJy)'
                if type(mask)==list: print 'mask        = ',mask,'\t# Name(s) of mask image(s) used in CLEANing'
                if type(mask)==str: print 'mask        = "'+str(mask)+'"','\t# Name(s) of mask image(s) used in CLEANing'
                print 'nchan       = ',nchan,'\t# Number of channels to select (mode=channel)'
                print 'start       = ',start,'\t# Start channel, 0-relative'
                print 'width       = ',width,'\t# Channel width (value > 1 indicates channel averaging)'
                print 'step        = ',step,'\t# Increment between channels/velocity'
                print 'imsize      = ',imsize,'# Image size i spatial pixels (x,y)'
                print 'cell        = ',cell,'\t# Cell size in arcseconds (x,y)'
                if type(stokes)==list: print 'stokes      = ',stokes,'\t# Stokes parameter to image (I,IV,IQU,IQUV)'
                if type(stokes)==str: print 'stokes      = "'+str(stokes)+'"','\t# Stokes parameter to image (I,IV,IQU,IQUV)'
                print 'fieldid     = ',fieldid,'\t# Field index identifier (0-based)'
                print 'field       = "'+str(field)+'"','\t# Field name(s) - minimum matches; use spaces to separate fields'
		print 'phasecenter= ',phasecenter,'\t# Field index id for phase center'
                print 'spwid       = ',spwid,'\t# Spectral window identifier (0-based)'
                print 'weighting   = "'+str(weighting)+'"','# Weighting to apply to visibilities\n\t\t\t (natural, uniform, briggs, radial, superuniform)'
		print 'rmode        = "'+str(rmode)+'"','\t# Robustness mode (for Briggs weighting)'
                print 'robust      = ',robust,'\t# Briggs robustness parameter'
		print 'scaletype   = "'+str(scaletype)+'"','\t# Image plane flux scale type (NONE, SAULT)'
		print 'constpb     = ',constpb,'\t# PB is constant above this level (in SAULT weighting)'
		print 'minpb       = ',minpb,'\t# Minimum PB level to use'
	
	if taskname=='plotants':
		print ''
		print 'plotants -- Plot antenna distribution in local reference frame:'
		print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'

	if taskname=='plotcal':
		print ''
		print 'plotcal -- Plot calibration solutions'
		print ''
		print 'tablein     = "'+str(tablein)+'"','\t# Name of input calibration table'
		print 'yaxis       = "'+str(yaxis)+'"','\t# Data to plot along yaxis (e.g., amp, phase)'
		print 'antennaid   = ',antennaid,'\t# Antenna index identifier(s); -1 indicates all'
		print 'caldescid   = ',caldescid,'\t# Data Description (spw,polarization) id; -1 is all'
		print 'subplot     = ',subplot,'\t# Panel number on display screen (yxn)'
		print 'multiplot   = ',multiplot,'\t# Plot data on separate plots'
		print 'plotsymbol  = "'+str(plotsymbol)+'"','\t# pylab plot symbol'

        if taskname=='plotxy':
                print ''
                print 'plotxy -- Plot points for selected X and Y axes'
                print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
                print 'xaxis       = "'+str(xaxis)+'"','\t# Data to plot along x-axis (azimuth, baseline, \n\t\t\t channel, elevation, hourangle, parallacticangle, time, u, uvdist, x)'
                print 'yaxis       = "'+str(yaxis)+'"','\t# Data to plot along y-axis (amp, phase, v)'
                print 'datacolumn  = "'+str(datacolumn)+'"','\t# Data column (data (raw), corrected, model)'
                print 'antennaid   = ',antennaid,'\t# Antenna index identifier; -1 indicates all'
                print 'spwid       = ',spwid,'\t# Spectral window identifier (0-based)'
                print 'fieldid     = ',fieldid,'\t# Field index identifier (0-based)'
                print 'field       = "'+str(field)+'"','\t# Field name(s) - minimum matches; use spaces to separate fields'
                print 'timerange   = "'+str(timerange)+'"'
                print 'correlations= "'+str(correlations)+'"','\t# Correlation(s) to plot (RR,LL,RR LL,XX,YY,XX YY)'
                print 'nchan       = ',nchan,'\t# Number of channels to select (mode=channel)'
                print 'start       = ',start,'\t# Start channel, 0-relative'
                print 'width       = ',width,'\t# Channel width (value > 1 indicates averaging)'
                print 'subplot     = ',subplot,'\t# Panel number on display screen (yxn)'
                print 'overplot    = ',overplot,'\t# Overplot values on current plot (if possible)'
                print 'plotsymbol  = "'+str(plotsymbol)+'"','\t# pylab plot symbol'
                print 'title       = "'+str(title)+'"','\t# Plot title (above plot)'
                print 'xlabels     = "'+str(xlabels)+'"','\t# Label for x-axis'
                print 'ylabels     = "'+str(ylabels)+'"','\t# Label for y-axis'
                print 'iteration   = "'+str(iteration)+'"','\t# Plot each value of this data axis on separate panel \n\t\t\t (antenna, baseline, time)'
                print 'fontsize    = ',fontsize,'\t# Font size for labels'
                print 'windowsize  = ',windowsize,'\t# Window size'

        if taskname=='pointcal':
                print ''
                print 'pointcal -- Solve for pointing error calibration:'
                print ''
                print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
		print 'model       = "'+str(model)+'"','\t# Name of input model (component list or image)'
                print 'caltable    = "'+str(caltable)+'"','\t# Name of output Gain calibration table'
                print 'mode        = "'+str(mode)+'"','\t# Type of data selection (channel, velocity, none)'
                print 'nchan       = ',nchan,'\t# Number of channels to select (mode=channel)'
                print 'start       = ',start,'\t# Start channel, 0-relative'
                print 'step        = ',step,'\t# Increment between channels/velocity'
                print 'msselect    = "'+str(msselect)+'"','\t# Optional subset of data to select'
                print 'gaincurve   = ',gaincurve,'\t# Apply VLA antenna gain curve correction'
		print 'opacity     = ',opacity,'\t# Opacity correction to apply (nepers)'
                print 'gaintable   = "'+str(gaintable)+'"','\t# Gain calibration solutions to apply'
                print 'gainselect  = "'+str(gainselect)+'"','\t# Select subset of calibration solutions from gaintable'
                print 'solint      = ',solint,'\t# Solution interval (sec)'
                print 'refant      = ',refant,'\t# Reference Antenna'

	if taskname=='restore':
		print ''
		print 'restore -- Restore parameters to system defaults'
		print ''

	if taskname=='setjy':
		print ''
		print 'setjy -- Compute the model visibility for the specified source flux density:'
		print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
		print 'fieldid     = ',fieldid,'\t# Field index identifier (0-based)'
                print 'field       = "'+str(field)+'"','\t# Field name(s) - minimum matches; use spaces to separate fields'
		print 'spwid       = ',spwid,'\t# Spectral window identifier (0-based)'
		print 'fluxdensity = ',fluxdensity,'\t# Specified flux density [I,Q,U,V];-1 will lookup values'
		print 'standard    = "'+str(standard)+'"','\t# Flux density standard'

	if taskname=='smooth':
		print ''
		print 'smooth -- Smooth calibration solution(s) derived from one or more sources'
		print ''
                print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
		print 'tablein     = "'+str(tablein)+'"','\t# Input calibration table'
		print 'caltable    = "'+str(caltable)+'"','\t# Output calibration table (smoothed)'
		print 'field       = "'+str(field)+'"','\t# Optional subset of calibration to select'
		print 'smoothtype  = "'+str(smoothtype)+'"','\t# The smoothing filter to be used'
		print 'smoothtime  = ',smoothtime,'\t# Smoothing filter time (sec)'

	if taskname=='split':
		print ''
		print 'split -- Create a new data set (MS) from a subset of an existing data set:'
		print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
		print 'outputvis   = "'+str(outputvis)+'"','\t# Name of output visibility file (MS)'
		print 'fieldid     = ',fieldid,'\t# Field index identifier (0-based)'
                print 'field       = "'+str(field)+'"','\t# Field name(s) - minimum matches; use spaces to separate fields'
		print 'spwid       = ',spwid,'\t# Spectral window identifier (0-based)'
		print 'nchan       = ',nchan,'\t# Number of channels to select (mode=channel)'
		print 'start       = ',start,'\t# Start channel, 0-relative'
		print 'step        = ',step,'\t# Increment between channels/velocity'
		print 'antenna     = "'+str(antenna)+'"','\t# Antenna selection'
		print 'timebin     = "'+str(timebin)+'"','\t# Value for time averaging'
		print 'timerange   = "'+str(timerange)+'"','\t# Select time range subset of data'
		print 'datacolumn  = "'+str(datacolumn)+'"','\t# Which data column to split out (DATA, \n\t\t\t\t CORRECTED, MODEL)'

	if taskname=='uvmodelfit':
		print ''
		print 'uvmodelfit -- Fit a single component source model to the uv data:'
		print ''
		print 'vis         = "'+str(vis)+'"','\t# Name of input visibility file (MS)'
		print 'niter       = ',niter,'\t# Number of fitting iterations to execute'
		print 'comptype    = "'+str(comptype)+'"','\t# component model type (P=point source,\n\t\t\t G=gaussian, D=elliptical disk)'
		print 'sourcepar   = ',sourcepar,'# Starting guess for component parameters (flux,xoffset,yoffset)'
		print 'fixpar      = ',fixpar,'\t# Control which parameters vary in fit; [True,True,True]=all vary'
		print 'file        = "'+str(file)+'"','\t# Optional output component list table'
		print 'mode        = "'+str(mode)+'"','\t# Type of data selection (channel, velocity, none)'
		print 'nchan       = ',nchan,'\t# Number of channels to select (mode=channel)'
		print 'start       = ',start,'\t# Start channel, 0-relative'
		print 'step        = ',step,'\t# Increment between channels/velocity'
		print 'msselect    = "'+str(msselect)+'"','\t# Optional subset of data to select'

	if taskname=='viewer':
		print ''
		print 'viewer -- View an image or visibility data set:'
		print ''
		print 'imagename   = "'+str(imagename)+'"','\t# Name of file to visualize (MS or image)'

	
def saveinputs(taskname=None,outfile=''):
        """ Save current input values to file on disk for a specified task: 

	taskname -- Name of task
		default: <unset>; example: taskname='bandpass'
		<Options: type tasklist() for the complete list>
	outfile -- Output file for the task inputs
		default: taskname.saved; example: outfile=taskname.orion

	"""
        myf=sys._getframe(0).f_globals
        if taskname==None: taskname=myf['taskname']
        myf['taskname']=taskname

        try:
                parameter_checktype(['taskname','outfile'],[taskname,outfile],[str,str])
                parameter_checkmenu('taskname',taskname,['accum','bandpass','blcal','browsetable','clean','contsub','correct','exportuvfits','feather','flagautocorr','flagdata','flagxy','fluxscale','fringecal','ft','gaincal','imhead','importvla','importasdm','importfits','importuvfits','clearcal','invert','listhistory','listobs','makemask','mosaic','plotants','plotcal','plotxy','pointcal','smooth','setjy','split','uvmodelfit','viewer'])
        except TypeError, e:
                print "saveinputs -- TypeError: ", e
                return
        except ValueError, e:
                print "saveinputs -- OptionError: ", e
                return


	if outfile=='': outfile=taskname+'.saved'
	taskparameterfile=open(outfile,'w')

	if taskname=='accum':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
		print >>taskparameterfile, 'tablein     = "'+str(tablein)+'"'
		print >>taskparameterfile, 'incrtable   = "'+str(incrtable)+'"'
                print >>taskparameterfile, 'caltable    = "'+str(caltable)+'"'
		print >>taskparameterfile, 'field       = "'+str(field)+'"'
		print >>taskparameterfile, 'calfield    = "'+str(calfield)+'"'
		print >>taskparameterfile, 'interp      = "'+str(interp)+'"'
		print >>taskparameterfile, 'accumtime   = ',accumtime
		print >>taskparameterfile, 'spwmap      = ',spwmap

        if taskname=='bandpass':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'caltable    = "'+str(caltable)+'"'
                print >>taskparameterfile, 'mode        = "'+str(mode)+'"'
                print >>taskparameterfile, 'nchan       = ',nchan
                print >>taskparameterfile, 'start       = ',start
                print >>taskparameterfile, 'step        = ',step
                print >>taskparameterfile, 'msselect    = "'+str(msselect)+'"'
                print >>taskparameterfile, 'solint      = ',solint
                print >>taskparameterfile, 'refant      = ',refant
                print >>taskparameterfile, 'bandtype    = "'+str(bandtype)+'"'
                print >>taskparameterfile, 'append      = ',append
                print >>taskparameterfile, 'degamp      = ',degamp
                print >>taskparameterfile, 'degphase    = ',degphase
                print >>taskparameterfile, 'visnorm     = ',visnorm
                print >>taskparameterfile, 'solnorm      = ',solnorm
                print >>taskparameterfile, 'maskcenter  = ',maskcenter
                print >>taskparameterfile, 'maskedge    = ',maskedge
                print >>taskparameterfile, 'gaincurve   = ',gaincurve
                print >>taskparameterfile, 'opacity     = ',opacity
                print >>taskparameterfile, 'gaintable   = "'+str(gaintable)+'"'
                print >>taskparameterfile, 'gainselect  = "'+str(gainselect)+'"'
		print >>taskparameterfile, 'bptable     = "'+str(bptable)+'"'
		print >>taskparameterfile, 'pointtable  = "'+str(pointtable)+'"'

        if taskname=='blcal':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'caltable    = "'+str(caltable)+'"'
                print >>taskparameterfile, 'mode        = "'+str(mode)+'"'
                print >>taskparameterfile, 'nchan       = ',nchan
                print >>taskparameterfile, 'start       = ',start
                print >>taskparameterfile, 'step        = ',step
                print >>taskparameterfile, 'msselect    = "'+str(msselect)+'"'
                print >>taskparameterfile, 'gaincurve   = ',gaincurve
                print >>taskparameterfile, 'opacity     = ',opacity
                print >>taskparameterfile, 'gaintable   = "'+str(gaintable)+'"'
                print >>taskparameterfile, 'gainselect  = "'+str(gainselect)+'"'
                print >>taskparameterfile, 'solint      = ',solint
                print >>taskparameterfile, 'refant      = ',refant
                print >>taskparameterfile, 'freqdep     = ',freqdep

	if taskname=='browsetable':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
		print >>taskparameterfile, 'tablename   = "'+str(tablename)+'"'

        if taskname=='clean':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'imagename   = "'+str(imagename)+'"'
                print >>taskparameterfile, 'mode        = "'+str(mode)+'"'
                print >>taskparameterfile, 'alg         = "'+str(alg)+'"'
                print >>taskparameterfile, 'niter       = ',niter
                print >>taskparameterfile, 'gain        = ',gain
                print >>taskparameterfile, 'threshold   = ',threshold
                if type(mask)==list: print >>taskparameterfile, 'mask        = ',mask
		if type(mask)==str:  print >>taskparameterfile, 'mask        = "'+str(mask)+'"'
                if type(cleanbox)==list: print >>taskparameterfile, 'cleanbox    = ',cleanbox
                if type(cleanbox)==str:  print >>taskparameterfile, 'cleanbox    = "'+str(cleanbox)+'"'
                print >>taskparameterfile, 'nchan       = ',nchan
                print >>taskparameterfile, 'start       = ',start
                print >>taskparameterfile, 'width       = ',width
                print >>taskparameterfile, 'step        = ',step
                print >>taskparameterfile, 'imsize      = ',imsize
                print >>taskparameterfile, 'cell        = ',cell
                if type(stokes)==list: print >>taskparameterfile, 'stokes      = ',stokes
                if type(stokes)==str:  print >>taskparameterfile, 'stokes      = "'+str(stokes)+'"'
                print >>taskparameterfile, 'fieldid     = ',fieldid
		print >>taskparameterfile, 'field       = "'+str(field)+'"'
                print >>taskparameterfile, 'spwid       = ',spwid
                print >>taskparameterfile, 'weighting   = "'+str(weighting)+'"'
                print >>taskparameterfile, 'rmode       = "'+str(rmode)+'"'
                print >>taskparameterfile, 'robust      = ',robust

        if taskname=='clearcal':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'

        if taskname=='contsub':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'fieldid     = ',fieldid
		print >>taskparameterfile, 'field       = "'+str(field)+'"'
                print >>taskparameterfile, 'spwid       = ',spwid
                print >>taskparameterfile, 'channels    = ',channels
                print >>taskparameterfile, 'solint      = ',solint
                print >>taskparameterfile, 'fitorder    = ',fitorder
                print >>taskparameterfile, 'fitmode     = "'+str(fitmode)+'"'

        if taskname=='correct':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'msselect    = "'+str(msselect)+'"'
                print >>taskparameterfile, 'gaincurve   = ',gaincurve
                print >>taskparameterfile, 'opacity     = ',opacity
                print >>taskparameterfile, 'gaintable   = "'+str(gaintable)+'"'
                print >>taskparameterfile, 'gainselect  = "'+str(gainselect)+'"'
                print >>taskparameterfile, 'bptable     = "'+str(bptable)+'"'
                print >>taskparameterfile, 'blbased     = ',blbased
		print >>taskparameterfile, 'pointtable  = "'+str(pointtable)+'"'
		print >>taskparameterfile, 'calwt       = ',calwt

	if taskname=='exportuvfits':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
		print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'fitsfile    = "'+str(fitsfile)+'"'
                print >>taskparameterfile, 'datacolumn  = "'+str(datacolumn)+'"'
                print >>taskparameterfile, 'fieldid     = ',fieldid
		print >>taskparameterfile, 'field       = "'+str(field)+'"'
                print >>taskparameterfile, 'spwid       = ',spwid
                print >>taskparameterfile, 'nchan       = ',nchan
                print >>taskparameterfile, 'start       = ',start
                print >>taskparameterfile, 'width       = ',width
		print >>taskparameterfile, 'writesyscal = ',writesyscal
		print >>taskparameterfile, 'multisource = ',multisource
		print >>taskparameterfile, 'combinespw  = ',combinespw
		print >>taskparameterfile, 'writestation= ',writestation

        if taskname=='feather':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'imagename   = "'+str(imagename)+'"'
                print >>taskparameterfile, 'highres     = "'+str(highres)+'"'
                print >>taskparameterfile, 'lowres      = "'+str(lowres)+'"'

        if taskname=='flagautocorr':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'

        if taskname=='flagdata':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'antennaid   = ',antennaid
                print >>taskparameterfile, 'baseline    = ',baseline
                print >>taskparameterfile, 'chans       = ',chans
                print >>taskparameterfile, 'clipfunction= "'+str(clipfunction)+'"'
                print >>taskparameterfile, 'clipcorr    = "'+str(clipcorr)+'"'
                print >>taskparameterfile, 'clipminmax  = ',clipminmax
                print >>taskparameterfile, 'fieldid     = ',fieldid
		print >>taskparameterfile, 'field       = "'+str(field)+'"'
                print >>taskparameterfile, 'spwid       = ',spwid
                print >>taskparameterfile, 'timerange   = "'+str(timerange)+'"'
                print >>taskparameterfile, 'unflag      = ',unflag


        if taskname=='flagxy':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'xaxis       = "'+str(xaxis)+'"'
                print >>taskparameterfile, 'yaxis       = "'+str(yaxis)+'"'
                print >>taskparameterfile, 'datacolumn  = "'+str(datacolumn)+'"'
                print >>taskparameterfile, 'antennaid   = ',antennaid
                print >>taskparameterfile, 'spwid       = ',spwid
                print >>taskparameterfile, 'fieldid     = ',fieldid
		print >>taskparameterfile, 'field       = "'+str(field)+'"'
                print >>taskparameterfile, 'timerange   = "'+str(timerange)+'"'
                print >>taskparameterfile, 'correlations= "'+str(correlations)+'"'
                print >>taskparameterfile, 'nchan       = ',nchan
                print >>taskparameterfile, 'start       = ',start
                print >>taskparameterfile, 'width       = ',width
                print >>taskparameterfile, 'subplot     = ',subplot
                print >>taskparameterfile, 'overplot    = ',overplot
                print >>taskparameterfile, 'plotsymbol  = "'+str(plotsymbol)+'"'
                print >>taskparameterfile, 'title       = "'+str(title)+'"'
                print >>taskparameterfile, 'xlabels     = "'+str(xlabels)+'"'
                print >>taskparameterfile, 'ylabels     = "'+str(ylabels)+'"'
                print >>taskparameterfile, 'iteration   = "'+str(iteration)+'"'
                print >>taskparameterfile, 'fontsize    = ',fontsize
                print >>taskparameterfile, 'windowsize  = ',windowsize
                print >>taskparameterfile, 'region      = ',region
                print >>taskparameterfile, 'diskwrite   = ',diskwrite

        if taskname=='fluxscale':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'caltable    = "'+str(caltable)+'"'
                print >>taskparameterfile, 'fluxtable   = "'+str(fluxtable)+'"'
                print >>taskparameterfile, 'reference   = ',reference
                print >>taskparameterfile, 'transfer    = ',transfer
		print >>taskparameterfile, 'append      = ',append
		print >>taskparameterfile, 'refspwmap   = ',refspwmap

        if taskname=='fringecal':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'caltable    = "'+str(caltable)+'"'
                print >>taskparameterfile, 'mode        = "'+str(mode)+'"'
                print >>taskparameterfile, 'nchan       = ',nchan
                print >>taskparameterfile, 'start       = ',start
                print >>taskparameterfile, 'step        = ',step
                print >>taskparameterfile, 'msselect    = "'+str(msselect)+'"'
                print >>taskparameterfile, 'gaincurve   = ',gaincurve
                print >>taskparameterfile, 'opacity     = ',opacity
                print >>taskparameterfile, 'gaintable   = "'+str(gaintable)+'"'
                print >>taskparameterfile, 'gainselect  = "'+str(gainselect)+'"'
                print >>taskparameterfile, 'solint      = ',solint
                print >>taskparameterfile, 'refant      = ',refant

        if taskname=='ft':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
		print >>taskparameterfile, 'fieldid     = ',fieldid
		print >>taskparameterfile, 'field       = "'+str(field)+'"'
                print >>taskparameterfile, 'model       = "'+str(model)+'"'
                print >>taskparameterfile, 'complist    = "'+str(complist)+'"'
                print >>taskparameterfile, 'incremental = "'+str(incremental)+'"'

        if taskname=='gaincal':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'caltable    = "'+str(caltable)+'"'
                print >>taskparameterfile, 'mode        = "'+str(mode)+'"'
                print >>taskparameterfile, 'nchan       = ',nchan
                print >>taskparameterfile, 'start       = ',start
                print >>taskparameterfile, 'step        = ',step
                print >>taskparameterfile, 'msselect    = "'+str(msselect)+'"'
		print >>taskparameterfile, 'uvrange     = ',uvrange
                print >>taskparameterfile, 'solint      = ',solint
                print >>taskparameterfile, 'refant      = ',refant
		print >>taskparameterfile, 'gaintype    = "'+str(gaintype)+'"'
		print >>taskparameterfile, 'calmode     = "'+str(calmode)+'"'
		print >>taskparameterfile, 'append      = ',append
		print >>taskparameterfile, 'splinetime  = ',splinetime
		print >>taskparameterfile, 'npointaver  = ',npointaver
		print >>taskparameterfile, 'phasewrap   = ',phasewrap
                print >>taskparameterfile, 'gaincurve   = ',gaincurve
                print >>taskparameterfile, 'opacity     = ',opacity
                print >>taskparameterfile, 'gaintable   = "'+str(gaintable)+'"'
		print >>taskparameterfile, 'bptable     = "'+str(bptable)+'"'
		print >>taskparameterfile, 'pointtable  = "'+str(pointtable)+'"'

        if taskname=='imhead':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'imagename   = "'+str(imagename)+'"'
                print >>taskparameterfile, 'doppler     = "'+str(doppler)+'"'
                print >>taskparameterfile, 'pixelorder  = ',pixelorder

        if taskname=='importvla':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'archivefiles= ',archivefiles
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'bandname    = "'+str(bandname)+'"'
                print >>taskparameterfile, 'freqtol     = ',freqtol

	if taskname=='importasdm':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
		print >>taskparameterfile, 'asdm        = "'+str(asdm)+'"'

        if taskname=='importfits':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'fitsfile    = "'+str(fitsfile)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'

        if taskname=='importuvfits':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'fitsfile    = "'+str(fitsfile)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'

        if taskname=='invert':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'imagename   = "'+str(imagename)+'"'
                print >>taskparameterfile, 'mode        = "'+str(mode)+'"'
                print >>taskparameterfile, 'nchan       = ',nchan
                print >>taskparameterfile, 'start       = ',start
                print >>taskparameterfile, 'width       = ',width
                print >>taskparameterfile, 'step        = ',step
                print >>taskparameterfile, 'imsize      = ',imsize
                print >>taskparameterfile, 'cell        = ',cell
                if type(stokes)==list: print >>taskparameterfile, 'stokes      = ',stokes
                if type(stokes)==str:  print >>taskparameterfile, 'stokes      = "'+str(stokes)+'"'
                print >>taskparameterfile, 'fieldid     = ',fieldid
		print >>taskparameterfile, 'field       = "'+str(field)+'"'
                print >>taskparameterfile, 'spwid       = ',spwid
                print >>taskparameterfile, 'weighting   = "'+str(weighting)+'"'
                print >>taskparameterfile, 'rmode        = "'+str(rmode)+'"'
                print >>taskparameterfile, 'robust      = ',robust

        if taskname=='listhistory':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'

        if taskname=='listobs':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'verbose     = ',verbose

        if taskname=='makemask':
                print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
		print >>taskparameterfile, 'image       = "'+str(image)+'"'
		print >>taskparameterfile, 'interactive = ',interactive
		if type(cleanbox)==list: print >>taskparameterfile,'cleanbox    = ',cleanbox
		if type(cleanbox)==str:  print >>taskparameterfile,'cleanbox    = "'+str(cleanbox)+'"'
		print >>taskparameterfile, 'expr        = "'+str(expr)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'imagename   = "'+str(imagename)+'"'
                print >>taskparameterfile, 'mode        = "'+str(mode)+'"'
                print >>taskparameterfile, 'nchan       = ',nchan
                print >>taskparameterfile, 'start       = ',start
                print >>taskparameterfile, 'width       = ',width
                print >>taskparameterfile, 'step        = ',step
                print >>taskparameterfile, 'imsize      = ',imsize
                print >>taskparameterfile, 'cell        = ',cell
                if type(stokes)==list: print >>taskparameterfile, 'stokes      = ',stokes
                if type(stokes)==str:  print >>taskparameterfile, 'stokes      = "'+str(stokes)+'"'
                print >>taskparameterfile, 'fieldid     = ',fieldid
                print >>taskparameterfile, 'field       = "'+str(field)+'"'
                print >>taskparameterfile, 'spwid       = ',spwid

        if taskname=='mosaic':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'imagename   = "'+str(imagename)+'"'
                print >>taskparameterfile, 'mode        = "'+str(mode)+'"'
                print >>taskparameterfile, 'alg         = "'+str(alg)+'"'
                print >>taskparameterfile, 'niter       = ',niter
                print >>taskparameterfile, 'gain        = ',gain
                print >>taskparameterfile, 'threshold   = ',threshold
                if type(mask)==list: print >>taskparameterfile, 'mask        = ',mask
                if type(mask)==str:  print >>taskparameterfile, 'mask        = "'+str(mask)+'"'
                print >>taskparameterfile, 'nchan       = ',nchan
                print >>taskparameterfile, 'start       = ',start
                print >>taskparameterfile, 'width       = ',width
                print >>taskparameterfile, 'step        = ',step
                print >>taskparameterfile, 'imsize      = ',imsize
                print >>taskparameterfile, 'cell        = ',cell
                if type(stokes)==list: print >>taskparameterfile, 'stokes      = ',stokes
                if type(stokes)==str:  print >>taskparameterfile, 'stokes      = "'+str(stokes)+'"'
                print >>taskparameterfile, 'fieldid     = ',fieldid
		print >>taskparameterfile, 'field       = "'+str(field)+'"'
		print >>taskparameterfile, 'phasecenter=',phasecenter
                print >>taskparameterfile, 'spwid       = ',spwid
                print >>taskparameterfile, 'weighting   = "'+str(weighting)+'"'
                print >>taskparameterfile, 'rmode        = "'+str(rmode)+'"'
                print >>taskparameterfile, 'robust      = ',robust
                print >>taskparameterfile, 'scaletype   = "'+str(scaletype)+'"'
                print >>taskparameterfile, 'constpb     = ',constpb
                print >>taskparameterfile, 'minpb       = ',minpb

        if taskname=='plotants':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'

	if taskname=='plotcal':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
		print >>taskparameterfile, 'tablein     = "'+str(tablein)+'"'
		print >>taskparameterfile, 'yaxis       = "'+str(yaxis)+'"'
		print >>taskparameterfile, 'antennaid   = ',antennaid
		print >>taskparameterfile, 'caldescid   = ',caldescid
		print >>taskparameterfile, 'subplot     = ',subplot
		print >>taskparameterfile, 'multiplot   = ',multiplot
		print >>taskparameterfile, 'plotsymbol  = "'+str(plotsymbol)+'"'

        if taskname=='plotxy':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'xaxis       = "'+str(xaxis)+'"'
                print >>taskparameterfile, 'yaxis       = "'+str(yaxis)+'"'
                print >>taskparameterfile, 'datacolumn  = "'+str(datacolumn)+'"'
                print >>taskparameterfile, 'antennaid   = ',antennaid
                print >>taskparameterfile, 'spwid       = ',spwid
                print >>taskparameterfile, 'fieldid     = ',fieldid
		print >>taskparameterfile, 'field       = "'+str(field)+'"'
                print >>taskparameterfile, 'timerange   = "'+str(timerange)+'"'
                print >>taskparameterfile, 'correlations= "'+str(correlations)+'"'
                print >>taskparameterfile, 'nchan       = ',nchan
                print >>taskparameterfile, 'start       = ',start
                print >>taskparameterfile, 'width       = ',width
                print >>taskparameterfile, 'subplot     = ',subplot
                print >>taskparameterfile, 'overplot    = ',overplot
                print >>taskparameterfile, 'plotsymbol  = "'+str(plotsymbol)+'"'
                print >>taskparameterfile, 'title       = "'+str(title)+'"'
                print >>taskparameterfile, 'xlabels     = "'+str(xlabels)+'"'
                print >>taskparameterfile, 'ylabels     = "'+str(ylabels)+'"'
                print >>taskparameterfile, 'iteration   = "'+str(iteration)+'"'
                print >>taskparameterfile, 'fontsize    = ',fontsize
                print >>taskparameterfile, 'windowsize  = ',windowsize

        if taskname=='pointcal':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
		print >>taskparameterfile, 'model       = "'+str(model)+'"'
                print >>taskparameterfile, 'caltable    = "'+str(caltable)+'"'
                print >>taskparameterfile, 'mode        = "'+str(mode)+'"'
                print >>taskparameterfile, 'nchan       = ',nchan
                print >>taskparameterfile, 'start       = ',start
                print >>taskparameterfile, 'step        = ',step
                print >>taskparameterfile, 'msselect    = "'+str(msselect)+'"'
                print >>taskparameterfile, 'gaincurve   = ',gaincurve
                print >>taskparameterfile, 'opacity     = ',opacity
                print >>taskparameterfile, 'gaintable   = "'+str(gaintable)+'"'
                print >>taskparameterfile, 'gainselect  = "'+str(gainselect)+'"'
                print >>taskparameterfile, 'solint      = ',solint
                print >>taskparameterfile, 'refant      = ',refant

        if taskname=='setjy':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'fieldid     = ',fieldid
		print >>taskparameterfile, 'field       = "'+str(field)+'"'
                print >>taskparameterfile, 'spwid       = ',spwid
                print >>taskparameterfile, 'fluxdensity = ',fluxdensity
                print >>taskparameterfile, 'standard    = "'+str(standard)+'"'

	if taskname=='smooth':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
		print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
		print >>taskparameterfile, 'tablein     = "'+str(tablein)+'"'
		print >>taskparameterfile, 'caltable    = "'+str(caltable)+'"'
		print >>taskparameterfile, 'field       = "'+str(field)+'"'
		print >>taskparameterfile, 'smoothtype  = "'+str(smoothtype)+'"'
		print >>taskparameterfile, 'smoothtime  = ',smoothtime

        if taskname=='split':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'outputvis   = "'+str(outputvis)+'"'
                print >>taskparameterfile, 'fieldid     = ',fieldid
		print >>taskparameterfile, 'field       = "'+str(field)+'"'
                print >>taskparameterfile, 'spwid       = ',spwid
                print >>taskparameterfile, 'nchan       = ',nchan
                print >>taskparameterfile, 'start       = ',start
                print >>taskparameterfile, 'step        = ',step
                print >>taskparameterfile, 'antenna     = "'+str(antenna)+'"'
                print >>taskparameterfile, 'timebin     = "'+str(timebin)+'"'
                print >>taskparameterfile, 'timerange   = "'+str(timerange)+'"'
                print >>taskparameterfile, 'datacolumn  = "'+str(datacolumn)+'"'

        if taskname=='uvmodelfit':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'vis         = "'+str(vis)+'"'
                print >>taskparameterfile, 'niter       = ',niter
                print >>taskparameterfile, 'comptype    = "'+str(comptype)+'"'
                print >>taskparameterfile, 'sourcepar   = ',sourcepar
                print >>taskparameterfile, 'fixpar      = ',fixpar
                print >>taskparameterfile, 'file        = "'+str(file)+'"'
                print >>taskparameterfile, 'mode        = "'+str(mode)+'"'
                print >>taskparameterfile, 'nchan       = ',nchan
                print >>taskparameterfile, 'start       = ',start
                print >>taskparameterfile, 'step        = ',step
                print >>taskparameterfile, 'msselect    = "'+str(msselect)+'"'

        if taskname=='viewer':
		print >>taskparameterfile, 'taskname    = "'+str(taskname)+'"'
                print >>taskparameterfile, 'imagename   = "'+str(imagename)+'"'

	taskparameterfile.close()

		
####################

def exit(): 
	print 'Use CNTRL-D to exit'
	return

import pylab as pl
#from pylab import *

# Provide flexibility for boolean representation in the CASA shell
true  = True
T     = True
false = False
F     = False

# setup available tasks
#
from tasks import *
#import asap as sd
from parameter_dictionary import *
from task_help import *
#from params import *


#        print 'Hint: type "autocall 2" at the CASA prompt to enable autoparenthesis'
#        print '   (this enables invoking functions without enclosing arguments in'
#        print '   parentheses)'
#        print ''


startup()
sys.argv.append( '-pylab')
sys.argv.append( '-prompt_in1')
sys.argv.append( 'CASA <\#>: ')
sys.argv.append( '-autocall')
sys.argv.append( '2')
sys.argv.append( '-colors')
sys.argv.append('LightBG')
sys.argv.append('-logfile')
sys.argv.append('ipython.log')
sys.argv.append('-ipythondir')
sys.argv.append(os.environ['HOME']+'/.casa/ipython')

#ipshell = IPython.Shell.IPShell( argv=['-pylab', '-prompt_in1','CASA <\#>: ','-autocall','2','-colors','LightBG','-logfile','ipython.log'], user_ns=globals() )
#ipshell.mainloop()
IPython.Shell.start(user_ns=globals()).mainloop()

os.kill(logpid,9)
if vwrpid!=9999: os.kill(vwrpid,9)
print "leaving casapy..."
