#############################################################################
# $Id:$
# Test Name:                                                                #
# alma-m100-analysis-hpc-regression.py                                      #
# An ALMA Science Verification Data Analysis Regression                     #
# using observation of M100 from September 2011                             # 
#                                                                           # 
# Rationale for Inclusion:                                                  #
#    Need test of complete ALMA analysis chain                              #
#                                                                           # 
# Input data:                                                               #
#     two ASDMs                                                             #
#     the clean masks                                                       #
#                                                                           #
#############################################################################

# alma-m100-analysis-hpc-regression.py
# An ALMA Science Verification Data Analysis Regression
# using observations of M100 from September 2011 and _parallelisation_


step_title = { 0 : 'Data import and partitioning',
	       1 : 'Generate antenna position cal tables',
	       2 : 'Generate tsys cal tables',
	       3 : 'Correct the Titan position',
	       4 : 'Apriori flagging',
	       5 : 'Generate WVR cal tables',
	       6 : 'Generate delay calibration tables',
               7 : 'Apply antpos, wvr, tsys, and delay cal tables',
	       8 : 'Split off non-wvr spws and save flags',
               9 : 'Flagging',
	       10: 'Rebin to a reduced resolution of approx. 10 km/s',
	       11: 'Fast phase-only gaincal for bandpass',
               12: 'Bandpass',
               13: 'Setjy',
               14: 'Fast phase-only gaincal',
               15: 'Slow phase-only gaincal',
               16: 'Slow amp and phase gaincal',
               17: 'Fluxscale',
               18: 'Applycal',
               19: 'Test image of the secondary phase cal',
               20: 'Test image of the primary phase cal',
               21: 'Test image of Titan',
               22: 'Split off calibrated M100 data',
               23: 'Concatenate M100 data',
               24: 'Average concatenated M100 data in time',
               25: 'Continuum image of M100',
               26: 'Determine and subtract continuum',
               27: 'Test image of central field',
               28: 'Clean line cube mosaic',
               29: 'Make moment maps',
               30: 'Verification of the regression results'
               }

# global defs
basename=['X54','X220']
makeplots=False
print "Make plots?", makeplots
usepclean=True
print "Use pclean?", usepclean
therefant = 'DV01'
mynumsubmss = 8

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
#   execfile('alma-m100-analysis-hpc-regression.py')
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
        

# default ASDM dataset name
myasdm_dataset_name = "uid___A002_X2a5c2f_X54"
myasdm_dataset2_name = "uid___A002_X2a5c2f_X220"

# get the dataset name from the wrapper if possible
mydict = locals()
if mydict.has_key("asdm_dataset_name"):
    if(myasdm_dataset_name != mydict["asdm_dataset_name"]):
        raise("Wrong input file 1")
if mydict.has_key("asdm_dataset2_name"):
    if(myasdm_dataset2_name != mydict["asdm_dataset2_name"]):
        raise("Wrong input file 2")        

# data import and partitioning
mystep = 0
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    importasdm(asdm='uid___A002_X2a5c2f_X54',vis='X54-monolith.ms',asis='Stati* Anten*', overwrite=True)
    importasdm(asdm='uid___A002_X2a5c2f_X220',vis='X220-monolith.ms',asis='Stati* Anten*', overwrite=True)

    os.system('rm -rf X54.ms* X220.ms*')

	# Note: the versionname after partition is called partition_1, not Original!
    partition(vis='X54-monolith.ms', outputvis='X54.ms', numsubms=mynumsubmss, separationaxis='scan')
    partition(vis='X220-monolith.ms', outputvis='X220.ms', numsubms=mynumsubmss, separationaxis='scan')

    if(makeplots):
        for name in basename:
            os.system('rm -rf plotants-'+name+'.png')
            plotants(name+'.ms', figfile='plotants-'+name+'.png')

    timing()


# Antenna positions
mystep = 1
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        os.system('rm -rf cal-antpos_'+name)

    gencal(vis = 'X54.ms',
	   caltable = 'cal-antpos_X54',
	   caltype = 'antpos',
	   antenna = 'CM01,PM02,PM04,DV08,DV06,DV13,DV03,DV14',
	   parameter = [-0.000379055076246,0.000910912511392,-0.000226045671848,7.8790821135e-05,0.000363811850548,-0.000224065035582,-0.000315962965482,0.000397399838991,-0.000581170175089,-6.35427422822e-05,0.00129573699087,-0.000625061802566,-0.000167516991496,0.000174060463905,-0.000417742878199,-0.00010875146836,0.000319179147482,-0.000588130671531,0.000142965465784,0.000455257482827,0.000168651808053,-0.00150847900659,0.00357818510383,0.000811365433037]
	   )
    gencal(vis = 'X220.ms',
	   caltable = 'cal-antpos_X220',
	   caltype = 'antpos',
	   antenna = 'CM01,PM02,PM04,DV08,DV06,DV13,DV03,DV14',
	   parameter = [-0.000379055076246,0.000910912511392,-0.000226045671848,7.8790821135e-05,0.000363811850548,-0.000224065035582,-0.000315962965482,0.000397399838991,-0.000581170175089,-6.35427422822e-05,0.00129573699087,-0.000625061802566,-0.000167516991496,0.000174060463905,-0.000417742878199,-0.00010875146836,0.000319179147482,-0.000588130671531,0.000142965465784,0.000455257482827,0.000168651808053,-0.00150847900659,0.00357818510383,0.000811365433037]
	   )

    timing()


