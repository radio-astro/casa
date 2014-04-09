#################################
## Tool level test script for the refactored imager framework
#################################
from refimagerhelper import PySynthesisImager, PyParallelContSynthesisImager,PyParallelCubeSynthesisImager, PyParallelDeconvolver, PyParallelImagerHelper,ImagerParameters

import commands
#### Specify parameters.
def getparams(testnum=1,testid=0, parallelmajor=False,parallelminor=False,parallelcube=False):

     # Iteration parameters - common to all tests below
     niter=200
     cycleniter=100
     threshold=0.001
     loopgain=0.1

     # Interaction ON or OFF
     interactive=False


     if(testnum==18): ## mode=cubesrc (For interface test purpose only, the mode is not fully implemeted yet)
          casalog.post("==================================");
          casalog.post("Test 18 image-field, cubesrc --- Real Imaging with various cube parameter specifications");
          #casalog.post("==================================");
          paramList = ImagerParameters(msname='DataTest/point_twospws.ms', field='0',\
                                       spw='0',\
                                       usescratch=True,readonly=True,\
                                       imagename="CubesrcTest", mode='cubesrc',\
                                       nchan=10,\
                                       start="1.1GHz",\
                                       step="",\
                                       veltype='radio',\
                                       sysvel="-3000km/s",\
                                       sysvelframe="LSRK",\
                                       restfreq=["1.0GHz"],\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)

     if(testnum==17): ## mode=cubedata (For interface test purpose only, the mode is not full implemented yet)
          casalog.post("==================================");
          casalog.post("Test 17 image-field, cubedata --- Real Imaging with various cube parameter specifications");
          #casalog.post("==================================");
          paramList = ImagerParameters(msname='DataTest/point_twospws.ms', field='0',\
                                       spw='0',\
                                       usescratch=True,readonly=True,\
                                       imagename="CubedataTest", mode='cubedata',\
                                       nchan=10,\
                                       start=5,\
                                       step=1,\
                                       veltype='radio',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)

     if(testnum==16):  ## 2 image-fields, mfs - one with nterms 1 and one with nterms 2
          casalog.post("==================================");
          casalog.post("Test 16 image-fields, mfs (one with nterms=1. one with nterms=2).");
          casalog.post("==================================");
          
          write_file('out16.txt', 'imagename=mytest1\nnchan=1\nimsize=[80,80]\ncellsize=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nntaylorterms=2\nmtype=multiterm\nrestfreq=[1.5GHz]\nalgo=msmfs')
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',\
                                       field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       outlierfile='out16.txt',\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1, start='1.0GHz', step='4.0GHz',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)
     
     if(testnum==15):  ## 2 image-fields, mfs, Overlapping models. Both multi-term
          casalog.post("==================================");
          casalog.post("Test 15 two overlapping image-fields, mfs, both multi term");
          casalog.post("==================================");
          
          write_file('out15.txt', 'imagename=mytest1\nnchan=1\nimsize=[200,200]\ncellsize=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:59:02.426 +40.51.14.559\n')
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',\
                                       field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       outlierfile='out15.txt',\
                                       mode='mfs',\
                                       ntaylorterms=2,mtype='multiterm',restfreq=['1.5GHz'],\
                                       algo='msmfs',\
                                       imagename='mytest0', nchan=1, start='1.0GHz', step='4.0GHz',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:58:39.580 +40.55.55.931",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)


     if(testnum==14):  ## 2 image-fields, mfs, Overlapping models. Single term
          casalog.post("==================================");
          casalog.post("Test 14 two overlapping image-fields, mfs, single term");
          casalog.post("==================================");
          
          write_file('out14.txt', 'imagename=mytest1\nnchan=1\nimsize=[200,200]\ncellsize=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:59:02.426 +40.51.14.559')
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',\
                                       field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       outlierfile='out14.txt',\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1, start='1.0GHz', step='4.0GHz',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:58:39.580 +40.55.55.931",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)


     if(testnum==13): ## 1 image-field, cube --- Real Imaging with various cube parameter specifications
          casalog.post("==================================");
          casalog.post("Test 13 image-field, cube --- Real Imaging with various cube parameter specifications");
          #casalog.post("==================================");
          qfstart=qa.quantity("1.1GHz")
          qvstart=qa.quantity("-59958.5km/s")
          mfstart=me.frequency('LSRK',qa.quantity("1.1GHz"))
          mvstart=me.radialvelocity('BARY',qa.quantity("-59976.1km/s"))
          dop = me.todoppler('radio',mfstart,qa.quantity('1.0GHz'))
          qvstep = qa.quantity("11991.700km/s")
          mvstep = me.radialvelocity('TOPO',qvstep)

          testList = {
                      0:{'imagename':'Cubetest_chandefstdefstep','spw':'0','start':0,'step':1,'frame':'LSRK',
                        'desc':'channel, default start and step, LSRK'},
                      1:{'imagename':'Cubetest_chandefstdefsteptopo','spw':'0','start':0,'step':1, 'frame':'TOPO',
                        'desc':'channel, default start and step, TOPO'},
                      2:{'imagename':'Cubetest_chandefststep2','spw':'0','start':0,'step':2, 'frame':'LSRK',
                        'desc':'channel, default start, step=2, LSRK'},
                      3:{'imagename':'Cubetest_chanst5wd1','spw':'0','start':5,'step':1, 'frame':'LSRK',
                        'desc':'channel, start=5, default step, LSRK'},
                      4:{'imagename':'Cubetest_chandefstwd1spwsel','spw':'0:5~19','start':0,'step':1, 'frame':'LSRK',
                        'desc':'channel, spw=0:5~19, LSRK'},
                      5:{'imagename':'Cubetest_freqdefstwd2','spw':'0','start':'','step':'40MHz','frame':'TOPO',
                        'desc':'frequency, default start, step=\'40MHz\', TOPO'},
                      6:{'imagename':'Cubetest_freqst5defwd','spw':'0','start':'1.1GHz','step':'','frame':'TOPO',
                        'desc':'frequency, start=\'1.1GHz\', default step, TOPO'},
                      #7:{'imagename':'Cubetest_freqst5defwdspwsel','spw':'0:6~19','start':'1.1GHz','step':'','frame':'TOPO',
                      7:{'imagename':'Cubetest_freqst5defwdspwsel','spw':'0:4~19','start':'1.1GHz','step':'','frame':'TOPO',
                        'desc':'frequency, start=\'1.1GHz\', default step, spw=0:6~19, TOPO'},
                      8:{'imagename':'Cubetest_freqst10wdm','spw':'0','start':'1.2GHz','step':'-20MHz','frame':'TOPO',
                        'desc':'frequency, start=\'1.2GHz\', step=\'-20MHz\', TOPO'},
                      9:{'imagename':'Cubetest_veldefstwd2','spw':'0','start':'','step':'11991.700km/s','frame':'TOPO',
                        'desc':'frequency, default start, step=\'11991.700km/s\', TOPO'},
                     10:{'imagename':'Cubetest_veldefstwd2m','spw':'0','start':'','step':'-11991.700km/s','frame':'TOPO',
                        'desc':'velocity, default start, step=\'-11991.700km/s\', TOPO'},
                     11:{'imagename':'Cubetest_velst10defwd','spw':'0','start':'-59958.5km/s','step':'','frame':'TOPO',
                        'desc':'velocity, start=\'-59958.5km/s\', default step, TOPO'},
                     12:{'imagename':'Cubetest_velst10defwdbary','spw':'0','start':'-59976.1km/s','step':'','frame':'BARY',
                        'desc':'velocity, start=\'-59976.1km/s\', default step, BARY'},
                     #13:{'imagename':'Cubetest_optvelst10defwdlsrk','spw':'0','start':'-49962.6km/s','step':'4234.40km/s',
                     13:{'imagename':'Cubetest_optvelst10defwdlsrk','spw':'0','start':'-49962.6km/s','step':'5500.0km/s',
                        'veltype':'optical','frame':'LSRK',
                        'desc':'velocity, start=\'-49962.6km/s\', default step, veltype=optical LSRK'},
                     14:{'imagename':'Cubetest_stqfreqdefwd','spw':'0','start':qfstart,'step':'', 'veltype':'radio','frame':'',
                        'desc':'frequency, start=%s, default step, veltype=radio TOPO' % qfstart},
                     15:{'imagename':'Cubetest_stmfreqdefwd','spw':'0','start':mfstart,'step':'', 'veltype':'radio','frame':'',
                        'desc':'frequency, start=%s, default step, veltype=radio LSRK' % mfstart},
                     16:{'imagename':'Cubetest_stqveldefwd','spw':'0','start':qvstart,'step':'','frame':'TOPO',
                        'desc':'velocity, start=%s, default step, TOPO' % qvstart},
                     17:{'imagename':'Cubetest_stmveldefwd','spw':'0','start':mvstart,'step':'','frame':'TOPO',
                        'desc':'velocity, start=%s, default step, BARY' % mvstart},
                     18:{'imagename':'Cubetest_veldefstqvstep','spw':'0','start':'','step':qvstep,'frame':'TOPO',
                        'desc':'velocity, default start, step=%s, TOPO' % qvstep},
                     19:{'imagename':'Cubetest_veldefstmvstep','spw':'0','start':'','step':mvstep,'frame':'TOPO',
                        'desc':'velocity, default start, step=%s, TOPO' % mvstart},
                     20:{'imagename':'Cubetest_stdopdefwd','spw':'0','start':dop,'step':'','frame':'TOPO',
                        'desc':'doppler, start=%s, default step, LSRK' % dop}
                    }

          casalog.post("Sub-test "+str(testid)+":"+testList[testid]['desc']);
          casalog.post("==================================");
          if testList[testid].has_key('veltype'):
               inveltype=testList[testid]['veltype']
          else:
               inveltype='radio'
          paramList = ImagerParameters(msname='DataTest/point_twospws.ms', field='0',\
                                       spw=testList[testid]['spw'],\
                                       usescratch=True,readonly=True,\
                                       imagename=testList[testid]['imagename'], mode='cube',\
                                       nchan=10,\
                                       start=testList[testid]['start'],\
                                       step=testList[testid]['step'],\
                                       veltype=inveltype,\
                                       frame=testList[testid]['frame'],\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)



     if(testnum==12):  ## 1 image-field, mfs --- WB AWP ( multi term )
          casalog.post("==================================");
          casalog.post("Test 12 image-field, mfs --- WB AWP( multi term )");
          casalog.post("==================================");
          paramList = ImagerParameters(msname='DataTest/reg_mawproject.ms',
                                       field='*',scan='',
                                       spw='*',\
                                       usescratch=True,readonly=True,\
                                       imagename='mytest0', nchan=1,start='1.5GHz', step='1.0GHz',\
                                       imsize=[512,512],\
                                       cellsize=['10.0arcsec','10.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ntaylorterms=2,mtype='multiterm',restfreq=['1.5GHz'],\
                                       ftmachine='awprojectft', 
                                       startmodel='', weighting='natural',\
                                       aterm=True, psterm=False, mterm=True,\
                                       wbawp = True, 
                                       cfcache = "perm.wb.nt2.mytest0.cfcache",\
                                       dopointing = False, dopbcorr = True, conjbeams = True, 
                                       computepastep =360.0, rotatepastep =5.0,\
                                       algo='msmfs',\
                                       pblimit=0.1,normtype='flatnoise',
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)

     if(testnum==11):  ## 1 image-field, mfs --- WB AWP ( single term )
          casalog.post("==================================");
          casalog.post("Test 11 image-field, mfs --- WB AWP (single term)");
          casalog.post("==================================");
          ## Note : For this test, if doMajor peak comes to 0.006 instead of 0.69, 
          ##           it means unnormalized weight is being used (refilled perhaps but not normed).
          paramList = ImagerParameters(msname='DataTest/reg_mawproject.ms',
                                       field='1',scan='',
                                       spw='*',\
                                       usescratch=True,readonly=True,\
                                       imagename='mytest0', nchan=1,start='1.5GHz', step='1.0GHz',\
                                       imsize=[512,512],\
                                       cellsize=['10.0arcsec','10.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='awprojectft', 
                                       startmodel='', weighting='natural',\
                                       aterm=True, psterm=False, mterm=True,\
                                       wbawp = True, 
                                       cfcache = "perm.wb.mytest0.cfcache",\
                                       dopointing = False, dopbcorr = True, conjbeams = True, 
                                       computepastep =360.0, rotatepastep =5.0,\
                                       algo='hogbom',\
                                       pblimit=0.1,normtype='flatnoise',
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)

     if(testnum==10):  ## 1 image-field, mfs --- Narrow-band AWP
          casalog.post("==================================");
          casalog.post("Test 10 image-field, mfs --- WB AWP");
          casalog.post("==================================");
          paramList = ImagerParameters(msname='DataTest/reg_mawproject.ms', #_offcenter.ms',
                                       field='*',scan='',
                                       spw='1',\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,start='1.5GHz', step='0.3GHz',\
                                       imsize=[512,512],\
                                       cellsize=['10.0arcsec','10.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='awprojectft', 
                                       startmodel='', weighting='natural',\
                                       aterm=True, psterm=False, mterm=True,\
                                       wbawp = False, 
                                       cfcache = "perm.mytest0.cfcache.mos",#.offcenter",\
                                       dopointing = False, dopbcorr = True, conjbeams = True, 
                                       computepastep =360.0, rotatepastep =5.0,\
                                       algo='hogbom',\
                                       #pblimit=0.1,normtype='flatsky',
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)


     if(testnum==9):  ## Image domain mosaic for single-term (narrow band )
          casalog.post("==================================");
          casalog.post("Test 9 : Image domain mosaic for single-term");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/reg_mawproject.ms',\
                                       field='',spw='1',scan='',\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,start='1.5GHz', step='4.0GHz',\
                                       ntaylorterms=1,mtype='imagemosaic',restfreq=['1.5GHz'],\
                                       imsize=[512,512],\
                                       cellsize=['10.0arcsec','10.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       #phasecenter=1,\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       #pblimit=0.1,normtype='flatsky',
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)
 

     if(testnum==8):  # MTMFS + Facets
          casalog.post("==================================");
          casalog.post("Test 8 : 1 image-field, mfs, nt=1, 2x2 facets ");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,start='1.0GHz', step='4.0GHz',\
                                       ntaylorterms=2,mtype='multiterm',restfreq=['1.5GHz'],\
                                       imsize=[200,200], facets=2,\
                                       cellsize=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:00.2 +40.50.15.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='msmfs',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)


     if(testnum==7):  # MTMFS 
          casalog.post("==================================");
          casalog.post("Test 7 : 1 image-field, mfs, With ntaylorterms=2 ");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,start='1.0GHz', step='4.0GHz',\
                                       ntaylorterms=2,mtype='multiterm',
                                       reffreq='1.6GHz',\
                                       #restfreq=['1.5GHz'],\
                                       imsize=[200,200],\
                                       cellsize=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:00.2 +40.50.15.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='msmfs',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)


     if(testnum==6): ## Facetted imaging
          casalog.post("==================================");
          casalog.post("Test 6 : 1 image-field, mfs, nt=1, 2x2 facets ");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1, start='1.0GHz', step='4.0GHz',\
                                       ntaylorterms=1,mtype='default',restfreq=['1.5GHz'],\
                                       imsize=[200,200], facets=2,\
                                       cellsize=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:00.2 +40.50.15.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)


     if(testnum==5):  ## 1 image-field, mfs, multiple input MSs --- Real Imaging.
          casalog.post("==================================");
          casalog.post("Test 5 image-field, mfs, multiple input MSs --- Real Imaging.");
          casalog.post("==================================");
          paramList = ImagerParameters(msname=['DataTest/point_onespw0.ms','DataTest/point_onespw1.ms'],\
                                       field='0',spw=['0','0'],\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1, start='1.0GHz', step='4.0GHz',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)


     if(testnum==4):  ## 2 image-fields, one cube, one mfs --- Real Imaging.
          casalog.post("==================================");
          casalog.post("Test 4 image-fields, one cube, one mfs --- Real Imaging.");
          casalog.post("==================================");
          
          write_file('out4.txt', 'imagename=mytest1\nnchan=1\nimsize=[80,80]\ncellsize=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543\nfreqstep=4.0GHz')
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',\
                                       field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       outlierfile='out4.txt',\
                                       mode='cube',\
                                       imagename='mytest0', nchan=2, start='1.0GHz', step='1.0GHz',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)


     
     if(testnum==3):  ## 2 image-fields, mfs --- Real Imaging.
          casalog.post("==================================");
          casalog.post("Test 3 image-fields, mfs --- Real Imaging.");
          casalog.post("==================================");
          
          write_file('out3.txt', 'imagename=mytest1\nnchan=1\nimsize=[80,80]\ncellsize=[8.0arcsec,8.0arcsec]\nphasecenter=J2000 19:58:40.895 +40.55.58.543')
          paramList = ImagerParameters(msname='DataTest/twopoints_twochan.ms',\
                                       field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       outlierfile='out3.txt',\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1, start='1.0GHz', step='4.0GHz',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'], 
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)
     
     
     if(testnum==2):  ## 1 image-field, cube --- Real Imaging.
          casalog.post("==================================");
          casalog.post("Test 2 image-field, cube --- Real Imaging.");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/point_twospws.ms', field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       mode='cube',\
                                       imagename='mytest0', nchan=10,start='1.0GHz', step='40MHz',\
                                       imsize=[100,100],\
                                       cellsize=['8.0arcsec','8.0arcsec'],\
                                       phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ftmachine='GridFT', startmodel='', weighting='natural',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)
     

     if(testnum==1):  ## 1 image-field, mfs --- Real Imaging.
          casalog.post("==================================");
          casalog.post("Test 1 image-field, mfs --- Real Imaging.");
          casalog.post("==================================");
          
          paramList = ImagerParameters(msname='DataTest/point_twospws.ms',field='0',spw='0',\
                                       usescratch=True,readonly=True,\
                                       mode='mfs',\
                                       imagename='mytest0', nchan=1,start='1.0GHz', step='4.0GHz',\
                                       imsize=[110,110],\
                                       cellsize=['8.0arcsec','8.0arcsec'],\
                                       ### center
                                       #phasecenter="J2000 19:59:28.500 +40.44.01.50",\
                                       ### offset
                                       #phasecenter="J2000 19:59:23.591 +40.44.01.50",\
                                       phasecenter=0,
                                       ftmachine='GridFT', startmodel='', weighting='briggs',\
                                       algo='hogbom',\
                                       niter=niter,cycleniter=cycleniter,\
                                       threshold=threshold,loopgain=loopgain,\
                                       interactive=interactive)



     ### Make a cluster def file if a parallel test is to run.
     if parallelmajor==True or parallelminor==True or parallelcube==True:
        clusterdef = 'cfgfile.txt'
        defstr = ''
        defstr = defstr + 'vega, 2,'+os.getcwd() #+'/aatest'
