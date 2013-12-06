#############################################################################
# $Id:$
# Test Name:                                                                #
# ngc3256-analysis-regression.py                                            #
# An ALMA Science Verification Data Analysis Regression                     #
# using observations of NGC3256 from April 2011                             # 
#                                                                           # 
# Rationale for Inclusion:                                                  #
#    Need tests of ALMA analysis chain with selfcal                         #
#                                                                           # 
# Input data:                                                               #
#     six MSs                                                               #
#                                                                           #
#############################################################################


step_title = { 0 : 'Data preparation', 
               1 : 'Generate caltables',
	       2 : 'Apriori flagging',
	       3 : 'Delay cal for antenna DV07',
               4 : 'Apply wvr and delay cal tables, split out corrected data',
               5 : 'Apply tsys correction',
               6 : 'Concatenate',
               7 : 'More flagging and fixplanets on Titan',
               8 : 'Fast phase-only gaincal on the bandpass calibrator',
               9 : 'Bandpass calibration',
               10 : 'Set fluxscale on Titan',
               11 : 'Slow amplitude and phase gaincal',
               12 : 'Adjust absolute flux scale',
               13 : 'Apply bandpass and gain calibration',
               14 : 'Final flagging',
               15 : 'Final calibration',
               16 : 'Image the phase calibrator',
               17 : 'Apply calibration to flux calibrator',
               18 : 'Image flux calibrator',
               19 : 'Split out the calibrated target data',
               20 : 'Image the NGC3256 continuum',
               21 : 'Slow phase selfcal',
               22 : 'Image the NGC3256 continuum again after selfcal',
               23 : 'Determine and subtract continuum',
               24 : 'Clean the NGC3256 CO line cube',
               25 : 'Evaluate the NGC3256 CO line cube',
               26 : 'Clean the NGC3256 CNhi line cube',
               27 : 'Evaluate the NGC3256 CNhi line cube',
               28 : 'Clean the NGC3256 CNlo line cube',
               29 : 'Evaluate the NGC3256 CNlo line cube',
               30 : 'Verify regression results'
               }

# global defs
basename=['uid___A002_X1d54a1_X5','uid___A002_X1d54a1_X174','uid___A002_X1d54a1_X2e3',
          'uid___A002_X1d5a20_X5','uid___A002_X1d5a20_X174','uid___A002_X1d5a20_X330']
makeplots=True
therefant = 'DV04'

# regression results storage
rmspcal = 0
peakpcal = 0
rmstitan = 0
peaktitan = 0
rmscont = 0
peakcont = 0
rmscontsc = 0
peakcontsc = 0

#############################

# Some infrastructure to make repeating individual parts
#   of this workflow more convenient.

thesteps = []

try:
    print 'List of steps to be executed ...', mysteps
    thesteps = mysteps
except:
    print 'global variable mysteps not set.'
if (thesteps==[]):
    thesteps = range(0,len(step_title))
    print 'mysteps empty. Executing all steps: ', thesteps

# The Python variable 'mysteps' will control which steps
# are executed when you start the script using
#   execfile('alma-m100-analysis-regression.py')
# e.g. setting
#   mysteps = [2,3,4]
# before starting the script will make the script execute
# only steps 2, 3, and 4
# Setting mysteps = [] will make it execute all steps.

totaltime = 0
inittime = time.time()
ttime = inittime
steptime = []

def timing():
    global totaltime
    global inittime
    global ttime
    global steptime
    global step_title
    global mystep
    global thesteps
    thetime = time.time()
    dtime = thetime - ttime
    steptime.append(dtime)
    totaltime += dtime
    ttime = thetime
    casalog.origin('TIMING')
    casalog.post( 'Step '+str(mystep)+': '+step_title[mystep], 'WARN')
    casalog.post( 'Time now: '+str(ttime), 'WARN')
    casalog.post( 'Time used this step: '+str(dtime), 'WARN')
    casalog.post( 'Total time used so far: ' + str(totaltime), 'WARN')
    casalog.post( 'Step  Time used (s)     Fraction of total time (percent) [description]', 'WARN')
    for i in range(0, len(steptime)):
        casalog.post( '  '+str(thesteps[i])+'   '+str(steptime[i])+'  '+str(steptime[i]/totaltime*100.)
                      +' ['+step_title[thesteps[i]]+']', 'WARN')

# default tarfile name containing the input MSs        
mytarfile_name = 'NGC3256_Band3_UnCalibratedMSandTablesForReduction.tgz'
# get the dataset name from the wrapper if possible
mydict = locals()
if mydict.has_key("tarfile_name"):
    if(mytarfile_name != mydict["tarfile_name"]):
        raise Exception, 'Wrong input file 1'

# data preparation
mystep = 0
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    if not os.path.exists(mytarfile_name):
        raise Exception, 'Cannot find input file '+mytarfile_name
    for name in basename:
        os.system('rm -rf '+name+'.ms')

    os.system('tar xvzf '+mytarfile_name)
    for name in basename:
        os.system('mv NGC3256_Band3_UnCalibratedMSandTablesForReduction/'+name+'.ms .')

    timing()