# Tsys table generation
mystep = 2
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        os.system('rm -rf cal-tsys_'+name+'.fdm')
        gencal(
            vis=name+'.ms',
            caltype='tsys', 
            caltable='cal-tsys_'+name+'.fdm')

    if(makeplots):
        for spw in ['9','11','13','15']:
            for name in basename:
                plotcal(caltable='cal-tsys_'+name+'.fdm', xaxis='freq', yaxis='amp',
                        spw=spw, subplot=721, overplot=False,
                        iteration='antenna', plotrange=[0, 0, 40, 180], plotsymbol='.',
                        figfile='cal-tsys_per_spw_'+spw+'_'+name+'.png')

    timing()


# correct Titan position
mystep = 3
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]
    
    for name in basename:
	    fixplanets(vis=name+'.ms', field='Titan', fixuvw=True, refant=therefant)

    timing()


# Apriori flagging
mystep = 4
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    # Create flagcmd input list
    flagcmd = ["mode='manual' antenna='CM01'",
               "mode='manual' intent='*POINTING*'",
               "mode='manual' intent='*ATMOSPHERE*'",
               "mode='shadow'",
               "mode='manual' autocorr=True spw='>0'" # do not flag the WVR data
               ] 
                  
    for name in basename:

        flagmanager(vis=name+'.ms', mode='restore', versionname='partition_1')
        
        flagdata(vis=name + '.ms', mode='list', inpfile=flagcmd, flagbackup=False)

	if(makeplots):
		# Plot amplitude vs time
		plotms(vis=name+'.ms', xaxis='time', yaxis='amp', spw='1',
		       averagedata=T, avgchannel='4000', coloraxis='field',
		       iteraxis='spw', plotfile=name+'-amp-vs-time.png', overwrite=True)

    timing()


# WVR cal table generation
mystep = 5
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        os.system('rm -rf cal-wvr_'+name)

#    os.system('/home/arcproc/opsw/alma/wvrgcal-stable/bin/wvrgcal --ms X54.ms --output cal-wvr_X54 --toffset -1 --wvrflag CM01 --segsource --tie "1224+213 Phase","M100" --statsource "1224+213 Phase"')

#    os.system('/home/arcproc/opsw/alma/wvrgcal-stable/bin/wvrgcal --ms X220.ms --output cal-wvr_X220 --toffset -1 --wvrflag CM01 --segsource --tie "1224+213 Phase","M100" --statsource "1224+213 Phase"')
    wvrgcal(vis="X54.ms", caltable='cal-wvr_X54', toffset=-1, wvrflag=['CM01'], segsource=True, tie=["1224+213 Phase,M100"], statsource="1224+213 Phase")
    wvrgcal(vis='X220.ms', caltable='cal-wvr_X220', toffset=-1, wvrflag=['CM01'], segsource=True, tie=["1224+213 Phase,M100"], statsource="1224+213 Phase")

    timing()


# delay calibration table generation
mystep = 6
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        os.system('rm -rf cal-delay_'+name+'.K')
	gaincal(vis=name+'.ms',caltable='cal-delay_'+name+'.K',
		field='*Phase*',spw='1,3,5,7',
		solint='inf',combine='scan',refant=therefant,
		gaintable='cal-antpos_'+name,
		gaintype='K')

    timing()


# applycal
mystep = 7
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    print "Using linear interpolation for Tsys in applycal ..."
    tsysinterp='linear'
    tsysspwmap=[0,9,0,11,0,13,0,15]

    for myfield in ['3c273 - Bandpass','Titan','3c273 - Phase','1224+213 Phase','M100']:
        print "Field ", myfield
        for name in basename:
            applycal(vis=name+'.ms', 
                     spw='1,3,5,7',
                     field=myfield, gainfield=[myfield,myfield,'',''],
                     interp=['nearest',tsysinterp,'nearest','nearest'],
                     spwmap=[[],tsysspwmap,[],[]],
                     gaintable=['cal-wvr_'+name,'cal-tsys_'+name+'.fdm','cal-delay_'+name+'.K','cal-antpos_'+name],
                     flagbackup=F)


    timing()