#        defstr = defstr + '\n'
#        defstr = defstr + 'tara, 2,'+os.getcwd() # +'/aatest'
        # remove file first
        if os.path.exists(clusterdef):
            os.system('rm -f '+ clusterdef)
        
        # save to a file    
        with open(clusterdef, 'w') as f:
            f.write(defstr)
        f.close()
     else:
        clusterdef=""

     ### Check input parameters, and parse outlier files.
     if paramList.checkParameters() == False:
        return [None, "", False, False,False]

     paramList.printParameters()

     return [ paramList , clusterdef, parallelmajor, parallelminor, parallelcube ]

#####################################################


#######################################
###    Function to run the Imaging.
#######################################
def doClean( params = [None,"",False,False,False] , doplot=True ):

    os.system('rm -rf mytest*')

    pmajor = params[2]
    pcube = params[4]

    if pmajor==False and pcube==False:
         imager = PySynthesisImager(params[0])
    elif pmajor==True:
         imager = PyParallelContSynthesisImager(params[0],params[1])
    elif pcube==True:
         imager = PyParallelCubeSynthesisImager(params[0],params[1])
    else:
         print 'Invalid parallel combination in doClean.'
         return

    ### Set up Imagers, Deconvolvers, IterControl, and ParallelSync.
    imager.initializeImagers()
    imager.initializeDeconvolvers()
    imager.initializeNormalizers()
    imager.initializeIterationControl()

    ### Run it.
    imager.makePSF()
    imager.runMajorMinorLoops()

    imager.restoreImages()

    if( doplot == True ):
         imager.getSummary();

    imager.deleteTools()