# generate caltables
mystep = 1
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        flagmanager(vis = name+'.ms', mode = 'save', versionname = 'Original')
        os.system('rm -rf cal-tsys_'+name+'.calnew cal-*'+name+'.Wnew')

        gencal(vis=name+'.ms',
               caltype='tsys',
               caltable='cal-tsys_'+name+'.calnew')

	wvrgcal(vis=name+'.ms', caltable='cal-'+name+'.Wnew',  
                toffset=-1, segsource=True,
		tie=["Titan,1037-295,NGC3256"], statsource="1037-295",
                smooth='2.88s')

    if makeplots:
        for spw in ['1','3','5','7']:
            for name in basename:
                plotbandpass(caltable='cal-tsys_'+name+'.calnew', xaxis='freq', yaxis='amp',
                             spw=spw, overlay='time', # also try overlay='antenna'
                             plotrange=[0, 0, 40, 180],
                             figfile='cal-tsys_per_spw_'+spw+'_'+name+'.png', interactive=False)

    timing()

# Apriori flagging
mystep = 2
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        flagmanager(vis = name+'.ms', mode = 'restore', versionname = 'Original')

	flagdata(vis=name+'.ms', flagbackup = F, mode = 'shadow')
	flagdata(vis=name+'.ms',mode='manual', autocorr=True)
        flagdata(vis=name+'.ms', mode='manual', flagbackup = F, intent='*POINTING*')
        flagdata(vis=name+'.ms', mode='manual', flagbackup = F, intent='*ATMOSPHERE*')

        flagmanager(vis = name+'.ms', mode = 'save', versionname = 'Apriori')

    timing()

# Delay Correction for Antenna DV07
mystep = 3
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for i in range(3): # loop over the first three ms's
	name=basename[i]

	os.system('rm -rf cal-'+name+'_del.Knew')
        gencal(vis=name+'.ms', caltable='cal-'+name+'_del.Knew',
               caltype='sbd', antenna='DV07', pol='X,Y', spw='1,3,5,7',
               parameter=[1.00, 1.10, -3.0, -3.0, -3.05, -3.05, -3.05, -3.05])

    timing()

# Applycal of delay and  wvr corrections
mystep = 4
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]
    for i in range(3): # loop over the first three data sets
	name=basename[i]
	applycal(vis=name+'.ms', flagbackup=F, spw='1,3,5,7',
		 interp=['nearest','nearest'], gaintable=['cal-'+name+'_del.Knew', 'cal-'+name+'.Wnew'])

    for i in range(3,6): # loop over the last three data sets
        name=basename[i]
        applycal(vis=name+'.ms', flagbackup=F, spw='1,3,5,7',
                 interp='nearest', gaintable='cal-'+name+'.Wnew')

    for name in basename:
	os.system('rm -rf '+name+'_K_WVR.ms*')
	split(vis=name+'.ms', outputvis=name+'_K_WVR.ms',
              datacolumn='corrected', spw='0~7')

        flagmanager(vis = name+'_K_WVR.ms', mode = 'save', versionname = 'Original')

    timing()

# Applycal of tsys corrections, split out corrected data
mystep = 5
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    flagmanager(vis ='uid___A002_X1d54a1_X174_K_WVR.ms', mode = 'restore', versionname = 'Original')
       
    flagdata(vis='uid___A002_X1d54a1_X174_K_WVR.ms', mode='manual',
             antenna='DV04', flagbackup = T, scan='4,5,9', spw='7')

    for name in basename:

	for field in ['Titan','1037*','NGC*']:
		applycal(vis=name+'_K_WVR.ms', spw='1,3,5,7', 
                         flagbackup=F, field=field, gainfield=field,
                         interp='linear,spline', gaintable=['cal-tsys_'+name+'.calnew'])

        os.system('rm -rf '+name+'_line.ms*')
	split(vis=name+'_K_WVR.ms', outputvis=name+'_line.ms',
              datacolumn='corrected', spw='1,3,5,7')


    timing()

# concatenate the MSs
mystep = 6
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    comvis=[]
    for name in basename:
	comvis.append(name+'_line.ms')

    os.system('rm -rf ngc3256_line.ms*')
    concat(vis=comvis, concatvis='ngc3256_line.ms')

    flagmanager(vis ='ngc3256_line.ms', mode = 'save', versionname = 'Original')

    timing()

# fixplanets and flag the concatenated data
mystep = 7
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    flagmanager(vis ='ngc3256_line.ms', mode = 'restore', versionname = 'Original')

    flagdata(vis='ngc3256_line.ms', flagbackup=T, spw='*:0~16,*:125~127')

    flagdata(vis = 'ngc3256_line.ms', flagbackup = T,
	timerange='>2011/04/16/12:00:00', field='Titan')

    fixplanets(vis='ngc3256_line.ms', field='Titan', fixuvw=True)

    flagdata(vis='ngc3256_line.ms', flagbackup=T, spw='3',
             correlation='YY', mode='manual',
             antenna='DV07', timerange='')

    flagdata(vis='ngc3256_line.ms', flagbackup=T, spw='3',
             correlation='YY', mode='manual',
             antenna='DV08', timerange='>2011/04/17/03:00:00')

    flagdata(vis='ngc3256_line.ms', flagbackup=T, spw='0',
             correlation='', mode='manual',
             antenna='PM03', timerange='2011/04/17/02:15:00~02:15:50')

    flagdata(vis='ngc3256_line.ms', flagbackup=T, spw='2,3', 
             correlation='', mode='manual',
             antenna='PM03', timerange='2011/04/16/04:13:50~04:18:00')

    flagdata(vis='ngc3256_line.ms', flagbackup=T, spw='',
             correlation='', mode='manual',
             antenna='PM03&DV10', timerange='>2011/04/16/15:00:00')

    timing()