# Split and save flags
mystep = 8
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        os.system('rm -rf '+name+'-line.ms')
        split(vis=name+'.ms',
              outputvis=name+'-line.ms',
              spw='1,3,5,7',
              datacolumn='corrected',
              keepmms=True
              )
        flagmanager(vis = name+'-line.ms',
                    mode = 'save',
                    versionname = 'apriori')
    timing()

            
# flagging
mystep = 9
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    # Create flagcmd input list (could also call flagdata twice alternatively)
    flagcmd = ["mode='manual' field='' spw='0~3:0~10;3800~3839'",
    		  "mode='manual' field='' spw='0~3:239;447~448;720~721;2847~2848'"]
              
    for name in basename:

        flagmanager(vis=name + '-line.ms', mode='restore',
                    versionname='apriori')
        
        flagdata(vis=name + '-line.ms', mode='list', inpfile=flagcmd, flagbackup=False)

    # some integrations are off
    flagdata(vis='X220-line.ms', mode='manual',
              timerange='19:52:55~19:53:04', flagbackup=False)

    flagdata(vis='X54-line.ms',
              antenna='PM01',
              timerange='19:03:35~19:03:42',
              mode='manual',
              flagbackup=False)

    flagdata(vis='X54-line.ms',
              antenna='DV04',
              timerange='19:38:45~19:38:55',
              mode='manual',
              flagbackup=False)

    timing()

# Bin it up to lower spectral resolution to about 10 km/s
mystep = 10
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        os.system('rm -rf '+name+'-line-vs.ms')
        split(vis=name+'-line.ms',
              outputvis=name+'-line-vs.ms',
              datacolumn='data',
              width='8',
              keepmms=True
              )

        flagmanager(vis=name+'-line-vs.ms', mode='save', versionname='apriori')

    timing()


# Fast phase-only gaincal for bandpass
mystep = 11
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        os.system('rm -rf cal-'+name+'-BPint.Gp')
        gaincal(vis=name+'-line-vs.ms', 
                caltable='cal-'+name+'-BPint.Gp', 
                spw='*:190~310',
                field='*Bandpass*',
                selectdata=F, solint='int', refant=therefant, calmode='p')

        if(makeplots):
            plotcal(caltable='cal-'+name+'-BPint.Gp', 
                    xaxis = 'time', yaxis = 'phase',
                    poln='X', plotsymbol='o', plotrange = [0,0,-180,180], 
                    iteration = 'spw',
                    figfile='cal-'+name+'-phase_vs_time_XX.BPint.Gp.png', subplot = 221)
		
            plotcal(caltable='cal-'+name+'-BPint.Gp', 
                    xaxis = 'time', yaxis = 'phase',
                    poln='Y', plotsymbol='o', plotrange = [0,0,-180,180], 
                    iteration = 'spw',
                    figfile='cal-'+name+'-phase_vs_time_YY.BPint.Gp.png', subplot = 221)

    timing()

# Bandpass
mystep = 12
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        os.system('rm -rf cal-'+name+'.B1')
        
        bandpass(vis=name+'-line-vs.ms', 
                 caltable='cal-'+name+'.B1', 
                 field='*Bandpass*',
                 bandtype='B', fillgaps=10, solnorm = T, combine='',
                 selectdata=F,
                 solint='inf',
                 refant=therefant,
                 gaintable='cal-'+name+'-BPint.Gp')
	
        if(makeplots):
            for spw in ['0','1','2','3']:
                plotcal(caltable = 'cal-'+name+'.B1', 
                        xaxis='freq', yaxis='phase', spw=spw, antenna='',
                        iteration='antenna',
                        subplot=431, overplot=False, plotrange = [0,0,-70,70],
                        plotsymbol='.', timerange='',
                        figfile='cal-'+name+'-phase.spw'+spw+'.B1.png')

                plotcal(caltable = 'cal-'+name+'.B1', 
                        xaxis='freq', yaxis='amp', spw=spw,
                        iteration='antenna',
                        subplot=431, overplot=False,
                        plotsymbol='.', timerange='',
                        figfile='cal-'+name+'-amplitude.spw'+spw+'.B1.png')
    timing()
	

# Setjy
# Strong line for Titan is obvious
# Noisy for uvdistances less than 40 klambda
mystep = 13
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        flagmanager(vis=name+'-line-vs.ms', mode='restore', versionname='apriori')

        flagdata(vis=name + '-line-vs.ms',
                  field='Titan',
                  mode='manual',
                  uvrange='0~40',
                  spw='',
                  flagbackup=False)
        
        flagdata(vis=name + '-line-vs.ms',
                  field='Titan',
                  mode='manual',
                  uvrange='',
                  spw='0:200~479',
                  flagbackup=False)
 
	setjy(vis=name+'-line-vs.ms',
              field='Titan',
              standard='Butler-JPL-Horizons 2010', 
              scalebychan=False,
	      spw='0,1,2,3')

    timing()