########################################


########################################
########################################
#  Run only Major Cycle
########################################
def doMajor( params = [None,"",False,False,False] , doplot=True , tomake='both'):

    os.system('rm -rf mytest*')

    pmajor=params[2]
    pcube = params[4]

    if pmajor==False and pcube==False:
         imager = PySynthesisImager(params[0])
    elif pmajor==True:
         imager = PyParallelContSynthesisImager(params[0],params[1])
    elif pcube==True:
         imager = PyParallelCubeSynthesisImager(params[0],params[1])
    else:
         print 'Invalid parallel combination in doClean.'
         return


    ### Set up Imagers and ParallelSync.
    imager.initializeImagers()
    imager.initializeNormalizers()

    ### Run it.
    if tomake=='both' or tomake=='psf':
         imager.makePSF()
    if tomake=='both':
         imager.runMajorCycle()

    imager.deleteTools()

########################################
########################################
###   Run only the minor cycle....
########################################
def doMinor( params = [None,"",False,False,False] , doplot=True ):

##    os.system('rm -rf mytest*')

    pminor=params[3]

    if pminor==False:
         imager = PySynthesisImager(params[0]) 
    else:
         imager = PyParallelDeconvolver(params[0],params[1])

    ### Set up Deconvolvers and IterControl
    imager.initializeDeconvolvers()
    imager.initializeIterationControl()

    # Run it
    while ( not imager.hasConverged() ):
        imager.runMinorCycle()

    imager.restoreImages()

    if( doplot == True ):
         imager.getSummary();

    imager.deleteTools()