# Fast phase-only gaincal on the bandpass calibrator
mystep = 8
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf cal-ngc3256.G1')
    gaincal(vis='ngc3256_line.ms', caltable='cal-ngc3256.G1', spw='*:40~80', field='1037*',
            selectdata=T, solint='int', refant=therefant, calmode='p')

    if makeplots:
        plotcal(caltable = 'cal-ngc3256.G1', xaxis = 'time', yaxis = 'phase',
                poln='X', plotsymbol='o', plotrange = [0,0,-180,180], iteration = 'spw',
                figfile='cal-phase_vs_time_XX.G1.png', subplot = 221)
        plotcal(caltable = 'cal-ngc3256.G1', xaxis = 'time', yaxis = 'phase',
                poln='Y', plotsymbol='o', plotrange = [0,0,-180,180], iteration = 'spw',
                figfile='cal-phase_vs_time_YY.G1.png', subplot = 221)

    timing()

# Bandpass calibration
mystep = 9
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf cal-ngc3256.B1')

    bandpass(vis = 'ngc3256_line.ms', caltable = 'cal-ngc3256.B1', 
             gaintable = 'cal-ngc3256.G1', timerange='<2011/04/16/15:00:00',
             field = '1037*', minblperant=3, minsnr=2, solint='inf', combine='scan,obs',
             bandtype='B', fillgaps=1, refant = therefant, solnorm = T)
    
    bandpass(vis = 'ngc3256_line.ms', caltable = 'cal-ngc3256.B1', 
             gaintable = 'cal-ngc3256.G1', timerange='>2011/04/16/15:00:00',
             field = '1037*', minblperant=3, minsnr=2, solint='inf', combine='scan,obs',
             bandtype='B', fillgaps=1, refant = therefant, solnorm = T, append=True)
    
    if makeplots:
        plotbandpass(caltable = 'cal-ngc3256.B1', xaxis='freq', yaxis='phase', plotrange = [0,0,-70,70],
                     overlay='antenna', figfile='bandpass.B1.png', interactive=False)

        plotbandpass(caltable = 'cal-ngc3256.B1', xaxis='freq', yaxis='amp', overlay='antenna',
                     figfile='bandpass.B2.png', interactive=False)

    timing()

# Set fluxscale on Titan
mystep = 10
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    setjy(vis='ngc3256_line.ms', field='Titan', standard='Butler-JPL-Horizons 2012', 
          spw='0,1,2,3', scalebychan=False, usescratch=False)

    timing()

# Amplitude and Phase gaincal
mystep = 11
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    gaincal(vis = 'ngc3256_line.ms', caltable = 'cal-ngc3256.G2', spw =
            '*:16~112', field = '1037*,Titan', minsnr=1.0,
            solint= 'inf', selectdata=T, solnorm=False, refant = therefant,
            gaintable = 'cal-ngc3256.B1', calmode = 'ap')
    
    if makeplots:
        plotcal(caltable = 'cal-ngc3256.G2', xaxis = 'time', yaxis = 'phase',
                poln='X', plotsymbol='o', plotrange = [0,0,-180,180], iteration= 'spw', 
                figfile='cal-phase_vs_time_XX.G2.png', subplot = 221)
        plotcal(caltable = 'cal-ngc3256.G2', xaxis = 'time', yaxis = 'phase',
                poln='Y', plotsymbol='o', plotrange = [0,0,-180,180], iteration= 'spw', 
                figfile='cal-phase_vs_time_YY.G2.png', subplot = 221)
        plotcal(caltable = 'cal-ngc3256.G2', xaxis = 'time', yaxis = 'amp',
                poln='X', plotsymbol='o', plotrange = [], iteration = 'spw',
                figfile='cal-amp_vs_time_XX.G2.png', subplot = 221)
        plotcal(caltable = 'cal-ngc3256.G2', xaxis = 'time', yaxis = 'amp',
                poln='Y', plotsymbol='o', plotrange = [], iteration = 'spw',
                figfile='cal-amp_vs_time_YY.G2.png', subplot = 221)

    timing()

# Adjust absolute fluxscale
mystep = 12
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    fluxscale(vis="ngc3256_line.ms", caltable="cal-ngc3256.G2",
              fluxtable="cal-ngc3256.G2.flux", reference="Titan",
              transfer="1037*", refspwmap=[0,1,1,1])

    timing()

# Apply bandpass and gain calibration
mystep = 13
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    applycal(vis='ngc3256_line.ms', flagbackup=F, field='NGC*,1037*',
             interp=['nearest','nearest'], gainfield = ['1037*', '1037*'],
             gaintable=['cal-ngc3256.G2.flux', 'cal-ngc3256.B1'])

    flagmanager(vis = 'ngc3256_line.ms', mode = 'save', versionname = 'step12')

    timing()

