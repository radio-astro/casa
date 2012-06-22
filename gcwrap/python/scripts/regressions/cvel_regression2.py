#############################################################################
# $Id:$
# Test Name:                                                                #
#    Regression Test Script for the cvel task                               #
#                                                                           #
# Rationale for Inclusion:                                                  #
#    The task cvel needs to be exercised and compared to clean              #
#                                                                           # 
# Features tested:                                                          #
#    1) does cvel run without raising exceptions for input frame TOPO       #
#       and all possible output frames?                                     #
#    2) can clean process the cvel output?                                  #
#    3) does cvel+clean produce compatible results to clean-only?           #
#       (channel flux values, channel world coordinates)                    # 
#                                                                           #
# Input data:                                                               #
#     one dataset, one scan of a VLA observation provided by Crystal Brogan #
#                                                                           #
#############################################################################

# copy module needed to create dictionaries to store the results
import copy

myname = 'cvel_regression'

# default dataset name
dataset_name_orig = 'W3OH_MC.UVFITS'

# get the dataset name from the wrapper if possible
mydict = locals()
if mydict.has_key("dataset_name"):
    dataset_name_orig = mydict["dataset_name"]

def isnear(a,b,p):
    print "  ", a, b
    if(a==b):
        print "  exactly equal"
        return True
    dev = abs(a-b)
    print "  deviation = ", dev
    if(dev<=p):
        return True
    return False

def isrnear(a,b,p):
    print "  ", a, b
    if(a==b):
        print "  exactly equal"
        return True
    rdev = abs(a-b)/abs(max(a,b))
    print "  relative deviation = ", rdev
    if(rdev<=p):
        return True
    return False


# start frequencies of the grids for the different output reference frames

freqmodestart = { 'TOPO': '1.665627437e+09Hz', # 1.665261226e+09
                  'LSRK': '1.665621980e+09Hz', # 1.665255769e+09
                  'LSRD': '1.665630426e+09Hz', # 1.665264213e+09
                  'BARY': '1.665644446e+09Hz', # 1.665278230e+09
                  'GALACTO': '1.664750775e+09Hz', # 1.664384756e+09
                  'LGROUP': '1.664162945e+09Hz', # 1.663797056e+09
                  'CMB': '1.666500334e+09Hz' # 1.666133931e+09
                  }

# hanning smooth switches for the different output frames

dohanning = { 'TOPO': False,
              'LSRK': True,
              'LSRD': False,
              'BARY': False,
              'GALACTO': False,
              'LGROUP': False,
              'CMB': False
              }

#channel widths

freqmodewidth = { 'TOPO': '1.52588e+03Hz',
                  'LSRK': '1.52588e+03Hz',
                  'LSRD': '1.52589e+03Hz',
                  'BARY': '1.52590e+03Hz',
                  'GALACTO': '1.52508e+03Hz',
                  'LGROUP': '1.52454e+03Hz',
                  'CMB': '1.52668e+03Hz'
                  }

#nchan

freqmodenchan = { 'TOPO': 45,
                  'LSRK': 45,
                  'LSRD': 45,
                  'BARY': 45,
                  'GALACTO': 45,
                  'LGROUP': 45,
                  'CMB': 45
                  }

# channel to test with imstats

peakchan = '20'
otherchan1 = '5'
otherchan2 = '11'
otherchan3 = '19'
otherchan4 = '28'

testregion = '128,128,128,128' 

# storage for results
imstats = { 'TOPO': 0,'LSRK': 0, 'LSRD': 0, 'BARY': 0, 'GALACTO': 0, 'LGROUP': 0, 'CMB': 0 }
mode_imstats = { peakchan: copy.deepcopy(imstats),
                 otherchan1: copy.deepcopy(imstats),
                 otherchan2: copy.deepcopy(imstats),
                 otherchan3: copy.deepcopy(imstats),
                 otherchan4: copy.deepcopy(imstats) }