########################################

########################################
#
#   Predict Model only (for use by setjy too)
#
########################################
########################################
#
#   Restore model image, Inputs : model, residual, psf.
#
########################################
def doRestore( params = [None,"",False,False,False] ):

    pminor=params[3]
    imager = PySynthesisImager(params[0]) 
    imager.initializeDeconvolvers()
    imager.restoreImages()
    imager.deleteTools()

########################################
def doSetjy( params = [None,"",False,False,False] ):

    pminor=params[3]
    imager = PySynthesisImager(params[0]) 
    imager.initializeDeconvolvers()

    imager.predictModel()

    imager.deleteTools()


########################################

def write_file(filename,str_text):
    '''Save the string in a text file'''
    inp = filename
    cmd = str_text
    # remove file first
    if os.path.exists(inp):
        os.system('rm -f '+ inp)
    # save to a file    
    with open(inp, 'w') as f:
        f.write(cmd)
    f.close()
    return


########################################

## Test function for live ImageStore transport between tools.
def toolTestMajorCycle( testnum=1 ):
     params = getparams( testnum=testnum )[0]

     SItool = casac.synthesisimager()
     SItool.initializemajorcycle( params.getSelPars(), params.getImagePars(), params.getGridPars() )

     PStool = casac.synthesisnormalizer()
     normpars = {'imagename':params.getImagePars()['0']['imagename']}
     PStool.setupnormalizer( normpars )

     PStool.scattermodel()
     SItool.makepsf()
     SItool.executemajorcycle()

     PStool.gatherpsfweight( )
     PStool.gatherresidual( )
     
     PStool.dividepsfbyweight()
     PStool.divideresidualbyweight()

     SItool.done()
     PStool.done()