# Final flagging
mystep = 14
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    flagmanager(vis = 'ngc3256_line.ms', mode = 'restore', versionname = 'step12')
    
    flagdata(vis='ngc3256_line.ms', mode='manual',
	timerange='2011/04/16/04:13:35~04:13:45', flagbackup = T)
    
    flagdata(vis='ngc3256_line.ms', mode='manual',
	timerange='2011/04/16/05:21:13~05:21:19', flagbackup = T)
    
    flagdata(vis='ngc3256_line.ms', mode='manual',
	timerange='2011/04/16/04:16:40~04:16:49', flagbackup = T)
    
    flagdata(vis='ngc3256_line.ms', mode='manual',
	timerange='2011/04/16/04:14:00~04:17:10', antenna='PM03', flagbackup = T)

    flagdata(vis='ngc3256_line.ms', mode='manual',
	timerange='2011/04/17/01:52:20~01:53:10', antenna='DV10', flagbackup = T)
    
    flagdata(vis='ngc3256_line.ms', mode='manual',
	timerange='2011/04/17/00:35:30~01:20:20', antenna='DV04', spw='3', flagbackup = T)

    flagmanager(vis = 'ngc3256_line.ms', mode = 'save', versionname = 'step13')

    timing()

# Final calibration
mystep = 15
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    delmod('ngc3256_line.ms')

    os.system('rm -rf cal-ngc3256.G1n cal-ngc3256.B1n')
    
    gaincal(vis='ngc3256_line.ms', caltable='cal-ngc3256.G1n', spw='*:40~80', field='1037*',
            selectdata=T, solint='int', refant=therefant, calmode='p')


    bandpass(vis = 'ngc3256_line.ms', caltable = 'cal-ngc3256.B1n', 
             gaintable = 'cal-ngc3256.G1n', timerange='<2011/04/16/15:00:00',
             field = '1037*', minblperant=3, minsnr=2, solint='inf', combine='scan,obs',
             bandtype='B', fillgaps=1, refant = therefant, solnorm = T)
    
    bandpass(vis = 'ngc3256_line.ms', caltable = 'cal-ngc3256.B1n', 
             gaintable = 'cal-ngc3256.G1n', timerange='>2011/04/16/15:00:00',
             field = '1037*', minblperant=3, minsnr=2, solint='inf', combine='scan,obs',
             bandtype='B', fillgaps=1, refant = therefant, solnorm = T, append=True)

    setjy(vis='ngc3256_line.ms', field='Titan', standard='Butler-JPL-Horizons 2012', 
          spw='0,1,2,3', usescratch=False)

    gaincal(vis = 'ngc3256_line.ms', caltable = 'cal-ngc3256.G2n', spw =
            '*:16~112', field = '1037*,Titan', minsnr=1.0,
            solint= 'inf', selectdata=T, solnorm=False, refant = therefant,
            gaintable = 'cal-ngc3256.B1n', calmode = 'ap')

    fluxscale( vis="ngc3256_line.ms", caltable="cal-ngc3256.G2n",
               fluxtable="cal-ngc3256.G2n.flux", reference="Titan",
               transfer="1037*", refspwmap=[0,1,1,1])

    applycal( vis='ngc3256_line.ms', flagbackup=F, field='NGC*,1037*',
              interp=['nearest','nearest'], gainfield = ['1037*', '1037*'],
              gaintable=['cal-ngc3256.G2n.flux', 'cal-ngc3256.B1n'])

    timing()

# Image the phase calibrator
mystep = 16
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf result-phasecal_cont*')
    clean(vis='ngc3256_line.ms', imagename='result-phasecal_cont', field='1037*',
          spw='*:20~120', selectdata=T, mode='mfs', niter=500,
          gain=0.1, threshold='0.75mJy', psfmode='hogbom',
          interactive=False, mask=[62, 62, 67, 67], imsize=128,
          cell='1arcsec', weighting='briggs', robust=0.0, nterms=2)
    
    calstat=imstat(imagename='result-phasecal_cont.image.tt0', region='', box='85,8,120,120')
    rmspcal=(calstat['rms'][0])
    print '>> rms in phase calibrator image: '+str(rmspcal)
    calstat=imstat(imagename='result-phasecal_cont.image.tt0', region='')
    peakpcal=(calstat['max'][0])
    print '>> Peak in phase calibrator image: '+str(peakpcal)
    print '>> Dynamic range in phase calibrator image: '+str(peakpcal/rmspcal)

    if makeplots:
        imview(raster={'file': 'result-phasecal_cont.image.tt0', 'colorwedge':T,
                       'range':[-0.004, 0.250], 'scaling':-1.5, 'colormap':'Rainbow 2'},
               out='result-phasecal_map.png', zoom=1)

        imview(raster={'file': 'result-phasecal_cont.image.tt0', 'colorwedge':T,
                       'colormap':'Rainbow 2'},
               out='result-phasecal_map-lin.png', zoom=1)

    timing()

