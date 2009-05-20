import shutil

#
# This test script just runs flagdata() in various modes,
# but currently does no checking of the results
#

theMS = 'pointingtest.ms'

shutil.copytree(os.environ.get('CASAPATH').split()[0]+"/data/regression/pointing/"+theMS,
                theMS)

vis=theMS

do_vector = True
if do_vector:
    mode='manualflag'
    clipexpr=['ABS RR']
    clipminmax=[[0,0]]
    clipcolumn=['DATA']
    clipoutside=[true]
    autocorr=[false]
    antenna=['2']
    unflag=[false]
    spw=['']
    correlation=['']
    field=['']
    uvrange=['']
    timerange=['']
    scan=['']
    feed=['']
    array=['']
    
    inp(flagdata)
    flagdata()
    
    clipexpr *= 2
    clipminmax *= 2
    clipcolumn *= 2
    clipoutside *= 2
    autocorr *= 2
    antenna *= 2
    unflag *= 2
    spw *= 2
    correlation *= 2
    field *= 2
    uvrange *= 2
    timerange *= 2
    scan *= 2
    feed *= 2
    array *= 2
    
    antenna = ['1', '2']
    
    inp(flagdata)
    flagdata()

flagdata(vis=vis, antenna='2')
flagdata(vis=vis, mode='summary', antenna='2')
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, spw='0')
flagdata(vis=vis, mode='summary', antenna='2')
flagdata(vis=vis, unflag=true)
flagdata(vis=vis, correlation='LL')
flagdata(vis=vis, correlation='LL,RR')
flagdata(vis=vis, correlation='LL RR')
flagdata(vis=vis, correlation='LL ,, ,  ,RR')
flagdata(vis=vis, mode='summary', antenna='2')
flagdata(vis=vis, unflag=true)
flagdata(vis=vis, field='0')
flagdata(vis=vis, mode='summary', antenna='2')
flagdata(vis=vis, unflag=true)
flagdata(vis=vis, uvrange='200~400m')
flagdata(vis=vis, mode='summary', antenna='2')
flagdata(vis=vis, unflag=true)
flagdata(vis=vis, timerange='20:21:00~23:00:00')
flagdata(vis=vis, mode='summary', antenna='2')
flagdata(vis=vis, unflag=true)
flagdata(vis=vis, scan='3')
flagdata(vis=vis, mode='summary', antenna='2')
flagdata(vis=vis, unflag=true)
# feed not implemented flagdata(vis=vis, feed='27')
# flagdata(vis=vis, unflag=true)
flagdata(vis=vis, array='0')
flagdata(vis=vis, mode='summary', antenna='2')
flagdata(vis=vis, unflag=true)
