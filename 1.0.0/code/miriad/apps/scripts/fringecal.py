#####
#
# Fill, Calibrate and Test Map a standard short CARMA fringetest
# 
# you exectute this script from within casapy using
#    execfile('fringecal.py')
#
#####

from glob import glob as filesearch
import os


#  default for fringe test

vis    = 'fringe.3C273.2008jun18.4.miriad'
source = '3C273'
ms     = 'test1.ms'
 

def fringe(vis=vis, ms=ms, field='0'):
    do_bandpass = False
    do_setflux  = False
    do_gaincal  = False

    # carmafiller
    cmd = 'carmafiller vis=%s ms=%s' % (vis,ms)
    os.system(cmd)

    # make a listing
    listobs(vis=ms)

    # FldId     FieldName
    #   0       NOISE_C
    #   1       3C273_C
    #
    # SpwId
    #  0-5   wideband data (15 channels, LSB/USB)
    
    print '--start calibration over: run cleancal on data--'

    clearcal(vis=ms)

    # Bandpass calibration

    if do_bandpass:
        print '--Bandpass--'
        default('bandpass')
        bandpass(vis=ms,caltable=ms+'.bcal',gaintable='',gainfield='',
             interp='',field=field, spw='',selectdata=False,gaincurve=False,
             opacity=0.0,bandtype='B',solint='1000s',combine='scan',refant='8',
             solnorm=False)
        print '--Bandpass done--'

    #####
    # setjy and gain calibration
    # using 3C273 and assuming pt source with flux density 17 Jy
    # using same value as determined from MIRIAD's CalGet, for consistency
    #
    # applying same flux to all spws 
    #####
        
    if do_setflux:
        setjy(vis=ms, field=field,fluxdensity=[17.0,0.,0.,0.], spw='0~5')

    #####
    # Gain calibration
    # gaintable=*.bcal applies bandpass cal from earlier; 
    # caltable=        stores output gain solutions
    #
    # Added 9/2, more edits 9/28/09: initial phase-only calibration on 
    # calibrators to stabilize against phase variations over a scan
    # Do gain calibration on both phase and flux cals
    # 29sep09: for consistency with MIRIAD reduction, use wideband only to 
    #   calibrate gains
    #####

    
    # To match miriad calibration parameters (using USB wideband only,
    #   averaging over 10minute intervals)
    # Note:  30oct09: needed to add combine='scan' to ensure 10min intervals
    #
    # This case: using the original wideband and letting CASA average

    if do_gaincal:
        gaincal(vis=ms,caltable=ms+'.gcal',gaintable=ms+'.bcal',
            gainfield=field, field=field, spw='0~5', gaintype='G',
            minsnr=2.0, refant='8', gaincurve=False, opacity=0.0, 
            solint='600s', combine='scan')

    # To match miriad calibration parameters (using USB wideband only,
    #   averaging over 10minute intervals)
    #
    # This case: using output wideband average channel read in from MIRIAD
    #gaincal(vis=catfile,caltable=msdir+proj+'.phacal',gaintable=msdir+proj+'.bcal',
    #        gainfield='2', field='2,3', spw='9', gaintype='G',calmode='p',
    #        minsnr=2.0, refant='8', gaincurve=False, opacity=0.0, 
    #       solint='600s', combine='scan')
    
        print '--gaincal done--'

    print '--fluxscale (nothing) done--'

    #####
    # Apply calibration to calibrator and source data
    # spwmap parameter is set to use the wideband data to set the gains for all
    # spw's
    # gtable already applied to ftable, so don't include in gaintable list
    #
    # 9/28/09: fix spwmap to default so that wides and narrows get appropriately applied,
    # explicitly listed all cal files to be applied
    # spwmap=[[0,0,0],[0,0,0],[]] means (assuming a total of 3 spw's):
    #           wideband spw=0 gets applied to all spw for gain calibration
    #           wideband spw=0 gets applied to all spw for flux calibration
    #           each spw gets applied to itself for bandpass calibration
    #####

    # for version using original wideband channels
    if False:
        applycal(vis=ms,field='',spw='', gaincurve=False, opacity=0.0, 
                 gaintable=[ms+'.gcal',ms+'.bcal'],
                 gainfield=[field])
        # spwmap=[[3,3,3,3,3,3,3,3,3,3,3,3],[3,3,3,3,3,3,3,3,3,3,3,3],[]])

    # for version using MIRIAD-produced wideband average
    #applycal(vis=catfile,field='',spw='', gaincurve=False, opacity=0.0, 
    #         gaintable=[msdir+proj+'.gcal',msdir+proj+'.fcal',msdir+proj+'.bcal'],
    #         gainfield=['3','3','2'],spwmap=[[9,9,9,9,9,9],[9,9,9,9,9,9],[]])


    #####
    # Imaging the calibrator(s)
    #####

    clean(vis=ms,imagename=ms+'.fill.clean',cell=[2.,2.],imsize=[128,128],
          field=field,spw='1',threshold=10., psfmode='hogbom',
#          mode='channel', nchan=2, start=5, width=1,
          mode='mfs',
          weighting='natural',
#          weighting='uniform',
#          weighting='briggs', robust=0,
          niter=1000,stokes='I')