# Apply calibration to flux calibrator
mystep = 17
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    applycal(vis='ngc3256_line.ms', flagbackup=F, field='Titan',
             interp=['nearest', 'nearest'], gainfield = ['Titan', '1037*'],
             gaintable=['cal-ngc3256.G2.flux', 'cal-ngc3256.B1'])

    timing()

# Image flux calibrator
mystep = 18
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf result-ampcal_cont*')
    clean(vis='ngc3256_line.ms', imagename='result-ampcal_cont', 
          field='Titan', spw='0:20~120,1:20~120', mode='mfs', niter=200, 
          threshold='5mJy', psfmode='hogbom', mask=[62, 62, 67, 67], imsize=128,
          cell='1arcsec', weighting='briggs', robust=0.0)

    calstat=imstat(imagename="result-ampcal_cont.image",region="",box="85,8,120,120")
    rmstitan=(calstat['rms'][0])
    print ">> rms in amp calibrator image: "+str(rmstitan)
    calstat=imstat(imagename="result-ampcal_cont.image",region="")
    peaktitan=(calstat['max'][0])
    print ">> Peak in amp calibrator image: "+str(peaktitan)
    print ">> Dynamic range in amp calibrator image: "+str(peaktitan/rmstitan)

    if makeplots:
        imview(raster={'file': 'result-ampcal_cont.image', 'colorwedge':T,
                       'range':[-0.02, 0.250], 'scaling':-1.5, 'colormap':'Rainbow 2'},
               out='result-ampcal_map.png', zoom=1)

        imview(raster={'file': 'result-ampcal_cont.image', 'colorwedge':T,
                       'colormap':'Rainbow 2'},
               out='result-ampcal_map-lin.png', zoom=1)

    timing()

# split out the calibrated target data
mystep = 19
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf ngc3256_line_target.ms*')
    split(vis='ngc3256_line.ms', outputvis='ngc3256_line_target.ms',
          field='NGC*')

# Image the NGC3256 continuum
mystep = 20
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf result-ngc3256_cont*')
    clean( vis='ngc3256_line_target.ms', imagename='result-ngc3256_cont',
           spw='0:20~53;71~120,1:70~120,2:20~120,3:20~120', psfmode='hogbom',
           mode='mfs', niter=500, threshold='0.3mJy', mask=[42,43,59,60],
           imsize=100, cell='1arcsec', weighting='briggs', robust=0.0, 
           interactive=False, usescratch=False)

    calstat=imstat(imagename='result-ngc3256_cont.image', region='', box='10,10,90,35')
    rmscont=(calstat['rms'][0])
    print '>> rms in continuum image: '+str(rmscont)
    calstat=imstat(imagename='result-ngc3256_cont.image', region='')
    peakcont=(calstat['max'][0])
    print '>> Peak in continuum image: '+str(peakcont)
    print '>> Dynamic range in continuum image: '+str(peakcont/rmscont)

    if makeplots:
        imview(raster={'file': 'result-ngc3256_cont.image', 'colorwedge':T,
                       'range':[-0.001, 0.009], 'scaling':0, 'colormap':'Rainbow 2'},
               out='result-ngc3256_cont.png', zoom=2)

    timing()

# Slow phase selfcal
mystep = 21
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf cal-ngc3256_cont_30m.Gp')
    gaincal(vis='ngc3256_line_target.ms', field='NGC*',
            caltable='cal-ngc3256_cont_30m.Gp',
            spw='0:20~53;71~120,1:70~120,2:20~120,3:20~120',
            solint='1800s', refant='DV07', calmode='p',
            minblperant=3)

    if makeplots:
        plotcal(caltable = 'cal-ngc3256_cont_30m.Gp', 
                xaxis = 'time', yaxis = 'phase', 
                poln='X', plotsymbol='o', plotrange = [0,0,-180,180],
                iteration = 'spw', figfile='cal-phase_vs_time_XX_30_Gp.png',
                subplot = 221)

        plotcal(caltable = 'cal-ngc3256_cont_30m.Gp', 
                xaxis = 'time', yaxis = 'phase', 
                poln='Y', plotsymbol='o', plotrange = [0,0,-180,180],
                iteration = 'spw', figfile='cal-phase_vs_time_YY_30_Gp.png',
                subplot = 221)

    applycal(vis='ngc3256_line_target.ms', interp='linear',
             gaintable='cal-ngc3256_cont_30m.Gp')

    timing()

# Image the NGC3256 continuum again after selfcal
mystep = 22
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf result-ngc3256_cont_sc1*')
    clean(vis='ngc3256_line_target.ms', imagename='result-ngc3256_cont_sc1',
          spw='0:20~53;71~120,1:70~120,2:20~120,3:20~120', psfmode='hogbom', 
          mode='mfs', niter=500, threshold='0.13mJy', mask=[42,43,59,60], 
          imsize=100, cell='1arcsec', weighting='briggs', robust=0.0,
          interactive=False, usescratch=False)

    calstat=imstat(imagename='result-ngc3256_cont_sc1.image', region='', 
                   box='10,10,90,35')
    rmscontsc=(calstat['rms'][0])
    print '>> rms in continuum image: '+str(rmscontsc)
    calstat=imstat(imagename='result-ngc3256_cont_sc1.image', region='')
    peakcontsc=(calstat['max'][0])
    print '>> Peak in continuum image: '+str(peakcontsc)
    print '>> Dynamic range in continuum image: '+str(peakcontsc/rmscontsc)

    if makeplots:
        imview(raster={'file': 'result-ngc3256_cont_sc1.image', 'colorwedge':T,
                       'range':[-0.001, 0.010], 'scaling':0, 'colormap':'Rainbow 2'},
               out='result-ngc3256_cont_sc1.png', zoom=2)

    timing()