# Fast phase-only gaincal 
mystep = 14
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        os.system('rm -rf cal-'+name+'-int.Gp')
	gaincal(vis=name+'-line-vs.ms', 
		caltable='cal-'+name+'-int.Gp', 
		spw='*:25~455', 
		field='*Phase*,*Band*,Titan',
		gaintable='cal-'+name+'.B1',
		selectdata=F, solint='int', 
		refant=therefant, calmode='p')

	if(makeplots):
            plotcal(caltable='cal-'+name+'-int.Gp', 
                    xaxis = 'time', yaxis = 'phase',
                    poln='X', plotsymbol='o', plotrange = [0,0,-180,180], 
                    iteration = 'spw',
                    figfile='cal-'+name+'-phase_vs_time_XX.int.Gp.png', subplot = 221)

            plotcal(caltable='cal-'+name+'-int.Gp', 
                    xaxis = 'time', yaxis = 'phase',
                    poln='Y', plotsymbol='o', plotrange = [0,0,-180,180], 
                    iteration = 'spw',
                    figfile='cal-'+name+'-phase_vs_time_YY.int.Gp.png', subplot = 221)

    timing()


# Slow phase-only gaincal 
mystep = 15
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        os.system('rm -rf cal-'+name+'-scan.Gp')
	gaincal(vis=name+'-line-vs.ms', 
		caltable='cal-'+name+'-scan.Gp', 
		spw='*:25~455', 
		field='*Phase*,*Band*,Titan',
		gaintable='cal-'+name+'.B1',
		selectdata=F, solint='inf', 
		refant=therefant, calmode='p')

	if(makeplots):
		plotcal(caltable='cal-'+name+'-scan.Gp', 
			xaxis = 'time', yaxis = 'phase',
			poln='X', plotsymbol='o', plotrange = [0,0,-180,180], 
			iteration = 'spw',
			figfile='cal-'+name+'-phase_vs_time_XX.scan.Gp.png', subplot = 221)

		plotcal(caltable='cal-'+name+'-scan.Gp', 
			xaxis = 'time', yaxis = 'phase',
			poln='Y', plotsymbol='o', plotrange = [0,0,-180,180], 
			iteration = 'spw',
			figfile='cal-'+name+'-phase_vs_time_YY.scan.Gp.png', subplot = 221)

    timing()


# Slow amplitude and phase gaincal 
mystep = 16
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        os.system('rm -rf cal-'+name+'-scan.Gap')
	gaincal(vis=name+'-line-vs.ms', 
		caltable='cal-'+name+'-scan.Gap', 
		spw='*:25~455', 
		field='*Phase*,*Band*,Titan',
		gaintable=['cal-'+name+'.B1','cal-'+name+'-int.Gp'],
		selectdata=F, solint='inf', 
		refant=therefant, calmode='ap')

	if(makeplots):
		plotcal(caltable='cal-'+name+'-scan.Gap', 
			xaxis = 'time', yaxis = 'amp',
			poln='X', plotsymbol='o', 
			iteration = 'spw',
			figfile='cal-'+name+'-amp_vs_time_XX.scan.Gap.png', subplot = 221)

		plotcal(caltable='cal-'+name+'-scan.Gap', 
			xaxis = 'time', yaxis = 'amp',
			poln='Y', plotsymbol='o', 
			iteration = 'spw',
			figfile='cal-'+name+'-amp_vs_time_YY.scan.Gap.png', subplot = 221)

    timing()

# Fluxscale
mystep = 17
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
	fluxscale(vis=name+'-line-vs.ms',caltable='cal-'+name+'-scan.Gap',
		  fluxtable='cal-'+name+'.flux',reference='Titan',transfer='*Phase*,*Band*')

# Restore flags on calibrators?
#for name in basename:
	# flagmanager(vis=name+'-line-vs.ms',mode='restore',versionname='apriori')

    timing()