###################################################

## Test function for parameter-list interface for the major cycle.
def toolTestMajorCycle2( testnum=1 ):

     os.system('rm -rf mytest*')

     params = getparams( testnum=testnum )[0]
     allselpars = params.getSelPars()
     allimagepars = params.getImagePars()
     allgridpars = params.getGridPars()

     SItool = casac.synthesisimager()

     for mss in sorted(allselpars.keys()):
          SItool.selectdata( **(allselpars[ mss ] ) )

     for fld in sorted(allimagepars.keys()):
          print 'Setup imaging and image for field ' + str(fld)
          SItool.defineimage( **(allimagepars[fld]) )

     SItool.setweighting( **allgridpars )

     PStools = []
     nfld = len( allimagepars.keys() )
     for fld in range(0, nfld ):
          PStool.append(casac.synthesisnormalizer())
          normpars = {'imagename':allimagepars[ (allimagepars.keys())[fld] ]['imagename']}
          PStool.setupnormalizer( normpars )

     for fld in range(0, nfld):
          PStool[fld].scattermodel()

     SItool.makepsf()
     SItool.executemajorcycle()

     for fld in range(0, nfld):
          PStool[fld].gatherpsfweight( )
          PStool[fld].gatherresidual( )
     
          PStool[fld].dividepsfbyweight()
          PStool[fld].divideresidualbyweight()
          
     SItool.done()

     for fld in range(0, nfld):
          PStool[fld].done()