# Determine and subtract continuum
mystep = 23
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    uvcontsub(vis = 'ngc3256_line_target.ms',
              fitspw='0:20~53;71~120,1:70~120,2:20~120,3:20~120', solint ='int', 
              fitorder = 1,
              combine='spw')

    timing()

# Clean the NGC3256 line cube
mystep = 24
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf result-ngc3256_line_CO.*')
    clean(vis='ngc3256_line_target.ms.contsub', imagename='result-ngc3256_line_CO',
          spw='0:38~87', mode='channel', start='', nchan=50, width='', 
          psfmode='hogbom', outframe='LSRK', restfreq='115.271201800GHz', 
          mask=[53,50,87,83], niter=500, interactive=False, imsize=128, cell='1arcsec', 
          weighting='briggs', robust=0.0, threshold='5mJy')

    timing()

# Evaluate the NGC3256 line cube
mystep = 25
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf myresults.tbl')
    slsearch(outfile='myresults.tbl', freqrange = [84,116], species=['COv=0'])
    sl.open('myresults.tbl')
    sl.list()

    tb.open('myresults.tbl') 
    restfreq=tb.getcol('FREQUENCY')[0]
    tb.close()

    os.system('rm -rf result-ngc3256_CO1-0.mom0*')
    immoments(imagename='result-ngc3256_line_CO.image', moments=[0],
              chans='15~34', box='38,38,90,90', axis='spectral',
              includepix=[0.02, 10000], outfile='result-ngc3256_CO1-0.mom0')

    os.system('rm -rf result-ngc3256_CO1-0.mom1*')
    immoments(imagename='result-ngc3256_line_CO.image', moments=[1],
              chans='15~34', box='38,38,90,90', axis='spectral',
              includepix=[0.045, 10000], outfile='result-ngc3256_CO1-0.mom1')

    os.system('rm -rf result-ngc3256_CO1-0.mom2*')
    immoments(imagename='result-ngc3256_line_CO.image', moments=[2],
              chans='5~44', box='38,38,90,90', axis='spectral',
              includepix=[0.035, 10000], outfile='result-ngc3256_CO1-0.mom2')

    if makeplots:
        imview(contour={'file': 'result-ngc3256_CO1-0.mom0','levels': 
                        [5,10,20,40,80,160],'base':0,'unit':1}, 
               raster={'file': 'result-ngc3256_CO1-0.mom1','range': [2630,2920],
                       'colorwedge':T, 'colormap': 'Rainbow 2'}, out='result-CO_velfield.png')
        
        imview(contour={'file': 'result-ngc3256_CO1-0.mom1','levels': 
                        [2650,2700,2750,2800,2850,2900],'base':0,'unit':1}, 
               raster={'file': 'result-ngc3256_CO1-0.mom0', 'colorwedge':T,
                       'colormap': 'Rainbow 2','scaling':-1.0,'range': [0.8,250]}, 
               out='result-CO_map.png')
        
        imview(contour={'file': 'result-ngc3256_CO1-0.mom2','levels': 
                        [20,30,40,50,60],'base':0,'unit':1}, 
               raster={'file': 'result-ngc3256_CO1-0.mom2', 'colorwedge':T,
                       'colormap': 'Greyscale 1','scaling':-1.0,'range': [0,74]}, 
               out='result-CO_dispersion.png')

    timing()

# Clean the NGC3256 CNhi line cube
mystep = 26
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf result-ngc3256_line_CNhi.*')
    clean(vis='ngc3256_line_target.ms', imagename='result-ngc3256_line_CNhi',
          outframe='LSRK', spw='1:50~76', start='', nchan=27, width='',
          restfreq='113.48812GHz', selectdata=T, mode='channel',
          niter=500, gain=0.1, psfmode='hogbom', mask=[53,50,87,83],
          interactive=False, imsize=128, cell='1arcsec',
          weighting='briggs', robust=0.0, threshold='2mJy',
          usescratch=False)

    timing()

# Evaluate the NGC3256 CNhi line cube
mystep = 27
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf result-ngc3256_CNhi.mom.*')
    immoments( imagename='result-ngc3256_line_CNhi.image', moments=[0,1],
               chans='5~18', axis='spectral', box='38,38,90,90',
               includepix=[0.005, 10000], outfile='result-ngc3256_CNhi.mom')
    if makeplots:
        imview(contour={'file': 'result-ngc3256_CNhi.mom.integrated','range': []}, 
               raster={'file': 'result-ngc3256_CNhi.mom.weighted_coord',
                       'range': [2630,2920],'colorwedge':T,
                       'colormap': 'Rainbow 2'}, out='result-CNhi_velfield.png')
        imview(contour={'file': 'result-ngc3256_CNhi.mom.weighted_coord','levels': 
                        [2650,2700,2750,2800,2850,2900],'base':0,'unit':1}, 
               raster={'file': 'result-ngc3256_CNhi.mom.integrated','colorwedge':T,
                       'colormap': 'Rainbow 2'}, out='result-CNhi_map.png')

    timing()