# Applycal
mystep = 18
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
	# to the bandpass cal
	applycal(vis=name+'-line-vs.ms',field='*Band*',
	        gaintable=['cal-'+name+'.B1','cal-'+name+'-int.Gp','cal-'+name+'.flux'],
	        interp=['nearest','nearest','nearest'],
	        gainfield=['*Band*','*Band*','*Band*'],
		calwt=F,
		flagbackup=T)

	# to the secondary phase cal
	applycal(vis=name+'-line-vs.ms',field='3c273 - Phase',
	        gaintable=['cal-'+name+'.B1','cal-'+name+'-scan.Gp','cal-'+name+'.flux'],
       	 	interp=['nearest','linear','linear'],
       	 	gainfield=['*Band*','1224*','1224*'],
		calwt=F,
		flagbackup=T)

	# to the primary phase cal
	applycal(vis=name+'-line-vs.ms',field='1224*',
        	gaintable=['cal-'+name+'.B1','cal-'+name+'-int.Gp','cal-'+name+'.flux'],
        	interp=['nearest','nearest','nearest'],
        	gainfield=['*Band*','1224*','1224*'],
		calwt=F,
		flagbackup=T)

	# to Titan
        applycal(vis=name+'-line-vs.ms',field='Titan',
                 gaintable=['cal-'+name+'.B1','cal-'+name+'-int.Gp','cal-'+name+'.flux'],
                 interp=['nearest','nearest','nearest'],
                 gainfield=['*Band*','Titan','Titan'],
                 calwt=F,
                 flagbackup=T)

	# to M100
	applycal(vis=name+'-line-vs.ms',field='M100',
        	gaintable=['cal-'+name+'.B1','cal-'+name+'-scan.Gp','cal-'+name+'.flux'],
        	interp=['nearest','linear','linear'],
        	gainfield=['*Band*','1224*','1224*'],
		calwt=F,
		flagbackup=T)


# For X146 the calibrated fluxes for the secondary phase cal are different for the different pols. What
# is going on? 

    timing()


# Test image of the secondary phase cal
mystep = 19
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        os.system('rm -rf test-'+name+'-sec_phasecal*')
        if usepclean:
            #pclean does not handle masks except for image masks
            # so making it
            myim=casac.imager()
            myim.open(name+'-line-vs.ms')
            myim.selectvis(field='3c*Ph*', spw='0~3')
            myim.defineimage(nx=200, cellx='0.5arcsec')
            myim.regionmask(mask='test-'+name+'-sec_phasecal.mask',boxes=[[96,96,104,104]])
            myim.done()
            pclean(vis=name+'-line-vs.ms',
                   imagename='test-'+name+'-sec_phasecal',
                   field='3c*Ph*',spw='0~3',
                   nterms=2,
                   alg='multiscale', scales=[0],
                   mode='mfs',niter=100,
                   interactive=False,
                   mask='test-'+name+'-sec_phasecal.mask', imsize=200,cell='0.5arcsec')
        else:
            clean(vis=name+'-line-vs.ms',
                  imagename='test-'+name+'-sec_phasecal',
                  field='3c*Ph*',spw='0~3',
                  nterms=2,
                  mode='mfs',niter=100,
                  interactive=False,
                  mask=[96, 96, 104, 104],imsize=200,cell='0.5arcsec')
    if makeplots:
        for name in basename:
            imview(raster={'file': 'test-'+name+'-sec_phasecal.image.tt0', 'colorwedge':T,
                           'range':[-0.02, 8.0], 'scaling':-1.5, 'colormap':'Rainbow 2'},
                   out='test-'+name+'-sec_phasecal.png', zoom=1)

    timing()


# Test image of the primary phase cal
mystep = 20
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        os.system('rm -rf test-'+name+'-prim_phasecal*')
        if usepclean:
            #pclean does not handle masks except for image masks
            # so making it
            myim=casac.imager()
            myim.open(name+'-line-vs.ms')
            myim.selectvis(field='1224*', spw='0~3')
            myim.defineimage(nx=200, cellx='0.5arcsec')
            myim.regionmask(mask='test-'+name+'-prim_phasecal.mask',boxes=[[96,96,104,104]])
            myim.done()
            pclean(vis=name+'-line-vs.ms',
                   imagename='test-'+name+'-prim_phasecal',
                   field='1224*',spw='0~3',
                   nterms=2,
                   alg='multiscale', scales=[0],
                   mode='mfs',niter=100,
                   interactive=False,
                   mask='test-'+name+'-prim_phasecal.mask',imsize=200,cell='0.5arcsec')
        else:
            clean(vis=name+'-line-vs.ms',
                  imagename='test-'+name+'-prim_phasecal',
                  field='1224*',spw='0~3',
                  nterms=2,
                  mode='mfs',niter=100,
                  interactive=False,
                  mask=[96, 96, 104, 104],imsize=200,cell='0.5arcsec')

    if makeplots:
        for name in basename:
            imview(raster={'file': 'test-'+name+'-prim_phasecal.image.tt0', 'colorwedge':T,
                           'range':[-0.005, 0.9], 'scaling':-2.5, 'colormap':'Rainbow 2'},
                   out='test-'+name+'-prim_phasecal.png', zoom=1)
            calstat=imstat(imagename='test-'+name+'-prim_phasecal.image.tt0', region='', box='30,30,170,80')
            rms=(calstat['rms'][0])
            print '>> rms in phase calibrator image: '+str(rms)
            calstat=imstat(imagename='test-'+name+'-prim_phasecal.image.tt0', region='')
            peak=(calstat['max'][0])
            print '>> Peak in phase calibrator image: '+str(peak)
            print '>> Dynamic range in phase calibrator image: '+str(peak/rms)

    timing()