def simple(vis=vis, ms=None):
    if ms==None:
        ms = vis + ".ms"
    print '--simple filler --',vis
    # carmafiller
    cmd = 'rm -rf %s*; carmafiller vis=%s ms=%s' % (ms,vis,ms)
    print "CMD: ",cmd
    os.system(cmd)
    print '--simple listobs --'
    # make a listing
    listobs(vis=ms)

    print '--simple clean --'
    clean(vis=ms,imagename=ms+'.fill.clean',
          cell=[2.,2.],
          imsize=[128,128],
          field='0',
          threshold=10., 
          psfmode='hogbom',
#          psfmode='clark',
          spw='5',
#          mode='channel', nchan=-1, 
#          mode='channel', nchan=-1, start=0, width=1,
          mode='channel', nchan=15,
#          mode='velocity', nchan=6, start='-300km/s', width='100km/s',
#          mode='mfs',
          weighting='natural',
#          weighting='uniform',
#          weighting='briggs', robust=0,
          niter=10,
          stokes='I',
          )
    print '--simple done --'
    cmd = 'du -s %s*' % ms
    os.system(cmd)

def old(ms=ms):
    print '--old clean --',ms
    # make a listing
    listobs(vis=ms)

    clean(vis=ms,imagename=ms+'.fill.clean',
          cell=[2.,2.],
          imsize=[128,128],
          field='0',
          threshold=10., 
          psfmode='hogbom',
#          psfmode='clark',
#          spw='0',
#          mode='channel', nchan=-1, 
#          mode='channel', nchan=-1, start=0, width=1,
          mode='channel', 
#          mode='velocity', nchan=6, start='-300km/s', width='100km/s',
#          mode='mfs',
          weighting='natural',
#          weighting='uniform',
#          weighting='briggs', robust=0,
          niter=10,
          stokes='I',
          )
    print '--simple done --'
    cmd = 'du -s %s*' % ms
    os.system(cmd)

def strange(vis1=vis, vis2=vis, ms=ms):
    print '--simple2 filler --',vis1,vis2

    if True:
        cmd = 'rm -rf %s*; carmafiller vis=%s ms=%s; ls -l %s/' % (ms,vis1,ms,ms)
        print "CMD: ",cmd
        os.system(cmd)
        print '--simple clean1 --'
        clean(vis=ms,imagename=ms+'_im',
          cell=[2.,2.],imsize=[128,128],field='0',spw='0',
          threshold=10., psfmode='hogbom',weighting='natural',
          mode='channel', nchan=15,
          niter=10,stokes='I'
          )

    if True:
        # uncommenting out the next line will fix this run, which normally always is ok
        #ms=ms+"_"
        cmd = 'rm -rf %s*; carmafiller vis=%s ms=%s; sum %s/* %s/*/*' % (ms,vis2,ms,ms,ms)
        print "CMD: ",cmd
        os.system(cmd)
        print '--simple clean2 --'
        clean(vis=ms,imagename=ms+'_im',
          cell=[2.,2.],imsize=[128,128],field='0',spw='0',
          threshold=10., psfmode='hogbom',weighting='natural',
          mode='channel', nchan=15,
          niter=10,stokes='I'
          )
    print '--simple2 done --'        

if __name__ == '__main__':
    #fringe(vis,ms)
    #fringe('vis3','test1.ms','0')
    #simple('vis3','test1.ms')
    #simple('vis4','test1.ms')
    #simple('vis5','test1.ms')

    #
    #simple('vis13','test2.ms') - bad
    #simple('vis14b','test3.ms') - ok
    #simple('vis14b','test2.ms') - ok
    #simple('vis13','test4.ms')- bad
    #simple('vis14b','test4.ms') - bad
    #simple('vis12','test5.ms')
    #simple('vis14b','test5.ms')
    # now go out of casapy and do again
    #
    # reproduce the error
    #strange('vis12','vis14b','test6.ms')
    #strange('vis14b','vis14b','test6.ms')
    #
    #simple('vis13','test13.ms') # - bad
    #simple('vis3') -ok
    #old('vis12.ms')
    #old('c0437.ms')
    #simple('c0437')
    old('orimsr123b.ms3')