###################################################


def checkDataPartitioningCode():

     ## Make parameter lists.
     #paramList = ImagerParameters(msname=['x1.ms','x2.ms'], field='0',spw=['0','2'], usescratch=True)
     ## Sync input lists to the same size.
     #paramList.checkParameters()

     params = getparams( testnum=3 ,parallelmajor=True )
     paramList = params[0]
     clusterdeffile = params[1]

     selpars = paramList.getSelPars()
     impars = paramList.getImagePars()

     synu = casac.synthesisutils()
     print synu.contdatapartition( selpars , 2)

     print synu.cubedatapartition( selpars, 2)
     
     print synu.cubeimagepartition( impars, 2)

     synu.done()

#     ppar = PyParallelImagerHelper(clusterdef=clusterdeffile)
#
#     print 'Selpars : ', selpars
#     newselpars = ppar.partitionCubeDataSelection( selpars )
#
#     print 'Impars : ', impars
#     newimpars = ppar.partitionCubeDeconvolution( impars )
#
#     ppar.takedownCluster()
#

     # The output dictionary should be indexed as follows ( for 2 nodes, and 2 MSs )
     #{ '0' : { 'ms0' : { 'msname':xxx1, 'spw':yyy1 } ,
     #            'ms1' : { 'msname':xxx2, 'spw':yyy1 } 
     #         }
     #  '1' : { 'ms0' : { 'msname':xxx1, 'spw':yyy2 } ,
     #            'ms1' : { 'msname':xxx2, 'spw':yyy2 } 
     #        }
     # }
     #