# Test image of Titan
mystep = 21
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
        os.system('rm -rf test-'+name+'-Titan*')
        if usepclean:
            #pclean does not handle masks except for image masks
            # so making it
            myim=casac.imager()
            myim.open(name+'-line-vs.ms')
            myim.selectvis(field='Titan', spw='0~3')
            myim.defineimage(nx=200, cellx='0.5arcsec')
            myim.regionmask(mask='test-'+name+'-Titan.mask',boxes=[[96,96,104,104]])
            myim.done()
            pclean(vis=name+'-line-vs.ms',
                   imagename='test-'+name+'-Titan',
                   field='Titan',spw='0~3',
                   mode='mfs',niter=100,
                   interactive=False,
                   mask='test-'+name+'-Titan.mask',imsize=200,cell='0.5arcsec')
        else:
            clean(vis=name+'-line-vs.ms',
                  imagename='test-'+name+'-Titan',
                  field='Titan',spw='0~3',
                  mode='mfs',niter=100,
                  interactive=False,
                  mask=[96, 96, 104, 104],imsize=200,cell='0.5arcsec')

    timing()


# Split off the calibrated data on M100
mystep = 22
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    for name in basename:
	os.system('rm -rf '+name+'-calibrated.ms*')
	split(vis=name+'-line-vs.ms',field='M100',
	      outputvis=name+'-calibrated.ms',
	      datacolumn = 'corrected',
	      keepflags=F,
              keepmms=True
              )

    timing()

# Concat
mystep = 23
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf M100all.ms*')
    virtualconcat(vis=['X54-calibrated.ms', 'X220-calibrated.ms'],
                  concatvis='M100all.ms',
                  copypointing=False,
                  keepcopy=False # set this to True to keep a copy of the input (takes time)
                  )

    timing()


# rebin the data
mystep = 24
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf M100all_lores.ms*')
    split(vis='M100all.ms', outputvis='M100all_lores.ms',
          datacolumn='data',
          timebin='60s',
          keepmms=True 
          )

    timing()


# Continuum image
mystep = 25
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf M100cont.*')
    if usepclean:
        pclean(vis = 'M100all_lores.ms',
               imagename = 'M100cont',
               field='2~47',
               spw='0:10~210;256~440,1~3:10~460',
               mode = 'mfs',
               niter = 1000,
               mask='M100cont-orig.mask',
               ftmachine = 'mosaic',
               interactive = F,
               imsize = 200,
               cell = '0.5arcsec',
               phasecenter='J2000 12h22m54.9 +15d49m15')
    else:
        clean(vis = 'M100all_lores.ms',
              imagename = 'M100cont',
              field='2~47',
              spw='0:10~210;256~440,1~3:10~460',
              mode = 'mfs',
              niter = 1000,
              mask='M100cont-orig.mask',
              imagermode = 'mosaic',
              interactive = F,
              imsize = 200,
              cell = '0.5arcsec',
              phasecenter='J2000 12h22m54.9 +15d49m15')

# Continuum peak is 0.5 mJy. Too weak for self-cal...

    timing()


# uvcontsub2
mystep = 26
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]
    
    os.system('rm -rf M100all_lores.ms.c*')
    uvcontsub(vis='M100all_lores.ms',field='',fitspw='0:10~205;260~440',
              combine='',solint='inf',fitorder=1,spw='0',want_cont=False)

    timing()


# Test image of central field
mystep = 27
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf test-M100line.*')
    if usepclean:
        pclean(
            vis='M100all_lores.ms.contsub',
            imagename='test-M100line',
            field='26',
            spw='0:231~248',
            mode='mfs',
            niter=500,gain=0.1,threshold='0.0mJy',
            interactive=False,
            mask='test-M100line-orig.mask',
            imsize=200,cell='0.5arcsec',
            phasecenter='',
            stokes='I',
            weighting='briggs',robust=0.5,
            cyclefactor=1.5)
    else:
        clean(
            vis='M100all_lores.ms.contsub',
            imagename='test-M100line',
            field='26',
            spw='0:231~248',
            mode='mfs',
            niter=500,gain=0.1,threshold='0.0mJy',
            imagermode='csclean',
            interactive=False,
            mask='test-M100line-orig.mask',
            outframe='',veltype='radio',
            imsize=200,cell='0.5arcsec',
            phasecenter='',
            stokes='I',
            weighting='briggs',robust=0.5,
            npercycle=100,cyclefactor=1.5,cyclespeedup=-1)

    timing()