cvel_imstats = { 'frequency': copy.deepcopy(mode_imstats),
                 'radio velocity': copy.deepcopy(mode_imstats),
                 'optical velocity': copy.deepcopy(mode_imstats),
                 'channel': copy.deepcopy(mode_imstats) }
cleanonly_imstats = copy.deepcopy(cvel_imstats)

imvals = { 'TOPO': [],'LSRK': [], 'LSRD': [], 'BARY': [], 'GALACTO': [], 'LGROUP': [], 'CMB': [] }
cvel_imvals = { 'frequency': copy.deepcopy(imvals),
                'radio velocity': copy.deepcopy(imvals),
                'optical velocity': copy.deepcopy(imvals),
                'channel': copy.deepcopy(imvals) }
cleanonly_imvals = copy.deepcopy(cvel_imvals)
cvel_chanfreqs = copy.deepcopy(cvel_imvals)
cleanonly_chanfreqs = copy.deepcopy(cvel_imvals)


# Also: clean needs write access to the input MS, so we need a local copy anyway.

dataset_name = dataset_name_orig+".ms"

importuvfits(fitsfile=dataset_name_orig, vis=dataset_name)

os.system('cp -RL '+dataset_name+' input.ms')
os.system('chmod -R u+w input.ms')
os.system('cp -RL '+dataset_name+' input2.ms')
os.system('chmod -R u+w input2.ms')
clean_inputvis_local_copy = 'input.ms'

clean_inputvis_local_copy2 = 'input2.ms' # we need a second copy for the hanning smoothed cases
hanningsmooth(vis=clean_inputvis_local_copy2)


# loop over all possible output reference frames

# these are all possible frames: 
frames_to_do = ['TOPO','LSRK', 'LSRD', 'BARY', 'GALACTO', 'LGROUP', 'CMB']

# the most critical one is CMB (largest freq shift)
#frames_to_do = ['CMB']

# in order to shorten the test, leave out LSRD, GALACTO, and TOPO
#frames_to_do = ['LGROUP', 'LSRK', 'BARY', 'CMB']

