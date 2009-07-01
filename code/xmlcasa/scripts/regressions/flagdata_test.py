import shutil

#
# This test script just runs flagdata() in various modes,
# but currently does no checking of the results
#

def test_eq(result, total, flagged):

    print "%s of %s data was flagged, expected %s of %s" % \
    (result['flagged'], result['total'], flagged, total)
    if result['total'] != total:
        raise Exception("%s flags in total; %s expected" % (result['total'], total))
    if result['flagged'] != flagged:
        raise Exception("%s flags set; %s expected" % (result['flagged'], flagged))


# Test of mode=alma
vis='ngc5921.ms'
os.system('rm -rf ' + vis)
importuvfits(os.environ.get('CASAPATH').split()[0] + \
             '/data/regression/ngc5921/ngc5921.fits', \
             vis)
listobs(vis=vis, verbose=false)
flagdata(vis=vis, unflag=true)
flagdata(vis=vis, mode='alma', recipe='vla_recipe')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 196434, 0)


# Tests of anything else
vis = 'pointingtest.ms'

shutil.copytree(os.environ.get('CASAPATH').split()[0] +
                "/data/regression/pointing/" + vis,
                vis)

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

test_eq(flagdata(vis=vis, mode='summary'), 2000700, 305052)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, mode='shadow', diameter=40)
test_eq(flagdata(vis=vis, mode='summary'), 2000700, 15860) # doesn't work
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, mode='shadow')
test_eq(flagdata(vis=vis, mode='summary'), 2000700, 15860)
flagdata(vis=vis, unflag=true)


flagdata(vis=vis, mode='shadow', correlation='LL')
test_eq(flagdata(vis=vis, mode='summary'), 2000700, 7930)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, antenna='2')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 148200, 148200)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, spw='0')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 148200, 148200)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, correlation='LL')
flagdata(vis=vis, correlation='LL,RR')
flagdata(vis=vis, correlation='LL RR')
flagdata(vis=vis, correlation='LL ,, ,  ,RR')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 148200, 148200)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, field='0')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 148200, 148200)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, uvrange='200~400m')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 148200, 31388)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, timerange='20:21:00~23:00:00')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 148200, 49608)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, scan='3')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 148200, 52)
flagdata(vis=vis, unflag=true)

# feed not implemented flagdata(vis=vis, feed='27')
# flagdata(vis=vis, unflag=true)

flagdata(vis=vis, array='0')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 148200, 148200)
flagdata(vis=vis, unflag=true)
