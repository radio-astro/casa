import shutil

#
# Test of flagdata modes
#

def test_eq(result, total, flagged):

    print "%s of %s data was flagged, expected %s of %s" % \
    (result['flagged'], result['total'], flagged, total)
    if result['total'] != total:
        raise Exception("%s data in total; %s expected" % (result['total'], total))
    if result['flagged'] != flagged:
        raise Exception("%s flags set; %s expected" % (result['flagged'], flagged))


default(flagdata)

vis = 'flagdatatest.ms'
if False:
    # Do not use copytree because of
    # http://bugs.python.org/issue1545
    shutil.copytree(os.environ.get('CASAPATH').split()[0] +
                    "/data/regression/flagdata/" + vis,
                    vis)
else:
    os.system('cp -r ' + \
              os.environ.get('CASAPATH').split()[0] +
              "/data/regression/flagdata/" + vis + ' ' + vis)

flagdata(vis=vis, unflag=true)




print "Test of vector mode"
default(flagdata)
vis = 'flagdatatest.ms'
clipminmax=[0.0, 0.2]
antenna = ['1', '2']
clipcolumn = ['DATA', 'datA']

inp(flagdata)
flagdata()
test_eq(flagdata(vis=vis, mode='summary'), 2000700, 465398)
flagdata(vis=vis, unflag=true)



print "Test of flagmanager mode=list, flagbackup=True/False"
flagmanager(vis=vis, mode='list')
fg.open(vis)
if len(fg.getflagversionlist()) != 5:
    raise Exception()
fg.done()

flagdata(vis=vis, unflag=true, flagbackup=false)
flagmanager(vis=vis, mode='list')
fg.open(vis)
if len(fg.getflagversionlist()) != 5:
    raise Exception()
fg.done()

flagdata(vis=vis, unflag=true, flagbackup=true)
flagmanager(vis=vis, mode='list')
fg.open(vis)
if len(fg.getflagversionlist()) != 6:
    raise Exception()
fg.done()

print "Test of flagmanager mode=rename"
flagmanager(vis=vis, mode='rename', oldname='manualflag_3', versionname='Ha! The best version ever!', comment='This is a *much* better name')
flagmanager(vis=vis, mode='list')
fg.open(vis)
if len(fg.getflagversionlist()) != 6:
    raise Exception()
fg.done()


print "Test of flagging statistics and queries"
vis='ngc5921.ms'
os.system('rm -rf ' + vis)
importuvfits(os.environ.get('CASAPATH').split()[0] + \
             '/data/regression/ngc5921/ngc5921.fits', \
             vis)

flagdata(vis=vis, unflag=true)
flagdata(vis=vis, correlation='LL')
flagdata(vis=vis, spw='0:17~19')
flagdata(vis=vis, antenna='5&&9')
flagdata(vis=vis, antenna='14')
flagdata(vis=vis, field='1')
s = flagdata(vis=vis, mode='summary', minrel=0.9)
assert s['antenna'].keys() == ['14']
assert '5&&9' in s['baseline'].keys()
assert set(s['channel'].keys()) == set(['17', '18', '19'])
assert s['correlation'].keys() == ['1']  # LL
assert s['field'].keys() == ['1']
assert set(s['scan'].keys()) == set(['2', '4', '5', '7']) # field 1
s = flagdata(vis=vis, mode='summary', maxrel=0.8)
assert set(s['field'].keys()) == set(['0', '2'])
s = flagdata(vis=vis, mode='summary', minabs=400000)
assert set(s['scan'].keys()) == set(['3', '6'])
s = flagdata(vis=vis, mode='summary', minabs=400000, maxabs=450000)
assert s['scan'].keys() == ['3']
flagdata(vis=vis, unflag=true)


print "Test of autoflag, algorithm=timemed"
flagdata(vis=vis, mode='autoflag', algorithm='timemed', window=3)
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 4725)
flagdata(vis=vis, unflag=true)

print "Test of autoflag, algorithm=freqmed"
flagdata(vis=vis, mode='autoflag', algorithm='freqmed')
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 28916)
flagdata(vis=vis, unflag=true)