for frame in frames_to_do:
    
    restfrq = 1.6654018E9
    restfreqstr = str(restfrq)+'Hz'
    
    ### frequency mode
    
    outvis = 'W3OH_'+frame+'_cvel_freq.ms'
    os.system('rm -rf '+outvis)
    
    casalog.post(outvis, 'INFO')
    
    cvel(vis=dataset_name, outputvis=outvis,
         mode='frequency',nchan=freqmodenchan[frame],
         start=freqmodestart[frame],
         width=freqmodewidth[frame],
         interpolation='linear',
         phasecenter='',
         outframe=frame,
         hanning = dohanning[frame])
    
    invis = 'W3OH_'+frame+'_cvel_freq.ms'
    iname = 'W3OH_'+frame+'_cvel_freq_clean'
    os.system('rm -rf '+iname+'.*')
    
    casalog.post(iname, 'INFO')
    
    clean(vis=invis,
          imagename=iname,
          field='',spw='',
          cell=[0.01,0.01],imsize=[256,256],
          stokes='I',
          mode='frequency',nchan=freqmodenchan[frame],
          start=freqmodestart[frame],
          width=freqmodewidth[frame],
          interpolation='linear',
          psfmode='clark',imagermode='csclean',
          scaletype='SAULT',
          niter=0,threshold='1.5mJy',
          restfreq=restfreqstr,
          phasecenter='',
          mask='',
          weighting='briggs',
          interactive=F,
          minpb=0.3,pbcor=F)
    
    cvel_imstats['frequency'][peakchan][frame] = imstat(iname+'.image', box=testregion, chans=peakchan)
    cvel_imstats['frequency'][otherchan1][frame] = imstat(iname+'.image', box=testregion, chans=otherchan1)
    cvel_imstats['frequency'][otherchan2][frame] = imstat(iname+'.image', box=testregion, chans=otherchan2)
    cvel_imstats['frequency'][otherchan3][frame] = imstat(iname+'.image', box=testregion, chans=otherchan3)
    cvel_imstats['frequency'][otherchan4][frame] = imstat(iname+'.image', box=testregion, chans=otherchan4)
    cvel_imvals['frequency'][frame] = imval(iname+'.image', box=testregion)

    ia.open(iname+'.image')
    chlist = range(freqmodenchan[frame])
    fqlist = []
    #find spectral coordinates
    for i in chlist:
        myw = ia.toworld([128,128,0,i],'n')
        fqlist.append(myw['numeric'][3])
    ia.close()
    cvel_chanfreqs['frequency'][frame] = fqlist
    
    ############
    
    iname = 'W3OH_'+frame+'_freq_clean'
    os.system('rm -rf '+iname+'.*')
    
    casalog.post(iname, 'INFO')

    cvis = clean_inputvis_local_copy    
    if(dohanning[frame]):
        casalog.post('Will Hanning smooth before cleaning ...', 'INFO')
        cvis = clean_inputvis_local_copy2

    clean(vis=cvis,
          imagename=iname,
          field='', spw='',
          cell=[0.01,0.01],imsize=[256,256],
          stokes='I',
          mode='frequency',nchan=freqmodenchan[frame],
          start=freqmodestart[frame],
          width=freqmodewidth[frame],
          outframe=frame,
          interpolation='linear',
          psfmode='clark',imagermode='csclean',
          scaletype='SAULT',
          niter=0,threshold='1.5mJy',
          restfreq=restfreqstr,
          phasecenter='',
          mask='',
          weighting='briggs',
          interactive=F,
          minpb=0.3,pbcor=F)
    
    cleanonly_imstats['frequency'][peakchan][frame] = imstat(iname+'.image', box=testregion, chans=peakchan)
    cleanonly_imstats['frequency'][otherchan1][frame] = imstat(iname+'.image', box=testregion, chans=otherchan1)
    cleanonly_imstats['frequency'][otherchan2][frame] = imstat(iname+'.image', box=testregion, chans=otherchan2)
    cleanonly_imstats['frequency'][otherchan3][frame] = imstat(iname+'.image', box=testregion, chans=otherchan3)
    cleanonly_imstats['frequency'][otherchan4][frame] = imstat(iname+'.image', box=testregion, chans=otherchan4)
    cleanonly_imvals['frequency'][frame] = imval(iname+'.image', box=testregion)

    ia.open(iname+'.image')
    chlist = range(freqmodenchan[frame])
    fqlist = []
    #find spectral coordinates
    for i in chlist:
        myw = ia.toworld([128,128,0,i],'n')
        fqlist.append(myw['numeric'][3])
    ia.close()
    cleanonly_chanfreqs['frequency'][frame] = fqlist

    
    #### velocity mode (radio)
    
    f1 = qa.quantity(freqmodestart[frame])['value']
    f2 = f1+qa.quantity(freqmodewidth[frame])['value']
    
    vrads = (restfrq-f1)/restfrq *  2.99792E8
    vradstart = str(vrads)+'m/s'
    vradw = (restfrq-f2)/restfrq *  2.99792E8 - vrads
    vradwidth = str(vradw)+'m/s'
    
    outvis = 'W3OH_'+frame+'_cvel_vrad.ms'
    os.system('rm -rf '+outvis)
    
    casalog.post(outvis, 'INFO')
    
    cvel(vis=dataset_name,outputvis=outvis,
         mode='velocity',nchan=freqmodenchan[frame],
         start=vradstart,
         width=vradwidth,
         interpolation='linear',
         phasecenter='',
         restfreq=restfreqstr,
         outframe=frame,
         hanning=dohanning[frame])
    
    invis = 'W3OH_'+frame+'_cvel_vrad.ms'
    iname = 'W3OH_'+frame+'_cvel_vrad_clean'
    os.system('rm -rf '+iname+'.*')
    
    casalog.post(iname, 'INFO')
    
    clean(vis=invis,
          imagename=iname,
          field='',spw='',
          cell=[0.01,0.01],imsize=[256,256],
          stokes='I',
          mode='velocity',nchan=freqmodenchan[frame],
          start=vradstart,
          width=vradwidth,
          interpolation='linear',
          psfmode='clark',imagermode='csclean',
          scaletype='SAULT',
          niter=0,threshold='1.5mJy',
          restfreq=restfreqstr,
          phasecenter='',
          mask='',
          weighting='briggs',
          interactive=F,
          minpb=0.3,pbcor=F)
    
    cvel_imstats['radio velocity'][peakchan][frame] = imstat(iname+'.image', box=testregion, chans=peakchan)
    cvel_imstats['radio velocity'][otherchan1][frame] = imstat(iname+'.image', box=testregion, chans=otherchan1)
    cvel_imstats['radio velocity'][otherchan2][frame] = imstat(iname+'.image', box=testregion, chans=otherchan2)
    cvel_imstats['radio velocity'][otherchan3][frame] = imstat(iname+'.image', box=testregion, chans=otherchan3)
    cvel_imstats['radio velocity'][otherchan4][frame] = imstat(iname+'.image', box=testregion, chans=otherchan4)
    cvel_imvals['radio velocity'][frame] = imval(iname+'.image', box=testregion)

    ia.open(iname+'.image')
    chlist = range(freqmodenchan[frame])
    fqlist = []
    #find spectral coordinates
    for i in chlist:
        myw = ia.toworld([128,128,0,i],'n')
        fqlist.append(myw['numeric'][3])
    ia.close()
    cvel_chanfreqs['radio velocity'][frame] = fqlist

    ###############

    iname = 'W3OH_'+frame+'_vrad_clean'
    os.system('rm -rf '+iname+'.*')
    
    casalog.post(iname, 'INFO')
    
    cvis = clean_inputvis_local_copy    
    if(dohanning[frame]):
        casalog.post('Will Hanning smooth before cleaning ...', 'INFO')
        cvis = clean_inputvis_local_copy2
    
    clean(vis=cvis,
          imagename=iname,
          field='', spw='',
          cell=[0.01,0.01],imsize=[256,256],
          stokes='I',
          mode='velocity',nchan=freqmodenchan[frame],
          start=vradstart,
          width=vradwidth,
          outframe=frame,
          interpolation='linear',
          psfmode='clark',imagermode='csclean',
          scaletype='SAULT',
          niter=0,threshold='1.5mJy',
          restfreq=restfreqstr,
          phasecenter='',
          mask='',
          weighting='briggs',
          interactive=F,
          minpb=0.3,pbcor=F)
    
    cleanonly_imstats['radio velocity'][peakchan][frame] = imstat(iname+'.image', box=testregion, chans=peakchan)
    cleanonly_imstats['radio velocity'][otherchan1][frame] = imstat(iname+'.image', box=testregion, chans=otherchan1)
    cleanonly_imstats['radio velocity'][otherchan2][frame] = imstat(iname+'.image', box=testregion, chans=otherchan2)
    cleanonly_imstats['radio velocity'][otherchan3][frame] = imstat(iname+'.image', box=testregion, chans=otherchan3)
    cleanonly_imstats['radio velocity'][otherchan4][frame] = imstat(iname+'.image', box=testregion, chans=otherchan4)
    cleanonly_imvals['radio velocity'][frame] = imval(iname+'.image', box=testregion)

    ia.open(iname+'.image')
    chlist = range(freqmodenchan[frame])
    fqlist = []
    #find spectral coordinates
    for i in chlist:
        myw = ia.toworld([128,128,0,i],'n')
        fqlist.append(myw['numeric'][3])
    ia.close()
    cleanonly_chanfreqs['radio velocity'][frame] = fqlist

    #### velocity mode (optical)
    
    lambda0 = 2.99792E8/restfrq
    lambda1 = 2.99792E8/f1
    lambda2 = 2.99792E8/f2
    vopts = (lambda1-lambda0)/lambda0 * 2.99792E8
    voptw = (lambda2-lambda0)/lambda0 * 2.99792E8 - vopts
    voptstart = str(vopts)+'m/s'
    voptwidth = str(voptw)+'m/s'
    
    outvis = 'W3OH_'+frame+'_cvel_vopt.ms'
    os.system('rm -rf '+outvis)
    
    casalog.post(outvis, 'INFO')
    
    cvel(vis=dataset_name, outputvis=outvis,
         mode='velocity',nchan=freqmodenchan[frame],
         start=voptstart,
         width=voptwidth,
         interpolation='linear',
         phasecenter='',
         restfreq=restfreqstr,
         outframe=frame,
         veltype='optical',
         hanning=dohanning[frame])
    
    invis = 'W3OH_'+frame+'_cvel_vopt.ms'
    iname = 'W3OH_'+frame+'_cvel_vopt_clean'
    os.system('rm -rf '+iname+'.*')
    
    casalog.post(iname, 'INFO')
    
    clean(vis=invis,
          imagename=iname,
          field='',spw='',
          cell=[0.01,0.01],imsize=[256,256],
          stokes='I',
          mode='velocity',nchan=freqmodenchan[frame],
          start=voptstart,
          width=voptwidth,
          interpolation='linear',
          psfmode='clark',imagermode='csclean',
          scaletype='SAULT',
          niter=0,threshold='1.5mJy',
          restfreq=restfreqstr,
          phasecenter='',
          mask='',
          weighting='briggs',
          interactive=F,
          minpb=0.3,pbcor=F,
          veltype='optical')
    
    cvel_imstats['optical velocity'][peakchan][frame] = imstat(iname+'.image', box=testregion, chans=peakchan)
    cvel_imstats['optical velocity'][otherchan1][frame] = imstat(iname+'.image', box=testregion, chans=otherchan1)
    cvel_imstats['optical velocity'][otherchan2][frame] = imstat(iname+'.image', box=testregion, chans=otherchan2)
    cvel_imstats['optical velocity'][otherchan3][frame] = imstat(iname+'.image', box=testregion, chans=otherchan3)
    cvel_imstats['optical velocity'][otherchan4][frame] = imstat(iname+'.image', box=testregion, chans=otherchan4)
    cvel_imvals['optical velocity'][frame] = imval(iname+'.image', box=testregion)

    ia.open(iname+'.image')
    chlist = range(freqmodenchan[frame])
    fqlist = []
    #find spectral coordinates
    for i in chlist:
        myw = ia.toworld([128,128,0,i],'n')
        fqlist.append(myw['numeric'][3])
    ia.close()
    cvel_chanfreqs['optical velocity'][frame] = fqlist

    #######################
    
    iname = 'W3OH_'+frame+'_vopt_clean'
    os.system('rm -rf '+iname+'.*')
    
    casalog.post(iname, 'INFO')

    cvis = clean_inputvis_local_copy    
    if(dohanning[frame]):
        casalog.post('Will Hanning smooth before cleaning ...', 'INFO')
        cvis = clean_inputvis_local_copy2
    
    clean(vis=cvis,
          imagename=iname,
          field='', spw='',
          cell=[0.01,0.01],imsize=[256,256],
          stokes='I',
          mode='velocity',nchan=freqmodenchan[frame],
          start=voptstart,
          width=voptwidth,
          outframe=frame,
          interpolation='linear',
          psfmode='clark',imagermode='csclean',
          scaletype='SAULT',
          niter=0,threshold='1.5mJy',
          restfreq=restfreqstr,
          phasecenter='',
          mask='',
          weighting='briggs',
          interactive=F,
          minpb=0.3,pbcor=F,
          veltype='optical')
    
    cleanonly_imstats['optical velocity'][peakchan][frame] = imstat(iname+'.image', box=testregion, chans=peakchan)
    cleanonly_imstats['optical velocity'][otherchan1][frame] = imstat(iname+'.image', box=testregion, chans=otherchan1)
    cleanonly_imstats['optical velocity'][otherchan2][frame] = imstat(iname+'.image', box=testregion, chans=otherchan2)
    cleanonly_imstats['optical velocity'][otherchan3][frame] = imstat(iname+'.image', box=testregion, chans=otherchan3)
    cleanonly_imstats['optical velocity'][otherchan4][frame] = imstat(iname+'.image', box=testregion, chans=otherchan4)
    cleanonly_imvals['optical velocity'][frame] = imval(iname+'.image', box=testregion)

    ia.open(iname+'.image')
    chlist = range(freqmodenchan[frame])
    fqlist = []
    #find spectral coordinates
    for i in chlist:
        myw = ia.toworld([128,128,0,i],'n')
        fqlist.append(myw['numeric'][3])
    ia.close()
    cleanonly_chanfreqs['optical velocity'][frame] = fqlist