# Clean the NGC3256 CNlo line cube
mystep = 28
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf result-ngc3256_line_CNlo.*')
    clean( vis='ngc3256_line_target.ms', imagename='result-ngc3256_line_CNlo',
           outframe='LSRK', spw='1:29~54', start='', nchan=26, width='',
           restfreq='113.17049GHz', selectdata=T, mode='channel',
           niter=300, gain=0.1, psfmode='hogbom', mask=[53,50,87,83],
           interactive=False, imsize=128, cell='1arcsec',
           weighting='briggs', robust=0.0, threshold='2mJy',
           usescratch=False)

    timing()

# Evaluate the NGC3256 CNlo line cube
mystep = 29
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf result-ngc3256_CNlo.mom.*')
    immoments( imagename='result-ngc3256_line_CNlo.image', moments=[0,1],
               chans='8~18', axis='spectral', box='38,38,90,90',
               includepix=[0.003, 10000], outfile='result-ngc3256_CNlo.mom')

    if makeplots:
        imview(contour={'file': 'result-ngc3256_CNlo.mom.integrated','range': []}, 
               raster={'file': 'result-ngc3256_CNlo.mom.weighted_coord',
                       'range': [2630,2920],'colorwedge':T,
                       'colormap': 'Rainbow 2'}, out='result-CNlo_velfield.png')
        
        imview(contour={'file': 'result-ngc3256_CNlo.mom.weighted_coord','levels': 
                        [2650,2700,2750,2800,2850,2900],'base':0,'unit':1}, 
               raster={'file': 'result-ngc3256_CNlo.mom.integrated','colorwedge':T,
                       'colormap': 'Rainbow 2'}, out='result-CNlo_map.png')

    timing()