print "Test of channel average"
flagdata(vis=vis, channelavg=False, clipminmax=[30, 60])
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 1414186)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, channelavg=True, clipminmax=[30, 60])
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 1347822)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, channelavg=False, clipminmax=[30, 60], spw='0:0~10')
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 242053)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, channelavg=True, clipminmax=[30, 60], spw='0:0~10')
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 231374)
flagdata(vis=vis, unflag=true)


print "Test of mode = 'quack'"
print "parallel quack"
flagdata(vis=vis, mode='quack', quackinterval=[1.0, 5.0], antenna=['2', '3'], correlation='RR')
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 22365)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, mode='quack', quackmode='beg', quackinterval=1)
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 329994)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, mode='quack', quackmode='endb', quackinterval=1)
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 333396)
flagdata(vis=vis, unflag=true)


flagdata(vis=vis, mode='quack', quackmode='end', quackinterval=1)
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 2520882)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, mode='quack', quackmode='tail', quackinterval=1)
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 2524284)
flagdata(vis=vis, unflag=true)

print "quack mode quackincrement"
flagdata(vis=vis, mode='quack', quackinterval=50, quackmode='endb', quackincrement=true)
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 571536)

flagdata(vis=vis, mode='quack', quackinterval=20, quackmode='endb', quackincrement=true)
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 857304)

flagdata(vis=vis, mode='quack', quackinterval=150, quackmode='endb', quackincrement=true)
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 1571724)

flagdata(vis=vis, mode='quack', quackinterval=50, quackmode='endb', quackincrement=true)
test_eq(flagdata(vis=vis, mode='summary'), 2854278, 1762236)
flagdata(vis=vis, unflag=true)

flagmanager(vis=vis, mode='list')


print "Test of mode='rfi'"
vis='flagdatatest.ms'
flagdata(vis=vis, mode='rfi')
test_eq(flagdata(vis=vis, mode='summary'), 2000700, 9142)
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 148200, 354)
flagdata(vis=vis, unflag=true)

print "Test of mode = 'shadow'"
flagdata(vis=vis, mode='shadow', diameter=40)
test_eq(flagdata(vis=vis, mode='summary'), 2000700, 38610)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, mode='shadow')
test_eq(flagdata(vis=vis, mode='summary'), 2000700, 15860)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, mode='shadow', correlation='LL')
test_eq(flagdata(vis=vis, mode='summary'), 2000700, 7930)
flagdata(vis=vis, unflag=true)

flagmanager(vis=vis, mode='list')

print "Test various selections"
vis='ngc5921.ms'
flagdata(vis=vis, antenna='2')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 196434, 196434)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, spw='0')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 196434, 196434)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, correlation='LL')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 196434, 98217)
flagdata(vis=vis, correlation='LL,RR')
flagdata(vis=vis, correlation='LL RR')
flagdata(vis=vis, correlation='LL ,, ,  ,RR')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 196434, 196434)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, field='0')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 196434, 39186)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, uvrange='200~400m')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 196434, 55944)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, timerange='09:50:00~10:20:00')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 196434, 6552)
flagdata(vis=vis, unflag=true)

flagdata(vis=vis, scan='3')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 196434, 52416)
flagdata(vis=vis, unflag=true)

# feed not implemented flagdata(vis=vis, feed='27')
# flagdata(vis=vis, unflag=true)

flagdata(vis=vis, array='0')
test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 196434, 196434)
flagdata(vis=vis, unflag=true)

flagmanager(vis=vis, mode='list')


if False: 
    print "Test of mode = 'alma'"
    vis='cwilson2.ms'
    for r in ['vla_recipe', 'pdb_recipe']:
        os.system('rm -rf ' + vis)
        shutil.copytree(os.environ.get('CASAPATH').split()[0] +
                        "/data/regression/flagdata/" + vis,
                        vis)
        listobs(vis=vis, verbose=false)
        flagdata(vis=vis, unflag=true)
        flagdata(vis=vis, mode='alma', recipe=r, gain=['1'], flux=['0'], bpass=['0'], source=['2'])
        if r == 'vla_recipe':
            expected = 89334
        else:
            expected = 90846
        test_eq(flagdata(vis=vis, mode='summary', antenna='2'), 261450, expected)

    flagmanager(vis=vis, mode='list')