# pclean line cube mosaic
mystep = 28
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf M100line.*')
    if usepclean:
        pclean(vis='M100all_lores.ms.contsub',imagename='M100line',
               field='2~47',
               spw='0:220~259',
               mode='channel',
               niter=1000,gain=0.1,threshold='0.0mJy', alg='clark',
               ftmachine='mosaic',
               interactive=False,
               mask='M100line-orig.mask',
               nchan=40,start=220,
               width=1,
               imsize=600,cell='0.5arcsec',
               phasecenter='J2000 12h22m54.9 +15d49m10',
               restfreq='115.271201800GHz',stokes='I',
               weighting='briggs',robust=0.5,
               pbcor=False,
               cyclefactor=1.5)
    else:
        clean(vis='M100all_lores.ms.contsub',imagename='M100line',
              field='2~47',
              spw='0:220~259',
              mode='channel',
              niter=1000,gain=0.1,threshold='0.0mJy',psfmode='clark',
              imagermode='mosaic',ftmachine='mosaic',mosweight=False,
              scaletype='SAULT',
              interactive=False,
              mask='M100line-orig.mask',
              nchan=40,start=220,
              width=1,
              outframe='',veltype='radio',
              imsize=600,cell='0.5arcsec',
              phasecenter='J2000 12h22m54.9 +15d49m10',
              restfreq='115.271201800GHz',stokes='I',
              weighting='briggs',robust=0.5,
              pbcor=False,minpb=0.2,
              npercycle=100,cyclefactor=1.5,cyclespeedup=-1)

    timing()

# Moments
mystep = 29
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    os.system('rm -rf M100-CO.mom?')
    immoments(imagename='M100line.image',
              moments=[0],
              axis='spectral',
              region='',box='100,110,515,500',
              chans='7~35',
              mask='',
              outfile='M100-CO.mom0',
              includepix=[0.03, 1000000])

    immoments(imagename='M100line.image',
              moments=[1],
              axis='spectral',
              region='',box='100,110,515,500',
              chans='7~35',
              mask='',
              outfile='M100-CO.mom1',
              includepix=[0.035, 1000000])

    if makeplots:
        os.system('rm -rf M100-CO_velfield.png')
        imview(contour={'file': 'M100-CO.mom0','levels': 
                        [1,2,5,10,20,40,80,160],'base':0,'unit':1}, 
               raster={'file': 'M100-CO.mom1','range': [1440,1700],
                       'colorwedge':T, 'colormap': 'Rainbow 2'}, out='M100-CO_velfield.png')
        os.system('rm -rf M100-CO_map.png')
        imview(contour={'file': 'M100-CO.mom1','levels': 
                        [1430,1460,1490,1520,1550,1580,1610,1640,1670,1700],'base':0,'unit':1}, 
               raster={'file': 'M100-CO.mom0', 'colorwedge':T,
                       'colormap': 'Rainbow 2','scaling':-1.8,'range': [0.5,20]}, 
               out='M100-CO_map.png')

        os.system('rm -rf M100-CO_contmap.png')
        imview(contour={'file': 'M100cont.image','levels': 
                        [0.00025,0.0004],'base':0,'unit':1}, 
               zoom=3,
               raster={'file': 'M100-CO.mom0', 'colorwedge':T,
                       'colormap': 'Rainbow 2','scaling':0,'range': [0.8,40]}, 
               out='M100-CO_contmap.png')

    timing()