def checkPars():

     params = getparams( testnum=7 )
     paramList = params[0]

     selpars = paramList.getSelPars()
     impars = paramList.getImagePars()

     synu = casac.synthesisutils()

#     print selpars['ms0']
#     fixrec = synu.checkselectionparams( selpars['ms0'] )
#     print fixrec
#     print synu.checkselectionparams( fixrec )


     print impars['0']
     fixrec = synu.checkimageparams( impars['0'] )
     print "---------------------------------------------"
     print fixrec
     print "---------------------------------------------"
     print synu.checkimageparams( fixrec )

     synu.done()

def testImageCoordinates( testnum=1, testid=0):
    
    multitest=False
    if testnum==13: multitest=True
    if multitest and testid==-1: testid=range(21)
    if type(testid)==int: testid=[testid]
    for tst in testid:
      params = getparams( testnum, testid=tst )
      paramList = params[0]
      impars=paramList.getImagePars()['0']
      if os.path.exists(impars['imagename']+'.residual'):
        os.system('rm -rf '+impars['imagename']+'*')

      doMajor(params)

#def testmakeimage():
#     params = getparams( testnum=1 )
#     paramList = params[0]
#     impars = (paramList.getImagePars())['0']
#
#     os.system('rm -rf ' + impars['imagename'])
#
#     synu = casac.synthesisutils()
#     #synu.makeimage( impars , 'DataTest/twopoints_twochan.ms')
#     synu.makeimage( impars , 'DataTest/point_twospws.ms')
#     synu.done()