# end loop over frames

# Analysis

passed = True
tolerance = 0.001 # absolute tolerance [Jy/beam]
rtolerance = 0.07 # relative tolerance
numpoints = 0.
avdev = 0.
maxdev = 0.
maxdevat = " "
problems = 0
for frame in frames_to_do:
    for mode in ['frequency', 'radio velocity', 'optical velocity']:
        # make comparison plot
        sparr = cleanonly_imvals[mode][frame]['data']
        sparr_cvel = cvel_imvals[mode][frame]['data']
        fqarr = pl.array(cleanonly_chanfreqs[mode][frame]) * 1E6 # convert to Hz
        fqarr_cvel = pl.array(cvel_chanfreqs[mode][frame]) * 1E6 # convert to Hz

        pl.plot(fqarr, sparr, 'b', linewidth=1.0)
        pl.plot(fqarr_cvel, sparr_cvel, 'r', linewidth=1.0)
        #pl.xlim(1665.62,1665.72)
        pl.xlabel(frame+' Frequency (MHz)')
        pl.ylabel('Flux Density (Jy/beam)')
        pl.title('W3OH '+frame+', '+mode+'-mode cvel+clean = red, clean-only = blue')
        pl.savefig('testcvelclean'+frame+mode[0]+'.png',format='png')
        pl.close()
        
        for chan in mode_imstats.keys():
            isok = true
            c1 = cleanonly_imstats[mode][chan][frame]['max']
            c2 = cvel_imstats[mode][chan][frame]['max']
            print "Testing ", frame, ", ",  mode, ", Hanning ", dohanning[frame], ", box ", testregion, ", channel ", chan, " ..."
            if(abs(c1-c2) > maxdev):
                maxdev = abs(c1-c2)
                maxdevat = mode+" mode for output frame "+frame\
                           +":\n    cvel+clean finds max flux in channel "+str(chan)+" to be "+str(c2)\
                           +"\n    clean-only finds max flux in channel "+str(chan)+" to be "+str(c1)
            if not (isnear(c1,c2, tolerance) or isrnear(c1,c2, rtolerance)):
                print " ** Problem in ", mode, " mode for output frame ", frame, ":"
                print "     cvel+clean finds max flux in channel ", chan, " to be ", c2
                print "     clean-only finds max flux in channel ", chan, " to be ", c1
                passed = False
                isok = False
                problems +=1

            avdev += abs(c1-c2)
            numpoints += 1.
            
            s1 = cleanonly_imstats[mode][chan][frame]['maxposf']
            s2 = cvel_imstats[mode][chan][frame]['maxposf']
            if(not s1 == s2):
                print " ** Problem in ", mode, " mode for output frame ", frame, ":"
                print "     cvel+clean finds world coordinates for channel ", chan, " to be ", s2
                print "     clean-only finds world coordinates for channel ", chan, " to be ", s1
                passed = False
                isok = False
                problems +=1
            else:
                print "  World coordinates identical == ", s2

            if isok:
                print "... OK"      

if(numpoints > 0.):
    print numpoints, " spectral points compared, average deviation = ", avdev/numpoints, " Jy"
    print "   maximum deviation = ", maxdev, " in ", maxdevat 
                    
if passed:
    print "PASSED"
else:
    print "Execution successful but found ", problems, " issues in analysis of results."
    print "FAILED"
    raise