mystep = 30
if(mystep in thesteps):
    print 'Step ', mystep, step_title[mystep]

    resrms = []
    respeak = []

    # expectation values set 14 March 2012 based on analysis using CASA 3.3
    exppeak33 = [1.11061167717, 1.08436012268]
    exprms33 = [0.000449335755548, 0.000499602989294]
    #expectation values set 15 March 2012 based on analysis using CASA active r18746
    exppeakr18746 = [1.11075341702, 1.08440160751]
    exprmsr18746 = [0.000527012278326, 0.000579607207328] # note worse RMS
    #expectation values set 17 Sept  2012 based on analysis using CASA active r21038
    # (change was due to a modification of the fluxscale task in r21037)
    exppeak21037 = [1.11501789093, 1.08849704266]
    exprms21037 = [0.000528678297997, 0.000582209031563]
    #expectation values set 17 April 2013 based on analysis using CASA trunk r23890
    # (change was due to use of Butler-JPL-Horizons 2012 in the setjy task in stead of the 2010 standard)
    exppeak = [1.18009662628,1.15104115009]
    exprms = [0.000589906820096, 0.000633491261397]

    for name in basename:

        calstat=imstat(imagename='test-'+name+'-prim_phasecal.image.tt0', region='', box='30,30,170,80')
        rms=(calstat['rms'][0])
        casalog.post( name+': rms in phase calibrator image: '+str(rms))
        calstat=imstat(imagename='test-'+name+'-prim_phasecal.image.tt0', region='')
        peak=(calstat['max'][0])
        casalog.post( name+': Peak in phase calibrator image: '+str(peak))
        casalog.post( name+': Dynamic range in phase calibrator image: '+str(peak/rms))

        resrms.append(rms)
        respeak.append(peak)

    resrmsm = []
    respeakm = []

    # expectation values set 14 March 2012 based on analysis using CASA 3.3
    exppeakm33 = 0.164112448692
    exprmsm33 = 0.0083269206807
    #expectation values set 15 March 2012 based on analysis using CASA active r18746
    exppeakmr18746 = 0.163885176182
    exprmsmr18746 = 0.00828791689128
    #expectation values set 17 Sept 2012 based on analysis using CASA active r21038
    exppeakm = 0.164736732841
    exprmsm = 0.00830688327551
    #expectation values set 17 April 2013 based on analysis using CASA active r23890
    exppeakm = 0.180228888988 
    exprmsm = 0.00912253372371 

    calstat=imstat(imagename='test-M100line.image', region='', box='42,115,65,134')
    resrmsm=(calstat['rms'][0])
    calstat=imstat(imagename='test-M100line.image', region='')
    respeakm=(calstat['max'][0])

    casalog.post( ' rms in M100: '+str(resrmsm))
    casalog.post( ' Peak in M100: '+str(respeakm))
    casalog.post( ' Dynamic range in M100: '+str(respeakm/resrmsm))


    timing()

    # print results to logger
    casalog.origin('SUMMARY')
    casalog.post("\n***** Peak and RMS of the images of the primary phase calibrator *****")
    casalog.post( "Dataset, Peak (expectation, expectation CASA 3.3), RMS (expectation, expectation CASA 3.3)")
    casalog.post( "------------------------------------------------------------------------------------------")
    for i in [0,1]:
        casalog.post( basename[i]+","+ str(respeak[i])+ "("+str(exppeak[i])+","+ str(exppeak33[i])+"),"
                      + str(resrms[i])+ "("+str(exprms[i])+","+str(exprms33[i])+")")
        
    casalog.post( "------------------------------------------------------------------------------------------")

    casalog.post( "\n***** Peak and RMS of the image of the central field of the M100 mosaic  *****")

    casalog.post( "M100: Peak (expectation, expectation CASA 3.3), RMS (expectation, expectation CASA 3.3)")
    casalog.post( "------------------------------------------------------------------------------------------")
    casalog.post( str(respeakm)+ "("+str(exppeakm)+","+ str(exppeakm33)+"),"+ str(resrmsm)+ "("+str(exprmsm)+","+str(exprmsm33)+")")
    casalog.post( "------------------------------------------------------------------------------------------")


    passed = True

    for i in [0,1]:
        peakdev = abs(respeak[i]-exppeak[i])/exppeak[i]*100.
        if (peakdev > 0.5):
            casalog.post( 'ERROR: Peak in primary phase calibrator image '+str(i)+' deviates from expectation by '+str(peakdev)+' percent.', 'WARN')
            passed = False

        rmsdev = abs(resrms[i]-exprms[i])/exprms[i]*100.
        if (rmsdev > 0.5):
            casalog.post( 'ERROR: RMS in primary phase calibrator image '+str(i)+' deviates from expectation by '+str(rmsdev)+' percent.','WARN')
            passed = False

    peakmdev = abs(respeakm-exppeakm)/exppeakm*100.
    if (peakmdev > 0.5):
        casalog.post( 'ERROR: Peak in M100 central field image '+str(i)+' deviates from expectation by '+str(peakmdev)+' percent.','WARN')
        passed = False

    rmsmdev = abs(resrmsm-exprmsm)/exprmsm*100.
    if (rmsmdev > 0.5):
        casalog.post( 'ERROR: RMS in M100 central field image '+str(i)+' deviates from expectation by '+str(rmsmdev)+' percent.','WARN')
        passed = False

    if not os.path.exists('M100-CO.mom0'):
        casalog.post( 'ERROR: M100 line cube moment 0 map was not created!','WARN')
        passed = False
        
    if not os.path.exists('M100-CO.mom1'):
        casalog.post( 'ERROR: M100 line cube moment 1 map was not created!','WARN')
        passed = False
        
    if not passed:
        raise Exception, 'Results are different from expectations by more than 0.5 percent.'

    casalog.post( "\nAll peak and RMS values within 0.5 percent of the expectation.")
    

print 'Done.'