# Verify regression results
mystep = 30
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    # reference values obtained with the NGC3256 Band 3 CASA guide for CASA 3.3 (9 May 2012, DP)
    refrmspcal33 = 0.000556594866794
    refpeakpcal33 = 1.86264276505
    refrmstitan33 = 0.00490083405748
    refpeaktitan33 = 0.361426800489
    refrmscont33 = 0.000324592343532
    refpeakcont33 = 0.00705300131813
    refrmscontsc33 = 8.03611983429e-05
    refpeakcontsc33 = 0.00976239796728

    # reference values obtained with this script using CASA stable r19569 (10 May 2012, DP)
    #refrmspcal    = 0.000612945703324
    #refpeakpcal   = 1.86513638496
    #refrmstitan   = 0.00491443555802
    #refpeaktitan  = 0.361274629831
    #refrmscont    = 0.00032463966636
    #refpeakcont   = 0.00699360202998
    #refrmscontsc  = 7.81473427196e-05
    #refpeakcontsc = 0.00983608141541 

    # reference values obtained with this script using CASA stable r21621 (15 Oct 2012, DP)
    #refrmspcal    =  0.000963084050454
    #refpeakpcal   =  1.87274956703    
    #refrmstitan   =  0.00491269072518 
    #refpeaktitan  =  0.361274719238   
    #refrmscont    =  0.000305994151859
    #refpeakcont   =  0.00709502119571 
    #refrmscontsc  =  7.5960662798e-05 
    #refpeakcontsc =  0.00963686406612 

    # reference values obtained with this script using CASA 4.0.1 (12 Feb 2013, DP)
    #refrmspcal    =  0.000967486877926
    #refpeakpcal   =  1.87324357033
    #refrmstitan   =  0.00491303578019
    #refpeaktitan  =  0.361277967691
    #refrmscont    =  0.000327011366608
    #refpeakcont   =  0.00702479109168
    #refrmscontsc  =  7.84753283369e-05
    #refpeakcontsc =  0.00987098459154

    # reference values obtained with this script using CASA 4.1 prerelease 2, Butler 2012 (17 Apr 2013, DP)
    refrmspcal    = 0.00101008242927
    refpeakpcal   = 1.9241631031
    refrmstitan   = 0.0050711822696
    refpeaktitan  = 0.372662633657 
    refrmscont    = 0.000336007156875
    refpeakcont   = 0.00723568536341 
    refrmscontsc  = 8.06680545793e-05
    refpeakcontsc = 0.0101557951421 

    devrmspcal = abs(rmspcal-refrmspcal)/refrmspcal*100.
    devpeakpcal = abs(peakpcal-refpeakpcal)/refpeakpcal*100.
    devrmstitan = abs(rmstitan-refrmstitan)/refrmstitan*100.
    devpeaktitan = abs(peaktitan-refpeaktitan)/refpeaktitan*100.
    devrmscont = abs(rmscont-refrmscont)/refrmscont*100.
    devpeakcont = abs(peakcont-refpeakcont)/refpeakcont*100.
    devrmscontsc = abs(rmscontsc-refrmscontsc)/refrmscontsc*100.
    devpeakcontsc = abs(peakcontsc-refpeakcontsc)/refpeakcontsc*100.

    casalog.origin('SUMMARY')
    casalog.post("\n***** Peak and RMS of the images of the primary phase calibrator, Titan, and the Target *****")
    casalog.post( "Field, Peak (expectation, expectation CASA 3.3), RMS (expectation, expectation CASA 3.3)")
    casalog.post( "------------------------------------------------------------------------------------------")
    casalog.post("Phasecal, "+str(peakpcal)+" ("+str(refpeakpcal)+", "+str(refpeakpcal33)+"), "
                 +str(rmspcal)+" ("+str(refrmspcal)+", "+str(refrmspcal33)+")")
    casalog.post( "------------------------------------------------------------------------------------------")
    casalog.post("Titan, "+str(peaktitan)+" ("+str(refpeaktitan)+", "+str(refpeaktitan33)+"), "
                 +str(rmstitan)+" ("+str(refrmstitan)+", "+str(refrmstitan33)+")")
    casalog.post( "------------------------------------------------------------------------------------------")
    casalog.post("NGC3256 continuum, "+str(peakcont)+" ("+str(refpeakcont)+", "+str(refpeakcont33)+"), "
                 +str(rmscont)+" ("+str(refrmscont)+", "+str(refrmscont33)+")")
    casalog.post( "------------------------------------------------------------------------------------------")
    casalog.post("NGC3256 selfcaled continuum, "+str(peakcontsc)+" ("+str(refpeakcontsc)+", "+str(refpeakcontsc33)+"), "
                 +str(rmscontsc)+" ("+str(refrmscontsc)+", "+str(refrmscontsc33)+")")
    casalog.post( "------------------------------------------------------------------------------------------")

    print rmspcal, refrmspcal
    print peakpcal, refpeakpcal
    print rmstitan, refrmstitan
    print peaktitan, refpeaktitan
    print rmscont, refrmscont
    print peakcont, refpeakcont
    print rmscontsc, refrmscontsc
    print peakcontsc, refpeakcontsc

    passed = True

    if devpeakpcal>0.5:
        casalog.post( 'ERROR: Peak in primary phase calibrator image deviates from expectation by '+str(devpeakpcal)+' percent.', 'WARN')
        passed = False
    if devrmspcal>0.5:
        casalog.post( 'ERROR: RMS in primary phase calibrator image deviates from expectation by '+str(devrmspcal)+' percent.', 'WARN')
        passed = False
    if devpeaktitan>0.5:
        casalog.post( 'ERROR: Peak in image of Titan deviates from expectation by '+str(devpeaktitan)+' percent.', 'WARN')
        passed = False
    if devrmstitan>0.5:
        casalog.post( 'ERROR: RMS in image of Titan deviates from expectation by '+str(devrmstitan)+' percent.', 'WARN')
        passed = False
    if devpeakcont>0.5:
        casalog.post( 'ERROR: Peak in continuum image deviates from expectation by '+str(devpeakcont)+' percent.', 'WARN')
        passed = False
    if devrmscont>0.5:
        casalog.post( 'ERROR: RMS in continuum image deviates from expectation by '+str(devrmscont)+' percent.', 'WARN')
        passed = False
    if devpeakcontsc>0.5:
        casalog.post( 'ERROR: Peak in selfcal continuum image deviates from expectation by '+str(devpeakcontsc)+' percent.', 'WARN')
        passed = False
    if devrmscontsc>0.5:
        casalog.post( 'ERROR: RMS in selfcal continuum image deviates from expectation by '+str(devrmscontsc)+' percent.', 'WARN')
        passed = False

    if not passed:
        casalog.post('Results are different from expectations by more than 0.5 percent.', 'WARN')
    else:
        casalog.post( "\nAll peak and RMS values within 0.5 percent of the expectation.")
        
    pngfiles = ['cal-tsys_per_spw_1_uid___A002_X1d54a1_X174.png',
                'cal-tsys_per_spw_7_uid___A002_X1d5a20_X5.png',
                'cal-phase_vs_time_XX.G1.png',
                'cal-phase_vs_time_YY.G1.png',
                'bandpass.B1.png',
                'cal-phase_vs_time_XX.G2.png',
                'cal-phase_vs_time_YY.G2.png',
                'cal-amp_vs_time_XX.G2.png',
                'cal-amp_vs_time_YY.G2.png',
                'result-phasecal_map.png',
                'result-phasecal_map-lin.png',
                'result-ampcal_map.png',
                'result-ampcal_map-lin.png',
                'result-ngc3256_cont.png',
                'cal-phase_vs_time_XX_30_Gp.png',
                'cal-phase_vs_time_YY_30_Gp.png',
                'result-ngc3256_cont_sc1.png',
                'result-CO_velfield.png',
                'result-CO_map.png',
                'result-CO_dispersion.png',
                'result-CNhi_velfield.png',
                'result-CNhi_map.png',
                'result-CNlo_velfield.png',
                'result-CNlo_map.png']

    for pngfile in pngfiles:
        if not os.path.exists(pngfile):
            casalog.post('Plot file '+pngfile+' was not created!', 'WARN')
            passed = False
            
    if not passed:
        raise Exception, 'Regression failed.'

    print "Passed."

    timing()
